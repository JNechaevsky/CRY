//
// Copyright(C) 2025 Polina "Aura" N.
// Copyright(C) 2025 Julia Nechaevskaya
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

#include <stdlib.h>
#include "doomdef.h"
#include "r_local.h"
#include "v_video.h"

#include "id_vars.h"

static const int shadow_alpha = 128; // [JN] TODO - contrast?

// -----------------------------------------------------------------------------
// R_DrawTLColumn
// [PN/JN] Translucent column, overlay blending.
// -----------------------------------------------------------------------------

void R_DrawTLColumn (void)
{
    const int count = dc_yh - dc_yl;
    if (count < 0)
        return; // No pixels to draw

    // Local pointers for improved memory access
    const byte *restrict const sourcebase   = dc_source;
    const byte *restrict const brightmap    = dc_brightmap;
    const pixel_t *restrict const colormap0 = dc_colormap[0];
    const pixel_t *restrict const colormap1 = dc_colormap[1];
    int y_start = dc_yl;
    int y_end = dc_yh;

    // Setup scaling
    fixed_t frac = dc_texturemid + (y_start - centery) * dc_iscale;

    // Precompute initial destination pointer
    pixel_t *restrict dest = ylookup[y_start] + columnofs[flipviewwidth[dc_x]];

    if (vid_resolution > 1) // Duplicate pixels vertically for performance
    {
        const int step = 2;
        const fixed_t fracstep = dc_iscale * step;

        // Compute one pixel, write it to two vertical lines
        while (y_start < y_end)
        {
            const unsigned s = sourcebase[frac >> FRACBITS];
            const pixel_t bg = *dest;
            const pixel_t fg = brightmap[s] ? colormap1[s] : colormap0[s];
            const pixel_t blended = I_BlendOver168_32(bg, fg);

            // Write two pixels (current and next line)
            dest[0] = blended;
            dest[1] = blended;

            // Move to next pair
            dest    += step;
            frac    += fracstep;
            y_start += step;
        }

        // Handle final odd line
        if (y_start == y_end)
        {
            const unsigned s = sourcebase[frac >> FRACBITS];
            const pixel_t bg = *dest;
            const pixel_t fg = brightmap[s] ? colormap1[s] : colormap0[s];
            const pixel_t blended = I_BlendOver168_32(bg, fg);

            dest[0] = blended;
        }
    }
    else // No vertical duplication in 1× mode
    {
        const fixed_t fracstep = dc_iscale;

        while (y_start <= y_end)
        {
            const unsigned s = sourcebase[frac >> FRACBITS];
            const pixel_t bg = *dest;
            const pixel_t fg = brightmap[s] ? colormap1[s] : colormap0[s];
            const pixel_t blended = I_BlendOver168_32(bg, fg);

            *dest = blended;
            dest++;
            frac += fracstep;
            ++y_start;
        }
    }
}

