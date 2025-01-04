//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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
//	Gamma correction LUT stuff.
//	Functions to draw patches (by post) directly to screen.
//	Functions to blit a block to the screen.
//

#include <stdio.h>
#include <string.h>
#include <math.h>

#define MINIZ_NO_STDIO
#define MINIZ_NO_ZLIB_APIS
#include "miniz.h"

#include "i_system.h"
#include "i_input.h"
#include "i_swap.h"
#include "i_video.h"
#include "m_bbox.h"
#include "m_config.h"
#include "m_misc.h"
#include "v_trans.h"
#include "v_video.h"
#include "w_wad.h"
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

static int dirtybox[4]; 


//
// V_MarkRect 
// 

void V_MarkRect(int x, int y, int width, int height) 
{ 
    // If we are temporarily using an alternate screen, do not 
    // affect the update box.

    if (dest_screen == I_VideoBuffer)
    {
        M_AddToBox (dirtybox, x, y); 
        M_AddToBox (dirtybox, x + width-1, y + height-1); 
    }
} 

//
// V_CopyRect 
// 

void V_CopyRect(int srcx, int srcy, pixel_t *source,
                int width, int height,
                int destx, int desty)
{ 
    pixel_t *src;
    pixel_t *dest;
 
#ifdef RANGECHECK 
    if (srcx < 0
     || srcx + width > SCREENWIDTH
     || srcy < 0
     || srcy + height > SCREENHEIGHT 
     || destx < 0
     || destx /* + width */ > SCREENWIDTH
     || desty < 0
     || desty /* + height */ > SCREENHEIGHT)
    {
        // [JN] Note: should be I_Error, but use return instead
        // until status bar background buffer gets rewritten values.
        // I_Error ("Bad V_CopyRect");
        return;
    }
#endif 

    // [crispy] prevent framebuffer overflow
    if (destx + width > SCREENWIDTH)
	width = SCREENWIDTH - destx;
    if (desty + height > SCREENHEIGHT)
	height = SCREENHEIGHT - desty;

    V_MarkRect(destx, desty, width, height); 
 
    src = source + SCREENWIDTH * srcy + srcx; 
    dest = dest_screen + SCREENWIDTH * desty + destx; 

    for ( ; height>0 ; height--) 
    { 
        memcpy(dest, src, width * sizeof(*dest));
        src += SCREENWIDTH; 
        dest += SCREENWIDTH; 
    } 
} 

//
// V_DrawPatch
// Masks a column based masked pic to the screen. 
//

// [crispy] four different rendering functions
// for each possible combination of dp_translation and dp_translucent:
// (1) normal, opaque patch
static inline pixel_t drawpatchpx00 (const pixel_t dest, const pixel_t source)
{return palette_pointer[source];}
// (2) color-translated, opaque patch
static inline pixel_t drawpatchpx01 (const pixel_t dest, const pixel_t source)
{return palette_pointer[dp_translation[source]];}
// (3) normal, translucent patch
static inline pixel_t drawpatchpx10 (const pixel_t dest, const pixel_t source)
{return I_BlendOver(dest, palette_pointer[source], 168);}
// (4) color-translated, translucent patch
static inline pixel_t drawpatchpx11 (const pixel_t dest, const pixel_t source)
{return I_BlendOver(dest, palette_pointer[dp_translation[source]], 168);}

// [JN] The shadow of the patch rendering function:
static inline pixel_t drawshadow_doom (const pixel_t dest, const pixel_t source)
{return I_BlendDark(dest, 160);} // [JN] 160 of 256 full translucency.

// [crispy] array of function pointers holding the different rendering functions
typedef pixel_t drawpatchpx_t (const pixel_t dest, const pixel_t source);
static drawpatchpx_t *const drawpatchpx_a[2][2] = {{drawpatchpx11, drawpatchpx10}, {drawpatchpx01, drawpatchpx00}};

static fixed_t dx, dxi, dy, dyi;

