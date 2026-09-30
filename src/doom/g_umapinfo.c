//
// Copyright(C) 2017 Christoph Oelckers
// Copyright(C) 2021 Roman Fomin
// Copyright(C) 2026 Polina "Aura" N.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
// [PN] Minimal UMAPINFO support for CRY (a compact subset of Woof's
// g_umapinfo.c, tailored to the fields cry.wad actually uses).
//
// Parses the optional UMAPINFO lump once at startup (after the WADs are
// loaded) and applies the supported keys. Currently applied:
//   - levelname      -> overrides level_names[] (automap & intermission)
//   - skytexture     -> per-map sky, see G_InitSkyTextures()
//   - skytexture2    -> per-map second sky layer (Jaguar double-sky)
//   - skyscrollspeed -> per-map sky panning speed (cry.wad extension)
//   - music          -> per-map music lump, see S_ChangeMusic()
//   - next           -> level advance, see G_DoCompleted()
//   - nextsecret     -> secret exit destination, see G_DoCompleted()
//   - intertext      -> finale text shown after this level, see F_StartFinale()
//   - endcast        -> roll call after the text, see F_StartFinale()/F_Ticker()

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomtype.h"
#include "z_zone.h"
#include "w_wad.h"
#include "p_local.h"
#include "g_umapinfo.h"

// -----------------------------------------------------------------------------
// Tokenizer
// -----------------------------------------------------------------------------

enum
{
    UT_EOF,        // end of lump
    UT_WORD,       // bare identifier/number, stored in umi_kw
    UT_STR,        // quoted string (may span lines), stored in umi_str
    UT_PUNCT       // single character { } = etc., stored in umi_kw[0]
};

#define UMI_MAX_TOKEN UMI_TEXT_LEN   // tokenizer holds up to a full intertext
#define UMI_NAME_LEN  33   // UMAPINFO spec: levelname is max 32 chars

static char         umi_kw[UMI_MAX_TOKEN];
static char         umi_str[UMI_MAX_TOKEN];
static char         umi_keybuf[UMI_MAX_TOKEN];
static const char  *umi_key;        // saved key of the current "key = value"
static const char  *umi_rover;
static const char  *umi_limit;

// Parsed level names live here; level_names[i] is redirected to them.
static char *umi_levelnames;        // [num_level_names][UMI_NAME_LEN]

// Per-map sky overrides (see umapinfo_map_t in the header).
static umapinfo_map_t *umi_maps;    // [num_level_names]

static boolean umi_ieq (const char *a, const char *b)
{
    while (*a && *b)
    {
        const int ca = (*a >= 'A' && *a <= 'Z') ? *a + 32 : *a;
        const int cb = (*b >= 'A' && *b <= 'Z') ? *b + 32 : *b;

        if (ca != cb)
            return false;

        a++;
        b++;
    }
    return *a == *b;
}

static boolean umi_is_wordchar (int c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
}

// Fetch next token. Words and single punctuation land in umi_kw,
// quoted strings in umi_str.
static int umi_next (void)
{
    for (;;)
    {
        // skip whitespace
        while (umi_rover < umi_limit &&
               (*umi_rover == ' '  || *umi_rover == '\t' ||
                *umi_rover == '\r' || *umi_rover == '\n'))
        {
            umi_rover++;
        }

        if (umi_rover >= umi_limit)
            return UT_EOF;

        // // comment: skip to end of line
        if (*umi_rover == '/' &&
            umi_rover + 1 < umi_limit && umi_rover[1] == '/')
        {
            while (umi_rover < umi_limit && *umi_rover != '\n')
                umi_rover++;
            continue;
        }

        // # comment (UMAPINFO spec): skip to end of line
        if (*umi_rover == '#')
        {
            while (umi_rover < umi_limit && *umi_rover != '\n')
                umi_rover++;
            continue;
        }

        // quoted string: raw copy until the closing quote (may span lines)
        if (*umi_rover == '"')
        {
            int n = 0;

            umi_rover++;
            while (umi_rover < umi_limit && *umi_rover != '"')
            {
                if (n < UMI_MAX_TOKEN - 1)
                    umi_str[n++] = *umi_rover;
                umi_rover++;
            }
            umi_str[n] = '\0';

            if (umi_rover < umi_limit)
                umi_rover++;                    // closing quote

            return UT_STR;
        }

        // bare word
        if (umi_is_wordchar(*umi_rover))
        {
            int n = 0;

            while (umi_rover < umi_limit && umi_is_wordchar(*umi_rover))
            {
                if (n < UMI_MAX_TOKEN - 1)
                    umi_kw[n++] = *umi_rover;
                umi_rover++;
            }
            umi_kw[n] = '\0';
            return UT_WORD;
        }

        // any other single character: punctuation
        umi_kw[0] = *umi_rover++;
        umi_kw[1] = '\0';
        return UT_PUNCT;
    }
}

