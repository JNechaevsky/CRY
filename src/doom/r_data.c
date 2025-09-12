//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2025 Julia Nechaevskaya
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
//	Preparation of data for rendering,
//	generation of lookups, caching, retrieval by name.
//

#include <stdio.h>
#include "i_swap.h"
#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"
#include "m_misc.h"
#include "p_local.h"
#include "r_collit.h"
#include "doomstat.h"
#include "v_trans.h"
#include "v_video.h"

#include "id_vars.h"


//
// Graphics.
// DOOM graphics for walls and sprites
// is stored in vertical runs of opaque pixels (posts).
// A column is composed of zero or more posts,
// a patch or sprite is composed of zero or more columns.
// 



//
// Texture definition.
// Each texture is composed of one or more patches,
// with patches being lumps stored in the WAD.
// The lumps are referenced by number, and patched
// into the rectangular texture space using origin
// and possibly other attributes.
//
typedef PACKED_STRUCT (
{
    short	originx;
    short	originy;
    short	patch;
    short	stepdir;
    short	colormap;
}) mappatch_t;


//
// Texture definition.
// A DOOM wall texture is a list of patches
// which are to be combined in a predefined order.
//
typedef PACKED_STRUCT (
{
    char		name[8];
    int			masked;	
    short		width;
    short		height;
    int                 obsolete;
    short		patchcount;
    mappatch_t	patches[1];
}) maptexture_t;


// A single patch from a texture definition,
//  basically a rectangular area within
//  the texture rectangle.
typedef struct
{
    // Block origin (allways UL),
    // which has allready accounted
    // for the internal origin of the patch.
    short	originx;	
    short	originy;
    int		patch;
} texpatch_t;


// A maptexturedef_t describes a rectangular texture,
//  which is composed of one or more mappatch_t structures
//  that arrange graphic patches.

typedef struct texture_s texture_t;

struct texture_s
{
    // Keep name for switch changing, etc.
    char	name[8];		
    short	width;
    short	height;

    // Index in textures list

    int         index;

    // Next in hash table chain

    texture_t  *next;
    
    // All the patches[patchcount]
    //  are drawn back to front into the cached texture.
    short	patchcount;
    texpatch_t	patches[1];		
};



int		firstflat;
int		lastflat;
int		numflats;

int		firstpatch;
int		lastpatch;
int		numpatches;

int		firstspritelump;
int		lastspritelump;
int		numspritelumps;

int		numtextures;
texture_t**	textures;
texture_t**     textures_hashtable;


int*			texturewidthmask;
int*			texturewidth; // [crispy] texture width for wrapping column getter function
// needed for texture pegging
fixed_t*		textureheight; // [crispy] texture height for Tutti-Frutti fix
int*			texturecompositesize;
short**			texturecolumnlump;
unsigned**		texturecolumnofs; // [crispy] column offsets for composited translucent mid-textures on 2S walls
unsigned**		texturecolumnofs2; // [crispy] column offsets for composited opaque textures
byte**			texturecomposite; // [crispy] composited translucent mid-textures on 2S walls
byte**			texturecomposite2; // [crispy] composited opaque textures
const byte**	texturebrightmap; // [crispy] brightmaps

// for global animation
int*		flattranslation;
int*		texturetranslation;

// needed for pre rendering
fixed_t*	spritewidth;	
fixed_t*	spriteoffset;
fixed_t*	spritetopoffset;

lighttable_t	*colormaps;
lighttable_t	*invulmaps;
lighttable_t	*pal_color; // [crispy] array holding palette colors for true color mode
lighttable_t	*cry_color; // [JN] array holding CRY patch drawing palette colors


//
// MAPTEXTURE_T CACHING
// When a texture is first needed,
//  it counts the number of composite columns
//  required in the texture and allocates space
//  for a column directory and any new columns.
// The directory will simply point inside other patches
//  if there is only one patch in a given column,
//  but any columns with multiple patches
//  will have new column_ts generated.
//



// -----------------------------------------------------------------------------
// R_DrawColumnInCache
// [PN] Clips and draws a patch column into the cache with branch-reduced
// clipping and restrict hints for faster execution. Based on Killough’s
// rewrite that fixed the Medusa bug.
// -----------------------------------------------------------------------------

static void R_DrawColumnInCache
(
    const column_t *restrict patch,
    byte *restrict           cache,
    int                      originy,
    int                      cacheheight,
    byte *restrict           marks
)
{
    // [PN] Aliasing hints + const where applicable
    const column_t *restrict col   = (const column_t *)patch;
    byte           *restrict dst   = cache;
    byte           *restrict mark  = marks;

    // [PN] Hot locals, declared near use
    int top = -1;

    // [PN] Single-pass, clamp with start/end to reduce branches
    while (col->topdelta != 0xff)
    {
        const int topdelta = col->topdelta;

        // [crispy] support for DeePsea tall patches
        if (topdelta <= top)
        {
            top += topdelta;
        }
        else
        {
            top = topdelta;
        }

        const int count = col->length;
        const int start = originy + top;
        const int end   = start + count;

        // Clip once; compute copy window [dst_start, dst_end)
        const int dst_start = start < 0 ? 0 : start;
        const int dst_end   = end   > cacheheight ? cacheheight : end;
        const int n         = dst_end - dst_start;

        if (n > 0)
        {
            // Source pointer adjusted by the amount we clipped at the top
            const byte *restrict src = (const byte *)col + 3 + (dst_start - start);

            // memcpy + memset; aliasing is restricted for better codegen
            memcpy(dst  + dst_start, src,  (size_t)n);
            memset(mark + dst_start, 0xFF, (size_t)n);
        }

        // Advance to next post (length bytes + 4 header bytes)
        col = (const column_t *)((const byte *)col + col->length + 4);
    }
}

