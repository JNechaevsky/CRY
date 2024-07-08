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
extern lighttable_t   *colormaps_D97C45;
extern lighttable_t   *colormaps_FF7F7F;
extern lighttable_t   *colormaps_55B828;
extern lighttable_t   *colormaps_BBE357;
extern lighttable_t   *colormaps_949DB9;
extern lighttable_t   *colormaps_2A2F6B;
extern lighttable_t   *colormaps_50ADAC;
extern lighttable_t   *colormaps_CCE4A5;
extern lighttable_t   *colormaps_CCEA5F;

// Visplanes data
extern lighttable_t ***zlight_EEC06B;
extern lighttable_t ***zlight_D97C45;
extern lighttable_t ***zlight_FF7F7F;
extern lighttable_t ***zlight_55B828;
extern lighttable_t ***zlight_BBE357;
extern lighttable_t ***zlight_949DB9;
extern lighttable_t ***zlight_2A2F6B;
extern lighttable_t ***zlight_50ADAC;
extern lighttable_t ***zlight_CCE4A5;
extern lighttable_t ***zlight_CCEA5F;

// Segments data
extern lighttable_t ***scalelight_EEC06B;
extern lighttable_t ***scalelight_D97C45;
extern lighttable_t ***scalelight_FF7F7F;
extern lighttable_t ***scalelight_55B828;
extern lighttable_t ***scalelight_BBE357;
extern lighttable_t ***scalelight_949DB9;
extern lighttable_t ***scalelight_2A2F6B;
extern lighttable_t ***scalelight_50ADAC;
extern lighttable_t ***scalelight_CCE4A5;
extern lighttable_t ***scalelight_CCEA5F;


// Main colormaps
extern void R_AllocateColoredColormaps (void);
extern void R_InitColoredColormap (byte k, const float scale, const byte *colormap_name);
extern const int R_CalculateColoredColormap (void);

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

// Sprites coloring
extern lighttable_t  *R_ColoredSprColorize (int color);


// Colored lighting injection data type
typedef struct
{
    int map;
    int sector;
    int color;
} sectorcolor_t;

extern const sectorcolor_t sectorcolor[];

// Colored colormaps
extern const byte C_EEC06B[];
extern const byte C_D97C45[];
extern const byte C_FF7F7F[];
extern const byte C_55B828[];
extern const byte C_BBE357[];
extern const byte C_949DB9[];
extern const byte C_2A2F6B[];
extern const byte C_50ADAC[];
extern const byte C_CCE4A5[];
extern const byte C_CCEA5F[];
