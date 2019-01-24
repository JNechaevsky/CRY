//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2018-2019 Julia Nechaevskaya
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

#include "z_zone.h"
#include "m_misc.h"
#include "m_random.h"
#include "deh_main.h"
#include "i_swap.h"
#include "i_system.h"
#include "w_wad.h"
#include "g_game.h"
#include "r_local.h"
#include "s_sound.h"
#include "doomstat.h"
#include "sounds.h"
#include "v_video.h"
#include "wi_stuff.h"
#include "hu_stuff.h"
#include "d_englsh.h"
#include "jn.h"


// =============================================================================
// Data needed to add patches to full screen intermission pics.
// Patches are statistics messages, and animations.
// Loads of by-pixel layout and placement, offsets etc.
// =============================================================================


// GLOBAL LOCATIONS
#define WI_TITLEY       2
#define WI_SPACINGY     33

// SINGPLE-PLAYER STUFF
#define SP_STATSX       50
#define SP_STATSY       50
#define SP_TIMEX        16
#define SP_TIMEY        (ORIGHEIGHT-32)


typedef enum
{
    ANIM_ALWAYS,
    ANIM_RANDOM,
    ANIM_LEVEL
} animenum_t;

typedef struct
{
    int x;
    int y;
} point_t;


// -----------------------------------------------------------------------------
// Animation.
// There is another anim_t used in p_spec.
// -----------------------------------------------------------------------------

typedef struct
{
    animenum_t type;

    int     period;  // period in tics between animations
    int     nanims;  // number of animation frames
    point_t loc; // location of animation

    // ALWAYS: n/a,
    // RANDOM: period deviation (<256),
    // LEVEL: level
    int     data1;

    // ALWAYS: n/a,
    // RANDOM: random base period,
    // LEVEL: n/a
    int     data2; 

    patch_t *p[3];  // actual graphics for frames of animations

    // following must be initialized to zero before use!

    int nexttic;    // next value of bcnt (used in conjunction with period)
    int lastdrawn;  // last drawn animation frame
    int ctr;        // next frame number to animate
    int state;      // used by RANDOM and LEVEL when animating
} anim_t;


// =============================================================================
// GENERAL DATA
// =============================================================================


// -----------------------------------------------------------------------------
// Locally used stuff.
// -----------------------------------------------------------------------------

// States for single-player
#define SP_KILLS        0
#define SP_ITEMS        2
#define SP_SECRET       4
#define SP_FRAGS        6 
#define SP_TIME         8 
#define SP_PAR          ST_TIME
#define SP_PAUSE        1

// used to accelerate or skip a stage
static int acceleratestage;

// wbs->pnum
static int me;

 // specifies current state
static stateenum_t state;

// contains information passed into intermission
static wbstartstruct_t  *wbs;
static wbplayerstruct_t *plrs;  // wbs->plyr[]

// used for general timing
static int cnt;  

// used for timing of background animation
static int bcnt;

// signals to refresh everything for one frame
static int firstrefresh; 

static int cnt_kills[MAXPLAYERS];
static int cnt_items[MAXPLAYERS];
static int cnt_secret[MAXPLAYERS];
static int cnt_time;
static int cnt_par;
static int cnt_pause;

// # of commercial levels
static int NUMCMAPS; 


// =============================================================================
// GRAPHICS
// =============================================================================

static patch_t *percent;    // %
static patch_t *colon;      // graphics
static patch_t *num[10];    // 0-9 graphic
static patch_t *wiminus;    // minus sign
static patch_t *background; // Buffer storing the backdrop

// =============================================================================
// CODE
// =============================================================================

// slam background
void WI_slamBackground(void)
{
    // [Julia] Remove level's remaining  background, fill it with black color
    V_DrawFilledBox(viewwindowx, viewwindowy, scaledviewwidth, scaledviewheight, 0);
    V_DrawPatch(0, 0, background);
}


// The ticker is used to detect keys
//  because of timing issues in netgames.
boolean WI_Responder(event_t* ev)
{
    return false;
}


// -----------------------------------------------------------------------------
// WI_drawNum
//
// Draws a number.
// If digits > 0, then use that many digits minimum,
//  otherwise only use as many as necessary.
// Returns new x position.
// -----------------------------------------------------------------------------

