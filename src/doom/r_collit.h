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


// Colored colormap lumps
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
extern const byte C_B30202[];
extern const byte C_B87A15[];
extern const byte C_FFD000[];
extern const byte C_FFDE4C[];
extern const byte C_FFF588[];
extern const byte C_043E8B[];
extern const byte C_5B4318[];
extern const byte C_4F5D8B[];
extern const byte C_D46D3D[];
extern const byte C_04918B[];
extern const byte C_FF3030[];
extern const byte C_311A59[];
extern const byte C_FFAFAF[];
extern const byte C_ECB866[];
extern const byte C_C63F23[];
extern const byte C_9BC8CD[];
extern const byte C_666666[];
extern const byte C_777777[];


// Main colormaps initialization functions
extern void R_AllocateColoredColormaps (void);
extern void R_InitColoredColormaps (const byte k, const float scale, const int j);

// Colored visplanes initialization functions
extern void R_ColoredZLightFreeI (const int i);
extern void R_ColoredZLightFree (void);
extern void R_ColoredZLightMalloc (void);
extern void R_ColoredZLightMAXLIGHTZ (const int i);
extern void R_ColoredZLightLevels (const int i, const int j, const int level);

// Colored segments/sprites initialization functions
extern void R_ColoredScLightFreeI (const int i);
extern void R_ColoredScLightFree (void);
extern void R_ColoredScLightMalloc (void);
extern void R_ColoredScLightMAXLIGHTSCALE (int i);
extern void R_ColoredScLightLevels (const int i, const int j, const int level);

// Coloring lookup tables
extern lighttable_t **R_ColoredVisplanesColorize (const int light, const int color);
extern lighttable_t **R_ColoredSegsColorize (const int lightnum, const int color);
extern lighttable_t  *R_ColoredSprColorize (const int color);

// Colored lighting injection tables
typedef struct
{
    int map;
    int sector;
    int color;
} sectorcolor_t;

extern const sectorcolor_t *sectorcolor;
extern void  P_SetSectorColorTable (int area);


