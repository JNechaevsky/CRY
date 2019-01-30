//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2019 Julia Nechaevskaya
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


#include "doomdef.h"
#include "deh_main.h"
#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"
#include "r_local.h"
#include "v_video.h"
#include "v_trans.h"
#include "doomstat.h"
#include "jn.h"


// ?
#define MAXWIDTH    1120
#define MAXHEIGHT   832

// status bar height at bottom of screen
// [Julia] Jaguar status bar height is 40 pixels
#define SBARHEIGHT  (40 << hires)

//
// All drawing to the view buffer is accomplished in this file.
// The other refresh files only know about ccordinates,
//  not the architecture of the frame buffer.
// Conveniently, the frame buffer is a linear one,
//  and we need only the base address,
//  and the total size == width*height*depth/8.,
//
byte *viewimage; 
int   viewwidth;
int   scaledviewwidth;
int   viewheight;
int   scaledviewheight;
int   viewwindowx;
int   viewwindowy; 
byte *ylookup[MAXHEIGHT]; 
int   columnofs[MAXWIDTH]; 


// Color tables for different players,
//  translate a limited part to another
//  (color ramps used for  suit colors).
//
byte translations[3][256];	
 
// Backing buffer containing the bezel drawn around the screen and 
// surrounding background.
static byte *background_buffer = NULL;

// R_DrawColumn
// Source is the top of the column to scale.
lighttable_t *dc_colormap; 
int           dc_x; 
int           dc_yl; 
int           dc_yh; 
fixed_t       dc_iscale; 
fixed_t       dc_texturemid;
int           dc_texheight;

// first pixel in a column (possibly virtual) 
byte *dc_source;		

// just for profiling 
int dccount;


// -----------------------------------------------------------------------------
// R_DrawColumn
//
// A column is a vertical slice/span from a wall texture that,
//  given the DOOM style restrictions on the view orientation,
//  will always have constant z depth.
// Thus a special case loop for very fast rendering can
//  be used. It has also been used with Wolfenstein 3D.
// -----------------------------------------------------------------------------
void R_DrawColumn (void) 
{ 
    int      count;
    byte    *dest;  // killough
    fixed_t  frac;  // killough
    fixed_t  fracstep;
    int heightmask = dc_texheight-1;
    const byte *source = dc_source;
    const lighttable_t *colormap = dc_colormap;

    count = dc_yh - dc_yl + 1;

    if (count <= 0)    // Zero length, column does not exceed a pixel.
    return;

#ifdef RANGECHECK
    if ((unsigned)dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
    }
#endif

    // Framebuffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows?
    dest = ylookup[dc_yl] + columnofs[dc_x];

    // Determine scaling, which is the only mapping to be done.
    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centery)*fracstep;

    // Inner loop that does the actual texture mapping,
    //  e.g. a DDA-lile scaling.
    // This is as fast as it gets.       (Yeah, right!!! -- killough)
    //
    // killough 2/1/98: more performance tuning
    {
        if (dc_texheight & heightmask)   // not a power of 2 -- killough
        {
            heightmask++;
            heightmask <<= FRACBITS;

            if (frac < 0)
            while ((frac += heightmask) < 0);
            else
            while (frac >= heightmask)
            frac -= heightmask;

            do
            {
                // Re-map color indices from wall texture column
                //  using a lighting/special effects LUT.
                *dest = colormap[source[frac>>FRACBITS]];
                dest += SCREENWIDTH;                     // killough 11/98
                if ((frac += fracstep) >= heightmask)
                frac -= heightmask;
            }
            while (--count);
        }
        else
        {
            while ((count-=2)>=0)   // texture height is a power of 2 -- killough
            {
                *dest = colormap[source[(frac>>FRACBITS) & heightmask]];
                dest += SCREENWIDTH;   // killough 11/98
                frac += fracstep;
                *dest = colormap[source[(frac>>FRACBITS) & heightmask]];
                dest += SCREENWIDTH;   // killough 11/98
                frac += fracstep;
            }
            if (count & 1)
            *dest = colormap[source[(frac>>FRACBITS) & heightmask]];
        }
    }
}


