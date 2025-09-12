//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2025 Julia Nechaevskaya
// Copyright(C) 2025 Polina "Aura" N.
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

#include <stdio.h>
#include <string.h>
#include <math.h>

#define MINIZ_NO_STDIO
#define MINIZ_NO_ZLIB_APIS
#include "miniz.h"

#include "doomtype.h"
#include "i_input.h"
#include "i_swap.h"
#include "i_system.h"
#include "i_video.h"
#include "m_bbox.h"
#include "m_config.h"
#include "m_misc.h"
#include "v_video.h"
#include "z_zone.h"

#include "id_vars.h"


// TODO: There are separate RANGECHECK defines for different games, but this
// is common code. Fix this.
#define RANGECHECK


// [JN] Color translation.
byte *dp_translation = NULL;
boolean dp_translucent = false;

// [JN] pointer to cry_color / pal_color
pixel_t *palette_pointer;

// The screen buffer that the v_video.c code draws to.
static pixel_t *dest_screen = NULL;

// [crispy] resolution-agnostic patch drawing
static fixed_t dx, dxi, dy, dyi;


// -----------------------------------------------------------------------------
// V_MarkRect
// -----------------------------------------------------------------------------

void V_MarkRect(int x, int y, int width, int height) 
{
    static int dirtybox[4];

    // If we are temporarily using an alternate screen, do not
    // affect the update box.

    if (dest_screen == I_VideoBuffer)
    {
        M_AddToBox (dirtybox, x, y);
        M_AddToBox (dirtybox, x + width-1, y + height-1);
    }
}

// -----------------------------------------------------------------------------
// V_CopyRect
// -----------------------------------------------------------------------------

void V_CopyRect(int srcx, int srcy, pixel_t *source,
                int width, int height,
                int destx, int desty)
{
    // ---- Hot globals cached locally ----
    const int     sw   = SCREENWIDTH;
    const int     sh   = SCREENHEIGHT;
    pixel_t      *dst0 = dest_screen;

#ifdef RANGECHECK
    if (srcx  < 0 || srcx + width  > sw ||
        srcy  < 0 || srcy + height > sh ||
        destx < 0 || destx         > sw ||
        desty < 0 || desty         > sh)
    {
        // keep original semantics: early return instead of I_Error
        return;
    }
#endif

    // Prevent framebuffer overflow (crispy semantics)
    if (destx + width  > sw) width  = sw - destx;
    if (desty + height > sh) height = sh - desty;

    if (width <= 0 || height <= 0)
        return;

    V_MarkRect(destx, desty, width, height);

    pixel_t *src_row  = source + srcy * sw + srcx;
    pixel_t *dest_row = dst0   + desty * sw + destx;

    const size_t row_bytes = (size_t)width * sizeof(*dest_row);

    // Fast path: full-width blit starting at x==0 → one big transfer
    if (srcx == 0 && destx == 0 && width == sw)
    {
        pixel_t *srcp = source + srcy * sw;
        pixel_t *dstp = dst0   + desty * sw;

        if (dstp == srcp)
            return; // exact same region

        if (source == dst0)
        {
            // Same buffer: memmove handles overlap (top/bottom)
            memmove(dstp, srcp, (size_t)height * (size_t)sw * sizeof(*dstp));
        }
        else
        {
            memcpy(dstp, srcp, (size_t)height * (size_t)sw * sizeof(*dstp));
        }
        return;
    }

    // General path: copy row-by-row.
    // If copying within the same screen buffer and dest is below src, go bottom-up
    // to avoid clobbering yet-to-be-copied rows. Use memmove per row to be safe
    // for horizontal overlap in the same buffer.
    const int same_buffer = (source == dst0);

    if (same_buffer && desty > srcy)
    {
        // Bottom-up
        src_row  += (height - 1) * sw;
        dest_row += (height - 1) * sw;

        for (int r = 0; r < height; ++r)
        {
            memmove(dest_row, src_row, row_bytes);
            src_row  -= sw;
            dest_row -= sw;
        }
    }
    else
    {
        // Top-down
        for (int r = 0; r < height; ++r)
        {
            if (same_buffer)
                memmove(dest_row, src_row, row_bytes);
            else
                memcpy (dest_row, src_row, row_bytes);

            src_row  += sw;
            dest_row += sw;
        }
    }
} 

// -----------------------------------------------------------------------------
// V_DrawPatch
// Masks a column based masked pic to the screen.
// -----------------------------------------------------------------------------

