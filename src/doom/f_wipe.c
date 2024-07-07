//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2024 Julia Nechaevskaya
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


#include <string.h>
#include "z_zone.h"
#include "i_video.h"
#include "v_video.h"
#include "st_bar.h"

#include "id_vars.h"


// =============================================================================
// SCREEN WIPE PACKAGE
// =============================================================================

static pixel_t *wipe_scr_start;
static pixel_t *wipe_scr_end;
static pixel_t *wipe_scr;
static int     *y;

// -----------------------------------------------------------------------------
// wipe_shittyColMajorXform
// -----------------------------------------------------------------------------

static void wipe_shittyColMajorXform (dpixel_t *array)
{
    const int width = SCREENWIDTH/2;
    dpixel_t *dest = (dpixel_t*) Z_Malloc(width*SCREENHEIGHT*sizeof(*dest), PU_STATIC, 0);

    for (int y = 0 ; y < SCREENHEIGHT ; y++)
    {
        for (int x = 0 ; x < width ; x++)
        {
            dest[x*SCREENHEIGHT+y] = array[y*width+x];
        }
    }

    memcpy(array, dest, width*SCREENHEIGHT*sizeof(*dest));

    Z_Free(dest);
}

// -----------------------------------------------------------------------------
// wipe_initMelt
// -----------------------------------------------------------------------------

static void wipe_initMelt (void)
{
    // copy start screen to main screen
    memcpy(wipe_scr, wipe_scr_start, SCREENWIDTH*SCREENHEIGHT*sizeof(*wipe_scr));

    // makes this wipe faster (in theory)
    // to have stuff in column-major format
    wipe_shittyColMajorXform((dpixel_t*)wipe_scr_start);
    wipe_shittyColMajorXform((dpixel_t*)wipe_scr_end);

    // setup initial column positions
    // (y<0 => not ready to scroll yet)
    y = (int *) Z_Malloc(SCREENWIDTH*sizeof(int), PU_STATIC, 0);
    y[0] = -1;

    for (int i = 1 ; i < SCREENWIDTH ; i++)
    {
        y[i] = y[i-1] + 2;
    }
}

// -----------------------------------------------------------------------------
// wipe_doMelt
// -----------------------------------------------------------------------------

static int wipe_doMelt (int ticks)
{
    const int delay = vid_screenwipe == 2 ? 32 : 12; // 32 = fast, 12 = slow
    const int width = SCREENWIDTH/2;

    boolean	done = true;

    while (ticks--)
    {
        for (int i = 0 ; i < width ; i++)
        {
            if (y[i]<0)
            {
                y[i]++; done = false;
            }
            else
            if (y[i] < SCREENHEIGHT)
            {
                y[i] += delay;
                done = false;
            }
        }
    }

    return done;
}

// -----------------------------------------------------------------------------
// wipe_exitMelt
// -----------------------------------------------------------------------------

static void wipe_exitMelt (void)
{
    Z_Free(y);
    Z_Free(wipe_scr_start);
    Z_Free(wipe_scr_end);
    // [JN] Refresh status bar background after loading is finished.
    st_fullupdate = true;
}

// -----------------------------------------------------------------------------
// wipe_StartScreen
// -----------------------------------------------------------------------------

void wipe_StartScreen (void)
{
    wipe_scr_start = Z_Malloc(SCREENWIDTH * SCREENHEIGHT * sizeof(*wipe_scr_start), PU_STATIC, NULL);
    I_ReadScreen(wipe_scr_start);
}

// -----------------------------------------------------------------------------
// wipe_EndScreen
// -----------------------------------------------------------------------------

void wipe_EndScreen (void)
{
    wipe_scr_end = Z_Malloc(SCREENWIDTH * SCREENHEIGHT * sizeof(*wipe_scr_end), PU_STATIC, NULL);
    I_ReadScreen(wipe_scr_end);
    V_DrawBlock(0, 0, SCREENWIDTH, SCREENHEIGHT, wipe_scr_start); // restore start scr.
}

// -----------------------------------------------------------------------------
// wipe_ScreenWipe
// -----------------------------------------------------------------------------

const int wipe_ScreenWipe (const int ticks)
{
    // when zero, stop the wipe
    static boolean go = false;

    // initial stuff
    if (!go)
    {
        go = true;
        wipe_scr = I_VideoBuffer;
        wipe_initMelt();
    }

    // do a piece of wipe-in
    V_MarkRect(0, 0, SCREENWIDTH, SCREENHEIGHT);

    // final stuff
    if ((*wipe_doMelt)(ticks))
    {
        go = false;
        wipe_exitMelt();
    }

    if (vid_screenwipe)
    {
        V_DrawPatch(132, 72, W_CacheLumpName("LOADING", PU_CACHE));
    }

    return !go;
}