// -----------------------------------------------------------------------------
// R_GenerateComposite
// [PN] Builds composite columns for a texture and reconstructs true posts from
//  transparency marks with fewer branches and restrict-qualified pointers.
//  Based on Killough’s rewrite that fixed the Medusa bug.
// -----------------------------------------------------------------------------

static void R_GenerateComposite (int texnum)
{
    texture_t *const texture = textures[texnum];
    const int width  = texture->width;
    const int height = texture->height;

    byte *const block = Z_Malloc(texturecompositesize[texnum], PU_STATIC, &texturecomposite[texnum]);
    byte *const block2 = Z_Malloc((size_t)width * (size_t)height, PU_STATIC, &texturecomposite2[texnum]);

    short    *restrict collump = texturecolumnlump[texnum];
    unsigned *restrict colofs  = texturecolumnofs [texnum];
    unsigned *restrict colofs2 = texturecolumnofs2[texnum];

    // marks[y + x*height] == 0xFF for opaque pixels of column x
    byte *const marks = (byte *)calloc((size_t)width, (size_t)height);

    // [crispy] initialize composite background to palette index 0 (usually black)
    memset(block, 0, (size_t)texturecompositesize[texnum]);

    // 1) Composite all contributing patches into 'block' and stamp marks[]
    for (int i = 0; i < texture->patchcount; ++i)
    {
        const texpatch_t *const patch = &texture->patches[i];
        const patch_t *const realpatch = W_CacheLumpNum(patch->patch, PU_CACHE);

        int x1 = patch->originx;
        int x2 = x1 + SHORT(realpatch->width);

        if (x1 < 0)        x1 = 0;
        if (x2 > width)    x2 = width;
        if (x1 >= x2)      continue;

        // Column offsets base, indexed by absolute x
        const int *const cofs_base = realpatch->columnofs - x1;
        const byte *const rp_base  = (const byte *)realpatch;

        for (int x = x1; x < x2; ++x)
        {
            const unsigned ofs = (unsigned)LONG(cofs_base[x]);
            const column_t *patchcol = (const column_t *)(rp_base + ofs);

            // [crispy] single-patched columns are normally not composited,
            // but we do composite all columns; single-patched use originy=0.
            const int originy = (collump[x] >= 0) ? 0 : patch->originy;

            R_DrawColumnInCache(patchcol,
                                block + colofs[x],
                                originy,
                                height,
                                marks + (size_t)x * (size_t)height);
        }
    }

    // 2) Convert composited columns into true posts (Medusa-safe) and
    //    copy an opaque linear copy into block2 for fast sampling.
    byte *const source = (byte *)I_Realloc(NULL, (size_t)height); // temporary column

    for (int i = 0; i < width; ++i)
    {
        column_t *col = (column_t *)(block + colofs[i] - 3); // cached column header
        const byte *restrict mark = marks + (size_t)i * (size_t)height;

        // Save the fully composited payload for shuffling
        memcpy(source, (const byte *)col + 3, (size_t)height);

        // [crispy] copy composited columns to opaque texture
        memcpy(block2 + colofs2[i], source, (size_t)height);

        // Reconstruct posts by scanning transparency marks
        int j = 0;
        int abstop, reltop = 0;
        boolean relative = false;

        for (;;)
        {
            // Skip transparent cells; reltop caps relative topdelta at < 254
            while (j < height && reltop < 254 && !mark[j])
            {
                ++j;
                ++reltop;
            }

            if (j >= height)
            {
                col->topdelta = -1; // end-of-column marker (0xFF)
                break;
            }

            // First 0..253 pixels: absolute topdelta; after that: relative
            col->topdelta = relative ? reltop : j;
            abstop = j;

            // Once we pass 254 boundary, switch to relative topdelta
            if (abstop >= 254)
            {
                relative = true;
                reltop = 0;
            }

            // Count opaque run (bounded by height and reltop < 254)
            unsigned len = 0;
            while (j < height && reltop < 254 && mark[j])
            {
                ++j;
                ++reltop;
                ++len;
            }

            col->length = len; // (Killough 12/98: 32-bit len in builder, truncates to byte in struct)
            memcpy((byte *)col + 3, source + abstop, len);

            // Next post
            col = (column_t *)((byte *)col + len + 4);
        }
    }

    free(source);
    free(marks);

    // Purgable from zone memory now that caches are built
    Z_ChangeTag(block,  PU_CACHE);
    Z_ChangeTag(block2, PU_CACHE);
}

// -----------------------------------------------------------------------------
// R_GenerateLookup
//  [PN] Rewritten for fewer passes and better aliasing hints.
//  Keeps Medusa bug protections and Crispy semantics intact.
// -----------------------------------------------------------------------------

