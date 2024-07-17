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

#include <stdio.h>

#include "am_map.h"
#include "ct_chat.h"
#include "doomstat.h"
#include "d_englsh.h"
#include "g_game.h"
#include "i_system.h"
#include "m_controls.h"
#include "m_menu.h"
#include "m_misc.h"
#include "p_local.h"
#include "st_bar.h"
#include "v_trans.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

#include "id_vars.h"


// drawing stuff
#define AM_NUMMARKPOINTS 10

// [JN] FRACTOMAPBITS: overflow-safe coordinate system.
// Written by Andrey Budko (entryway), adapted from prboom-plus/src/am_map.*
#define MAPBITS 12
#define FRACTOMAPBITS (FRACBITS-MAPBITS)

// scale on entry
#define INITSCALEMTOF (.2*FRACUNIT)

// [JN] How much the automap moves window per tic in frame-buffer coordinates.
static int f_paninc;
#define F_PANINC_SLOW 4  // 140 map units in 1 second.
#define F_PANINC_FAST 8  // 280 map units in 1 second.
static int f_paninc_zoom;
#define F_PANINC_ZOOM_SLOW 8   // 280 map units in 1 second.
#define F_PANINC_ZOOM_FAST 16  // 560 map units in 1 second.

// [JN] How much zoom-in per tic goes to 2x in 1 second.
static int m_zoomin;
#define M_ZOOMIN_SLOW ((int) (1.04*FRACUNIT))
#define M_ZOOMIN_FAST ((int) (1.08*FRACUNIT))

// [JN] How much zoom-out per tic pulls out to 0.5x in 1 second.
static int m_zoomout;
#define M_ZOOMOUT_SLOW ((int) (FRACUNIT/1.04))
#define M_ZOOMOUT_FAST ((int) (FRACUNIT/1.08))

// [crispy] zoom faster with the mouse wheel
#define M2_ZOOMIN_SLOW  ((int) (1.08*FRACUNIT))
#define M2_ZOOMOUT_SLOW ((int) (FRACUNIT/1.08))
#define M2_ZOOMIN_FAST  ((int) (1.5*FRACUNIT))
#define M2_ZOOMOUT_FAST ((int) (FRACUNIT/1.5))
static int m_zoomin_mouse;
static int m_zoomout_mouse;
static boolean mousewheelzoom;

// translates between frame-buffer and map distances
#define FTOM(x) (((int64_t)((x)<<16) * scale_ftom) >> FRACBITS)
#define MTOF(x) ((((int64_t)(x) * scale_mtof) >> FRACBITS)>>16)

// translates between frame-buffer and map coordinates
#define CXMTOF(x) (f_x + MTOF((x)-m_x))
#define CYMTOF(y) (f_y + (f_h - MTOF((y)-m_y)))

// [JN] Jaguar: emulate blinking arrow.
static int blinking_arrow;

// [JN] ReMood-inspired IDDT monster coloring, slightly optimized
// for uncapped framerate and uses different coloring logics:
// Active monsters: up-up-up-up
// Inactive monsters: up-down-up-down
#define IDDT_REDS_RANGE (10)
#define IDDT_REDS_MIN   (176)
#define IDDT_REDS_MAX   (176+ IDDT_REDS_RANGE)
static  int     iddt_reds_active;
static  int     iddt_reds_inactive = 176;
static  boolean iddt_reds_direction = false;
// [JN] Pulse player arrow in Spectator mode.
#define ARROW_WHITE_RANGE (10)
#define ARROW_WHITE_MIN   (80)
#define ARROW_WHITE_MAX   (96)
static  int     arrow_color = 80;
static  boolean arrow_color_direction = false;

typedef struct
{
    int x, y;
} fpoint_t;

typedef struct
{
    fpoint_t a, b;
} fline_t;

typedef struct
{
    mpoint_t a, b;
} mline_t;

#define M_ARRAY_INIT_CAPACITY 500
#include "m_array.h"

typedef struct
{
    mline_t l;
    int color;
} am_line_t;

static am_line_t *lines_1S = NULL;

// -----------------------------------------------------------------------------
// The vector graphics for the automap.
// A line drawing of the player pointing right, starting from the middle.
// -----------------------------------------------------------------------------

#define R (FRACUNIT)
static mline_t thintriangle_guy[] = {
    { { (fixed_t)(-.5*R), (fixed_t)(-.7*R) }, { (fixed_t)(R    ), (fixed_t)(0    ) } },
    { { (fixed_t)(R    ), (fixed_t)(0    ) }, { (fixed_t)(-.5*R), (fixed_t)(.7*R ) } },
    { { (fixed_t)(-.5*R), (fixed_t)(.7*R ) }, { (fixed_t)(-.5*R), (fixed_t)(-.7*R) } }
};
#undef R


boolean automapactive = false;

int iddt_cheating = 0;
static boolean grid = false;

// location of window on screen
static int  f_x;
static int  f_y;

// size of window on screen
static int  f_w;
static int  f_h;

static mpoint_t m_paninc;     // how far the window pans each tic (map coords)
static fixed_t  mtof_zoommul; // how far the window zooms in each tic (map coords)
static fixed_t  ftom_zoommul; // how far the window zooms in each tic (fb coords)
static fixed_t  curr_mtof_zoommul; // [JN] Zooming interpolation.

static int64_t  m_x, m_y;     // LL x,y where the window is on the map (map coords)
static int64_t  m_x2, m_y2;   // UR x,y where the window is on the map (map coords)
static fixed_t  prev_m_x, prev_m_y; // [JN] Panning interpolation.

// width/height of window on map (map coords)
static int64_t  m_w;
static int64_t  m_h;

// based on level size
static fixed_t  min_x;
static fixed_t  min_y; 
static fixed_t  max_x;
static fixed_t  max_y;

static fixed_t  max_w; // max_x-min_x,
static fixed_t  max_h; // max_y-min_y

static fixed_t  min_scale_mtof; // used to tell when to stop zooming out
static fixed_t  max_scale_mtof; // used to tell when to stop zooming in

// old stuff for recovery later
static int64_t old_m_w, old_m_h;
static int64_t old_m_x, old_m_y;

// used by MTOF to scale from map-to-frame-buffer coords
static fixed_t scale_mtof = (fixed_t)INITSCALEMTOF;
static fixed_t prev_scale_mtof = (fixed_t)INITSCALEMTOF; // [JN] Panning interpolation.
// used by FTOM to scale from frame-buffer-to-map coords (=1/scale_mtof)
static fixed_t scale_ftom;

static player_t *plr; // the player represented by an arrow

static patch_t *marknums[10]; // numbers used for marking by the automap
// [JN] killough 2/22/98: Remove limit on automap marks,
// and make variables external for use in savegames.
mpoint_t *markpoints = NULL;     // where the points are
int       markpointnum = 0;      // next point to be assigned (also number of points now)
int       markpointnum_max = 0;  // killough 2/22/98

static int followplayer = 1; // specifies whether to follow the player around

static boolean stopped = true;

// [crispy] Antialiased lines from Heretic with more colors
#define NUMSHADES 8
#define NUMSHADES_BITS 3 // log2(NUMSHADES)
static pixel_t color_shades[NUMSHADES * 256];

// Forward declare for AM_LevelInit
static void AM_drawFline_Vanilla(fline_t* fl, int color);
static void AM_drawFline_Smooth(fline_t* fl, int color);
// Indirect through this to avoid having to test crispy->smoothmap for every line
void (*AM_drawFline)(fline_t*, int) = AM_drawFline_Vanilla;

