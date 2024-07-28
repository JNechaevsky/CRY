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


#include <stdlib.h>
#include <string.h>
#include "z_zone.h"
#include "i_video.h"
#include "v_trans.h" // [crispy] blending functions
#include "v_video.h"
#include "m_random.h"
#include "st_bar.h"

#include "id_vars.h"


// =============================================================================
// SCREEN WIPE PACKAGE
// =============================================================================

static pixel_t *wipe_scr_start;
static pixel_t *wipe_scr_end;
static pixel_t *wipe_scr;
static int     *y;

// [JN] Function pointers to different wipe effects.
static void (*wipe_init) (void);
static boolean (*wipe_do) (int ticks);

// [crispy] Additional fail-safe counter for performing crossfade effect.
static int fade_counter;

// -----------------------------------------------------------------------------
// wipe_shittyColMajorXform
// -----------------------------------------------------------------------------

static void wipe_shittyColMajorXform (dpixel_t *array)
{
    const int width = SCREENWIDTH/2;
    dpixel_t *dest = (dpixel_t*) malloc(width*SCREENHEIGHT*sizeof(*dest));

    for (int y = 0 ; y < SCREENHEIGHT ; y++)
    {
        for (int x = 0 ; x < width ; x++)
        {
            dest[x*SCREENHEIGHT+y] = array[y*width+x];
        }
    }

    memcpy(array, dest, width*SCREENHEIGHT*sizeof(*dest));

    free(dest);
}


// =============================================================================
// Loading
// =============================================================================

static void wipe_initLoading (void)
{
    // copy start screen to main screen
    memcpy(wipe_scr, wipe_scr_start, SCREENWIDTH*SCREENHEIGHT*sizeof(*wipe_scr));

    // makes this wipe faster (in theory)
    // to have stuff in column-major format
    wipe_shittyColMajorXform((dpixel_t*)wipe_scr_start);
    wipe_shittyColMajorXform((dpixel_t*)wipe_scr_end);

    // setup initial column positions
    // (y<0 => not ready to scroll yet)
    y = (int *) malloc(SCREENWIDTH*sizeof(int));
    y[0] = -1;

    for (int i = 1 ; i < SCREENWIDTH ; i++)
    {
        y[i] = y[i-1] + 2;
    }
}

static boolean wipe_doLoading (int ticks)
{
    const int delay = 12 * vid_resolution;
    const int width = SCREENWIDTH/2;

    boolean	done = true;

    while (ticks--)
    {
        for (int i = 0 ; i < width ; i++)
        {
            if (y[i] < SCREENHEIGHT)
            {
                y[i] += delay;
                done = false;
            }
        }
    }

    V_DrawPatch(132, 72, W_CacheLumpName("LOADING", PU_CACHE));

    return done;
}

// =============================================================================
// Melt
// =============================================================================

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
    y = (int *) malloc(SCREENWIDTH*sizeof(int));
    y[0] = -(ID_RealRandom()%16);

    for (int i = 1 ; i < SCREENWIDTH ; i++)
    {
        const int r = (ID_RealRandom()%3) - 1;

        y[i] = y[i-1] + r;

        if (y[i] > 0)
        {
            y[i] = 0;
        }
        else
        if (y[i] == -16)
        {
            y[i] = -15;
        }
    }
}

static boolean wipe_doMelt (int ticks)
{
    int j;
    int dy;
    int idx;
    const int width = SCREENWIDTH/2;

    dpixel_t *s;
    dpixel_t *d;
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
                dy = (y[i] < 16) ? y[i]+1 : (8 * vid_resolution);

                if (y[i]+dy >= SCREENHEIGHT)
                {
                    dy = SCREENHEIGHT - y[i];
                }

                s = &((dpixel_t *)wipe_scr_end)[i*SCREENHEIGHT+y[i]];
                d = &((dpixel_t *)wipe_scr)[y[i]*width+i];
                idx = 0;

                for (j = dy ; j ; j--)
                {
                    d[idx] = *(s++);
                    idx += width;
                }

                y[i] += dy;
                s = &((dpixel_t *)wipe_scr_start)[i*SCREENHEIGHT];
                d = &((dpixel_t *)wipe_scr)[y[i]*width+i];
                idx = 0;

                for (j=SCREENHEIGHT-y[i];j;j--)
                {
                    d[idx] = *(s++);
                    idx += width;
                }

                done = false;
            }
        }
    }

    return done;
}