int WI_drawNum (int x, int y, int n, int digits)
{
    int fontwidth = SHORT(num[0]->width);
    int neg;
    int temp;

    if (digits < 0)
    {
        if (!n)
        {
            // make variable-length zeros 1 digit long
            digits = 1;
        }
        else
        {
            // figure out # of digits in #
            digits = 0;
            temp = n;

            while (temp)
            {
                temp /= 10;
                digits++;
            }
        }
    }

    neg = n < 0;
    if (neg)
    n = -n;

    // if non-number, do not draw it
    if (n == 1994)
    return 0;

    // draw the new number
    while (digits--)
    {
        x -= fontwidth;
        V_DrawShadowedPatch(x, y, num[ n % 10 ]);
        n /= 10;
    }

    // draw a minus sign if necessary
    if (neg)
    V_DrawShadowedPatch(x-=8, y, wiminus);

    return x;
}


// -----------------------------------------------------------------------------
// WI_drawPercent
// -----------------------------------------------------------------------------

void WI_drawPercent (int x, int y, int p)
{
    if (p < 0)
    return;

    V_DrawShadowedPatch(x, y, percent);
    WI_drawNum(x, y, p, -1);
}


// -----------------------------------------------------------------------------
// Display level completion time and par,
//  or "sucks" message if overflow.
// -----------------------------------------------------------------------------

void WI_drawTime (int x, int y, int t, boolean suck)
{
    int div;
    int n;

    if (t<0)
    return;

    if (t <= 61*59 || !suck)
    {
        div = 1;

        do
        {
            n = (t / div) % 60;
            x = WI_drawNum(x, y, n, 2) - SHORT(colon->width);
            div *= 60;

            // draw
            if (div==60 || t / div)
            {
                V_DrawShadowedPatch(x, y, colon);
            }
        } while (t / div);
    }
    else
    {
        // [Julia] Write "sucks"
        HU_WriteTextBig (198, 113, "sucks");
    }
}


// -----------------------------------------------------------------------------
// WI_End
// -----------------------------------------------------------------------------

void WI_End(void)
{
    void WI_unloadData(void);
    WI_unloadData();
}


// -----------------------------------------------------------------------------
// WI_initNoState
// -----------------------------------------------------------------------------

void WI_initNoState(void)
{
    state = NoState;
    acceleratestage = 0;
    cnt = 10;
}


// -----------------------------------------------------------------------------
// WI_updateNoState
// -----------------------------------------------------------------------------

void WI_updateNoState(void)
{
    if (!--cnt)
    {
        // Don't call WI_End yet.  G_WorldDone doesnt immediately 
        // change gamestate, so WI_Drawer is still going to get
        // run until that happens.  If we do that after WI_End
        // (which unloads all the graphics), we're in trouble.

        //WI_End();
        G_WorldDone();
    }
}

static boolean  snl_pointeron = false;


// -----------------------------------------------------------------------------
// WI_drawShowNextLoc
// -----------------------------------------------------------------------------

void WI_drawShowNextLoc(void)
{
    // [Julia] Just clean up intermission stats
    WI_slamBackground(); 
}


// -----------------------------------------------------------------------------
// WI_drawNoState
// -----------------------------------------------------------------------------

void WI_drawNoState(void)
{
    snl_pointeron = true;
    WI_drawShowNextLoc();
}

static int sp_state;


// -----------------------------------------------------------------------------
// WI_initStats
// -----------------------------------------------------------------------------

void WI_initStats(void)
{
    state = StatCount;
    acceleratestage = 0;
    sp_state = 1;
    cnt_kills[0] = cnt_items[0] = cnt_secret[0] = -1;
    cnt_time = cnt_par = -1;
    cnt_pause = TICRATE;
}


// -----------------------------------------------------------------------------
// WI_updateStats
//
// [Julia] Heavily modified to simulate Jaguar intermission screen
// -----------------------------------------------------------------------------