// [crispy] automap rotate mode needs these early on
static void AM_rotate (int64_t *x, int64_t *y, angle_t a);
static void AM_rotatePoint (mpoint_t *pt);
static mpoint_t mapcenter;
static angle_t mapangle;

static void AM_drawCrosshair(boolean force);

// -----------------------------------------------------------------------------
// AM_Init
// [JN] Predefine some variables at program startup.
// -----------------------------------------------------------------------------

void AM_Init (void)
{
    char namebuf[9];

    for (int i = 0 ; i < 10 ; i++)
    {
        snprintf(namebuf, 9, "MICRO_%d", i);
        marknums[i] = W_CacheLumpName(namebuf, PU_STATIC);
    }

    for (int color = 0; color < 256; ++color)
    {
#define REINDEX(I) (color + I * 256)
        // Pick a range of shades for a steep gradient to keep lines thin
        const int shade_index[NUMSHADES] =
        {
            REINDEX(0), REINDEX(1), REINDEX(2), REINDEX(3), REINDEX(5), REINDEX(7), REINDEX(9), REINDEX(11),
        };
#undef REINDEX
        for (int shade = 0; shade < NUMSHADES; ++shade)
        {
            color_shades[color * NUMSHADES + shade] = colormaps[shade_index[shade]];
        }
    }
}

// -----------------------------------------------------------------------------
// AM_activateNewScale
// Changes the map scale after zooming or translating.
// -----------------------------------------------------------------------------

static void AM_activateNewScale (void)
{
    m_x += m_w/2;
    m_y += m_h/2;
    m_w  = FTOM(f_w);
    m_h  = FTOM(f_h);
    m_x -= m_w/2;
    m_y -= m_h/2;
    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}

// -----------------------------------------------------------------------------
// AM_saveScaleAndLoc
// Saves the current center and zoom.
// Affects the variables that remember old scale and loc.
// -----------------------------------------------------------------------------

static void AM_saveScaleAndLoc (void)
{
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;
}

// -----------------------------------------------------------------------------
// AM_restoreScaleAndLoc
// Restores the center and zoom from locally saved values.
// Affects global variables for location and scale.
// -----------------------------------------------------------------------------

static void AM_restoreScaleAndLoc (void)
{
    m_w = old_m_w;
    m_h = old_m_h;

    if (!followplayer)
    {
        m_x = old_m_x;
        m_y = old_m_y;
    }
    else 
    {
        m_x = (plr->mo->x >> FRACTOMAPBITS) - m_w/2;
        m_y = (plr->mo->y >> FRACTOMAPBITS) - m_h/2;
    }

    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;

    // Change the scaling multipliers
    scale_mtof = FixedDiv(f_w<<FRACBITS, m_w);
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
}

// -----------------------------------------------------------------------------
// AM_addMark
// Adds a marker at the current location.
// -----------------------------------------------------------------------------

static void AM_addMark (void)
{
    // [JN] killough 2/22/98: remove limit on automap marks
    if (markpointnum >= markpointnum_max)
    {
        markpoints = I_Realloc(markpoints,
                              (markpointnum_max = markpointnum_max ? 
                               markpointnum_max*2 : 16) * sizeof(*markpoints));
    }

    // [crispy] keep the map static in overlay mode if not following the player
    if (!followplayer)
    {
        markpoints[markpointnum].x = m_x + m_w/2;
        markpoints[markpointnum].y = m_y + m_h/2;
    }
    else
    {
        markpoints[markpointnum].x = plr->mo->x >> FRACTOMAPBITS;
        markpoints[markpointnum].y = plr->mo->y >> FRACTOMAPBITS;
    }
    markpointnum++;
}

// -----------------------------------------------------------------------------
// AM_findMinMaxBoundaries
// Determines bounding box of all vertices, 
// sets global variables controlling zoom range.
// -----------------------------------------------------------------------------

static void AM_findMinMaxBoundaries (void)
{
    int     i;
    fixed_t a, b;

    min_x = min_y =  INT_MAX;
    max_x = max_y = -INT_MAX;

    for (i = 0 ; i < numvertexes ; i++)
    {
        if (vertexes[i].x < min_x)
        {
            min_x = vertexes[i].x;
        }
        else if (vertexes[i].x > max_x)
        {
            max_x = vertexes[i].x;
        }

        if (vertexes[i].y < min_y)
        {
            min_y = vertexes[i].y;
        }
        else if (vertexes[i].y > max_y)
        {
            max_y = vertexes[i].y;
        }
    }

    // [crispy] cope with huge level dimensions which span the entire INT range
    max_w = (max_x >>= FRACTOMAPBITS) - (min_x >>= FRACTOMAPBITS);
    max_h = (max_y >>= FRACTOMAPBITS) - (min_y >>= FRACTOMAPBITS);

    a = FixedDiv(f_w<<FRACBITS, max_w);
    b = FixedDiv(f_h<<FRACBITS, max_h);

    min_scale_mtof = a < b ? a : b;
    max_scale_mtof = FixedDiv(f_h<<FRACBITS, 2*FRACUNIT);
}

// -----------------------------------------------------------------------------
// AM_changeWindowLoc
// Moves the map window by the global variables m_paninc.x, m_paninc.y
// -----------------------------------------------------------------------------

static void AM_changeWindowLoc (void)
{
    int64_t incx, incy;

    if (m_paninc.x || m_paninc.y)
    {
        followplayer = 0;
    }

    if (vid_uncapped_fps && realleveltime > oldleveltime)
    {
        incx = FixedMul(m_paninc.x, fractionaltic);
        incy = FixedMul(m_paninc.y, fractionaltic);
    }
    else
    {
        incx = m_paninc.x;
        incy = m_paninc.y;
    }

    if (automap_rotate)
    {
        AM_rotate(&incx, &incy, -mapangle);
    }

    m_x = prev_m_x + incx;
    m_y = prev_m_y + incy;

    if (m_x + m_w/2 > max_x)
    {
        m_x = max_x - m_w/2;
    }
    else if (m_x + m_w/2 < min_x)
    {
        m_x = min_x - m_w/2;
    }

    if (m_y + m_h/2 > max_y)
    {
        m_y = max_y - m_h/2;
    }
    else if (m_y + m_h/2 < min_y)
    {
        m_y = min_y - m_h/2;
    }

    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}

// -----------------------------------------------------------------------------
// AM_initVariables
// -----------------------------------------------------------------------------

void AM_initVariables (void)
{
    automapactive = true;

    m_paninc.x = m_paninc.y = 0;
    ftom_zoommul = FRACUNIT;
    mtof_zoommul = FRACUNIT;
    mousewheelzoom = false; // [crispy]

    m_w = FTOM(f_w);
    m_h = FTOM(f_h);

    // [JN] Find player to center.
    plr = &players[displayplayer];

    m_x = (plr->mo->x >> FRACTOMAPBITS) - m_w/2;
    m_y = (plr->mo->y >> FRACTOMAPBITS) - m_h/2;

    AM_Ticker();
    AM_changeWindowLoc();

    // for saving & restoring
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;
}

// -----------------------------------------------------------------------------
// AM_clearMarks
// -----------------------------------------------------------------------------

void AM_clearMarks (void)
{
    markpointnum = 0;
}

// -----------------------------------------------------------------------------
// AM_SetdrawFline 
// -----------------------------------------------------------------------------

void AM_SetdrawFline (void)
{
    AM_drawFline = automap_smooth ? AM_drawFline_Smooth : AM_drawFline_Vanilla;
}    

// -----------------------------------------------------------------------------
// AM_LevelInit
// Should be called at the start of every level.
// Right now, i figure it out myself.
// -----------------------------------------------------------------------------

