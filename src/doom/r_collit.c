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


#include <stdlib.h>

#include "z_zone.h"
#include "doomstat.h"
#include "r_collit.h"

#include "id_vars.h"


// =============================================================================
//
//                            COLORED SECTOR LIGHTING
//
// =============================================================================

// Main colormaps
lighttable_t   *colormaps_EEC06B;  // Bright yellow/gold
lighttable_t   *colormaps_D97C45;  // Middle yellow/gold (also E29A56)
lighttable_t   *colormaps_FF7F7F;  // Bright red
lighttable_t   *colormaps_55B828;  // Bright green
lighttable_t   *colormaps_BBE357;  // Slime green
lighttable_t   *colormaps_949DB9;  // Bright desaturated blue (also 6B779E)
lighttable_t   *colormaps_2A2F6B;  // Middle desaturated blue (also 032149)
lighttable_t   *colormaps_50ADAC;  // Middle cyan (also 31A29F)
lighttable_t   *colormaps_CCE4A5;  // Middle green-yellow
lighttable_t   *colormaps_CCEA5F;  // Bright green-yellow
lighttable_t   *colormaps_B30202;  // Middle red
lighttable_t   *colormaps_B87A15;  // Middle orange
lighttable_t   *colormaps_FFD000;  // Middle yellow
lighttable_t   *colormaps_FFDE4C;  // Middle-bright yellow
lighttable_t   *colormaps_FFF588;  // Bright yellow
lighttable_t   *colormaps_043E8B;  // Un-darked cyanic blue
lighttable_t   *colormaps_5B4318;  // Dark brown (also 74561F)
lighttable_t   *colormaps_4F5D8B;  // Dark cyanic blue 2
lighttable_t   *colormaps_D46D3D;  // Middle orange 2
lighttable_t   *colormaps_04918B;  // Middle saturated cyan
lighttable_t   *colormaps_FF3030;  // Bright saturated red
lighttable_t   *colormaps_311A59;  // Un-darked magenta
lighttable_t   *colormaps_FFAFAF;  // Brighter red (also FFCECE)
lighttable_t   *colormaps_ECB866;  // Bright orange
lighttable_t   *colormaps_C63F23;  // Middle orange 3
lighttable_t   *colormaps_9BC8CD;  // Bright cyan
lighttable_t   *colormaps_666666;  // Special green (final level)
lighttable_t   *colormaps_777777;  // Special red (final level)

// Visplane light tables
lighttable_t ***zlight_EEC06B = NULL;
lighttable_t ***zlight_D97C45 = NULL;
lighttable_t ***zlight_FF7F7F = NULL;
lighttable_t ***zlight_55B828 = NULL;
lighttable_t ***zlight_BBE357 = NULL;
lighttable_t ***zlight_949DB9 = NULL;
lighttable_t ***zlight_2A2F6B = NULL;
lighttable_t ***zlight_50ADAC = NULL;
lighttable_t ***zlight_CCE4A5 = NULL;
lighttable_t ***zlight_CCEA5F = NULL;
lighttable_t ***zlight_B30202 = NULL;
lighttable_t ***zlight_B87A15 = NULL;
lighttable_t ***zlight_FFD000 = NULL;
lighttable_t ***zlight_FFDE4C = NULL;
lighttable_t ***zlight_FFF588 = NULL;
lighttable_t ***zlight_043E8B = NULL;
lighttable_t ***zlight_5B4318 = NULL;
lighttable_t ***zlight_4F5D8B = NULL;
lighttable_t ***zlight_D46D3D = NULL;
lighttable_t ***zlight_04918B = NULL;
lighttable_t ***zlight_FF3030 = NULL;
lighttable_t ***zlight_311A59 = NULL;
lighttable_t ***zlight_FFAFAF = NULL;
lighttable_t ***zlight_ECB866 = NULL;
lighttable_t ***zlight_C63F23 = NULL;
lighttable_t ***zlight_9BC8CD = NULL;
lighttable_t ***zlight_666666 = NULL;
lighttable_t ***zlight_777777 = NULL;

// Segment/sprite light tables
lighttable_t ***scalelight_EEC06B = NULL;
lighttable_t ***scalelight_D97C45 = NULL;
lighttable_t ***scalelight_FF7F7F = NULL;
lighttable_t ***scalelight_55B828 = NULL;
lighttable_t ***scalelight_BBE357 = NULL;
lighttable_t ***scalelight_949DB9 = NULL;
lighttable_t ***scalelight_2A2F6B = NULL;
lighttable_t ***scalelight_50ADAC = NULL;
lighttable_t ***scalelight_CCE4A5 = NULL;
lighttable_t ***scalelight_CCEA5F = NULL;
lighttable_t ***scalelight_B30202 = NULL;
lighttable_t ***scalelight_B87A15 = NULL;
lighttable_t ***scalelight_FFD000 = NULL;
lighttable_t ***scalelight_FFDE4C = NULL;
lighttable_t ***scalelight_FFF588 = NULL;
lighttable_t ***scalelight_043E8B = NULL;
lighttable_t ***scalelight_5B4318 = NULL;
lighttable_t ***scalelight_4F5D8B = NULL;
lighttable_t ***scalelight_D46D3D = NULL;
lighttable_t ***scalelight_04918B = NULL;
lighttable_t ***scalelight_FF3030 = NULL;
lighttable_t ***scalelight_311A59 = NULL;
lighttable_t ***scalelight_FFAFAF = NULL;
lighttable_t ***scalelight_ECB866 = NULL;
lighttable_t ***scalelight_C63F23 = NULL;
lighttable_t ***scalelight_9BC8CD = NULL;
lighttable_t ***scalelight_666666 = NULL;
lighttable_t ***scalelight_777777 = NULL;


// =============================================================================
//
//                   MAIN COLORMAPS INITIALIZATION FUNCTIONS
//
// =============================================================================

void R_AllocateColoredColormaps (void)
{
    colormaps_EEC06B = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_D97C45 = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_FF7F7F = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_55B828 = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_BBE357 = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_949DB9 = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_2A2F6B = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_50ADAC = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_CCE4A5 = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_CCEA5F = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_B30202 = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_B87A15 = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_FFD000 = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_FFDE4C = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_FFF588 = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_043E8B = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_5B4318 = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_4F5D8B = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_D46D3D = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_04918B = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_FF3030 = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_311A59 = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_FFAFAF = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_ECB866 = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_C63F23 = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_9BC8CD = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_666666 = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    colormaps_777777 = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
}

static int r_clrmp_color, g_clrmp_color, b_clrmp_color;

static void R_InitColoredColormap (const byte k, const float scale, const byte *lump_name, lighttable_t *colormap_name, const int j)
{
    r_clrmp_color = gammatable[vid_gamma][lump_name[3 * k + 0]] * (1. - scale) + gammatable[vid_gamma][0] * scale;
    g_clrmp_color = gammatable[vid_gamma][lump_name[3 * k + 1]] * (1. - scale) + gammatable[vid_gamma][0] * scale;
    b_clrmp_color = gammatable[vid_gamma][lump_name[3 * k + 2]] * (1. - scale) + gammatable[vid_gamma][0] * scale;
    
    colormap_name[j] = 0xff000000 | (r_clrmp_color << 16) | (g_clrmp_color << 8) | b_clrmp_color;
}

void R_InitColoredColormaps (const byte k, const float scale, const int j)
{
    R_InitColoredColormap(k, scale, C_EEC06B, colormaps_EEC06B, j);
    R_InitColoredColormap(k, scale, C_D97C45, colormaps_D97C45, j);
    R_InitColoredColormap(k, scale, C_FF7F7F, colormaps_FF7F7F, j);
    R_InitColoredColormap(k, scale, C_55B828, colormaps_55B828, j);
    R_InitColoredColormap(k, scale, C_BBE357, colormaps_BBE357, j);
    R_InitColoredColormap(k, scale, C_949DB9, colormaps_949DB9, j);
    R_InitColoredColormap(k, scale, C_2A2F6B, colormaps_2A2F6B, j);
    R_InitColoredColormap(k, scale, C_50ADAC, colormaps_50ADAC, j);
    R_InitColoredColormap(k, scale, C_CCE4A5, colormaps_CCE4A5, j);
    R_InitColoredColormap(k, scale, C_CCEA5F, colormaps_CCEA5F, j);
    R_InitColoredColormap(k, scale, C_B30202, colormaps_B30202, j);
    R_InitColoredColormap(k, scale, C_B87A15, colormaps_B87A15, j);
    R_InitColoredColormap(k, scale, C_FFD000, colormaps_FFD000, j);
    R_InitColoredColormap(k, scale, C_FFDE4C, colormaps_FFDE4C, j);
    R_InitColoredColormap(k, scale, C_FFF588, colormaps_FFF588, j);
    R_InitColoredColormap(k, scale, C_043E8B, colormaps_043E8B, j);
    R_InitColoredColormap(k, scale, C_5B4318, colormaps_5B4318, j);
    R_InitColoredColormap(k, scale, C_4F5D8B, colormaps_4F5D8B, j);
    R_InitColoredColormap(k, scale, C_D46D3D, colormaps_D46D3D, j);
    R_InitColoredColormap(k, scale, C_04918B, colormaps_04918B, j);
    R_InitColoredColormap(k, scale, C_FF3030, colormaps_FF3030, j);
    R_InitColoredColormap(k, scale, C_311A59, colormaps_311A59, j);
    R_InitColoredColormap(k, scale, C_FFAFAF, colormaps_FFAFAF, j);
    R_InitColoredColormap(k, scale, C_ECB866, colormaps_C63F23, j);
    R_InitColoredColormap(k, scale, C_C63F23, colormaps_ECB866, j);
    R_InitColoredColormap(k, scale, C_9BC8CD, colormaps_9BC8CD, j);
    R_InitColoredColormap(k, scale, C_666666, colormaps_777777, j);
    R_InitColoredColormap(k, scale, C_777777, colormaps_666666, j);
}

// =============================================================================
//
//                  COLORED VISPLANES INITIALIZATION FUNCTIONS
//
// =============================================================================

void R_ColoredZLightFreeI (const int i)
{
    free(zlight_EEC06B[i]);
    free(zlight_D97C45[i]);
    free(zlight_FF7F7F[i]);
    free(zlight_55B828[i]);
    free(zlight_BBE357[i]);
    free(zlight_949DB9[i]);
    free(zlight_2A2F6B[i]);
    free(zlight_50ADAC[i]);
    free(zlight_CCE4A5[i]);
    free(zlight_CCEA5F[i]);
    free(zlight_B30202[i]);
    free(zlight_B87A15[i]);
    free(zlight_FFD000[i]);
    free(zlight_FFDE4C[i]);
    free(zlight_FFF588[i]);
    free(zlight_043E8B[i]);
    free(zlight_5B4318[i]);
    free(zlight_4F5D8B[i]);
    free(zlight_D46D3D[i]);
    free(zlight_04918B[i]);
    free(zlight_FF3030[i]);
    free(zlight_311A59[i]);
    free(zlight_FFAFAF[i]);
    free(zlight_ECB866[i]);
    free(zlight_C63F23[i]);
    free(zlight_9BC8CD[i]);
    free(zlight_666666[i]);
    free(zlight_777777[i]);
}

void R_ColoredZLightFree (void)
{
    free(zlight_EEC06B);
    free(zlight_D97C45);
    free(zlight_FF7F7F);
    free(zlight_55B828);
    free(zlight_BBE357);
    free(zlight_949DB9);
    free(zlight_2A2F6B);
    free(zlight_50ADAC);
    free(zlight_CCE4A5);
    free(zlight_CCEA5F);
    free(zlight_B30202);
    free(zlight_B87A15);
    free(zlight_FFD000);
    free(zlight_FFDE4C);
    free(zlight_FFF588);
    free(zlight_043E8B);
    free(zlight_5B4318);
    free(zlight_4F5D8B);
    free(zlight_D46D3D);
    free(zlight_04918B);
    free(zlight_FF3030);
    free(zlight_311A59);
    free(zlight_FFAFAF);
    free(zlight_ECB866);
    free(zlight_C63F23);
    free(zlight_9BC8CD);
    free(zlight_666666);
    free(zlight_777777);
}

void R_ColoredZLightMalloc (void)
{
    zlight_EEC06B = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_D97C45 = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_FF7F7F = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_55B828 = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_BBE357 = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_949DB9 = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_2A2F6B = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_50ADAC = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_CCE4A5 = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_CCEA5F = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_B30202 = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_B87A15 = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_FFD000 = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_FFDE4C = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_FFF588 = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_043E8B = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_5B4318 = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_4F5D8B = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_D46D3D = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_04918B = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_FF3030 = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_311A59 = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_FFAFAF = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_ECB866 = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_C63F23 = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_9BC8CD = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_666666 = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_777777 = malloc(LIGHTLEVELS * sizeof(*zlight));
}