void R_DrawTLColumnLow (void)
{
    const int count = dc_yh - dc_yl;
    if (count < 0)
        return; // No pixels to draw

    // Low detail: double horizontal resolution
    const int x = dc_x << 1;

    // Local pointers for improved memory access
    const byte *restrict const sourcebase   = dc_source;
    const byte *restrict const brightmap    = dc_brightmap;
    const pixel_t *restrict const colormap0 = dc_colormap[0];
    const pixel_t *restrict const colormap1 = dc_colormap[1];
    int y_start = dc_yl;
    int y_end = dc_yh;

    // Setup scaling
    fixed_t frac = dc_texturemid + (y_start - centery) * dc_iscale;

    // Precompute initial destination pointer
    pixel_t *restrict dest1 = ylookup[y_start] + columnofs[flipviewwidth[x]];
    pixel_t *restrict dest2 = ylookup[y_start] + columnofs[flipviewwidth[x + 1]];

    if (vid_resolution > 1) // Duplicate pixels vertically for performance
    {
        const int step = 2;
        const fixed_t fracstep = dc_iscale * step;

        // Compute one pixel, write it to two vertical lines
        while (y_start < y_end)
        {
            const unsigned s  = sourcebase[frac >> FRACBITS];
            const pixel_t bg1 = *dest1;
            const pixel_t bg2 = *dest2;
            const pixel_t fg  = brightmap[s] ? colormap1[s] : colormap0[s];
            const pixel_t blended1 = I_BlendOver168_32(bg1, fg);
            const pixel_t blended2 = I_BlendOver168_32(bg2, fg);
            // Process two lines for both columns
            dest1[0] = blended1;
            dest1[1] = blended1;
            dest2[0] = blended2;
            dest2[1] = blended2;

            // Move to next pair of lines
            dest1 += step;
            dest2 += step;
            frac += fracstep;
            y_start += step;
        }

        // Handle final odd line
        if (y_start == y_end)
        {
            const unsigned s  = sourcebase[frac >> FRACBITS];
            const pixel_t bg1 = *dest1;
            const pixel_t bg2 = *dest2;
            const pixel_t fg  = brightmap[s] ? colormap1[s] : colormap0[s];
            const pixel_t blended1 = I_BlendOver168_32(bg1, fg);
            const pixel_t blended2 = I_BlendOver168_32(bg2, fg);

            dest1[0] = blended1;
            dest2[0] = blended2;
        }
    }
    else // No vertical duplication in 1× mode
    {
        const fixed_t fracstep = dc_iscale;

        while (y_start <= y_end)
        {
            const unsigned s  = sourcebase[frac >> FRACBITS];
            const pixel_t bg1 = *dest1;
            const pixel_t bg2 = *dest2;
            const pixel_t fg  = brightmap[s] ? colormap1[s] : colormap0[s];
            const pixel_t blended1 = I_BlendOver168_32(bg1, fg);
            const pixel_t blended2 = I_BlendOver168_32(bg2, fg);

            *dest1 = blended1;
            *dest2 = blended2;
            dest1++;
            dest2++;
            frac += fracstep;
            ++y_start;
        }
    }
}

// -----------------------------------------------------------------------------
// R_DrawTLAddColumn
// [PN/JN] Translucent column, additive blending.
// -----------------------------------------------------------------------------

void R_DrawTLAddColumn (void)
{
    const int count = dc_yh - dc_yl;
    if (count < 0)
        return; // No pixels to draw

    // Local pointers for improved memory access
    const byte *restrict const sourcebase   = dc_source;
    const byte *restrict const brightmap    = dc_brightmap;
    const pixel_t *restrict const colormap0 = dc_colormap[0];
    const pixel_t *restrict const colormap1 = dc_colormap[1];
    int y_start = dc_yl;
    int y_end = dc_yh;

    // Setup scaling
    fixed_t frac = dc_texturemid + (y_start - centery) * dc_iscale;

    // Precompute initial destination pointer
    pixel_t *restrict dest = ylookup[y_start] + columnofs[flipviewwidth[dc_x]];

    if (vid_resolution > 1) // Duplicate pixels vertically for performance
    {
        const int step = 2;
        const fixed_t fracstep = dc_iscale * step;

        // Compute one pixel, write it to two vertical lines
        while (y_start < y_end)
        {
            const unsigned s = sourcebase[frac >> FRACBITS];
            const pixel_t bg = *dest;
            const pixel_t fg = brightmap[s] ? colormap1[s] : colormap0[s];
            const pixel_t blended = I_BlendAdd_32(bg, fg);

            // Write two pixels (current and next line)
            dest[0] = blended;
            dest[1] = blended;

            // Move to next pair
            dest    += step;
            frac    += fracstep;
            y_start += step;
        }

        // Handle final odd line
        if (y_start == y_end)
        {
            const unsigned s = sourcebase[frac >> FRACBITS];
            const pixel_t bg = *dest;
            const pixel_t fg = brightmap[s] ? colormap1[s] : colormap0[s];
            const pixel_t blended = I_BlendAdd_32(bg, fg);

            dest[0] = blended;
        }
    }
    else // No vertical duplication in 1× mode
    {
        const fixed_t fracstep = dc_iscale;

        while (y_start <= y_end)
        {
            const unsigned s = sourcebase[frac >> FRACBITS];
            const pixel_t bg = *dest;
            const pixel_t fg = brightmap[s] ? colormap1[s] : colormap0[s];
            const pixel_t blended = I_BlendAdd_32(bg, fg);

            *dest = blended;
            dest++;
            frac += fracstep;
            ++y_start;
        }
    }
}