void AM_LevelInit (boolean reinit)
{
    static int f_h_old;

    f_x = f_y = 0;
    f_w = SCREENWIDTH;
    f_h = SCREENHEIGHT - (ST_HEIGHT * vid_resolution);

    AM_SetdrawFline();

    AM_findMinMaxBoundaries();

    // [crispy] preserve map scale when re-initializing
    if (reinit && f_h_old)
    {
        scale_mtof = scale_mtof * f_h / f_h_old;
        AM_drawCrosshair(true);
    }
    else
    {
        scale_mtof = FixedDiv(min_scale_mtof, (int) (0.7*FRACUNIT));
    }

    if (scale_mtof > max_scale_mtof)
    {
        scale_mtof = min_scale_mtof;
    }

    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);

    f_h_old = f_h;

    // [JN] If running Deathmatch mode, mark all automap lines as mapped
    // so they will appear initially. DM mode is not about map reveal.
    if (deathmatch)
    {
        for (int i = 0 ; i < numlines ; i++)
        {
            lines[i].flags |= ML_MAPPED;
        }
    }
}

// -----------------------------------------------------------------------------
// AM_Stop
// -----------------------------------------------------------------------------

void AM_Stop (void)
{
    automapactive = false;
    stopped = true;
}

// -----------------------------------------------------------------------------
// AM_Start
// -----------------------------------------------------------------------------

void AM_Start (void)
{
    static int lastlevel = -1, lastepisode = -1;

    if (!stopped)
    {
        AM_Stop();
    }

    stopped = false;

    if (lastlevel != gamemap || lastepisode != gameepisode)
    {
        AM_LevelInit(false);
        lastlevel = gamemap;
        lastepisode = gameepisode;
    }

    AM_initVariables();
}

// -----------------------------------------------------------------------------
// AM_minOutWindowScale
// Set the window scale to the maximum size.
// -----------------------------------------------------------------------------

static void AM_minOutWindowScale (void)
{
    scale_mtof = min_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    AM_activateNewScale();
}

// -----------------------------------------------------------------------------
// AM_maxOutWindowScale
// Set the window scale to the minimum size.
// -----------------------------------------------------------------------------

static void AM_maxOutWindowScale (void)
{
    scale_mtof = max_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    AM_activateNewScale();
}

// -----------------------------------------------------------------------------
// AM_Responder
// Handle events (user inputs) in automap mode.
// -----------------------------------------------------------------------------

boolean AM_Responder (event_t *ev)
{
    int         rc;
    static int  bigstate=0;
    static char buffer[20];
    int         key;

    // [JN] If run button is hold, pan/zoom Automap faster.    
    if (speedkeydown())
    {
        f_paninc = F_PANINC_FAST;
        f_paninc_zoom = F_PANINC_ZOOM_FAST;
        m_zoomin = M_ZOOMIN_FAST;
        m_zoomout = M_ZOOMOUT_FAST;
        m_zoomin_mouse = M2_ZOOMIN_FAST;
        m_zoomout_mouse = M2_ZOOMOUT_FAST;
    }
    else
    {
        f_paninc = F_PANINC_SLOW;
        f_paninc_zoom = F_PANINC_ZOOM_SLOW;
        m_zoomin = M_ZOOMIN_SLOW;
        m_zoomout = M_ZOOMOUT_SLOW;
        m_zoomin_mouse = M2_ZOOMIN_SLOW;
        m_zoomout_mouse = M2_ZOOMOUT_SLOW;
    }

    rc = false;

    if (ev->type == ev_joystick && joybautomap >= 0
    && (ev->data1 & (1 << joybautomap)) != 0)
    {
        joywait = I_GetTime() + 5;

        if (!automapactive)
        {
            AM_Start ();
            if (!automap_overlay)
            {
                // [JN] Redraw status bar background.
                st_fullupdate = true;
            }
        }
        else
        {
            bigstate = 0;
            AM_Stop ();
        }

        return true;
    }

    if (!automapactive)
    {
        if (ev->type == ev_keydown && ev->data1 == key_map_toggle)
        {
            AM_Start ();
            if (!automap_overlay)
            {
                // [JN] Redraw status bar background.
                st_fullupdate = true;
            }
            rc = true;
        }
    }
    // [crispy] zoom Automap with the mouse wheel
    // [JN] Mouse wheel "buttons" hardcoded.
    else if (ev->type == ev_mouse && !menuactive)
    {
        if (/*mousebmapzoomout >= 0 &&*/ ev->data1 & (1 << 4 /*mousebmapzoomout*/))
        {
            mtof_zoommul = m_zoomout_mouse;
            ftom_zoommul = m_zoomin_mouse;
            curr_mtof_zoommul = mtof_zoommul;
            mousewheelzoom = true;
            rc = true;
        }
        else
        if (/*mousebmapzoomin >= 0 &&*/ ev->data1 & (1 << 3 /*mousebmapzoomin*/))
        {
            mtof_zoommul = m_zoomin_mouse;
            ftom_zoommul = m_zoomout_mouse;
            curr_mtof_zoommul = mtof_zoommul;
            mousewheelzoom = true;
            rc = true;
        }
    }
    else if (ev->type == ev_keydown)
    {
        rc = true;
        key = ev->data1;

        if (key == key_map_east)          // pan right
        {
            if (!followplayer)
            {
                m_paninc.x = gp_flip_levels ?
                             -FTOM(f_paninc * vid_resolution) : FTOM(f_paninc * vid_resolution);
            }
            else
            {
                rc = false;
            }
        }
        else if (key == key_map_west)     // pan left
        {
            if (!followplayer)
            {
                m_paninc.x = gp_flip_levels ?
                             FTOM(f_paninc * vid_resolution) : -FTOM(f_paninc * vid_resolution);
            }
            else
            {
                rc = false;
            }
        }
        else if (key == key_map_north)    // pan up
        {
            if (!followplayer)
            {
                m_paninc.y = FTOM(f_paninc * vid_resolution);
            }
            else
            {
                rc = false;
            }
        }
        else if (key == key_map_south)    // pan down
        {
            if (!followplayer)
            {
                m_paninc.y = -FTOM(f_paninc * vid_resolution);
            }
            else
            {
                rc = false;
            }
        }
        else if (key == key_map_zoomout)  // zoom out
        {
            mtof_zoommul = m_zoomout;
            ftom_zoommul = m_zoomin;
            curr_mtof_zoommul = mtof_zoommul;
        }
        else if (key == key_map_zoomin)   // zoom in
        {
            mtof_zoommul = m_zoomin;
            ftom_zoommul = m_zoomout;
            curr_mtof_zoommul = mtof_zoommul;
        }
        else if (key == key_map_toggle)   // toggle map (tab)
        {
            bigstate = 0;
            AM_Stop ();
        }
        else if (key == key_map_maxzoom)
        {
            bigstate = !bigstate;

            if (bigstate)
            {
                AM_saveScaleAndLoc();
                AM_minOutWindowScale();
            }
            else
            {
                AM_restoreScaleAndLoc();
            }
        }
        else if (key == key_map_follow)
        {
            followplayer = !followplayer;

            CT_SetMessage(plr, followplayer ? AMSTR_FOLLOWON : AMSTR_FOLLOWOFF, false, NULL);
        }
        else if (key == key_map_grid)
        {
            grid = !grid;

            CT_SetMessage(plr, grid ? AMSTR_GRIDON : AMSTR_GRIDOFF, false, NULL);
        }
        else if (key == key_map_mark)
        {
            M_snprintf(buffer, sizeof(buffer), "%s %d", AMSTR_MARKEDSPOT, markpointnum);
            CT_SetMessage(plr, buffer, false, NULL);
            AM_addMark();
        }
        else if (key == key_map_clearmark && markpointnum > 0)
        {
            // [JN] Clear all mark by holding "run" button and pressing "clear mark".
            if (speedkeydown())
            {
                AM_clearMarks();
                CT_SetMessage(plr, AMSTR_MARKSCLEARED, false, NULL);
            }
            else
            {
                markpointnum--;
                M_snprintf(buffer, sizeof(buffer), "%s %d", AMSTR_MARKCLEARED, markpointnum);
                CT_SetMessage(plr, buffer, false, NULL);
            }
        }
        else if (key == key_map_rotate)
        {
            // [JN] Automap rotate mode.
            automap_rotate = !automap_rotate;
            CT_SetMessage(plr, automap_rotate ? ID_AUTOMAPROTATE_ON : ID_AUTOMAPROTATE_OFF, false, NULL);
        }
        else if (key == key_map_overlay)
        {
            // [JN] Automap overlay mode.
            automap_overlay = !automap_overlay;
            if (automap_overlay)
            {
                CT_SetMessage(plr, ID_AUTOMAPOVERLAY_ON, false, NULL);
            }
            else
            {
                CT_SetMessage(plr, ID_AUTOMAPOVERLAY_OFF, false, NULL);
                // [JN] Redraw status bar background.
                st_fullupdate = true;
            }
        }
        else
        {
            rc = false;
        }
    }
    else if (ev->type == ev_keyup)
    {
        rc = false;
        key = ev->data1;

        if (key == key_map_east)
        {
            if (!followplayer)
            {
                m_paninc.x = 0;
            }
        }
        else if (key == key_map_west)
        {
            if (!followplayer)
            {
                m_paninc.x = 0;
            }
        }
        else if (key == key_map_north)
        {
            if (!followplayer)
            {
                m_paninc.y = 0;
            }
        }
        else if (key == key_map_south)
        {
            if (!followplayer)
            {
                m_paninc.y = 0;
            }
        }
        else if (key == key_map_zoomout || key == key_map_zoomin)
        {
            mtof_zoommul = FRACUNIT;
            ftom_zoommul = FRACUNIT;
        }
    }

    return rc;
}

