//
// Copyright(C) 2023 by Ryan Krafnick
// Copyright(C) 2018-2026 Julia Nechaevskaya
// Copyright(C) 2024-2026 Polina "Aura" N.
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
//  DSDA Font, embedded into CRL executable.
//

#include <ctype.h>      // toupper
#include "dsda_font.h"
#include "i_swap.h"     // SHORT
#include "m_misc.h"     // M_snprintf
#include "v_patch.h"
#include "v_video.h"
#include "w_wad.h"      // W_CheckNumForName, W_CacheLumpName
#include "z_zone.h"     // PU_STATIC


#define HU_FONTSTART    33
#define HU_FONTSIZE     94      // [PN] ASCII 33 ('!') through ASCII 126 ('~').

patch_t *dsda_font[HU_FONTSIZE];


// -----------------------------------------------------------------------------
// DSDA_FontInit
//  Initialize DSDA font at program startup. Called by CT_Init.
//  [PN] Every glyph is looked up as a lump first, so a font provided by a PWAD
//  replaces the embedded one, glyph by glyph. The embedded patches are
//  the fallback, so the font works even when nothing provides them.
// -----------------------------------------------------------------------------

void DSDA_FontInit(void)
{
    for (int i = 0; i < HU_FONTSIZE; i++)
    {
        char name[10];

        M_snprintf(name, sizeof(name), "DIG%03d", HU_FONTSTART + i);

        // [JN] CRY - always load fonts from CRY.wad.
        dsda_font[i] = W_CacheLumpName(name, PU_STATIC);
    }
}

// -----------------------------------------------------------------------------
// DSDA_DrawText
// -----------------------------------------------------------------------------

void DSDA_DrawText(int x, int y, const char *const text, byte *const table)
{
    dp_translation = table;

    for (const char *ch = text; *ch; ch++)
    {
        const int c = toupper((unsigned char)*ch);

        if (c < 33 || c > 126)
        {
            x += 4;  // Space or non-printable character
            continue;
        }

        patch_t *const patch = dsda_font[c - HU_FONTSTART];
        V_DrawPatch(x, y, patch);
        x += patch->width;
    }
    
    dp_translation = NULL;
}

// -----------------------------------------------------------------------------
// DSDA_DrawTextCentered
// -----------------------------------------------------------------------------

void DSDA_DrawTextCentered(int y, const char *const text, byte *const table)
{
    // Find width
    int total_width = 0;
    for (const char *ch = text; *ch; ch++)
    {
        const int c = toupper((unsigned char)*ch);
        const int idx = c - HU_FONTSTART;
        
        if (idx >= 0 && idx < HU_FONTSIZE && dsda_font[idx])
            total_width += dsda_font[idx]->width;
        else if (c < 33 || c > 126)
            total_width += 4;  // Space or non-printable character
    }

    // Draw text on the screen
    int x = (320 - total_width) / 2; // 320 = SCREENWIDTH
    dp_translation = table;

    for (const char *ch = text; *ch; ch++)
    {
        const int c = toupper((unsigned char)*ch);
        const int idx = c - HU_FONTSTART;

        if (idx >= 0 && idx < HU_FONTSIZE && dsda_font[idx])
        {
            patch_t *const patch = dsda_font[idx];
            V_DrawPatch(x, y, patch);
            x += patch->width;
        }
        else if (c < 33 || c > 126)
        {
            x += 4;
        }
    }

    dp_translation = NULL;
}

// -----------------------------------------------------------------------------
// DSDA_StringWidth
// -----------------------------------------------------------------------------

int DSDA_StringWidth(const char *const string)
{
    int w = 0;

    for (int i = 0; i < strlen(string); i++)
    {
        const int c = toupper(string[i]) - HU_FONTSTART;

        if (c < 0 || c >= HU_FONTSIZE)
        w += 4;
        else
        w += SHORT(dsda_font[c]->width);
    }

    return w;
}