static void R_GenerateLookup (int texnum)
{
    texture_t *const texture = textures[texnum];
    const int width  = texture->width;
    const int height = texture->height;

    // Composited buffers not yet built.
    texturecomposite [texnum] = 0;
    texturecomposite2[texnum] = 0;
    texturecompositesize[texnum] = 0;

    short    *restrict collump  = texturecolumnlump[texnum];
    unsigned *restrict colofs   = texturecolumnofs [texnum];
    unsigned *restrict colofs2  = texturecolumnofs2[texnum];

    // Use byte for patchcount (as before) and 16-bit for postcount to avoid overflow.
    unsigned char  *restrict patchcount = (unsigned char *) Z_Malloc((size_t)width, PU_STATIC, NULL);
    unsigned short *restrict postcount  = (unsigned short*) Z_Malloc((size_t)width * sizeof(unsigned short), PU_STATIC, NULL);

    memset(patchcount, 0, (size_t)width);
    memset(postcount,  0, (size_t)width * sizeof(unsigned short));

    int err = 0;

    // Single pass over all patches:
    // - accumulate patchcount[x]
    // - prime collump[x] / colofs[x] (last writer wins, vanilla behavior)
    // - accumulate total number of posts per column (for Medusa-safe csize)
    for (int i = 0; i < texture->patchcount; ++i)
    {
        const texpatch_t *const p = &texture->patches[i];
        const int pat = p->patch;
        const patch_t *const realpatch = W_CacheLumpNum(pat, PU_CACHE);

        int x1 = p->originx;
        int x2 = x1 + SHORT(realpatch->width);

        if (x1 < 0)
        {
            x1 = 0;
        }
        if (x2 > width)
        {
            x2 = width;
        }
        // Column-ofs table base; allow direct index by screen x
        const int *const cofs_base = realpatch->columnofs - p->originx;

        // Absolute column size guard (Killough 12/98).
        const unsigned limit = (unsigned)height * 3u + 3u;

        for (int x = x1; x < x2; ++x)
        {
            // Bump number of patches touching this column.
            ++patchcount[x];

            // Keep last lump/offset (vanilla search order).
            collump[x] = (short)pat;
            colofs [x] = (unsigned)LONG(cofs_base[x]) + 3u;

            // [PN] Count posts now (previously a second pass):
            // Still Medusa-safe: walk posts until 0xff, bounded by 'limit'.
            const column_t *col = (const column_t *)((const byte *)realpatch + LONG(cofs_base[x]));
            const byte *const base = (const byte *)col;

            while (col->topdelta != 0xff)
            {
                ++postcount[x];

                // Guard against malformed columns.
                const unsigned delta = (unsigned)((const byte *)col - base);
                if (delta > limit)
                {
                    break;
                }
                col = (const column_t *)((const byte *)col + col->length + 4);
            }
        }
    }

    // Build per-column directory and total composite size.
    // Keep Crispy behavior: we generate composites for ALL columns.
    int csize = 0;

    for (int x = 0; x < width; ++x)
    {
        if (!patchcount[x] && !err++)
        {
            // Non-fatal, consistent with modified Crispy behavior.
            printf("R_GenerateLookup: column without a patch (%.8s)\n", texture->name); // [PN] warn once
        }

        // [PN] Use cached block for multi-patched or patch-less columns.
        // Single-patch columns keep collump[x] != -1 to preserve originY=0 path during composite.
        if (patchcount[x] > 1 || !patchcount[x])
        {
            collump[x] = -1;
        }

        // Column header starts at csize+3 (3 bytes header).
        colofs[x] = (unsigned)csize + 3u;

        // Medusa-safe: 4 bytes per post + trailing stop byte.
        // postcount[x] is total across all contributing patches.
        csize += 4 * (int)postcount[x] + 5;

        // Add room for the column payload (height bytes).
        csize += height;

        // Opaque composite (linear storage by columns * height).
        colofs2[x] = (unsigned)(x * height);
    }

    texturecompositesize[texnum] = csize;

    Z_Free(patchcount);
    Z_Free(postcount);
}

// -----------------------------------------------------------------------------
// R_GetColumn
// [crispy] wrapping column getter function for any non-power-of-two textures
// -----------------------------------------------------------------------------

byte *R_GetColumn (int tex, int col)
{
    const int width = texturewidth[tex];
    const int mask = texturewidthmask[tex];

    if (mask + 1 == width)
    {
        col &= mask;
    }
    else
    {
        while (col < 0)
        {
          col += width;
        }
        col %= width;
    }

    if (!texturecomposite2[tex])
        R_GenerateComposite(tex);

    const int ofs = texturecolumnofs2[tex][col];

    return texturecomposite2[tex] + ofs;
}

// -----------------------------------------------------------------------------
// R_GetColumnMod
// [crispy] wrapping column getter function for composited translucent mid-textures on 2S walls
// -----------------------------------------------------------------------------

byte *R_GetColumnMod (int tex, int col)
{
    int ofs;

    while (col < 0)
        col += texturewidth[tex];

    col %= texturewidth[tex];
    ofs = texturecolumnofs[tex][col];

    if (!texturecomposite[tex])
        R_GenerateComposite (tex);

    return texturecomposite[tex] + ofs;
}