void R_DrawTLAddColumnLow (void)
{
    const int count = dc_yh - dc_yl;
    if (count < 0)
        return; // No pixels to draw

    // Low detail: double horizontal resolution
    const int x = dc_x << 1;

    // Local pointers for improved memory access
    const byte *restrict const sourcebase   = dc_source;
    const byte *restrict const brightmap    = dc_brightmap;
    const pixel_t *restrict const colormap0 = dc_colormap[0];
    const pixel_t *restrict const colormap1 = dc_colormap[1];
    int y_start = dc_yl;
    int y_end = dc_yh;

    // Setup scaling
    fixed_t frac = dc_texturemid + (y_start - centery) * dc_iscale;

    // Precompute initial destination pointer
    pixel_t *restrict dest1 = ylookup[y_start] + columnofs[flipviewwidth[x]];
    pixel_t *restrict dest2 = ylookup[y_start] + columnofs[flipviewwidth[x + 1]];

    if (vid_resolution > 1) // Duplicate pixels vertically for performance
    {
        const int step = 2;
        const fixed_t fracstep = dc_iscale * step;

        // Compute one pixel, write it to two vertical lines
        while (y_start < y_end)
        {
            const unsigned s  = sourcebase[frac >> FRACBITS];
            const pixel_t bg1 = *dest1;
            const pixel_t bg2 = *dest2;
            const pixel_t fg  = brightmap[s] ? colormap1[s] : colormap0[s];
            const pixel_t blended1 = I_BlendAdd_32(bg1, fg);
            const pixel_t blended2 = I_BlendAdd_32(bg2, fg);
            // Process two lines for both columns
            dest1[0] = blended1;
            dest1[1] = blended1;
            dest2[0] = blended2;
            dest2[1] = blended2;

            // Move to next pair of lines
            dest1 += step;
            dest2 += step;
            frac += fracstep;
            y_start += step;
        }

        // Handle final odd line
        if (y_start == y_end)
        {
            const unsigned s  = sourcebase[frac >> FRACBITS];
            const pixel_t bg1 = *dest1;
            const pixel_t bg2 = *dest2;
            const pixel_t fg  = brightmap[s] ? colormap1[s] : colormap0[s];
            const pixel_t blended1 = I_BlendAdd_32(bg1, fg);
            const pixel_t blended2 = I_BlendAdd_32(bg2, fg);

            dest1[0] = blended1;
            dest2[0] = blended2;
        }
    }
    else // No vertical duplication in 1× mode
    {
        const fixed_t fracstep = dc_iscale;

        while (y_start <= y_end)
        {
            const unsigned s  = sourcebase[frac >> FRACBITS];
            const pixel_t bg1 = *dest1;
            const pixel_t bg2 = *dest2;
            const pixel_t fg  = brightmap[s] ? colormap1[s] : colormap0[s];
            const pixel_t blended1 = I_BlendAdd_32(bg1, fg);
            const pixel_t blended2 = I_BlendAdd_32(bg2, fg);

            *dest1 = blended1;
            *dest2 = blended2;
            dest1++;
            dest2++;
            frac += fracstep;
            ++y_start;
        }
    }
}

// -----------------------------------------------------------------------------
// R_DrawFuzzTLColumn
// [PN/JN] Draw translucent column for fuzz effect, overlay blending.
// -----------------------------------------------------------------------------