void V_DrawPatch(int x, int y, patch_t *patch)
{
    int count;
    int col;
    column_t *column;
    pixel_t *desttop;
    pixel_t *dest;
    byte *source;
    int w;

    // ---- Hot globals cached locally (helps register allocation) ----
    const int      sw       = SCREENWIDTH;
    const int      sh       = SCREENHEIGHT;
    const int      ws_delta = WIDESCREENDELTA;
    pixel_t *restrict dst_screen = dest_screen;
    const pixel_t *restrict pal  = palette_pointer;

    const fixed_t ldx  = dx;
    const fixed_t ldy  = dy;
    const fixed_t ldxi = dxi;
    const fixed_t ldyi = dyi;

    // Fast-path flags/pointers (constant for the whole call).
    const boolean use_trans = dp_translucent;
    const byte *restrict xlat = dp_translation;

    // Position patch (Crispy-Doom style offsets + widescreen).
    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);
    x += ws_delta; // horizontal widescreen offset

    // Mark dirty rectangle (original semantics).
    V_MarkRect(x, y, SHORT(patch->width), SHORT(patch->height));

    // Left clipping in fixed-point column space.
    col = 0;
    if (x < 0)
    {
        col += ldxi * ((-x * ldx) >> FRACBITS);
        x = 0;
    }

    // Compute top pointer of the first column on the destination buffer.
    desttop = dst_screen
            + ((y * ldy) >> FRACBITS) * sw
            + ((x * ldx) >> FRACBITS);

    w = SHORT(patch->width);

    // Convert x to screen-space pixels once; inner loop just ++x.
    x = (x * ldx) >> FRACBITS;

    // Iterate columns in fixed-point (scaled drawing).
    for (; col < (w << FRACBITS); x++, col += ldxi, desttop++)
    {
        int topdelta = -1;

        // Right clipping: nothing more to draw on this scanline.
        if (x >= sw)
            break;

        // Column pointer from patch data.
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // Step through the posts in a column.
        while (column->topdelta != 0xff)
        {
            int top;
            int srccol = 0;

            // DeePsea tall-patch support (accumulating topdelta).
            if (column->topdelta <= topdelta)
                topdelta += column->topdelta;
            else
                topdelta = column->topdelta;

            // Compute starting y and pointers for this post.
            top    = ((y + topdelta) * ldy) >> FRACBITS;
            source = (byte *)column + 3;
            dest   = desttop + ((topdelta * ldy) >> FRACBITS) * sw;
            count  = (column->length * ldy) >> FRACBITS;

            // Bottom clip against screen height.
            if (top + count > sh)
                count = sh - top;

            // If entirely below the screen, stop scanning this column early.
            if (count < 1)
                break;

            // Top clip once (remove per-pixel "if (top++ >= 0)").
            if (top < 0)
            {
                const int skip = -top;
                if (skip >= count)
                {
                    // Whole post is above the screen; advance to next post.
                    column = (column_t *)((byte *)column + column->length + 4);
                    continue;
                }
                dest   += skip * sw;
                srccol += ldyi * skip;
                count  -= skip;
                // top is effectively 0 now
            }

            // Hot loop: four specialized paths (opaque/translated/translucent).
            if (!use_trans)            // Opaque
            {
                if (!xlat)             // Opaque, no translation
                {
                    while (count--)
                    {
                        *dest = pal[source[srccol >> FRACBITS]];
                        srccol += ldyi;
                        dest   += sw;
                    }
                }
                else                    // Opaque, translated
                {
                    while (count--)
                    {
                        *dest = pal[xlat[source[srccol >> FRACBITS]]];
                        srccol += ldyi;
                        dest   += sw;
                    }
                }
            }
            else                        // Translucent (50% over dest)
            {
                if (!xlat)              // Translucent, no translation
                {
                    while (count--)
                    {
                        const byte s = source[srccol >> FRACBITS];
                        const pixel_t fg = pal[s];
                        *dest = I_BlendOver128_32(*dest, fg);
                        srccol += ldyi;
                        dest   += sw;
                    }
                }
                else                    // Translucent, translated
                {
                    while (count--)
                    {
                        const byte s = xlat[source[srccol >> FRACBITS]];
                        const pixel_t fg = pal[s];
                        *dest = I_BlendOver128_32(*dest, fg);
                        srccol += ldyi;
                        dest   += sw;
                    }
                }
            }

            // Next post in this column.
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

// -----------------------------------------------------------------------------
// V_DrawShadowedOptional
//  Draws patch with shadow if "Text casts shadows" feature is enabled.
//   dest  - main patch, drawed second on top of shadow.
//   dest2 - shadow, drawed first below main patch.
// -----------------------------------------------------------------------------

void V_DrawShadowedPatchOptional(int x, int y, patch_t *patch)
{
    int count, count2, col;
    column_t *column;
    pixel_t *desttop, *dest;
    byte *source;
    pixel_t *dest2;
    int w;

    // ---- Hot globals cached locally (helps register allocation) ----
    const int      sw       = SCREENWIDTH;
    const int      sh       = SCREENHEIGHT;
    const int      ws_delta = WIDESCREENDELTA;
    pixel_t *restrict dst_screen = dest_screen;
    const pixel_t *restrict pal  = palette_pointer;
    const int      vres    = vid_resolution;
    const boolean  do_shadow = msg_text_shadows;

    const fixed_t ldx  = dx;
    const fixed_t ldy  = dy;
    const fixed_t ldxi = dxi;
    const fixed_t ldyi = dyi;

    // Patch flags/pointers (constant for the whole call).
    const boolean use_trans = dp_translucent;
    const byte *restrict xlat = dp_translation;

    // Shadow placement helper (keep original semantics).
    const int shadow_shift = (sw + 1) * vres;

    // Position patch (Crispy-style offsets + widescreen).
    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);
    x += ws_delta; // horizontal widescreen offset

    // Mark dirty rectangle (original semantics).
    V_MarkRect(x, y, SHORT(patch->width), SHORT(patch->height));

    // Left clipping in fixed-point column space.
    col = 0;
    if (x < 0)
    {
        col += ldxi * ((-x * ldx) >> FRACBITS);
        x = 0;
    }

    // Compute top pointer of the first column on the destination buffer.
    desttop = dst_screen + ((y * ldy) >> FRACBITS) * sw + ((x * ldx) >> FRACBITS);

    w = SHORT(patch->width);

    // Convert x to screen-space pixels once; inner loop just ++x.
    x = (x * ldx) >> FRACBITS;

    // Shadow bottom limit (accounts for vertical shift by vres).
    const int sh_limit_shadow = sh - (1 * vres);

    // Iterate columns in fixed-point (scaled drawing).
    for ( ; col < (w << FRACBITS) ; x++, col += ldxi, desttop++)
    {
        int topdelta = -1;

        // Right clipping: nothing more to draw on this scanline.
        if (x >= sw)
            break;

        // Column pointer from patch data.
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // Step through the posts in a column.
        while (column->topdelta != 0xff)
        {
            int top;
            int srccol = 0;

            // DeePsea tall-patch support (accumulating topdelta).
            if (column->topdelta <= topdelta)
                topdelta += column->topdelta;
            else
                topdelta = column->topdelta;

            // Starting y and pointers for this post.
            top    = ((y + topdelta) * ldy) >> FRACBITS;
            source = (byte *)column + 3;

            dest  = desttop + ((topdelta * ldy) >> FRACBITS) * sw;
            dest2 = dest + shadow_shift;

            count  = (column->length * ldy) >> FRACBITS;
            count2 = count;

            // Bottom clip: shadow vs main patch have different limits.
            if (top + count2 > sh_limit_shadow)
                count2 = sh_limit_shadow - top;
            if (top + count > sh)
                count = sh - top;

            // Nothing left for this post? Skip to the next post.
            if (count < 1 || count2 < 1)
            {
                column = (column_t *)((byte *)column + column->length + 4);
                continue;
            }

            // Top clip once (removes per-pixel 'if (top++>=0)' branches).
            if (top < 0)
            {
                const int skip = -top;

                if (skip >= count || skip >= count2)
                {
                    // Whole post is above the screen; advance to next post.
                    column = (column_t *)((byte *)column + column->length + 4);
                    continue;
                }

                dest   += skip * sw;
                dest2  += skip * sw;
                srccol += ldyi * skip;
                count  -= skip;
                count2 -= skip;
                // top is effectively 0 now
            }

            // --- Draw shadow first (optional), no per-pixel top checks ---
            if (do_shadow)
            {
                // Note: previously used drawshadow_*; now inlined dark blend.
                // Source index was not used for darkening; we blend a constant alpha.
                int n = count2;
                while (n--) { *dest2 = I_BlendDark_32(*dest2, 128); dest2 += sw; }
            }

            // --- Draw the actual patch (four specialized fast paths) ---
            if (!use_trans)            // Opaque
            {
                if (!xlat)             // Opaque, no translation
                {
                    int n = count;
                    while (n--)
                    {
                        *dest = pal[source[srccol >> FRACBITS]];
                        srccol += ldyi;
                        dest   += sw;
                    }
                }
                else                    // Opaque, translated
                {
                    int n = count;
                    while (n--)
                    {
                        *dest = pal[xlat[source[srccol >> FRACBITS]]];
                        srccol += ldyi;
                        dest   += sw;
                    }
                }
            }
            else                        // Translucent over destination
            {
                if (!xlat)              // Translucent, no translation
                {
                    int n = count;
                    while (n--)
                    {
                        const byte s = source[srccol >> FRACBITS];
                        const pixel_t fg = pal[s];
                        *dest = I_BlendOver128_32(*dest, fg);
                        srccol += ldyi;
                        dest   += sw;
                    }
                }
                else                    // Translucent, translated
                {
                    int n = count;
                    while (n--)
                    {
                        const byte s = xlat[source[srccol >> FRACBITS]];
                        const pixel_t fg = pal[s];
                        *dest = I_BlendOver128_32(*dest, fg);
                        srccol += ldyi;
                        dest   += sw;
                    }
                }
            }

            // Next post in this column.
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

// -----------------------------------------------------------------------------
// V_DrawPatchFullScreen
// -----------------------------------------------------------------------------

void V_DrawPatchFullScreen(patch_t *patch, boolean flipped)
{
    const int x = DivRoundClosest((ORIGWIDTH - SHORT(patch->width)), 2);
    static int black = -1;

    patch->leftoffset = 0;
    patch->topoffset = 0;

    if (black == -1)
    {
        black = I_MapRGB(0x00, 0x00, 0x00);
    }

    // [crispy] fill pillarboxes in widescreen mode
    if (SCREENWIDTH != NONWIDEWIDTH)
    {
        V_DrawFilledBox(0, 0, SCREENWIDTH, SCREENHEIGHT, black);
    }

    if (flipped)
    {
        V_DrawPatchFlipped(x, 0, patch);
    }
    else
    {
        V_DrawPatch(x, 0, patch);
    }
}

// -----------------------------------------------------------------------------
// V_DrawPatchFlipped
//  Masks a column based masked pic to the screen.
//  Flips horizontally, e.g. to mirror face.
// -----------------------------------------------------------------------------

void V_DrawPatchFlipped(int x, int y, patch_t *patch)
{
    int count;
    int col;
    column_t *column;
    pixel_t *desttop;
    pixel_t *dest;
    byte *source;
    int w;

    // ---- Hot globals cached locally (helps register allocation) ----
    const int      sw       = SCREENWIDTH;
    const int      sh       = SCREENHEIGHT;
    const int      ws_delta = WIDESCREENDELTA;
    pixel_t *restrict dst_screen = dest_screen;
    const pixel_t *restrict pal  = palette_pointer;

    const fixed_t ldx  = dx;
    const fixed_t ldy  = dy;
    const fixed_t ldxi = dxi;
    const fixed_t ldyi = dyi;

    // Position patch (Crispy-style offsets + widescreen).
    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);
    x += ws_delta; // horizontal widescreen offset

    // Mark dirty rectangle (original semantics).
    V_MarkRect(x, y, SHORT(patch->width), SHORT(patch->height));

    // Left clipping in fixed-point column space.
    col = 0;
    if (x < 0)
    {
        col += ldxi * ((-x * ldx) >> FRACBITS);
        x = 0;
    }

    // Compute top pointer of the first column on the destination buffer.
    desttop = dst_screen
            + ((y * ldy) >> FRACBITS) * sw
            + ((x * ldx) >> FRACBITS);

    w = SHORT(patch->width);

    // Convert x to screen-space pixels once; inner loop just ++x.
    x = (x * ldx) >> FRACBITS;

    // Iterate columns in fixed-point (scaled drawing), mirrored horizontally.
    for ( ; col < (w << FRACBITS) ; x++, col += ldxi, desttop++)
    {
        int topdelta = -1;

        // Right clipping: nothing more to draw on this scanline.
        if (x >= sw)
            break;

        // Column pointer from patch data, mirrored: w-1-(col>>FRACBITS)
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[w - 1 - (col >> FRACBITS)]));

        // Step through the posts in a column.
        while (column->topdelta != 0xff)
        {
            int top;
            int srccol = 0;

            // DeePsea tall-patch support (accumulating topdelta).
            if (column->topdelta <= topdelta)
                topdelta += column->topdelta;
            else
                topdelta = column->topdelta;

            // Compute starting y and pointers for this post.
            top    = ((y + topdelta) * ldy) >> FRACBITS;
            source = (byte *)column + 3;
            dest   = desttop + ((topdelta * ldy) >> FRACBITS) * sw;
            count  = (column->length * ldy) >> FRACBITS;

            // Bottom clip against screen height.
            if (top + count > sh)
                count = sh - top;

            // If entirely below the screen, stop scanning this column early.
            if (count < 1)
                break;

            // Top clip once (remove per-pixel "if (top++ >= 0)").
            if (top < 0)
            {
                const int skip = -top;
                if (skip >= count)
                {
                    // Whole post is above the screen; advance to next post.
                    column = (column_t *)((byte *)column + column->length + 4);
                    continue;
                }
                dest   += skip * sw;
                srccol += ldyi * skip;
                count  -= skip;
                // top is effectively 0 now
            }

            // Hot loop: opaque draw, no translation/translucency by design.
            {
                int n = count;
                while (n--)
                {
                    *dest = pal[source[srccol >> FRACBITS]];
                    srccol += ldyi;
                    dest   += sw;
                }
            }

            // Next post in this column.
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }

}

// -----------------------------------------------------------------------------
// [JN] V_DrawPatchFinale
// Draws pixel-doubled sprite. Used exclusively on casting sequence screen.
// Written with extensive support of Fabian Greffrath, thanks! (2019-01-16)
// Adapted for quad resolution by Roman Fomin, thanks as well! (2023-01-10)
// And finally, resolution independent implemenation! :-)      (2024-03-11)
// -----------------------------------------------------------------------------

void V_DrawPatchFinale (int x, int y, patch_t *patch)
{
	int       count, col, w, f;
	column_t *column;
	pixel_t  *desttop;
	pixel_t  *dest;
	byte     *source;

	// [JN] Resolution independent multipler:
	const int m = vid_resolution * 2;
	
	y -= SHORT(patch->topoffset);
	x -= SHORT(patch->leftoffset);
	x += (WIDESCREENDELTA/2);

	V_MarkRect(x, y, SHORT(patch->width), SHORT(patch->height));

	col = 0;
	desttop = dest_screen  + (y * m) * SCREENWIDTH + x;
	w = SHORT(patch->width);

	for ( ; col<w ; x++, col++, desttop++)
	{
		column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			for (f = 0; f < m; f++)
			{
				source = (byte *)column + 3;

				dest = desttop + column->topdelta * (SCREENWIDTH * m)
					 + (x * (m - 1)) + f;

				count = column->length;

				while (count--)
				{
					for (int g = 0; g < m; g++)
					{
						*dest = palette_pointer[*source];
						dest += SCREENWIDTH;
					}
					source++;
				}
			}
			column = (column_t *)((byte *)column + column->length + 4);
		}
	}
}

// -----------------------------------------------------------------------------
// V_DrawFadePatch
//  Draws translucent patch with given alpha value. For TrueColor only.
// -----------------------------------------------------------------------------

void V_DrawFadePatch (int x, int y, const patch_t *restrict patch, int alpha)
{
    // ---- Hot globals cached locally ----
    const int      sw        = SCREENWIDTH;
    const int      sh        = SCREENHEIGHT;
    const int      vres      = vid_resolution;
    const int      ws_delta  = WIDESCREENDELTA;
    pixel_t *restrict dst    = dest_screen;
    const pixel_t *restrict pal = palette_pointer;
    const fixed_t  ldx  = dx;
    const fixed_t  ldy  = dy;
    const fixed_t  ldxi = dxi;
    const fixed_t  ldyi = dyi;

    // Quick rejects / normalization for alpha
    if (alpha <= 0)
        return;
    if (alpha > 255)
        alpha = 255;

    // Position (Crispy offsets + widescreen)
    x += ws_delta - SHORT(patch->leftoffset);
    y -= SHORT(patch->topoffset);

    const int pw = SHORT(patch->width);
    const int ph = SHORT(patch->height);
    const int vw = sw / vres;
    const int vh = sh / vres;

    // Strict in-bounds guard (as in original)
    if (x < 0 || x + pw > vw || y < 0 || y + ph > vh)
        return;

    // Precompute column start on destination
    pixel_t *restrict desttop =
        dst + ((y * ldy) >> FRACBITS) * sw + ((x * ldx) >> FRACBITS);

    // Build a 256-entry LUT of truecolor pixels once per call, accounting for translation.
    // This removes per-pixel translation branches and extra indirections.
    pixel_t src_lut[256];
    if (dp_translation)
    {
        const byte *restrict xlat = dp_translation;
        for (int i = 0; i < 256; ++i)
            src_lut[i] = pal[xlat[i]];
    }
    else
    {
        for (int i = 0; i < 256; ++i)
            src_lut[i] = pal[i];
    }

    // Fast path if alpha == 255: plain opaque copy of translated pal color.
    if (alpha == 255)
    {
        for (int col = 0; col < (pw << FRACBITS); col += ldxi, desttop++)
        {
            const column_t *restrict column =
                (const column_t *)((const byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

            while (column->topdelta != 0xff)
            {
                const int count = (column->length * ldy) >> FRACBITS;
                pixel_t *restrict dest =
                    desttop + ((column->topdelta * ldy) >> FRACBITS) * sw;
                const byte *restrict source = (const byte *)column + 3;

                // No need for top/bottom clipping here because of strict in-bounds guard above.
                int n = count, srccol = 0;
                while (n--)
                {
                    const byte s = source[srccol >> FRACBITS];
                    *dest = src_lut[s];
                    srccol += ldyi;
                    dest   += sw;
                }

                column = (const column_t *)((const byte *)column + column->length + 4);
            }
        }
        return;
    }

    // General path: alpha-blend over destination (TrueColor).
    for (int col = 0; col < (pw << FRACBITS); col += ldxi, desttop++)
    {
        const column_t *restrict column =
            (const column_t *)((const byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        while (column->topdelta != 0xff)
        {
            const int count = (column->length * ldy) >> FRACBITS;
            pixel_t *restrict dest =
                desttop + ((column->topdelta * ldy) >> FRACBITS) * sw;
            const byte *restrict source = (const byte *)column + 3;

            int n = count, srccol = 0;
            while (n--)
            {
                const byte s = source[srccol >> FRACBITS];
                *dest = I_BlendOver_32(*dest, src_lut[s], alpha);
                srccol += ldyi;
                dest   += sw;
            }

            column = (const column_t *)((const byte *)column + column->length + 4);
        }
    }
}

// -----------------------------------------------------------------------------
// V_DrawBlock
//  Draw a linear block of pixels into the view buffer.
// -----------------------------------------------------------------------------

void V_DrawBlock(int x, int y, int width, int height, pixel_t *src)
{ 
    // ---- Hot globals cached locally (helps register allocation) ----
    const int      sw   = SCREENWIDTH;
    const int      sh   = SCREENHEIGHT;
    const int      vres = vid_resolution;
    pixel_t *restrict dst_screen_local = dest_screen;

#ifdef RANGECHECK
    if (x < 0 || x + width > sw || y < 0 || y + height > sh)
    {
        I_Error("Bad V_DrawBlock");
    }
#endif

    // Trivial reject
    if (width <= 0 || height <= 0 || src == NULL)
        return;

    V_MarkRect(x, y, width, height);

    // Compute destination start once. Note: y is scaled by vid_resolution.
    pixel_t *restrict dest = dst_screen_local + (y * vres) * sw + x;

    // Fast path: full-width blit starting at x==0 → one big memcpy
    if (x == 0 && width == sw)
    {
        const size_t bytes = (size_t)height * (size_t)sw * sizeof(*dest);
        memcpy(dest, src, bytes);
        return;
    }

    // General path: row-by-row memcpy with screen stride
    pixel_t *restrict s = src;
    const size_t row_bytes = (size_t)width * sizeof(*dest);

    for (int h = 0; h < height; ++h)
    {
        memcpy(dest, s, row_bytes);
        s    += width;   // advance source by the block width
        dest += sw;      // advance dest by full screen stride
    }
} 

// -----------------------------------------------------------------------------
// V_DrawScaledBlock
//  [crispy] scaled version of V_DrawBlock()
// -----------------------------------------------------------------------------

void V_DrawScaledBlock(int x, int y, int width, int height, byte *src)
{
    // ---- Hot globals cached locally (helps register allocation) ----
    const int      sw       = SCREENWIDTH;
    const int      sh       = SCREENHEIGHT;
    const int      vres     = vid_resolution;
    const int      ws_delta = WIDESCREENDELTA;
    pixel_t *restrict dst   = dest_screen;
    const pixel_t *restrict pal = palette_pointer;

    // Widescreen horizontal offset
    x += ws_delta;

    // Destination rect in screen pixels
    const int rx = x * vres;
    const int ry = y * vres;
    const int rw = width  * vres;
    const int rh = height * vres;

    // Clip to screen
    int dx0 = rx < 0 ? 0 : rx;
    int dy0 = ry < 0 ? 0 : ry;
    int dx1 = rx + rw; if (dx1 > sw) dx1 = sw;
    int dy1 = ry + rh; if (dy1 > sh) dy1 = sh;

    const int dw = dx1 - dx0;
    const int dh = dy1 - dy0;
    if (dw <= 0 || dh <= 0)
        return;

    // Offsets of the clipped block relative to the un-clipped scaled origin
    const int xoff = dx0 - rx;  // 0..vres-1
    const int yoff = dy0 - ry;  // 0..vres-1

    // Starting destination pointer
    pixel_t *restrict dest_row = dst + dy0 * sw + dx0;

    // Vertical mapping without per-row division:
    // src_y starts at floor(yoff / vres), hy = yoff % vres; each row increments hy,
    // and when hy == vres we reset to 0 and increment src_y.
    int src_y = (vres > 1) ? (yoff / vres) : yoff;     // safe for vres==1
    int hy    = (vres > 1) ? (yoff % vres) : 0;

    for (int i = 0; i < dh; ++i)
    {
        const byte *restrict src_row = src + src_y * width;

        // Horizontal mapping without per-pixel division:
        // src_x starts at floor(xoff / vres), hx = xoff % vres; each pixel increments hx,
        // and when hx == vres we reset to 0 and increment src_x.
        int src_x = (vres > 1) ? (xoff / vres) : xoff;
        int hx    = (vres > 1) ? (xoff % vres) : 0;

        pixel_t *restrict d = dest_row;

        // Inner loop: nearest-neighbor scale using accumulators (no divisions)
        for (int j = 0; j < dw; ++j)
        {
            d[j] = pal[src_row[src_x]];

            // advance horizontal accumulator
            if (++hx == vres)
            {
                hx = 0;
                ++src_x;
                // (src_x will never exceed width-1 due to outer clipping)
            }
        }

        // advance destination one screen row
        dest_row += sw;

        // advance vertical accumulator
        if (++hy == vres)
        {
            hy = 0;
            ++src_y;
            // (src_y will never exceed height-1 due to outer clipping)
        }
    }
}

// -----------------------------------------------------------------------------
// V_DrawFilledBox
// -----------------------------------------------------------------------------

void V_DrawFilledBox(int x, int y, int w, int h, int c)
{
    // ---- Hot globals cached locally ----
    const int      sw = SCREENWIDTH;
    const int      sh = SCREENHEIGHT;
    pixel_t *restrict screen = I_VideoBuffer;

#ifdef RANGECHECK
    if (x < 0 || x + w > sw || y < 0 || y + h > sh)
    {
        I_Error("Bad V_DrawFilledBox");
    }
#endif

    if (w <= 0 || h <= 0)
        return;

    V_MarkRect(x, y, w, h);

    pixel_t *restrict row0 = screen + y * sw + x;
    const pixel_t color = (pixel_t)c;

    // Fill the first row (unrolled)
    {
        pixel_t *d = row0;
        int n = w;

        while (n >= 8)
        {
            d[0] = color; d[1] = color; d[2] = color; d[3] = color;
            d[4] = color; d[5] = color; d[6] = color; d[7] = color;
            d += 8; n -= 8;
        }
        for (; n > 0; n--) // Tail loop, safe & warning-free
            *d++ = color;
    }

    if (h == 1)
        return;

    // Copy the filled row to the remaining rows using memcpy
    {
        const size_t row_bytes = (size_t)w * sizeof(*row0);
        pixel_t *restrict dst = row0 + sw;

        for (int y1 = 1; y1 < h; ++y1)
        {
            memcpy(dst, row0, row_bytes);
            dst += sw;
        }
    }
}

void V_DrawHorizLine(int x, int y, int w, int c)
{
    pixel_t *buf;
    int x1;

    // [crispy] prevent framebuffer overflows
    if (x + w > (unsigned)SCREENWIDTH)
	w = SCREENWIDTH - x;

    buf = I_VideoBuffer + SCREENWIDTH * y + x;

    for (x1 = 0; x1 < w; ++x1)
    {
        *buf++ = c;
    }
}

void V_DrawVertLine(int x, int y, int h, int c)
{
    pixel_t *buf;
    int y1;

    buf = I_VideoBuffer + SCREENWIDTH * y + x;

    for (y1 = 0; y1 < h; ++y1)
    {
        *buf = c;
        buf += SCREENWIDTH;
    }
}

void V_DrawBox(int x, int y, int w, int h, int c)
{
    V_DrawHorizLine(x, y, w, c);
    V_DrawHorizLine(x, y+h-1, w, c);
    V_DrawVertLine(x, y, h, c);
    V_DrawVertLine(x+w-1, y, h, c);
}

// -----------------------------------------------------------------------------
// V_FillFlat
//  [crispy] Unified function of flat filling. Used for intermission
//  and finale screens, view border and status bar's wide screen mode.
//  [PN] Avoid per-pixel divisions by using accumulators; cache hot globals.
// -----------------------------------------------------------------------------

void V_FillFlat(int y_start, int y_stop, int x_start, int x_stop,
                const byte *src, pixel_t *dest)
{
    // ---- Hot globals cached locally ----
    const int      vres = vid_resolution;
    const pixel_t *restrict pal = palette_pointer;

    const int dw = x_stop - x_start;
    const int dh = y_stop - y_start;
    if (dw <= 0 || dh <= 0)
        return;

    pixel_t *restrict d = dest;

    // Initial source Y index and accumulator (no per-row division)
    int src_y = (vres > 1) ? ((y_start / vres) & 63) : (y_start & 63);
    int hy    = (vres > 1) ?  (y_start % vres)       : 0;

    for (int i = 0; i < dh; ++i)
    {
        const int row_base = (src_y << 6);            // src_y * 64
        const byte *restrict row = src + row_base;

        // Initial source X index and accumulator for this row
        int src_x = (vres > 1) ? ((x_start / vres) & 63) : (x_start & 63);
        int hx    = (vres > 1) ?  (x_start % vres)       : 0;

        // Inner loop: no divisions; wrap indices with &63
        for (int j = 0; j < dw; ++j)
        {
            *d++ = pal[row[src_x]];

            if (++hx == vres)
            {
                hx = 0;
                src_x = (src_x + 1) & 63;
            }
        }

        if (++hy == vres)
        {
            hy = 0;
            src_y = (src_y + 1) & 63;
        }
    }
}

//
// V_Init
// 

void V_Init (void) 
{ 
    // [crispy] initialize resolution-agnostic patch drawing
    if (NONWIDEWIDTH && SCREENHEIGHT)
    {
        dx = (NONWIDEWIDTH << FRACBITS) / ORIGWIDTH;
        dxi = (ORIGWIDTH << FRACBITS) / NONWIDEWIDTH + 1;  // [JN] +1 for multiple resolutions
        dy = (SCREENHEIGHT << FRACBITS) / ORIGHEIGHT;
        dyi = (ORIGHEIGHT << FRACBITS) / SCREENHEIGHT + 1; // [JN] +1 for multiple resolutions
    }
}

// Set the buffer that the code draws to.

void V_UseBuffer(pixel_t *buffer)
{
    dest_screen = buffer;
}

// Restore screen buffer to the i_video screen buffer.

void V_RestoreBuffer(void)
{
    dest_screen = I_VideoBuffer;
}

//
// SCREEN SHOTS
//


//
// WritePNGfile
//

static void WritePNGfile (const char *filename)
{
    byte *data;
    int width, height;
    size_t png_data_size = 0;

    I_RenderReadPixels(&data, &width, &height);
    {
        void *pPNG_data = tdefl_write_image_to_png_file_in_memory(data, width, height, 4, &png_data_size);

        if (!pPNG_data)
        {
            return;
        }
        else
        {
            FILE *handle = M_fopen(filename, "wb");
            fwrite(pPNG_data, 1, png_data_size, handle);
            fclose(handle);
            mz_free(pPNG_data);
        }
    }

    free(data);
}

//
// V_ScreenShot
//

void V_ScreenShot(const char *format)
{
    int i;
    char lbmname[16]; // haleyjd 20110213: BUG FIX - 12 is too small!
    char *file;
    
    // find a file name to save it to

    for (i=0; i<=9999; i++)
    {
        M_snprintf(lbmname, sizeof(lbmname), format, i, "png");
        // [JN] Construct full path to screenshot file.
        file = M_StringJoin(screenshotdir, lbmname, NULL);

        if (!M_FileExists(file))
        {
            break;      // file doesn't exist
        }
        free(file);
    }

    if (i == 10000)
    {
        I_Error ("V_ScreenShot: Couldn't create a PNG");
    }

    WritePNGfile(file);
	free(file);
}