static void GenerateTextureHashTable(void)
{
    texture_t **rover;
    int i;
    int key;

    textures_hashtable 
            = Z_Malloc(sizeof(texture_t *) * numtextures, PU_STATIC, 0);

    memset(textures_hashtable, 0, sizeof(texture_t *) * numtextures);

    // Add all textures to hash table

    for (i=0; i<numtextures; ++i)
    {
        // Store index

        textures[i]->index = i;

        // Vanilla Doom does a linear search of the texures array
        // and stops at the first entry it finds.  If there are two
        // entries with the same name, the first one in the array
        // wins. The new entry must therefore be added at the end
        // of the hash chain, so that earlier entries win.

        key = W_LumpNameHash(textures[i]->name) % numtextures;

        rover = &textures_hashtable[key];

        while (*rover != NULL)
        {
            rover = &(*rover)->next;
        }

        // Hook into hash table

        textures[i]->next = NULL;
        *rover = textures[i];
    }
}

//------------------------------------------------------------------------------
// R_InitTextures
//  Initializes the texture list with the textures from the world map.
//  [crispy] partly rewritten to merge PNAMES and TEXTURE1/2 lumps.
//------------------------------------------------------------------------------

void R_InitTextures (void)
{
    // Working pointers
    texture_t *texture;
    texpatch_t *patch;

    // Scratch
    const int *restrict maptex = NULL;
    const int *restrict directory = NULL;

    char name[9];

    int *restrict patchlookup;

    int nummappatches = 0;
    int offset;
    int maxoff = 0;

    // [crispy] local helper structs
    typedef struct
    {
        int lumpnum;
        const void *names;
        short nummappatches;
        short summappatches;
        const char *name_p;
    } pnameslump_t;

    typedef struct
    {
        int lumpnum;
        const int *maptex;
        int maxoff;
        short numtextures;
        short sumtextures;
        short pnamesoffset;
    } texturelump_t;

    pnameslump_t *restrict pnameslumps = NULL;
    texturelump_t *restrict texturelumps = NULL;
    texturelump_t *texturelump;

    int maxpnameslumps = 1; // PNAMES
    int maxtexturelumps = 2; // TEXTURE1, TEXTURE2

    int numpnameslumps = 0;
    int numtexturelumps = 0;

    // [crispy] allocate memory for the pnameslumps and texturelumps arrays
    pnameslumps  = I_Realloc(pnameslumps,  maxpnameslumps  * sizeof(*pnameslumps));
    texturelumps = I_Realloc(texturelumps, maxtexturelumps * sizeof(*texturelumps));

    // [crispy] make sure the first available TEXTURE1/2 lumps are always processed first
    texturelumps[numtexturelumps++].lumpnum = W_GetNumForName("TEXTURE1");
    {
        const int t2 = W_CheckNumForName("TEXTURE2");
        if (t2 != -1)
        {
            texturelumps[numtexturelumps++].lumpnum = t2;
        }
        else
        {
            texturelumps[numtexturelumps].lumpnum = -1;
        }
    }

    // [crispy] fill the arrays with all available PNAMES lumps
    // and the remaining available TEXTURE1/2 lumps
    for (int i = numlumps - 1; i >= 0; --i)
    {
        if (!strncasecmp(lumpinfo[i]->name, "PNAMES", 6))
        {
            if (numpnameslumps == maxpnameslumps)
            {
                maxpnameslumps++;
                pnameslumps = I_Realloc(pnameslumps, maxpnameslumps * sizeof(*pnameslumps));
            }

            pnameslumps[numpnameslumps].lumpnum = i;
            pnameslumps[numpnameslumps].names = W_CacheLumpNum(pnameslumps[numpnameslumps].lumpnum, PU_STATIC);
            pnameslumps[numpnameslumps].nummappatches = LONG(*((const int *) pnameslumps[numpnameslumps].names));

            // [crispy] accumulated number of patches in the lookup tables
            // excluding the current one
            pnameslumps[numpnameslumps].summappatches = nummappatches;
            pnameslumps[numpnameslumps].name_p = (const char *)pnameslumps[numpnameslumps].names + 4;

            // [crispy] calculate total number of patches
            nummappatches += pnameslumps[numpnameslumps].nummappatches;
            numpnameslumps++;
        }
        else if (!strncasecmp(lumpinfo[i]->name, "TEXTURE", 7))
        {
            // [crispy] support only TEXTURE1/2 lumps, not TEXTURE3 etc.
            if (lumpinfo[i]->name[7] != '1' && lumpinfo[i]->name[7] != '2')
            {
                continue;
            }

            // [crispy] make sure the first available TEXTURE1/2 lumps are not processed again
            if (i == texturelumps[0].lumpnum
            ||  i == texturelumps[1].lumpnum) // [crispy] may still be -1
            {
                continue;
            }

            if (numtexturelumps == maxtexturelumps)
            {
                maxtexturelumps++;
                texturelumps = I_Realloc(texturelumps, maxtexturelumps * sizeof(*texturelumps));
            }

            // [crispy] do not proceed any further, yet
            // we first need a complete pnameslumps[] array and need
            // to process texturelumps[0] (and also texturelumps[1]) as well
            texturelumps[numtexturelumps].lumpnum = i;
            numtexturelumps++;
        }
    }

    // [crispy] fill up the patch lookup table
    name[8] = '\0';
    patchlookup = Z_Malloc((size_t)nummappatches * sizeof(*patchlookup), PU_STATIC, NULL);

    for (int i = 0, k = 0; i < numpnameslumps; ++i)
    {
        for (int j = 0; j < pnameslumps[i].nummappatches; ++j)
        {
            int p;

            M_StringCopy(name, pnameslumps[i].name_p + j * 8, sizeof(name));
            p = W_CheckNumForName(name);
            if (!V_IsPatchLump(p))
            {
                p = -1;
            }

            // [crispy] if the name is unambiguous, use the lump we found
            patchlookup[k++] = p;
        }
    }

    // [crispy] calculate total number of textures
    numtextures = 0;
    for (int i = 0; i < numtexturelumps; ++i)
    {
        texturelumps[i].maptex    = W_CacheLumpNum(texturelumps[i].lumpnum, PU_STATIC);
        texturelumps[i].maxoff    = W_LumpLength(texturelumps[i].lumpnum);
        texturelumps[i].numtextures = LONG(*texturelumps[i].maptex);

        // [crispy] accumulated number of textures in the texture files
        // including the current one
        numtextures += texturelumps[i].numtextures;
        texturelumps[i].sumtextures = numtextures;

        // [crispy] link textures to their own WAD's patch lookup table (if any)
        texturelumps[i].pnamesoffset = 0;
        for (int j = 0; j < numpnameslumps; ++j)
        {
            // [crispy] both are from the same WAD?
            if (lumpinfo[texturelumps[i].lumpnum]->wad_file ==
                lumpinfo[pnameslumps[j].lumpnum]->wad_file)
            {
                texturelumps[i].pnamesoffset = pnameslumps[j].summappatches;
                break;
            }
        }
    }

    // [crispy] release memory allocated for patch lookup tables
    for (int i = 0; i < numpnameslumps; ++i)
    {
        W_ReleaseLumpNum(pnameslumps[i].lumpnum);
    }
    free(pnameslumps);

    // [crispy] pointer to (i.e. actually before) the first texture file
    texturelump = texturelumps - 1; // [crispy] gets immediately increased below

    textures             = Z_Malloc(numtextures * sizeof(*textures),             PU_STATIC, 0);
    texturecolumnlump    = Z_Malloc(numtextures * sizeof(*texturecolumnlump),    PU_STATIC, 0);
    texturecolumnofs     = Z_Malloc(numtextures * sizeof(*texturecolumnofs),     PU_STATIC, 0);
    texturecolumnofs2    = Z_Malloc(numtextures * sizeof(*texturecolumnofs2),    PU_STATIC, 0);
    texturecomposite     = Z_Malloc(numtextures * sizeof(*texturecomposite),     PU_STATIC, 0);
    texturecomposite2    = Z_Malloc(numtextures * sizeof(*texturecomposite2),    PU_STATIC, 0);
    texturecompositesize = Z_Malloc(numtextures * sizeof(*texturecompositesize), PU_STATIC, 0);
    texturewidthmask     = Z_Malloc(numtextures * sizeof(*texturewidthmask),     PU_STATIC, 0);
    texturewidth         = Z_Malloc(numtextures * sizeof(*texturewidth),         PU_STATIC, 0);
    textureheight        = Z_Malloc(numtextures * sizeof(*textureheight),        PU_STATIC, 0);
    texturebrightmap     = Z_Malloc(numtextures * sizeof(*texturebrightmap),     PU_STATIC, 0);

    for (int i = 0; i < numtextures; ++i, ++directory)
    {
        if (!(i & 63))
        {
            printf(".");
        }

        // [crispy] initialize for the first texture file lump,
        // skip through empty texture file lumps which do not contain any texture
        while (texturelump == texturelumps - 1 || i == texturelump->sumtextures)
        {
            // [crispy] start looking in next texture file
            texturelump = texturelump + 1;
            maptex      = texturelump->maptex;
            maxoff      = texturelump->maxoff;
            directory   = maptex + 1;
        }

        offset = LONG(*directory);

        if (offset > maxoff)
        {
            I_Error("R_InitTextures: bad texture directory");
        }

        const maptexture_t *mtexture = (const maptexture_t *)((const byte *)maptex + offset);

        texture = textures[i] = Z_Malloc(sizeof(texture_t) + sizeof(texpatch_t) * (SHORT(mtexture->patchcount) - 1), PU_STATIC, 0);

        texture->width      = SHORT(mtexture->width);
        texture->height     = SHORT(mtexture->height);
        texture->patchcount = SHORT(mtexture->patchcount);

        memcpy(texture->name, mtexture->name, sizeof(texture->name));

        const mappatch_t *mpatch = &mtexture->patches[0];
        patch = &texture->patches[0];

        // [crispy] initialize brightmaps
        texturebrightmap[i] = R_BrightmapForTexName(texture->name);

        for (int j = 0; j < texture->patchcount; ++j, ++mpatch, ++patch)
        {
            short p;
            patch->originx = SHORT(mpatch->originx);
            patch->originy = SHORT(mpatch->originy);

            // [crispy] apply offset for patches not in the first available patch offset table
            p = SHORT(mpatch->patch) + texturelump->pnamesoffset;

            // [crispy] catch out-of-range patches
            if (p < nummappatches)
            {
                patch->patch = patchlookup[p];
            }

            if (patch->patch == -1 || p >= nummappatches)
            {
                char texturename[9];
                texturename[8] = '\0';
                memcpy(texturename, texture->name, 8);

                // [crispy] make non-fatal
                fprintf(stderr, "R_InitTextures: Missing patch in texture %s\n", texturename);
                patch->patch = W_CheckNumForName("WIPCNT"); // [crispy] dummy patch
            }
        }

        texturecolumnlump[i] = Z_Malloc(texture->width * sizeof(**texturecolumnlump), PU_STATIC, 0);
        texturecolumnofs[i]  = Z_Malloc(texture->width * sizeof(**texturecolumnofs),  PU_STATIC, 0);
        texturecolumnofs2[i] = Z_Malloc(texture->width * sizeof(**texturecolumnofs2), PU_STATIC, 0);

        int j = 1;
        while (j * 2 <= texture->width)
        {
            j <<= 1;
        }

        texturewidthmask[i] = j - 1;
        textureheight[i]    = texture->height << FRACBITS;

        // [crispy] texture width for wrapping column getter function
        texturewidth[i] = texture->width;
    }

    Z_Free(patchlookup);

    // [crispy] release memory allocated for texture files
    for (int i = 0; i < numtexturelumps; ++i)
    {
        W_ReleaseLumpNum(texturelumps[i].lumpnum);
    }
    free(texturelumps);

    // Precalculate whatever possible.
    for (int i = 0; i < numtextures; ++i)
    {
        R_GenerateLookup(i);
    }

    // Create translation table for global animation.
    texturetranslation = Z_Malloc((numtextures + 1) * sizeof(*texturetranslation), PU_STATIC, 0);

    for (int i = 0; i < numtextures; ++i)
    {
        texturetranslation[i] = i;
    }

    GenerateTextureHashTable();
}