void V_DrawPatch(int x, int y, patch_t *patch)
{ 
    int count;
    int col;
    column_t *column;
    pixel_t *desttop;
    pixel_t *dest;
    byte *source;
    int w;

    // [crispy] four different rendering functions
    drawpatchpx_t *const drawpatchpx = drawpatchpx_a[!dp_translucent][!dp_translation];

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);
    x += WIDESCREENDELTA; // [crispy] horizontal widescreen offset

    V_MarkRect(x, y, SHORT(patch->width), SHORT(patch->height));

    col = 0;
    if (x < 0)
    {
	col += dxi * ((-x * dx) >> FRACBITS);
	x = 0;
    }

    desttop = dest_screen + ((y * dy) >> FRACBITS) * SCREENWIDTH + ((x * dx) >> FRACBITS);

    w = SHORT(patch->width);

    // convert x to screen position
    x = (x * dx) >> FRACBITS;

    for ( ; col<w << FRACBITS ; x++, col+=dxi, desttop++)
    {
        int topdelta = -1;

        // [crispy] too far right / width
        if (x >= SCREENWIDTH)
        {
            break;
        }

        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            int top, srccol = 0;
            // [crispy] support for DeePsea tall patches
            if (column->topdelta <= topdelta)
            {
                topdelta += column->topdelta;
            }
            else
            {
                topdelta = column->topdelta;
            }
            top = ((y + topdelta) * dy) >> FRACBITS;
            source = (byte *)column + 3;
            dest = desttop + ((topdelta * dy) >> FRACBITS)*SCREENWIDTH;
            count = (column->length * dy) >> FRACBITS;

            // [crispy] too low / height
            if (top + count > SCREENHEIGHT)
            {
                count = SCREENHEIGHT - top;
            }

            // [crispy] nothing left to draw?
            if (count < 1)
            {
                break;
            }

            while (count--)
            {
                // [crispy] too high
                if (top++ >= 0)
                {
                    *dest = drawpatchpx(*dest, source[srccol >> FRACBITS]);
                }
                srccol += dyi;
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

//
// V_DrawShadowedOptional
// [JN] Draws patch with shadow if "Text casts shadows" feature is enabled.
//  dest  - main patch, drawed second on top of shadow.
//  dest2 - shadow, drawed first below main patch.
//

void V_DrawShadowedPatchOptional(int x, int y, patch_t *patch)
{
    int count, count2, col;
    column_t *column;
    pixel_t *desttop, *dest;
    byte *source;
    pixel_t *dest2;
    int w;

    // [JN] Simplify math for shadow placement.
    const int shadow_shift = (SCREENWIDTH + 1) * vid_resolution;
    // [crispy] four different rendering functions
    drawpatchpx_t *const drawpatchpx = drawpatchpx_a[!dp_translucent][!dp_translation];

    // [JN] Shadow, blending depending on game type:
    drawpatchpx_t *const drawpatchpx2 = drawshadow_doom;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);
    x += WIDESCREENDELTA; // [crispy] horizontal widescreen offset

    V_MarkRect(x, y, SHORT(patch->width), SHORT(patch->height));

    col = 0;
    if (x < 0)
    {
	col += dxi * ((-x * dx) >> FRACBITS);
	x = 0;
    }

    desttop = dest_screen + ((y * dy) >> FRACBITS) * SCREENWIDTH + ((x * dx) >> FRACBITS);

    w = SHORT(patch->width);

    // convert x to screen position
    x = (x * dx) >> FRACBITS;

    for ( ; col<w << FRACBITS ; x++, col+=dxi, desttop++)
    {
        int topdelta = -1;

        // [crispy] too far right / width
        if (x >= SCREENWIDTH)
        {
            break;
        }

        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            int top, top2, srccol = 0;
            // [crispy] support for DeePsea tall patches
            if (column->topdelta <= topdelta)
            {
                topdelta += column->topdelta;
            }
            else
            {
                topdelta = column->topdelta;
            }
            top = ((y + topdelta) * dy) >> FRACBITS;
            top2 = top;
            source = (byte *)column + 3;
            dest = desttop + ((topdelta * dy) >> FRACBITS)*SCREENWIDTH;
            dest2 = dest + shadow_shift;
            count2 = count = (column->length * dy) >> FRACBITS;

            if (top + count2 > (SCREENHEIGHT - (1 * vid_resolution)))
            {
                count2 = (SCREENHEIGHT - (1 * vid_resolution)) - top;
            }
            // [crispy] too low / height
            if (top + count > SCREENHEIGHT)
            {
                count = SCREENHEIGHT - top;
            }

            // [crispy] nothing left to draw?
            if (count < 1 || count2 < 1)
            {
                break;
            }

            if (msg_text_shadows)
            {
                while (count2--)
                {
                    if (top2++ >= 0)
                    {
                        *dest2 = drawpatchpx2(*dest2, source[srccol >> FRACBITS]);
                    }
                    dest2 += SCREENWIDTH;
                }
            }
            while (count--)
            {
                // [crispy] too high
                if (top++ >= 0)
                {
                    *dest = drawpatchpx(*dest, source[srccol >> FRACBITS]);
                }
                srccol += dyi;
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawPatchFullScreen(patch_t *patch, boolean flipped)
{
    const int x = DivRoundClosest((ORIGWIDTH - SHORT(patch->width)), 2);

    patch->leftoffset = 0;
    patch->topoffset = 0;

    // [crispy] fill pillarboxes in widescreen mode
    if (SCREENWIDTH != NONWIDEWIDTH)
    {
        V_DrawFilledBox(0, 0, SCREENWIDTH, SCREENHEIGHT, 0);
    }

    V_DrawPatch(x, 0, patch);
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

//
// V_DrawBlock
// Draw a linear block of pixels into the view buffer.
//

void V_DrawBlock(int x, int y, int width, int height, pixel_t *src)
{ 
    pixel_t *dest;
 
#ifdef RANGECHECK 
    if (x < 0
     || x + width >SCREENWIDTH
     || y < 0
     || y + height > SCREENHEIGHT)
    {
	I_Error ("Bad V_DrawBlock");
    }
#endif 
 
    V_MarkRect (x, y, width, height); 
 
    dest = dest_screen + (y * vid_resolution) * SCREENWIDTH + x;

    while (height--) 
    { 
	memcpy (dest, src, width * sizeof(*dest));
	src += width; 
	dest += SCREENWIDTH; 
    } 
} 

void V_DrawFilledBox(int x, int y, int w, int h, int c)
{
    pixel_t *buf, *buf1;
    int x1, y1;

    buf = I_VideoBuffer + SCREENWIDTH * y + x;

    for (y1 = 0; y1 < h; ++y1)
    {
        buf1 = buf;

        for (x1 = 0; x1 < w; ++x1)
        {
            *buf1++ = c;
        }

        buf += SCREENWIDTH;
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

// [crispy] Unified function of flat filling. Used for intermission
// and finale screens, view border and status bar's wide screen mode.
void V_FillFlat(int y_start, int y_stop, int x_start, int x_stop,
                const byte *src, pixel_t *dest)
{
    int x, y;

    for (y = y_start; y < y_stop; y++)
    {
        for (x = x_start; x < x_stop; x++)
        {
            *dest++ = palette_pointer[src[(((y / vid_resolution) & 63) * 64)
                                         + ((x / vid_resolution) & 63)]];
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
    // no-op!
    // There used to be separate screens that could be drawn to; these are
    // now handled in the upper layers.
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

void WritePNGfile (char *filename)
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

void V_ScreenShot(char *format)
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
    }

    if (i == 10000)
    {
        I_Error ("V_ScreenShot: Couldn't create a PNG");
    }

    WritePNGfile(file);
}
