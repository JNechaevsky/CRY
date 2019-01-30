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


#ifndef __R_MAIN__
#define __R_MAIN__

#include "d_player.h"
#include "r_data.h"


//
// POV related.
//
extern fixed_t viewcos;
extern fixed_t viewsin;

extern int viewwindowx;
extern int viewwindowy;
extern int centerx;
extern int centery;

extern fixed_t centerxfrac;
extern fixed_t centeryfrac;
extern fixed_t projection;

extern int validcount;
extern int linecount;
extern int loopcount;


//
// Lighting LUT.
// Used for z-depth cuing per column/row,
//  and other lighting effects (sector ambient, flash).
//

// Lighting constants.
// Now why not 32 levels here?
#define LIGHTLEVELS         16
#define LIGHTSEGSHIFT       4

#define MAXLIGHTSCALE       48
#define LIGHTSCALESHIFT     12

// [crispy] & [Julia] smoother diminished lighting
#define MAXLIGHTZ           1024
#define LIGHTZSHIFT         17


extern lighttable_t *scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *scalelightfixed[MAXLIGHTSCALE];
extern lighttable_t *zlight[LIGHTLEVELS][MAXLIGHTZ];

// [Julia] Brightmaps
extern lighttable_t *fullbright_dimmeditems[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *fullbright_redonly[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *fullbright_brighttan[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *fixedcolormap;

extern int extralight;


// Number of diminishing brightness levels.
// There a 0-31, i.e. 32 LUT in the COLORMAP lump.
//
// [Julia] Jaguar: reduced from 32 to 28, to make a dark areas brighten.
// Note: there are no COLORMAP in Jaguar version at all, and CRY effect
// seems to be making some dark areas too well-lit. But seriously, this
// game and this palette are deserved to have a dark areas, but
// not too bright ones, because of things are initially lit.
// So I think using a value of 28 is quite a resonable compromise.
//  
#define NUMCOLORMAPS		28


// [AM] Fractional part of the current tic, in the half-open
//      range of [0.0, 1.0).  Used for interpolation.
extern fixed_t fractionaltic;


// Blocky/low detail mode.
//B remove this?
//  0 = high, 1 = low
extern int detailshift;	


//
// Function pointers to switch refresh/drawing functions.
// Used to select shadow mode etc.
//
extern void	(*colfunc) (void);
extern void	(*transcolfunc) (void);
extern void	(*basecolfunc) (void);
extern void	(*fuzzcolfunc) (void);
extern void	(*tlcolfunc) (void);
// No shadow effects on floors.
extern void (*spanfunc) (void);


//
// Utility functions.
int R_PointOnSide (fixed_t x, fixed_t y, node_t* node);
int R_PointOnSegSide (fixed_t x, fixed_t y, seg_t* line);

angle_t R_PointToAngle (fixed_t x, fixed_t y);
angle_t R_PointToAngle2 (fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);

fixed_t R_PointToDist (fixed_t x, fixed_t y);
fixed_t R_ScaleFromGlobalAngle (angle_t visangle);
subsector_t *R_PointInSubsector (fixed_t x, fixed_t y);

void R_AddPointToBox (int x, int y, fixed_t *box);


// [AM] Interpolate between two angles.
angle_t R_InterpolateAngle(angle_t oangle, angle_t nangle, fixed_t scale);


//
// REFRESH - the actual rendering functions.
//

// Called by G_Drawer.
void R_RenderPlayerView (player_t *player);

// Called by startup code.
void R_Init (void);

// Called by M_Responder.
void R_SetViewSize (int blocks, int detail);

// [Julia] Brightmaps
void R_InitBrightmaps (void);

#endif