void R_DrawFuzzTLColumn (void)
{
    const int count = dc_yh - dc_yl;
    if (count < 0)
        return; // No pixels to draw

    // Local pointers for improved memory access
    const byte *restrict const sourcebase   = dc_source;
    const byte *restrict const brightmap    = dc_brightmap;
    const pixel_t *restrict const colormap0 = dc_colormap[0];
    const pixel_t *restrict const colormap1 = dc_colormap[1];
    int y_start = dc_yl;
    int y_end = dc_yh;

    // Setup scaling
    fixed_t frac = dc_texturemid + (y_start - centery) * dc_iscale;

    // Precompute initial destination pointer
    pixel_t *restrict dest = ylookup[y_start] + columnofs[flipviewwidth[dc_x]];

    if (vid_resolution > 1) // Duplicate pixels vertically for performance
    {
        const int step = 2;
        const fixed_t fracstep = dc_iscale * step;

        // Compute one pixel, write it to two vertical lines
        while (y_start < y_end)
        {
            const unsigned s = sourcebase[frac >> FRACBITS];
            const pixel_t bg = *dest;
            const pixel_t fg = brightmap[s] ? colormap1[s] : colormap0[s];
            const pixel_t blended = I_BlendOver64_32(bg, fg);

            // Write two pixels (current and next line)
            dest[0] = blended;
            dest[1] = blended;

            // Move to next pair
            dest    += step;
            frac    += fracstep;
            y_start += step;
        }

        // Handle final odd line
        if (y_start == y_end)
        {
            const unsigned s = sourcebase[frac >> FRACBITS];
            const pixel_t bg = *dest;
            const pixel_t fg = brightmap[s] ? colormap1[s] : colormap0[s];
            const pixel_t blended = I_BlendOver64_32(bg, fg);

            dest[0] = blended;
        }
    }
    else // No vertical duplication in 1× mode
    {
        const fixed_t fracstep = dc_iscale;

        while (y_start <= y_end)
        {
            const unsigned s = sourcebase[frac >> FRACBITS];
            const pixel_t bg = *dest;
            const pixel_t fg = brightmap[s] ? colormap1[s] : colormap0[s];
            const pixel_t blended = I_BlendOver64_32(bg, fg);

            *dest = blended;
            dest++;
            frac += fracstep;
            ++y_start;
        }
    }
}

void R_DrawFuzzTLColumnLow (void)
{
    const int count = dc_yh - dc_yl;
    if (count < 0)
        return; // No pixels to draw

    // Low detail: double horizontal resolution
    const int x = dc_x << 1;

    // Local pointers for improved memory access
    const byte *restrict const sourcebase   = dc_source;
    const byte *restrict const brightmap    = dc_brightmap;
    const pixel_t *restrict const colormap0 = dc_colormap[0];
    const pixel_t *restrict const colormap1 = dc_colormap[1];
    int y_start = dc_yl;
    int y_end = dc_yh;

    // Setup scaling
    fixed_t frac = dc_texturemid + (y_start - centery) * dc_iscale;

    // Precompute initial destination pointer
    pixel_t *restrict dest1 = ylookup[y_start] + columnofs[flipviewwidth[x]];
    pixel_t *restrict dest2 = ylookup[y_start] + columnofs[flipviewwidth[x + 1]];

    if (vid_resolution > 1) // Duplicate pixels vertically for performance
    {
        const int step = 2;
        const fixed_t fracstep = dc_iscale * step;

        // Compute one pixel, write it to two vertical lines
        while (y_start < y_end)
        {
            const unsigned s  = sourcebase[frac >> FRACBITS];
            const pixel_t bg1 = *dest1;
            const pixel_t bg2 = *dest2;
            const pixel_t fg  = brightmap[s] ? colormap1[s] : colormap0[s];
            const pixel_t blended1 = I_BlendOver64_32(bg1, fg);
            const pixel_t blended2 = I_BlendOver64_32(bg2, fg);
            // Process two lines for both columns
            dest1[0] = blended1;
            dest1[1] = blended1;
            dest2[0] = blended2;
            dest2[1] = blended2;

            // Move to next pair of lines
            dest1 += step;
            dest2 += step;
            frac += fracstep;
            y_start += step;
        }

        // Handle final odd line
        if (y_start == y_end)
        {
            const unsigned s  = sourcebase[frac >> FRACBITS];
            const pixel_t bg1 = *dest1;
            const pixel_t bg2 = *dest2;
            const pixel_t fg  = brightmap[s] ? colormap1[s] : colormap0[s];
            const pixel_t blended1 = I_BlendOver64_32(bg1, fg);
            const pixel_t blended2 = I_BlendOver64_32(bg2, fg);

            dest1[0] = blended1;
            dest2[0] = blended2;
        }
    }
    else // No vertical duplication in 1× mode
    {
        const fixed_t fracstep = dc_iscale;

        while (y_start <= y_end)
        {
            const unsigned s  = sourcebase[frac >> FRACBITS];
            const pixel_t bg1 = *dest1;
            const pixel_t bg2 = *dest2;
            const pixel_t fg  = brightmap[s] ? colormap1[s] : colormap0[s];
            const pixel_t blended1 = I_BlendOver64_32(bg1, fg);
            const pixel_t blended2 = I_BlendOver64_32(bg2, fg);

            *dest1 = blended1;
            *dest2 = blended2;
            dest1++;
            dest2++;
            frac += fracstep;
            ++y_start;
        }
    }
}