// =============================================================================
// Crossfade
// =============================================================================

static const uint8_t alpha_table[] = {
      0,   8,  16,  24,  32,  40,  48,  56,
     64,  72,  80,  88,  96, 104, 112, 120,
    128, 136, 144, 152, 160, 168, 176, 184,
    192, 200, 208, 216, 224, 232, 240, 248,
};

static void wipe_initCrossfade (void)
{
    y = (int *) malloc(SCREENWIDTH*sizeof(int));
    memcpy(wipe_scr, wipe_scr_start, SCREENWIDTH*SCREENHEIGHT*sizeof(*wipe_scr));
    // [JN] Arm fail-safe crossfade counter with...
    // 32 screen screen transitions in TrueColor render,
    // to keep effect smooth enough.
    fade_counter = 32;
}

static boolean wipe_doCrossfade (int ticks)
{
    pixel_t   *cur_screen = wipe_scr;
    pixel_t   *end_screen = wipe_scr_end;
    const int  pix = SCREENWIDTH*SCREENHEIGHT;
    boolean changed = false;

    // [crispy] reduce fail-safe crossfade counter tics
    if (--fade_counter > 0)
    {
        // [JN] Keep solid background to prevent blending with empty space.
        V_DrawBlock(0, 0, SCREENWIDTH, SCREENHEIGHT, wipe_scr_start);

        for (int i = pix; i > 0; i--)
        {
            if (*cur_screen != *end_screen && fade_counter)
            {
                changed = true;
                *cur_screen = I_BlendOver(*end_screen, *cur_screen, alpha_table[fade_counter]);
            }
            ++cur_screen;
            ++end_screen;
        }
    }

    return !changed;
}


// -----------------------------------------------------------------------------
// wipe_exitMelt
// -----------------------------------------------------------------------------

static void wipe_exit (void)
{
    free(y);
    free(wipe_scr_start);
    free(wipe_scr_end);
    // [JN] Refresh status bar background after loading is finished.
    st_fullupdate = true;
}

// -----------------------------------------------------------------------------
// wipe_StartScreen
// -----------------------------------------------------------------------------

void wipe_StartScreen (void)
{
    wipe_scr_start = malloc(SCREENWIDTH * SCREENHEIGHT * sizeof(*wipe_scr_start));
    I_ReadScreen(wipe_scr_start);
}

// -----------------------------------------------------------------------------
// wipe_EndScreen
// -----------------------------------------------------------------------------

void wipe_EndScreen (void)
{
    wipe_scr_end = malloc(SCREENWIDTH * SCREENHEIGHT * sizeof(*wipe_scr_end));
    I_ReadScreen(wipe_scr_end);
    V_DrawBlock(0, 0, SCREENWIDTH, SCREENHEIGHT, wipe_scr_start); // restore start scr.
}

// -----------------------------------------------------------------------------
// wipe_ScreenWipe
// -----------------------------------------------------------------------------

boolean wipe_ScreenWipe (const int ticks)
{
    // when zero, stop the wipe
    static boolean go = false;

    // [JN] Initialize function pointers for different wipe effects.
    switch (vid_screenwipe)
    {
        case 1: // Loading
            wipe_init = wipe_initLoading;
            wipe_do = wipe_doLoading;
        break;
        case 2: // Melt
            wipe_init = wipe_initMelt;
            wipe_do = wipe_doMelt;
        break;
        case 3: // Crossfade
            wipe_init = wipe_initCrossfade;
            wipe_do = wipe_doCrossfade;
        break;
    }

    // initial stuff
    if (!go)
    {
        go = true;
        wipe_scr = I_VideoBuffer;
        wipe_init();
    }

    // final stuff
    if ((*wipe_do)(ticks))
    {
        go = false;
        wipe_exit();
    }

    return !go;
}
