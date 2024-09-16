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
//   Duh.
// 


#pragma once

#include "doomdef.h"
#include "d_event.h"
#include "d_ticcmd.h"


extern boolean G_CheckDemoStatus (void);
extern boolean G_Responder (event_t *ev);
void G_FastResponder (void); // [crispy]
void G_PrepTiccmd (void); // [crispy]

extern fixed_t forwardmove[2];
extern fixed_t sidemove[2];


extern void G_BuildTiccmd (ticcmd_t *cmd, int maketic); 
extern void G_DeferedInitNew (skill_t skill, int episode, int map);
extern void G_DoCompleted (void); 
extern void G_DoLoadGame (void);
extern void G_DoLoadLevel (void); 
extern void G_DoNewGame (void); 
extern void G_DoPlayDemo (void); 
extern void G_DoReborn (int playernum); 
extern void G_DoSaveGame (void); 
extern void G_DoVictory (void); 
extern void G_DoWorldDone (void); 
extern void G_DrawMouseSpeedBox (void);
extern void G_ExitLevel (void);
extern void G_InitNew (skill_t skill, int episode, int map);
extern void G_InitSkyTextures (void); 
extern void G_LoadGame (char *name);
extern void G_PlayerReborn (int player);
extern void G_SaveGame (int slot, char *description);
extern void G_ScreenShot (void);
extern void G_SecretExitLevel (void);
extern void G_Ticker (void);
extern void G_WorldDone (void);

// [crispy] holding down the "Run" key may trigger special behavior
extern boolean speedkeydown (void);