// -----------------------------------------------------------------------------
// R_DrawFuzzTLTransColumn
// [PN/JN] Translucent, translated fuzz column.
// -----------------------------------------------------------------------------

void R_DrawFuzzTLTransColumn (void)
{
    const int count = dc_yh - dc_yl;
    if (count < 0)
        return; // No pixels to draw

    // Local pointers for improved memory access
    const byte *restrict const sourcebase   = dc_source;
    const byte *restrict const translation  = dc_translation;
    const pixel_t *restrict const colormap0 = dc_colormap[0];
    int y_start = dc_yl;
    int y_end = dc_yh;

    // Setup scaling
    fixed_t frac = dc_texturemid + (y_start - centery) * dc_iscale;

    // Precompute initial destination pointer
    pixel_t *restrict dest = ylookup[y_start] + columnofs[flipviewwidth[dc_x]];

    if (vid_resolution > 1) // Duplicate pixels vertically for performance
    {
        const int step = 2;
        const fixed_t fracstep = dc_iscale * step;

        // Compute one pixel, write it to two vertical lines
        while (y_start < y_end)
        {
            const unsigned s = sourcebase[frac >> FRACBITS];
            const pixel_t bg = *dest;
            const pixel_t fg = colormap0[translation[s]];
            const pixel_t blended = I_BlendOver64_32(bg, fg);

            // Write two pixels (current and next line)
            dest[0] = blended;
            dest[1] = blended;

            // Move to next pair
            dest    += step;
            frac    += fracstep;
            y_start += step;
        }

        // Handle final odd line
        if (y_start == y_end)
        {
            const unsigned s = sourcebase[frac >> FRACBITS];
            const pixel_t bg = *dest;
            const pixel_t fg = colormap0[translation[s]];
            const pixel_t blended = I_BlendOver64_32(bg, fg);

            dest[0] = blended;
        }
    }
    else // No vertical duplication in 1× mode
    {
        const fixed_t fracstep = dc_iscale;

        while (y_start <= y_end)
        {
            const unsigned s = sourcebase[frac >> FRACBITS];
            const pixel_t bg = *dest;
            const pixel_t fg = colormap0[translation[s]];
            const pixel_t blended = I_BlendOver64_32(bg, fg);

            *dest = blended;
            dest++;
            frac += fracstep;
            ++y_start;
        }
    }
}

