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
#include "doomstat.h"
#include "g_game.h"
#include "i_swap.h"
#include "m_menu.h"
#include "p_local.h"
#include "s_sound.h"
#include "v_video.h"
#include "wi_stuff.h"
#include "z_zone.h"

#include "id_vars.h"
#include "id_func.h"


// [JN] Jaguar: only 24 maps available.
#define NUMMAPS		24

// Singple-player stuff
#define SP_STATSX	50


static int acceleratestage;	// used to accelerate or skip a stage
static int me;				// wbs->pnum
static int cnt;				// used for general timing
static int bcnt;			// used for timing of background animation
static int sp_state;		// state of drawing/counting

// stats counters
static int cnt_kills[MAXPLAYERS];
static int cnt_items[MAXPLAYERS];
static int cnt_secret[MAXPLAYERS];
static int cnt_time;
static int cnt_pause;

 // specifies current state
static stateenum_t	state;

// contains information passed into intermission
static wbstartstruct_t*	wbs;

// wbs->plyr[]
static wbplayerstruct_t* plrs;

// Graphics
static patch_t *colon;		// ":" graphics
static patch_t *percent;	// percent sign
static patch_t *num[10];	// 0-9 digits


// -----------------------------------------------------------------------------
// WI_drawNum
// Draws a number. If digits > 0, then use that many digits minimum,
// otherwise only use as many as necessary. Returns new x position.
// -----------------------------------------------------------------------------

static const int WI_drawNum (int x, int y, int n, int digits)
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
		V_DrawShadowedPatchOptional(x, y, num[ n % 10 ]);
		n /= 10;
	}

	return x;
}

// -----------------------------------------------------------------------------
// WI_drawPercent
// -----------------------------------------------------------------------------

static void WI_drawPercent (int x, int y, int p)
{
	if (p < 0)
	{
		return;
	}

	V_DrawShadowedPatchOptional(x, y, percent);
	WI_drawNum(x, y, p, -1);
}

// -----------------------------------------------------------------------------
// WI_drawTime
// Display level completion time and par, or "sucks" message if overflow.
// -----------------------------------------------------------------------------

static void WI_drawTime (int x, int y, int t, boolean suck)
{
	int div;
	int n;

	if (t < 0)
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
				V_DrawShadowedPatchOptional(x, y, colon);
			}
		} while (t / div);
	}
	else
	{
		// [JN] Write "sucks"
		M_WriteTextBig (198, 113, "sucks", NULL);
	}
}

// -----------------------------------------------------------------------------
// WI_initNoState
// -----------------------------------------------------------------------------

static void WI_initNoState (void)
{
	state = NoState;
	acceleratestage = 0;
	cnt = 10;
}

// -----------------------------------------------------------------------------
// WI_initNoState
// -----------------------------------------------------------------------------

static void WI_updateNoState (void)
{
    if (!--cnt)
    {
		G_WorldDone();
    }
}

// -----------------------------------------------------------------------------
// WI_drawNoState
// -----------------------------------------------------------------------------

static void WI_drawNoState (void)
{
	// [JN] No-op.
}

// -----------------------------------------------------------------------------
// WI_initStats
// -----------------------------------------------------------------------------

static void WI_initStats (void)
{
	state = StatCount;
	acceleratestage = 0;
	sp_state = 1;
	cnt_kills[0] = cnt_items[0] = cnt_secret[0] = -1;
	cnt_time = -1;
	cnt_pause = TICRATE;
}

// -----------------------------------------------------------------------------
// WI_updateStats
// [JN] Heavily modified to simulate Jaguar intermission screen.
// -----------------------------------------------------------------------------