// -----------------------------------------------------------------------------
// R_DrawColumnLow
// -----------------------------------------------------------------------------

void R_DrawColumnLow (void) 
{ 
    int       count; 
    byte     *dest; 
    byte     *dest2;
    byte     *dest3;
    byte     *dest4;
    fixed_t   frac;
    fixed_t   fracstep;	 
    int       x;
    int       heightmask = dc_texheight - 1;

    count = dc_yh - dc_yl; 

    // Zero length.
    if (count < 0) 
    return; 

#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
    }
    //	dccount++; 
#endif 

    // Blocky mode, need to multiply by 2.
    x = dc_x << 1;

    dest  = ylookup[(dc_yl << hires)] + columnofs[x];
    dest2 = ylookup[(dc_yl << hires)] + columnofs[x+1];
    dest3 = ylookup[(dc_yl << hires) + 1] + columnofs[x];
    dest4 = ylookup[(dc_yl << hires) + 1] + columnofs[x+1];

    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep;

    // heightmask is the Tutti-Frutti fix -- killough
    if (dc_texheight & heightmask) // not a power of 2 -- killough
    {
        heightmask++;
        heightmask <<= FRACBITS;

        if (frac < 0)
        while ((frac += heightmask) < 0);
        else
        while (frac >= heightmask)
        frac -= heightmask;

        do
        {
            *dest2 = *dest = dc_colormap[dc_source[frac>>FRACBITS]];

            dest += SCREENWIDTH << hires;
            dest2 += SCREENWIDTH << hires;

            if (hires)
            {
                *dest4 = *dest3 = dc_colormap[dc_source[frac>>FRACBITS]];
                dest3 += SCREENWIDTH << hires;
                dest4 += SCREENWIDTH << hires;
            }

            if ((frac += fracstep) >= heightmask)
            frac -= heightmask;
        } while (count--);
    }
    else // texture height is a power of 2 -- killough
    {
        do 
        {
            // Hack. Does not work corretly.
            *dest2 = *dest = dc_colormap[dc_source[(frac>>FRACBITS)&heightmask]];
            dest += SCREENWIDTH << hires;
            dest2 += SCREENWIDTH << hires;

            if (hires)
            {
                *dest4 = *dest3 = dc_colormap[dc_source[(frac>>FRACBITS)&heightmask]];
                dest3 += SCREENWIDTH << hires;
                dest4 += SCREENWIDTH << hires;
            }

            frac += fracstep; 

        } while (count--);
    }
}


// -----------------------------------------------------------------------------
// R_DrawTranslatedColumn
//
// R_DrawTranslatedColumn
// Used to draw player sprites
//  with the green colorramp mapped to others.
// Could be used with different translation
//  tables, e.g. the lighter colored version
//  of the BaronOfHell, the HellKnight, uses
//  identical sprites, kinda brightened up.
// -----------------------------------------------------------------------------

byte *dc_translation;
byte *translationtables;

void R_DrawTranslatedColumn (void) 
{ 
    int      count; 
    byte    *dest; 
    fixed_t  frac;
    fixed_t  fracstep;	 

    count = dc_yh - dc_yl; 
    if (count < 0) 
    return; 

#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
    }    
#endif 

    dest = ylookup[dc_yl] + columnofs[dc_x]; 

    // Looks familiar.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep; 

    // Here we do an additional index re-mapping.
    do 
    {
        // Translation tables are used
        //  to map certain colorramps to other ones,
        //  used with PLAY sprites.
        // Thus the "green" ramp of the player 0 sprite
        //  is mapped to gray, red, black/indigo. 
        *dest = dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]];
        dest += SCREENWIDTH;
	
        frac += fracstep; 
    } while (count--); 
} 


// -----------------------------------------------------------------------------
// R_DrawTranslatedColumnLow
// -----------------------------------------------------------------------------