// -----------------------------------------------------------------------------
// AM_changeWindowScale
// Automap zooming.
// -----------------------------------------------------------------------------

static void AM_changeWindowScale (void)
{
    if (vid_uncapped_fps && realleveltime > oldleveltime)
    {
        float f_paninc_smooth = (float)f_paninc_zoom / (float)FRACUNIT * (float)fractionaltic;

        if (f_paninc_smooth < 0.01f)
        {
            f_paninc_smooth = 0.01f;
        }
    
        scale_mtof = prev_scale_mtof;

        if (curr_mtof_zoommul == m_zoomin)
        {
            mtof_zoommul = ((int) ((float)FRACUNIT * (1.00f + f_paninc_smooth / 200.0f)));
            ftom_zoommul = ((int) ((float)FRACUNIT / (1.00f + f_paninc_smooth / 200.0f)));
        }
        if (curr_mtof_zoommul == m_zoomout)
        {
            mtof_zoommul = ((int) ((float)FRACUNIT / (1.00f + f_paninc_smooth / 200.0f)));
            ftom_zoommul = ((int) ((float)FRACUNIT * (1.00f + f_paninc_smooth / 200.0f)));
        }
    }

    // Change the scaling multipliers
    scale_mtof = FixedMul(scale_mtof, mtof_zoommul);
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);

    // [crispy] reset after zooming with the mouse wheel
    if (mousewheelzoom)
    {
        mtof_zoommul = FRACUNIT;
        ftom_zoommul = FRACUNIT;
        mousewheelzoom = false;
    }

    if (scale_mtof < min_scale_mtof)
    {
        AM_minOutWindowScale();
    }
    else if (scale_mtof > max_scale_mtof)
    {
        AM_maxOutWindowScale();
    }
    else
    {
        AM_activateNewScale();
    }
}

// -----------------------------------------------------------------------------
// AM_doFollowPlayer
// Turn on follow mode - the map scrolls opposite to player motion.
// -----------------------------------------------------------------------------

static void AM_doFollowPlayer (void)
{
    // [JN] Use interpolated player coords for smooth
    // scrolling and static player arrow position.
    // [crispy] FTOM(MTOF()) is needed to fix map line jitter in follow mode.
    if (vid_resolution > 1)
    {
        m_x = (viewx >> FRACTOMAPBITS) - m_w/2;
        m_y = (viewy >> FRACTOMAPBITS) - m_h/2;
    }
    else
    {
        m_x = FTOM(MTOF(viewx >> FRACTOMAPBITS)) - m_w/2;
        m_y = FTOM(MTOF(viewy >> FRACTOMAPBITS)) - m_h/2;
    }

    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}

// -----------------------------------------------------------------------------
// AM_Ticker
// Updates on Game Tick.
// -----------------------------------------------------------------------------

void AM_Ticker (void)
{
    if (!automapactive)
    {
        return;
    }

    prev_scale_mtof = scale_mtof;
    prev_m_x = m_x;
    prev_m_y = m_y;

    // [JN] Jaguar: blinking player arrow:
    if (blinking_arrow++ > 16)
    {
        blinking_arrow = 0;
    }

    // [JN] Animate IDDT monster colors:

    // Inactive:
    if (gametic & 1)
    {
        // Brightening
        if (!iddt_reds_direction && ++iddt_reds_inactive == IDDT_REDS_MAX)
        {
            iddt_reds_direction = true;
        }
        // Darkening
        else
        if (iddt_reds_direction && --iddt_reds_inactive == IDDT_REDS_MIN)
        {
            iddt_reds_direction = false;
        }
    }

    // Active:
    iddt_reds_active = (172) + ((gametic >> 1) % IDDT_REDS_RANGE);

    // [JN] Pulse player arrow in Spectator mode:

    // Brightening
    if (!arrow_color_direction && ++arrow_color == ARROW_WHITE_MAX)
    {
        arrow_color_direction = true;
    }
    // Darkening
    else
    if (arrow_color_direction && --arrow_color == ARROW_WHITE_MIN)
    {
        arrow_color_direction = false;
    }
}

// -----------------------------------------------------------------------------
// AM_clearFB
// Clear automap frame buffer.
// -----------------------------------------------------------------------------

static void AM_clearFB (void)
{
    memset(I_VideoBuffer, 0, f_w*f_h*sizeof(*I_VideoBuffer));
}

// -----------------------------------------------------------------------------
// AM_shadeBackground
//  [JN] Shade background in overlay mode.
// -----------------------------------------------------------------------------

static void AM_shadeBackground (void)
{
    const int height = dp_screen_size > 10 ?
                       SCREENHEIGHT : (SCREENHEIGHT - (ST_HEIGHT * vid_resolution));

        for (int y = 0; y < SCREENWIDTH * height ; y++)
        {
            I_VideoBuffer[y] = I_BlendDark(I_VideoBuffer[y], I_ShadeFactor[automap_shading]);
        }
}

// -----------------------------------------------------------------------------
// AM_clipMline
// Automap clipping of lines.
//
// Based on Cohen-Sutherland clipping algorithm but with a slightly
// faster reject and precalculated slopes.  If the speed is needed,
// use a hash algorithm to handle  the common cases.
// -----------------------------------------------------------------------------

