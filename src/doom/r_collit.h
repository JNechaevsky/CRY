//
// Copyright(C) 2015-2024 Fabian Greffrath
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
extern const byte C_EEC06B_CRY[], C_EEC06B_DOOM[];
extern const byte C_D97C45_CRY[], C_D97C45_DOOM[];
extern const byte C_FF7F7F_CRY[], C_FF7F7F_DOOM[];
extern const byte C_55B828_CRY[], C_55B828_DOOM[];
extern const byte C_BBE357_CRY[], C_BBE357_DOOM[];
extern const byte C_949DB9_CRY[], C_949DB9_DOOM[];
extern const byte C_2B3BFF_CRY[], C_2B3BFF_DOOM[];
extern const byte C_50ADAC_CRY[], C_50ADAC_DOOM[];
extern const byte C_CCE4A5_CRY[], C_CCE4A5_DOOM[];
extern const byte C_CCEA5F_CRY[], C_CCEA5F_DOOM[];
extern const byte C_B30202_CRY[], C_B30202_DOOM[];
extern const byte C_B87A15_CRY[], C_B87A15_DOOM[];
extern const byte C_FFD000_CRY[], C_FFD000_DOOM[];
extern const byte C_FFDE4C_CRY[], C_FFDE4C_DOOM[];
extern const byte C_FFF588_CRY[], C_FFF588_DOOM[];
extern const byte C_3089FF_CRY[], C_3089FF_DOOM[];
extern const byte C_A88139_CRY[], C_A88139_DOOM[];
extern const byte C_7084C4_CRY[], C_7084C4_DOOM[];
extern const byte C_D46D3D_CRY[], C_D46D3D_DOOM[];
extern const byte C_05A8A0_CRY[], C_05A8A0_DOOM[];
extern const byte C_FF3030_CRY[], C_FF3030_DOOM[];
extern const byte C_6435B5_CRY[], C_6435B5_DOOM[];
extern const byte C_FFAFAF_CRY[], C_FFAFAF_DOOM[];
extern const byte C_ECB866_CRY[], C_ECB866_DOOM[];
extern const byte C_C63F23_CRY[], C_C63F23_DOOM[];
extern const byte C_9BC8CD_CRY[], C_9BC8CD_DOOM[];
extern const byte C_666666_CRY[], C_666666_DOOM[];
extern const byte C_777777_CRY[], C_777777_DOOM[];

extern const byte C_CEA98B_CRY[], C_CEA98B_DOOM[];
extern const byte C_FFCD5A_CRY[], C_FFCD5A_DOOM[];
extern const byte C_AC785B_CRY[], C_AC785B_DOOM[];


// Composing functions
extern void R_AllocateColoredColormaps (void);
extern void R_GenerateColoredColormaps (const byte k, const float scale, const int j);

// Initialization functions
extern void R_InitColoredLightTables (void);
extern void R_GenerateColoredSClights (const int width);

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