void WI_updateStats(void)
{
    if (acceleratestage && sp_state != 10)
    {
        acceleratestage = 0;
        cnt_kills[0] = (plrs[me].skills * 100) / wbs->maxkills;
        cnt_items[0] = (plrs[me].sitems * 100) / wbs->maxitems;
        cnt_secret[0] = (plrs[me].ssecret * 100) / wbs->maxsecret;
        cnt_time = plrs[me].stime / TICRATE;
        sp_state = 10;
    }

    // [Julia] Jaguar: count everything simultaneously:
    if (sp_state == 2)
    {
        // [Julia] Jaguar: count 2x time faster (standard: += 2)
        cnt_kills[0] += 4;
        cnt_items[0] += 4;
        cnt_secret[0] += 4;
        cnt_time += 4;

        // [Julia] Don't go higher than 100%
        if (cnt_kills[0] >= (plrs[me].skills * 100) / wbs->maxkills)
            cnt_kills[0] = (plrs[me].skills * 100) / wbs->maxkills;

        if (cnt_items[0] >= (plrs[me].sitems * 100) / wbs->maxitems)
            cnt_items[0] = (plrs[me].sitems * 100) / wbs->maxitems;

        if (cnt_secret[0] >= (plrs[me].ssecret * 100) / wbs->maxsecret)
            cnt_secret[0] = (plrs[me].ssecret * 100) / wbs->maxsecret;

        if (cnt_time >= plrs[me].stime / TICRATE)
            cnt_time = plrs[me].stime / TICRATE;

        // [Julia] Now, if all countings performed, ready to go to next level.
        // If not performed, i.e. still counting, pressing 'use' will
        // finish counting and allows to go to next level. Any questions?
        if (cnt_kills[0] == (plrs[me].skills * 100) / wbs->maxkills
        &&  cnt_items[0] == (plrs[me].sitems * 100) / wbs->maxitems
        &&  cnt_secret[0] == (plrs[me].ssecret * 100) / wbs->maxsecret
        &&  cnt_time == plrs[me].stime / TICRATE)
        {
            sp_state = 10;
        }
    }

    else if (sp_state == 10)
    {
        if (acceleratestage)
        {
            // [Julia] feedback sound
            S_StartSound(0, sfx_sgcock);
            WI_initNoState();
        }
    }
    else if (sp_state & 1)
    {
        if (!--cnt_pause)
        {
            sp_state++;
            cnt_pause = 1;
        }
    }
}


// -----------------------------------------------------------------------------
// WI_drawStats
// -----------------------------------------------------------------------------