static boolean AM_clipMline (mline_t *ml, fline_t *fl)
{
    enum
    {
        LEFT   = 1,
        RIGHT  = 2,
        BOTTOM = 4,
        TOP	   = 8
    };

    int	outcode1 = 0;
    int	outcode2 = 0;
    int	outside;

    int      dx;
    int      dy;
    fpoint_t tmp = { 0, 0 };

#define DOOUTCODE(oc, mx, my) \
    (oc) = 0; \
    if ((my) < 0) (oc) |= TOP; \
    else if ((my) >= f_h) (oc) |= BOTTOM; \
    if ((mx) < 0) (oc) |= LEFT; \
    else if ((mx) >= f_w) (oc) |= RIGHT;

    // do trivial rejects and outcodes
    if (ml->a.y > m_y2)
    {
        outcode1 = TOP;
    }
    else if (ml->a.y < m_y)
    {
        outcode1 = BOTTOM;
    }

    if (ml->b.y > m_y2)
    {
        outcode2 = TOP;
    }
    else if (ml->b.y < m_y)
    {
        outcode2 = BOTTOM;
    }

    if (outcode1 & outcode2)
    {
        return false; // trivially outside
    }

    if (ml->a.x < m_x)
    {
        outcode1 |= LEFT;
    }
    else if (ml->a.x > m_x2)
    {
        outcode1 |= RIGHT;
    }

    if (ml->b.x < m_x)
    {
        outcode2 |= LEFT;
    }
    else if (ml->b.x > m_x2)
    {
        outcode2 |= RIGHT;
    }

    if (outcode1 & outcode2)
    {
        return false; // trivially outside
    }

    // transform to frame-buffer coordinates.
    fl->a.x = CXMTOF(ml->a.x);
    fl->a.y = CYMTOF(ml->a.y);
    fl->b.x = CXMTOF(ml->b.x);
    fl->b.y = CYMTOF(ml->b.y);

    DOOUTCODE(outcode1, fl->a.x, fl->a.y);
    DOOUTCODE(outcode2, fl->b.x, fl->b.y);

    if (outcode1 & outcode2)
    {
        return false;
    }

    while (outcode1 | outcode2)
    {
        // may be partially inside box
        // find an outside point
        if (outcode1)
        {
            outside = outcode1;
        }
        else
        {
            outside = outcode2;
        }

        // clip to each side
        if (outside & TOP)
        {
            dy = fl->a.y - fl->b.y;
            dx = fl->b.x - fl->a.x;
            // [JN] 'int64_t' math to avoid overflows on long lines.
            tmp.x = fl->a.x + (fixed_t)(((int64_t)dx*(fl->a.y-f_y))/dy);
            tmp.y = 0;
        }
        else if (outside & BOTTOM)
        {
            dy = fl->a.y - fl->b.y;
            dx = fl->b.x - fl->a.x;
            tmp.x = fl->a.x + (fixed_t)(((int64_t)dx*(fl->a.y-(f_y+f_h)))/dy);
            tmp.y = f_h - 1;
        }
        else if (outside & RIGHT)
        {
            dy = fl->b.y - fl->a.y;
            dx = fl->b.x - fl->a.x;
            tmp.y = fl->a.y + (fixed_t)(((int64_t)dy*(f_x+f_w-1 - fl->a.x))/dx);
            tmp.x = f_w-1;
        }
        else if (outside & LEFT)
        {
            dy = fl->b.y - fl->a.y;
            dx = fl->b.x - fl->a.x;
            tmp.y = fl->a.y + (fixed_t)(((int64_t)dy*(f_x-fl->a.x))/dx);
            tmp.x = 0;
        }
        else
        {
            tmp.x = 0;
            tmp.y = 0;
        }

        if (outside == outcode1)
        {
            fl->a = tmp;
            DOOUTCODE(outcode1, fl->a.x, fl->a.y);
        }
        else
        {
            fl->b = tmp;
            DOOUTCODE(outcode2, fl->b.x, fl->b.y);
        }

        if (outcode1 & outcode2)
        {
            return false; // trivially outside
        }
    }

    return true;
}
#undef DOOUTCODE

// -----------------------------------------------------------------------------
// AM_drawFline
// Classic Bresenham w/ whatever optimizations needed for speed.
// -----------------------------------------------------------------------------

static void AM_drawFline_Vanilla (fline_t *fl, int color)
{
    int x;
    int y;
    int dx;
    int dy;
    int sx;
    int sy;
    int ax;
    int ay;
    int d;

    // For debugging only
    if (fl->a.x < 0 || fl->a.x >= f_w
    ||  fl->a.y < 0 || fl->a.y >= f_h
    ||  fl->b.x < 0 || fl->b.x >= f_w
    ||  fl->b.y < 0 || fl->b.y >= f_h)
    {
        return;
    }

#define PUTDOT_RAW(xx,yy,cc) I_VideoBuffer[(yy)*f_w+(flipscreenwidth[xx])]=(cc)
#define PUTDOT(xx,yy,cc) PUTDOT_RAW(xx,yy,(pal_color[(cc)]))

    dx = fl->b.x - fl->a.x;
    ax = 2 * (dx<0 ? -dx : dx);
    sx = dx<0 ? -1 : 1;

    dy = fl->b.y - fl->a.y;
    ay = 2 * (dy<0 ? -dy : dy);
    sy = dy<0 ? -1 : 1;

    x = fl->a.x;
    y = fl->a.y;

    if (ax > ay)
    {
        d = ay - ax/2;

        while (1)
        {
            PUTDOT(x,y,color);

            if (x == fl->b.x)
            {
                return;
            }

            if (d>=0)
            {
                y += sy;
                d -= ax;
            }
            x += sx;
            d += ay;
        }
    }
    else
    {
        d = ax - ay/2;

        while (1)
        {
            PUTDOT(x, y, color);

            if (y == fl->b.y)
            {
                return;
            }

            if (d >= 0)
            {
                x += sx;
                d -= ay;
            }
            y += sy;
            d += ax;
        }
    }
}

// -----------------------------------------------------------------------------
// AM_drawFline_Smooth
// [crispy] Adapted from Heretic's DrawWuLine
// -----------------------------------------------------------------------------