void R_DrawTranslatedColumnLow (void) 
{ 
    int      count; 
    byte    *dest; 
    byte    *dest2; 
    byte    *dest3;
    byte    *dest4;
    fixed_t  frac;
    fixed_t  fracstep;	 
    int      x;

    count = dc_yh - dc_yl; 
    if (count < 0) 
    return; 

    // low detail, need to scale by 2
    x = dc_x << 1;

#ifdef RANGECHECK 
    if ((unsigned)x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, x);
    }
#endif 

    dest  = ylookup[(dc_yl << hires)] + columnofs[x];
    dest2 = ylookup[(dc_yl << hires)] + columnofs[x+1];
    dest3 = ylookup[(dc_yl << hires) + 1] + columnofs[x];
    dest4 = ylookup[(dc_yl << hires) + 1] + columnofs[x+1];

    // Looks familiar.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep; 

    // Here we do an additional index re-mapping.
    do 
    {
        // Translation tables are used
        //  to map certain colorramps to other ones,
        //  used with PLAY sprites.
        // Thus the "green" ramp of the player 0 sprite
        //  is mapped to gray, red, black/indigo. 
        *dest  = dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]];
        *dest2 = dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]];
        dest  += SCREENWIDTH << hires;
        dest2 += SCREENWIDTH << hires;
        if (hires)
        {
            *dest3 = dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]];
            *dest4 = dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]];
            dest3 += SCREENWIDTH << hires;
            dest4 += SCREENWIDTH << hires;
        }

        frac += fracstep; 
    } while (count--); 
}


// -----------------------------------------------------------------------------
// R_DrawTLColumn
// -----------------------------------------------------------------------------

void R_DrawTLColumn (void)
{
    int      count;
    byte    *dest;
    fixed_t  frac;
    fixed_t  fracstep;

    count = dc_yh - dc_yl;
    if (count < 0)
    return;

#ifdef RANGECHECK
    if ((unsigned)dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
    }
#endif

    dest = ylookup[dc_yl] + columnofs[dc_x];

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centery)*fracstep;

    do
    {
        *dest = tranmap[(*dest<<8)+dc_colormap[dc_source[frac>>FRACBITS]]];
        dest += SCREENWIDTH;

        frac += fracstep;
    } while (count--);
}


// -----------------------------------------------------------------------------
// R_DrawTLColumnLow
//
// [crispy] draw translucent column, low-resolution version
// -----------------------------------------------------------------------------

void R_DrawTLColumnLow (void)
{
    int      count;
    byte    *dest;
    byte    *dest2;
    byte    *dest3;
    byte    *dest4;
    fixed_t  frac;
    fixed_t  fracstep;
    int      x;

    count = dc_yh - dc_yl;
    if (count < 0)
    return;

    x = dc_x << 1;

#ifdef RANGECHECK
    if ((unsigned)x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, x);
    }
#endif

    dest  = ylookup[(dc_yl << hires)] + columnofs[x];
    dest2 = ylookup[(dc_yl << hires)] + columnofs[x+1];
    dest3 = ylookup[(dc_yl << hires) + 1] + columnofs[x];
    dest4 = ylookup[(dc_yl << hires) + 1] + columnofs[x+1];

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centery)*fracstep;

    do
    {
        *dest  = tranmap[(*dest<<8)+dc_colormap[dc_source[frac>>FRACBITS]]];
        *dest2 = tranmap[(*dest2<<8)+dc_colormap[dc_source[frac>>FRACBITS]]];
        dest  += SCREENWIDTH << hires;
        dest2 += SCREENWIDTH << hires;

        if (hires)
        {
            *dest3 = tranmap[(*dest3<<8)+dc_colormap[dc_source[frac>>FRACBITS]]];
            *dest4 = tranmap[(*dest4<<8)+dc_colormap[dc_source[frac>>FRACBITS]]];
            dest3 += SCREENWIDTH << hires;
            dest4 += SCREENWIDTH << hires;
        }

        frac += fracstep;
    } while (count--);
}