static void WI_updateStats (void)
{
	if (acceleratestage && sp_state != 10)
	{
		acceleratestage = 0;
		cnt_kills[0] = (plrs[me].skills * 100) / wbs->maxkills;
		cnt_items[0] = (plrs[me].sitems * 100) / wbs->maxitems;
		cnt_secret[0] = (plrs[me].ssecret * 100) / wbs->maxsecret;
		cnt_time = plrs[me].stime / TICRATE;
		// S_StartSound(0, sfx_barexp);
		sp_state = 10;
	}

	// [JN] Jaguar: count everything simultaneously.
	if (sp_state == 2)
	{
		cnt_kills[0] += 2;
		cnt_items[0] += 2;
		cnt_secret[0] += 2;
		cnt_time += 3;  // [JN] Count time 2x faster

		// [JN] Don't go higher than 100%
		if (cnt_kills[0] >= (plrs[me].skills * 100) / wbs->maxkills)
			cnt_kills[0] = (plrs[me].skills * 100) / wbs->maxkills;

		if (cnt_items[0] >= (plrs[me].sitems * 100) / wbs->maxitems)
			cnt_items[0] = (plrs[me].sitems * 100) / wbs->maxitems;

		if (cnt_secret[0] >= (plrs[me].ssecret * 100) / wbs->maxsecret)
			cnt_secret[0] = (plrs[me].ssecret * 100) / wbs->maxsecret;

		if (cnt_time >= plrs[me].stime / TICRATE)
			cnt_time = plrs[me].stime / TICRATE;

		// [JN] Now, if all countings performed, ready to go to next level.
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
			// [JN] feedback sound
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

static void WI_drawStats (void)
{
	char str[128];
	// [crispy] draw total time after level time and par time
	const int ttime = wbs->totaltimes / TICRATE;

	V_DrawPatchFullScreen(W_CacheLumpName(("M_TITLE"), PU_CACHE), false);

    // Finished level stuff
    if (wbs->last < NUMMAPS)
    {
		sprintf(str, "%s", level_names[gamemap-1]);
		M_WriteTextBigCentered(11, str, NULL);
		M_WriteTextBigCentered(29, "Finished", NULL);
	}

	// Kills
	M_WriteTextBig(71, 51, "Kills", NULL);
	WI_drawPercent(ORIGWIDTH - SP_STATSX, 51, cnt_kills[0]);

	// Items
	M_WriteTextBig(66, 69, "Items", NULL);
	WI_drawPercent(ORIGWIDTH - SP_STATSX, 69, cnt_items[0]);

	// Secrets
	M_WriteTextBig(30, 87, "Secrets", NULL);
	WI_drawPercent(ORIGWIDTH - SP_STATSX, 87, cnt_secret[0]);

	// Time
	M_WriteTextBig(74, 111, "Time", NULL);
	WI_drawTime(ORIGWIDTH - SP_STATSX, 114, cnt_time, true);

	// Total time. Show total time only after level time is counted.
	M_WriteTextBig(59, 129, "Total", NULL);
	if (cnt_time == plrs[me].stime / TICRATE)
	WI_drawTime(ORIGWIDTH - SP_STATSX, 132, ttime, false);

	// Draws which level you are entering...
	// Don't draw "Entering Military Base" after finishing map 23.
	if (gamemap != 23)
	{
		M_WriteTextBigCentered (155, "Entering", NULL);
		sprintf(str, "%s", level_names[wminfo.next]);
		M_WriteTextBigCentered(173, str, NULL);
	}
}

// -----------------------------------------------------------------------------
// WI_checkForAccelerate
// -----------------------------------------------------------------------------

static void WI_checkForAccelerate (void)
{
	int		  i;
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

			// [JN] Pressing PAUSE should not accelerate intermission screen
			if (player->cmd.buttons & BTS_PAUSE)
			{
				acceleratestage = 0;
			}
		}
	}
}

// -----------------------------------------------------------------------------
// WI_Ticker
// Updates stuff each tick
// -----------------------------------------------------------------------------

void WI_Ticker (void)
{
	// [JN] Make PAUSE working properly on intermission screen
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
// WI_Drawer
// -----------------------------------------------------------------------------

static void WI_initVariables (wbstartstruct_t *wbstartstruct)
{
	wbs = wbstartstruct;
	acceleratestage = 0;
	cnt = bcnt = 0;
	me = wbs->pnum;
	plrs = wbs->plyr;

	if (!wbs->maxkills)
	wbs->maxkills = 1;

	if (!wbs->maxitems)
	wbs->maxitems = 1;

	if (!wbs->maxsecret)
	wbs->maxsecret = 1;

	wbs->epsd -= 3;
}

// -----------------------------------------------------------------------------
// WI_Start
// -----------------------------------------------------------------------------

void WI_Start (wbstartstruct_t *wbstartstruct)
{
	WI_initVariables(wbstartstruct);
	WI_initStats();
}

// -----------------------------------------------------------------------------
// WI_Init
// -----------------------------------------------------------------------------

void WI_Init (void)
{
	char name[9];

	// numbers 0-9
    for (int i = 0 ; i < 10 ; i++)
    {
		snprintf(name, 9, "NUM_%d", i);
		num[i] = W_CacheLumpName(name, PU_STATIC);
	}

	// ":"
	colon = W_CacheLumpName("COLUMN", PU_STATIC);

	// percent sign
	percent = W_CacheLumpName("PERCENT", PU_STATIC);
}