static void AM_drawFline_Smooth(fline_t* fl, int color)
{
    int X0 = fl->a.x, Y0 = fl->a.y, X1 = fl->b.x, Y1 = fl->b.y;
    const pixel_t* BaseColor = &color_shades[color * NUMSHADES];

    unsigned short IntensityShift, ErrorAdj, ErrorAcc;
    unsigned short ErrorAccTemp, Weighting, WeightingComplementMask;
    short DeltaX, DeltaY, Temp, XDir;

    /* Make sure the line runs top to bottom */
    if (Y0 > Y1)
    {
        Temp = Y0;
        Y0 = Y1;
        Y1 = Temp;
        Temp = X0;
        X0 = X1;
        X1 = Temp;
    }

    /* Draw the initial pixel, which is always exactly intersected by
       the line and so needs no weighting */
    /* Always write the raw color value because we've already performed the necessary lookup
     * into colormap */
    PUTDOT_RAW(X0, Y0, BaseColor[0]);

    if ((DeltaX = X1 - X0) >= 0)
    {
        XDir = 1;
    }
    else
    {
        XDir = -1;
        DeltaX = -DeltaX;       /* make DeltaX positive */
    }
    /* Special-case horizontal, vertical, and diagonal lines, which
       require no weighting because they go right through the center of
       every pixel */
    if ((DeltaY = Y1 - Y0) == 0)
    {
        /* Horizontal line */
        while (DeltaX-- != 0)
        {
            X0 += XDir;
            PUTDOT_RAW(X0, Y0, BaseColor[0]);
        }
        return;
    }
    if (DeltaX == 0)
    {
        /* Vertical line */
        do
        {
            Y0++;
            PUTDOT_RAW(X0, Y0, BaseColor[0]);
        }
        while (--DeltaY != 0);
        return;
    }
    //diagonal line.
    if (DeltaX == DeltaY)
    {
        do
        {
            X0 += XDir;
            Y0++;
            PUTDOT_RAW(X0, Y0, BaseColor[0]);
        }
        while (--DeltaY != 0);
        return;
    }
    /* Line is not horizontal, diagonal, or vertical */
    ErrorAcc = 0;               /* initialize the line error accumulator to 0 */
    /* # of bits by which to shift ErrorAcc to get intensity level */
    IntensityShift = 16 - NUMSHADES_BITS;
    /* Mask used to flip all bits in an intensity weighting, producing the
       result (1 - intensity weighting) */
    WeightingComplementMask = NUMSHADES - 1;
    /* Is this an X-major or Y-major line? */
    if (DeltaY > DeltaX)
    {
        /* Y-major line; calculate 16-bit fixed-point fractional part of a
           pixel that X advances each time Y advances 1 pixel, truncating the
           result so that we won't overrun the endpoint along the X axis */
        ErrorAdj = ((unsigned int) DeltaX << 16) / (unsigned int) DeltaY;
        /* Draw all pixels other than the first and last */
        while (--DeltaY)
        {
            ErrorAccTemp = ErrorAcc;    /* remember currrent accumulated error */
            ErrorAcc += ErrorAdj;       /* calculate error for next pixel */
            if (ErrorAcc <= ErrorAccTemp)
            {
                /* The error accumulator turned over, so advance the X coord */
                X0 += XDir;
            }
            Y0++;               /* Y-major, so always advance Y */
            /* The IntensityBits most significant bits of ErrorAcc give us the
               intensity weighting for this pixel, and the complement of the
               weighting for the paired pixel */
            Weighting = ErrorAcc >> IntensityShift;
            PUTDOT_RAW(X0, Y0, BaseColor[Weighting]);
            PUTDOT_RAW(X0 + XDir, Y0, BaseColor[(Weighting ^ WeightingComplementMask)]);
        }
        /* Draw the final pixel, which is always exactly intersected by the line
           and so needs no weighting */
        PUTDOT_RAW(X1, Y1, BaseColor[0]);
        return;
    }
    /* It's an X-major line; calculate 16-bit fixed-point fractional part of a
       pixel that Y advances each time X advances 1 pixel, truncating the
       result to avoid overrunning the endpoint along the X axis */
    ErrorAdj = ((unsigned int) DeltaY << 16) / (unsigned int) DeltaX;
    /* Draw all pixels other than the first and last */
    while (--DeltaX)
    {
        ErrorAccTemp = ErrorAcc;        /* remember currrent accumulated error */
        ErrorAcc += ErrorAdj;   /* calculate error for next pixel */
        if (ErrorAcc <= ErrorAccTemp)
        {
            /* The error accumulator turned over, so advance the Y coord */
            Y0++;
        }
        X0 += XDir;             /* X-major, so always advance X */
        /* The IntensityBits most significant bits of ErrorAcc give us the
           intensity weighting for this pixel, and the complement of the
           weighting for the paired pixel */
        Weighting = ErrorAcc >> IntensityShift;
        PUTDOT_RAW(X0, Y0, BaseColor[Weighting]);
        PUTDOT_RAW(X0, Y0 + 1, BaseColor[(Weighting ^ WeightingComplementMask)]);

    }
    /* Draw the final pixel, which is always exactly intersected by the line
       and so needs no weighting */
    PUTDOT_RAW(X1, Y1, BaseColor[0]);
}

// -----------------------------------------------------------------------------
// AM_drawMline
// Clip lines, draw visible parts of lines.
// -----------------------------------------------------------------------------

static void AM_drawMline (mline_t *ml, int color)
{
    static fline_t fl;

    if (AM_clipMline(ml, &fl))
    {
        // draws it on frame buffer using fb coords
        AM_drawFline(&fl, color);
    }
}

// -----------------------------------------------------------------------------
// AM_drawGrid
// Draws flat (floor/ceiling tile) aligned grid lines.
// -----------------------------------------------------------------------------

static void AM_drawGrid (void)
{
    int64_t x, y;
    int64_t start, end;
    const fixed_t gridsize = MAPBLOCKUNITS << MAPBITS;
    mline_t ml;

    // Figure out start of vertical gridlines
    start = m_x;
    if (automap_rotate)
    {
        start -= m_h / 2;
    }

    if ((start-(bmaporgx>>FRACTOMAPBITS))%gridsize)
    {
        start -= ((start-(bmaporgx>>FRACTOMAPBITS))%gridsize);
    }

    end = m_x + m_w;

    if (automap_rotate)
    {
        end += m_h / 2;
    }

    // draw vertical gridlines
    for (x = start ; x < end ; x += gridsize)
    {
        ml.a.x = x;
        ml.b.x = x;
        // [crispy] moved here
        ml.a.y = m_y;
        ml.b.y = m_y+m_h;
        if (automap_rotate)
        {
            ml.a.y -= m_w / 2;
            ml.b.y += m_w / 2;
            AM_rotatePoint(&ml.a);
            AM_rotatePoint(&ml.b);
        }
        AM_drawMline(&ml, 104);
    }

    // Figure out start of horizontal gridlines
    start = m_y;
    if (automap_rotate)
    {
        start -= m_w / 2;
    }

    if ((start-(bmaporgy>>FRACTOMAPBITS))%gridsize)
    {
        start -= ((start-(bmaporgy>>FRACTOMAPBITS))%gridsize);
    }

    end = m_y + m_h;

    if (automap_rotate)
    {
        end += m_w / 2;
    }

    // draw horizontal gridlines
    for (y = start ; y < end ; y += gridsize)
    {
        ml.a.y = y;
        ml.b.y = y;
        // [crispy] moved here
        ml.a.x = m_x;
        ml.b.x = m_x + m_w;
        if (automap_rotate)
        {
            ml.a.x -= m_h / 2;
            ml.b.x += m_h / 2;
            AM_rotatePoint(&ml.a);
            AM_rotatePoint(&ml.b);
        }
        AM_drawMline(&ml, 104);
    }
}

// -----------------------------------------------------------------------------
// AM_drawWalls
// Determines visible lines, draws them. 
// This is LineDef based, not LineSeg based.
// -----------------------------------------------------------------------------