void WI_drawStats(void)
{
    int lh = (3*SHORT(num[0]->height))/2;   // line height

    WI_slamBackground();

    // [Julia] Finished level stuff
    if (wbs->last < NUMCMAPS)
    {
        // [Julia] Write <LevelName> 
        HU_WriteTextBigCentered (2, 
        gamemap ==  1 ? JAGLVL_01 :
        gamemap ==  2 ? JAGLVL_02 :
        gamemap ==  3 ? JAGLVL_03 :
        gamemap ==  4 ? JAGLVL_04 :
        gamemap ==  5 ? JAGLVL_05 :
        gamemap ==  6 ? JAGLVL_06 :
        gamemap ==  7 ? JAGLVL_07 :
        gamemap ==  8 ? JAGLVL_08 :
        gamemap ==  9 ? JAGLVL_09 :
        gamemap == 10 ? JAGLVL_10 :
        gamemap == 11 ? JAGLVL_11 :
        gamemap == 12 ? JAGLVL_12 :
        gamemap == 13 ? JAGLVL_13 :
        gamemap == 14 ? JAGLVL_14 :
        gamemap == 15 ? JAGLVL_15 :
        gamemap == 16 ? JAGLVL_16 :
        gamemap == 17 ? JAGLVL_17 :
        gamemap == 18 ? JAGLVL_18 :
        gamemap == 18 ? JAGLVL_18 :
        gamemap == 19 ? JAGLVL_19 :
        gamemap == 20 ? JAGLVL_20 :
        gamemap == 21 ? JAGLVL_21 :
        gamemap == 22 ? JAGLVL_22 :
        gamemap == 23 ? JAGLVL_23 : 
        gamemap == 24 ? JAGLVL_24 :
        gamemap == 25 ? JAGLVL_25 :
                        JAGLVL_26);

        // [Julia] Write "Finished"
        HU_WriteTextBigCentered (20, "Finished");
    }

    // Kills
    HU_WriteTextBig(91, 50, "Kills");
    WI_drawPercent(ORIGWIDTH - SP_STATSX, SP_STATSY, cnt_kills[0]);

    // Items
    HU_WriteTextBig(86, 68, "Items");
    WI_drawPercent(ORIGWIDTH - SP_STATSX, SP_STATSY+lh, cnt_items[0]);

    // Secrets
    HU_WriteTextBig(50, 86, "Secrets");
    WI_drawPercent(ORIGWIDTH - SP_STATSX, SP_STATSY+2*lh, cnt_secret[0]);

    // Time
    HU_WriteTextBig(94, 113, "Time");
    WI_drawTime(ORIGWIDTH - SP_STATSX, SP_STATSY+4*lh-8+2, cnt_time, true);

    // [Julia] Draw total times only after finishing last level
    if (gamemap == 23)
    {
        // [crispy] draw total time after level time and par time
        const int ttime = wbs->totaltimes / TICRATE;

        HU_WriteTextBig(79, 131, "Total");
        
        // [Julia] Show total time only after level time is counted
        if (cnt_time == plrs[me].stime / TICRATE)
        WI_drawTime(ORIGWIDTH - SP_STATSX, SP_STATSY+5*lh-8+2, ttime, false);
    }

    // Draws which level you are entering...
    // [Julia] DON'T draw "Entering Military Base" after finishing map 23
    if (gamemap != 23)
    {
        // [Julia] Write "Entering"
        HU_WriteTextBigCentered (148, "Entering");

        // [Julia] Write <NextLevelName>
        // Note: wminfo.next is 0 biased, unlike gamemap, remember?
        // That's why I'm using "-1" here.
        HU_WriteTextBigCentered (166, 
        wminfo.next ==  1-1 ? JAGLVL_01 :
        wminfo.next ==  2-1 ? JAGLVL_02 :
        wminfo.next ==  3-1 ? JAGLVL_03 :
        wminfo.next ==  4-1 ? JAGLVL_04 :
        wminfo.next ==  5-1 ? JAGLVL_05 :
        wminfo.next ==  6-1 ? JAGLVL_06 :
        wminfo.next ==  7-1 ? JAGLVL_07 :
        wminfo.next ==  8-1 ? JAGLVL_08 :
        wminfo.next ==  9-1 ? JAGLVL_09 :
        wminfo.next == 10-1 ? JAGLVL_10 :
        wminfo.next == 11-1 ? JAGLVL_11 :
        wminfo.next == 12-1 ? JAGLVL_12 :
        wminfo.next == 13-1 ? JAGLVL_13 :
        wminfo.next == 14-1 ? JAGLVL_14 :
        wminfo.next == 15-1 ? JAGLVL_15 :
        wminfo.next == 16-1 ? JAGLVL_16 :
        wminfo.next == 17-1 ? JAGLVL_17 :
        wminfo.next == 18-1 ? JAGLVL_18 :
        wminfo.next == 18-1 ? JAGLVL_18 :
        wminfo.next == 19-1 ? JAGLVL_19 :
        wminfo.next == 20-1 ? JAGLVL_20 :
        wminfo.next == 21-1 ? JAGLVL_21 :
        wminfo.next == 22-1 ? JAGLVL_22 :
        wminfo.next == 23-1 ? JAGLVL_23 : 
        wminfo.next == 24-1 ? JAGLVL_24 :
        wminfo.next == 25-1 ? JAGLVL_25 :
                              JAGLVL_26);
    }
}


// -----------------------------------------------------------------------------
// WI_checkForAccelerate
// -----------------------------------------------------------------------------

void WI_checkForAccelerate(void)
{
    int       i;
    player_t *player;

    // check for button presses to skip delays
    for (i=0, player = players ; i<MAXPLAYERS ; i++, player++)
    {
        if (playeringame[i])
        {
            if (player->cmd.buttons & BT_ATTACK)
            {
                if (!player->attackdown)
                acceleratestage = 1;
                player->attackdown = true;
            }
            else
            {
                player->attackdown = false;
            }

            if (player->cmd.buttons & BT_USE)
            {
                if (!player->usedown)
                {
                    acceleratestage = 1;
                }
                player->usedown = true;
            }
            else
            {
                player->usedown = false;
            }

            // [Julia] Pressing PAUSE should not accelerate intermission screen
            if (player->cmd.buttons & BTS_PAUSE)
            {
                acceleratestage = 0;
            }
        }
    }
}


