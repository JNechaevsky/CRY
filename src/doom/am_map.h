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

#pragma once


#include "d_event.h"
#include "m_cheat.h"


typedef struct
{
    int64_t x,y;
} mpoint_t;

extern boolean AM_Responder (event_t *ev);

extern cheatseq_t cheat_amap;

extern int iddt_cheating;
extern int markpointnum, markpointnum_max;

extern mpoint_t *markpoints;

extern void AM_clearMarks (void);
extern void AM_Drawer (void);
extern void AM_Init (void);
extern void AM_initVariables (void);
extern void AM_LevelInit (boolean reinit);
extern void AM_LevelNameDrawer (void);
extern void AM_SetdrawFline (void);
extern void AM_Start (void);
extern void AM_Stop (void);
extern void AM_Ticker (void);