static void AM_drawWalls (void)
{
    static mline_t l;

    for (int i = 0 ; i < numlines ; i++)
    {
        l.a.x = lines[i].v1->x >> FRACTOMAPBITS;
        l.a.y = lines[i].v1->y >> FRACTOMAPBITS;
        l.b.x = lines[i].v2->x >> FRACTOMAPBITS;
        l.b.y = lines[i].v2->y >> FRACTOMAPBITS;

        if (automap_rotate)
        {
            AM_rotatePoint(&l.a);
            AM_rotatePoint(&l.b);
        }

        if (iddt_cheating || (lines[i].flags & ML_MAPPED))
        {
            if ((lines[i].flags & ML_DONTDRAW) && !iddt_cheating)
            {
                continue;
            }

            if (iddt_cheating || (lines[i].flags & ML_MAPPED))
            {
                if (!lines[i].backsector)
                {
                    // [JN] Highlight secret sectors
                    if (automap_secrets > 1 && lines[i].frontsector->special == 9)
                    {
                        array_push(lines_1S, ((am_line_t){l, 214}));
                    }
                    // [plums] show revealed secrets
                    else if (automap_secrets && lines[i].frontsector->oldspecial == 9)
                    {
                        array_push(lines_1S, ((am_line_t){l, 112}));
                    }
                    else
                    {
                        array_push(lines_1S, ((am_line_t){l, 32}));
                    }
                }
                else
                {
                    // Teleport line
                    if (lines[i].special == 39 || lines[i].special == 97)
                    {
                        AM_drawMline(&l, 120);
                    }
                    // Secret door
                    else if (lines[i].flags & ML_SECRET)
                    {
                        AM_drawMline(&l, 32);
                    }
                    
					// [JN] RED Key-locked doors
					else
					if (lines[i].special == 28  || lines[i].special == 33
					||  lines[i].special == 134 || lines[i].special == 135)
					{
						AM_drawMline(&l, 176);
					}
					// [JN] BLUE Key-locked doors
					else
					if (lines[i].special == 26  || lines[i].special == 32
					||  lines[i].special == 99  || lines[i].special == 133)
					{
						AM_drawMline(&l, 200);
					}
					// [JN] YELLOW Key-locked doors
					else
					if (lines[i].special == 27  || lines[i].special == 34
					||  lines[i].special == 136 || lines[i].special == 137)
					{
						AM_drawMline(&l, 228);
					}
                    // [JN] Highlight secret sectors
                    else if (automap_secrets > 1
                    && (lines[i].frontsector->special == 9
                    ||  lines[i].backsector->special == 9))
                    {
                        AM_drawMline(&l, 214);
                    }
                    // [plums] show revealed secrets
                    else if (automap_secrets
                    && (lines[i].frontsector->oldspecial == 9
                    ||  lines[i].backsector->oldspecial == 9))
                    {
                        AM_drawMline(&l, 112);
                    }
                    // Any special linedef
                    else if (lines[i].special)
                    {
                        AM_drawMline(&l, 252);
                    }
                    // Floor level change
                    else if (lines[i].backsector->floorheight != lines[i].frontsector->floorheight)
                    {
                        AM_drawMline(&l, 163);
                    }
                    // Ceiling level change
                    else if (lines[i].backsector->ceilingheight != lines[i].frontsector->ceilingheight)
                    {
                        AM_drawMline(&l, 75);
                    }
                    // Hidden gray walls
                    else if (iddt_cheating)
                    {
                        AM_drawMline(&l, 96);
                    }
                }
                // [JN] Exit (can be one-sided or two-sided)
                if (lines[i].special == 11 || lines[i].special == 51
                ||  lines[i].special == 52 || lines[i].special == 124)
                {
                    array_push(lines_1S, ((am_line_t){l, 195}));
                }
            }
            else if (plr->powers[pw_allmap])
            {
                if (!(lines[i].flags & ML_DONTDRAW)) AM_drawMline(&l, 99);
            }
        }
        else if (plr->powers[pw_allmap])
        {
            if (!(lines[i].flags & ML_DONTDRAW))
            {
                AM_drawMline(&l, 99);
            }
        }
    }

    for (int i = 0; i < array_size(lines_1S); ++i)
    {
        AM_drawMline(&lines_1S[i].l, lines_1S[i].color);
    }
    array_clear(lines_1S);
}

// -----------------------------------------------------------------------------
// AM_rotate
// Rotation in 2D. Used to rotate player arrow line character.
// -----------------------------------------------------------------------------

static void AM_rotate (int64_t *x, int64_t *y, angle_t a)
{
    int64_t tmpx;

    a >>= ANGLETOFINESHIFT;

    tmpx = FixedMul(*x, finecosine[a])
         - FixedMul(*y, finesine[a]);

    *y = FixedMul(*x, finesine[a])
       + FixedMul(*y, finecosine[a]);

    *x = tmpx;
}

// -----------------------------------------------------------------------------
// AM_rotatePoint
// [crispy] rotate point around map center
// adapted from prboom-plus/src/am_map.c:898-920
// -----------------------------------------------------------------------------

static void AM_rotatePoint (mpoint_t *pt)
{
    int64_t tmpx;
    const angle_t actualangle = ((!(!followplayer && automap_overlay)) ?
                                 ANG90 - viewangle : mapangle) >> ANGLETOFINESHIFT;

    pt->x -= mapcenter.x;
    pt->y -= mapcenter.y;

    tmpx = (int64_t)FixedMul(pt->x, finecosine[actualangle])
         - (int64_t)FixedMul(pt->y, finesine[actualangle])
         + mapcenter.x;

    pt->y = (int64_t)FixedMul(pt->x, finesine[actualangle])
          + (int64_t)FixedMul(pt->y, finecosine[actualangle])
          + mapcenter.y;

    pt->x = tmpx;
}

// -----------------------------------------------------------------------------
// AM_drawLineCharacter
// Draws a vector graphic according to numerous parameters.
// -----------------------------------------------------------------------------

static void AM_drawLineCharacter (mline_t *lineguy, int lineguylines,
                                  fixed_t scale, angle_t angle, int color,
                                  fixed_t x, fixed_t y)
{
    int     i;
    mline_t l;

    if (automap_rotate)
    {
        angle += mapangle;
    }

    for (i = 0 ; i < lineguylines ; i++)
    {
        l.a.x = lineguy[i].a.x;
        l.a.y = lineguy[i].a.y;

        if (scale)
        {
            l.a.x = FixedMul(scale, l.a.x);
            l.a.y = FixedMul(scale, l.a.y);
        }

        if (angle)
        {
            AM_rotate(&l.a.x, &l.a.y, angle);
        }

        l.a.x += x;
        l.a.y += y;

        l.b.x = lineguy[i].b.x;
        l.b.y = lineguy[i].b.y;

        if (scale)
        {
            l.b.x = FixedMul(scale, l.b.x);
            l.b.y = FixedMul(scale, l.b.y);
        }

        if (angle)
        {
            AM_rotate(&l.b.x, &l.b.y, angle);
        }

        l.b.x += x;
        l.b.y += y;

        AM_drawMline(&l, color);
    }
}

// -----------------------------------------------------------------------------
// AM_drawPlayers
// Draws the player arrow in single player, 
// or all the player arrows in a netgame.
// -----------------------------------------------------------------------------


static void AM_drawPlayers (void)
{
    // [JN] Jaguar: blinking player arrow:
    if (blinking_arrow > 3)
    {
        mpoint_t pt;

        // [JN] Smooth player arrow rotation.
        // Keep arrow static in Spectator + rotate mode.
        const angle_t smoothangle = (crl_spectating && automap_rotate) ?
                                     plr->mo->angle :
                                     automap_rotate ? plr->mo->angle : viewangle;

        // [JN] Interpolate player arrow.
        pt.x = viewx >> FRACTOMAPBITS;
        pt.y = viewy >> FRACTOMAPBITS;

        // [JN] Prevent arrow jitter in non-hires mode.
        if (vid_resolution == 1)
        {
            pt.x = FTOM(MTOF(pt.x));
            pt.y = FTOM(MTOF(pt.y));
        }

        if (automap_rotate)
        {
            AM_rotatePoint(&pt);
        }

        AM_drawLineCharacter(thintriangle_guy, arrlen(thintriangle_guy), 0,
                             smoothangle, 116, pt.x, pt.y);
    }
}

// -----------------------------------------------------------------------------
// AM_drawThings
// Draws the things on the automap in double IDDT cheat mode.
// -----------------------------------------------------------------------------