// -----------------------------------------------------------------------------
// Map label -> 0-based index ("MAP03" -> 2, "E1M7" -> 6); -1 if not mappable.
// CRY uses one flat mapset, so ExMy folds like MAPxx.
// -----------------------------------------------------------------------------

static int umi_mapindex (const char *label)
{
    if ((label[0] == 'M' || label[0] == 'm') &&
        (label[1] == 'A' || label[1] == 'a') &&
        (label[2] == 'P' || label[2] == 'p') &&
         label[3] >= '0' && label[3] <= '9')
    {
        return atoi(label + 3) - 1;
    }

    if ((label[0] == 'E' || label[0] == 'e') &&
         label[1] >= '1' && label[1] <= '9'  &&
        (label[2] == 'M' || label[2] == 'm') &&
         label[3] >= '1' && label[3] <= '9')
    {
        return (label[1] - '1') * 9 + (label[3] - '1');
    }

    return -1;
}

// -----------------------------------------------------------------------------
// Parsing
// -----------------------------------------------------------------------------

// Consume tokens until the matching '}' of a block we are not interested in.
static void umi_skip_block (void)
{
    int depth = 1;

    for (;;)
    {
        const int t = umi_next();

        if (t == UT_EOF)
            return;

        if (t == UT_PUNCT)
        {
            if (umi_kw[0] == '{')
                depth++;
            else if (umi_kw[0] == '}' && --depth == 0)
                return;
        }
    }
}