// -----------------------------------------------------------------------------
// R_InitFlats
// -----------------------------------------------------------------------------

void R_InitFlats (void)
{
    firstflat = W_GetNumForName ("F_START") + 1;
    lastflat = W_GetNumForName ("F_END") - 1;
    numflats = lastflat - firstflat + 1;

    // Create translation table for global animation.
    flattranslation = Z_Malloc ((numflats+1)*sizeof(*flattranslation), PU_STATIC, 0);

    for (int i = 0; i < numflats; i++)
    {
        flattranslation[i] = i;
    }

    // [PN] Generate hash table for flats.
    W_HashNumForNameFromTo (firstflat, lastflat, numflats);
}

// -----------------------------------------------------------------------------
// R_InitSpriteLumps
//  Finds the width and hoffset of all sprites in the wad,
//  so the sprite does not need to be cached completely
//  just for having the header info ready during rendering.
// -----------------------------------------------------------------------------

void R_InitSpriteLumps (void)
{
    firstspritelump = W_GetNumForName ("S_START") + 1;
    lastspritelump = W_GetNumForName ("S_END") - 1;

    numspritelumps = lastspritelump - firstspritelump + 1;
    spritewidth = Z_Malloc (numspritelumps*sizeof(*spritewidth), PU_STATIC, 0);
    spriteoffset = Z_Malloc (numspritelumps*sizeof(*spriteoffset), PU_STATIC, 0);
    spritetopoffset = Z_Malloc (numspritelumps*sizeof(*spritetopoffset), PU_STATIC, 0);

    for (int i = 0; i < numspritelumps; i++)
    {
        if (!(i&63))
            printf (".");

        patch_t *patch = W_CacheLumpNum (firstspritelump + i, PU_CACHE);
        spritewidth[i] = SHORT(patch->width) << FRACBITS;
        spriteoffset[i] = SHORT(patch->leftoffset) << FRACBITS;
        spritetopoffset[i] = SHORT(patch->topoffset) << FRACBITS;
    }
}