// -----------------------------------------------------------------------------
// R_InitTranslationTables
// Creates the translation tables to map
//  the green color ramp to gray, brown, red.
// Assumes a given structure of the PLAYPAL.
// Could be read from a lump instead.
// -----------------------------------------------------------------------------

void R_InitTranslationTables (void)
{
    int i;

    // [Julia] Load a tint map for shadowed text
    V_LoadTintMap();

    translationtables = Z_Malloc (256*3, PU_STATIC, 0);

    // translate just the 16 green colors
    for (i=0 ; i<256 ; i++)
    {
        if (i >= 0x70 && i<= 0x7f)
        {
            // map green ramp to gray, brown, red
            translationtables[i] = 0x60 + (i&0xf);
            translationtables [i+256] = 0x40 + (i&0xf);
            translationtables [i+512] = 0x20 + (i&0xf);
        }
        else
        {
            // Keep all other colors as is.
            translationtables[i] = translationtables[i+256] 
            = translationtables[i+512] = i;
        }
    }
}


// -----------------------------------------------------------------------------
// R_DrawSpan 
// With DOOM style restrictions on view orientation,
//  the floors and ceilings consist of horizontal slices
//  or spans with constant z depth.
// However, rotation around the world z axis is possible,
//  thus this mapping, while simpler and faster than
//  perspective correct texture mapping, has to traverse
//  the texture at an angle in all but a few cases.
// In consequence, flats are not stored by column (like walls),
//  and the inner loop has to step in texture space u and v.
// -----------------------------------------------------------------------------

int ds_y; 
int ds_x1; 
int ds_x2;

lighttable_t *ds_colormap; 

fixed_t ds_xfrac; 
fixed_t ds_yfrac; 
fixed_t ds_xstep; 
fixed_t ds_ystep;

byte *ds_source;    // start of a 64*64 tile image 
int dscount;        // just for profiling


// -----------------------------------------------------------------------------
// R_DrawSpan
//
// Draws the actual span.
// -----------------------------------------------------------------------------

void R_DrawSpan (void) 
{ 
    // unsigned int position, step;
    byte        *dest;
    int          count;
    int          spot;
    unsigned int xtemp, ytemp;

#ifdef RANGECHECK
    if (ds_x2 < ds_x1 || ds_x1<0 || ds_x2>=SCREENWIDTH || (unsigned)ds_y>SCREENHEIGHT)
    {
        I_Error("R_DrawSpan: %i to %i at %i", ds_x1,ds_x2,ds_y);
    }
#endif

    // Pack position and step variables into a single 32-bit integer,
    // with x in the top 16 bits and y in the bottom 16 bits.  For
    // each 16-bit part, the top 6 bits are the integer part and the
    // bottom 10 bits are the fractional part of the pixel position.
    dest = ylookup[ds_y] + columnofs[ds_x1];

    // We do not check for zero spans here?
    count = ds_x2 - ds_x1;

    do
    {
        // Calculate current texture index in u,v.
        // [crispy] fix flats getting more distorted the closer they are to the right
        ytemp = (ds_yfrac >> 10) & 0x0fc0;
        xtemp = (ds_xfrac >> 16) & 0x3f;
        spot = xtemp | ytemp;

        // Lookup pixel from flat texture tile,
        //  re-index using light/colormap.
        *dest++ = ds_colormap[ds_source[spot]];

        // position += step;
        ds_xfrac += ds_xstep;
        ds_yfrac += ds_ystep;
    } while (count--);
}