// Parse the body of a "MAP MAPxx {" block; idx is the 0-based map number
// (may be -1 for unknown labels: keys are consumed but applied to nothing).
static void umi_parse_map_block (int idx)
{
    for (;;)
    {
        int t = umi_next();

        if (t == UT_EOF)
            return;

        if (t == UT_PUNCT && umi_kw[0] == '}')
            return;                                  // end of map block

        if (t != UT_WORD)
            continue;

        // Save the key before further reads clobber umi_kw.
        umi_key = umi_keybuf;
        strcpy(umi_keybuf, umi_kw);

        // All supported fields are "key = value"; require the '='.
        if (umi_next() != UT_PUNCT || umi_kw[0] != '=')
            continue;

        t = umi_next();                              // string or bare word
        if (t == UT_EOF)
            return;
        if (t != UT_STR && t != UT_WORD)
            continue;

        {
            const char *value = (t == UT_STR) ? umi_str : umi_kw;

            // --- applied now ---
            if (idx >= 0 && idx < num_level_names)
            {
                if (umi_ieq(umi_key, "levelname"))
                {
                    char *dst = umi_levelnames + idx * UMI_NAME_LEN;

                    strncpy(dst, value, UMI_NAME_LEN - 1);
                    dst[UMI_NAME_LEN - 1] = '\0';
                    level_names[idx] = dst;
                }
                else if (umi_ieq(umi_key, "skytexture"))
                {
                    strncpy(umi_maps[idx].sky1, value, 8);
                    umi_maps[idx].sky1[8] = '\0';
                }
                else if (umi_ieq(umi_key, "skytexture2"))
                {
                    strncpy(umi_maps[idx].sky2, value, 8);
                    umi_maps[idx].sky2[8] = '\0';
                }
                else if (umi_ieq(umi_key, "skyscrollspeed"))
                {
                    const int sp = atoi(value);
                    umi_maps[idx].speed = (sp >= 0) ? sp : -1;
                }
                else if (umi_ieq(umi_key, "music"))
                {
                    strncpy(umi_maps[idx].music, value, 8);
                    umi_maps[idx].music[8] = '\0';
                }
                else if (umi_ieq(umi_key, "next"))
                {
                    umi_maps[idx].next = umi_mapindex(value);
                }
                else if (umi_ieq(umi_key, "nextsecret"))
                {
                    umi_maps[idx].nextsecret = umi_mapindex(value);
                }
                else if (umi_ieq(umi_key, "intertext"))
                {
                    strncpy(umi_maps[idx].intertext, value, UMI_TEXT_LEN - 1);
                    umi_maps[idx].intertext[UMI_TEXT_LEN - 1] = '\0';
                }
                else if (umi_ieq(umi_key, "endcast"))
                {
                    umi_maps[idx].endcast =
                        (umi_ieq(value, "true") || umi_ieq(value, "1")) ? 1 :
                        (umi_ieq(value, "false") || umi_ieq(value, "0")) ? 0 : -1;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
// UMAPINFO_Parse
// -----------------------------------------------------------------------------

void UMAPINFO_Parse (void)
{
    const lumpindex_t lump = W_CheckNumForName("UMAPINFO");
    const char *data;

    if (lump < 0)
        return;                                      // no lump: vanilla names

    if (!umi_levelnames)
    {
        int i;

        umi_levelnames = Z_Malloc(num_level_names * UMI_NAME_LEN,
                                  PU_STATIC, NULL);
        umi_maps = Z_Malloc(num_level_names * sizeof(umi_maps[0]),
                            PU_STATIC, NULL);

        for (i = 0; i < num_level_names; i++)
        {
            umi_maps[i].sky1[0] = '\0';
            umi_maps[i].sky2[0] = '\0';
            umi_maps[i].music[0] = '\0';
            umi_maps[i].intertext[0] = '\0';
            umi_maps[i].speed = -1;
            umi_maps[i].next = -1;
            umi_maps[i].nextsecret = -1;
            umi_maps[i].endcast = -1;
        }
    }

    data = (const char *) W_CacheLumpNum(lump, PU_STATIC);
    umi_rover = data;
    umi_limit = data + W_LumpLength(lump);

    for (;;)
    {
        const int t = umi_next();

        if (t == UT_EOF)
            break;

        if (t != UT_WORD)
            continue;

        if (umi_ieq(umi_kw, "map"))
        {
            int idx;

            if (umi_next() != UT_WORD)               // map label
                continue;

            idx = umi_mapindex(umi_kw);

            if (umi_next() == UT_PUNCT && umi_kw[0] == '{')
                umi_parse_map_block(idx);
            // No opening brace: our WADs never do that; the stray token is
            // simply dropped on the next outer iteration.
        }
        else if (umi_ieq(umi_kw, "episode"))
        {
            // Swallow the block so its keys are not read as top-level ones.
            boolean saw_brace = false;
            int t2;

            while ((t2 = umi_next()) != UT_EOF)
            {
                if (t2 == UT_PUNCT && umi_kw[0] == '{')
                {
                    saw_brace = true;
                    break;
                }
            }

            if (saw_brace)
                umi_skip_block();
        }
    }

    W_ReleaseLumpNum(lump);
}

// -----------------------------------------------------------------------------
// UMAPINFO_GetMap
// -----------------------------------------------------------------------------

const umapinfo_map_t *UMAPINFO_GetMap (int mapnum)
{
    if (!umi_maps || mapnum < 1 || mapnum > num_level_names)
        return NULL;

    return &umi_maps[mapnum - 1];
}