void R_DrawFuzzTLTransColumnLow (void)
{
    const int count = dc_yh - dc_yl;
    if (count < 0)
        return; // No pixels to draw

    // Low detail: double horizontal resolution
    const int x = dc_x << 1;

    // Local pointers for improved memory access
    const byte *restrict const sourcebase   = dc_source;
    const byte *restrict const translation  = dc_translation;
    const pixel_t *restrict const colormap0 = dc_colormap[0];
    int y_start = dc_yl;
    int y_end = dc_yh;

    // Setup scaling
    fixed_t frac = dc_texturemid + (y_start - centery) * dc_iscale;

    // Precompute initial destination pointer
    pixel_t *restrict dest1 = ylookup[y_start] + columnofs[flipviewwidth[x]];
    pixel_t *restrict dest2 = ylookup[y_start] + columnofs[flipviewwidth[x + 1]];

    if (vid_resolution > 1) // Duplicate pixels vertically for performance
    {
        const int step = 2;
        const fixed_t fracstep = dc_iscale * step;

        // Compute one pixel, write it to two vertical lines
        while (y_start < y_end)
        {
            const unsigned s  = sourcebase[frac >> FRACBITS];
            const pixel_t bg1 = *dest1;
            const pixel_t bg2 = *dest2;
            const pixel_t fg  = colormap0[translation[s]];
            const pixel_t blended1 = I_BlendOver64_32(bg1, fg);
            const pixel_t blended2 = I_BlendOver64_32(bg2, fg);
            // Process two lines for both columns
            dest1[0] = blended1;
            dest1[1] = blended1;
            dest2[0] = blended2;
            dest2[1] = blended2;

            // Move to next pair of lines
            dest1 += step;
            dest2 += step;
            frac += fracstep;
            y_start += step;
        }

        // Handle final odd line
        if (y_start == y_end)
        {
            const unsigned s  = sourcebase[frac >> FRACBITS];
            const pixel_t bg1 = *dest1;
            const pixel_t bg2 = *dest2;
            const pixel_t fg  = colormap0[translation[s]];
            const pixel_t blended1 = I_BlendOver64_32(bg1, fg);
            const pixel_t blended2 = I_BlendOver64_32(bg2, fg);

            dest1[0] = blended1;
            dest2[0] = blended2;
        }
    }
    else // No vertical duplication in 1× mode
    {
        const fixed_t fracstep = dc_iscale;

        while (y_start <= y_end)
        {
            const unsigned s  = sourcebase[frac >> FRACBITS];
            const pixel_t bg1 = *dest1;
            const pixel_t bg2 = *dest2;
            const pixel_t fg  = colormap0[translation[s]];
            const pixel_t blended1 = I_BlendOver64_32(bg1, fg);
            const pixel_t blended2 = I_BlendOver64_32(bg2, fg);

            *dest1 = blended1;
            *dest2 = blended2;
            dest1++;
            dest2++;
            frac += fracstep;
            ++y_start;
        }
    }
}

// -----------------------------------------------------------------------------
// R_DrawShadowColumn
// [PN/JN] Darken destination pixels using I_BlendDark, preserving sprite mask.
// -----------------------------------------------------------------------------

void R_DrawShadowColumn (void)
{
    const int count = dc_yh - dc_yl;
    if (count < 0)
        return; // No pixels to draw

    const int dark_amount = shadow_alpha;
    int y_start = dc_yl;
    int y_end = dc_yh;
    pixel_t *restrict dest = ylookup[y_start] + columnofs[flipviewwidth[dc_x]];

    if (vid_resolution > 1)
    {
        const int step = 2;

        while (y_start < y_end)
        {
            const pixel_t blended = I_BlendDark_32(*dest, dark_amount);

            dest[0] = blended;
            dest[1] = blended;

            dest += step;
            y_start += step;
        }

        if (y_start == y_end)
        {
            dest[0] = I_BlendDark_32(*dest, dark_amount);
        }
    }
    else
    {
        while (y_start <= y_end)
        {
            *dest = I_BlendDark_32(*dest, dark_amount);
            dest++;
            ++y_start;
        }
    }
}