// -----------------------------------------------------------------------------
// R_DrawSpanLow
//
// Again..
// -----------------------------------------------------------------------------
void R_DrawSpanLow (void)
{
    unsigned int  xtemp, ytemp;
    byte         *dest, *dest2;
    int           count;
    int           spot;

#ifdef RANGECHECK
    if (ds_x2 < ds_x1 || ds_x1<0 || ds_x2>=SCREENWIDTH || (unsigned)ds_y>SCREENHEIGHT)
    {
        I_Error("R_DrawSpan: %i to %i at %i", ds_x1,ds_x2,ds_y);
    }
#endif

    count = (ds_x2 - ds_x1);

    // Blocky mode, need to multiply by 2.
    ds_x1 <<= 1;
    ds_x2 <<= 1;

    dest  = ylookup[(ds_y << hires)] + columnofs[ds_x1];
    dest2 = ylookup[(ds_y << hires) + 1] + columnofs[ds_x1];

    do
    {
        // Calculate current texture index in u,v.
        // [crispy] fix flats getting more distorted the closer they are to the right
        ytemp = (ds_yfrac >> 10) & 0x0fc0;
        xtemp = (ds_xfrac >> 16) & 0x3f;
        spot = xtemp | ytemp;

        // Lowres/blocky mode does it twice,
        //  while scale is adjusted appropriately.
        *dest++ = ds_colormap[ds_source[spot]];
        *dest++ = ds_colormap[ds_source[spot]];
        if (hires)
        {
            *dest2++ = ds_colormap[ds_source[spot]];
            *dest2++ = ds_colormap[ds_source[spot]];
        }

    ds_xfrac += ds_xstep;
    ds_yfrac += ds_ystep;

    } while (count--);
}


// -----------------------------------------------------------------------------
// R_InitBuffer 
// Creats lookup tables that avoid
//  multiplies and other hazzles
//  for getting the framebuffer address
//  of a pixel to draw.
// -----------------------------------------------------------------------------

void R_InitBuffer (int width, int height) 
{ 
    int i; 

    // Handle resize,
    //  e.g. smaller view windows
    //  with border and/or status bar.
    viewwindowx = (SCREENWIDTH-width) >> 1; 

    // Column offset. For windows.
    for (i=0 ; i<width ; i++) 
    columnofs[i] = viewwindowx + i;

    // Same with base row offset.
    if (width == SCREENWIDTH) 
    viewwindowy = 0; 
    else 
    viewwindowy = (SCREENHEIGHT-SBARHEIGHT-height) >> 1; 

    // Preclaculate all row offsets.
    for (i=0 ; i<height ; i++) 
    ylookup[i] = I_VideoBuffer + (i+viewwindowy)*SCREENWIDTH; 
}