static void AM_drawThings (void)
{
    int       i;
    mpoint_t  pt;
    mobj_t   *t;
    angle_t   actualangle;
    // RestlessRodent -- Carbon copy from ReMooD
    int       color = 112;

    for (i = 0 ; i < numsectors ; i++)
    {
        t = sectors[i].thinglist;
        while (t)
        {
            // [JN] Use actual radius for things drawing.
            const fixed_t actualradius = t->radius >> FRACTOMAPBITS;
                
            // [crispy] do not draw an extra triangle for the player
            if (t == plr->mo)
            {
                t = t->snext;
                continue;
            }

            // [JN] Interpolate things if possible.
            if (vid_uncapped_fps && realleveltime > oldleveltime)
            {
                pt.x = LerpFixed(t->oldx, t->x) >> FRACTOMAPBITS;
                pt.y = LerpFixed(t->oldy, t->y) >> FRACTOMAPBITS;
                actualangle = LerpAngle(t->oldangle, t->angle);
            }
            else
            {
                pt.x = t->x >> FRACTOMAPBITS;
                pt.y = t->y >> FRACTOMAPBITS;
                actualangle = t->angle;
            }

            // [JN] Keep things static in Spectator + rotate mode.
            if (crl_spectating && automap_rotate)
            {
                actualangle = t->angle - mapangle - viewangle + ANG90;
            }

            if (automap_rotate)
            {
                AM_rotatePoint(&pt);
            }

            // [JN] IDDT extended colors:
            // [crispy] draw blood splats and puffs as small squares
            if (t->type == MT_BLOOD || t->type == MT_PUFF)
            {
                AM_drawLineCharacter(thintriangle_guy, arrlen(thintriangle_guy),
                                     actualradius >> 2, actualangle, 96, pt.x, pt.y);
            }
            else
            {
                // [JN] CRL - ReMooD-inspired monsters coloring.
                if (t->target && t->state && t->state->action.acv != A_Look)
                {
                    color = iddt_reds_active;
                }
                else
                {
                    color = iddt_reds_inactive;
                }

                AM_drawLineCharacter(thintriangle_guy, arrlen(thintriangle_guy), 
                                     actualradius, actualangle, 
                                     // Monsters
                                     t->flags & MF_COUNTKILL ? (t->health > 0 ? color : 96) :
                                     // Explosive barrels (does not have a MF_COUNTKILL flag)
                                     t->type == MT_BARREL ? 231 :
                                     // Countable items
                                     t->flags & MF_COUNTITEM ? 112 :
                                     // Everything else
                                     96,
                                     pt.x, pt.y);
            }

            t = t->snext;
        }
    }
}

// -----------------------------------------------------------------------------
// AM_drawMarks
// Draw the marked locations on the automap.
// -----------------------------------------------------------------------------

#define MARK_W      (5)
#define MARK_FLIP_1 (1)
#define MARK_FLIP_2 (9)

static void AM_drawMarks (void)
{
    int i, fx, fy;
    int fx_flip; // [crispy] support for marks drawing in flipped levels
    mpoint_t pt;

    // [JN] killough 2/22/98: remove automap mark limit
    for ( i = 0 ; i < markpointnum ; i++)
    {
        if (markpoints[i].x != -1)
        {
            int j = i;

            // [crispy] center marks around player
            pt.x = markpoints[i].x;
            pt.y = markpoints[i].y;

            if (automap_rotate)
            {
                AM_rotatePoint(&pt);
            }

            fx = (CXMTOF(pt.x) / vid_resolution) - 1;
            fy = (CYMTOF(pt.y) / vid_resolution) - 2;
            fx_flip = (flipscreenwidth[CXMTOF(pt.x)] / vid_resolution) - 1;

            do
            {
                int d = j % 10;

                // killough 2/22/98: less spacing for '1'
                if (d == 1)
                {
                    fx += (MARK_FLIP_1);
                }

                if (fx >= f_x && fx <= (f_w / vid_resolution) - 5
                &&  fy >= f_y && fy <= (f_h / vid_resolution) - 6)
                {
                    // [JN] Single there is neither AMMNUM patches in Jaguar,
                    // nor STGNUM, just use MICRO_* (STYSNUM) colored gray.
                    dp_translation = cr[CR_GRAY];
                    V_DrawPatch(fx_flip - WIDESCREENDELTA, fy, marknums[d]);
                    dp_translation = NULL;
                }

                // killough 2/22/98: 1 space backwards
                fx_flip -= MARK_W - (MARK_FLIP_1);

                j /= 10;
            } while (j > 0);
        }
    }
}

// -----------------------------------------------------------------------------
// AM_drawCrosshair
// -----------------------------------------------------------------------------

static void AM_drawCrosshair (boolean force)
{
    // [crispy] draw an actual crosshair
    if (!followplayer || force)
    {
        static fline_t h, v;

        if (!h.a.x || force)
        {
            h.a.x = h.b.x = v.a.x = v.b.x = f_x + f_w / 2;
            h.a.y = h.b.y = v.a.y = v.b.y = f_y + f_h / 2;
            h.a.x -= 2; h.b.x += 2;
            v.a.y -= 2; v.b.y += 2;
        }

        // [JN] Do not draw crosshair while video re-init functions.
        if (!force)
        {
            AM_drawFline(&h, 96);
            AM_drawFline(&v, 96);
        }
    }
}

// -----------------------------------------------------------------------------
// AM_LevelNameDrawer
// -----------------------------------------------------------------------------

void AM_LevelNameDrawer (void)
{
    static char str[128];

    sprintf(str, "%s", level_names[gamemap-1]);
    M_WriteText(0 - WIDESCREENDELTA, 152, str, NULL);
}

// -----------------------------------------------------------------------------
// AM_Drawer
// -----------------------------------------------------------------------------

void AM_Drawer (void)
{
    if (!automapactive)
    {
        return;
    }
    
    // [JN] Moved from AM_Ticker for drawing interpolation.
    if (followplayer)
    {
        AM_doFollowPlayer();
    }

    // Change the zoom if necessary.
    // [JN] Moved from AM_Ticker for zooming interpolation.
    if (ftom_zoommul != FRACUNIT)
    {
        AM_changeWindowScale();
    }

    // Change X and Y location.
    // [JN] Moved from AM_Ticker for paning interpolation.
    if (m_paninc.x || m_paninc.y)
    {
        AM_changeWindowLoc();
    }

    // [crispy] required for AM_rotatePoint()
    if (automap_rotate)
    {
        mapcenter.x = m_x + m_w / 2;
        mapcenter.y = m_y + m_h / 2;
        // [crispy] keep the map static in overlay mode
        // if not following the player
        if (!(!followplayer && automap_overlay))
        {
            mapangle = ANG90 - plr->mo->angle;
        }
    }

	if (!automap_overlay)
    {
		AM_clearFB();
		pspr_interp = false;  // [JN] Supress interpolated weapon bobbing.
    }

    if (automap_shading && automap_overlay)
    {
        AM_shadeBackground();
    }

    if (grid)
    {
        AM_drawGrid();
    }

    AM_drawWalls();

    AM_drawPlayers();

    if (iddt_cheating == 2)
    {
        AM_drawThings();
    }

    // [JN] Do not draw in following mode.
    if (!followplayer)
    {
        AM_drawCrosshair(false);
    }

    AM_drawMarks();

    // [JN] Draw level name only if Level Name widget is set to "automap".
    if (!widget_levelname)
    {
        AM_LevelNameDrawer();
    }

    V_MarkRect(f_x, f_y, f_w, f_h);
}
