//
// Copyright(C) 2013-2024 Brad Harding
// Copyright(C) 2024 Julia Nechaevskaya
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

#include "r_local.h"


// Main colormaps
extern lighttable_t   *colormaps_EEC06B;
extern lighttable_t   *colormaps_FF7F7F;

// Visplanes data
extern lighttable_t ***zlight_EEC06B;
extern lighttable_t ***zlight_FF7F7F;

// Segments data
extern lighttable_t ***scalelight_EEC06B;
extern lighttable_t ***scalelight_FF7F7F;


// Main colormaps
extern void R_AllocateColoredColormaps (void);

// Colored visplanes initialization
extern void R_ColoredVisplanesFreeI (int i);
extern void R_ColoredVisplanesFree (void);
extern void R_ColoredVisplanesMalloc (void);
extern void R_ColoredVisplanesMAXLIGHTZ (int i);
extern void R_ColoredVisplanesIJLevel (int i, int j, int level);

// Colored segments initialization
extern void R_ColoredSegsFreeI (int i);
extern void R_ColoredSegsFree (void);
extern void R_ColoredSegsMalloc (void);
extern void R_ColoredSegsMAXLIGHTSCALE (int i);
extern void R_ColoredSegsIJLevel (int i, int j, int level);


// Visplanes coloring
extern lighttable_t **R_ColoredVisplanesColorize (int light, int color);

// Segments coloring
extern lighttable_t **R_ColoredSegsColorize (int lightnum, int color);


// Colored lighting injection data type
typedef struct
{
    int map;
    int sector;
    int color;
} sectorcolor_t;

extern const sectorcolor_t sectorcolor[];