// -----------------------------------------------------------------------------
// R_InitColormaps
// -----------------------------------------------------------------------------

// [JN] Accurate and resource independent first COLORMAP row,
// the only one needed to generate colormaps in TrueColor mode.
static const byte colormap[256] = {
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
    17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
    32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
    48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
    64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
    80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
    96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
    112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
    128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
    144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
    160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
    176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
    192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
    208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
    224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
    240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255
};

void R_InitColormaps (void)
{
	int i, j = 0;
	byte r, g, b;

	byte *const playpal = W_CacheLumpName("PLAYPAL", PU_STATIC);
	byte *const crypal = W_CacheLumpName("CRYPAL", PU_STATIC);
	byte *const invulpal = W_CacheLumpName("PLAYINVL", PU_STATIC);
	byte *const invulcry = W_CacheLumpName("CRYINVL", PU_STATIC);
	// [JN] Which palette to use for in-game rendering, CRYPAL or PLAYPAL?
	byte *const render_pointer = dp_cry_palette ? crypal : playpal;
	// [JN] Which palette to use for invulnerability palette, CRYINVL or PLAYINVL?
	byte *const invul_pointer = dp_cry_palette ? invulcry : invulpal;

	if (!colormaps)
	{
		colormaps = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
		invulmaps = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
		R_AllocateColoredColormaps();
	}

    // [PN] Precompute gamma'ed base RGB for both palettes (once per index)
    const byte *const restrict gtab = gammatable[vid_gamma];
    const int gamma_black = gtab[0];
    
    byte base_gamma_render[256][3];
    byte base_gamma_invul [256][3];
    
    for (int p = 0; p < 256; ++p)
    {
        // [PN] Render palette (CRYPAL or PLAYPAL)
        base_gamma_render[p][0] = gtab[render_pointer[3 * p + 0]];
        base_gamma_render[p][1] = gtab[render_pointer[3 * p + 1]];
        base_gamma_render[p][2] = gtab[render_pointer[3 * p + 2]];
    
        // [PN] Invulnerability palette (CRYINVL or PLAYINVL)
        base_gamma_invul[p][0]  = gtab[invul_pointer [3 * p + 0]];
        base_gamma_invul[p][1]  = gtab[invul_pointer [3 * p + 1]];
        base_gamma_invul[p][2]  = gtab[invul_pointer [3 * p + 2]];
    }
    
    // [PN] Build colormaps with simple gamma-space fade to black
    for (int c = 0; c < NUMCOLORMAPS; ++c)
    {
        const double scale = (double)c / (double)NUMCOLORMAPS;
        const double k0    = 1.0 - scale;          // weight of base color
        const double kB    = (double)gamma_black * scale; // weight of black (gamma(0))
    
        lighttable_t *const restrict row_col  = &colormaps [c * 256];
        lighttable_t *const restrict row_inv  = &invulmaps [c * 256];
    
        for (int i = 0; i < 256; ++i)
        {
            const byte k = colormap[i]; // mapping index (identity in your table)
    
            // [PN] Normal colormap (render palette)
            const int R = (int)(base_gamma_render[k][0] * k0 + kB);
            const int G = (int)(base_gamma_render[k][1] * k0 + kB);
            const int B = (int)(base_gamma_render[k][2] * k0 + kB);
    
            row_col[i] = 0xff000000 | ((byte)R << 16) | ((byte)G << 8) | (byte)B;
    
            // [PN] Colored colormaps generator still expects a linear index.
            // Use computed absolute index instead of j-counter.
            R_GenerateColoredColormaps(k, (float)scale, c * 256 + i);
    
            // [PN] Invulnerability colormap (invul palette)
            const int Ri = (int)(base_gamma_invul[k][0] * k0 + kB);
            const int Gi = (int)(base_gamma_invul[k][1] * k0 + kB);
            const int Bi = (int)(base_gamma_invul[k][2] * k0 + kB);
    
            row_inv[i] = 0xff000000 | ((byte)Ri << 16) | ((byte)Gi << 8) | (byte)Bi;
        }
    }

	if (!pal_color)
	{
		pal_color = (pixel_t*) Z_Malloc(256 * sizeof(pixel_t), PU_STATIC, 0);
	}

	for (i = 0, j = 0; i < 256; i++)
	{
		r = gammatable[vid_gamma][playpal[3 * i + 0]];
		g = gammatable[vid_gamma][playpal[3 * i + 1]];
		b = gammatable[vid_gamma][playpal[3 * i + 2]];

		pal_color[j++] = 0xff000000 | (r << 16) | (g << 8) | b;
	}

	if (!cry_color)
	{
		cry_color = (pixel_t*) Z_Malloc(256 * sizeof(pixel_t), PU_STATIC, 0);
	}

	for (i = 256, j = 0; i < 512; i++)
	{
		r = gammatable[vid_gamma][crypal[3 * i + 0]];
		g = gammatable[vid_gamma][crypal[3 * i + 1]];
		b = gammatable[vid_gamma][crypal[3 * i + 2]];

		cry_color[j++] = 0xff000000 | (r << 16) | (g << 8) | b;
	}

	// [JN] Which palette to use for patch drawing, CRYPAL or PLAYPAL?
	palette_pointer = dp_cry_palette ? cry_color : pal_color;

	W_ReleaseLumpName("PLAYPAL");
	W_ReleaseLumpName("CRYPAL");
	W_ReleaseLumpName("PLAYINVL");
	W_ReleaseLumpName("CRYINVL");
}