void R_DrawShadowColumnLow (void)
{
    const int count = dc_yh - dc_yl;
    if (count < 0)
        return; // No pixels to draw

    const int x = dc_x << 1;
    const int dark_amount = shadow_alpha;
    int y_start = dc_yl;
    int y_end = dc_yh;
    pixel_t *restrict dest1 = ylookup[y_start] + columnofs[flipviewwidth[x]];
    pixel_t *restrict dest2 = ylookup[y_start] + columnofs[flipviewwidth[x + 1]];

    if (vid_resolution > 1)
    {
        const int step = 2;

        while (y_start < y_end)
        {
            const pixel_t blended1 = I_BlendDark_32(*dest1, dark_amount);
            const pixel_t blended2 = I_BlendDark_32(*dest2, dark_amount);

            dest1[0] = blended1;
            dest1[1] = blended1;
            dest2[0] = blended2;
            dest2[1] = blended2;

            dest1 += step;
            dest2 += step;
            y_start += step;
        }

        if (y_start == y_end)
        {
            dest1[0] = I_BlendDark_32(*dest1, dark_amount);
            dest2[0] = I_BlendDark_32(*dest2, dark_amount);
        }
    }
    else
    {
        while (y_start <= y_end)
        {
            *dest1 = I_BlendDark_32(*dest1, dark_amount);
            *dest2 = I_BlendDark_32(*dest2, dark_amount);
            dest1++;
            dest2++;
            ++y_start;
        }
    }
}

// -----------------------------------------------------------------------------
// R_DrawTransTLFuzzColumn
// [PN/JN] Translucent, translated fuzz column.
// -----------------------------------------------------------------------------

void R_DrawTransTLFuzzColumn(void)
{
    const int count = dc_yh - dc_yl;
    if (count < 0)
        return; // No pixels to draw

    // Destination pointer calculation
    pixel_t *restrict dest = ylookup[dc_yl] + columnofs[flipviewwidth[dc_x]];

    // Setup scaling
    const fixed_t fracstep = dc_iscale;
    fixed_t frac = dc_texturemid + (dc_yl - centery) * fracstep;

    // Local pointers for improved memory access
    const byte *restrict const sourcebase = dc_source;
    const byte *restrict const translation = dc_translation;
    const pixel_t *restrict const colormap0 = dc_colormap[0];

    // Aggressive optimization: compact loop for blending pixels
    const int iterations = count + 1;
    for (int i = 0; i < iterations; ++i)
    {
        const unsigned s = sourcebase[frac >> FRACBITS];   // Texture sample
        const unsigned t = translation[s];                // Translation lookup
        *dest = I_BlendOver64_32(*dest, colormap0[t]); // Blend operation inline

        dest++;
        frac += fracstep;    // Increment texture coordinate
    }
}

// -----------------------------------------------------------------------------
// R_DrawTransTLFuzzColumnLow
// [PN/JN] Translucent, translated fuzz column, low-resolution version.
// -----------------------------------------------------------------------------

void R_DrawTransTLFuzzColumnLow(void)
{
    const int count = dc_yh - dc_yl;
    if (count < 0)
        return; // No pixels to draw

    // Blocky mode: double the x coordinate
    const int x = dc_x << 1;

    // Destination pointer calculations
    pixel_t *restrict dest = ylookup[dc_yl] + columnofs[flipviewwidth[x]];
    pixel_t *restrict dest2 = ylookup[dc_yl] + columnofs[flipviewwidth[x + 1]];

    // Setup scaling
    const fixed_t fracstep = dc_iscale;
    fixed_t frac = dc_texturemid + (dc_yl - centery) * fracstep;

    // Local pointers to improve memory access
    const byte *restrict const sourcebase = dc_source;
    const byte *restrict const translation = dc_translation;
    const pixel_t *restrict const colormap0 = dc_colormap[0];

    // Aggressively optimized loop for blending pixels
    const int iterations = count + 1;
    for (int i = 0; i < iterations; ++i)
    {
        const unsigned s = sourcebase[frac >> FRACBITS];   // Texture sample
        const pixel_t destrgb = colormap0[translation[s]]; // Translation + colormap lookup

        // Blend operation inline
        *dest = I_BlendOver64_32(*dest, destrgb);
        *dest2 = I_BlendOver64_32(*dest2, destrgb);

        // Advance destination pointers and texture coordinate
        dest++;
        dest2++;
        frac += fracstep;
    }
}
