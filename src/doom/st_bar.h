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
// DESCRIPTION:
//	Status bar code.
//	Does the face/direction indicator animatin.
//	Does palette indicators as well (red pain/berserk, bright pickup)
//

#ifndef __STSTUFF_H__
#define __STSTUFF_H__

#include "doomtype.h"
#include "d_event.h"

// Size of statusbar.
// Now sensitive for scaling.
#define ST_HEIGHT	40
#define ST_WIDTH	ORIGWIDTH
#define ST_Y		(ORIGHEIGHT - ST_HEIGHT)


//
// STATUS BAR
//

// Called by main loop.
boolean ST_Responder (event_t* ev);

// Called by main loop.
void ST_Ticker (void);

// Called by main loop.
void ST_Drawer (boolean force);

// Called when the console player is spawned on each level.
void ST_Start (void);

// Called by startup code.
void ST_Init (void);

// [JN] Allocate rectangle sizes for status bar buffered drawing.
extern void ST_InitElementsBackground (void);

extern int st_palette;
extern boolean st_fullupdate;

//
// CHEAT SEQUENCE PACKAGE
//

#define CHEAT(value, parameters) \
    { value, sizeof(value) - 1, parameters, 0, 0, "" }

#define MAX_CHEAT_LEN 25
#define MAX_CHEAT_PARAMS 5

typedef struct
{
    // settings for this cheat

    char sequence[MAX_CHEAT_LEN];
    size_t sequence_len;
    int parameter_chars;

    // state used during the game

    size_t chars_read;
    int param_chars_read;
    char parameter_buf[MAX_CHEAT_PARAMS];
} cheatseq_t;

extern cheatseq_t cheat_mus;
extern cheatseq_t cheat_god;
extern cheatseq_t cheat_ammo;
extern cheatseq_t cheat_ammonokey;
extern cheatseq_t cheat_noclip;
extern cheatseq_t cheat_commercial_noclip;
extern cheatseq_t cheat_powerup[7];
extern cheatseq_t cheat_choppers;
extern cheatseq_t cheat_clev;
extern cheatseq_t cheat_mypos;

#endif