//
// R_InitHSVColors
// [crispy] initialize color translation and color strings tables
//
static void R_InitHSVColors (void)
{
	byte *playpal = W_CacheLumpName("PLAYPAL", PU_STATIC);
	char c[3];
	int i, j;
	boolean keepgray = false;

	if (!crstr)
	    crstr = I_Realloc(NULL, CRMAX * sizeof(*crstr));

	// [crispy] check for status bar graphics replacements
	i = W_CheckNumForName("sttnum0"); // [crispy] Status Bar '0'
    keepgray = W_CheckMultipleLumps("sttnum0") < 2;

	// [crispy] CRMAX - 2: don't override the original GREN and BLUE2 Boom tables
	for (i = 0; i < CRMAX - 2; i++)
	{
	    for (j = 0; j < 256; j++)
	    {
		cr[i][j] = V_Colorize(playpal, i, j, keepgray);
	    }

	    M_snprintf(c, sizeof(c), "%c%c", cr_esc, '0' + i);
	    crstr[i] = M_StringDuplicate(c);
	}

	W_ReleaseLumpName("PLAYPAL");

	i = W_CheckNumForName("CRGREEN");
	if (i >= 0)
	{
	    cr[CR_RED2GREEN] = W_CacheLumpNum(i, PU_STATIC);
	}

	i = W_CheckNumForName("CRBLUE2");
	if (i == -1)
	    i = W_CheckNumForName("CRBLUE");
	if (i >= 0)
	{
	    cr[CR_RED2BLUE] = W_CacheLumpNum(i, PU_STATIC);
	}
}