void R_ColoredZLightMAXLIGHTZ (const int i)
{
    zlight_EEC06B[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_D97C45[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_FF7F7F[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_55B828[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_BBE357[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_949DB9[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_2A2F6B[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_50ADAC[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_CCE4A5[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_CCEA5F[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_B30202[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_B87A15[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_FFD000[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_FFDE4C[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_FFF588[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_043E8B[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_5B4318[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_4F5D8B[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_D46D3D[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_04918B[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_FF3030[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_311A59[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_FFAFAF[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_ECB866[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_C63F23[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_9BC8CD[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_666666[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_777777[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
}

void R_ColoredZLightLevels (const int i, const int j, const int level)
{
    zlight_EEC06B[i][j] = colormaps_EEC06B + level*256;
    zlight_D97C45[i][j] = colormaps_D97C45 + level*256;
    zlight_FF7F7F[i][j] = colormaps_FF7F7F + level*256;
    zlight_55B828[i][j] = colormaps_55B828 + level*256;
    zlight_BBE357[i][j] = colormaps_BBE357 + level*256;
    zlight_949DB9[i][j] = colormaps_949DB9 + level*256;
    zlight_2A2F6B[i][j] = colormaps_2A2F6B + level*256;
    zlight_50ADAC[i][j] = colormaps_50ADAC + level*256;
    zlight_CCE4A5[i][j] = colormaps_CCE4A5 + level*256;
    zlight_CCEA5F[i][j] = colormaps_CCEA5F + level*256;
    zlight_B30202[i][j] = colormaps_B30202 + level*256;
    zlight_B87A15[i][j] = colormaps_B87A15 + level*256;
    zlight_FFD000[i][j] = colormaps_FFD000 + level*256;
    zlight_FFDE4C[i][j] = colormaps_FFDE4C + level*256;
    zlight_FFF588[i][j] = colormaps_FFF588 + level*256;
    zlight_043E8B[i][j] = colormaps_043E8B + level*256;
    zlight_5B4318[i][j] = colormaps_5B4318 + level*256;
    zlight_4F5D8B[i][j] = colormaps_4F5D8B + level*256;
    zlight_D46D3D[i][j] = colormaps_D46D3D + level*256;
    zlight_04918B[i][j] = colormaps_04918B + level*256;
    zlight_FF3030[i][j] = colormaps_FF3030 + level*256;
    zlight_311A59[i][j] = colormaps_311A59 + level*256;
    zlight_FFAFAF[i][j] = colormaps_FFAFAF + level*256;
    zlight_ECB866[i][j] = colormaps_ECB866 + level*256;
    zlight_C63F23[i][j] = colormaps_C63F23 + level*256;
    zlight_9BC8CD[i][j] = colormaps_9BC8CD + level*256;
    zlight_666666[i][j] = colormaps_777777 + level*256;
    zlight_777777[i][j] = colormaps_666666 + level*256;
}

// =============================================================================
//
//              COLORED SEGMENTS/SPRITES INITIALIZATION FUNCTIONS
//
// =============================================================================

void R_ColoredScLightFreeI (const int i)
{
    free(scalelight_EEC06B[i]);
    free(scalelight_D97C45[i]);
    free(scalelight_FF7F7F[i]);
    free(scalelight_55B828[i]);
    free(scalelight_BBE357[i]);
    free(scalelight_949DB9[i]);
    free(scalelight_2A2F6B[i]);
    free(scalelight_50ADAC[i]);
    free(scalelight_CCE4A5[i]);
    free(scalelight_CCEA5F[i]);
    free(scalelight_B30202[i]);
    free(scalelight_B87A15[i]);
    free(scalelight_FFD000[i]);
    free(scalelight_FFDE4C[i]);
    free(scalelight_FFF588[i]);
    free(scalelight_043E8B[i]);
    free(scalelight_5B4318[i]);
    free(scalelight_4F5D8B[i]);
    free(scalelight_D46D3D[i]);
    free(scalelight_04918B[i]);
    free(scalelight_FF3030[i]);
    free(scalelight_311A59[i]);
    free(scalelight_FFAFAF[i]);
    free(scalelight_ECB866[i]);
    free(scalelight_C63F23[i]);
    free(scalelight_9BC8CD[i]);
    free(scalelight_666666[i]);
    free(scalelight_777777[i]);
}

void R_ColoredScLightFree (void)
{
    free(scalelight_EEC06B);
    free(scalelight_D97C45);
    free(scalelight_FF7F7F);
    free(scalelight_55B828);
    free(scalelight_BBE357);
    free(scalelight_949DB9);
    free(scalelight_2A2F6B);
    free(scalelight_50ADAC);
    free(scalelight_CCE4A5);
    free(scalelight_CCEA5F);
    free(scalelight_B30202);
    free(scalelight_B87A15);
    free(scalelight_FFD000);
    free(scalelight_FFDE4C);
    free(scalelight_FFF588);
    free(scalelight_043E8B);
    free(scalelight_5B4318);
    free(scalelight_4F5D8B);
    free(scalelight_D46D3D);
    free(scalelight_04918B);
    free(scalelight_FF3030);
    free(scalelight_311A59);
    free(scalelight_FFAFAF);
    free(scalelight_ECB866);
    free(scalelight_C63F23);
    free(scalelight_9BC8CD);
    free(scalelight_666666);
    free(scalelight_777777);
}

void R_ColoredScLightMalloc (void)
{
    scalelight_EEC06B = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_D97C45 = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_FF7F7F = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_55B828 = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_BBE357 = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_949DB9 = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_2A2F6B = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_50ADAC = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_CCE4A5 = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_CCEA5F = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_B30202 = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_B87A15 = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_FFD000 = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_FFDE4C = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_FFF588 = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_043E8B = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_5B4318 = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_4F5D8B = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_D46D3D = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_04918B = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_FF3030 = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_311A59 = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_FFAFAF = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_ECB866 = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_C63F23 = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_9BC8CD = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_666666 = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_777777 = malloc(LIGHTLEVELS * sizeof(*scalelight));
}

void R_ColoredScLightMAXLIGHTSCALE (int i)
{
    scalelight_EEC06B[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_D97C45[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_FF7F7F[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_55B828[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_BBE357[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_949DB9[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_2A2F6B[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_50ADAC[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_CCE4A5[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_CCEA5F[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_B30202[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_B87A15[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_FFD000[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_FFDE4C[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_FFF588[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_043E8B[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_5B4318[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_4F5D8B[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_D46D3D[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_04918B[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_FF3030[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_311A59[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_FFAFAF[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_ECB866[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_C63F23[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_9BC8CD[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_666666[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_777777[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
}

void R_ColoredScLightLevels (const int i, const int j, const int level)
{
    scalelight_EEC06B[i][j] = colormaps_EEC06B + level*256;
    scalelight_D97C45[i][j] = colormaps_D97C45 + level*256;
    scalelight_FF7F7F[i][j] = colormaps_FF7F7F + level*256;
    scalelight_55B828[i][j] = colormaps_55B828 + level*256;
    scalelight_BBE357[i][j] = colormaps_BBE357 + level*256;
    scalelight_949DB9[i][j] = colormaps_949DB9 + level*256;
    scalelight_2A2F6B[i][j] = colormaps_2A2F6B + level*256;
    scalelight_50ADAC[i][j] = colormaps_50ADAC + level*256;
    scalelight_CCE4A5[i][j] = colormaps_CCE4A5 + level*256;
    scalelight_CCEA5F[i][j] = colormaps_CCEA5F + level*256;
    scalelight_B30202[i][j] = colormaps_B30202 + level*256;
    scalelight_B87A15[i][j] = colormaps_B87A15 + level*256;
    scalelight_FFD000[i][j] = colormaps_FFD000 + level*256;
    scalelight_FFDE4C[i][j] = colormaps_FFDE4C + level*256;
    scalelight_FFF588[i][j] = colormaps_FFF588 + level*256;
    scalelight_043E8B[i][j] = colormaps_043E8B + level*256;
    scalelight_5B4318[i][j] = colormaps_5B4318 + level*256;
    scalelight_4F5D8B[i][j] = colormaps_4F5D8B + level*256;
    scalelight_D46D3D[i][j] = colormaps_D46D3D + level*256;
    scalelight_04918B[i][j] = colormaps_04918B + level*256;
    scalelight_FF3030[i][j] = colormaps_FF3030 + level*256;
    scalelight_311A59[i][j] = colormaps_311A59 + level*256;
    scalelight_FFAFAF[i][j] = colormaps_FFAFAF + level*256;
    scalelight_ECB866[i][j] = colormaps_ECB866 + level*256;
    scalelight_C63F23[i][j] = colormaps_C63F23 + level*256;
    scalelight_9BC8CD[i][j] = colormaps_9BC8CD + level*256;
    scalelight_666666[i][j] = colormaps_777777 + level*256;
    scalelight_777777[i][j] = colormaps_666666 + level*256;
}

// =============================================================================
//
//                            COLORING LOOKUP TABLES
//
// =============================================================================

lighttable_t **R_ColoredVisplanesColorize (const int light, const int color)
{
    if (vis_colored_lighting)
    {
        switch (color)
        {
            case 0xEEC06B:  return zlight_EEC06B[light]; break;
            case 0xD97C45:  return zlight_D97C45[light]; break;
            case 0xFF7F7F:  return zlight_FF7F7F[light]; break;
            case 0x55B828:  return zlight_55B828[light]; break;
            case 0xBBE357:  return zlight_BBE357[light]; break;
            case 0x949DB9:  return zlight_949DB9[light]; break;
            case 0x2A2F6B:  return zlight_2A2F6B[light]; break;
            case 0x50ADAC:  return zlight_50ADAC[light]; break;
            case 0xCCE4A5:  return zlight_CCE4A5[light]; break;
            case 0xCCEA5F:  return zlight_CCEA5F[light]; break;
            case 0xB30202:  return zlight_B30202[light]; break;
            case 0xB87A15:  return zlight_B87A15[light]; break;
            case 0xFFD000:  return zlight_FFD000[light]; break;
            case 0xFFDE4C:  return zlight_FFDE4C[light]; break;
            case 0xFFF588:  return zlight_FFF588[light]; break;
            case 0x043E8B:  return zlight_043E8B[light]; break;
            case 0x5B4318:  return zlight_5B4318[light]; break;
            case 0x4F5D8B:  return zlight_4F5D8B[light]; break;
            case 0xD46D3D:  return zlight_D46D3D[light]; break;
            case 0x04918B:  return zlight_04918B[light]; break;
            case 0xFF3030:  return zlight_FF3030[light]; break;
            case 0x311A59:  return zlight_311A59[light]; break;
            case 0xFFAFAF:  return zlight_FFAFAF[light]; break;
            case 0xECB866:  return zlight_C63F23[light]; break;
            case 0xC63F23:  return zlight_ECB866[light]; break;
            case 0x9BC8CD:  return zlight_9BC8CD[light]; break;
            case 0x666666:  return zlight_666666[light]; break;
            case 0x777777:  return zlight_777777[light]; break;
        }
    }

    return zlight[light];
}

lighttable_t **R_ColoredSegsColorize (const int lightnum, const int color)
{
    const int l = LIGHTLEVELS - 1;

    if (vis_colored_lighting)
    {
        switch (color)
        {
            case 0xEEC06B:  return scalelight_EEC06B[BETWEEN(0, l, lightnum)];  break;
            case 0xD97C45:  return scalelight_D97C45[BETWEEN(0, l, lightnum)];  break;
            case 0xFF7F7F:  return scalelight_FF7F7F[BETWEEN(0, l, lightnum)];  break;
            case 0x55B828:  return scalelight_55B828[BETWEEN(0, l, lightnum)];  break;
            case 0xBBE357:  return scalelight_BBE357[BETWEEN(0, l, lightnum)];  break;
            case 0x949DB9:  return scalelight_949DB9[BETWEEN(0, l, lightnum)];  break;
            case 0x2A2F6B:  return scalelight_2A2F6B[BETWEEN(0, l, lightnum)];  break;
            case 0x50ADAC:  return scalelight_50ADAC[BETWEEN(0, l, lightnum)];  break;
            case 0xCCE4A5:  return scalelight_CCE4A5[BETWEEN(0, l, lightnum)];  break;
            case 0xCCEA5F:  return scalelight_CCEA5F[BETWEEN(0, l, lightnum)];  break;
            case 0xB30202:  return scalelight_B30202[BETWEEN(0, l, lightnum)];  break;
            case 0xB87A15:  return scalelight_B87A15[BETWEEN(0, l, lightnum)];  break;
            case 0xFFD000:  return scalelight_FFD000[BETWEEN(0, l, lightnum)];  break;
            case 0xFFDE4C:  return scalelight_FFDE4C[BETWEEN(0, l, lightnum)];  break;
            case 0xFFF588:  return scalelight_FFF588[BETWEEN(0, l, lightnum)];  break;
            case 0x043E8B:  return scalelight_043E8B[BETWEEN(0, l, lightnum)];  break;
            case 0x5B4318:  return scalelight_5B4318[BETWEEN(0, l, lightnum)];  break;
            case 0x4F5D8B:  return scalelight_4F5D8B[BETWEEN(0, l, lightnum)];  break;
            case 0xD46D3D:  return scalelight_D46D3D[BETWEEN(0, l, lightnum)];  break;
            case 0x04918B:  return scalelight_04918B[BETWEEN(0, l, lightnum)];  break;
            case 0xFF3030:  return scalelight_FF3030[BETWEEN(0, l, lightnum)];  break;
            case 0x311A59:  return scalelight_311A59[BETWEEN(0, l, lightnum)];  break;
            case 0xFFAFAF:  return scalelight_FFAFAF[BETWEEN(0, l, lightnum)];  break;
            case 0xECB866:  return scalelight_C63F23[BETWEEN(0, l, lightnum)];  break;
            case 0xC63F23:  return scalelight_ECB866[BETWEEN(0, l, lightnum)];  break;
            case 0x9BC8CD:  return scalelight_9BC8CD[BETWEEN(0, l, lightnum)];  break;
            case 0x666666:  return scalelight_666666[BETWEEN(0, l, lightnum)];  break;
            case 0x777777:  return scalelight_777777[BETWEEN(0, l, lightnum)];  break;
        }
    }

    return scalelight[BETWEEN(0, l, lightnum)]; 
}

lighttable_t *R_ColoredSprColorize (const int color)
{
    switch (color)
    {
        case 0xEEC06B:  return colormaps_EEC06B;  break;
        case 0xD97C45:  return colormaps_D97C45;  break;
        case 0xFF7F7F:  return colormaps_FF7F7F;  break;
        case 0x55B828:  return colormaps_55B828;  break;
        case 0xBBE357:  return colormaps_BBE357;  break;
        case 0x949DB9:  return colormaps_949DB9;  break;
        case 0x2A2F6B:  return colormaps_2A2F6B;  break;
        case 0x50ADAC:  return colormaps_50ADAC;  break;
        case 0xCCE4A5:  return colormaps_CCE4A5;  break;
        case 0xCCEA5F:  return colormaps_CCEA5F;  break;
        case 0xB30202:  return colormaps_B30202;  break;
        case 0xB87A15:  return colormaps_B87A15;  break;
        case 0xFFD000:  return colormaps_FFD000;  break;
        case 0xFFDE4C:  return colormaps_FFDE4C;  break;
        case 0xFFF588:  return colormaps_FFF588;  break;
        case 0x043E8B:  return colormaps_043E8B;  break;
        case 0x5B4318:  return colormaps_5B4318;  break;
        case 0x4F5D8B:  return colormaps_4F5D8B;  break;
        case 0xD46D3D:  return colormaps_D46D3D;  break;
        case 0x04918B:  return colormaps_04918B;  break;
        case 0xFF3030:  return colormaps_FF3030;  break;
        case 0x311A59:  return colormaps_311A59;  break;
        case 0xFFAFAF:  return colormaps_FFAFAF;  break;
        case 0xECB866:  return colormaps_C63F23;  break;
        case 0xC63F23:  return colormaps_ECB866;  break;
        case 0x9BC8CD:  return colormaps_9BC8CD;  break;
        case 0x666666:  return colormaps_666666;  break;
        case 0x777777:  return colormaps_777777;  break;
        default:        return colormaps;         break;
    }
}

// =============================================================================
//
//                      COLORED LIGHTING INJECTION TABLES
//
// =============================================================================

const sectorcolor_t *sectorcolor;

#define SECTORCOLOR_END    { -1, 0, 0x000000 }
static const sectorcolor_t sectorcolor_dummy[] = { SECTORCOLOR_END };

//
// Area 1: Hangar
//

static const sectorcolor_t sectorcolor_map01[] =
{
    // map, sector, color table
    {    1,      0,    0xFF7F7F },
    {    1,      1,    0xFF7F7F },
    {    1,      2,    0xFF7F7F },
    {    1,      3,    0x2A2F6B },
    {    1,      4,    0x2A2F6B },
    {    1,      5,    0x2A2F6B },
    {    1,      6,    0x2A2F6B },
    {    1,      7,    0x2A2F6B },
    {    1,      8,    0xFFF588 },
    {    1,     15,    0x55B828 },
    {    1,     17,    0x55B828 },
    {    1,     19,    0x55B828 },
    {    1,     20,    0x55B828 },
    {    1,     21,    0x55B828 },
    {    1,     22,    0x50ADAC },
    {    1,     31,    0xFFF588 },
    {    1,     33,    0x949DB9 },
    {    1,     34,    0x949DB9 },
    {    1,     35,    0x949DB9 },
    {    1,     36,    0x949DB9 },
    {    1,     37,    0x949DB9 },
    {    1,     38,    0x949DB9 },
    {    1,     39,    0xEEC06B },
    {    1,     40,    0xFFF588 },
    {    1,     41,    0x949DB9 },
    {    1,     42,    0xBBE357 },
    {    1,     47,    0x949DB9 },
    {    1,     48,    0x949DB9 },
    {    1,     53,    0xD97C45 },
    {    1,     64,    0xEEC06B },
    {    1,     65,    0xEEC06B },
    {    1,     67,    0xFF7F7F },
    {    1,     68,    0xFF7F7F },
    {    1,     69,    0xFF7F7F },
    {    1,     70,    0xFF7F7F },
    {    1,     71,    0xFF7F7F },
    {    1,     72,    0xFF7F7F },
    {    1,     73,    0xFF7F7F },
    {    1,     81,    0x55B828 },
    SECTORCOLOR_END
};

//
// Area 2: Plant
//

static const sectorcolor_t sectorcolor_map02[] =
{
    {    2,      1,    0xFF7F7F },
    {    2,      2,    0xFF7F7F },
    {    2,     30,    0x043E8B },
    {    2,     34,    0xFFF588 },
    {    2,     66,    0xCCE4A5 },
    {    2,     71,    0xCCE4A5 },
    {    2,     79,    0xB30202 },
    {    2,     84,    0xB30202 },
    {    2,     85,    0xCCE4A5 },
    {    2,     87,    0xB30202 },
    {    2,     88,    0xCCE4A5 },
    {    2,     92,    0x50ADAC },
    {    2,     93,    0x50ADAC },
    {    2,     94,    0x50ADAC },
    {    2,     95,    0x50ADAC },
    {    2,     96,    0x50ADAC },
    {    2,     97,    0x50ADAC },
    {    2,     98,    0xB87A15 },
    {    2,     99,    0xB87A15 },
    {    2,    100,    0xCCE4A5 },
    {    2,    101,    0xFFD000 },
    {    2,    102,    0xFFDE4C },
    {    2,    120,    0x55B828 },
    {    2,    121,    0xBBE357 },
    {    2,    122,    0x55B828 },
    {    2,    124,    0x55B828 },
    {    2,    132,    0xFFF588 },
    {    2,    136,    0xFFF588 },
    {    2,    137,    0xFFF588 },
    {    2,    149,    0xFF7F7F },
    {    2,    152,    0xFF7F7F },
    {    2,    157,    0xFF7F7F },
    {    2,    181,    0x55B828 },
    {    2,    182,    0xBBE357 },
    {    2,    183,    0xBBE357 },
    {    2,    189,    0xCCE4A5 },
    {    2,    190,    0xCCEA5F },
    {    2,    191,    0xFF7F7F },
    SECTORCOLOR_END
};

//
// Area 3: Toxin Refinery
//

static const sectorcolor_t sectorcolor_map03[] =
{
    {    3,      3,    0xFF7F7F },
    {    3,      4,    0xFF7F7F },
    {    3,      5,    0xFF7F7F },
    {    3,      6,    0xFF7F7F },
    {    3,      7,    0x4F5D8B },
    {    3,     10,    0x4F5D8B },
    {    3,     11,    0x4F5D8B },
    {    3,     12,    0x4F5D8B },
    {    3,     13,    0x4F5D8B },
    {    3,     22,    0xFFF588 },
    {    3,     26,    0x043E8B },
    {    3,     28,    0x949DB9 },
    {    3,     33,    0x4F5D8B },
    {    3,     34,    0x4F5D8B },
    {    3,     35,    0x4F5D8B },
    {    3,     36,    0x4F5D8B },
    {    3,     37,    0x4F5D8B },
    {    3,     38,    0x4F5D8B },
    {    3,     39,    0x4F5D8B },
    {    3,     41,    0x949DB9 },
    {    3,     45,    0xBBE357 },
    {    3,     58,    0x55B828 },
    {    3,     59,    0xBBE357 },
    {    3,     60,    0xBBE357 },
    {    3,     61,    0x55B828 },
    {    3,     62,    0x55B828 },
    {    3,     64,    0xBBE357 },
    {    3,     65,    0xBBE357 },
    {    3,     66,    0xBBE357 },
    {    3,     74,    0xFFF588 },
    {    3,     75,    0x55B828 },
    {    3,     76,    0xFFF588 },
    {    3,     77,    0x043E8B },
    {    3,     78,    0xBBE357 },
    {    3,     79,    0x043E8B },
    {    3,     80,    0xFF7F7F },
    {    3,     81,    0xFF7F7F },
    {    3,     82,    0xFF7F7F },
    {    3,     83,    0xFF7F7F },
    {    3,     92,    0xBBE357 },
    {    3,     93,    0xBBE357 },
    {    3,     96,    0x55B828 },
    {    3,     97,    0xBBE357 },
    {    3,     98,    0xBBE357 },
    {    3,    100,    0x55B828 },
    {    3,    101,    0xBBE357 },
    {    3,    108,    0xBBE357 },
    {    3,    111,    0xBBE357 },
    {    3,    114,    0x55B828 },
    {    3,    115,    0x55B828 },
    {    3,    116,    0x55B828 },
    {    3,    117,    0xBBE357 },
    {    3,    118,    0xBBE357 },
    {    3,    119,    0xFFF588 },
    {    3,    123,    0x5B4318 },
    {    3,    126,    0x043E8B },
    {    3,    128,    0x043E8B },
    {    3,    132,    0x5B4318 },
    {    3,    133,    0x5B4318 },
    {    3,    134,    0x5B4318 },
    {    3,    135,    0x5B4318 },
    {    3,    136,    0x5B4318 },
    {    3,    137,    0x5B4318 },
    {    3,    138,    0xBBE357 },
    {    3,    139,    0xBBE357 },
    {    3,    140,    0x5B4318 },
    {    3,    151,    0xBBE357 },
    {    3,    157,    0x043E8B },
    {    3,    158,    0xFFF588 },
    {    3,    159,    0xFFF588 },
    SECTORCOLOR_END
};

//
// Area 4: Command Control
//

static const sectorcolor_t sectorcolor_map04[] =
{
    {    4,     11,    0x043E8B },
    {    4,     16,    0xFF7F7F },
    {    4,     20,    0x55B828 },
    {    4,     23,    0xFFF588 },
    {    4,     24,    0xFFF588 },
    {    4,     25,    0xFFF588 },
    {    4,     27,    0xFFF588 },
    {    4,     34,    0x50ADAC },
    {    4,     35,    0x50ADAC },
    {    4,     36,    0x50ADAC },
    {    4,     37,    0x50ADAC },
    {    4,     38,    0x50ADAC },
    {    4,     42,    0x043E8B },
    {    4,     43,    0x043E8B },
    {    4,     46,    0xBBE357 },
    {    4,     47,    0xBBE357 },
    {    4,     48,    0xBBE357 },
    {    4,     51,    0xBBE357 },
    {    4,     60,    0xBBE357 },
    {    4,     61,    0x043E8B },
    {    4,     71,    0xBBE357 },
    {    4,     72,    0xBBE357 },
    {    4,     75,    0x55B828 },
    {    4,     77,    0xFFF588 },
    {    4,     92,    0xFFF588 },
    {    4,     94,    0xFFF588 },
    {    4,     96,    0xFFF588 },
    {    4,     99,    0xFFF588 },
    {    4,    101,    0x55B828 },
    {    4,    102,    0x55B828 },
    {    4,    113,    0xFFF588 },
    {    4,    114,    0xFFF588 },
    {    4,    115,    0xFFF588 },
    {    4,    118,    0xFFF588 },
    {    4,    120,    0x043E8B },
    SECTORCOLOR_END
};

//
// Area 5: Phobos Lab
//

static const sectorcolor_t sectorcolor_map05[] =
{
    {    5,      0,    0x4F5D8B },
    {    5,      3,    0xBBE357 },
    {    5,      6,    0x043E8B },
    {    5,      7,    0x043E8B },
    {    5,      9,    0x4F5D8B },
    {    5,     10,    0xFF7F7F },
    {    5,     11,    0xFF7F7F },
    {    5,     12,    0xFF7F7F },
    {    5,     14,    0x949DB9 },
    {    5,     15,    0x949DB9 },
    {    5,     16,    0x949DB9 },
    {    5,     17,    0x949DB9 },
    {    5,     18,    0x949DB9 },
    {    5,     19,    0x949DB9 },
    {    5,     21,    0x949DB9 },
    {    5,     28,    0x949DB9 },
    {    5,     31,    0x55B828 },
    {    5,     33,    0x55B828 },
    {    5,     36,    0x949DB9 },
    {    5,     37,    0x949DB9 },
    {    5,     38,    0xD46D3D },
    {    5,     39,    0xD46D3D },
    {    5,     42,    0x55B828 },
    {    5,     43,    0x55B828 },
    {    5,     46,    0x949DB9 },
    {    5,     48,    0x55B828 },
    {    5,     50,    0x55B828 },
    {    5,     63,    0x949DB9 },
    {    5,     64,    0x949DB9 },
    {    5,     65,    0x949DB9 },
    {    5,     69,    0x043E8B },
    {    5,     71,    0xB30202 },
    {    5,     72,    0xB30202 },
    {    5,     78,    0x55B828 },
    {    5,     81,    0xFFF588 },
    {    5,     99,    0x55B828 },
    {    5,    101,    0x55B828 },
    {    5,    111,    0x55B828 },
    {    5,    120,    0xFFF588 },
    {    5,    121,    0xFFF588 },
    {    5,    122,    0x043E8B },
    {    5,    123,    0x043E8B },
    SECTORCOLOR_END
};

//
// Area 6: Central Processing
//

static const sectorcolor_t sectorcolor_map06[] =
{
    {    6,      0,    0x55B828 },
    {    6,      1,    0x55B828 },
    {    6,      2,    0x2A2F6B },
    {    6,      3,    0x55B828 },
    {    6,      4,    0x55B828 },
    {    6,      5,    0x55B828 },
    {    6,      6,    0x55B828 },
    {    6,      7,    0x55B828 },
    {    6,      8,    0x2A2F6B },
    {    6,      9,    0x55B828 },
    {    6,     10,    0x043E8B },
    {    6,     12,    0x043E8B },
    {    6,     17,    0xBBE357 },
    {    6,     18,    0xBBE357 },
    {    6,     25,    0x949DB9 },
    {    6,     26,    0x949DB9 },
    {    6,     29,    0x4F5D8B },
    {    6,     30,    0x4F5D8B },
    {    6,     31,    0x4F5D8B },
    {    6,     32,    0x4F5D8B },
    {    6,     34,    0xBBE357 },
    {    6,     35,    0x55B828 },
    {    6,     36,    0x55B828 },
    {    6,     39,    0x043E8B },
    {    6,     43,    0xBBE357 },
    {    6,     48,    0x55B828 },
    {    6,     50,    0xBBE357 },
    {    6,     52,    0xFFF588 },
    {    6,     53,    0xFFF588 },
    {    6,     54,    0x55B828 },
    {    6,     55,    0xBBE357 },
    {    6,     59,    0xFF7F7F },
    {    6,     76,    0xFF7F7F },
    {    6,     77,    0xBBE357 },
    {    6,     78,    0xBBE357 },
    {    6,     83,    0x55B828 },
    {    6,     84,    0x55B828 },
    {    6,     92,    0x949DB9 },
    {    6,     93,    0x949DB9 },
    {    6,     95,    0xFFF588 },
    {    6,     96,    0xFFF588 },
    {    6,     97,    0xFFF588 },
    {    6,     98,    0xFFF588 },
    {    6,     99,    0x4F5D8B },
    {    6,    100,    0x4F5D8B },
    {    6,    101,    0x4F5D8B },
    {    6,    104,    0x55B828 },
    {    6,    105,    0x55B828 },
    {    6,    112,    0x55B828 },
    {    6,    113,    0x55B828 },
    {    6,    132,    0x4F5D8B },
    {    6,    133,    0x4F5D8B },
    {    6,    147,    0xFF7F7F },
    {    6,    148,    0xFF7F7F },
    {    6,    151,    0xFF7F7F },
    {    6,    153,    0x55B828 },
    {    6,    160,    0xBBE357 },
    {    6,    165,    0xFFF588 },
    {    6,    167,    0xFFF588 },
    {    6,    172,    0xFF7F7F },
    {    6,    173,    0xFF7F7F },
    {    6,    174,    0xFF7F7F },
    {    6,    175,    0xFF7F7F },
    {    6,    176,    0xFFF588 },
    {    6,    177,    0xFFF588 },
    {    6,    178,    0xFFF588 },
    {    6,    179,    0xFFF588 },
    {    6,    180,    0x043E8B },
    SECTORCOLOR_END
};

//
// Area 7: Computer Station
//

static const sectorcolor_t sectorcolor_map07[] =
{
    {    7,      3,    0x55B828 },
    {    7,      4,    0xBBE357 },
    {    7,      7,    0x4F5D8B },
    {    7,      8,    0x4F5D8B },
    {    7,     11,    0x4F5D8B },
    {    7,     12,    0x4F5D8B },
    {    7,     13,    0x4F5D8B },
    {    7,     14,    0x311A59 },
    {    7,     15,    0x311A59 },
    {    7,     17,    0x55B828 },
    {    7,     27,    0x55B828 },
    {    7,     28,    0x55B828 },
    {    7,     30,    0x55B828 },
    {    7,     31,    0x55B828 },
    {    7,     35,    0xFFF588 },
    {    7,     38,    0xFFF588 },
    {    7,     53,    0xBBE357 },
    {    7,     54,    0xBBE357 },
    {    7,     55,    0x55B828 },
    {    7,     56,    0xBBE357 },
    {    7,     57,    0xBBE357 },
    {    7,     59,    0xBBE357 },
    {    7,     60,    0xFF7F7F },
    {    7,     61,    0xFF3030 },
    {    7,     62,    0xFF3030 },
    {    7,     66,    0xBBE357 },
    {    7,     73,    0x043E8B },
    {    7,     74,    0x043E8B },
    {    7,     84,    0x04918B },
    {    7,     88,    0x043E8B },
    {    7,     93,    0x043E8B },
    {    7,     98,    0x4F5D8B },
    {    7,    101,    0xFFF588 },
    {    7,    102,    0xFFF588 },
    {    7,    107,    0x311A59 },
    {    7,    113,    0xFF7F7F },
    {    7,    114,    0xFF7F7F },
    {    7,    115,    0xFF7F7F },
    {    7,    116,    0xFF7F7F },
    {    7,    119,    0x043E8B },
    {    7,    133,    0x311A59 },
    {    7,    136,    0x311A59 },
    {    7,    137,    0x311A59 },
    {    7,    140,    0xFF7F7F },
    {    7,    141,    0xFF7F7F },
    {    7,    142,    0xFFF588 },
    {    7,    143,    0xFFF588 },
    {    7,    144,    0x043E8B },
    {    7,    145,    0x043E8B },
    SECTORCOLOR_END
};

//
// Area 8: Phobos Anomaly
//

static const sectorcolor_t sectorcolor_map08[] =
{
    {    8,      0,    0xFFAFAF },
    {    8,      1,    0xD97C45 },
    {    8,      3,    0xD97C45 },
    {    8,      5,    0x4F5D8B },
    {    8,      7,    0xFF3030 },
    {    8,     12,    0xD97C45 },
    {    8,     13,    0xD97C45 },
    {    8,     30,    0x043E8B },
    {    8,     52,    0x4F5D8B },
    {    8,     53,    0xB30202 },
    SECTORCOLOR_END
};

//
// Area 9: Deimos Anomaly
//

static const sectorcolor_t sectorcolor_map09[] =
{
    {    9,      1,    0x043E8B },
    {    9,      3,    0x043E8B },
    {    9,      4,    0xB30202 },
    {    9,      6,    0xFF7F7F },
    {    9,      9,    0xB30202 },
    {    9,      3,    0xFFF588 },
    {    9,     10,    0xFFAFAF },
    {    9,     13,    0xFFF588 },
    {    9,     17,    0xB30202 },
    {    9,     20,    0xFF7F7F },
    {    9,     22,    0xFF7F7F },
    {    9,     25,    0xFF7F7F },
    {    9,     26,    0xFF7F7F },
    {    9,     27,    0xFF7F7F },
    {    9,     32,    0xB30202 },
    {    9,     33,    0xECB866 },
    {    9,     35,    0xFFF588 },
    {    9,     36,    0xB30202 },
    {    9,     37,    0xC63F23 },
    {    9,     38,    0xC63F23 },
    {    9,     40,    0xECB866 },
    {    9,     41,    0xECB866 },
    {    9,     42,    0xECB866 },
    {    9,     43,    0x4F5D8B },
    {    9,     44,    0xB30202 },
    {    9,     45,    0xB30202 },
    {    9,     48,    0xB30202 },
    {    9,     50,    0xFFAFAF },
    {    9,     51,    0xFFAFAF },
    {    9,     54,    0xFFF588 },
    {    9,     57,    0xB30202 },
    {    9,     59,    0xB30202 },
    {    9,     65,    0xFFF588 },
    {    9,     66,    0xFFF588 },
    {    9,     67,    0xFFF588 },
    {    9,     68,    0xFFF588 },
    {    9,     69,    0xB30202 },
    {    9,     70,    0xC63F23 },
    {    9,     71,    0xC63F23 },
    {    9,     72,    0xC63F23 },
    {    9,     73,    0xD97C45 },
    {    9,     74,    0xB30202 },
    {    9,     75,    0xD97C45 },
    {    9,     76,    0xD97C45 },
    {    9,     77,    0xB30202 },
    {    9,     78,    0xB30202 },
    {    9,     79,    0xFFF588 },
    {    9,     80,    0x043E8B },
    {    9,     82,    0x043E8B },
    {    9,     83,    0xB30202 },
    {    9,     84,    0x4F5D8B },
    {    9,     85,    0xB30202 },
    {    9,     88,    0x043E8B },
    SECTORCOLOR_END
};

//
// Area 10: Containment Area
//

static const sectorcolor_t sectorcolor_map10[] =
{
    {   10,      0,    0x9BC8CD },
    {   10,      1,    0x9BC8CD },
    {   10,      2,    0x9BC8CD },
    {   10,     11,    0xFFF588 },
    {   10,     14,    0xFFF588 },
    {   10,     18,    0x9BC8CD },
    {   10,     19,    0x9BC8CD },
    {   10,     20,    0x9BC8CD },
    {   10,     21,    0x9BC8CD },
    {   10,     22,    0x9BC8CD },
    {   10,     23,    0x9BC8CD },
    {   10,     26,    0x50ADAC },
    {   10,     27,    0x50ADAC },
    {   10,     28,    0x50ADAC },
    {   10,     29,    0x50ADAC },
    {   10,     30,    0xFFF588 },
    {   10,     32,    0xFFF588 },
    {   10,     43,    0x50ADAC },
    {   10,     44,    0x50ADAC },
    {   10,     45,    0x50ADAC },
    {   10,     49,    0x9BC8CD },
    {   10,     50,    0x50ADAC },
    {   10,     51,    0x9BC8CD },
    {   10,     54,    0x043E8B },
    {   10,     63,    0x043E8B },
    {   10,     64,    0x043E8B },
    {   10,     65,    0x043E8B },
    {   10,     68,    0x50ADAC },
    {   10,     78,    0x9BC8CD },
    {   10,     79,    0x9BC8CD },
    {   10,     81,    0xFF7F7F },
    {   10,     83,    0x043E8B },
    {   10,     84,    0x9BC8CD },
    {   10,     86,    0x9BC8CD },
    {   10,     93,    0xFFF588 },
    {   10,     99,    0xBBE357 },
    {   10,    100,    0xBBE357 },
    {   10,    101,    0xFF7F7F },
    {   10,    102,    0xFF7F7F },
    {   10,    104,    0xFF7F7F },
    {   10,    103,    0xFF7F7F },
    {   10,    107,    0xBBE357 },
    {   10,    109,    0xFF7F7F },
    {   10,    110,    0xFF7F7F },
    {   10,    111,    0x55B828 },
    {   10,    112,    0xBBE357 },
    {   10,    113,    0xBBE357 },
    {   10,    121,    0x4F5D8B },
    {   10,    126,    0x5B4318 },
    {   10,    127,    0x5B4318 },
    {   10,    128,    0x043E8B },
    {   10,    129,    0x043E8B },
    {   10,    130,    0xFFF588 },
    SECTORCOLOR_END
};

//
// Area 11: Refinery
//

static const sectorcolor_t sectorcolor_map11[] =
{
    {   11,      3,    0xFF7F7F },
    {   11,      7,    0xFFAFAF },
    {   11,      8,    0x55B828 },
    {   11,      9,    0xBBE357 },
    {   11,     11,    0x55B828 },
    {   11,     17,    0x55B828 },
    {   11,     18,    0x55B828 },
    {   11,     19,    0x55B828 },
    {   11,     20,    0xBBE357 },
    {   11,     21,    0xBBE357 },
    {   11,     22,    0xFFAFAF },
    {   11,     27,    0x949DB9 },
    {   11,     28,    0x949DB9 },
    {   11,     34,    0x043E8B },
    {   11,     35,    0x043E8B },
    {   11,     36,    0x55B828 },
    {   11,     38,    0xBBE357 },
    {   11,     40,    0xBBE357 },
    {   11,     41,    0xBBE357 },
    {   11,     42,    0xBBE357 },
    {   11,     60,    0x55B828 },
    {   11,     62,    0x55B828 },
    {   11,     67,    0xFF7F7F },
    {   11,     83,    0x043E8B },
    {   11,     84,    0xFFAFAF },
    {   11,     90,    0xD97C45 },
    {   11,     97,    0x043E8B },
    {   11,    100,    0x043E8B },
    SECTORCOLOR_END
};

//
// Area 12: Deimos Lab
//

static const sectorcolor_t sectorcolor_map12[] =
{
    {   12,      0,    0xFFAFAF },
    {   12,      3,    0xFFAFAF },
    {   12,      5,    0xFFAFAF },
    {   12,      6,    0xFFAFAF },
    {   12,      7,    0xFFAFAF },
    {   12,      8,    0xFFAFAF },
    {   12,      9,    0xFFAFAF },
    {   12,     10,    0xFFAFAF },
    {   12,     13,    0xB30202 },
    {   12,     14,    0xFFF588 },
    {   12,     15,    0xFFF588 },
    {   12,     16,    0xFFAFAF },
    {   12,     19,    0xFFAFAF },
    {   12,     20,    0xFFAFAF },
    {   12,     21,    0xFFAFAF },
    {   12,     22,    0xFFAFAF },
    {   12,     23,    0xFF7F7F },
    {   12,     24,    0xFF7F7F },
    {   12,     25,    0xFF7F7F },
    {   12,     26,    0xFF7F7F },
    {   12,     27,    0xFFF588 },
    {   12,     28,    0x043E8B },
    {   12,     29,    0x043E8B },
    {   12,     30,    0x043E8B },
    {   12,     31,    0xFF7F7F },
    {   12,     34,    0xFF7F7F },
    {   12,     39,    0x55B828 },
    {   12,     40,    0xFFF588 },
    {   12,     41,    0xFF7F7F },
    {   12,     42,    0xFF7F7F },
    {   12,     43,    0xFFAFAF },
    {   12,     44,    0xFFAFAF },
    {   12,     45,    0xFFAFAF },
    {   12,     49,    0xBBE357 },
    {   12,     51,    0x55B828 },
    {   12,     52,    0xD97C45 },
    {   12,     53,    0x55B828 },
    {   12,     54,    0xBBE357 },
    {   12,     55,    0xBBE357 },
    {   12,     58,    0xB30202 },
    {   12,     59,    0xD97C45 },
    {   12,     60,    0xD97C45 },
    {   12,     61,    0xD97C45 },
    {   12,     63,    0xD97C45 },
    {   12,     64,    0xD97C45 },
    {   12,     65,    0xD97C45 },
    {   12,     67,    0xD97C45 },
    {   12,     70,    0xFFF588 },
    {   12,     71,    0xFFF588 },
    {   12,     73,    0xFFF588 },
    {   12,     74,    0xFFF588 },
    {   12,     77,    0x55B828 },
    {   12,     78,    0xBBE357 },
    {   12,     84,    0xB30202 },
    {   12,     85,    0xBBE357 },
    {   12,     89,    0xCCEA5F },
    {   12,     90,    0xCCEA5F },
    {   12,     92,    0xCCEA5F },
    {   12,     91,    0xCCEA5F },
    {   12,     93,    0xCCEA5F },
    {   12,     96,    0xCCEA5F },
    {   12,     98,    0xCCEA5F },
    {   12,     99,    0xCCEA5F },
    {   12,    106,    0xBBE357 },
    {   12,    108,    0xC63F23 },
    {   12,    109,    0xC63F23 },
    {   12,    110,    0xC63F23 },
    {   12,    111,    0xD97C45 },
    {   12,    112,    0xC63F23 },
    {   12,    113,    0xD97C45 },
    {   12,    132,    0xCCEA5F },
    {   12,    133,    0xCCEA5F },
    {   12,    134,    0xCCEA5F },
    {   12,    137,    0xFFF588 },
    {   12,    139,    0xCCEA5F },
    {   12,    140,    0xCCEA5F },
    {   12,    141,    0xCCEA5F },
    {   12,    142,    0xCCEA5F },
    {   12,    144,    0xBBE357 },
    {   12,    146,    0xBBE357 },
    {   12,    147,    0xFFF588 },
    {   12,    148,    0xFFF588 },
    {   12,    150,    0xB30202 },
    {   12,    151,    0xD97C45 },
    {   12,    152,    0xC63F23 },
    {   12,    153,    0xC63F23 },
    {   12,    154,    0xC63F23 },
    {   12,    155,    0xC63F23 },
    {   12,    156,    0xC63F23 },
    {   12,    157,    0xC63F23 },
    {   12,    158,    0xB30202 },
    {   12,    159,    0xFF7F7F },
    {   12,    160,    0xC63F23 },
    {   12,    161,    0xC63F23 },
    {   12,    162,    0xC63F23 },
    {   12,    163,    0xB30202 },
    {   12,    165,    0x043E8B },
    {   12,    167,    0xFF7F7F },
    {   12,    168,    0xFFF588 },
    {   12,    169,    0xFFF588 },
    {   12,    170,    0xB30202 },
    {   12,    171,    0xBBE357 },
    {   12,    172,    0xBBE357 },
    {   12,    173,    0xBBE357 },
    {   12,    174,    0xBBE357 },
    {   12,    175,    0xBBE357 },
    {   12,    176,    0xBBE357 },
    SECTORCOLOR_END
};

//
// Area 13: Command Center
//

static const sectorcolor_t sectorcolor_map13[] =
{
    {   13,      0,    0xECB866 },
    {   13,      3,    0xECB866 },
    {   13,      5,    0xEEC06B },
    {   13,     11,    0xECB866 },
    {   13,     20,    0xFFF588 },
    {   13,     21,    0xD97C45 },
    {   13,     23,    0xFFF588 },
    {   13,     25,    0xECB866 },
    {   13,     27,    0xB87A15 },
    {   13,     35,    0x949DB9 },
    {   13,     65,    0xFFF588 },
    {   13,     67,    0xCCEA5F },
    {   13,     70,    0xCCEA5F },
    {   13,     73,    0xD97C45 },
    {   13,     77,    0x2A2F6B },
    {   13,     78,    0x2A2F6B },
    {   13,     79,    0x2A2F6B },
    {   13,     80,    0x2A2F6B },
    {   13,     81,    0x2A2F6B },
    {   13,     82,    0x2A2F6B },
    {   13,     83,    0x2A2F6B },
    {   13,     84,    0x2A2F6B },
    {   13,     85,    0x2A2F6B },
    {   13,     86,    0x2A2F6B },
    {   13,     87,    0x2A2F6B },
    {   13,     88,    0xD97C45 },
    {   13,     89,    0xD97C45 },
    {   13,     95,    0x2A2F6B },
    {   13,     96,    0x2A2F6B },
    {   13,     97,    0x2A2F6B },
    {   13,     98,    0x2A2F6B },
    {   13,     99,    0x2A2F6B },
    {   13,    100,    0x2A2F6B },
    {   13,    102,    0xD97C45 },
    {   13,    103,    0xD97C45 },
    {   13,    104,    0xD97C45 },
    {   13,    105,    0xD97C45 },
    {   13,    108,    0xD97C45 },
    {   13,    109,    0xD97C45 },
    {   13,    110,    0xD97C45 },
    {   13,    112,    0xD97C45 },
    {   13,    113,    0xFFF588 },
    {   13,    114,    0xFFF588 },
    {   13,    115,    0xC63F23 },
    {   13,    116,    0xC63F23 },
    {   13,    117,    0xB87A15 },
    {   13,    118,    0xB87A15 },
    SECTORCOLOR_END
};

//
// Area 14: Halls of the Damned
//

static const sectorcolor_t sectorcolor_map14[] =
{
    {   14,      0,    0xFFF588 },
    {   14,      3,    0xFFF588 },
    {   14,      5,    0xFFF588 },
    {   14,      6,    0xFFF588 },
    {   14,      1,    0xFF7F7F },
    {   14,      2,    0xFF7F7F },
    {   14,      7,    0x55B828 },
    {   14,      8,    0xBBE357 },
    {   14,      9,    0x55B828 },
    {   14,     12,    0x043E8B },
    {   14,     17,    0xBBE357 },
    {   14,     18,    0x55B828 },
    {   14,     19,    0xBBE357 },
    {   14,     20,    0xBBE357 },
    {   14,     21,    0xBBE357 },
    {   14,     22,    0xBBE357 },
    {   14,     23,    0xBBE357 },
    {   14,     27,    0x9BC8CD },
    {   14,     28,    0x9BC8CD },
    {   14,     29,    0x9BC8CD },
    {   14,     30,    0x9BC8CD },
    {   14,     32,    0x9BC8CD },
    {   14,     33,    0xFFF588 },
    {   14,     34,    0x9BC8CD },
    {   14,     35,    0x9BC8CD },
    {   14,     36,    0x9BC8CD },
    {   14,     37,    0x9BC8CD },
    {   14,     39,    0x9BC8CD },
    {   14,     46,    0xFFF588 },
    {   14,     51,    0x50ADAC },
    {   14,     52,    0x50ADAC },
    {   14,     55,    0xFFF588 },
    {   14,     56,    0xFFF588 },
    {   14,     57,    0x50ADAC },
    {   14,     58,    0x50ADAC },
    {   14,     59,    0x50ADAC },
    {   14,     60,    0x50ADAC },
    {   14,     61,    0x50ADAC },
    {   14,     62,    0x9BC8CD },
    {   14,     63,    0x9BC8CD },
    {   14,     64,    0x50ADAC },
    {   14,     69,    0x9BC8CD },
    {   14,     70,    0x50ADAC },
    {   14,     71,    0x9BC8CD },
    {   14,     81,    0xFF7F7F },
    {   14,     82,    0xB30202 },
    {   14,     83,    0xB30202 },
    {   14,     89,    0x50ADAC },
    {   14,     90,    0x50ADAC },
    {   14,     91,    0x50ADAC },
    {   14,     92,    0x50ADAC },
    {   14,     96,    0x50ADAC },
    {   14,     97,    0x50ADAC },
    {   14,    105,    0x50ADAC },
    {   14,    111,    0x043E8B },
    {   14,    112,    0x043E8B },
    {   14,    113,    0xFF7F7F },
    {   14,    114,    0xFF7F7F },
    {   14,    115,    0xFFF588 },
    {   14,    116,    0x043E8B },
    SECTORCOLOR_END
};

//
// Area 15: Spawning Vats
//

static const sectorcolor_t sectorcolor_map15[] =
{
    {   15,      0,    0xC63F23 },
    {   15,      1,    0xB30202 },
    {   15,      2,    0xC63F23 },
    {   15,      3,    0xFF7F7F },
    {   15,      4,    0xC63F23 },
    {   15,      5,    0x55B828 },
    {   15,      6,    0x55B828 },
    {   15,      7,    0xFFAFAF },
    {   15,      8,    0x55B828 },
    {   15,      9,    0x55B828 },
    {   15,     10,    0x55B828 },
    {   15,     11,    0xBBE357 },
    {   15,     12,    0xFFAFAF },
    {   15,     13,    0xBBE357 },
    {   15,     14,    0xBBE357 },
    {   15,     15,    0xFF7F7F },
    {   15,     16,    0xBBE357 },
    {   15,     17,    0xBBE357 },
    {   15,     18,    0x55B828 },
    {   15,     19,    0x55B828 },
    {   15,     20,    0xBBE357 },
    {   15,     21,    0xC63F23 },
    {   15,     22,    0xBBE357 },
    {   15,     23,    0xC63F23 },
    {   15,     25,    0xFFAFAF },
    {   15,     28,    0xC63F23 },
    {   15,     29,    0xC63F23 },
    {   15,     30,    0xFFAFAF },
    {   15,     31,    0xFFAFAF },
    {   15,     33,    0xFFAFAF },
    {   15,     34,    0xFFAFAF },
    {   15,     35,    0xFFAFAF },
    {   15,     36,    0xFFAFAF },
    {   15,     37,    0xC63F23 },
    {   15,     38,    0xC63F23 },
    {   15,     39,    0xC63F23 },
    {   15,     40,    0xC63F23 },
    {   15,     42,    0xFFAFAF },
    {   15,     44,    0xFFAFAF },
    {   15,     50,    0xECB866 },
    {   15,     51,    0xECB866 },
    {   15,     55,    0xECB866 },
    {   15,     56,    0xECB866 },
    {   15,     60,    0xFFAFAF },
    {   15,     61,    0xFFAFAF },
    {   15,     62,    0xFFAFAF },
    {   15,     63,    0xFFAFAF },
    {   15,     66,    0xFFAFAF },
    {   15,     67,    0xFFAFAF },
    {   15,     68,    0xFFAFAF },
    {   15,     69,    0xFFAFAF },
    {   15,     71,    0xFFAFAF },
    {   15,     72,    0xFFAFAF },
    {   15,     73,    0xFFAFAF },
    {   15,     85,    0xFFAFAF },
    {   15,     88,    0xFFF588 },
    {   15,     89,    0xFF7F7F },
    {   15,     90,    0xFFF588 },
    {   15,     91,    0xFFF588 },
    {   15,     92,    0xFF7F7F },
    {   15,     93,    0xFF7F7F },
    {   15,     94,    0xBBE357 },
    {   15,     95,    0xFF7F7F },
    {   15,     96,    0xFF7F7F },
    {   15,    104,    0xFF7F7F },
    {   15,    105,    0xFF7F7F },
    {   15,    106,    0xFF7F7F },
    {   15,    107,    0xFF7F7F },
    {   15,    108,    0xCCEA5F },
    {   15,    113,    0x55B828 },
    {   15,    114,    0x55B828 },
    {   15,    115,    0x55B828 },
    {   15,    116,    0x55B828 },
    {   15,    117,    0x55B828 },
    {   15,    118,    0x55B828 },
    {   15,    119,    0x55B828 },
    {   15,    120,    0xECB866 },
    {   15,    123,    0xECB866 },
    {   15,    124,    0xECB866 },
    {   15,    125,    0xECB866 },
    {   15,    126,    0xECB866 },
    {   15,    127,    0xECB866 },
    {   15,    128,    0xECB866 },
    {   15,    129,    0xECB866 },
    SECTORCOLOR_END
};

//
// Area 16: Tower of Babel
//

static const sectorcolor_t sectorcolor_map16[] =
{
    {   16,      0,    0xFFAFAF },
    {   16,      1,    0xFFAFAF },
    {   16,      3,    0xFFAFAF },
    {   16,      2,    0xFF7F7F },
    {   16,      5,    0xFFAFAF },
    {   16,      6,    0xFFAFAF },
    {   16,      7,    0xFFAFAF },
    {   16,      9,    0xFFAFAF },
    {   16,     11,    0xFFAFAF },
    {   16,     14,    0xFFAFAF },
    {   16,     16,    0xFFAFAF },
    {   16,     17,    0xFFAFAF },
    {   16,     18,    0xFFAFAF },
    {   16,     19,    0xFFAFAF },
    {   16,     20,    0xFFAFAF },
    {   16,     21,    0xFFAFAF },
    {   16,     22,    0xEEC06B },
    {   16,     23,    0xEEC06B },
    {   16,     26,    0xFFAFAF },
    {   16,     29,    0xEEC06B },
    {   16,     32,    0xEEC06B },
    SECTORCOLOR_END
};

//
// Area 17: Hell Keep
//

static const sectorcolor_t sectorcolor_map17[] =
{
    {   17,      7,    0xFFAFAF },
    {   17,      8,    0xFFAFAF },
    {   17,      9,    0xFFAFAF },
    {   17,     10,    0xFFAFAF },
    {   17,     11,    0xFFAFAF },
    {   17,     12,    0xFFAFAF },
    {   17,     13,    0xFFAFAF },
    {   17,     14,    0xFFAFAF },
    {   17,     15,    0xFFAFAF },
    {   17,     16,    0xFFAFAF },
    {   17,     19,    0x043E8B },
    {   17,     20,    0x043E8B },
    {   17,     28,    0xFFAFAF },
    {   17,     31,    0xFFAFAF },
    {   17,     32,    0xFFAFAF },
    {   17,     33,    0xFF7F7F },
    {   17,     34,    0xFF7F7F },
    {   17,     36,    0xFFF588 },
    {   17,     37,    0xFFAFAF },
    {   17,     43,    0xFFAFAF },
    {   17,     45,    0xFFAFAF },
    {   17,     65,    0xFFAFAF },
    {   17,     76,    0xB30202 },
    {   17,     77,    0xC63F23 },
    {   17,     78,    0xD97C45 },
    {   17,     79,    0xFF7F7F },
    {   17,     81,    0xFF7F7F },
    {   17,     83,    0xC63F23 },
    {   17,     84,    0xC63F23 },
    {   17,     85,    0xC63F23 },
    {   17,     86,    0xC63F23 },
    {   17,     88,    0xC63F23 },
    {   17,     89,    0xD97C45 },
    {   17,     94,    0xFFAFAF },
    {   17,    100,    0xFFAFAF },
    {   17,    101,    0xC63F23 },
    {   17,    102,    0xC63F23 },
    {   17,    103,    0xC63F23 },
    {   17,    104,    0xC63F23 },
    {   17,    105,    0xD97C45 },
    {   17,    106,    0xFFF588 },
    SECTORCOLOR_END
};

//
// Area 18: Pandemonium
//

static const sectorcolor_t sectorcolor_map18[] =
{
    {   18,      8,    0xC63F23 },
    {   18,     12,    0x55B828 },
    {   18,     13,    0xBBE357 },
    {   18,     17,    0xFFAFAF },
    {   18,     18,    0xFFAFAF },
    {   18,     19,    0xFFAFAF },
    {   18,     32,    0xFFAFAF },
    {   18,     33,    0xFFAFAF },
    {   18,     35,    0xFFAFAF },
    {   18,     36,    0xFFAFAF },
    {   18,     37,    0xFFAFAF },
    {   18,     38,    0xFFAFAF },
    {   18,     39,    0xFFAFAF },
    {   18,     40,    0xFFAFAF },
    {   18,     41,    0xFFAFAF },
    {   18,     47,    0xCCEA5F },
    {   18,     48,    0xCCEA5F },
    {   18,     62,    0xFFAFAF },
    {   18,     63,    0xFFAFAF },
    {   18,     64,    0x949DB9 },
    {   18,     67,    0xC63F23 },
    {   18,     68,    0xECB866 },
    {   18,     70,    0xCCEA5F },
    {   18,     71,    0xECB866 },
    {   18,     72,    0xFFAFAF },
    {   18,     73,    0xFFAFAF },
    {   18,     75,    0xEEC06B },
    {   18,     77,    0xEEC06B },
    {   18,     79,    0xEEC06B },
    {   18,     81,    0x4F5D8B },
    {   18,     82,    0xD97C45 },
    {   18,     83,    0xD97C45 },
    {   18,     85,    0xEEC06B },
    {   18,     86,    0x4F5D8B },
    {   18,     87,    0x4F5D8B },
    {   18,     88,    0x4F5D8B },
    {   18,     89,    0xD97C45 },
    {   18,     90,    0x4F5D8B },
    {   18,     91,    0x4F5D8B },
    {   18,     92,    0x4F5D8B },
    {   18,     93,    0x4F5D8B },
    {   18,     97,    0x043E8B },
    {   18,     98,    0x043E8B },
    {   18,     99,    0x043E8B },
    {   18,    100,    0xFF7F7F },
    {   18,    101,    0xFF7F7F },
    {   18,    104,    0xBBE357 },
    {   18,    106,    0x4F5D8B },
    {   18,    111,    0xC63F23 },
    {   18,    112,    0xFFF588 },
    {   18,    114,    0xC63F23 },
    {   18,    115,    0xC63F23 },
    {   18,    117,    0x4F5D8B },
    {   18,    118,    0x4F5D8B },
    {   18,    119,    0x4F5D8B },
    {   18,    120,    0xEEC06B },
    SECTORCOLOR_END
};

//
// Area 19: House of Pain
//

static const sectorcolor_t sectorcolor_map19[] =
{
    {   19,      0,    0xFF7F7F },
    {   19,      1,    0xFF7F7F },
    {   19,      2,    0xFFAFAF },
    {   19,      3,    0xFFF588 },
    {   19,      4,    0xFFF588 },
    {   19,      5,    0xFFAFAF },
    {   19,      6,    0xFFAFAF },
    {   19,      7,    0xFFAFAF },
    {   19,     10,    0xFFAFAF },
    {   19,     11,    0xFFAFAF },
    {   19,     12,    0xFFAFAF },
    {   19,     13,    0xFFAFAF },
    {   19,     18,    0xC63F23 },
    {   19,     19,    0xC63F23 },
    {   19,     20,    0xFFAFAF },
    {   19,     21,    0xFFAFAF },
    {   19,     25,    0xC63F23 },
    {   19,     27,    0xB30202 },
    {   19,     28,    0xFFAFAF },
    {   19,     29,    0xFFAFAF },
    {   19,     30,    0xFFAFAF },
    {   19,     31,    0xD46D3D },
    {   19,     32,    0xD46D3D },
    {   19,     33,    0xFFAFAF },
    {   19,     35,    0xFFAFAF },
    {   19,     36,    0xFFAFAF },
    {   19,     37,    0xFFAFAF },
    {   19,     39,    0xFFAFAF },
    {   19,     40,    0xB30202 },
    {   19,     41,    0xFFAFAF },
    {   19,     42,    0xFFAFAF },
    {   19,     49,    0xB30202 },
    {   19,     51,    0xFFAFAF },
    {   19,     52,    0xFFAFAF },
    {   19,     53,    0xFFAFAF },
    {   19,     54,    0xFFAFAF },
    {   19,     55,    0xFFAFAF },
    {   19,     61,    0xB30202 },
    {   19,     64,    0x043E8B },
    {   19,     66,    0x043E8B },
    {   19,     65,    0xFF7F7F },
    {   19,     73,    0xFFAFAF },
    {   19,     77,    0xFF7F7F },
    {   19,     78,    0xFF7F7F },
    {   19,     80,    0xFFAFAF },
    {   19,     81,    0xFFAFAF },
    {   19,     86,    0x043E8B },
    {   19,     87,    0x043E8B },
    {   19,     88,    0x043E8B },
    SECTORCOLOR_END
};

//
// Area 20: Unholy Cathedral
//

static const sectorcolor_t sectorcolor_map20[] =
{
    {   20,      0,    0xFF7F7F },
    {   20,      1,    0xFF7F7F },
    {   20,      2,    0xFF7F7F },
    {   20,      4,    0xFFAFAF },
    {   20,      5,    0xFF7F7F },
    {   20,      6,    0xFFAFAF },
    {   20,      7,    0xFF7F7F },
    {   20,     10,    0xD97C45 },
    {   20,     11,    0xD97C45 },
    {   20,     12,    0xD97C45 },
    {   20,     13,    0xD97C45 },
    {   20,     15,    0xD97C45 },
    {   20,     16,    0xFF7F7F },
    {   20,     18,    0xFF7F7F },
    {   20,     19,    0xD97C45 },
    {   20,     20,    0xD97C45 },
    {   20,     21,    0xC63F23 },
    {   20,     22,    0xD97C45 },
    {   20,     24,    0xC63F23 },
    {   20,     25,    0xD97C45 },
    {   20,     26,    0xD97C45 },
    {   20,     27,    0xD97C45 },
    {   20,     30,    0xD97C45 },
    {   20,     34,    0xFF7F7F },
    {   20,     35,    0xD97C45 },
    {   20,     36,    0xD97C45 },
    {   20,     37,    0xD97C45 },
    {   20,     38,    0xFFAFAF },
    {   20,     39,    0xD97C45 },
    {   20,     40,    0xD97C45 },
    {   20,     41,    0xD97C45 },
    {   20,     42,    0xD97C45 },
    {   20,     43,    0xD97C45 },
    {   20,     44,    0xD97C45 },
    {   20,     45,    0xD97C45 },
    {   20,     46,    0xD97C45 },
    {   20,     47,    0xD97C45 },
    {   20,     48,    0xD97C45 },
    {   20,     49,    0xFF7F7F },
    {   20,     57,    0xB30202 },
    {   20,     65,    0xFFAFAF },
    {   20,     66,    0xFF7F7F },
    {   20,     72,    0xB30202 },
    {   20,     77,    0xB30202 },
    {   20,     78,    0xFF7F7F },
    {   20,     79,    0xB30202 },
    {   20,     92,    0xFF7F7F },
    {   20,     93,    0xFF7F7F },
    {   20,    102,    0xFF7F7F },
    {   20,    105,    0xFF7F7F },
    {   20,    106,    0xFF7F7F },
    {   20,    107,    0xFFAFAF },
    {   20,    108,    0xFFAFAF },
    {   20,    109,    0x043E8B },
    {   20,    110,    0xFF7F7F },
    {   20,    116,    0xFF7F7F },
    {   20,    117,    0xFF7F7F },
    {   20,    118,    0xFF7F7F },
    {   20,    120,    0xB30202 },
    {   20,    127,    0x55B828 },
    {   20,    132,    0x55B828 },
    {   20,    143,    0x55B828 },
    {   20,    144,    0x55B828 },
    {   20,    148,    0xFF7F7F },
    {   20,    149,    0xFF7F7F },
    {   20,    150,    0xFF7F7F },
    {   20,    151,    0xBBE357 },
    {   20,    152,    0xB30202 },
    {   20,    158,    0xB30202 },
    {   20,    159,    0xFF7F7F },
    {   20,    162,    0xFF7F7F },
    {   20,    168,    0x9BC8CD },
    {   20,    169,    0x9BC8CD },
    {   20,    170,    0x9BC8CD },
    {   20,    171,    0x50ADAC },
    {   20,    172,    0x50ADAC },
    {   20,    173,    0x9BC8CD },
    {   20,    174,    0x9BC8CD },
    {   20,    176,    0x9BC8CD },
    {   20,    177,    0x9BC8CD },
    {   20,    178,    0xB30202 },
    {   20,    179,    0x9BC8CD },
    {   20,    190,    0x9BC8CD },
    {   20,    191,    0xFF7F7F },
    {   20,    192,    0xFFAFAF },
    {   20,    193,    0xEEC06B },
    {   20,    194,    0xEEC06B },
    {   20,    195,    0xEEC06B },
    {   20,    199,    0xEEC06B },
    {   20,    200,    0xEEC06B },
    {   20,    202,    0xD97C45 },
    {   20,    203,    0xD97C45 },
    {   20,    204,    0xD97C45 },
    {   20,    205,    0xD97C45 },
    {   20,    206,    0xD97C45 },
    {   20,    207,    0xD97C45 },
    {   20,    208,    0xD97C45 },
    {   20,    209,    0xD97C45 },
    {   20,    210,    0xD97C45 },
    {   20,    211,    0xD97C45 },
    {   20,    212,    0xD97C45 },
    {   20,    213,    0xD97C45 },
    {   20,    214,    0xD97C45 },
    {   20,    215,    0xD97C45 },
    {   20,    216,    0xD97C45 },
    {   20,    218,    0xFF7F7F },
    {   20,    224,    0xFF7F7F },
    {   20,    226,    0xB30202 },
    {   20,    227,    0xFFAFAF },
    {   20,    228,    0x043E8B },
    {   20,    229,    0x043E8B },
    SECTORCOLOR_END
};

//
// Area 21: Mt. Erebus
//

static const sectorcolor_t sectorcolor_map21[] =
{
    {   21,      0,    0xD97C45 },
    {   21,      3,    0xFF7F7F },
    {   21,      5,    0xFF7F7F },
    {   21,     15,    0x2A2F6B },
    {   21,     16,    0xFF7F7F },
    {   21,     17,    0xFF7F7F },
    {   21,     19,    0xFF7F7F },
    {   21,     20,    0xD97C45 },
    {   21,     21,    0xFF7F7F },
    {   21,     22,    0xFF7F7F },
    {   21,     23,    0xFF7F7F },
    {   21,     24,    0xFF7F7F },
    {   21,     25,    0xFF7F7F },
    {   21,     26,    0xC63F23 },
    {   21,     27,    0xC63F23 },
    {   21,     28,    0xFF7F7F },
    {   21,     29,    0xFF7F7F },
    {   21,     30,    0xFF7F7F },
    {   21,     31,    0xD97C45 },
    {   21,     32,    0x55B828 },
    {   21,     33,    0xFF7F7F },
    {   21,     36,    0xFF7F7F },
    {   21,     37,    0xFF7F7F },
    {   21,     38,    0xD97C45 },
    {   21,     40,    0xD97C45 },
    {   21,     41,    0xFF7F7F },
    {   21,     42,    0xFF7F7F },
    {   21,     43,    0xEEC06B },
    {   21,     44,    0xEEC06B },
    {   21,     45,    0xEEC06B },
    {   21,     46,    0xEEC06B },
    {   21,     47,    0xEEC06B },
    {   21,     48,    0xFF7F7F },
    {   21,     49,    0xFF7F7F },
    {   21,     50,    0xFF7F7F },
    {   21,     51,    0xEEC06B },
    {   21,     53,    0xFF7F7F },
    {   21,     54,    0xFF7F7F },
    {   21,     55,    0xB30202 },
    {   21,     56,    0xFF7F7F },
    {   21,     57,    0xFF7F7F },
    {   21,     58,    0xFF7F7F },
    {   21,     59,    0xFF7F7F },
    {   21,     60,    0xFF7F7F },
    {   21,     61,    0xFF7F7F },
    {   21,     63,    0xB30202 },
    {   21,     64,    0xFFF588 },
    {   21,     65,    0xFFF588 },
    {   21,     67,    0xFF7F7F },
    {   21,     73,    0xB30202 },
    {   21,     74,    0xFF7F7F },
    {   21,     84,    0xFF7F7F },
    {   21,     85,    0xFF7F7F },
    {   21,     86,    0xB30202 },
    {   21,     87,    0xFF7F7F },
    {   21,    102,    0xD97C45 },
    {   21,    103,    0x55B828 },
    {   21,    104,    0x55B828 },
    {   21,    105,    0x55B828 },
    {   21,    106,    0xB30202 },
    {   21,    107,    0x55B828 },
    {   21,    108,    0x55B828 },
    {   21,    109,    0x55B828 },
    {   21,    110,    0x55B828 },
    {   21,    111,    0xFFF588 },
    {   21,    112,    0x2A2F6B },
    SECTORCOLOR_END
};

//
// Area 22: Limbo
//

static const sectorcolor_t sectorcolor_map22[] =
{
    {   22,      0,    0xC63F23 },
    {   22,      1,    0xFF7F7F },
    {   22,      2,    0xC63F23 },
    {   22,      3,    0xC63F23 },
    {   22,      4,    0xFF7F7F },
    {   22,      5,    0xFF7F7F },
    {   22,      6,    0xC63F23 },
    {   22,      7,    0xFF7F7F },
    {   22,      8,    0xFF7F7F },
    {   22,      9,    0xFF7F7F },
    {   22,     10,    0xC63F23 },
    {   22,     11,    0xFF7F7F },
    {   22,     12,    0xFF7F7F },
    {   22,     13,    0xFF7F7F },
    {   22,     14,    0xFF7F7F },
    {   22,     15,    0xFF7F7F },
    {   22,     16,    0xFF7F7F },
    {   22,     17,    0xB30202 },
    {   22,     18,    0xC63F23 },
    {   22,     19,    0xFF7F7F },
    {   22,     20,    0xFF7F7F },
    {   22,     21,    0xFF7F7F },
    {   22,     22,    0xC63F23 },
    {   22,     23,    0xFF7F7F },
    {   22,     24,    0xFF7F7F },
    {   22,     25,    0xB30202 },
    {   22,     26,    0xC63F23 },
    {   22,     27,    0xFF7F7F },
    {   22,     28,    0xC63F23 },
    {   22,     29,    0xFF7F7F },
    {   22,     30,    0xC63F23 },
    {   22,     31,    0xFF7F7F },
    {   22,     33,    0xFFF588 },
    {   22,     34,    0xFFF588 },
    {   22,     35,    0xFFF588 },
    {   22,     36,    0xC63F23 },
    {   22,     37,    0xFF7F7F },
    {   22,     38,    0xFF7F7F },
    {   22,     40,    0xFFF588 },
    {   22,     43,    0xB30202 },
    {   22,     44,    0xFFF588 },
    {   22,     45,    0xFFF588 },
    {   22,     46,    0xFF7F7F },
    {   22,     47,    0x4F5D8B },
    {   22,     48,    0xC63F23 },
    {   22,     49,    0xFF7F7F },
    {   22,     50,    0xFF7F7F },
    {   22,     51,    0xC63F23 },
    {   22,     52,    0xC63F23 },
    {   22,     53,    0xFF7F7F },
    {   22,     54,    0xFF7F7F },
    {   22,     55,    0xB30202 },
    {   22,     56,    0xC63F23 },
    {   22,     57,    0xFF7F7F },
    {   22,     58,    0xFF7F7F },
    {   22,     59,    0xFF7F7F },
    {   22,     60,    0xB30202 },
    {   22,     61,    0xB30202 },
    {   22,     62,    0xFF7F7F },
    {   22,     63,    0xFF7F7F },
    {   22,     64,    0xFF7F7F },
    {   22,     65,    0xC63F23 },
    {   22,     66,    0xFF7F7F },
    {   22,     67,    0xFF7F7F },
    {   22,     68,    0xFF7F7F },
    {   22,     69,    0xFF7F7F },
    {   22,     70,    0xFF7F7F },
    {   22,     71,    0xB30202 },
    {   22,     72,    0xB30202 },
    {   22,     73,    0xFF7F7F },
    {   22,     74,    0xFF7F7F },
    {   22,     75,    0xFF7F7F },
    {   22,     79,    0xC63F23 },
    {   22,     80,    0xC63F23 },
    {   22,     81,    0x50ADAC },
    {   22,     82,    0x50ADAC },
    {   22,     83,    0x50ADAC },
    {   22,     84,    0x50ADAC },
    {   22,     85,    0x50ADAC },
    {   22,     86,    0xFF7F7F },
    {   22,     87,    0xFF7F7F },
    {   22,     88,    0xFF7F7F },
    {   22,     89,    0xB30202 },
    {   22,     90,    0xC63F23 },
    {   22,     91,    0xC63F23 },
    {   22,     92,    0xC63F23 },
    {   22,     93,    0x50ADAC },
    {   22,     94,    0x50ADAC },
    {   22,     95,    0xB30202 },
    {   22,     96,    0x50ADAC },
    {   22,     97,    0x50ADAC },
    {   22,     98,    0xB30202 },
    {   22,     99,    0xB30202 },
    {   22,    100,    0xFF7F7F },
    {   22,    101,    0x50ADAC },
    {   22,    102,    0x50ADAC },
    {   22,    103,    0x50ADAC },
    {   22,    104,    0xFF7F7F },
    {   22,    106,    0xC63F23 },
    {   22,    107,    0xFF7F7F },
    {   22,    108,    0x50ADAC },
    {   22,    109,    0x50ADAC },
    {   22,    110,    0x50ADAC },
    {   22,    111,    0x50ADAC },
    {   22,    113,    0x043E8B },
    {   22,    117,    0x043E8B },
    {   22,    118,    0x043E8B },
    {   22,    121,    0xFF7F7F },
    {   22,    122,    0xFF7F7F },
    {   22,    123,    0xFF7F7F },
    {   22,    125,    0x043E8B },
    {   22,    128,    0xFFAFAF },
    {   22,    129,    0x043E8B },
    {   22,    130,    0x043E8B },
    {   22,    131,    0xFFAFAF },
    {   22,    127,    0xFF7F7F },
    {   22,    132,    0xFF7F7F },
    {   22,    133,    0xFF7F7F },
    {   22,    134,    0xFF7F7F },
    {   22,    135,    0xB30202 },
    {   22,    136,    0xC63F23 },
    {   22,    137,    0xFF7F7F },
    {   22,    145,    0xC63F23 },
    {   22,    146,    0xC63F23 },
    {   22,    148,    0xC63F23 },
    {   22,    149,    0xFF7F7F },
    {   22,    150,    0xFF7F7F },
    {   22,    151,    0xFF7F7F },
    {   22,    152,    0xFF7F7F },
    {   22,    153,    0xFF7F7F },
    {   22,    154,    0xFF7F7F },
    {   22,    155,    0xB30202 },
    {   22,    156,    0xFF7F7F },
    SECTORCOLOR_END
};

//
// Area 23: Dis
//

static const sectorcolor_t sectorcolor_map23[] =
{
    {   23,      0,    0x50ADAC },
    {   23,      1,    0x777777 },
    {   23,      2,    0x50ADAC },
    {   23,      3,    0xFF7F7F },
    {   23,      4,    0xFF7F7F },
    {   23,      5,    0xFFF588 },
    {   23,      6,    0x043E8B },
    {   23,      7,    0x043E8B },
    {   23,      8,    0xFF7F7F },
    {   23,      9,    0x50ADAC },
    {   23,     10,    0x043E8B },
    {   23,     11,    0xFFF588 },
    {   23,     12,    0xB87A15 },
    {   23,     13,    0x666666 },
    {   23,     14,    0x50ADAC },
    {   23,     15,    0x043E8B },
    {   23,     16,    0x50ADAC },
    {   23,     17,    0xFF7F7F },
    SECTORCOLOR_END
};

//
// Area 24: Military Base
//

static const sectorcolor_t sectorcolor_map24[] =
{
    {   24,      5,    0xBBE357 },
    {   24,     23,    0xBBE357 },
    {   24,     24,    0x043E8B },
    {   24,     25,    0x043E8B },
    {   24,     26,    0xBBE357 },
    {   24,     27,    0xBBE357 },
    {   24,     28,    0xBBE357 },
    {   24,     29,    0x55B828 },
    {   24,     30,    0xBBE357 },
    {   24,     31,    0xBBE357 },
    {   24,     32,    0xBBE357 },
    {   24,     33,    0xBBE357 },
    {   24,     34,    0xBBE357 },
    {   24,     36,    0xBBE357 },
    {   24,     37,    0x55B828 },
    {   24,     38,    0x55B828 },
    {   24,     39,    0x55B828 },
    {   24,     46,    0x55B828 },
    {   24,     47,    0xFF7F7F },
    {   24,     50,    0x55B828 },
    {   24,     51,    0x55B828 },
    {   24,     52,    0xBBE357 },
    {   24,     58,    0xBBE357 },
    {   24,     59,    0xBBE357 },
    {   24,     60,    0xBBE357 },
    {   24,     61,    0xBBE357 },
    {   24,     62,    0xBBE357 },
    {   24,     64,    0xBBE357 },
    {   24,     65,    0xBBE357 },
    {   24,     66,    0x55B828 },
    {   24,     80,    0xEEC06B },
    {   24,     94,    0xECB866 },
    {   24,     99,    0xECB866 },
    {   24,    100,    0xECB866 },
    {   24,    103,    0x55B828 },
    {   24,    107,    0xFFF588 },
    {   24,    115,    0xFFF588 },
    {   24,    119,    0xFFF588 },
    {   24,    120,    0xFFF588 },
    {   24,    125,    0xFF7F7F },
    {   24,    127,    0xFF7F7F },
    {   24,    130,    0xD97C45 },
    {   24,    145,    0x043E8B },
    {   24,    147,    0xFFF588 },
    {   24,    148,    0xFF7F7F },
    {   24,    149,    0xFFF588 },
    SECTORCOLOR_END
};

void P_SetSectorColorTable (int area)
{
    switch (area)
    {
        case  1:  sectorcolor = sectorcolor_map01;  break;
        case  2:  sectorcolor = sectorcolor_map02;  break;
        case  3:  sectorcolor = sectorcolor_map03;  break;
        case  4:  sectorcolor = sectorcolor_map04;  break;
        case  5:  sectorcolor = sectorcolor_map05;  break;
        case  6:  sectorcolor = sectorcolor_map06;  break;
        case  7:  sectorcolor = sectorcolor_map07;  break;
        case  8:  sectorcolor = sectorcolor_map08;  break;
        case  9:  sectorcolor = sectorcolor_map09;  break;
        case 10:  sectorcolor = sectorcolor_map10;  break;
        case 11:  sectorcolor = sectorcolor_map11;  break;
        case 12:  sectorcolor = sectorcolor_map12;  break;
        case 13:  sectorcolor = sectorcolor_map13;  break;
        case 14:  sectorcolor = sectorcolor_map14;  break;
        case 15:  sectorcolor = sectorcolor_map15;  break;
        case 16:  sectorcolor = sectorcolor_map16;  break;
        case 17:  sectorcolor = sectorcolor_map17;  break;
        case 18:  sectorcolor = sectorcolor_map18;  break;
        case 19:  sectorcolor = sectorcolor_map19;  break;
        case 20:  sectorcolor = sectorcolor_map20;  break;
        case 21:  sectorcolor = sectorcolor_map21;  break;
        case 22:  sectorcolor = sectorcolor_map22;  break;
        case 23:  sectorcolor = sectorcolor_map23;  break;
        case 24:  sectorcolor = sectorcolor_map24;  break;
        default:  sectorcolor = sectorcolor_dummy;  break;
    }
}

// =============================================================================
//
//                           COLORED COLORMAP LUMPS
//
// =============================================================================

const byte C_EEC06B[] = {
    0,0,0,7,3,0,3,0,0,61,49,27,238,192,107,7,
    6,3,2,2,1,0,0,0,0,0,0,30,28,8,16,17,
    1,4,6,0,0,2,0,58,35,14,49,28,10,40,21,7,
    238,166,93,238,155,86,238,148,83,238,136,76,231,129,72,221,
    120,67,216,110,61,205,102,57,195,94,52,188,88,49,178,80,
    44,174,76,42,163,69,38,159,62,34,148,54,30,144,50,28,
    134,44,24,129,40,22,119,35,20,115,29,16,105,25,14,100,
    22,12,91,18,10,87,14,8,79,11,6,75,6,3,65,3,
    2,62,2,1,55,2,1,49,0,0,42,0,0,39,0,0,
    238,192,107,238,192,106,238,192,100,238,186,95,238,182,91,238,
    175,85,238,166,79,238,163,76,238,154,68,238,146,64,234,138,
    60,223,130,55,213,121,51,203,114,47,193,105,42,188,102,40,
    175,97,38,161,90,35,154,85,32,144,80,30,135,73,27,121,
    69,25,114,61,22,105,57,20,96,53,18,84,47,16,71,41,
    13,60,36,10,50,29,8,40,24,6,28,19,2,21,13,1,
    238,192,107,238,192,107,238,192,107,237,191,107,227,183,102,217,
    175,97,211,170,95,201,162,90,190,154,86,186,150,84,175,142,
    79,171,138,77,161,130,72,150,121,68,146,117,65,136,110,61,
    126,102,57,121,98,55,112,90,50,102,82,46,97,78,44,88,
    71,39,79,64,36,75,60,34,65,53,29,57,46,26,52,42,
    23,43,35,19,39,32,18,31,25,14,22,18,10,18,14,8,
    139,192,60,126,187,54,116,171,50,105,157,44,96,142,39,86,
    128,34,75,113,29,65,102,26,58,88,20,48,74,18,38,61,
    13,27,47,8,18,35,6,7,22,1,3,11,0,0,2,0,
    189,137,68,180,129,63,170,120,59,160,113,55,149,105,50,145,
    99,48,135,93,44,125,85,39,117,78,37,106,71,33,102,68,
    31,92,60,27,85,56,25,76,49,21,66,42,18,62,38,16,
    147,101,45,128,88,37,115,77,32,100,66,26,82,54,20,68,
    44,16,57,34,11,44,27,8,112,93,42,97,81,36,88,74,
    32,77,66,28,65,55,22,54,48,18,45,41,14,38,35,12,
    238,192,61,223,169,47,198,141,36,174,113,26,149,88,18,126,
    62,11,104,43,5,84,25,1,238,192,107,238,192,107,238,170,
    95,238,142,79,238,117,65,238,94,52,238,69,38,232,44,25,
    226,22,12,208,19,10,196,18,10,184,16,9,171,14,8,160,
    13,7,147,11,6,135,8,5,123,6,3,106,4,2,95,3,
    2,84,2,1,72,2,1,61,0,0,49,0,0,39,0,0,
    238,192,107,222,179,107,193,156,107,164,133,107,136,110,107,105,
    84,107,77,62,107,50,41,104,27,22,102,22,18,88,18,14,
    77,14,11,66,7,6,55,5,4,44,2,2,34,0,0,24,
    238,192,107,238,192,107,238,189,95,238,171,79,238,153,65,238,
    137,50,238,117,37,231,102,23,218,93,20,208,86,16,196,80,
    16,186,73,13,174,66,10,163,58,8,150,51,7,142,48,6,
    238,192,107,238,192,107,238,192,91,238,192,74,238,192,57,238,
    192,42,232,189,26,223,182,12,135,44,5,126,38,3,115,32,
    3,102,24,2,58,35,13,44,25,8,31,14,3,23,6,0,
    0,0,24,0,0,19,0,0,14,0,0,10,0,0,5,0,
    0,1,0,0,0,0,0,0,238,130,39,238,178,43,238,117,
    107,226,22,100,175,15,78,126,6,55,79,2,34,157,89,50
};

const byte C_D97C45[] = {
    0,0,0,7,2,0,3,0,0,55,32,18,217,124,69,7,
    4,2,2,1,1,0,0,0,0,0,0,27,18,5,14,11,
    1,3,4,0,0,1,0,53,22,9,44,18,7,37,14,5,
    217,107,60,217,100,56,217,96,53,217,88,49,211,83,46,202,
    78,43,197,71,40,187,66,37,178,61,34,171,57,32,163,52,
    29,158,49,27,149,44,25,145,40,22,135,35,19,131,33,18,
    123,28,16,117,26,14,109,23,13,105,19,11,95,16,9,91,
    14,8,83,12,6,79,9,5,72,7,4,68,4,2,60,2,
    1,56,1,1,50,1,1,45,0,0,38,0,0,36,0,0,
    217,124,69,217,124,68,217,124,64,217,120,61,217,118,58,217,
    113,55,217,107,51,217,105,49,217,99,44,217,94,41,214,89,
    38,203,84,36,194,78,33,186,73,30,176,68,27,171,66,26,
    160,63,24,147,58,22,140,55,21,131,52,19,123,47,18,111,
    44,16,104,39,14,95,37,13,88,35,12,77,30,10,65,26,
    8,54,23,7,46,19,5,37,16,4,26,12,1,20,8,1,
    217,124,69,217,124,69,217,124,69,216,124,69,207,118,66,197,
    113,63,192,110,61,183,105,58,174,99,55,169,97,54,160,91,
    51,156,89,50,146,84,47,137,78,44,133,76,42,124,71,40,
    115,66,37,111,63,35,102,58,32,93,53,29,89,51,28,80,
    46,25,72,41,23,68,39,22,60,34,19,52,30,17,48,27,
    15,39,22,12,36,20,11,28,16,9,20,12,6,16,9,5,
    127,124,38,115,121,35,106,110,32,95,102,29,88,91,25,78,
    83,22,68,73,19,60,66,17,53,57,13,43,48,11,35,39,
    9,25,31,5,16,22,4,6,14,1,3,7,0,0,1,0,
    173,89,44,164,83,41,155,78,38,146,73,35,136,68,32,132,
    64,31,123,60,28,114,55,25,106,51,24,97,46,21,93,44,
    20,84,39,17,77,36,16,69,32,13,60,27,12,56,25,11,
    134,65,29,117,57,24,105,50,21,91,42,17,75,35,13,62,
    28,10,52,22,7,40,18,5,102,60,27,89,53,23,80,48,
    21,70,43,18,60,35,14,49,31,12,41,26,9,35,22,8,
    217,124,40,203,109,30,180,91,23,158,73,17,136,57,12,115,
    40,7,94,28,3,77,16,1,217,124,69,217,124,69,217,110,
    61,217,92,51,217,75,42,217,61,34,217,44,25,212,29,16,
    206,14,8,190,12,7,179,12,6,168,10,6,156,9,5,146,
    8,5,134,7,4,123,5,3,112,4,2,97,2,1,87,2,
    1,77,1,1,66,1,1,55,0,0,45,0,0,36,0,0,
    217,124,69,203,116,69,176,101,69,150,86,69,124,71,69,95,
    54,69,71,40,69,46,26,67,25,14,65,20,12,57,16,9,
    50,13,7,42,7,4,36,4,2,29,2,1,22,0,0,15,
    217,124,69,217,124,69,217,122,61,217,110,51,217,99,42,217,
    89,32,217,76,24,211,66,15,199,60,13,190,55,11,179,52,
    10,169,47,9,158,42,7,149,37,5,137,33,4,129,31,4,
    217,124,69,217,124,69,217,124,58,217,124,48,217,124,37,217,
    124,27,212,122,17,203,118,8,123,29,3,115,25,2,105,21,
    2,93,16,1,53,22,9,40,16,5,28,9,2,21,4,0,
    0,0,15,0,0,12,0,0,9,0,0,6,0,0,3,0,
    0,1,0,0,0,0,0,0,217,84,25,217,115,28,217,75,
    69,206,14,65,160,10,51,115,4,36,72,1,22,143,57,32
};

const byte C_FF7F7F[] = {
    0,0,0,8,2,0,3,0,0,65,32,32,255,127,127,8,
    4,4,2,1,1,0,0,0,0,0,0,32,18,9,17,11,
    1,4,4,0,0,1,0,62,23,17,52,18,12,43,14,8,
    255,110,110,255,103,103,255,98,98,255,90,90,248,85,85,237,
    80,80,231,73,73,220,67,67,209,62,62,201,58,58,191,53,
    53,186,50,50,175,45,45,170,41,41,159,36,36,154,33,33,
    144,29,29,138,26,26,128,23,23,123,19,19,112,16,16,107,
    14,14,98,12,12,93,9,9,85,7,7,80,4,4,70,2,
    2,66,1,1,59,1,1,53,0,0,45,0,0,42,0,0,
    255,127,127,255,127,126,255,127,119,255,123,113,255,121,108,255,
    116,101,255,110,94,255,108,91,255,102,81,255,97,76,251,91,
    71,239,86,66,228,80,61,218,75,55,207,70,50,201,67,48,
    188,64,45,173,59,41,165,56,38,154,53,35,145,48,32,130,
    45,29,122,40,26,112,38,24,103,35,21,90,31,18,76,27,
    15,64,24,12,54,19,9,43,16,7,30,12,2,23,8,1,
    255,127,127,255,127,127,255,127,127,254,127,127,243,121,121,232,
    116,116,226,113,113,215,107,107,204,102,102,199,99,99,188,94,
    94,183,91,91,172,86,86,161,80,80,156,78,78,146,73,73,
    135,67,67,130,65,65,120,60,60,109,54,54,104,52,52,94,
    47,47,85,42,42,80,40,40,70,35,35,61,30,30,56,28,
    28,46,23,23,42,21,21,33,16,16,24,12,12,19,9,9,
    149,127,71,135,124,64,124,113,59,112,104,53,103,94,46,92,
    85,41,80,75,35,70,67,30,62,58,24,51,49,21,41,40,
    16,29,31,10,19,23,7,7,14,1,3,7,0,0,1,0,
    203,91,80,193,85,75,182,80,70,171,75,65,160,69,60,155,
    66,57,145,62,52,134,56,47,125,52,44,114,47,39,109,45,
    36,99,40,32,91,37,29,81,32,24,71,28,22,66,25,19,
    157,67,53,137,58,43,123,51,38,107,43,31,88,36,23,73,
    29,18,61,22,13,47,18,9,120,62,50,104,54,43,94,49,
    38,82,44,33,70,36,26,58,32,21,48,27,16,41,23,14,
    255,127,73,239,112,55,212,93,43,186,75,31,160,58,21,135,
    41,13,111,28,5,90,16,1,255,127,127,255,127,127,255,113,
    113,255,94,94,255,77,77,255,62,62,255,45,45,249,29,29,
    242,14,14,223,12,12,210,12,12,197,10,10,183,9,9,171,
    8,8,157,7,7,145,5,5,132,4,4,114,2,2,102,2,
    2,90,1,1,77,1,1,65,0,0,53,0,0,42,0,0,
    255,127,127,238,119,127,207,103,127,176,88,127,146,73,127,112,
    56,127,83,41,127,54,27,124,29,14,121,24,12,105,19,9,
    91,15,7,78,8,4,66,5,2,53,2,1,40,0,0,28,
    255,127,127,255,127,127,255,125,113,255,113,94,255,101,77,255,
    91,60,255,78,43,248,67,27,234,61,24,223,57,19,210,53,
    18,199,48,16,186,43,12,175,38,9,161,34,8,152,32,7,
    255,127,127,255,127,127,255,127,108,255,127,88,255,127,68,255,
    127,49,249,125,31,239,121,14,145,29,5,135,25,4,123,21,
    3,109,16,2,62,23,16,47,16,9,33,9,3,25,4,0,
    0,0,28,0,0,22,0,0,16,0,0,12,0,0,5,0,
    0,1,0,0,0,0,0,0,255,86,47,255,118,51,255,77,
    127,242,14,119,188,10,93,135,4,66,85,1,40,168,59,59
};

const byte C_55B828[] = {
    0,0,0,3,3,0,1,0,0,22,47,10,85,184,40,3,
    6,1,1,1,0,0,0,0,0,0,0,11,27,3,6,17,
    0,1,6,0,0,1,0,21,33,5,17,27,4,14,20,3,
    85,159,35,85,149,32,85,142,31,85,131,28,83,123,27,79,
    115,25,77,105,23,73,97,21,70,90,20,67,84,18,64,76,
    17,62,73,16,58,66,14,57,59,13,53,52,11,51,48,11,
    48,42,9,46,38,8,43,34,7,41,28,6,37,24,5,36,
    21,5,33,17,4,31,14,3,28,11,2,27,6,1,23,3,
    1,22,2,0,20,2,0,18,0,0,15,0,0,14,0,0,
    85,184,40,85,184,40,85,184,37,85,178,35,85,175,34,85,
    167,32,85,159,30,85,156,29,85,147,26,85,140,24,84,132,
    22,80,124,21,76,116,19,73,109,17,69,101,16,67,97,15,
    63,93,14,58,86,13,55,82,12,51,76,11,48,70,10,43,
    66,9,41,58,8,37,55,8,34,51,7,30,45,6,25,39,
    5,21,35,4,18,28,3,14,23,2,10,18,1,8,12,0,
    85,184,40,85,184,40,85,184,40,85,183,40,81,175,38,77,
    167,36,75,163,35,72,155,34,68,147,32,66,144,31,63,136,
    29,61,132,29,57,124,27,54,116,25,52,113,24,49,105,23,
    45,97,21,43,94,20,40,87,19,36,79,17,35,75,16,31,
    68,15,28,61,13,27,58,13,23,51,11,20,44,10,19,40,
    9,15,33,7,14,30,7,11,24,5,8,17,4,6,14,3,
    50,184,22,45,179,20,41,164,19,37,151,17,34,136,15,31,
    123,13,27,108,11,23,97,10,21,84,8,17,71,7,14,58,
    5,10,45,3,6,33,2,2,21,0,1,11,0,0,2,0,
    68,131,25,64,123,24,61,115,22,57,108,20,53,100,19,52,
    95,18,48,89,16,45,82,15,42,75,14,38,68,12,36,65,
    11,33,58,10,30,54,9,27,47,8,24,40,7,22,37,6,
    52,97,17,46,84,14,41,74,12,36,63,10,29,52,7,24,
    42,6,20,32,4,16,26,3,40,89,16,35,78,13,31,71,
    12,27,63,10,23,53,8,19,46,7,16,39,5,14,33,4,
    85,184,23,80,162,17,71,135,13,62,108,10,53,84,7,45,
    59,4,37,41,2,30,24,0,85,184,40,85,184,40,85,163,
    35,85,136,30,85,112,24,85,90,20,85,66,14,83,43,9,
    81,21,5,74,18,4,70,17,4,66,15,3,61,14,3,57,
    12,3,52,11,2,48,8,2,44,6,1,38,4,1,34,3,
    1,30,2,0,26,1,0,22,0,0,18,0,0,14,0,0,
    85,184,40,79,172,40,69,149,40,59,127,40,49,105,40,37,
    81,40,28,60,40,18,39,39,10,21,38,8,17,33,6,14,
    29,5,11,25,3,6,21,2,4,17,1,1,13,0,0,9,
    85,184,40,85,184,40,85,181,35,85,164,30,85,146,24,85,
    131,19,85,113,14,83,97,8,78,89,8,74,82,6,70,76,
    6,66,70,5,62,63,4,58,56,3,54,49,3,51,46,2,
    85,184,40,85,184,40,85,184,34,85,184,28,85,184,21,85,
    184,16,83,181,10,80,175,5,48,43,2,45,37,1,41,31,
    1,36,23,1,21,33,5,16,24,3,11,14,1,8,6,0,
    0,0,9,0,0,7,0,0,5,0,0,4,0,0,2,0,
    0,0,0,0,0,0,0,0,85,124,15,85,171,16,85,112,
    40,81,21,37,63,14,29,45,6,21,28,1,13,56,85,19
};

const byte C_BBE357[] = {
    0,0,0,6,4,0,2,0,0,48,58,22,187,227,87,6,
    7,3,1,2,1,0,0,0,0,0,0,23,33,6,12,20,
    1,3,7,0,0,2,0,45,41,12,38,33,9,32,25,6,
    187,197,75,187,183,70,187,175,67,187,161,62,182,152,58,174,
    142,55,169,130,50,161,120,46,153,111,43,147,104,40,140,94,
    36,136,90,34,128,81,31,125,73,28,117,64,25,113,60,23,
    106,52,20,101,47,18,94,42,16,90,35,13,82,29,11,78,
    26,10,72,21,8,68,17,6,62,13,5,59,7,3,51,4,
    1,48,3,1,43,3,1,39,0,0,33,0,0,31,0,0,
    187,227,87,187,227,86,187,227,81,187,220,77,187,215,74,187,
    207,69,187,197,64,187,192,62,187,182,56,187,173,52,184,163,
    48,175,153,45,167,143,42,160,134,38,152,125,34,147,120,33,
    138,115,31,127,106,28,121,101,26,113,94,24,106,86,22,95,
    81,20,89,72,18,82,68,16,76,63,15,66,55,13,56,48,
    10,47,43,9,40,35,6,32,28,5,22,22,2,17,15,1,
    187,227,87,187,227,87,187,227,87,186,226,87,178,216,83,170,
    207,79,166,201,77,158,191,73,150,182,70,146,177,68,138,167,
    64,134,163,62,126,153,59,118,143,55,114,139,53,107,130,50,
    99,120,46,95,116,44,88,107,41,80,97,37,76,93,35,69,
    84,32,62,76,29,59,71,27,51,62,24,45,54,21,41,50,
    19,34,41,16,31,37,14,24,29,11,18,21,8,14,17,6,
    109,227,48,99,221,44,91,202,40,82,186,36,76,167,32,67,
    151,28,59,134,24,51,120,21,45,104,16,37,87,14,30,72,
    11,21,56,7,14,41,5,5,26,1,2,13,0,0,3,0,
    149,162,55,142,152,52,133,142,48,125,134,44,117,124,41,114,
    118,39,106,110,35,98,101,32,92,93,30,84,84,27,80,80,
    25,73,71,22,67,67,20,59,58,17,52,50,15,48,45,13,
    115,119,37,100,104,30,90,91,26,78,77,21,65,64,16,54,
    52,13,45,40,9,34,32,6,88,110,34,76,96,29,69,87,
    26,60,78,23,51,65,18,43,57,15,35,48,11,30,41,10,
    187,227,50,175,200,38,155,166,29,136,134,21,117,104,15,99,
    73,9,81,51,4,66,29,1,187,227,87,187,227,87,187,201,
    77,187,168,64,187,138,53,187,111,43,187,81,31,183,53,20,
    177,26,10,164,22,9,154,21,8,144,19,7,134,17,6,125,
    15,6,115,13,5,106,10,4,97,7,3,84,4,2,75,4,
    1,66,3,1,56,2,1,48,0,0,39,0,0,31,0,0,
    187,227,87,175,212,87,152,184,87,129,157,87,107,130,87,82,
    100,87,61,74,87,40,48,85,21,26,83,18,21,72,14,17,
    62,11,13,54,6,7,45,4,4,36,1,2,28,0,0,19,
    187,227,87,187,227,87,187,223,77,187,202,64,187,181,53,187,
    162,41,187,139,30,182,120,18,172,109,16,164,101,13,154,94,
    13,146,86,11,136,77,9,128,69,6,118,61,5,111,57,5,
    187,227,87,187,227,87,187,227,74,187,227,60,187,227,47,187,
    227,34,183,223,21,175,215,10,106,53,4,99,45,3,90,38,
    2,80,28,2,45,41,11,34,29,6,24,17,2,18,7,0,
    0,0,19,0,0,15,0,0,11,0,0,8,0,0,4,0,
    0,1,0,0,0,0,0,0,187,153,32,187,211,35,187,138,
    87,177,26,82,138,18,64,99,7,45,62,2,28,123,105,40
};

const byte C_949DB9[] = {
    0,0,0,5,2,0,2,0,0,38,40,47,148,157,185,5,
    5,6,1,1,1,0,0,0,0,0,0,19,23,14,10,14,
    2,2,5,0,0,1,0,36,28,25,30,23,18,25,17,12,
    148,136,160,148,127,149,148,121,143,148,111,131,144,105,124,138,
    99,116,134,90,106,128,83,98,121,77,91,117,72,85,111,65,
    77,108,62,73,102,56,66,99,50,59,92,44,52,89,41,49,
    84,36,42,80,33,38,74,29,34,71,24,28,65,20,24,62,
    18,21,57,15,17,54,12,14,49,9,11,46,5,6,41,2,
    3,38,2,2,34,2,2,31,0,0,26,0,0,24,0,0,
    148,157,185,148,157,184,148,157,173,148,152,164,148,149,157,148,
    143,147,148,136,137,148,133,132,148,126,118,148,119,110,146,113,
    103,139,106,96,132,99,89,127,93,81,120,86,73,117,83,70,
    109,79,65,100,73,60,96,70,56,89,65,52,84,60,47,75,
    56,43,71,50,38,65,47,35,60,44,31,52,38,27,44,33,
    22,37,30,18,31,24,14,25,20,10,17,15,4,13,10,2,
    148,157,185,148,157,185,148,157,185,147,156,184,141,150,176,135,
    143,168,131,139,164,125,132,156,118,126,148,115,123,144,109,116,
    136,106,113,133,100,106,125,93,99,117,91,96,113,85,90,106,
    78,83,98,75,80,94,70,74,87,63,67,79,60,64,75,55,
    58,68,49,52,62,46,49,58,41,43,51,35,38,44,33,34,
    41,27,28,33,24,26,30,19,20,24,14,15,17,11,12,14,
    86,157,103,78,153,94,72,140,86,65,129,77,60,116,67,53,
    105,59,46,92,51,41,83,44,36,72,35,30,60,30,24,50,
    23,17,39,15,11,28,10,4,18,2,2,9,0,0,2,0,
    118,112,117,112,105,110,106,99,102,99,92,94,93,86,87,90,
    81,83,84,76,75,78,70,68,73,64,64,66,58,57,63,55,
    53,57,49,46,53,46,43,47,40,36,41,34,32,38,31,28,
    91,83,78,80,72,63,71,63,55,62,54,45,51,44,34,42,
    36,27,35,28,20,27,22,13,70,76,73,60,66,62,55,60,
    55,48,54,48,41,45,38,34,39,31,28,33,24,24,28,20,
    148,157,106,139,139,81,123,115,62,108,92,45,93,72,31,78,
    50,20,64,35,8,52,20,2,148,157,185,148,157,185,148,139,
    164,148,116,137,148,95,112,148,77,91,148,56,66,145,36,43,
    140,18,21,129,15,18,122,15,17,114,13,15,106,12,14,99,
    10,12,91,9,11,84,7,8,77,5,6,66,3,4,59,2,
    3,52,2,2,45,1,1,38,0,0,31,0,0,24,0,0,
    148,157,185,138,147,185,120,127,185,102,108,185,85,90,185,65,
    69,185,48,51,185,31,33,180,17,18,176,14,15,152,11,12,
    133,9,9,114,5,5,96,3,3,77,1,1,59,0,0,41,
    148,157,185,148,157,185,148,155,164,148,140,137,148,125,112,148,
    112,87,148,96,63,144,83,39,136,76,35,129,70,28,122,65,
    27,115,60,23,108,54,18,102,47,13,93,42,12,88,39,10,
    148,157,185,148,157,185,148,157,157,148,157,128,148,157,99,148,
    157,72,145,155,46,139,149,21,84,36,8,78,31,6,71,26,
    4,63,20,4,36,28,23,27,20,13,19,12,4,15,5,1,
    0,0,41,0,0,33,0,0,24,0,0,17,0,0,8,0,
    0,1,0,0,0,0,0,0,148,106,68,148,146,75,148,95,
    185,140,18,173,109,12,136,78,5,96,49,1,59,98,73,86
};

const byte C_2A2F6B[] = {
    0,0,0,1,1,0,1,0,0,11,14,52,43,55,205,1,
    2,6,0,0,2,0,0,0,0,0,0,5,8,15,3,5,
    2,1,2,0,0,0,0,10,10,27,9,8,20,7,6,14,
    43,48,178,43,44,166,43,42,158,43,39,146,42,37,137,40,
    35,129,39,31,117,37,29,109,35,27,100,34,25,94,32,23,
    85,31,22,81,30,20,73,29,18,66,27,16,58,26,14,54,
    24,13,47,23,11,43,22,10,38,21,8,31,19,7,27,18,
    6,23,17,5,19,16,4,15,14,3,12,13,2,6,12,1,
    3,11,1,2,10,1,2,9,0,0,8,0,0,7,0,0,
    43,55,205,43,55,203,43,55,191,43,53,182,43,52,174,43,
    50,163,43,48,152,43,47,146,43,44,131,43,42,122,42,39,
    114,40,37,106,38,35,98,37,33,89,35,30,81,34,29,77,
    32,28,72,29,26,67,28,24,62,26,23,57,24,21,52,22,
    20,47,21,17,43,19,16,39,17,15,35,15,13,30,13,12,
    24,11,10,20,9,8,15,7,7,11,5,5,4,4,4,2,
    43,55,205,43,55,205,43,55,205,43,55,204,41,52,195,39,
    50,187,38,49,182,36,46,173,34,44,164,34,43,160,32,41,
    151,31,39,147,29,37,138,27,35,129,26,34,125,25,31,117,
    23,29,109,22,28,105,20,26,96,18,24,88,18,22,84,16,
    20,76,14,18,68,13,17,64,12,15,56,10,13,49,9,12,
    45,8,10,37,7,9,34,6,7,27,4,5,19,3,4,15,
    25,55,114,23,53,104,21,49,95,19,45,85,17,41,75,16,
    37,66,13,32,56,12,29,49,10,25,39,9,21,34,7,17,
    26,5,14,16,3,10,11,1,6,2,1,3,0,0,1,0,
    34,39,129,33,37,121,31,35,113,29,32,105,27,30,96,26,
    28,92,24,27,84,23,24,76,21,22,71,19,20,63,18,19,
    59,17,17,51,15,16,47,14,14,39,12,12,35,11,11,31,
    26,29,86,23,25,70,21,22,61,18,19,50,15,16,38,12,
    13,30,10,10,22,8,8,14,20,27,81,18,23,69,16,21,
    61,14,19,53,12,16,42,10,14,35,8,12,27,7,10,23,
    43,55,117,40,49,89,36,40,69,31,32,50,27,25,35,23,
    18,22,19,12,9,15,7,2,43,55,205,43,55,205,43,49,
    182,43,41,152,43,33,125,43,27,100,43,20,73,42,13,47,
    41,6,23,38,5,20,35,5,19,33,5,17,31,4,15,29,
    4,14,26,3,12,24,2,9,22,2,6,19,1,4,17,1,
    3,15,1,2,13,0,2,11,0,0,9,0,0,7,0,0,
    43,55,205,40,51,205,35,45,205,30,38,205,25,31,205,19,
    24,205,14,18,205,9,12,199,5,6,195,4,5,169,3,4,
    147,3,3,126,1,2,106,1,1,85,0,0,65,0,0,46,
    43,55,205,43,55,205,43,54,182,43,49,152,43,44,125,43,
    39,96,43,34,70,42,29,43,39,27,39,38,25,31,35,23,
    30,34,21,26,31,19,20,30,17,14,27,15,13,26,14,11,
    43,55,205,43,55,205,43,55,174,43,55,141,43,55,110,43,
    55,80,42,54,51,40,52,23,24,13,9,23,11,6,21,9,
    5,18,7,4,10,10,26,8,7,14,6,4,5,4,2,1,
    0,0,46,0,0,36,0,0,27,0,0,19,0,0,9,0,
    0,2,0,0,0,0,0,0,43,37,76,43,51,83,43,33,
    205,41,6,192,32,4,150,23,2,106,14,0,65,28,25,95
};

const byte C_50ADAC[] = {
    0,0,0,3,3,0,1,0,0,20,44,44,80,173,172,3,
    5,5,1,1,1,0,0,0,0,0,0,10,25,13,5,16,
    2,1,5,0,0,1,0,19,31,23,16,25,17,13,19,11,
    80,150,149,80,140,139,80,134,133,80,123,122,78,116,115,74,
    109,108,72,99,98,69,92,91,66,85,84,63,79,79,60,72,
    71,58,69,68,55,62,61,53,56,55,50,49,49,48,45,45,
    45,39,39,43,36,36,40,32,32,39,26,26,35,22,22,34,
    20,20,31,16,16,29,13,13,27,10,10,25,5,5,22,3,
    3,21,2,2,19,2,2,17,0,0,14,0,0,13,0,0,
    80,173,172,80,173,171,80,173,161,80,168,152,80,164,146,80,
    157,137,80,150,127,80,147,123,80,138,110,80,132,103,79,124,
    96,75,117,89,72,109,82,68,102,75,65,95,68,63,92,65,
    59,88,61,54,81,56,52,77,52,48,72,48,45,66,44,41,
    62,40,38,55,36,35,52,32,32,48,29,28,42,25,24,37,
    20,20,33,17,17,26,13,13,22,9,9,17,3,7,12,2,
    80,173,172,80,173,172,80,173,172,80,172,171,76,165,164,73,
    157,156,71,153,152,67,146,145,64,138,138,62,135,134,59,128,
    127,57,124,123,54,117,116,51,109,109,49,106,105,46,99,98,
    42,92,91,41,88,88,38,81,81,34,74,74,33,71,70,29,
    64,63,27,58,57,25,54,54,22,47,47,19,41,41,18,38,
    38,14,31,31,13,28,28,10,22,22,8,16,16,6,13,13,
    47,173,96,42,168,87,39,154,80,35,142,71,32,128,63,29,
    115,55,25,102,47,22,92,41,19,79,32,16,66,28,13,55,
    22,9,43,13,6,31,9,2,20,2,1,10,0,0,2,0,
    64,123,109,61,116,102,57,109,94,54,102,88,50,94,81,49,
    90,77,45,84,70,42,77,63,39,71,59,36,64,53,34,61,
    49,31,54,43,29,51,40,25,44,33,22,38,30,21,35,26,
    49,91,72,43,79,59,39,69,51,34,59,42,28,49,32,23,
    39,25,19,31,18,15,24,12,38,84,68,33,73,58,29,66,
    51,26,60,45,22,50,35,18,43,29,15,37,22,13,31,19,
    80,173,98,75,153,75,67,127,58,58,102,42,50,79,29,42,
    56,18,35,39,7,28,22,2,80,173,172,80,173,172,80,153,
    152,80,128,127,80,105,105,80,85,84,80,62,61,78,40,40,
    76,20,20,70,17,17,66,16,16,62,14,14,57,13,13,54,
    12,11,49,10,10,45,7,7,41,5,5,36,3,3,32,3,
    3,28,2,2,24,1,1,20,0,0,17,0,0,13,0,0,
    80,173,172,75,161,172,65,140,172,55,119,172,46,99,172,35,
    76,172,26,56,172,17,37,167,9,20,163,8,16,142,6,13,
    123,5,10,106,3,5,89,2,3,71,1,1,55,0,0,38,
    80,173,172,80,173,172,80,170,152,80,154,127,80,138,105,80,
    123,81,80,106,59,78,92,36,73,83,32,70,77,26,66,72,
    25,62,66,22,58,59,17,55,52,12,51,46,11,48,43,9,
    80,173,172,80,173,172,80,173,146,80,173,119,80,173,92,80,
    173,67,78,170,42,75,164,20,45,40,7,42,35,5,39,29,
    4,34,22,3,19,31,22,15,22,12,10,13,4,8,5,1,
    0,0,38,0,0,30,0,0,22,0,0,16,0,0,7,0,
    0,1,0,0,0,0,0,0,80,117,63,80,161,69,80,105,
    172,76,20,161,59,14,126,42,5,89,27,1,55,53,80,80
};

const byte C_CCE4A5[] = {
    0,0,0,6,4,0,2,0,0,52,58,42,204,228,165,6,
    7,5,2,2,1,0,0,0,0,0,0,26,33,12,14,21,
    2,3,7,0,0,2,0,50,41,22,42,33,16,34,25,11,
    204,198,143,204,184,133,204,176,127,204,162,117,198,153,111,190,
    143,104,185,131,94,176,121,87,167,112,81,161,105,76,153,95,
    69,149,90,65,140,81,59,136,73,53,127,64,47,123,60,43,
    115,52,38,110,47,34,102,42,30,98,35,25,90,30,21,86,
    26,19,78,21,16,74,17,12,68,13,10,64,7,5,56,4,
    3,53,3,2,47,3,2,42,0,0,36,0,0,34,0,0,
    204,228,165,204,228,164,204,228,154,204,221,146,204,216,140,204,
    207,131,204,198,122,204,193,118,204,182,105,204,173,98,201,164,
    92,191,154,85,182,144,79,174,135,72,166,125,65,161,121,62,
    150,115,58,138,106,54,132,101,50,123,95,46,116,87,42,104,
    81,38,98,72,34,90,68,31,82,63,28,72,55,24,61,48,
    19,51,43,16,43,35,12,34,29,9,24,22,3,18,15,2,
    204,228,165,204,228,165,204,228,165,203,227,164,194,217,157,186,
    207,150,181,202,146,172,192,139,163,182,132,159,178,129,150,168,
    122,146,164,118,138,154,111,129,144,104,125,139,101,117,131,94,
    108,121,87,104,116,84,96,107,78,87,97,71,83,93,67,75,
    84,61,68,76,55,64,72,52,56,63,45,49,55,39,45,50,
    36,37,41,30,34,38,27,26,30,21,19,21,16,15,17,12,
    119,228,92,108,222,83,99,203,76,90,187,69,82,168,60,74,
    152,53,64,134,45,56,121,39,50,105,31,41,88,27,33,72,
    21,23,56,13,15,41,9,6,26,2,2,13,0,0,3,0,
    162,163,104,154,153,98,146,143,91,137,134,84,128,124,78,124,
    118,74,116,111,67,107,101,61,100,93,57,91,84,50,87,80,
    47,79,72,41,73,67,38,65,58,32,57,50,28,53,46,25,
    126,120,69,110,105,56,98,91,49,86,78,40,70,64,30,58,
    52,24,49,40,17,38,32,12,96,111,65,83,97,56,75,88,
    49,66,79,43,56,65,34,46,57,28,38,48,21,33,41,18,
    204,228,94,191,201,72,170,167,56,149,134,40,128,105,28,108,
    73,17,89,51,7,72,30,2,204,228,165,204,228,165,204,202,
    146,204,169,122,204,139,100,204,112,81,204,81,59,199,53,38,
    194,26,19,178,22,16,168,21,16,158,19,14,146,17,12,137,
    15,11,126,13,10,116,10,7,106,7,5,91,4,3,82,4,
    3,72,3,2,62,2,1,52,0,0,42,0,0,34,0,0,
    204,228,165,190,213,165,166,185,165,141,157,165,117,131,165,90,
    100,165,66,74,165,43,48,160,23,26,157,19,21,136,15,17,
    118,12,13,102,6,7,85,4,4,69,2,2,52,0,0,37,
    204,228,165,204,228,165,204,224,146,204,203,122,204,182,100,204,
    163,78,204,139,56,198,121,35,187,110,31,178,102,25,168,95,
    24,159,87,21,149,78,16,140,69,12,129,61,10,122,57,9,
    204,228,165,204,228,165,204,228,140,204,228,114,204,228,89,204,
    228,64,199,224,41,191,216,19,116,53,7,108,46,5,98,38,
    4,87,29,3,50,41,21,38,30,12,26,17,4,20,7,1,
    0,0,37,0,0,29,0,0,21,0,0,16,0,0,7,0,
    0,1,0,0,0,0,0,0,204,154,61,204,212,67,204,139,
    165,194,26,155,150,18,121,108,7,85,68,2,52,134,106,76
};

const byte C_CCEA5F[] = {
    0,0,0,6,4,0,2,0,0,52,60,24,204,234,95,6,
    7,3,2,2,1,0,0,0,0,0,0,26,34,7,14,21,
    1,3,7,0,0,2,0,50,42,13,42,34,9,34,26,6,
    204,203,82,204,189,77,204,181,73,204,166,67,198,157,64,190,
    147,60,185,134,54,176,124,50,167,115,47,161,107,44,153,97,
    39,149,93,38,140,84,34,136,75,31,127,66,27,123,61,25,
    115,53,22,110,49,20,102,43,18,98,36,15,90,30,12,86,
    27,11,78,22,9,74,17,7,68,14,6,64,7,3,56,4,
    1,53,3,1,47,3,1,42,0,0,36,0,0,34,0,0,
    204,234,95,204,234,94,204,234,89,204,227,84,204,222,80,204,
    213,76,204,203,70,204,198,68,204,187,61,204,178,57,201,168,
    53,191,158,49,182,148,45,174,139,41,166,128,38,161,124,36,
    150,118,34,138,109,31,132,104,29,123,97,26,116,89,24,104,
    84,22,98,74,20,90,70,18,82,65,16,72,57,14,61,50,
    11,51,44,9,43,36,7,34,29,5,24,23,2,18,16,1,
    204,234,95,204,234,95,204,234,95,203,233,95,194,223,91,186,
    213,86,181,207,84,172,197,80,163,187,76,159,183,74,150,173,
    70,146,168,68,138,158,64,129,148,60,125,143,58,117,134,54,
    108,124,50,104,119,48,96,110,45,87,100,41,83,95,39,75,
    86,35,68,78,32,64,73,30,56,64,26,49,56,23,45,51,
    21,37,42,17,34,39,16,26,30,12,19,22,9,15,17,7,
    119,234,53,108,228,48,99,208,44,90,192,39,82,173,35,74,
    156,31,64,138,26,56,124,23,50,107,18,41,90,16,33,74,
    12,23,58,7,15,42,5,6,27,1,2,14,0,0,3,0,
    162,167,60,154,157,56,146,147,52,137,138,48,128,128,45,124,
    121,42,116,114,39,107,104,35,100,95,33,91,86,29,87,83,
    27,79,73,24,73,69,22,65,60,18,57,51,16,53,47,15,
    126,123,40,110,107,32,98,94,28,86,80,23,70,66,18,58,
    53,14,49,41,10,38,33,7,96,114,38,83,99,32,75,90,
    28,66,81,25,56,67,19,46,59,16,38,50,12,33,42,10,
    204,234,54,191,206,41,170,172,32,149,138,23,128,107,16,108,
    75,10,89,52,4,72,30,1,204,234,95,204,234,95,204,207,
    84,204,173,70,204,142,58,204,115,47,204,84,34,199,54,22,
    194,27,11,178,23,9,168,22,9,158,19,8,146,17,7,137,
    16,6,126,14,6,116,10,4,106,7,3,91,5,2,82,4,
    1,72,3,1,62,2,1,52,0,0,42,0,0,34,0,0,
    204,234,95,190,218,95,166,190,95,141,162,95,117,134,95,90,
    103,95,66,76,95,43,50,92,23,27,90,19,22,78,15,17,
    68,12,14,58,6,7,49,4,5,39,2,2,30,0,0,21,
    204,234,95,204,234,95,204,230,84,204,208,70,204,186,58,204,
    167,45,204,143,32,198,124,20,187,113,18,178,105,15,168,97,
    14,159,89,12,149,80,9,140,71,7,129,62,6,122,59,5,
    204,234,95,204,234,95,204,234,80,204,234,66,204,234,51,204,
    234,37,199,230,23,191,222,11,116,54,4,108,47,3,98,39,
    2,87,29,2,50,42,12,38,30,7,26,17,2,20,7,0,
    0,0,21,0,0,17,0,0,12,0,0,9,0,0,4,0,
    0,1,0,0,0,0,0,0,204,158,35,204,217,38,204,142,
    95,194,27,89,150,18,70,108,7,49,68,2,30,134,108,44
};

const byte C_B30202[] = {
    0,0,0,6,0,0,2,0,0,46,1,1,179,2,2,6,
    0,0,1,0,0,0,0,0,0,0,0,22,0,0,12,0,
    0,3,0,0,0,0,0,44,0,0,37,0,0,30,0,0,
    179,2,2,179,2,2,179,2,2,179,1,1,174,1,1,166,
    1,1,162,1,1,154,1,1,147,1,1,141,1,1,134,1,
    1,131,1,1,123,1,1,119,1,1,112,1,1,108,1,1,
    101,0,0,97,0,0,90,0,0,86,0,0,79,0,0,75,
    0,0,69,0,0,65,0,0,60,0,0,56,0,0,49,0,
    0,46,0,0,41,0,0,37,0,0,32,0,0,29,0,0,
    179,2,2,179,2,2,179,2,2,179,2,2,179,2,2,179,
    2,2,179,2,1,179,2,1,179,2,1,179,2,1,176,1,
    1,168,1,1,160,1,1,153,1,1,145,1,1,141,1,1,
    132,1,1,121,1,1,116,1,1,108,1,1,102,1,1,91,
    1,0,86,1,0,79,1,0,72,1,0,63,0,0,53,0,
    0,45,0,0,38,0,0,30,0,0,21,0,0,16,0,0,
    179,2,2,179,2,2,179,2,2,178,2,2,171,2,2,163,
    2,2,159,2,2,151,2,2,143,2,2,140,2,2,132,1,
    1,128,1,1,121,1,1,113,1,1,110,1,1,102,1,1,
    95,1,1,91,1,1,84,1,1,77,1,1,73,1,1,66,
    1,1,60,1,1,56,1,1,49,1,1,43,0,0,39,0,
    0,32,0,0,29,0,0,23,0,0,17,0,0,13,0,0,
    105,2,1,95,2,1,87,2,1,79,2,1,72,1,1,65,
    1,1,56,1,1,49,1,0,44,1,0,36,1,0,29,1,
    0,20,0,0,13,0,0,5,0,0,2,0,0,0,0,0,
    142,1,1,135,1,1,128,1,1,120,1,1,112,1,1,109,
    1,1,102,1,1,94,1,1,88,1,1,80,1,1,77,1,
    1,69,1,1,64,1,0,57,1,0,50,0,0,46,0,0,
    110,1,1,96,1,1,86,1,1,75,1,0,62,1,0,51,
    0,0,43,0,0,33,0,0,84,1,1,73,1,1,66,1,
    1,58,1,1,49,1,0,41,1,0,34,0,0,29,0,0,
    179,2,1,168,2,1,149,1,1,131,1,0,112,1,0,95,
    1,0,78,0,0,63,0,0,179,2,2,179,2,2,179,2,
    2,179,1,1,179,1,1,179,1,1,179,1,1,175,0,0,
    170,0,0,157,0,0,147,0,0,138,0,0,128,0,0,120,
    0,0,110,0,0,102,0,0,93,0,0,80,0,0,72,0,
    0,63,0,0,54,0,0,46,0,0,37,0,0,29,0,0,
    179,2,2,167,2,2,145,2,2,124,1,2,102,1,2,79,
    1,2,58,1,2,38,0,2,20,0,2,17,0,2,13,0,
    1,11,0,1,6,0,1,4,0,1,1,0,1,0,0,0,
    179,2,2,179,2,2,179,2,2,179,2,1,179,2,1,179,
    1,1,179,1,1,174,1,0,164,1,0,157,1,0,147,1,
    0,140,1,0,131,1,0,123,1,0,113,1,0,107,1,0,
    179,2,2,179,2,2,179,2,2,179,2,1,179,2,1,179,
    2,1,175,2,0,168,2,0,102,0,0,95,0,0,86,0,
    0,77,0,0,44,0,0,33,0,0,23,0,0,18,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,179,1,1,179,2,1,179,1,
    2,170,0,2,132,0,1,95,0,1,60,0,1,118,1,1
};

const byte C_B87A15[] = {
    0,0,0,6,2,0,2,0,0,47,31,5,184,122,21,6,
    4,1,1,1,0,0,0,0,0,0,0,23,18,2,12,11,
    0,3,4,0,0,1,0,45,22,3,38,18,2,31,13,1,
    184,106,18,184,99,17,184,94,16,184,87,15,179,82,14,171,
    77,13,167,70,12,159,65,11,151,60,10,145,56,10,138,51,
    9,134,48,8,126,44,7,123,39,7,115,34,6,111,32,6,
    104,28,5,100,25,4,92,22,4,89,19,3,81,16,3,77,
    14,2,71,11,2,67,9,2,61,7,1,58,4,1,51,2,
    0,48,1,0,43,1,0,38,0,0,32,0,0,30,0,0,
    184,122,21,184,122,21,184,122,20,184,118,19,184,116,18,184,
    111,17,184,106,16,184,103,15,184,98,13,184,93,13,181,88,
    12,172,82,11,165,77,10,157,72,9,149,67,8,145,65,8,
    136,62,7,125,57,7,119,54,6,111,51,6,105,46,5,94,
    44,5,88,39,4,81,36,4,74,34,4,65,30,3,55,26,
    2,46,23,2,39,19,2,31,15,1,22,12,0,17,8,0,
    184,122,21,184,122,21,184,122,21,183,122,21,175,116,20,167,
    111,19,163,108,19,155,103,18,147,98,17,144,95,16,136,90,
    15,132,88,15,124,82,14,116,77,13,113,75,13,105,70,12,
    97,65,11,94,62,11,87,57,10,79,52,9,75,50,9,68,
    45,8,61,41,7,58,38,7,51,33,6,44,29,5,40,27,
    5,33,22,4,30,20,3,24,16,3,17,11,2,14,9,2,
    108,122,12,97,119,11,89,109,10,81,100,9,74,90,8,66,
    81,7,58,72,6,51,65,5,45,56,4,37,47,3,30,39,
    3,21,30,2,14,22,1,5,14,0,2,7,0,0,1,0,
    146,87,13,139,82,12,131,77,12,123,72,11,115,67,10,112,
    63,9,105,59,9,97,54,8,90,50,7,82,45,6,79,43,
    6,71,38,5,66,36,5,58,31,4,51,27,4,48,24,3,
    113,64,9,99,56,7,89,49,6,77,42,5,63,34,4,53,
    28,3,44,22,2,34,17,1,87,59,8,75,52,7,68,47,
    6,59,42,5,51,35,4,42,31,4,35,26,3,30,22,2,
    184,122,12,172,108,9,153,89,7,134,72,5,115,56,4,97,
    39,2,80,27,1,65,16,0,184,122,21,184,122,21,184,108,
    19,184,90,16,184,74,13,184,60,10,184,44,7,180,28,5,
    175,14,2,161,12,2,152,11,2,142,10,2,132,9,2,123,
    8,1,113,7,1,105,5,1,95,4,1,82,2,0,74,2,
    0,65,1,0,56,1,0,47,0,0,38,0,0,30,0,0,
    184,122,21,172,114,21,149,99,21,127,84,21,105,70,21,81,
    54,21,60,40,21,39,26,20,21,14,20,17,11,17,14,9,
    15,11,7,13,6,4,11,4,2,9,1,1,7,0,0,5,
    184,122,21,184,122,21,184,120,19,184,109,16,184,97,13,184,
    87,10,184,75,7,179,65,4,169,59,4,161,55,3,152,51,
    3,144,46,3,134,42,2,126,37,1,116,33,1,110,31,1,
    184,122,21,184,122,21,184,122,18,184,122,14,184,122,11,184,
    122,8,180,120,5,172,116,2,105,28,1,97,24,1,89,21,
    0,79,15,0,45,22,3,34,16,1,24,9,0,18,4,0,
    0,0,5,0,0,4,0,0,3,0,0,2,0,0,1,0,
    0,0,0,0,0,0,0,0,184,82,8,184,113,8,184,74,
    21,175,14,20,136,10,15,97,4,11,61,1,7,121,56,10
};

const byte C_FFD000[] = {
    0,0,0,8,3,0,3,0,0,65,53,0,255,208,0,8,
    7,0,2,2,0,0,0,0,0,0,0,32,30,0,17,19,
    0,4,7,0,0,2,0,62,38,0,52,30,0,43,23,0,
    255,180,0,255,168,0,255,161,0,255,148,0,248,139,0,237,
    131,0,231,119,0,220,110,0,209,102,0,201,95,0,191,86,
    0,186,82,0,175,74,0,170,67,0,159,59,0,154,55,0,
    144,47,0,138,43,0,128,38,0,123,32,0,112,27,0,107,
    24,0,98,20,0,93,15,0,85,12,0,80,7,0,70,3,
    0,66,2,0,59,2,0,53,0,0,45,0,0,42,0,0,
    255,208,0,255,208,0,255,208,0,255,201,0,255,197,0,255,
    189,0,255,180,0,255,176,0,255,166,0,255,158,0,251,149,
    0,239,140,0,228,131,0,218,123,0,207,114,0,201,110,0,
    188,105,0,173,97,0,165,92,0,154,86,0,145,79,0,130,
    74,0,122,66,0,112,62,0,103,58,0,90,51,0,76,44,
    0,64,39,0,54,32,0,43,26,0,30,20,0,23,14,0,
    255,208,0,255,208,0,255,208,0,254,207,0,243,198,0,232,
    189,0,226,184,0,215,175,0,204,166,0,199,162,0,188,153,
    0,183,149,0,172,140,0,161,131,0,156,127,0,146,119,0,
    135,110,0,130,106,0,120,98,0,109,89,0,104,85,0,94,
    77,0,85,69,0,80,65,0,70,57,0,61,50,0,56,46,
    0,46,38,0,42,34,0,33,27,0,24,20,0,19,15,0,
    149,208,0,135,202,0,124,185,0,112,170,0,103,153,0,92,
    139,0,80,122,0,70,110,0,62,95,0,51,80,0,41,66,
    0,29,51,0,19,38,0,7,24,0,3,12,0,0,2,0,
    203,148,0,193,139,0,182,131,0,171,122,0,160,113,0,155,
    108,0,145,101,0,134,92,0,125,85,0,114,77,0,109,73,
    0,99,65,0,91,61,0,81,53,0,71,46,0,66,42,0,
    157,109,0,137,95,0,123,83,0,107,71,0,88,59,0,73,
    47,0,61,37,0,47,29,0,120,101,0,104,88,0,94,80,
    0,82,72,0,70,60,0,58,52,0,48,44,0,41,38,0,
    255,208,0,239,184,0,212,153,0,186,122,0,160,95,0,135,
    67,0,111,46,0,90,27,0,255,208,0,255,208,0,255,184,
    0,255,154,0,255,126,0,255,102,0,255,74,0,249,48,0,
    242,24,0,223,20,0,210,20,0,197,17,0,183,15,0,171,
    14,0,157,12,0,145,9,0,132,7,0,114,4,0,102,3,
    0,90,2,0,77,2,0,65,0,0,53,0,0,42,0,0,
    255,208,0,238,194,0,207,169,0,176,144,0,146,119,0,112,
    91,0,83,68,0,54,44,0,29,24,0,24,20,0,19,15,
    0,15,12,0,8,7,0,5,4,0,2,2,0,0,0,0,
    255,208,0,255,208,0,255,205,0,255,185,0,255,166,0,255,
    148,0,255,127,0,248,110,0,234,100,0,223,93,0,210,86,
    0,199,79,0,186,71,0,175,63,0,161,55,0,152,52,0,
    255,208,0,255,208,0,255,208,0,255,208,0,255,208,0,255,
    208,0,249,205,0,239,197,0,145,48,0,135,42,0,123,35,
    0,109,26,0,62,38,0,47,27,0,33,15,0,25,7,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,255,140,0,255,193,0,255,126,
    0,242,24,0,188,16,0,135,7,0,85,2,0,168,96,0
};

const byte C_FFDE4C[] = {
    0,0,0,8,3,0,3,0,0,65,57,19,255,222,76,8,
    7,2,2,2,1,0,0,0,0,0,0,32,32,6,17,20,
    1,4,7,0,0,2,0,62,40,10,52,32,7,43,24,5,
    255,192,66,255,179,61,255,172,59,255,158,54,248,149,51,237,
    139,48,231,127,44,220,118,40,209,109,37,201,102,35,191,92,
    32,186,88,30,175,79,27,170,71,24,159,63,21,154,58,20,
    144,50,17,138,46,16,128,41,14,123,34,12,112,29,10,107,
    25,9,98,21,7,93,17,6,85,13,4,80,7,2,70,3,
    1,66,3,1,59,3,1,53,0,0,45,0,0,42,0,0,
    255,222,76,255,222,75,255,222,71,255,215,67,255,211,64,255,
    202,61,255,192,56,255,188,54,255,178,49,255,169,45,251,159,
    42,239,150,39,228,140,36,218,131,33,207,122,30,201,118,29,
    188,112,27,173,104,25,165,98,23,154,92,21,145,84,19,130,
    79,18,122,71,16,112,66,14,103,62,13,90,54,11,76,47,
    9,64,42,7,54,34,6,43,28,4,30,22,1,23,15,1,
    255,222,76,255,222,76,255,222,76,254,221,76,243,212,72,232,
    202,69,226,197,67,215,187,64,204,178,61,199,173,59,188,164,
    56,183,159,55,172,150,51,161,140,48,156,136,46,146,127,44,
    135,118,40,130,113,39,120,104,36,109,95,32,104,91,31,94,
    82,28,85,74,25,80,70,24,70,61,21,61,53,18,56,49,
    17,46,40,14,42,37,13,33,29,10,24,21,7,19,17,6,
    149,222,42,135,216,38,124,198,35,112,182,32,103,164,28,92,
    148,24,80,131,21,70,118,18,62,102,14,51,85,13,41,71,
    10,29,55,6,19,40,4,7,25,1,3,13,0,0,3,0,
    203,158,48,193,149,45,182,139,42,171,131,39,160,121,36,155,
    115,34,145,108,31,134,98,28,125,91,26,114,82,23,109,78,
    22,99,70,19,91,65,18,81,57,15,71,49,13,66,44,12,
    157,117,32,137,102,26,123,89,23,107,76,18,88,63,14,73,
    50,11,61,39,8,47,31,5,120,108,30,104,94,26,94,85,
    23,82,77,20,70,64,15,58,56,13,48,47,10,41,40,8,
    255,222,44,239,196,33,212,163,26,186,131,18,160,102,13,135,
    71,8,111,50,3,90,29,1,255,222,76,255,222,76,255,197,
    67,255,165,56,255,135,46,255,109,37,255,79,27,249,51,18,
    242,25,9,223,22,7,210,21,7,197,18,6,183,17,6,171,
    15,5,157,13,4,145,10,3,132,7,2,114,4,1,102,3,
    1,90,3,1,77,2,1,65,0,0,53,0,0,42,0,0,
    255,222,76,238,207,76,207,180,76,176,153,76,146,127,76,112,
    98,76,83,72,76,54,47,74,29,25,72,24,21,63,19,17,
    55,15,13,47,8,7,39,5,4,32,2,2,24,0,0,17,
    255,222,76,255,222,76,255,219,67,255,198,56,255,177,46,255,
    158,36,255,136,26,248,118,16,234,107,14,223,99,12,210,92,
    11,199,84,10,186,76,7,175,67,5,161,59,5,152,56,4,
    255,222,76,255,222,76,255,222,64,255,222,52,255,222,41,255,
    222,30,249,219,19,239,211,9,145,51,3,135,44,2,123,37,
    2,109,28,1,62,40,10,47,29,5,33,17,2,25,7,0,
    0,0,17,0,0,13,0,0,10,0,0,7,0,0,3,0,
    0,1,0,0,0,0,0,0,255,150,28,255,206,31,255,135,
    76,242,25,71,188,17,56,135,7,39,85,2,24,168,103,35
};

const byte C_FFF588[] = {
    0,0,0,8,4,0,3,0,0,65,62,35,255,245,136,8,
    8,4,2,2,1,0,0,0,0,0,0,32,36,10,17,22,
    2,4,8,0,0,2,0,62,44,18,52,36,13,43,27,9,
    255,212,118,255,198,110,255,189,105,255,174,97,248,164,91,237,
    154,85,231,140,78,220,130,72,209,120,67,201,112,62,191,102,
    57,186,97,54,175,87,49,170,79,44,159,69,38,154,64,36,
    144,56,31,138,51,28,128,45,25,123,37,21,112,32,18,107,
    28,15,98,23,13,93,18,10,85,14,8,80,8,4,70,4,
    2,66,3,2,59,3,2,53,0,0,45,0,0,42,0,0,
    255,245,136,255,245,135,255,245,127,255,237,121,255,233,115,255,
    223,108,255,212,101,255,208,97,255,196,87,255,186,81,251,176,
    76,239,165,70,228,155,65,218,145,59,207,135,54,201,130,51,
    188,124,48,173,114,44,165,109,41,154,102,38,145,93,35,130,
    87,31,122,78,28,112,73,26,103,68,23,90,60,20,76,52,
    16,64,46,13,54,37,10,43,31,7,30,24,3,23,16,2,
    255,245,136,255,245,136,255,245,136,254,244,135,243,233,130,232,
    223,124,226,217,121,215,207,115,204,196,109,199,191,106,188,181,
    100,183,176,98,172,165,92,161,155,86,156,150,83,146,140,78,
    135,130,72,130,125,69,120,115,64,109,105,58,104,100,55,94,
    90,50,85,82,45,80,77,43,70,67,37,61,59,33,56,54,
    30,46,44,25,42,40,22,33,32,18,24,23,13,19,18,10,
    149,245,76,135,238,69,124,218,63,112,201,57,103,181,50,92,
    163,44,80,144,37,70,130,33,62,112,26,51,94,22,41,78,
    17,29,61,11,19,44,7,7,28,2,3,14,0,0,3,0,
    203,175,86,193,164,81,182,154,75,171,144,69,160,134,64,155,
    127,61,145,119,55,134,109,50,125,100,47,114,90,42,109,86,
    39,99,77,34,91,72,31,81,62,26,71,54,23,66,49,21,
    157,129,57,137,112,46,123,98,41,107,84,33,88,69,25,73,
    56,20,61,43,14,47,35,10,120,119,54,104,104,46,94,94,
    41,82,85,35,70,70,28,58,61,23,48,52,18,41,44,15,
    255,245,78,239,216,59,212,180,46,186,144,33,160,112,23,135,
    79,14,111,55,6,90,32,2,255,245,136,255,245,136,255,217,
    121,255,182,101,255,149,83,255,120,67,255,87,49,249,57,31,
    242,28,15,223,24,13,210,23,13,197,20,11,183,18,10,171,
    16,9,157,14,8,145,11,6,132,8,4,114,5,3,102,4,
    2,90,3,2,77,2,1,65,0,0,53,0,0,42,0,0,
    255,245,136,238,229,136,207,199,136,176,169,136,146,140,136,112,
    108,136,83,80,136,54,52,132,29,28,129,24,23,112,19,18,
    98,15,14,84,8,8,70,5,5,57,2,2,43,0,0,30,
    255,245,136,255,245,136,255,241,121,255,218,101,255,195,83,255,
    175,64,255,150,46,248,130,29,234,118,26,223,110,21,210,102,
    20,199,93,17,186,84,13,175,74,10,161,65,9,152,61,7,
    255,245,136,255,245,136,255,245,115,255,245,94,255,245,73,255,
    245,53,249,241,34,239,233,15,145,57,6,135,49,4,123,41,
    3,109,31,3,62,44,17,47,32,10,33,18,3,25,8,1,
    0,0,30,0,0,24,0,0,18,0,0,13,0,0,6,0,
    0,1,0,0,0,0,0,0,255,165,50,255,228,55,255,149,
    136,242,28,127,188,19,100,135,8,70,85,2,43,168,113,63
};

const byte C_043E8B[] = {
    0,0,0,0,1,0,0,0,0,1,16,35,4,62,139,0,
    2,4,0,0,1,0,0,0,0,0,0,1,9,10,0,6,
    2,0,2,0,0,0,0,1,11,19,1,9,14,1,7,9,
    4,54,120,4,50,112,4,48,107,4,44,99,4,42,93,4,
    39,87,4,35,80,3,33,74,3,30,68,3,28,64,3,26,
    58,3,25,55,3,22,50,3,20,45,2,18,39,2,16,37,
    2,14,32,2,13,29,2,11,26,2,9,21,2,8,18,2,
    7,16,2,6,13,1,5,10,1,4,8,1,2,4,1,1,
    2,1,1,2,1,1,2,1,0,0,1,0,0,1,0,0,
    4,62,139,4,62,138,4,62,130,4,60,123,4,59,118,4,
    56,111,4,54,103,4,53,99,4,50,89,4,47,83,4,44,
    77,4,42,72,4,39,67,3,37,61,3,34,55,3,33,52,
    3,31,49,3,29,45,3,27,42,2,26,39,2,24,35,2,
    22,32,2,20,29,2,18,26,2,17,23,1,15,20,1,13,
    16,1,12,14,1,9,10,1,8,8,0,6,3,0,4,2,
    4,62,139,4,62,139,4,62,139,4,62,138,4,59,132,4,
    56,126,4,55,123,3,52,117,3,50,111,3,48,108,3,46,
    102,3,44,100,3,42,94,3,39,88,2,38,85,2,35,80,
    2,33,74,2,32,71,2,29,65,2,27,59,2,25,57,1,
    23,51,1,21,46,1,19,44,1,17,38,1,15,33,1,14,
    31,1,11,25,1,10,23,1,8,18,0,6,13,0,5,10,
    2,62,77,2,60,70,2,55,64,2,51,58,2,46,51,1,
    41,45,1,36,38,1,33,33,1,28,26,1,24,23,1,20,
    17,0,15,11,0,11,8,0,7,2,0,4,0,0,1,0,
    3,44,88,3,42,82,3,39,76,3,36,71,3,34,65,2,
    32,62,2,30,57,2,27,51,2,25,48,2,23,43,2,22,
    40,2,19,35,1,18,32,1,16,27,1,14,24,1,12,21,
    2,33,58,2,28,47,2,25,41,2,21,34,1,18,26,1,
    14,20,1,11,15,1,9,10,2,30,55,2,26,47,1,24,
    41,1,21,36,1,18,28,1,16,23,1,13,18,1,11,15,
    4,62,80,4,55,61,3,45,47,3,36,34,3,28,23,2,
    20,15,2,14,6,1,8,2,4,62,139,4,62,139,4,55,
    123,4,46,103,4,38,84,4,30,68,4,22,50,4,14,32,
    4,7,16,3,6,14,3,6,13,3,5,11,3,5,10,3,
    4,9,2,4,8,2,3,6,2,2,4,2,1,3,2,1,
    2,1,1,2,1,0,1,1,0,0,1,0,0,1,0,0,
    4,62,139,4,58,139,3,50,139,3,43,139,2,35,139,2,
    27,139,1,20,139,1,13,135,0,7,132,0,6,114,0,5,
    100,0,4,86,0,2,72,0,1,58,0,0,44,0,0,31,
    4,62,139,4,62,139,4,61,123,4,55,103,4,49,84,4,
    44,65,4,38,47,4,33,29,4,30,26,3,28,21,3,26,
    20,3,24,17,3,21,14,3,19,10,3,17,9,2,16,8,
    4,62,139,4,62,139,4,62,118,4,62,96,4,62,75,4,
    62,54,4,61,34,4,59,16,2,14,6,2,12,4,2,10,
    3,2,8,3,1,11,17,1,8,10,1,5,3,0,2,1,
    0,0,31,0,0,25,0,0,18,0,0,13,0,0,6,0,
    0,1,0,0,0,0,0,0,4,42,51,4,58,56,4,38,
    139,4,7,130,3,5,102,2,2,72,1,0,44,3,29,64
};

const byte C_5B4318[] = {
    0,0,0,4,1,0,1,0,0,32,23,8,124,91,33,4,
    3,1,1,1,0,0,0,0,0,0,0,16,13,2,8,8,
    0,2,3,0,0,1,0,30,16,4,25,13,3,21,10,2,
    124,79,29,124,74,27,124,70,25,124,65,23,121,61,22,115,
    57,21,112,52,19,107,48,17,102,45,16,98,42,15,93,38,
    14,90,36,13,85,32,12,83,29,11,77,26,9,75,24,9,
    70,21,8,67,19,7,62,17,6,60,14,5,54,12,4,52,
    10,4,48,9,3,45,7,2,41,5,2,39,3,1,34,1,
    1,32,1,0,29,1,0,26,0,0,22,0,0,20,0,0,
    124,91,33,124,91,33,124,91,31,124,88,29,124,86,28,124,
    83,26,124,79,24,124,77,24,124,73,21,124,69,20,122,65,
    18,116,61,17,111,57,16,106,54,14,101,50,13,98,48,12,
    91,46,12,84,42,11,80,40,10,75,38,9,71,35,8,63,
    32,8,59,29,7,54,27,6,50,25,6,44,22,5,37,19,
    4,31,17,3,26,14,2,21,11,2,15,9,1,11,6,0,
    124,91,33,124,91,33,124,91,33,124,91,33,118,87,31,113,
    83,30,110,81,29,105,77,28,99,73,26,97,71,26,91,67,
    24,89,65,24,84,61,22,78,57,21,76,56,20,71,52,19,
    66,48,17,63,46,17,58,43,16,53,39,14,51,37,13,46,
    34,12,41,30,11,39,29,10,34,25,9,30,22,8,27,20,
    7,22,16,6,20,15,5,16,12,4,12,9,3,9,7,2,
    72,91,18,66,89,17,60,81,15,54,75,14,50,67,12,45,
    61,11,39,54,9,34,48,8,30,42,6,25,35,5,20,29,
    4,14,22,3,9,16,2,3,10,0,1,5,0,0,1,0,
    99,65,21,94,61,20,89,57,18,83,54,17,78,50,16,75,
    47,15,71,44,13,65,40,12,61,37,11,55,34,10,53,32,
    9,48,29,8,44,27,8,39,23,6,35,20,6,32,18,5,
    76,48,14,67,42,11,60,36,10,52,31,8,43,26,6,35,
    21,5,30,16,3,23,13,2,58,44,13,51,39,11,46,35,
    10,40,31,9,34,26,7,28,23,6,23,19,4,20,16,4,
    124,91,19,116,80,14,103,67,11,90,54,8,78,42,6,66,
    29,3,54,20,1,44,12,0,124,91,33,124,91,33,124,81,
    29,124,67,24,124,55,20,124,45,16,124,32,12,121,21,8,
    118,10,4,108,9,3,102,9,3,96,7,3,89,7,2,83,
    6,2,76,5,2,71,4,1,64,3,1,55,2,1,50,1,
    1,44,1,0,37,1,0,32,0,0,26,0,0,20,0,0,
    124,91,33,116,85,33,101,74,33,86,63,33,71,52,33,54,
    40,33,40,30,33,26,19,32,14,10,31,12,9,27,9,7,
    24,7,5,20,4,3,17,2,2,14,1,1,10,0,0,7,
    124,91,33,124,91,33,124,90,29,124,81,24,124,72,20,124,
    65,16,124,56,11,121,48,7,114,44,6,108,41,5,102,38,
    5,97,35,4,90,31,3,85,27,2,78,24,2,74,23,2,
    124,91,33,124,91,33,124,91,28,124,91,23,124,91,18,124,
    91,13,121,90,8,116,86,4,71,21,1,66,18,1,60,15,
    1,53,11,1,30,16,4,23,12,2,16,7,1,12,3,0,
    0,0,7,0,0,6,0,0,4,0,0,3,0,0,1,0,
    0,0,0,0,0,0,0,0,124,61,12,124,85,13,124,55,
    33,118,10,31,91,7,24,66,3,17,41,1,10,82,42,15
};

const byte C_4F5D8B[] = {
    0,0,0,2,1,0,1,0,0,20,24,35,79,93,139,2,
    3,4,1,1,1,0,0,0,0,0,0,10,13,10,5,8,
    2,1,3,0,0,1,0,19,17,19,16,13,14,13,10,9,
    79,81,120,79,75,112,79,72,107,79,66,99,77,62,93,73,
    58,87,72,53,80,68,49,74,65,46,68,62,43,64,59,39,
    58,58,37,55,54,33,50,53,30,45,49,26,39,48,24,37,
    45,21,32,43,19,29,40,17,26,38,14,21,35,12,18,33,
    11,16,30,9,13,29,7,10,26,5,8,25,3,4,22,1,
    2,20,1,2,18,1,2,16,0,0,14,0,0,13,0,0,
    79,93,139,79,93,138,79,93,130,79,90,123,79,88,118,79,
    85,111,79,81,103,79,79,99,79,74,89,79,71,83,78,67,
    77,74,63,72,71,59,67,68,55,61,64,51,55,62,49,52,
    58,47,49,54,43,45,51,41,42,48,39,39,45,35,35,40,
    33,32,38,30,29,35,28,26,32,26,23,28,23,20,24,20,
    16,20,18,14,17,14,10,13,12,8,9,9,3,7,6,2,
    79,93,139,79,93,139,79,93,139,79,93,138,75,89,132,72,
    85,126,70,82,123,67,78,117,63,74,111,62,73,108,58,69,
    102,57,67,100,53,63,94,50,59,88,48,57,85,45,53,80,
    42,49,74,40,47,71,37,44,65,34,40,59,32,38,57,29,
    34,51,26,31,46,25,29,44,22,26,38,19,22,33,17,20,
    31,14,17,25,13,15,23,10,12,18,7,9,13,6,7,10,
    46,93,77,42,90,70,38,83,64,35,76,58,32,69,51,29,
    62,45,25,55,38,22,49,33,19,43,26,16,36,23,13,30,
    17,9,23,11,6,17,8,2,11,2,1,5,0,0,1,0,
    63,66,88,60,62,82,56,58,76,53,55,71,50,51,65,48,
    48,62,45,45,57,42,41,51,39,38,48,35,34,43,34,33,
    40,31,29,35,28,27,32,25,24,27,22,20,24,20,19,21,
    49,49,58,42,43,47,38,37,41,33,32,34,27,26,26,23,
    21,20,19,16,15,15,13,10,37,45,55,32,39,47,29,36,
    41,25,32,36,22,27,28,18,23,23,15,20,18,13,17,15,
    79,93,80,74,82,61,66,68,47,58,55,34,50,43,23,42,
    30,15,34,21,6,28,12,2,79,93,139,79,93,139,79,82,
    123,79,69,103,79,57,84,79,46,68,79,33,50,77,22,32,
    75,11,16,69,9,14,65,9,13,61,8,11,57,7,10,53,
    6,9,49,5,8,45,4,6,41,3,4,35,2,3,32,1,
    2,28,1,2,24,1,1,20,0,0,16,0,0,13,0,0,
    79,93,139,74,87,139,64,75,139,55,64,139,45,53,139,35,
    41,139,26,30,139,17,20,135,9,11,132,7,9,114,6,7,
    100,5,5,86,2,3,72,2,2,58,1,1,44,0,0,31,
    79,93,139,79,93,139,79,92,123,79,83,103,79,74,84,79,
    66,65,79,57,47,77,49,29,72,45,26,69,42,21,65,39,
    20,62,35,17,58,32,14,54,28,10,50,25,9,47,23,8,
    79,93,139,79,93,139,79,93,118,79,93,96,79,93,75,79,
    93,54,77,92,34,74,88,16,45,22,6,42,19,4,38,16,
    3,34,12,3,19,17,17,15,12,10,10,7,3,8,3,1,
    0,0,31,0,0,25,0,0,18,0,0,13,0,0,6,0,
    0,1,0,0,0,0,0,0,79,63,51,79,86,56,79,57,
    139,75,11,130,58,7,102,42,3,72,26,1,44,52,43,64
};

const byte C_D46D3D[] = {
    0,0,0,7,2,0,2,0,0,54,28,16,212,109,61,7,
    3,2,2,1,0,0,0,0,0,0,0,27,16,5,14,10,
    1,3,3,0,0,1,0,52,20,8,43,16,6,36,12,4,
    212,94,53,212,88,49,212,84,47,212,77,43,206,73,41,197,
    68,38,192,62,35,183,58,32,174,53,30,167,50,28,159,45,
    25,155,43,24,145,39,22,141,35,20,132,31,17,128,29,16,
    120,25,14,115,23,13,106,20,11,102,17,9,93,14,8,89,
    12,7,81,10,6,77,8,5,71,6,4,67,3,2,58,2,
    1,55,1,1,49,1,1,44,0,0,37,0,0,35,0,0,
    212,109,61,212,109,61,212,109,57,212,106,54,212,103,52,212,
    99,49,212,94,45,212,92,44,212,87,39,212,83,36,209,78,
    34,199,74,32,190,69,29,181,65,27,172,60,24,167,58,23,
    156,55,22,144,51,20,137,48,18,128,45,17,121,41,16,108,
    39,14,101,35,13,93,32,11,86,30,10,75,27,9,63,23,
    7,53,21,6,45,17,5,36,14,3,25,11,1,19,7,1,
    212,109,61,212,109,61,212,109,61,211,109,61,202,104,58,193,
    99,55,188,97,54,179,92,51,170,87,49,165,85,48,156,80,
    45,152,78,44,143,74,41,134,69,39,130,67,37,121,62,35,
    112,58,32,108,56,31,100,51,29,91,47,26,86,44,25,78,
    40,22,71,36,20,67,34,19,58,30,17,51,26,15,47,24,
    13,38,20,11,35,18,10,27,14,8,20,10,6,16,8,5,
    124,109,34,112,106,31,103,97,28,93,89,25,86,80,22,76,
    73,20,67,64,17,58,58,15,52,50,11,42,42,10,34,35,
    8,24,27,5,16,20,3,6,12,1,2,6,0,0,1,0,
    169,78,39,160,73,36,151,68,33,142,64,31,133,59,29,129,
    56,27,121,53,25,111,48,22,104,44,21,95,40,19,91,38,
    17,82,34,15,76,32,14,67,28,12,59,24,11,55,22,9,
    131,57,26,114,50,21,102,44,18,89,37,15,73,31,11,61,
    25,9,51,19,6,39,15,4,100,53,24,86,46,21,78,42,
    18,68,38,16,58,31,12,48,27,10,40,23,8,34,20,7,
    212,109,35,199,96,27,176,80,21,155,64,15,133,50,10,112,
    35,6,92,24,3,75,14,1,212,109,61,212,109,61,212,97,
    54,212,81,45,212,66,37,212,53,30,212,39,22,207,25,14,
    201,12,7,185,11,6,175,10,6,164,9,5,152,8,5,142,
    7,4,131,6,4,121,5,3,110,3,2,95,2,1,85,2,
    1,75,1,1,64,1,0,54,0,0,44,0,0,35,0,0,
    212,109,61,198,102,61,172,88,61,146,75,61,121,62,61,93,
    48,61,69,35,61,45,23,59,24,12,58,20,10,50,16,8,
    44,12,6,38,7,3,32,4,2,25,2,1,19,0,0,14,
    212,109,61,212,109,61,212,107,54,212,97,45,212,87,37,212,
    78,29,212,67,21,206,58,13,195,53,11,185,49,9,175,45,
    9,165,41,8,155,37,6,145,33,4,134,29,4,126,27,3,
    212,109,61,212,109,61,212,109,52,212,109,42,212,109,33,212,
    109,24,207,107,15,199,103,7,121,25,3,112,22,2,102,18,
    1,91,14,1,52,20,8,39,14,4,27,8,1,21,3,0,
    0,0,14,0,0,11,0,0,8,0,0,6,0,0,3,0,
    0,0,0,0,0,0,0,0,212,74,22,212,101,25,212,66,
    61,201,12,57,156,9,45,112,3,32,71,1,19,140,50,28
};

const byte C_04918B[] = {
    0,0,0,0,2,0,0,0,0,1,37,35,4,145,139,0,
    5,4,0,1,1,0,0,0,0,0,0,1,21,10,0,13,
    2,0,5,0,0,1,0,1,26,19,1,21,14,1,16,9,
    4,126,120,4,117,112,4,112,107,4,103,99,4,97,93,4,
    91,87,4,83,80,3,77,74,3,71,68,3,67,64,3,60,
    58,3,57,55,3,52,50,3,47,45,2,41,39,2,38,37,
    2,33,32,2,30,29,2,27,26,2,22,21,2,19,18,2,
    16,16,2,14,13,1,11,10,1,9,8,1,5,4,1,2,
    2,1,2,2,1,2,2,1,0,0,1,0,0,1,0,0,
    4,145,139,4,145,138,4,145,130,4,140,123,4,138,118,4,
    132,111,4,126,103,4,123,99,4,116,89,4,110,83,4,104,
    77,4,98,72,4,92,67,3,86,61,3,80,55,3,77,52,
    3,73,49,3,68,45,3,64,42,2,60,39,2,55,35,2,
    52,32,2,46,29,2,43,26,2,40,23,1,35,20,1,31,
    16,1,27,14,1,22,10,1,18,8,0,14,3,0,10,2,
    4,145,139,4,145,139,4,145,139,4,144,138,4,138,132,4,
    132,126,4,129,123,3,122,117,3,116,111,3,113,108,3,107,
    102,3,104,100,3,98,94,3,92,88,2,89,85,2,83,80,
    2,77,74,2,74,71,2,68,65,2,62,59,2,59,57,1,
    53,51,1,48,46,1,45,44,1,40,38,1,35,33,1,32,
    31,1,26,25,1,24,23,1,19,18,0,14,13,0,11,10,
    2,145,77,2,141,70,2,129,64,2,119,58,2,107,51,1,
    97,45,1,85,38,1,77,33,1,67,26,1,56,23,1,46,
    17,0,36,11,0,26,8,0,16,2,0,9,0,0,2,0,
    3,103,88,3,97,82,3,91,76,3,85,71,3,79,65,2,
    75,62,2,71,57,2,64,51,2,59,48,2,53,43,2,51,
    40,2,45,35,1,43,32,1,37,27,1,32,24,1,29,21,
    2,76,58,2,67,47,2,58,41,2,49,34,1,41,26,1,
    33,20,1,26,15,1,20,10,2,71,55,2,61,47,1,56,
    41,1,50,36,1,42,28,1,36,23,1,31,18,1,26,15,
    4,145,80,4,128,61,3,106,47,3,85,34,3,67,23,2,
    47,15,2,32,6,1,19,2,4,145,139,4,145,139,4,129,
    123,4,107,103,4,88,84,4,71,68,4,52,50,4,34,32,
    4,16,16,3,14,14,3,14,13,3,12,11,3,11,10,3,
    10,9,2,9,8,2,6,6,2,5,4,2,3,3,2,2,
    2,1,2,2,1,1,1,1,0,0,1,0,0,1,0,0,
    4,145,139,4,135,139,3,118,139,3,100,139,2,83,139,2,
    64,139,1,47,139,1,31,135,0,16,132,0,14,114,0,11,
    100,0,9,86,0,5,72,0,3,58,0,1,44,0,0,31,
    4,145,139,4,145,139,4,143,123,4,129,103,4,115,84,4,
    103,65,4,89,47,4,77,29,4,70,26,3,65,21,3,60,
    20,3,55,17,3,49,14,3,44,10,3,39,9,2,36,8,
    4,145,139,4,145,139,4,145,118,4,145,96,4,145,75,4,
    145,54,4,143,34,4,138,16,2,34,6,2,29,4,2,24,
    3,2,18,3,1,26,17,1,19,10,1,11,3,0,5,1,
    0,0,31,0,0,25,0,0,18,0,0,13,0,0,6,0,
    0,1,0,0,0,0,0,0,4,98,51,4,135,56,4,88,
    139,4,16,130,3,11,102,2,5,72,1,1,44,3,67,64
};

const byte C_FF3030[] = {
    0,0,0,8,1,0,3,0,0,65,12,12,255,48,48,8,
    2,2,2,0,0,0,0,0,0,0,0,32,7,4,17,4,
    1,4,2,0,0,0,0,62,9,6,52,7,5,43,5,3,
    255,42,42,255,39,39,255,37,37,255,34,34,248,32,32,237,
    30,30,231,27,27,220,25,25,209,24,24,201,22,22,191,20,
    20,186,19,19,175,17,17,170,15,15,159,14,14,154,13,13,
    144,11,11,138,10,10,128,9,9,123,7,7,112,6,6,107,
    5,5,98,5,5,93,4,4,85,3,3,80,2,2,70,1,
    1,66,1,1,59,1,1,53,0,0,45,0,0,42,0,0,
    255,48,48,255,48,48,255,48,45,255,46,43,255,46,41,255,
    44,38,255,42,36,255,41,34,255,38,31,255,37,29,251,34,
    27,239,32,25,228,30,23,218,28,21,207,26,19,201,25,18,
    188,24,17,173,22,16,165,21,14,154,20,13,145,18,12,130,
    17,11,122,15,10,112,14,9,103,13,8,90,12,7,76,10,
    6,64,9,5,54,7,4,43,6,3,30,5,1,23,3,1,
    255,48,48,255,48,48,255,48,48,254,48,48,243,46,46,232,
    44,44,226,43,43,215,40,40,204,38,38,199,37,37,188,35,
    35,183,34,34,172,32,32,161,30,30,156,29,29,146,27,27,
    135,25,25,130,24,24,120,23,23,109,21,21,104,20,20,94,
    18,18,85,16,16,80,15,15,70,13,13,61,11,11,56,11,
    11,46,9,9,42,8,8,33,6,6,24,5,5,19,4,4,
    149,48,27,135,47,24,124,43,22,112,39,20,103,35,18,92,
    32,15,80,28,13,70,25,11,62,22,9,51,18,8,41,15,
    6,29,12,4,19,9,3,7,5,1,3,3,0,0,1,0,
    203,34,30,193,32,28,182,30,26,171,28,24,160,26,23,155,
    25,21,145,23,20,134,21,18,125,20,17,114,18,15,109,17,
    14,99,15,12,91,14,11,81,12,9,71,11,8,66,10,7,
    157,25,20,137,22,16,123,19,14,107,16,12,88,14,9,73,
    11,7,61,8,5,47,7,3,120,23,19,104,20,16,94,18,
    14,82,17,12,70,14,10,58,12,8,48,10,6,41,9,5,
    255,48,27,239,42,21,212,35,16,186,28,12,160,22,8,135,
    15,5,111,11,2,90,6,1,255,48,48,255,48,48,255,43,
    43,255,36,36,255,29,29,255,24,24,255,17,17,249,11,11,
    242,5,5,223,5,5,210,5,5,197,4,4,183,4,4,171,
    3,3,157,3,3,145,2,2,132,2,2,114,1,1,102,1,
    1,90,1,1,77,0,0,65,0,0,53,0,0,42,0,0,
    255,48,48,238,45,48,207,39,48,176,33,48,146,27,48,112,
    21,48,83,16,48,54,10,47,29,5,46,24,5,40,19,4,
    34,15,3,30,8,2,25,5,1,20,2,0,15,0,0,11,
    255,48,48,255,48,48,255,47,43,255,43,36,255,38,29,255,
    34,23,255,29,16,248,25,10,234,23,9,223,21,7,210,20,
    7,199,18,6,186,16,5,175,14,3,161,13,3,152,12,3,
    255,48,48,255,48,48,255,48,41,255,48,33,255,48,26,255,
    48,19,249,47,12,239,46,5,145,11,2,135,10,2,123,8,
    1,109,6,1,62,9,6,47,6,3,33,4,1,25,2,0,
    0,0,11,0,0,8,0,0,6,0,0,5,0,0,2,0,
    0,0,0,0,0,0,0,0,255,32,18,255,45,19,255,29,
    48,242,5,45,188,4,35,135,2,25,85,0,15,168,22,22
};

const byte C_311A59[] = {
    0,0,0,2,1,0,1,0,0,19,10,35,75,40,136,2,
    1,4,1,0,1,0,0,0,0,0,0,9,6,10,5,4,
    2,1,1,0,0,0,0,18,7,18,15,6,13,13,4,9,
    75,35,118,75,32,110,75,31,105,75,28,97,73,27,91,70,
    25,85,68,23,78,65,21,72,61,20,67,59,18,62,56,17,
    57,55,16,54,51,14,49,50,13,44,47,11,38,45,11,36,
    42,9,31,41,8,28,38,7,25,36,6,21,33,5,18,31,
    5,15,29,4,13,27,3,10,25,2,8,24,1,4,21,1,
    2,19,0,2,17,0,2,16,0,0,13,0,0,12,0,0,
    75,40,136,75,40,135,75,40,127,75,39,121,75,38,115,75,
    36,108,75,35,101,75,34,97,75,32,87,75,30,81,74,29,
    76,70,27,70,67,25,65,64,24,59,61,22,54,59,21,51,
    55,20,48,51,19,44,49,18,41,45,17,38,43,15,35,38,
    14,31,36,13,28,33,12,26,30,11,23,26,10,20,22,8,
    16,19,8,13,16,6,10,13,5,7,9,4,3,7,3,2,
    75,40,136,75,40,136,75,40,136,75,40,135,71,38,130,68,
    36,124,66,35,121,63,34,115,60,32,109,59,31,106,55,29,
    100,54,29,98,51,27,92,47,25,86,46,24,83,43,23,78,
    40,21,72,38,20,69,35,19,64,32,17,58,31,16,55,28,
    15,50,25,13,45,24,13,43,21,11,37,18,10,33,16,9,
    30,14,7,25,12,7,22,10,5,18,7,4,13,6,3,10,
    44,40,76,40,39,69,36,36,63,33,33,57,30,29,50,27,
    27,44,24,24,37,21,21,33,18,18,26,15,15,22,12,13,
    17,9,10,11,6,7,7,2,5,2,1,2,0,0,0,0,
    60,29,86,57,27,81,54,25,75,50,24,69,47,22,64,46,
    21,61,43,19,55,39,18,50,37,16,47,34,15,42,32,14,
    39,29,13,34,27,12,31,24,10,26,21,9,23,19,8,21,
    46,21,57,40,18,46,36,16,41,31,14,33,26,11,25,21,
    9,20,18,7,14,14,6,10,35,19,54,31,17,46,28,15,
    41,24,14,35,21,11,28,17,10,23,14,8,18,12,7,15,
    75,40,78,70,35,59,62,29,46,55,24,33,47,18,23,40,
    13,14,33,9,6,26,5,2,75,40,136,75,40,136,75,35,
    121,75,30,101,75,24,83,75,20,67,75,14,49,73,9,31,
    71,5,15,66,4,13,62,4,13,58,3,11,54,3,10,50,
    3,9,46,2,8,43,2,6,39,1,4,34,1,3,30,1,
    2,26,0,2,23,0,1,19,0,0,16,0,0,12,0,0,
    75,40,136,70,37,136,61,32,136,52,28,136,43,23,136,33,
    18,136,24,13,136,16,8,132,9,5,129,7,4,112,6,3,
    98,4,2,84,2,1,70,1,1,57,1,0,43,0,0,30,
    75,40,136,75,40,136,75,39,121,75,36,101,75,32,83,75,
    29,64,75,24,46,73,21,29,69,19,26,66,18,21,62,17,
    20,59,15,17,55,14,13,51,12,10,47,11,9,45,10,7,
    75,40,136,75,40,136,75,40,115,75,40,94,75,40,73,75,
    40,53,73,39,34,70,38,15,43,9,6,40,8,4,36,7,
    3,32,5,3,18,7,17,14,5,10,10,3,3,7,1,1,
    0,0,30,0,0,24,0,0,18,0,0,13,0,0,6,0,
    0,1,0,0,0,0,0,0,75,27,50,75,37,55,75,24,
    136,71,5,127,55,3,100,40,1,70,25,0,43,49,19,63
};

const byte C_FFAFAF[] = {
    0,0,0,8,3,0,3,0,0,65,45,45,255,175,175,8,
    5,5,2,1,1,0,0,0,0,0,0,32,25,13,17,16,
    2,4,5,0,0,1,0,62,32,23,52,25,17,43,19,12,
    255,152,152,255,141,141,255,135,135,255,124,124,248,117,117,237,
    110,110,231,100,100,220,93,93,209,86,86,201,80,80,191,73,
    73,186,69,69,175,62,62,170,56,56,159,49,49,154,46,46,
    144,40,40,138,36,36,128,32,32,123,27,27,112,23,23,107,
    20,20,98,16,16,93,13,13,85,10,10,80,5,5,70,3,
    3,66,2,2,59,2,2,53,0,0,45,0,0,42,0,0,
    255,175,175,255,175,174,255,175,163,255,170,155,255,166,148,255,
    159,139,255,152,130,255,148,125,255,140,112,255,133,104,251,126,
    97,239,118,91,228,110,84,218,104,76,207,96,69,201,93,66,
    188,89,62,173,82,57,165,78,53,154,73,49,145,67,45,130,
    62,40,122,56,36,112,52,33,103,49,30,90,43,25,76,37,
    21,64,33,17,54,27,13,43,22,10,30,17,3,23,12,2,
    255,175,175,255,175,175,255,175,175,254,174,174,243,167,167,232,
    159,159,226,155,155,215,148,148,204,140,140,199,137,137,188,129,
    129,183,126,126,172,118,118,161,110,110,156,107,107,146,100,100,
    135,93,93,130,89,89,120,82,82,109,75,75,104,71,71,94,
    65,65,85,58,58,80,55,55,70,48,48,61,42,42,56,38,
    38,46,32,32,42,29,29,33,23,23,24,16,16,19,13,13,
    149,175,97,135,170,89,124,156,81,112,143,73,103,129,64,92,
    117,56,80,103,48,70,93,42,62,80,33,51,67,29,41,56,
    22,29,43,14,19,32,10,7,20,2,3,10,0,0,2,0,
    203,125,110,193,117,104,182,110,96,171,103,89,160,95,82,155,
    91,78,145,85,71,134,78,65,125,71,60,114,65,54,109,62,
    50,99,55,44,91,51,40,81,45,34,71,38,30,66,35,27,
    157,92,73,137,80,60,123,70,52,107,60,43,88,49,32,73,
    40,25,61,31,19,47,25,12,120,85,69,104,74,59,94,67,
    52,82,60,45,70,50,36,58,44,30,48,37,23,41,32,19,
    255,175,100,239,154,76,212,128,59,186,103,43,160,80,30,135,
    56,19,111,39,8,90,23,2,255,175,175,255,175,175,255,155,
    155,255,130,130,255,106,106,255,86,86,255,62,62,249,40,40,
    242,20,20,223,17,17,210,16,16,197,14,14,183,13,13,171,
    12,12,157,10,10,145,8,8,132,5,5,114,3,3,102,3,
    3,90,2,2,77,1,1,65,0,0,53,0,0,42,0,0,
    255,175,175,238,163,175,207,142,175,176,121,175,146,100,175,112,
    77,175,83,57,175,54,37,170,29,20,166,24,16,144,19,13,
    126,15,10,108,8,5,91,5,3,73,2,1,56,0,0,39,
    255,175,175,255,175,175,255,172,155,255,156,130,255,139,106,255,
    125,82,255,107,60,248,93,37,234,84,33,223,78,27,210,73,
    25,199,67,22,186,60,17,175,53,12,161,47,11,152,44,10,
    255,175,175,255,175,175,255,175,148,255,175,121,255,175,94,255,
    175,68,249,172,43,239,166,20,145,40,8,135,35,5,123,30,
    4,109,22,3,62,32,22,47,23,12,33,13,4,25,5,1,
    0,0,39,0,0,31,0,0,23,0,0,16,0,0,8,0,
    0,1,0,0,0,0,0,0,255,118,65,255,163,71,255,106,
    175,242,20,164,188,14,128,135,5,91,85,1,56,168,81,81
};

const byte C_ECB866[] = {
    0,0,0,7,3,0,3,0,0,60,47,26,236,184,102,7,
    6,3,2,1,1,0,0,0,0,0,0,30,27,8,16,17,
    1,4,6,0,0,1,0,57,33,14,48,27,10,40,20,7,
    236,159,88,236,149,82,236,142,79,236,131,72,230,123,68,219,
    115,64,214,105,58,204,97,54,193,90,50,186,84,47,177,76,
    42,172,73,40,162,66,36,157,59,33,147,52,29,143,48,27,
    133,42,23,128,38,21,118,34,19,114,28,16,104,24,13,99,
    21,12,91,17,10,86,14,8,79,11,6,74,6,3,65,3,
    2,61,2,1,55,2,1,49,0,0,42,0,0,39,0,0,
    236,184,102,236,184,101,236,184,95,236,178,90,236,175,86,236,
    167,81,236,159,76,236,156,73,236,147,65,236,140,61,232,132,
    57,221,124,53,211,116,49,202,109,44,192,101,40,186,97,38,
    174,93,36,160,86,33,153,82,31,143,76,28,134,70,26,120,
    66,24,113,58,21,104,55,19,95,51,17,83,45,15,70,39,
    12,59,35,10,50,28,8,40,23,6,28,18,2,21,12,1,
    236,184,102,236,184,102,236,184,102,235,183,102,225,175,97,215,
    167,93,209,163,90,199,155,86,189,147,82,184,144,80,174,136,
    75,169,132,73,159,124,69,149,116,64,144,113,62,135,105,58,
    125,97,54,120,94,52,111,87,48,101,79,44,96,75,42,87,
    68,38,79,61,34,74,58,32,65,51,28,56,44,24,52,40,
    22,43,33,18,39,30,17,31,24,13,22,17,10,18,14,8,
    138,184,57,125,179,52,115,164,47,104,151,42,95,136,37,85,
    123,33,74,108,28,65,97,24,57,84,19,47,71,17,38,58,
    13,27,45,8,18,33,6,6,21,1,3,11,0,0,2,0,
    188,131,64,179,123,60,168,115,56,158,108,52,148,100,48,143,
    95,46,134,89,42,124,82,38,116,75,35,106,68,31,101,65,
    29,92,58,26,84,54,24,75,47,20,66,40,18,61,37,16,
    145,97,43,127,84,35,114,74,30,99,63,25,81,52,19,68,
    42,15,56,32,11,43,26,7,111,89,40,96,78,34,87,71,
    30,76,63,26,65,53,21,54,46,17,44,39,13,38,33,11,
    236,184,58,221,162,44,196,135,34,172,108,25,148,84,17,125,
    59,11,103,41,4,83,24,1,236,184,102,236,184,102,236,163,
    90,236,136,76,236,112,62,236,90,50,236,66,36,230,43,24,
    224,21,12,206,18,10,194,17,10,182,15,8,169,14,8,158,
    12,7,145,11,6,134,8,4,122,6,3,106,4,2,94,3,
    2,83,2,1,71,1,1,60,0,0,49,0,0,39,0,0,
    236,184,102,220,172,102,192,149,102,163,127,102,135,105,102,104,
    81,102,77,60,102,50,39,99,27,21,97,22,17,84,18,14,
    73,14,11,63,7,6,53,5,4,42,2,1,32,0,0,23,
    236,184,102,236,184,102,236,181,90,236,164,76,236,146,62,236,
    131,48,236,113,35,230,97,22,217,89,19,206,82,16,194,76,
    15,184,70,13,172,63,10,162,56,7,149,49,6,141,46,6,
    236,184,102,236,184,102,236,184,86,236,184,70,236,184,55,236,
    184,40,230,181,25,221,175,12,134,43,4,125,37,3,114,31,
    2,101,23,2,57,33,13,43,24,7,31,14,2,23,6,0,
    0,0,23,0,0,18,0,0,13,0,0,10,0,0,4,0,
    0,1,0,0,0,0,0,0,236,124,38,236,171,41,236,112,
    102,224,21,96,174,14,75,125,6,53,79,1,32,155,85,47
};

const byte C_C63F23[] = {
    0,0,0,6,1,0,2,0,0,50,16,9,198,63,35,6,
    2,1,2,0,0,0,0,0,0,0,0,25,9,3,13,6,
    0,3,2,0,0,0,0,48,11,5,40,9,3,33,7,2,
    198,55,30,198,51,28,198,49,27,198,45,25,193,42,23,184,
    40,22,179,36,20,171,33,19,162,31,17,156,29,16,148,26,
    15,144,25,14,136,22,12,132,20,11,123,18,10,120,17,9,
    112,14,8,107,13,7,99,12,6,96,10,5,87,8,5,83,
    7,4,76,6,3,72,5,3,66,4,2,62,2,1,54,1,
    1,51,1,0,46,1,0,41,0,0,35,0,0,33,0,0,
    198,63,35,198,63,35,198,63,33,198,61,31,198,60,30,198,
    57,28,198,55,26,198,53,25,198,50,22,198,48,21,195,45,
    19,186,42,18,177,40,17,169,37,15,161,35,14,156,33,13,
    146,32,12,134,29,11,128,28,11,120,26,10,113,24,9,101,
    22,8,95,20,7,87,19,7,80,18,6,70,15,5,59,13,
    4,50,12,3,42,10,3,33,8,2,23,6,1,18,4,0,
    198,63,35,198,63,35,198,63,35,197,63,35,189,60,33,180,
    57,32,175,56,31,167,53,30,158,50,28,155,49,27,146,46,
    26,142,45,25,134,42,24,125,40,22,121,39,21,113,36,20,
    105,33,19,101,32,18,93,30,16,85,27,15,81,26,14,73,
    23,13,66,21,12,62,20,11,54,17,10,47,15,8,43,14,
    8,36,11,6,33,10,6,26,8,5,19,6,3,15,5,3,
    116,63,19,105,61,18,96,56,16,87,52,15,80,46,13,71,
    42,11,62,37,10,54,33,8,48,29,7,40,24,6,32,20,
    4,23,16,3,15,11,2,5,7,0,2,4,0,0,1,0,
    158,45,22,150,42,21,141,40,19,133,37,18,124,34,16,120,
    33,16,113,31,14,104,28,13,97,26,12,89,23,11,85,22,
    10,77,20,9,71,19,8,63,16,7,55,14,6,51,13,5,
    122,33,15,106,29,12,96,25,10,83,21,9,68,18,6,57,
    14,5,47,11,4,36,9,2,93,31,14,81,27,12,73,24,
    10,64,22,9,54,18,7,45,16,6,37,13,5,32,11,4,
    198,63,20,186,56,15,165,46,12,144,37,9,124,29,6,105,
    20,4,86,14,2,70,8,0,198,63,35,198,63,35,198,56,
    31,198,47,26,198,38,21,198,31,17,198,22,12,193,15,8,
    188,7,4,173,6,3,163,6,3,153,5,3,142,5,3,133,
    4,2,122,4,2,113,3,2,102,2,1,89,1,1,79,1,
    1,70,1,0,60,0,0,50,0,0,41,0,0,33,0,0,
    198,63,35,185,59,35,161,51,35,137,43,35,113,36,35,87,
    28,35,64,21,35,42,13,34,23,7,33,19,6,29,15,5,
    25,12,4,22,6,2,18,4,1,15,2,0,11,0,0,8,
    198,63,35,198,63,35,198,62,31,198,56,26,198,50,21,198,
    45,16,198,39,12,193,33,7,182,30,7,173,28,5,163,26,
    5,155,24,4,144,21,3,136,19,2,125,17,2,118,16,2,
    198,63,35,198,63,35,198,63,30,198,63,24,198,63,19,198,
    63,14,193,62,9,186,60,4,113,15,2,105,13,1,96,11,
    1,85,8,1,48,11,4,36,8,2,26,5,1,19,2,0,
    0,0,8,0,0,6,0,0,5,0,0,3,0,0,2,0,
    0,0,0,0,0,0,0,0,198,42,13,198,59,14,198,38,
    35,188,7,33,146,5,26,105,2,18,66,0,11,130,29,16
};

const byte C_9BC8CD[] = {
    0,0,0,5,3,0,2,0,0,40,51,52,155,200,205,5,
    6,6,1,2,2,0,0,0,0,0,0,19,29,15,10,18,
    2,2,6,0,0,2,0,38,36,27,32,29,20,26,22,14,
    155,173,178,155,162,166,155,155,158,155,142,146,151,134,137,144,
    125,129,140,115,117,134,106,109,127,98,100,122,92,94,116,83,
    85,113,79,81,106,71,73,103,64,66,97,56,58,94,53,54,
    88,45,47,84,42,43,78,37,38,75,31,31,68,26,27,65,
    23,23,60,19,19,57,15,15,52,12,12,49,6,6,43,3,
    3,40,2,2,36,2,2,32,0,0,27,0,0,26,0,0,
    155,200,205,155,200,203,155,200,191,155,194,182,155,190,174,155,
    182,163,155,173,152,155,169,146,155,160,131,155,152,122,153,144,
    114,145,135,106,139,126,98,133,118,89,126,110,81,122,106,77,
    114,101,72,105,93,67,100,89,62,94,83,57,88,76,52,79,
    71,47,74,64,43,68,60,39,63,56,35,55,49,30,46,42,
    24,39,38,20,33,31,15,26,25,11,18,20,4,14,13,2,
    155,200,205,155,200,205,155,200,205,154,199,204,148,191,195,141,
    182,187,137,177,182,131,169,173,124,160,164,121,156,160,114,147,
    151,111,144,147,105,135,138,98,126,129,95,122,125,89,115,117,
    82,106,109,79,102,105,73,94,96,66,85,88,63,82,84,57,
    74,76,52,67,68,49,63,64,43,55,56,37,48,49,34,44,
    45,28,36,37,26,33,34,20,26,27,15,19,19,12,15,15,
    91,200,114,82,195,104,75,178,95,68,164,85,63,147,75,56,
    133,66,49,118,56,43,106,49,38,92,39,31,77,34,25,64,
    26,18,49,16,12,36,11,4,23,2,2,12,0,0,2,0,
    123,143,129,117,134,121,111,125,113,104,118,105,97,109,96,94,
    104,92,88,97,84,81,89,76,76,82,71,69,74,63,66,71,
    59,60,63,51,55,59,47,49,51,39,43,44,35,40,40,31,
    95,105,86,83,92,70,75,80,61,65,68,50,53,56,38,44,
    45,30,37,35,22,29,28,14,73,97,81,63,85,69,57,77,
    61,50,69,53,43,57,42,35,50,35,29,42,27,25,36,23,
    155,200,117,145,176,89,129,147,69,113,118,50,97,92,35,82,
    64,22,67,45,9,55,26,2,155,200,205,155,200,205,155,177,
    182,155,148,152,155,122,125,155,98,100,155,71,73,151,46,47,
    147,23,23,136,20,20,128,19,19,120,16,17,111,15,15,104,
    13,14,95,12,12,88,9,9,80,6,6,69,4,4,62,3,
    3,55,2,2,47,2,2,40,0,0,32,0,0,26,0,0,
    155,200,205,145,187,205,126,162,205,107,138,205,89,115,205,68,
    88,205,50,65,205,33,42,199,18,23,195,15,19,169,12,15,
    147,9,12,126,5,6,106,3,4,85,1,2,65,0,0,46,
    155,200,205,155,200,205,155,197,182,155,178,152,155,159,125,155,
    143,96,155,122,70,151,106,43,142,96,39,136,89,31,128,83,
    30,121,76,26,113,68,20,106,60,14,98,53,13,92,50,11,
    155,200,205,155,200,205,155,200,174,155,200,141,155,200,110,155,
    200,80,151,197,51,145,190,23,88,46,9,82,40,6,75,34,
    5,66,25,4,38,36,26,29,26,14,20,15,5,15,6,1,
    0,0,46,0,0,36,0,0,27,0,0,19,0,0,9,0,
    0,2,0,0,0,0,0,0,155,135,76,155,186,83,155,122,
    205,147,23,192,114,16,150,82,6,106,52,2,65,102,93,95
};

const byte C_666666[] = {
    0,0,0,3,4,0,1,0,0,33,65,33,255,255,255,3,
    8,3,1,2,1,0,0,0,0,0,0,14,37,7,6,23,
    1,1,8,0,0,2,0,31,46,15,25,37,10,19,28,6,
    255,221,202,255,206,180,255,197,167,255,181,146,243,171,134,226,
    160,120,217,146,105,200,135,93,184,125,83,173,117,75,159,106,
    65,153,101,61,139,91,52,132,82,45,119,72,38,114,67,35,
    102,58,28,96,53,25,86,47,22,81,39,17,70,33,14,66,
    29,12,58,24,10,54,19,7,47,15,6,44,8,3,36,4,
    1,34,3,1,29,3,1,25,0,0,21,0,0,19,0,0,
    255,255,255,255,255,252,255,255,227,255,247,209,255,242,194,255,
    232,176,255,221,157,255,216,148,255,204,124,255,194,112,248,183,
    100,229,172,90,212,161,80,197,151,69,181,140,61,173,135,56,
    156,129,52,136,119,46,127,113,41,114,106,37,103,97,33,88,
    91,29,80,81,25,70,76,22,63,71,19,52,62,16,41,54,
    13,32,48,10,26,39,7,19,32,5,13,25,2,9,17,1,
    255,255,255,255,255,255,255,255,255,253,254,253,236,243,236,218,
    232,218,209,226,209,193,215,193,177,204,177,170,199,170,156,188,
    156,149,183,149,135,172,135,122,161,122,116,156,116,105,146,105,
    93,135,93,88,130,88,78,120,78,68,109,68,63,104,63,55,
    94,55,47,85,47,44,80,44,36,70,36,31,61,31,27,56,
    27,21,46,21,19,42,19,14,33,14,10,24,10,7,19,7,
    108,255,100,93,248,87,82,227,76,70,209,65,63,188,54,53,
    170,45,44,150,36,36,135,31,31,117,22,24,98,19,19,81,
    14,12,63,8,7,46,5,2,29,1,1,15,0,0,3,0,
    176,182,122,162,171,110,148,160,98,134,150,88,120,139,78,115,
    132,72,103,124,63,92,113,55,83,104,50,72,94,42,68,90,
    39,59,80,32,52,75,29,45,65,23,37,56,20,34,51,17,
    117,134,66,95,117,49,81,102,41,66,87,31,50,72,22,39,
    58,16,31,45,11,22,36,7,78,124,61,63,108,48,55,98,
    41,45,88,34,36,73,25,28,64,19,22,54,14,19,46,12,
    255,255,105,229,225,69,188,187,48,153,150,31,120,117,19,93,
    82,11,69,57,4,52,33,1,255,255,255,255,255,255,255,226,
    209,255,189,157,255,155,115,255,125,83,255,91,52,245,59,29,
    234,29,12,205,25,10,186,24,10,167,21,8,149,19,7,134,
    17,6,117,15,6,103,11,4,90,8,3,72,5,2,62,4,
    1,52,3,1,41,2,1,33,0,0,25,0,0,19,0,0,
    255,255,255,227,238,255,181,207,255,140,176,255,105,146,255,70,
    112,255,46,83,255,26,54,243,12,29,234,10,24,186,7,19,
    149,6,15,117,3,8,90,2,5,65,1,2,45,0,0,28,
    255,255,255,255,255,255,255,251,209,255,227,157,255,203,115,255,
    182,78,255,156,49,243,135,26,221,123,22,205,114,17,186,106,
    16,170,97,14,153,87,10,139,77,7,122,68,6,112,64,5,
    255,255,255,255,255,255,255,255,194,255,255,140,255,255,95,255,
    255,59,245,251,32,229,242,12,103,59,4,93,51,3,81,43,
    2,68,32,2,31,46,14,22,33,7,14,19,2,10,8,0,
    0,0,28,0,0,21,0,0,14,0,0,10,0,0,4,0,
    0,1,0,0,0,0,0,0,255,172,55,255,237,63,255,155,
    255,234,29,229,156,20,154,93,8,90,47,2,45,130,118,76
};

const byte C_777777[] = {
    0,0,0,8,0,0,3,0,0,65,17,17,255,255,255,8,
    0,0,2,0,0,0,0,0,0,0,0,32,5,1,17,2,
    0,4,0,0,0,0,0,62,8,5,52,5,2,43,3,1,
    255,192,192,255,166,166,255,152,152,255,128,128,248,115,115,237,
    100,100,231,84,84,220,71,71,209,61,61,201,54,54,191,44,
    44,186,40,40,175,32,32,170,26,26,159,20,20,154,18,18,
    144,13,13,138,11,11,128,9,9,123,6,6,112,4,4,107,
    3,3,98,2,2,93,1,1,85,1,1,80,0,0,70,0,
    0,66,0,0,59,0,0,53,0,0,45,0,0,42,0,0,
    255,255,255,255,255,251,255,255,222,255,239,200,255,230,183,255,
    211,162,255,192,140,255,183,130,255,163,104,255,148,91,251,131,
    79,239,116,68,228,102,58,218,89,48,207,77,40,201,71,36,
    188,65,32,173,56,27,165,50,23,154,44,20,145,37,17,130,
    32,14,122,26,11,112,23,9,103,20,7,90,15,5,76,11,
    4,64,9,2,54,6,1,43,4,1,30,2,0,23,1,0,
    255,255,255,255,255,255,255,255,255,254,253,253,243,232,232,232,
    211,211,226,200,200,215,181,181,204,163,163,199,155,155,188,139,
    139,183,131,131,172,116,116,161,102,102,156,95,95,146,84,84,
    135,71,71,130,66,66,120,56,56,109,47,47,104,42,42,94,
    35,35,85,28,28,80,25,25,70,19,19,61,15,15,56,12,
    12,46,8,8,42,7,7,33,4,4,24,2,2,19,1,1,
    149,255,79,135,241,65,124,202,55,112,171,44,103,139,34,92,
    113,26,80,88,19,70,71,15,62,54,9,51,38,7,41,26,
    4,29,16,2,19,8,1,7,3,0,3,1,0,0,0,0,
    203,130,102,193,115,89,182,100,77,171,88,66,160,76,56,155,
    68,51,145,60,42,134,50,35,125,42,30,114,35,24,109,32,
    21,99,25,16,91,22,14,81,17,9,71,12,8,66,10,6,
    157,70,45,137,54,30,123,41,23,107,30,15,88,20,9,73,
    13,5,61,8,3,47,5,1,120,60,40,104,46,29,94,38,
    23,82,30,17,70,21,11,58,16,7,48,11,4,41,8,3,
    255,255,84,239,199,48,212,137,29,186,88,15,160,54,7,135,
    26,3,111,13,0,90,4,0,255,255,255,255,255,255,255,200,
    200,255,140,140,255,94,94,255,61,61,255,32,32,249,14,14,
    242,3,3,223,2,2,210,2,2,197,2,2,183,1,1,171,
    1,1,157,1,1,145,0,0,132,0,0,114,0,0,102,0,
    0,90,0,0,77,0,0,65,0,0,53,0,0,42,0,0,
    255,255,255,238,222,255,207,168,255,176,121,255,146,84,255,112,
    49,255,83,27,255,54,11,241,29,3,230,24,2,173,19,1,
    131,15,1,97,8,0,68,5,0,44,2,0,26,0,0,13,
    255,255,255,255,255,255,255,247,200,255,202,140,255,162,94,255,
    130,56,255,95,30,248,71,11,234,59,9,223,51,6,210,44,
    5,199,37,4,186,30,2,175,23,1,161,18,1,152,16,1,
    255,255,255,255,255,255,255,255,183,255,255,121,255,255,74,255,
    255,38,249,247,16,239,230,3,145,14,0,135,10,0,123,7,
    0,109,4,0,62,8,4,47,4,1,33,1,0,25,0,0,
    0,0,13,0,0,8,0,0,4,0,0,2,0,0,0,0,
    0,0,0,0,0,0,0,0,255,116,35,255,220,42,255,94,
    255,242,3,224,188,2,137,135,0,68,85,0,26,168,55,55
};