// -----------------------------------------------------------------------------
// WI_Ticker
//
// Updates stuff each tick
// -----------------------------------------------------------------------------

void WI_Ticker(void)
{
    // [Julia] Make PAUSE working properly on intermission screen
    if (paused)
    {
        return;
    }

    // counter for general background animation
    bcnt++;  

    if (bcnt == 1)
    {
        // intermission music
        S_ChangeMusic(mus_inter, true); 
    }

    WI_checkForAccelerate();

    switch (state)
    {
        case StatCount:
        WI_updateStats();
        break;

        case NoState:
        WI_updateNoState();
        break;
    }
}


// -----------------------------------------------------------------------------
// WI_loadUnloadData
//
// Common load/unload function.  Iterates over all the graphics
// lumps to be loaded/unloaded into memory.
// -----------------------------------------------------------------------------

typedef void (*load_callback_t)(char *lumpname, patch_t **variable);

static void WI_loadUnloadData(load_callback_t callback)
{
    int     i;
    char    name[9];

    // More hacks on minus sign.
    callback(DEH_String("WIMINUS"), &wiminus);

    for (i=0 ; i<10 ; i++)
    {
        // numbers 0-9
        DEH_snprintf(name, 9, "WINUM%d", i);
        callback(name, &num[i]);
    }

    callback(DEH_String("WIPCNT"), &percent);       // percent sign
    callback(DEH_String("WICOLON"), &colon);        // ":"

    M_StringCopy(name, DEH_String("M_TITLE"), sizeof(name));   // Background

    // Draw backdrop and save to a temporary buffer
    callback(name, &background);
}


// -----------------------------------------------------------------------------
// WI_loadCallback
// -----------------------------------------------------------------------------

static void WI_loadCallback(char *name, patch_t **variable)
{
    *variable = W_CacheLumpName(name, PU_STATIC);
}


// -----------------------------------------------------------------------------
// WI_loadData
// -----------------------------------------------------------------------------

void WI_loadData(void)
{
    NUMCMAPS = 26;  // [Julia] Only 26 maps available

    WI_loadUnloadData(WI_loadCallback);
}


// -----------------------------------------------------------------------------
// WI_unloadCallback
// -----------------------------------------------------------------------------

static void WI_unloadCallback(char *name, patch_t **variable)
{
    W_ReleaseLumpName(name);
    *variable = NULL;
}


// -----------------------------------------------------------------------------
// WI_unloadData
// -----------------------------------------------------------------------------

void WI_unloadData(void)
{
    WI_loadUnloadData(WI_unloadCallback);
}


// -----------------------------------------------------------------------------
// WI_Drawer
// -----------------------------------------------------------------------------

void WI_Drawer (void)
{
    switch (state)
    {
        case StatCount:
        WI_drawStats();
        break;

        case NoState:
        WI_drawNoState();
        break;
    }
}


// -----------------------------------------------------------------------------
// WI_initVariables
// -----------------------------------------------------------------------------

void WI_initVariables (wbstartstruct_t *wbstartstruct)
{
    wbs = wbstartstruct;

#ifdef RANGECHECKING
        RNGCHECK(wbs->last, 0, 8);
        RNGCHECK(wbs->next, 0, 8);
        RNGCHECK(wbs->pnum, 0, MAXPLAYERS);
        RNGCHECK(wbs->pnum, 0, MAXPLAYERS);
#endif

    acceleratestage = 0;
    cnt = bcnt = 0;
    firstrefresh = 1;
    me = wbs->pnum;
    plrs = wbs->plyr;

    if (!wbs->maxkills)
    wbs->maxkills = 1;

    if (!wbs->maxitems)
    wbs->maxitems = 1;

    if (!wbs->maxsecret)
    wbs->maxsecret = 1;

    if ( gamemode != retail )
        if (wbs->epsd > 2)
        wbs->epsd -= 3;
}


// -----------------------------------------------------------------------------
// WI_Start
// -----------------------------------------------------------------------------

void WI_Start (wbstartstruct_t *wbstartstruct)
{
    WI_initVariables(wbstartstruct);
    WI_loadData();
    WI_initStats();
}