// -----------------------------------------------------------------------------
// R_FillBackScreen
// Fills the back screen with a pattern
//  for variable screen sizes
// Also draws a beveled edge.
// -----------------------------------------------------------------------------
void R_FillBackScreen (void) 
{ 
    byte     *src;
    byte     *dest; 
    int       x;
    int       y; 
    patch_t  *patch;

    // [Julia] Jaguar border patch.
    char *name = DEH_String("FLOOR7_1");

    // If we are running full screen, there is no need to do any of this,
    // and the background buffer can be freed if it was previously in use.
    if (scaledviewwidth == SCREENWIDTH)
    {
        if (background_buffer != NULL)
        {
            Z_Free(background_buffer);
            background_buffer = NULL;
        }

        return;
    }

    // Allocate the background buffer if necessary
    if (background_buffer == NULL)
    {
        background_buffer = Z_Malloc(SCREENWIDTH * (SCREENHEIGHT - SBARHEIGHT) 
                          * sizeof(*background_buffer), PU_STATIC, NULL);
    }

    src = W_CacheLumpName(name, PU_CACHE); 
    dest = background_buffer;

    for (y=0 ; y<SCREENHEIGHT-SBARHEIGHT ; y++) 
    { 
        for (x=0 ; x<SCREENWIDTH/64 ; x++) 
        { 
            memcpy (dest, src+((y&63)<<6), 64); 
            dest += 64; 
        } 

        if (SCREENWIDTH&63) 
        { 
            memcpy (dest, src+((y&63)<<6), SCREENWIDTH&63); 
            dest += (SCREENWIDTH&63); 
        } 
    } 

    // Draw screen and bezel; this is done to a separate screen buffer.

    V_UseBuffer(background_buffer);

    patch = W_CacheLumpName(DEH_String("brdr_t"),PU_CACHE);

    for (x=0 ; x<(scaledviewwidth >> hires) ; x+=8)
    V_DrawPatch((viewwindowx >> hires)+x, (viewwindowy >> hires)-8, patch);
    patch = W_CacheLumpName(DEH_String("brdr_b"),PU_CACHE);

    for (x=0 ; x<(scaledviewwidth >> hires) ; x+=8)
    V_DrawPatch((viewwindowx >> hires)+x, (viewwindowy >> hires)+(scaledviewheight >> hires), patch);
    patch = W_CacheLumpName(DEH_String("brdr_l"),PU_CACHE);

    for (y=0 ; y<(scaledviewheight >> hires) ; y+=8)
    V_DrawPatch((viewwindowx >> hires)-8, (viewwindowy >> hires)+y, patch);
    patch = W_CacheLumpName(DEH_String("brdr_r"),PU_CACHE);

    for (y=0 ; y<(scaledviewheight >> hires); y+=8)
    V_DrawPatch((viewwindowx >> hires)+(scaledviewwidth >> hires), (viewwindowy >> hires)+y, patch);

    // Draw beveled edge. 
    V_DrawPatch((viewwindowx >> hires)-8,
                (viewwindowy >> hires)-8,
                W_CacheLumpName(DEH_String("brdr_tl"),PU_CACHE));

    V_DrawPatch((viewwindowx >> hires)+(scaledviewwidth >> hires),
                (viewwindowy >> hires)-8,
                W_CacheLumpName(DEH_String("brdr_tr"),PU_CACHE));

    V_DrawPatch((viewwindowx >> hires)-8,
                (viewwindowy >> hires)+(scaledviewheight >> hires),
                W_CacheLumpName(DEH_String("brdr_bl"),PU_CACHE));

    V_DrawPatch((viewwindowx >> hires)+(scaledviewwidth >> hires),
                (viewwindowy >> hires)+(scaledviewheight >> hires),
                W_CacheLumpName(DEH_String("brdr_br"),PU_CACHE));

    V_RestoreBuffer();
}


// -----------------------------------------------------------------------------
// R_VideoErase
//
// Copy a screen buffer.
// -----------------------------------------------------------------------------
void R_VideoErase (unsigned ofs, int count)
{ 
    // LFB copy.
    // This might not be a good idea if memcpy
    //  is not optiomal, e.g. byte by byte on
    //  a 32bit CPU, as GNU GCC/Linux libc did
    //  at one point.

    if (background_buffer != NULL)
    {
        memcpy(I_VideoBuffer + ofs, background_buffer + ofs, count * sizeof(*I_VideoBuffer));
    }
}


// -----------------------------------------------------------------------------
// R_DrawViewBorder
//
// R_DrawViewBorder
// Draws the border around the view
//  for different size windows?
// -----------------------------------------------------------------------------
void R_DrawViewBorder (void) 
{ 
    int	    top;
    int	    side;
    int	    ofs;
    int	    i; 

    if (scaledviewwidth == SCREENWIDTH) 
    return; 

    top = ((SCREENHEIGHT-SBARHEIGHT)-scaledviewheight)/2; 
    side = (SCREENWIDTH-scaledviewwidth)/2; 

    // copy top and one line of left side 
    R_VideoErase (0, top*SCREENWIDTH+side); 

    // copy one line of right side and bottom 
    ofs = (scaledviewheight+top)*SCREENWIDTH-side;
    R_VideoErase (ofs, top*SCREENWIDTH+side); 

    // copy sides using wraparound 
    ofs = top*SCREENWIDTH + SCREENWIDTH-side; 
    side <<= 1;

    for (i=1 ; i<scaledviewheight ; i++) 
    { 
        R_VideoErase (ofs, side); 
        ofs += SCREENWIDTH; 
    } 

    // ? 
    V_MarkRect (0,0,SCREENWIDTH, SCREENHEIGHT-SBARHEIGHT); 
}