//
// R_InitData
// Locates all the lumps
//  that will be used by all views
// Must be called after W_Init.
//
void R_InitData (void)
{
    R_InitFlats ();
    printf (".");
    R_InitTextures ();
    printf (".");
    R_InitSpriteLumps ();
    printf (".");
    R_InitColormaps ();
    printf (".");    
    R_InitHSVColors ();
    printf (".");    
    I_InitTCTransMaps ();
    printf (".");
}



//
// R_FlatNumForName
// Retrieval, get a flat number for a flat name.
//
int R_FlatNumForName(const char *name)
{
    int		i;

    i = W_CheckNumForNameFromTo (name, lastflat, firstflat);

    if (i == -1)
    {
	// [crispy] make missing flat non-fatal
	// and fix absurd flat name in error message
	fprintf (stderr, "R_FlatNumForName: %.8s not found\n", name);
	// [crispy] since there is no "No Flat" marker,
	// render missing flats as SKY
	return skyflatnum;
    }
    return i - firstflat;
}




//
// R_CheckTextureNumForName
// Check whether texture is available.
// Filter out NoTexture indicator.
//
int R_CheckTextureNumForName (const char *name)
{
    texture_t *texture;
    int key;

    // "NoTexture" marker.
    if (name[0] == '-')		
	return 0;
		
    key = W_LumpNameHash(name) % numtextures;

    texture=textures_hashtable[key]; 
    
    while (texture != NULL)
    {
	if (!strncasecmp (texture->name, name, 8) )
	    return texture->index;

        texture = texture->next;
    }
    
    return -1;
}



//
// R_TextureNumForName
// Calls R_CheckTextureNumForName,
//  aborts with error message.
//
int R_TextureNumForName(const char *name)
{
    int		i;
	
    i = R_CheckTextureNumForName (name);

    if (i==-1)
    {
	// [crispy] make missing texture non-fatal
	// and fix absurd texture name in error message
	fprintf (stderr, "R_TextureNumForName: %.8s not found\n", name);
	return 0;
    }
    return i;
}

// -----------------------------------------------------------------------------
// R_PrecacheLevel
//  Preloads all relevant graphics for the level.
//
// [PN] Optimized and refactored R_PrecacheLevel:
// - Uses a single zero-initialized buffer sized for the largest resource type.
// - Replaces multiple memset/malloc with one calloc and reuses the buffer across all stages.
// - All loops are forward for better cache efficiency.
// -----------------------------------------------------------------------------

#define MAX3(a,b,c) (((a)>(b))?((a)>(c)?(a):(c)):((b)>(c)?(b):(c)))

void R_PrecacheLevel(void)
{
    const size_t maxsize = MAX3(numtextures, numflats, numsprites);
    byte *restrict hitlist = (byte*)calloc(maxsize, 1);

    // Precache flats
    for (int i = 0; i < numsectors; ++i)
    {
        hitlist[sectors[i].floorpic] = 1;
        hitlist[sectors[i].ceilingpic] = 1;
    }
    for (int i = 0; i < numflats; ++i)
    {
        if (hitlist[i])
        {
            W_CacheLumpNum(firstflat + i, PU_CACHE);
        }
    }

    memset(hitlist, 0, maxsize);

    // Precache textures
    for (int i = 0; i < numsides; ++i)
    {
        hitlist[sides[i].bottomtexture] = 1;
        hitlist[sides[i].toptexture] = 1;
        hitlist[sides[i].midtexture] = 1;
    }

    hitlist[skytexture] = 1;

    for (int i = 0; i < numtextures; ++i)
    {
        if (hitlist[i])
        {
            texture_t * const texture = textures[i];
            for (int j = 0; j < texture->patchcount; ++j)
            {
                W_CacheLumpNum(texture->patches[j].patch, PU_CACHE);
            }
        }
    }

    memset(hitlist, 0, maxsize);

    // Precache sprites
    for (thinker_t *th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function.acp1 == (actionf_p1)P_MobjThinker)
        {
            hitlist[((const mobj_t*)th)->sprite] = 1;
        }
    }

    for (int i = 0; i < numsprites; ++i)
    {
        if (hitlist[i])
        {
            for (int j = 0; j < sprites[i].numframes; ++j)
            {
                const short *sflump = sprites[i].spriteframes[j].lump;
                for (int k = 0; k < 8; ++k)
                {
                    W_CacheLumpNum(firstspritelump + sflump[k], PU_CACHE);
                }
            }
        }
    }

    free(hitlist);
}
