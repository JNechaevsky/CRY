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

#include "id_vars.h"
#include "id_clght.h"


// =============================================================================
//
//                            COLORED SECTOR LIGHTING
//
// =============================================================================

// Main colormaps
lighttable_t   *colormaps_EEC06B;  // Bright yellow/gold
lighttable_t   *colormaps_D97C45;  // Middle yellow/gold
lighttable_t   *colormaps_FF7F7F;  // Bright red
lighttable_t   *colormaps_55B828;  // Bright green
lighttable_t   *colormaps_BBE357;  // Slime green
lighttable_t   *colormaps_949DB9;  // Bright desaturated blue
lighttable_t   *colormaps_2A2F6B;  // Middle desaturated blue
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
lighttable_t   *colormaps_FFAFAF;  // Brighter red
lighttable_t   *colormaps_ECB866;  // Bright orange
lighttable_t   *colormaps_C63F23;  // Middle orange 3
lighttable_t   *colormaps_9BC8CD;  // Bright cyan

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
}

// =============================================================================
//
//                  COLORED VISPLANES INITIALIZATION FUNCTIONS
//
// =============================================================================

void R_ColoredVisplanesFreeI (int i)
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
}

void R_ColoredVisplanesFree (void)
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
}

void R_ColoredVisplanesMalloc (void)
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
}

void R_ColoredVisplanesMAXLIGHTZ (int i)
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
}

void R_ColoredVisplanesIJLevel (int i, int j, int level)
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
}

// =============================================================================
//
//              COLORED SEGMENTS/SPRITES INITIALIZATION FUNCTIONS
//
// =============================================================================

void R_ColoredSegsFreeI (int i)
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
}

void R_ColoredSegsFree (void)
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
}

void R_ColoredSegsMalloc (void)
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
}

void R_ColoredSegsMAXLIGHTSCALE (int i)
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
}

void R_ColoredSegsIJLevel (int i, int j, int level)
{
    scalelight_EEC06B[i][j] = colormaps_EEC06B + level*256;
    scalelight_D97C45[i][j] = colormaps_EEC06B + level*256;
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
}

// =============================================================================
//
//                            COLORING LOOKUP TABLES
//
// =============================================================================

lighttable_t **R_ColoredVisplanesColorize (int light, int color)
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
        }
    }

    return zlight[light];
}

lighttable_t **R_ColoredSegsColorize (int lightnum, int color)
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
        }
    }

    return scalelight[BETWEEN(0, l, lightnum)]; 
}

lighttable_t *R_ColoredSprColorize (int color)
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
    {    1,     15,    0x55B828 },
    {    1,     17,    0x55B828 },
    {    1,     19,    0x55B828 },
    {    1,     20,    0x55B828 },
    {    1,     21,    0x55B828 },
    {    1,     22,    0x50ADAC },
    {    1,     33,    0x949DB9 },
    {    1,     34,    0x949DB9 },
    {    1,     35,    0x949DB9 },
    {    1,     36,    0x949DB9 },
    {    1,     37,    0x949DB9 },
    {    1,     38,    0x949DB9 },
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
        default:  sectorcolor = sectorcolor_dummy;  break;
    }
}

// =============================================================================
//
//                           COLORED COLORMAP LUMPS
//
// =============================================================================

const byte C_EEC06B[] = {
    0,0,0,29,17,5,21,11,3,70,56,31,238,192,107,25,
    20,11,18,14,8,10,8,5,7,5,3,44,41,13,33,32,
    6,21,23,3,14,17,0,74,44,18,66,38,15,59,32,11,
    238,138,77,231,129,72,227,123,68,219,114,63,216,108,60,208,
    102,57,204,93,52,197,87,48,189,81,45,186,75,42,178,69,
    38,175,66,37,167,59,33,163,53,30,156,47,26,152,44,25,
    145,38,21,141,35,20,133,32,18,130,26,15,122,23,13,119,
    20,11,111,17,10,107,14,8,100,11,6,96,8,5,89,5,
    3,85,5,3,77,5,3,74,0,0,66,0,0,63,0,0,
    238,177,94,238,171,89,238,165,84,238,159,78,238,156,75,238,
    150,70,238,144,65,238,141,62,238,135,55,231,129,52,223,123,
    48,216,117,45,208,111,42,201,105,38,193,99,35,189,96,33,
    178,93,31,167,87,30,160,84,28,152,81,26,145,75,25,133,
    72,23,126,66,21,119,62,20,111,59,18,100,53,16,89,50,
    15,77,47,13,70,41,11,59,35,10,48,32,8,40,26,6,
    223,180,100,216,174,97,208,168,94,204,165,92,197,159,89,189,
    153,85,186,150,84,178,144,80,171,138,77,167,135,75,160,129,
    72,156,126,70,148,120,67,141,114,63,137,111,62,130,105,58,
    122,99,55,119,96,53,111,90,50,104,84,47,100,81,45,92,
    75,42,85,69,38,81,66,37,74,59,33,66,53,30,63,50,
    28,55,44,25,51,41,23,44,35,20,36,29,16,33,26,15,
    111,192,47,104,180,43,96,168,40,89,156,37,85,144,33,77,
    132,30,70,120,26,63,111,23,59,99,20,51,87,18,44,75,
    15,36,62,11,29,50,10,21,38,6,18,26,5,10,17,3,
    178,126,60,171,120,57,163,114,53,156,108,50,148,102,47,145,
    96,45,137,93,42,130,87,38,122,81,37,115,75,33,111,72,
    31,104,66,28,96,62,26,89,56,23,81,50,21,77,47,20,
    148,99,42,133,90,35,122,81,31,111,72,26,96,62,21,85,
    53,18,74,44,15,63,38,11,115,96,42,104,87,37,96,81,
    33,85,75,30,77,66,25,66,59,21,59,53,18,51,47,16,
    238,192,48,219,165,37,201,141,28,182,117,20,163,93,13,145,
    69,8,126,50,3,107,32,0,238,192,107,238,165,92,238,141,
    78,238,117,65,238,93,52,238,72,40,238,47,26,238,23,13,
    238,0,0,223,0,0,212,0,0,201,0,0,189,0,0,178,
    0,0,167,0,0,156,0,0,145,0,0,130,0,0,119,0,
    0,107,0,0,96,0,0,85,0,0,74,0,0,63,0,0,
    216,174,107,186,150,107,160,129,107,133,108,107,107,87,107,77,
    62,107,51,41,107,25,20,107,0,0,107,0,0,95,0,0,
    85,0,0,75,0,0,65,0,0,55,0,0,45,0,0,35,
    238,192,107,238,177,92,238,162,78,238,150,65,238,135,52,238,
    123,38,238,108,25,238,96,11,227,87,10,219,84,6,208,78,
    6,201,72,5,189,66,3,182,59,0,171,53,0,163,50,0,
    238,192,107,238,192,90,238,192,75,238,192,60,238,192,45,238,
    192,30,238,192,15,238,192,0,156,47,0,148,41,0,137,35,
    0,126,26,0,74,44,16,63,35,11,51,26,8,44,20,5,
    0,0,35,0,0,30,0,0,25,0,0,20,0,0,15,0,
    0,10,0,0,5,0,0,0,238,120,28,238,174,31,238,93,
    107,238,0,107,193,0,87,148,0,65,104,0,45,156,81,45
};

const byte C_D97C45[] = {
    0,0,0,26,11,3,20,7,2,64,36,20,217,124,69,23,
    13,7,16,9,5,9,5,3,6,3,2,40,27,8,30,21,
    4,20,15,2,13,11,0,67,29,12,60,25,9,54,21,7,
    217,89,50,210,83,46,207,79,44,200,73,41,197,70,39,190,
    66,37,186,60,33,180,56,31,173,52,29,169,48,27,163,44,
    25,159,42,24,152,38,21,149,35,19,142,31,17,139,29,16,
    132,25,14,128,23,13,122,21,12,118,17,9,111,15,8,108,
    13,7,101,11,6,98,9,5,91,7,4,88,5,3,81,3,
    2,77,3,2,71,3,2,67,0,0,60,0,0,57,0,0,
    217,114,60,217,110,57,217,106,54,217,103,51,217,101,48,217,
    97,45,217,93,42,217,91,40,217,87,35,210,83,33,203,79,
    31,197,75,29,190,71,27,183,68,25,176,64,22,173,62,21,
    163,60,20,152,56,19,146,54,18,139,52,17,132,48,16,122,
    46,15,115,42,14,108,40,13,101,38,12,91,35,11,81,33,
    9,71,31,8,64,27,7,54,23,6,43,21,5,37,17,4,
    203,116,65,197,112,63,190,108,60,186,106,59,180,103,57,173,
    99,55,169,97,54,163,93,52,156,89,50,152,87,48,146,83,
    46,142,81,45,135,77,43,128,73,41,125,71,40,118,68,38,
    111,64,35,108,62,34,101,58,32,94,54,30,91,52,29,84,
    48,27,77,44,25,74,42,24,67,38,21,60,35,19,57,33,
    18,50,29,16,47,27,15,40,23,13,33,19,11,30,17,9,
    101,124,30,94,116,28,88,108,26,81,101,24,77,93,21,71,
    85,19,64,77,17,57,71,15,54,64,13,47,56,12,40,48,
    9,33,40,7,26,33,6,20,25,4,16,17,3,9,11,2,
    163,81,39,156,77,37,149,73,34,142,70,32,135,66,30,132,
    62,29,125,60,27,118,56,25,111,52,24,105,48,21,101,46,
    20,94,42,18,88,40,17,81,36,15,74,33,14,71,31,13,
    135,64,27,122,58,22,111,52,20,101,46,17,88,40,14,77,
    35,12,67,29,9,57,25,7,105,62,27,94,56,24,88,52,
    21,77,48,19,71,42,16,60,38,14,54,35,12,47,31,11,
    217,124,31,200,106,24,183,91,18,166,75,13,149,60,8,132,
    44,5,115,33,2,98,21,0,217,124,69,217,106,59,217,91,
    51,217,75,42,217,60,33,217,46,26,217,31,17,217,15,8,
    217,0,0,203,0,0,193,0,0,183,0,0,173,0,0,163,
    0,0,152,0,0,142,0,0,132,0,0,118,0,0,108,0,
    0,98,0,0,88,0,0,77,0,0,67,0,0,57,0,0,
    197,112,69,169,97,69,146,83,69,122,70,69,98,56,69,71,
    40,69,47,27,69,23,13,69,0,0,69,0,0,61,0,0,
    55,0,0,48,0,0,42,0,0,35,0,0,29,0,0,22,
    217,124,69,217,114,59,217,105,51,217,97,42,217,87,33,217,
    79,25,217,70,16,217,62,7,207,56,6,200,54,4,190,50,
    4,183,46,3,173,42,2,166,38,0,156,35,0,149,33,0,
    217,124,69,217,124,58,217,124,48,217,124,39,217,124,29,217,
    124,19,217,124,9,217,124,0,142,31,0,135,27,0,125,23,
    0,115,17,0,67,29,11,57,23,7,47,17,5,40,13,3,
    0,0,22,0,0,19,0,0,16,0,0,13,0,0,9,0,
    0,6,0,0,3,0,0,0,217,77,18,217,112,20,217,60,
    69,217,0,69,176,0,56,135,0,42,94,0,29,142,52,29
};

const byte C_FF7F7F[] = {
    0,0,0,31,11,5,23,7,3,75,37,37,255,127,127,27,
    13,13,19,9,9,11,5,5,7,3,3,47,27,15,35,21,
    7,23,15,3,15,11,0,79,29,21,71,25,17,63,21,13,
    255,91,91,247,85,85,243,81,81,235,75,75,231,71,71,223,
    67,67,219,61,61,211,57,57,203,53,53,199,49,49,191,45,
    45,187,43,43,179,39,39,175,35,35,167,31,31,163,29,29,
    155,25,25,151,23,23,143,21,21,139,17,17,131,15,15,127,
    13,13,119,11,11,115,9,9,107,7,7,103,5,5,95,3,
    3,91,3,3,83,3,3,79,0,0,71,0,0,67,0,0,
    255,117,111,255,113,105,255,109,99,255,105,93,255,103,89,255,
    99,83,255,95,77,255,93,73,255,89,65,247,85,61,239,81,
    57,231,77,53,223,73,49,215,69,45,207,65,41,203,63,39,
    191,61,37,179,57,35,171,55,33,163,53,31,155,49,29,143,
    47,27,135,43,25,127,41,23,119,39,21,107,35,19,95,33,
    17,83,31,15,75,27,13,63,23,11,51,21,9,43,17,7,
    239,119,119,231,115,115,223,111,111,219,109,109,211,105,105,203,
    101,101,199,99,99,191,95,95,183,91,91,179,89,89,171,85,
    85,167,83,83,159,79,79,151,75,75,147,73,73,139,69,69,
    131,65,65,127,63,63,119,59,59,111,55,55,107,53,53,99,
    49,49,91,45,45,87,43,43,79,39,39,71,35,35,67,33,
    33,59,29,29,55,27,27,47,23,23,39,19,19,35,17,17,
    119,127,55,111,119,51,103,111,47,95,103,43,91,95,39,83,
    87,35,75,79,31,67,73,27,63,65,23,55,57,21,47,49,
    17,39,41,13,31,33,11,23,25,7,19,17,5,11,11,3,
    191,83,71,183,79,67,175,75,63,167,71,59,159,67,55,155,
    63,53,147,61,49,139,57,45,131,53,43,123,49,39,119,47,
    37,111,43,33,103,41,31,95,37,27,87,33,25,83,31,23,
    159,65,49,143,59,41,131,53,37,119,47,31,103,41,25,91,
    35,21,79,29,17,67,25,13,123,63,49,111,57,43,103,53,
    39,91,49,35,83,43,29,71,39,25,63,35,21,55,31,19,
    255,127,57,235,109,43,215,93,33,195,77,23,175,61,15,155,
    45,9,135,33,3,115,21,0,255,127,127,255,109,109,255,93,
    93,255,77,77,255,61,61,255,47,47,255,31,31,255,15,15,
    255,0,0,239,0,0,227,0,0,215,0,0,203,0,0,191,
    0,0,179,0,0,167,0,0,155,0,0,139,0,0,127,0,
    0,115,0,0,103,0,0,91,0,0,79,0,0,67,0,0,
    231,115,127,199,99,127,171,85,127,143,71,127,115,57,127,83,
    41,127,55,27,127,27,13,127,0,0,127,0,0,113,0,0,
    101,0,0,89,0,0,77,0,0,65,0,0,53,0,0,41,
    255,127,127,255,117,109,255,107,93,255,99,77,255,89,61,255,
    81,45,255,71,29,255,63,13,243,57,11,235,55,7,223,51,
    7,215,47,5,203,43,3,195,39,0,183,35,0,175,33,0,
    255,127,127,255,127,107,255,127,89,255,127,71,255,127,53,255,
    127,35,255,127,17,255,127,0,167,31,0,159,27,0,147,23,
    0,135,17,0,79,29,19,67,23,13,55,17,9,47,13,5,
    0,0,41,0,0,35,0,0,29,0,0,23,0,0,17,0,
    0,11,0,0,5,0,0,0,255,79,33,255,115,37,255,61,
    127,255,0,127,207,0,103,159,0,77,111,0,53,167,53,53
};

const byte C_55B828[] = {
    0,0,0,10,17,2,8,11,1,25,54,12,85,184,40,9,
    19,4,6,14,3,4,8,2,2,5,1,16,40,5,12,31,
    2,8,22,1,5,17,0,26,43,7,24,37,5,21,31,4,
    85,132,29,82,123,27,81,118,26,78,109,24,77,103,22,74,
    97,21,73,89,19,70,83,18,68,77,17,66,71,16,64,66,
    14,62,63,14,60,57,12,58,51,11,56,45,10,54,43,9,
    52,37,8,50,34,7,48,31,7,46,25,5,44,22,5,42,
    19,4,40,17,4,38,14,3,36,11,2,34,8,2,32,5,
    1,30,5,1,28,5,1,26,0,0,24,0,0,22,0,0,
    85,170,35,85,164,33,85,158,31,85,152,29,85,149,28,85,
    144,26,85,138,24,85,135,23,85,129,21,82,123,19,80,118,
    18,77,112,17,74,106,16,72,100,14,69,95,13,68,92,12,
    64,89,12,60,83,11,57,80,11,54,77,10,52,71,9,48,
    69,9,45,63,8,42,60,7,40,57,7,36,51,6,32,48,
    5,28,45,5,25,40,4,21,34,4,17,31,3,14,25,2,
    80,172,37,77,167,36,74,161,35,73,158,34,70,152,33,68,
    146,32,66,144,31,64,138,30,61,132,29,60,129,28,57,123,
    27,56,121,26,53,115,25,50,109,24,49,106,23,46,100,22,
    44,95,21,42,92,20,40,86,19,37,80,17,36,77,17,33,
    71,16,30,66,14,29,63,14,26,57,12,24,51,11,22,48,
    11,20,43,9,18,40,9,16,34,7,13,28,6,12,25,5,
    40,184,17,37,172,16,34,161,15,32,149,14,30,138,12,28,
    126,11,25,115,10,22,106,9,21,95,7,18,83,7,16,71,
    5,13,60,4,10,48,4,8,37,2,6,25,2,4,17,1,
    64,121,22,61,115,21,58,109,20,56,103,19,53,97,17,52,
    92,17,49,89,16,46,83,14,44,77,14,41,71,12,40,69,
    12,37,63,11,34,60,10,32,54,9,29,48,8,28,45,7,
    53,95,16,48,86,13,44,77,12,40,69,10,34,60,8,30,
    51,7,26,43,5,22,37,4,41,92,16,37,83,14,34,77,
    12,30,71,11,28,63,9,24,57,8,21,51,7,18,45,6,
    85,184,18,78,158,14,72,135,11,65,112,7,58,89,5,52,
    66,3,45,48,1,38,31,0,85,184,40,85,158,34,85,135,
    29,85,112,24,85,89,19,85,69,15,85,45,10,85,22,5,
    85,0,0,80,0,0,76,0,0,72,0,0,68,0,0,64,
    0,0,60,0,0,56,0,0,52,0,0,46,0,0,42,0,
    0,38,0,0,34,0,0,30,0,0,26,0,0,22,0,0,
    77,167,40,66,144,40,57,123,40,48,103,40,38,83,40,28,
    60,40,18,40,40,9,19,40,0,0,40,0,0,36,0,0,
    32,0,0,28,0,0,24,0,0,21,0,0,17,0,0,13,
    85,184,40,85,170,34,85,155,29,85,144,24,85,129,19,85,
    118,14,85,103,9,85,92,4,81,83,4,78,80,2,74,74,
    2,72,69,2,68,63,1,65,57,0,61,51,0,58,48,0,
    85,184,40,85,184,34,85,184,28,85,184,22,85,184,17,85,
    184,11,85,184,5,85,184,0,56,45,0,53,40,0,49,34,
    0,45,25,0,26,43,6,22,34,4,18,25,3,16,19,2,
    0,0,13,0,0,11,0,0,9,0,0,7,0,0,5,0,
    0,4,0,0,2,0,0,0,85,115,11,85,167,12,85,89,
    40,85,0,40,69,0,32,53,0,24,37,0,17,56,77,17
};

const byte C_BBE357[] = {
    0,0,0,23,20,4,17,13,2,55,67,26,187,227,87,20,
    24,9,14,17,6,8,10,4,5,6,2,34,49,11,26,38,
    5,17,28,2,11,20,0,58,53,15,52,45,12,46,38,9,
    187,163,62,181,152,58,178,145,56,172,134,52,169,127,49,164,
    120,46,161,109,42,155,102,39,149,95,37,146,88,34,140,81,
    31,137,77,30,131,70,27,128,63,24,122,56,21,120,53,20,
    114,45,17,111,42,16,105,38,15,102,31,12,96,28,11,93,
    24,9,87,20,8,84,17,6,78,13,5,76,10,4,70,6,
    2,67,6,2,61,6,2,58,0,0,52,0,0,49,0,0,
    187,209,76,187,202,72,187,195,68,187,188,64,187,184,61,187,
    177,57,187,170,53,187,166,50,187,159,45,181,152,42,175,145,
    39,169,138,37,164,131,34,158,124,31,152,117,28,149,113,27,
    140,109,26,131,102,24,125,99,23,120,95,21,114,88,20,105,
    85,19,99,77,17,93,74,16,87,70,15,78,63,13,70,60,
    12,61,56,11,55,49,9,46,42,8,37,38,6,32,31,5,
    175,213,82,169,206,79,164,199,76,161,195,75,155,188,72,149,
    181,69,146,177,68,140,170,65,134,163,62,131,159,61,125,152,
    58,122,149,57,117,142,54,111,134,52,108,131,50,102,124,47,
    96,117,45,93,113,43,87,106,41,81,99,38,78,95,37,73,
    88,34,67,81,31,64,77,30,58,70,27,52,63,24,49,60,
    23,43,53,20,40,49,19,34,42,16,29,35,13,26,31,12,
    87,227,38,81,213,35,76,199,32,70,184,30,67,170,27,61,
    156,24,55,142,21,49,131,19,46,117,16,40,102,15,34,88,
    12,29,74,9,23,60,8,17,45,5,14,31,4,8,20,2,
    140,149,49,134,142,46,128,134,43,122,127,41,117,120,38,114,
    113,37,108,109,34,102,102,31,96,95,30,90,88,27,87,85,
    26,81,77,23,76,74,21,70,67,19,64,60,17,61,56,16,
    117,117,34,105,106,28,96,95,26,87,85,21,76,74,17,67,
    63,15,58,53,12,49,45,9,90,113,34,81,102,30,76,95,
    27,67,88,24,61,77,20,52,70,17,46,63,15,40,56,13,
    187,227,39,172,195,30,158,166,23,143,138,16,128,109,11,114,
    81,6,99,60,2,84,38,0,187,227,87,187,195,75,187,166,
    64,187,138,53,187,109,42,187,85,32,187,56,21,187,28,11,
    187,0,0,175,0,0,166,0,0,158,0,0,149,0,0,140,
    0,0,131,0,0,122,0,0,114,0,0,102,0,0,93,0,
    0,84,0,0,76,0,0,67,0,0,58,0,0,49,0,0,
    169,206,87,146,177,87,125,152,87,105,127,87,84,102,87,61,
    74,87,40,49,87,20,24,87,0,0,87,0,0,77,0,0,
    69,0,0,61,0,0,53,0,0,45,0,0,37,0,0,28,
    187,227,87,187,209,75,187,191,64,187,177,53,187,159,42,187,
    145,31,187,127,20,187,113,9,178,102,8,172,99,5,164,92,
    5,158,85,4,149,77,2,143,70,0,134,63,0,128,60,0,
    187,227,87,187,227,73,187,227,61,187,227,49,187,227,37,187,
    227,24,187,227,12,187,227,0,122,56,0,117,49,0,108,42,
    0,99,31,0,58,53,13,49,42,9,40,31,6,34,24,4,
    0,0,28,0,0,24,0,0,20,0,0,16,0,0,12,0,
    0,8,0,0,4,0,0,0,187,142,23,187,206,26,187,109,
    87,187,0,87,152,0,71,117,0,53,81,0,37,122,95,37
};

const byte C_949DB9[] = {
    0,0,0,18,14,8,13,9,5,44,46,54,148,157,185,16,
    17,20,11,12,14,6,7,8,4,4,5,27,34,22,20,26,
    11,13,19,5,9,14,0,46,36,31,41,31,25,37,26,20,
    148,113,133,143,105,124,141,100,118,136,93,110,134,88,104,129,
    83,98,127,76,89,122,71,83,118,66,78,115,61,72,111,56,
    66,109,54,63,104,49,57,102,44,52,97,39,46,95,36,43,
    90,31,37,88,29,34,83,26,31,81,22,25,76,19,22,74,
    17,20,69,14,17,67,12,14,62,9,11,60,7,8,55,4,
    5,53,4,5,48,4,5,46,0,0,41,0,0,39,0,0,
    148,145,162,148,140,153,148,135,144,148,130,136,148,127,130,148,
    123,121,148,118,112,148,115,107,148,110,95,143,105,89,139,100,
    83,134,95,78,129,91,72,125,86,66,120,81,60,118,78,57,
    111,76,54,104,71,52,99,68,49,95,66,46,90,61,43,83,
    58,40,78,54,37,74,51,34,69,49,31,62,44,28,55,41,
    25,48,39,22,44,34,20,37,29,17,30,26,14,25,22,11,
    139,147,173,134,142,168,129,137,162,127,135,159,122,130,153,118,
    125,147,115,123,144,111,118,139,106,113,133,104,110,130,99,105,
    124,97,103,121,92,98,115,88,93,110,85,91,107,81,86,101,
    76,81,95,74,78,92,69,73,86,64,68,81,62,66,78,57,
    61,72,53,56,66,50,54,63,46,49,57,41,44,52,39,41,
    49,34,36,43,32,34,40,27,29,34,23,24,28,20,22,25,
    69,157,81,64,147,75,60,137,69,55,127,63,53,118,57,48,
    108,52,44,98,46,39,91,40,37,81,34,32,71,31,27,61,
    25,23,51,20,18,41,17,13,31,11,11,22,8,6,14,5,
    111,103,104,106,98,98,102,93,92,97,88,86,92,83,81,90,
    78,78,85,76,72,81,71,66,76,66,63,71,61,57,69,58,
    54,64,54,49,60,51,46,55,46,40,50,41,37,48,39,34,
    92,81,72,83,73,60,76,66,54,69,58,46,60,51,37,53,
    44,31,46,36,25,39,31,20,71,78,72,64,71,63,60,66,
    57,53,61,52,48,54,43,41,49,37,37,44,31,32,39,28,
    148,157,83,136,135,63,125,115,49,113,95,34,102,76,22,90,
    56,14,78,41,5,67,26,0,148,157,185,148,135,159,148,115,
    136,148,95,112,148,76,89,148,58,69,148,39,46,148,19,22,
    148,0,0,139,0,0,132,0,0,125,0,0,118,0,0,111,
    0,0,104,0,0,97,0,0,90,0,0,81,0,0,74,0,
    0,67,0,0,60,0,0,53,0,0,46,0,0,39,0,0,
    134,142,185,115,123,185,99,105,185,83,88,185,67,71,185,48,
    51,185,32,34,185,16,17,185,0,0,185,0,0,165,0,0,
    147,0,0,130,0,0,112,0,0,95,0,0,78,0,0,60,
    148,157,185,148,145,159,148,132,136,148,123,112,148,110,89,148,
    100,66,148,88,43,148,78,20,141,71,17,136,68,11,129,63,
    11,125,58,8,118,54,5,113,49,0,106,44,0,102,41,0,
    148,157,185,148,157,156,148,157,130,148,157,104,148,157,78,148,
    157,52,148,157,25,148,157,0,97,39,0,92,34,0,85,29,
    0,78,22,0,46,36,28,39,29,20,32,22,14,27,17,8,
    0,0,60,0,0,52,0,0,43,0,0,34,0,0,25,0,
    0,17,0,0,8,0,0,0,148,98,49,148,142,54,148,76,
    185,148,0,185,120,0,150,92,0,112,64,0,78,97,66,78
};

const byte C_2A2F6B[] = {
    0,0,0,5,4,5,4,3,3,12,14,31,42,47,107,4,
    5,11,3,4,8,2,2,5,1,1,3,8,10,13,6,8,
    6,4,6,3,2,4,0,13,11,18,12,9,15,10,8,11,
    42,34,77,41,32,72,40,30,68,39,28,63,38,26,60,37,
    25,57,36,23,52,35,21,48,33,20,45,33,18,42,31,17,
    38,31,16,37,29,15,33,29,13,30,28,12,26,27,11,25,
    26,9,21,25,9,20,24,8,18,23,6,15,22,6,13,21,
    5,11,20,4,10,19,4,8,18,3,6,17,2,5,16,1,
    3,15,1,3,14,1,3,13,0,0,12,0,0,11,0,0,
    42,43,94,42,42,89,42,40,84,42,39,78,42,38,75,42,
    37,70,42,35,65,42,34,62,42,33,55,41,32,52,39,30,
    48,38,29,45,37,27,42,35,26,38,34,24,35,33,23,33,
    31,23,31,29,21,30,28,20,28,27,20,26,26,18,25,24,
    18,23,22,16,21,21,15,20,20,15,18,18,13,16,16,12,
    15,14,12,13,12,10,11,10,9,10,8,8,8,7,6,6,
    39,44,100,38,43,97,37,41,94,36,40,92,35,39,89,33,
    37,85,33,37,84,31,35,80,30,34,77,29,33,75,28,32,
    72,28,31,70,26,29,67,25,28,63,24,27,62,23,26,58,
    22,24,55,21,23,53,20,22,50,18,20,47,18,20,45,16,
    18,42,15,17,38,14,16,37,13,15,33,12,13,30,11,12,
    28,10,11,25,9,10,23,8,9,20,6,7,16,6,6,15,
    20,47,47,18,44,43,17,41,40,16,38,37,15,35,33,14,
    32,30,12,29,26,11,27,23,10,24,20,9,21,18,8,18,
    15,6,15,11,5,12,10,4,9,6,3,6,5,2,4,3,
    31,31,60,30,29,57,29,28,53,28,26,50,26,25,47,26,
    23,45,24,23,42,23,21,38,22,20,37,20,18,33,20,18,
    31,18,16,28,17,15,26,16,14,23,14,12,21,14,12,20,
    26,24,42,24,22,35,22,20,31,20,18,26,17,15,21,15,
    13,18,13,11,15,11,9,11,20,23,42,18,21,37,17,20,
    33,15,18,30,14,16,25,12,15,21,10,13,18,9,12,16,
    42,47,48,39,40,37,35,34,28,32,29,20,29,23,13,26,
    17,8,22,12,3,19,8,0,42,47,107,42,40,92,42,34,
    78,42,29,65,42,23,52,42,18,40,42,12,26,42,6,13,
    42,0,0,39,0,0,37,0,0,35,0,0,33,0,0,31,
    0,0,29,0,0,28,0,0,26,0,0,23,0,0,21,0,
    0,19,0,0,17,0,0,15,0,0,13,0,0,11,0,0,
    38,43,107,33,37,107,28,32,107,24,26,107,19,21,107,14,
    15,107,9,10,107,4,5,107,0,0,107,0,0,95,0,0,
    85,0,0,75,0,0,65,0,0,55,0,0,45,0,0,35,
    42,47,107,42,43,92,42,40,78,42,37,65,42,33,52,42,
    30,38,42,26,25,42,23,11,40,21,10,39,20,6,37,19,
    6,35,18,5,33,16,3,32,15,0,30,13,0,29,12,0,
    42,47,107,42,47,90,42,47,75,42,47,60,42,47,45,42,
    47,30,42,47,15,42,47,0,28,12,0,26,10,0,24,9,
    0,22,6,0,13,11,16,11,9,11,9,6,8,8,5,5,
    0,0,35,0,0,30,0,0,25,0,0,20,0,0,15,0,
    0,10,0,0,5,0,0,0,42,29,28,42,43,31,42,23,
    107,42,0,107,34,0,87,26,0,65,18,0,45,28,20,45
};

const byte C_50ADAC[] = {
    0,0,0,10,16,7,7,10,5,24,51,51,80,173,172,8,
    18,18,6,13,13,3,7,7,2,5,5,15,37,21,11,29,
    10,7,21,5,5,16,0,25,40,29,22,35,24,20,29,18,
    80,124,123,77,116,115,76,111,110,74,102,102,72,97,96,70,
    92,91,69,83,83,66,78,78,64,73,72,62,67,67,60,62,
    61,59,59,59,56,54,53,55,48,48,52,43,42,51,40,40,
    49,35,34,47,32,32,45,29,29,44,24,24,41,21,21,40,
    18,18,37,16,16,36,13,13,34,10,10,32,7,7,30,5,
    5,29,5,5,26,5,5,25,0,0,22,0,0,21,0,0,
    80,159,150,80,154,142,80,149,134,80,143,126,80,140,121,80,
    135,113,80,130,105,80,127,99,80,121,88,77,116,83,75,111,
    78,72,105,72,70,100,67,67,94,61,65,89,56,64,86,53,
    60,83,51,56,78,48,54,75,45,51,73,42,49,67,40,45,
    64,37,42,59,34,40,56,32,37,54,29,34,48,26,30,45,
    24,26,43,21,24,37,18,20,32,16,16,29,13,13,24,10,
    75,162,161,72,157,156,70,151,150,69,149,148,66,143,142,64,
    138,137,62,135,134,60,130,129,57,124,123,56,121,121,54,116,
    115,52,113,113,50,108,107,47,102,102,46,100,99,44,94,94,
    41,89,88,40,86,86,37,81,80,35,75,75,34,73,72,31,
    67,67,29,62,61,27,59,59,25,54,53,22,48,48,21,45,
    45,19,40,40,17,37,37,15,32,32,12,26,26,11,24,24,
    37,173,75,35,162,69,32,151,64,30,140,59,29,130,53,26,
    119,48,24,108,42,21,100,37,20,89,32,17,78,29,15,67,
    24,12,56,18,10,45,16,7,35,10,6,24,7,3,16,5,
    60,113,96,57,108,91,55,102,86,52,97,80,50,92,75,49,
    86,72,46,83,67,44,78,61,41,73,59,39,67,53,37,64,
    51,35,59,45,32,56,42,30,51,37,27,45,34,26,43,32,
    50,89,67,45,81,56,41,73,51,37,64,42,32,56,34,29,
    48,29,25,40,24,21,35,18,39,86,67,35,78,59,32,73,
    53,29,67,48,26,59,40,22,54,34,20,48,29,17,43,26,
    80,173,78,74,149,59,67,127,45,61,105,32,55,83,21,49,
    62,13,42,45,5,36,29,0,80,173,172,80,149,148,80,127,
    126,80,105,105,80,83,83,80,64,64,80,43,42,80,21,21,
    80,0,0,75,0,0,71,0,0,67,0,0,64,0,0,60,
    0,0,56,0,0,52,0,0,49,0,0,44,0,0,40,0,
    0,36,0,0,32,0,0,29,0,0,25,0,0,21,0,0,
    72,157,172,62,135,172,54,116,172,45,97,172,36,78,172,26,
    56,172,17,37,172,8,18,172,0,0,172,0,0,153,0,0,
    137,0,0,121,0,0,105,0,0,88,0,0,72,0,0,56,
    80,173,172,80,159,148,80,146,126,80,135,105,80,121,83,80,
    111,61,80,97,40,80,86,18,76,78,16,74,75,10,70,70,
    10,67,64,7,64,59,5,61,54,0,57,48,0,55,45,0,
    80,173,172,80,173,145,80,173,121,80,173,96,80,173,72,80,
    173,48,80,173,24,80,173,0,52,43,0,50,37,0,46,32,
    0,42,24,0,25,40,26,21,32,18,17,24,13,15,18,7,
    0,0,56,0,0,48,0,0,40,0,0,32,0,0,24,0,
    0,16,0,0,7,0,0,0,80,108,45,80,157,51,80,83,
    172,80,0,172,65,0,140,50,0,105,35,0,72,52,73,72
};

const byte C_CCE4A5[] = {
    0,0,0,25,21,7,18,13,5,60,67,49,204,228,165,22,
    24,17,15,17,12,9,10,7,6,6,5,38,49,20,28,38,
    10,18,28,5,12,21,0,63,53,28,57,46,23,50,38,17,
    204,164,118,198,153,111,194,146,105,188,135,98,185,128,93,178,
    121,87,175,110,80,169,103,74,162,96,69,159,89,64,153,81,
    59,150,78,56,143,71,51,140,63,46,134,56,41,130,53,38,
    124,46,33,121,42,30,114,38,28,111,31,23,105,28,20,102,
    24,17,95,21,15,92,17,12,86,13,10,82,10,7,76,6,
    5,73,6,5,66,6,5,63,0,0,57,0,0,54,0,0,
    204,210,144,204,203,137,204,196,129,204,189,121,204,185,116,204,
    178,108,204,171,100,204,167,95,204,160,85,198,153,80,191,146,
    74,185,139,69,178,131,64,172,124,59,166,117,54,162,114,51,
    153,110,49,143,103,46,137,99,43,130,96,41,124,89,38,114,
    85,36,108,78,33,102,74,30,95,71,28,86,63,25,76,60,
    23,66,56,20,60,49,17,50,42,15,41,38,12,34,31,10,
    191,214,155,185,207,149,178,199,144,175,196,142,169,189,137,162,
    182,131,159,178,129,153,171,124,146,164,118,143,160,116,137,153,
    111,134,149,108,127,142,103,121,135,98,118,131,95,111,124,90,
    105,117,85,102,114,82,95,106,77,89,99,72,86,96,69,79,
    89,64,73,81,59,70,78,56,63,71,51,57,63,46,54,60,
    43,47,53,38,44,49,36,38,42,30,31,35,25,28,31,23,
    95,228,72,89,214,67,82,199,61,76,185,56,73,171,51,66,
    156,46,60,142,41,54,131,36,50,117,30,44,103,28,38,89,
    23,31,74,17,25,60,15,18,46,10,15,31,7,9,21,5,
    153,149,93,146,142,87,140,135,82,134,128,77,127,121,72,124,
    114,69,118,110,64,111,103,59,105,96,56,98,89,51,95,85,
    49,89,78,43,82,74,41,76,67,36,70,60,33,66,56,30,
    127,117,64,114,106,54,105,96,49,95,85,41,82,74,33,73,
    63,28,63,53,23,54,46,17,98,114,64,89,103,56,82,96,
    51,73,89,46,66,78,38,57,71,33,50,63,28,44,56,25,
    204,228,74,188,196,56,172,167,43,156,139,30,140,110,20,124,
    81,12,108,60,5,92,38,0,204,228,165,204,196,142,204,167,
    121,204,139,100,204,110,80,204,85,61,204,56,41,204,28,20,
    204,0,0,191,0,0,182,0,0,172,0,0,162,0,0,153,
    0,0,143,0,0,134,0,0,124,0,0,111,0,0,102,0,
    0,92,0,0,82,0,0,73,0,0,63,0,0,54,0,0,
    185,207,165,159,178,165,137,153,165,114,128,165,92,103,165,66,
    74,165,44,49,165,22,24,165,0,0,165,0,0,147,0,0,
    131,0,0,116,0,0,100,0,0,85,0,0,69,0,0,54,
    204,228,165,204,210,142,204,192,121,204,178,100,204,160,80,204,
    146,59,204,128,38,204,114,17,194,103,15,188,99,10,178,92,
    10,172,85,7,162,78,5,156,71,0,146,63,0,140,60,0,
    204,228,165,204,228,139,204,228,116,204,228,93,204,228,69,204,
    228,46,204,228,23,204,228,0,134,56,0,127,49,0,118,42,
    0,108,31,0,63,53,25,54,42,17,44,31,12,38,24,7,
    0,0,54,0,0,46,0,0,38,0,0,30,0,0,23,0,
    0,15,0,0,7,0,0,0,204,142,43,204,207,49,204,110,
    165,204,0,165,166,0,134,127,0,100,89,0,69,134,96,69
};

const byte C_CCEA5F[] = {
    0,0,0,25,21,4,18,14,3,60,69,28,204,234,95,22,
    25,10,15,17,7,9,10,4,6,6,3,38,50,12,28,39,
    6,18,28,3,12,21,0,63,54,16,57,47,13,50,39,10,
    204,168,68,198,157,64,194,150,61,188,139,56,185,131,53,178,
    124,50,175,113,46,169,106,43,162,98,40,159,91,37,153,84,
    34,150,80,32,143,72,29,140,65,26,134,58,23,130,54,22,
    124,47,19,121,43,18,114,39,16,111,32,13,105,28,12,102,
    25,10,95,21,9,92,17,7,86,14,6,82,10,4,76,6,
    3,73,6,3,66,6,3,63,0,0,57,0,0,54,0,0,
    204,216,83,204,208,79,204,201,74,204,194,70,204,190,67,204,
    183,62,204,175,58,204,172,55,204,164,49,198,157,46,191,150,
    43,185,142,40,178,135,37,172,128,34,166,120,31,162,117,29,
    153,113,28,143,106,26,137,102,25,130,98,23,124,91,22,114,
    87,20,108,80,19,102,76,18,95,72,16,86,65,15,76,61,
    13,66,58,12,60,50,10,50,43,9,41,39,7,34,32,6,
    191,219,89,185,212,86,178,205,83,175,201,82,169,194,79,162,
    186,76,159,183,74,153,175,71,146,168,68,143,164,67,137,157,
    64,134,153,62,127,146,59,121,139,56,118,135,55,111,128,52,
    105,120,49,102,117,47,95,109,44,89,102,41,86,98,40,79,
    91,37,73,84,34,70,80,32,63,72,29,57,65,26,54,61,
    25,47,54,22,44,50,20,38,43,18,31,36,15,28,32,13,
    95,234,41,89,219,38,82,205,35,76,190,32,73,175,29,66,
    161,26,60,146,23,54,135,20,50,120,18,44,106,16,38,91,
    13,31,76,10,25,61,9,18,47,6,15,32,4,9,21,3,
    153,153,53,146,146,50,140,139,47,134,131,44,127,124,41,124,
    117,40,118,113,37,111,106,34,105,98,32,98,91,29,95,87,
    28,89,80,25,82,76,23,76,69,20,70,61,19,66,58,18,
    127,120,37,114,109,31,105,98,28,95,87,23,82,76,19,73,
    65,16,63,54,13,54,47,10,98,117,37,89,106,32,82,98,
    29,73,91,26,66,80,22,57,72,19,50,65,16,44,58,15,
    204,234,43,188,201,32,172,172,25,156,142,18,140,113,12,124,
    84,7,108,61,3,92,39,0,204,234,95,204,201,82,204,172,
    70,204,142,58,204,113,46,204,87,35,204,58,23,204,28,12,
    204,0,0,191,0,0,182,0,0,172,0,0,162,0,0,153,
    0,0,143,0,0,134,0,0,124,0,0,111,0,0,102,0,
    0,92,0,0,82,0,0,73,0,0,63,0,0,54,0,0,
    185,212,95,159,183,95,137,157,95,114,131,95,92,106,95,66,
    76,95,44,50,95,22,25,95,0,0,95,0,0,85,0,0,
    76,0,0,67,0,0,58,0,0,49,0,0,40,0,0,31,
    204,234,95,204,216,82,204,197,70,204,183,58,204,164,46,204,
    150,34,204,131,22,204,117,10,194,106,9,188,102,6,178,95,
    6,172,87,4,162,80,3,156,72,0,146,65,0,140,61,0,
    204,234,95,204,234,80,204,234,67,204,234,53,204,234,40,204,
    234,26,204,234,13,204,234,0,134,58,0,127,50,0,118,43,
    0,108,32,0,63,54,15,54,43,10,44,32,7,38,25,4,
    0,0,31,0,0,26,0,0,22,0,0,18,0,0,13,0,
    0,9,0,0,4,0,0,0,204,146,25,204,212,28,204,113,
    95,204,0,95,166,0,77,127,0,58,89,0,40,134,98,40
};

const byte C_B30202[] = {
    0,0,0,22,0,0,16,0,0,53,1,1,179,2,2,19,
    0,0,13,0,0,8,0,0,5,0,0,33,0,0,25,0,
    0,16,0,0,11,0,0,55,0,0,50,0,0,44,0,0,
    179,1,1,173,1,1,171,1,1,165,1,1,162,1,1,157,
    1,1,154,1,1,148,1,1,142,1,1,140,1,1,134,1,
    1,131,1,1,126,1,1,123,1,1,117,0,0,114,0,0,
    109,0,0,106,0,0,100,0,0,98,0,0,92,0,0,89,
    0,0,84,0,0,81,0,0,75,0,0,72,0,0,67,0,
    0,64,0,0,58,0,0,55,0,0,50,0,0,47,0,0,
    179,2,2,179,2,2,179,2,2,179,2,1,179,2,1,179,
    2,1,179,1,1,179,1,1,179,1,1,173,1,1,168,1,
    1,162,1,1,157,1,1,151,1,1,145,1,1,142,1,1,
    134,1,1,126,1,1,120,1,1,114,1,0,109,1,0,100,
    1,0,95,1,0,89,1,0,84,1,0,75,1,0,67,1,
    0,58,0,0,53,0,0,44,0,0,36,0,0,30,0,0,
    168,2,2,162,2,2,157,2,2,154,2,2,148,2,2,142,
    2,2,140,2,2,134,1,1,128,1,1,126,1,1,120,1,
    1,117,1,1,112,1,1,106,1,1,103,1,1,98,1,1,
    92,1,1,89,1,1,84,1,1,78,1,1,75,1,1,69,
    1,1,64,1,1,61,1,1,55,1,1,50,1,1,47,1,
    1,41,0,0,39,0,0,33,0,0,27,0,0,25,0,0,
    84,2,1,78,2,1,72,2,1,67,2,1,64,1,1,58,
    1,1,53,1,0,47,1,0,44,1,0,39,1,0,33,1,
    0,27,1,0,22,1,0,16,0,0,13,0,0,8,0,0,
    134,1,1,128,1,1,123,1,1,117,1,1,112,1,1,109,
    1,1,103,1,1,98,1,1,92,1,1,86,1,1,84,1,
    1,78,1,1,72,1,0,67,1,0,61,1,0,58,0,0,
    112,1,1,100,1,1,92,1,1,84,1,0,72,1,0,64,
    1,0,55,0,0,47,0,0,86,1,1,78,1,1,72,1,
    1,64,1,1,58,1,0,50,1,0,44,1,0,39,0,0,
    179,2,1,165,2,1,151,1,1,137,1,0,123,1,0,109,
    1,0,95,1,0,81,0,0,179,2,2,179,2,2,179,1,
    1,179,1,1,179,1,1,179,1,1,179,0,0,179,0,0,
    179,0,0,168,0,0,159,0,0,151,0,0,142,0,0,134,
    0,0,126,0,0,117,0,0,109,0,0,98,0,0,89,0,
    0,81,0,0,72,0,0,64,0,0,55,0,0,47,0,0,
    162,2,2,140,2,2,120,1,2,100,1,2,81,1,2,58,
    1,2,39,0,2,19,0,2,0,0,2,0,0,2,0,0,
    2,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,
    179,2,2,179,2,2,179,2,1,179,2,1,179,1,1,179,
    1,1,179,1,0,179,1,0,171,1,0,165,1,0,157,1,
    0,151,1,0,142,1,0,137,1,0,128,1,0,123,1,0,
    179,2,2,179,2,2,179,2,1,179,2,1,179,2,1,179,
    2,1,179,2,0,179,2,0,117,0,0,112,0,0,103,0,
    0,95,0,0,55,0,0,47,0,0,39,0,0,33,0,0,
    0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,179,1,1,179,2,1,179,1,
    2,179,0,2,145,0,2,112,0,1,78,0,1,117,1,1
};

const byte C_B87A15[] = {
    0,0,0,22,11,1,17,7,1,54,36,6,184,122,21,19,
    13,2,14,9,2,8,5,1,5,3,1,34,26,3,25,21,
    1,17,15,1,11,11,0,57,28,4,51,24,3,45,21,2,
    184,88,15,178,82,14,175,78,13,170,72,12,167,68,12,161,
    65,11,158,59,10,152,55,9,146,51,9,144,47,8,138,44,
    7,135,42,7,129,38,7,126,34,6,121,30,5,118,28,5,
    112,24,4,109,22,4,103,21,4,100,17,3,95,15,3,92,
    13,2,86,11,2,83,9,2,77,7,1,74,5,1,69,3,
    1,66,3,1,60,3,1,57,0,0,51,0,0,48,0,0,
    184,112,18,184,109,17,184,105,16,184,101,15,184,99,15,184,
    95,14,184,91,13,184,89,12,184,86,11,178,82,10,172,78,
    9,167,74,9,161,70,8,155,67,7,149,63,7,146,61,7,
    138,59,6,129,55,6,123,53,6,118,51,5,112,47,5,103,
    45,5,97,42,4,92,40,4,86,38,4,77,34,3,69,32,
    3,60,30,3,54,26,2,45,22,2,37,21,2,31,17,1,
    172,114,20,167,111,19,161,107,18,158,105,18,152,101,17,146,
    97,17,144,95,16,138,91,16,132,88,15,129,86,15,123,82,
    14,121,80,14,115,76,13,109,72,12,106,70,12,100,67,11,
    95,63,11,92,61,10,86,57,10,80,53,9,77,51,9,71,
    47,8,66,44,7,63,42,7,57,38,7,51,34,6,48,32,
    6,43,28,5,40,26,5,34,22,4,28,19,3,25,17,3,
    86,122,9,80,114,8,74,107,8,69,99,7,66,91,7,60,
    84,6,54,76,5,48,70,5,45,63,4,40,55,4,34,47,
    3,28,40,2,22,32,2,17,24,1,14,17,1,8,11,1,
    138,80,12,132,76,11,126,72,10,121,68,10,115,65,9,112,
    61,9,106,59,8,100,55,7,95,51,7,89,47,7,86,45,
    6,80,42,6,74,40,5,69,36,5,63,32,4,60,30,4,
    115,63,8,103,57,7,95,51,6,86,45,5,74,40,4,66,
    34,4,57,28,3,48,24,2,89,61,8,80,55,7,74,51,
    7,66,47,6,60,42,5,51,38,4,45,34,4,40,30,3,
    184,122,9,170,105,7,155,89,6,141,74,4,126,59,3,112,
    44,2,97,32,1,83,21,0,184,122,21,184,105,18,184,89,
    15,184,74,13,184,59,10,184,45,8,184,30,5,184,15,3,
    184,0,0,172,0,0,164,0,0,155,0,0,146,0,0,138,
    0,0,129,0,0,121,0,0,112,0,0,100,0,0,92,0,
    0,83,0,0,74,0,0,66,0,0,57,0,0,48,0,0,
    167,111,21,144,95,21,123,82,21,103,68,21,83,55,21,60,
    40,21,40,26,21,19,13,21,0,0,21,0,0,19,0,0,
    17,0,0,15,0,0,13,0,0,11,0,0,9,0,0,7,
    184,122,21,184,112,18,184,103,15,184,95,13,184,86,10,184,
    78,7,184,68,5,184,61,2,175,55,2,170,53,1,161,49,
    1,155,45,1,146,42,1,141,38,0,132,34,0,126,32,0,
    184,122,21,184,122,18,184,122,15,184,122,12,184,122,9,184,
    122,6,184,122,3,184,122,0,121,30,0,115,26,0,106,22,
    0,97,17,0,57,28,3,48,22,2,40,17,2,34,13,1,
    0,0,7,0,0,6,0,0,5,0,0,4,0,0,3,0,
    0,2,0,0,1,0,0,0,184,76,6,184,111,6,184,59,
    21,184,0,21,149,0,17,115,0,13,80,0,9,121,51,9
};

const byte C_FFD000[] = {
    0,0,0,31,19,0,23,12,0,75,61,0,255,208,0,27,
    22,0,19,15,0,11,9,0,7,6,0,47,45,0,35,35,
    0,23,25,0,15,19,0,79,48,0,71,42,0,63,35,0,
    255,149,0,247,139,0,243,133,0,235,123,0,231,117,0,223,
    110,0,219,100,0,211,94,0,203,87,0,199,81,0,191,74,
    0,187,71,0,179,64,0,175,58,0,167,51,0,163,48,0,
    155,42,0,151,38,0,143,35,0,139,29,0,131,25,0,127,
    22,0,119,19,0,115,15,0,107,12,0,103,9,0,95,6,
    0,91,6,0,83,6,0,79,0,0,71,0,0,67,0,0,
    255,192,0,255,185,0,255,179,0,255,172,0,255,169,0,255,
    162,0,255,156,0,255,153,0,255,146,0,247,139,0,239,133,
    0,231,126,0,223,120,0,215,113,0,207,107,0,203,104,0,
    191,100,0,179,94,0,171,91,0,163,87,0,155,81,0,143,
    77,0,135,71,0,127,68,0,119,64,0,107,58,0,95,55,
    0,83,51,0,75,45,0,63,38,0,51,35,0,43,29,0,
    239,195,0,231,188,0,223,182,0,219,179,0,211,172,0,203,
    166,0,199,162,0,191,156,0,183,149,0,179,146,0,171,139,
    0,167,136,0,159,130,0,151,123,0,147,120,0,139,113,0,
    131,107,0,127,104,0,119,97,0,111,91,0,107,87,0,99,
    81,0,91,74,0,87,71,0,79,64,0,71,58,0,67,55,
    0,59,48,0,55,45,0,47,38,0,39,32,0,35,29,0,
    119,208,0,111,195,0,103,182,0,95,169,0,91,156,0,83,
    143,0,75,130,0,67,120,0,63,107,0,55,94,0,47,81,
    0,39,68,0,31,55,0,23,42,0,19,29,0,11,19,0,
    191,136,0,183,130,0,175,123,0,167,117,0,159,110,0,155,
    104,0,147,100,0,139,94,0,131,87,0,123,81,0,119,77,
    0,111,71,0,103,68,0,95,61,0,87,55,0,83,51,0,
    159,107,0,143,97,0,131,87,0,119,77,0,103,68,0,91,
    58,0,79,48,0,67,42,0,123,104,0,111,94,0,103,87,
    0,91,81,0,83,71,0,71,64,0,63,58,0,55,51,0,
    255,208,0,235,179,0,215,153,0,195,126,0,175,100,0,155,
    74,0,135,55,0,115,35,0,255,208,0,255,179,0,255,153,
    0,255,126,0,255,100,0,255,77,0,255,51,0,255,25,0,
    255,0,0,239,0,0,227,0,0,215,0,0,203,0,0,191,
    0,0,179,0,0,167,0,0,155,0,0,139,0,0,127,0,
    0,115,0,0,103,0,0,91,0,0,79,0,0,67,0,0,
    231,188,0,199,162,0,171,139,0,143,117,0,115,94,0,83,
    68,0,55,45,0,27,22,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    255,208,0,255,192,0,255,175,0,255,162,0,255,146,0,255,
    133,0,255,117,0,255,104,0,243,94,0,235,91,0,223,84,
    0,215,77,0,203,71,0,195,64,0,183,58,0,175,55,0,
    255,208,0,255,208,0,255,208,0,255,208,0,255,208,0,255,
    208,0,255,208,0,255,208,0,167,51,0,159,45,0,147,38,
    0,135,29,0,79,48,0,67,38,0,55,29,0,47,22,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,255,130,0,255,188,0,255,100,
    0,255,0,0,207,0,0,159,0,0,111,0,0,167,87,0
};

const byte C_FFDE4C[] = {
    0,0,0,31,20,3,23,13,2,75,65,22,255,222,76,27,
    24,8,19,17,6,11,10,3,7,6,2,47,48,9,35,37,
    4,23,27,2,15,20,0,79,51,13,71,44,10,63,37,8,
    255,159,55,247,149,51,243,142,49,235,131,45,231,124,43,223,
    118,40,219,107,37,211,100,34,203,93,32,199,86,30,191,79,
    27,187,76,26,179,69,24,175,62,21,167,55,19,163,51,18,
    155,44,15,151,41,14,143,37,13,139,30,10,131,27,9,127,
    24,8,119,20,7,115,17,6,107,13,4,103,10,3,95,6,
    2,91,6,2,83,6,2,79,0,0,71,0,0,67,0,0,
    255,205,66,255,198,63,255,191,59,255,184,56,255,180,53,255,
    173,50,255,166,46,255,163,44,255,156,39,247,149,37,239,142,
    34,231,135,32,223,128,30,215,121,27,207,114,25,203,111,24,
    191,107,22,179,100,21,171,97,20,163,93,19,155,86,18,143,
    83,16,135,76,15,127,72,14,119,69,13,107,62,12,95,58,
    10,83,55,9,75,48,8,63,41,7,51,37,6,43,30,4,
    239,208,71,231,201,69,223,194,66,219,191,65,211,184,63,203,
    177,61,199,173,59,191,166,57,183,159,55,179,156,53,171,149,
    51,167,145,50,159,138,47,151,131,45,147,128,44,139,121,41,
    131,114,39,127,111,38,119,104,35,111,97,33,107,93,32,99,
    86,30,91,79,27,87,76,26,79,69,24,71,62,21,67,58,
    20,59,51,18,55,48,16,47,41,14,39,34,12,35,30,10,
    119,222,33,111,208,31,103,194,28,95,180,26,91,166,24,83,
    152,21,75,138,19,67,128,16,63,114,14,55,100,13,47,86,
    10,39,72,8,31,58,7,23,44,4,19,30,3,11,20,2,
    191,145,43,183,138,40,175,131,38,167,124,35,159,118,33,155,
    111,32,147,107,30,139,100,27,131,93,26,123,86,24,119,83,
    22,111,76,20,103,72,19,95,65,16,87,58,15,83,55,14,
    159,114,30,143,104,25,131,93,22,119,83,19,103,72,15,91,
    62,13,79,51,10,67,44,8,123,111,30,111,100,26,103,93,
    24,91,86,21,83,76,18,71,69,15,63,62,13,55,55,12,
    255,222,34,235,191,26,215,163,20,195,135,14,175,107,9,155,
    79,6,135,58,2,115,37,0,255,222,76,255,191,65,255,163,
    56,255,135,46,255,107,37,255,83,28,255,55,19,255,27,9,
    255,0,0,239,0,0,227,0,0,215,0,0,203,0,0,191,
    0,0,179,0,0,167,0,0,155,0,0,139,0,0,127,0,
    0,115,0,0,103,0,0,91,0,0,79,0,0,67,0,0,
    231,201,76,199,173,76,171,149,76,143,124,76,115,100,76,83,
    72,76,55,48,76,27,24,76,0,0,76,0,0,68,0,0,
    61,0,0,53,0,0,46,0,0,39,0,0,32,0,0,25,
    255,222,76,255,205,65,255,187,56,255,173,46,255,156,37,255,
    142,27,255,124,18,255,111,8,243,100,7,235,97,4,223,90,
    4,215,83,3,203,76,2,195,69,0,183,62,0,175,58,0,
    255,222,76,255,222,64,255,222,53,255,222,43,255,222,32,255,
    222,21,255,222,10,255,222,0,167,55,0,159,48,0,147,41,
    0,135,30,0,79,51,12,67,41,8,55,30,6,47,24,3,
    0,0,25,0,0,21,0,0,18,0,0,14,0,0,10,0,
    0,7,0,0,3,0,0,0,255,138,20,255,201,22,255,107,
    76,255,0,76,207,0,62,159,0,46,111,0,32,167,93,32
};

const byte C_FFF588[] = {
    0,0,0,31,22,6,23,14,4,75,72,40,255,245,136,27,
    26,14,19,18,10,11,11,6,7,7,4,47,53,17,35,41,
    8,23,30,4,15,22,0,79,57,23,71,49,19,63,41,14,
    255,176,98,247,164,91,243,157,87,235,145,81,231,137,76,223,
    130,72,219,118,66,211,110,61,203,103,57,199,95,53,191,87,
    49,187,84,46,179,76,42,175,68,38,167,61,34,163,57,31,
    155,49,27,151,45,25,143,41,23,139,34,19,131,30,17,127,
    26,14,119,22,12,115,18,10,107,14,8,103,11,6,95,7,
    4,91,7,4,83,7,4,79,0,0,71,0,0,67,0,0,
    255,226,119,255,218,113,255,210,106,255,203,100,255,199,95,255,
    191,89,255,184,83,255,180,78,255,172,70,247,164,66,239,157,
    61,231,149,57,223,141,53,215,134,49,207,126,44,203,122,42,
    191,118,40,179,110,38,171,107,36,163,103,34,155,95,31,143,
    91,29,135,84,27,127,80,25,119,76,23,107,68,21,95,64,
    19,83,61,17,75,53,14,63,45,12,51,41,10,43,34,8,
    239,230,127,231,222,123,223,214,119,219,210,117,211,203,113,203,
    195,108,199,191,106,191,184,102,183,176,98,179,172,95,171,164,
    91,167,160,89,159,153,85,151,145,81,147,141,78,139,134,74,
    131,126,70,127,122,68,119,114,63,111,107,59,107,103,57,99,
    95,53,91,87,49,87,84,46,79,76,42,71,68,38,67,64,
    36,59,57,31,55,53,29,47,45,25,39,37,21,35,34,19,
    119,245,59,111,230,55,103,214,51,95,199,46,91,184,42,83,
    168,38,75,153,34,67,141,29,63,126,25,55,110,23,47,95,
    19,39,80,14,31,64,12,23,49,8,19,34,6,11,22,4,
    191,160,76,183,153,72,175,145,68,167,137,63,159,130,59,155,
    122,57,147,118,53,139,110,49,131,103,46,123,95,42,119,91,
    40,111,84,36,103,80,34,95,72,29,87,64,27,83,61,25,
    159,126,53,143,114,44,131,103,40,119,91,34,103,80,27,91,
    68,23,79,57,19,67,49,14,123,122,53,111,110,46,103,103,
    42,91,95,38,83,84,31,71,76,27,63,68,23,55,61,21,
    255,245,61,235,210,46,215,180,36,195,149,25,175,118,17,155,
    87,10,135,64,4,115,41,0,255,245,136,255,210,117,255,180,
    100,255,149,83,255,118,66,255,91,51,255,61,34,255,30,17,
    255,0,0,239,0,0,227,0,0,215,0,0,203,0,0,191,
    0,0,179,0,0,167,0,0,155,0,0,139,0,0,127,0,
    0,115,0,0,103,0,0,91,0,0,79,0,0,67,0,0,
    231,222,136,199,191,136,171,164,136,143,137,136,115,110,136,83,
    80,136,55,53,136,27,26,136,0,0,136,0,0,121,0,0,
    108,0,0,95,0,0,83,0,0,70,0,0,57,0,0,44,
    255,245,136,255,226,117,255,207,100,255,191,83,255,172,66,255,
    157,49,255,137,31,255,122,14,243,110,12,235,107,8,223,99,
    8,215,91,6,203,84,4,195,76,0,183,68,0,175,64,0,
    255,245,136,255,245,115,255,245,95,255,245,76,255,245,57,255,
    245,38,255,245,19,255,245,0,167,61,0,159,53,0,147,45,
    0,135,34,0,79,57,21,67,45,14,55,34,10,47,26,6,
    0,0,44,0,0,38,0,0,31,0,0,25,0,0,19,0,
    0,12,0,0,6,0,0,0,255,153,36,255,222,40,255,118,
    136,255,0,136,207,0,110,159,0,83,111,0,57,167,103,57
};

const byte C_043E8B[] = {
    0,0,0,4,11,11,3,7,7,9,37,75,31,127,255,3,
    13,27,2,9,19,1,5,11,1,3,7,6,27,31,4,21,
    15,3,15,7,2,11,0,10,29,43,9,25,35,8,21,27,
    31,91,183,30,85,171,30,81,163,29,75,151,28,71,143,27,
    67,135,27,61,123,26,57,115,25,53,107,24,49,99,23,45,
    91,23,43,87,22,39,79,21,35,71,20,31,63,20,29,59,
    19,25,51,18,23,47,17,21,43,17,17,35,16,15,31,15,
    13,27,14,11,23,14,9,19,13,7,15,13,5,11,12,3,
    7,11,3,7,10,3,7,10,0,0,9,0,0,8,0,0,
    31,117,223,31,113,211,31,109,199,31,105,187,31,103,179,31,
    99,167,31,95,155,31,93,147,31,89,131,30,85,123,29,81,
    115,28,77,107,27,73,99,26,69,91,25,65,83,25,63,79,
    23,61,75,22,57,71,21,55,67,20,53,63,19,49,59,17,
    47,55,16,43,51,15,41,47,14,39,43,13,35,39,12,33,
    35,10,31,31,9,27,27,8,23,23,6,21,19,5,17,15,
    29,119,239,28,115,231,27,111,223,27,109,219,26,105,211,25,
    101,203,24,99,199,23,95,191,22,91,183,22,89,179,21,85,
    171,20,83,167,19,79,159,18,75,151,18,73,147,17,69,139,
    16,65,131,15,63,127,14,59,119,13,55,111,13,53,107,12,
    49,99,11,45,91,11,43,87,10,39,79,9,35,71,8,33,
    67,7,29,59,7,27,55,6,23,47,5,19,39,4,17,35,
    14,127,111,13,119,103,13,111,95,12,103,87,11,95,79,10,
    87,71,9,79,63,8,73,55,8,65,47,7,57,43,6,49,
    35,5,41,27,4,33,23,3,25,15,2,17,11,1,11,7,
    23,83,143,22,79,135,21,75,127,20,71,119,19,67,111,19,
    63,107,18,61,99,17,57,91,16,53,87,15,49,79,14,47,
    75,13,43,67,13,41,63,12,37,55,11,33,51,10,31,47,
    19,65,99,17,59,83,16,53,75,14,47,63,13,41,51,11,
    35,43,10,29,35,8,25,27,15,63,99,13,57,87,13,53,
    79,11,49,71,10,43,59,9,39,51,8,35,43,7,31,39,
    31,127,115,29,109,87,26,93,67,24,77,47,21,61,31,19,
    45,19,16,33,7,14,21,0,31,127,255,31,109,219,31,93,
    187,31,77,155,31,61,123,31,47,95,31,31,63,31,15,31,
    31,0,0,29,0,0,28,0,0,26,0,0,25,0,0,23,
    0,0,22,0,0,20,0,0,19,0,0,17,0,0,15,0,
    0,14,0,0,13,0,0,11,0,0,10,0,0,8,0,0,
    28,115,255,24,99,255,21,85,255,17,71,255,14,57,255,10,
    41,255,7,27,255,3,13,255,0,0,255,0,0,227,0,0,
    203,0,0,179,0,0,155,0,0,131,0,0,107,0,0,83,
    31,127,255,31,117,219,31,107,187,31,99,155,31,89,123,31,
    81,91,31,71,59,31,63,27,30,57,23,29,55,15,27,51,
    15,26,47,11,25,43,7,24,39,0,22,35,0,21,33,0,
    31,127,255,31,127,215,31,127,179,31,127,143,31,127,107,31,
    127,71,31,127,35,31,127,0,20,31,0,19,27,0,18,23,
    0,16,17,0,10,29,39,8,23,27,7,17,19,6,13,11,
    0,0,83,0,0,71,0,0,59,0,0,47,0,0,35,0,
    0,23,0,0,11,0,0,0,31,79,67,31,115,75,31,61,
    255,31,0,255,25,0,207,19,0,155,13,0,107,20,53,107
};

const byte C_5B4318[] = {
    0,0,0,11,6,1,8,4,1,27,20,7,91,67,24,10,
    7,3,7,5,2,4,3,1,2,2,1,17,14,3,12,11,
    1,8,8,1,5,6,0,28,16,4,25,13,3,22,11,3,
    91,48,17,88,45,16,87,43,15,84,40,14,82,38,13,80,
    35,13,78,32,12,75,30,11,72,28,10,71,26,9,68,24,
    9,67,23,8,64,21,7,62,19,7,60,17,6,58,16,6,
    55,13,5,54,12,4,51,11,4,50,9,3,47,8,3,45,
    7,3,42,6,2,41,5,2,38,4,1,37,3,1,34,2,
    1,32,2,1,30,2,1,28,0,0,25,0,0,24,0,0,
    91,62,21,91,60,20,91,58,19,91,55,18,91,54,17,91,
    52,16,91,50,15,91,49,14,91,47,12,88,45,12,85,43,
    11,82,41,10,80,39,9,77,37,9,74,34,8,72,33,7,
    68,32,7,64,30,7,61,29,6,58,28,6,55,26,6,51,
    25,5,48,23,5,45,22,4,42,21,4,38,19,4,34,18,
    3,30,17,3,27,14,3,22,12,2,18,11,2,15,9,1,
    85,63,22,82,61,22,80,59,21,78,58,21,75,55,20,72,
    53,19,71,52,19,68,50,18,65,48,17,64,47,17,61,45,
    16,60,44,16,57,42,15,54,40,14,52,39,14,50,37,13,
    47,34,12,45,33,12,42,31,11,40,29,10,38,28,10,35,
    26,9,32,24,9,31,23,8,28,21,7,25,19,7,24,18,
    6,21,16,6,20,14,5,17,12,4,14,10,4,12,9,3,
    42,67,10,40,63,10,37,59,9,34,54,8,32,50,7,30,
    46,7,27,42,6,24,39,5,22,34,4,20,30,4,17,26,
    3,14,22,3,11,18,2,8,13,1,7,9,1,4,6,1,
    68,44,13,65,42,13,62,40,12,60,38,11,57,35,10,55,
    33,10,52,32,9,50,30,9,47,28,8,44,26,7,42,25,
    7,40,23,6,37,22,6,34,20,5,31,18,5,30,17,4,
    57,34,9,51,31,8,47,28,7,42,25,6,37,22,5,32,
    19,4,28,16,3,24,13,3,44,33,9,40,30,8,37,28,
    7,32,26,7,30,23,6,25,21,5,22,19,4,20,17,4,
    91,67,11,84,58,8,77,49,6,70,41,4,62,32,3,55,
    24,2,48,18,1,41,11,0,91,67,24,91,58,21,91,49,
    18,91,41,15,91,32,12,91,25,9,91,17,6,91,8,3,
    91,0,0,85,0,0,81,0,0,77,0,0,72,0,0,68,
    0,0,64,0,0,60,0,0,55,0,0,50,0,0,45,0,
    0,41,0,0,37,0,0,32,0,0,28,0,0,24,0,0,
    82,61,24,71,52,24,61,45,24,51,38,24,41,30,24,30,
    22,24,20,14,24,10,7,24,0,0,24,0,0,21,0,0,
    19,0,0,17,0,0,15,0,0,12,0,0,10,0,0,8,
    91,67,24,91,62,21,91,56,18,91,52,15,91,47,12,91,
    43,9,91,38,6,91,33,3,87,30,2,84,29,1,80,27,
    1,77,25,1,72,23,1,70,21,0,65,19,0,62,18,0,
    91,67,24,91,67,20,91,67,17,91,67,13,91,67,10,91,
    67,7,91,67,3,91,67,0,60,17,0,57,14,0,52,12,
    0,48,9,0,28,16,4,24,12,3,20,9,2,17,7,1,
    0,0,8,0,0,7,0,0,6,0,0,4,0,0,3,0,
    0,2,0,0,1,0,0,0,91,42,6,91,61,7,91,32,
    24,91,0,24,74,0,19,57,0,15,40,0,10,60,28,10
};

const byte C_4F5D8B[] = {
    0,0,0,10,8,6,7,5,4,23,27,41,79,93,139,8,
    10,15,6,7,10,3,4,6,2,3,4,15,20,17,11,16,
    8,7,11,4,5,8,0,24,22,23,22,19,19,20,16,15,
    79,67,100,77,62,93,75,59,89,73,55,82,72,52,78,69,
    49,74,68,45,67,65,42,63,63,39,58,62,36,54,59,33,
    50,58,32,47,55,29,43,54,26,39,52,23,34,50,22,32,
    48,19,28,47,17,26,44,16,23,43,13,19,41,11,17,39,
    10,15,37,8,13,36,7,10,33,5,8,32,4,6,29,3,
    4,28,3,4,26,3,4,24,0,0,22,0,0,21,0,0,
    79,86,122,79,83,115,79,80,108,79,77,102,79,75,98,79,
    73,91,79,70,84,79,68,80,79,65,71,77,62,67,74,59,
    63,72,57,58,69,54,54,67,51,50,64,48,45,63,46,43,
    59,45,41,55,42,39,53,40,37,50,39,34,48,36,32,44,
    35,30,42,32,28,39,30,26,37,29,23,33,26,21,29,24,
    19,26,23,17,23,20,15,20,17,13,16,16,10,13,13,8,
    74,87,130,72,84,126,69,81,122,68,80,119,65,77,115,63,
    74,111,62,73,108,59,70,104,57,67,100,55,65,98,53,62,
    93,52,61,91,49,58,87,47,55,82,46,54,80,43,51,76,
    41,48,71,39,46,69,37,43,65,34,40,61,33,39,58,31,
    36,54,28,33,50,27,32,47,24,29,43,22,26,39,21,24,
    37,18,22,32,17,20,30,15,17,26,12,14,21,11,13,19,
    37,93,61,34,87,56,32,81,52,29,75,47,28,70,43,26,
    64,39,23,58,34,21,54,30,20,48,26,17,42,23,15,36,
    19,12,30,15,10,24,13,7,19,8,6,13,6,3,8,4,
    59,61,78,57,58,74,54,55,69,52,52,65,49,49,61,48,
    46,58,46,45,54,43,42,50,41,39,47,38,36,43,37,35,
    41,34,32,37,32,30,34,29,27,30,27,24,28,26,23,26,
    49,48,54,44,43,45,41,39,41,37,35,34,32,30,28,28,
    26,23,24,22,19,21,19,15,38,46,54,34,42,47,32,39,
    43,28,36,39,26,32,32,22,29,28,20,26,23,17,23,21,
    79,93,63,73,80,47,67,68,37,60,57,26,54,45,17,48,
    33,10,42,24,4,36,16,0,79,93,139,79,80,119,79,68,
    102,79,57,84,79,45,67,79,35,52,79,23,34,79,11,17,
    79,0,0,74,0,0,70,0,0,67,0,0,63,0,0,59,
    0,0,55,0,0,52,0,0,48,0,0,43,0,0,39,0,
    0,36,0,0,32,0,0,28,0,0,24,0,0,21,0,0,
    72,84,139,62,73,139,53,62,139,44,52,139,36,42,139,26,
    30,139,17,20,139,8,10,139,0,0,139,0,0,124,0,0,
    111,0,0,98,0,0,84,0,0,71,0,0,58,0,0,45,
    79,93,139,79,86,119,79,78,102,79,73,84,79,65,67,79,
    59,50,79,52,32,79,46,15,75,42,13,73,40,8,69,38,
    8,67,35,6,63,32,4,60,29,0,57,26,0,54,24,0,
    79,93,139,79,93,117,79,93,98,79,93,78,79,93,58,79,
    93,39,79,93,19,79,93,0,52,23,0,49,20,0,46,17,
    0,42,13,0,24,22,21,21,17,15,17,13,10,15,10,6,
    0,0,45,0,0,39,0,0,32,0,0,26,0,0,19,0,
    0,13,0,0,6,0,0,0,79,58,37,79,84,41,79,45,
    139,79,0,139,64,0,113,49,0,84,34,0,58,52,39,58
};

const byte C_D46D3D[] = {
    0,0,0,26,10,3,19,6,2,62,32,18,212,109,61,22,
    12,6,16,8,5,9,5,3,6,3,2,39,24,7,29,18,
    4,19,13,2,12,10,0,66,25,10,59,22,8,52,18,6,
    212,78,44,205,73,41,202,70,39,195,65,36,192,61,34,185,
    58,32,182,53,29,175,49,28,169,46,26,165,42,24,159,39,
    22,155,37,21,149,34,19,145,30,17,139,27,15,136,25,14,
    129,22,12,126,20,11,119,18,10,116,15,8,109,13,7,106,
    12,6,99,10,6,96,8,5,89,6,4,86,5,3,79,3,
    2,76,3,2,69,3,2,66,0,0,59,0,0,56,0,0,
    212,100,53,212,97,50,212,94,48,212,90,45,212,88,43,212,
    85,40,212,82,37,212,80,35,212,77,31,205,73,29,199,70,
    28,192,66,26,185,63,24,179,59,22,172,56,20,169,54,19,
    159,53,18,149,49,17,142,47,16,136,46,15,129,42,14,119,
    41,13,112,37,12,106,35,11,99,34,10,89,30,9,79,29,
    8,69,27,7,62,24,6,52,20,6,42,18,5,36,15,4,
    199,102,57,192,99,55,185,95,53,182,94,52,175,90,50,169,
    87,49,165,85,48,159,82,46,152,78,44,149,77,43,142,73,
    41,139,71,40,132,68,38,126,65,36,122,63,35,116,59,33,
    109,56,31,106,54,30,99,51,28,92,47,27,89,46,26,82,
    42,24,76,39,22,72,37,21,66,34,19,59,30,17,56,29,
    16,49,25,14,46,24,13,39,20,11,32,17,9,29,15,8,
    99,109,27,92,102,25,86,95,23,79,88,21,76,82,19,69,
    75,17,62,68,15,56,63,13,52,56,11,46,49,10,39,42,
    8,32,35,6,26,29,6,19,22,4,16,15,3,9,10,2,
    159,71,34,152,68,32,145,65,30,139,61,28,132,58,27,129,
    54,26,122,53,24,116,49,22,109,46,21,102,42,19,99,41,
    18,92,37,16,86,35,15,79,32,13,72,29,12,69,27,11,
    132,56,24,119,51,20,109,46,18,99,41,15,86,35,12,76,
    30,10,66,25,8,56,22,6,102,54,24,92,49,21,86,46,
    19,76,42,17,69,37,14,59,34,12,52,30,10,46,27,9,
    212,109,28,195,94,21,179,80,16,162,66,11,145,53,7,129,
    39,5,112,29,2,96,18,0,212,109,61,212,94,52,212,80,
    45,212,66,37,212,53,29,212,41,23,212,27,15,212,13,7,
    212,0,0,199,0,0,189,0,0,179,0,0,169,0,0,159,
    0,0,149,0,0,139,0,0,129,0,0,116,0,0,106,0,
    0,96,0,0,86,0,0,76,0,0,66,0,0,56,0,0,
    192,99,61,165,85,61,142,73,61,119,61,61,96,49,61,69,
    35,61,46,24,61,22,12,61,0,0,61,0,0,54,0,0,
    49,0,0,43,0,0,37,0,0,31,0,0,26,0,0,20,
    212,109,61,212,100,52,212,92,45,212,85,37,212,77,29,212,
    70,22,212,61,14,212,54,6,202,49,6,195,47,4,185,44,
    4,179,41,3,169,37,2,162,34,0,152,30,0,145,29,0,
    212,109,61,212,109,51,212,109,43,212,109,34,212,109,26,212,
    109,17,212,109,8,212,109,0,139,27,0,132,24,0,122,20,
    0,112,15,0,66,25,9,56,20,6,46,15,5,39,12,3,
    0,0,20,0,0,17,0,0,14,0,0,11,0,0,8,0,
    0,6,0,0,3,0,0,0,212,68,16,212,99,18,212,53,
    61,212,0,61,172,0,50,132,0,37,92,0,26,139,46,26
};

const byte C_04918B[] = {
    0,0,0,0,13,6,0,9,4,1,43,41,4,145,139,0,
    15,15,0,11,10,0,6,6,0,4,4,1,31,17,1,24,
    8,0,18,4,0,13,0,1,34,23,1,29,19,1,24,15,
    4,104,100,4,97,93,4,93,89,4,86,82,4,81,78,3,
    77,74,3,70,67,3,65,63,3,61,58,3,56,54,3,52,
    50,3,49,47,3,45,43,3,40,39,3,36,34,3,34,32,
    2,29,28,2,27,26,2,24,23,2,20,19,2,18,17,2,
    15,15,2,13,13,2,11,10,2,9,8,2,6,6,1,4,
    4,1,4,4,1,4,4,1,0,0,1,0,0,1,0,0,
    4,134,122,4,129,115,4,125,108,4,120,102,4,118,98,4,
    113,91,4,109,84,4,106,80,4,102,71,4,97,67,4,93,
    63,4,88,58,3,84,54,3,79,50,3,74,45,3,72,43,
    3,70,41,3,65,39,3,63,37,3,61,34,2,56,32,2,
    54,30,2,49,28,2,47,26,2,45,23,2,40,21,1,38,
    19,1,36,17,1,31,15,1,27,13,1,24,10,1,20,8,
    4,136,130,4,131,126,3,127,122,3,125,119,3,120,115,3,
    115,111,3,113,108,3,109,104,3,104,100,3,102,98,3,97,
    93,3,95,91,2,90,87,2,86,82,2,84,80,2,79,76,
    2,74,71,2,72,69,2,68,65,2,63,61,2,61,58,2,
    56,54,1,52,50,1,49,47,1,45,43,1,40,39,1,38,
    37,1,34,32,1,31,30,1,27,26,1,22,21,1,20,19,
    2,145,61,2,136,56,2,127,52,1,118,47,1,109,43,1,
    100,39,1,90,34,1,84,30,1,74,26,1,65,23,1,56,
    19,1,47,15,0,38,13,0,29,8,0,20,6,0,13,4,
    3,95,78,3,90,74,3,86,69,3,81,65,2,77,61,2,
    72,58,2,70,54,2,65,50,2,61,47,2,56,43,2,54,
    41,2,49,37,2,47,34,1,43,30,1,38,28,1,36,26,
    2,74,54,2,68,45,2,61,41,2,54,34,2,47,28,1,
    40,23,1,34,19,1,29,15,2,72,54,2,65,47,2,61,
    43,1,56,39,1,49,32,1,45,28,1,40,23,1,36,21,
    4,145,63,4,125,47,3,106,37,3,88,26,3,70,17,2,
    52,10,2,38,4,2,24,0,4,145,139,4,125,119,4,106,
    102,4,88,84,4,70,67,4,54,52,4,36,34,4,18,17,
    4,0,0,4,0,0,4,0,0,3,0,0,3,0,0,3,
    0,0,3,0,0,3,0,0,2,0,0,2,0,0,2,0,
    0,2,0,0,2,0,0,1,0,0,1,0,0,1,0,0,
    4,131,139,3,113,139,3,97,139,2,81,139,2,65,139,1,
    47,139,1,31,139,0,15,139,0,0,139,0,0,124,0,0,
    111,0,0,98,0,0,84,0,0,71,0,0,58,0,0,45,
    4,145,139,4,134,119,4,122,102,4,113,84,4,102,67,4,
    93,50,4,81,32,4,72,15,4,65,13,4,63,8,3,59,
    8,3,54,6,3,49,4,3,45,0,3,40,0,3,38,0,
    4,145,139,4,145,117,4,145,98,4,145,78,4,145,58,4,
    145,39,4,145,19,4,145,0,3,36,0,2,31,0,2,27,
    0,2,20,0,1,34,21,1,27,15,1,20,10,1,15,6,
    0,0,45,0,0,39,0,0,32,0,0,26,0,0,19,0,
    0,13,0,0,6,0,0,0,4,90,37,4,131,41,4,70,
    139,4,0,139,3,0,113,2,0,84,2,0,58,3,61,58
};

const byte C_FF3030[] = {
    0,0,0,31,4,2,23,3,1,75,14,14,255,48,48,27,
    5,5,19,4,4,11,2,2,7,1,1,47,10,6,35,8,
    3,23,6,1,15,4,0,79,11,8,71,10,7,63,8,5,
    255,34,34,247,32,32,243,31,31,235,28,28,231,27,27,223,
    25,25,219,23,23,211,22,22,203,20,20,199,19,19,191,17,
    17,187,16,16,179,15,15,175,13,13,167,12,12,163,11,11,
    155,10,10,151,9,9,143,8,8,139,7,7,131,6,6,127,
    5,5,119,4,4,115,4,4,107,3,3,103,2,2,95,1,
    1,91,1,1,83,1,1,79,0,0,71,0,0,67,0,0,
    255,44,42,255,43,40,255,41,37,255,40,35,255,39,34,255,
    37,31,255,36,29,255,35,28,255,34,25,247,32,23,239,31,
    22,231,29,20,223,28,19,215,26,17,207,25,16,203,24,15,
    191,23,14,179,22,13,171,21,13,163,20,12,155,19,11,143,
    18,10,135,16,10,127,16,9,119,15,8,107,13,7,95,13,
    7,83,12,6,75,10,5,63,9,4,51,8,4,43,7,3,
    239,45,45,231,43,43,223,42,42,219,41,41,211,40,40,203,
    38,38,199,37,37,191,36,36,183,34,34,179,34,34,171,32,
    32,167,31,31,159,30,30,151,28,28,147,28,28,139,26,26,
    131,25,25,127,24,24,119,22,22,111,21,21,107,20,20,99,
    19,19,91,17,17,87,16,16,79,15,15,71,13,13,67,13,
    13,59,11,11,55,10,10,47,9,9,39,7,7,35,7,7,
    119,48,21,111,45,19,103,42,18,95,39,16,91,36,15,83,
    33,13,75,30,12,67,28,10,63,25,9,55,22,8,47,19,
    7,39,16,5,31,13,4,23,10,3,19,7,2,11,4,1,
    191,31,27,183,30,25,175,28,24,167,27,22,159,25,21,155,
    24,20,147,23,19,139,22,17,131,20,16,123,19,15,119,18,
    14,111,16,13,103,16,12,95,14,10,87,13,10,83,12,9,
    159,25,19,143,22,16,131,20,14,119,18,12,103,16,10,91,
    13,8,79,11,7,67,10,5,123,24,19,111,22,16,103,20,
    15,91,19,13,83,16,11,71,15,10,63,13,8,55,12,7,
    255,48,22,235,41,16,215,35,13,195,29,9,175,23,6,155,
    17,4,135,13,1,115,8,0,255,48,48,255,41,41,255,35,
    35,255,29,29,255,23,23,255,18,18,255,12,12,255,6,6,
    255,0,0,239,0,0,227,0,0,215,0,0,203,0,0,191,
    0,0,179,0,0,167,0,0,155,0,0,139,0,0,127,0,
    0,115,0,0,103,0,0,91,0,0,79,0,0,67,0,0,
    231,43,48,199,37,48,171,32,48,143,27,48,115,22,48,83,
    16,48,55,10,48,27,5,48,0,0,48,0,0,43,0,0,
    38,0,0,34,0,0,29,0,0,25,0,0,20,0,0,16,
    255,48,48,255,44,41,255,40,35,255,37,29,255,34,23,255,
    31,17,255,27,11,255,24,5,243,22,4,235,21,3,223,19,
    3,215,18,2,203,16,1,195,15,0,183,13,0,175,13,0,
    255,48,48,255,48,40,255,48,34,255,48,27,255,48,20,255,
    48,13,255,48,7,255,48,0,167,12,0,159,10,0,147,9,
    0,135,7,0,79,11,7,67,9,5,55,7,4,47,5,2,
    0,0,16,0,0,13,0,0,11,0,0,9,0,0,7,0,
    0,4,0,0,2,0,0,0,255,30,13,255,43,14,255,23,
    48,255,0,48,207,0,39,159,0,29,111,0,20,167,20,20
};

const byte C_311A59[] = {
    0,0,0,16,6,10,12,4,6,38,21,68,129,70,232,14,
    7,25,10,5,17,6,3,10,4,2,6,24,15,28,18,12,
    14,12,9,6,8,6,0,40,16,39,36,14,32,32,12,25,
    129,50,166,125,47,156,123,45,148,119,41,137,117,39,130,113,
    37,123,111,34,112,107,32,105,103,29,97,101,27,90,97,25,
    83,95,24,79,91,22,72,89,19,65,84,17,57,82,16,54,
    78,14,46,76,13,43,72,12,39,70,10,32,66,9,28,64,
    7,25,60,6,21,58,5,17,54,4,14,52,3,10,48,2,
    6,46,2,6,42,2,6,40,0,0,36,0,0,34,0,0,
    129,65,203,129,62,192,129,60,181,129,58,170,129,57,163,129,
    55,152,129,52,141,129,51,134,129,49,119,125,47,112,121,45,
    105,117,43,97,113,40,90,109,38,83,105,36,76,103,35,72,
    97,34,68,91,32,65,87,30,61,82,29,57,78,27,54,72,
    26,50,68,24,46,64,23,43,60,22,39,54,19,35,48,18,
    32,42,17,28,38,15,25,32,13,21,26,12,17,22,10,14,
    121,66,217,117,63,210,113,61,203,111,60,199,107,58,192,103,
    56,185,101,55,181,97,52,174,93,50,166,91,49,163,87,47,
    156,84,46,152,80,44,145,76,41,137,74,40,134,70,38,126,
    66,36,119,64,35,116,60,33,108,56,30,101,54,29,97,50,
    27,90,46,25,83,44,24,79,40,22,72,36,19,65,34,18,
    61,30,16,54,28,15,50,24,13,43,20,11,35,18,10,32,
    60,70,101,56,66,94,52,61,86,48,57,79,46,52,72,42,
    48,65,38,44,57,34,40,50,32,36,43,28,32,39,24,27,
    32,20,23,25,16,18,21,12,14,14,10,10,10,6,6,6,
    97,46,130,93,44,123,89,41,116,84,39,108,80,37,101,78,
    35,97,74,34,90,70,32,83,66,29,79,62,27,72,60,26,
    68,56,24,61,52,23,57,48,21,50,44,18,46,42,17,43,
    80,36,90,72,33,76,66,29,68,60,26,57,52,23,46,46,
    19,39,40,16,32,34,14,25,62,35,90,56,32,79,52,29,
    72,46,27,65,42,24,54,36,22,46,32,19,39,28,17,35,
    129,70,105,119,60,79,109,51,61,99,43,43,89,34,28,78,
    25,17,68,18,6,58,12,0,129,70,232,129,60,199,129,51,
    170,129,43,141,129,34,112,129,26,86,129,17,57,129,9,28,
    129,0,0,121,0,0,115,0,0,109,0,0,103,0,0,97,
    0,0,91,0,0,84,0,0,78,0,0,70,0,0,64,0,
    0,58,0,0,52,0,0,46,0,0,40,0,0,34,0,0,
    117,63,232,101,55,232,87,47,232,72,39,232,58,32,232,42,
    23,232,28,15,232,14,7,232,0,0,232,0,0,207,0,0,
    185,0,0,163,0,0,141,0,0,119,0,0,97,0,0,76,
    129,70,232,129,65,199,129,59,170,129,55,141,129,49,112,129,
    45,83,129,39,54,129,35,25,123,32,21,119,30,14,113,28,
    14,109,26,10,103,24,6,99,22,0,93,19,0,89,18,0,
    129,70,232,129,70,196,129,70,163,129,70,130,129,70,97,129,
    70,65,129,70,32,129,70,0,84,17,0,80,15,0,74,13,
    0,68,10,0,40,16,35,34,13,25,28,10,17,24,7,10,
    0,0,76,0,0,65,0,0,54,0,0,43,0,0,32,0,
    0,21,0,0,10,0,0,0,129,44,61,129,63,68,129,34,
    232,129,0,232,105,0,188,80,0,141,56,0,97,84,29,97
};

const byte C_FFAFAF[] = {
    0,0,0,31,16,8,23,10,5,75,51,51,255,175,175,27,
    19,19,19,13,13,11,8,8,7,5,5,47,38,21,35,30,
    10,23,21,5,15,16,0,79,40,30,71,35,24,63,30,19,
    255,126,126,247,117,117,243,112,112,235,104,104,231,98,98,223,
    93,93,219,84,84,211,79,79,203,73,73,199,68,68,191,62,
    62,187,60,60,179,54,54,175,49,49,167,43,43,163,40,40,
    155,35,35,151,32,32,143,30,30,139,24,24,131,21,21,127,
    19,19,119,16,16,115,13,13,107,10,10,103,8,8,95,5,
    5,91,5,5,83,5,5,79,0,0,71,0,0,67,0,0,
    255,161,153,255,156,145,255,150,137,255,145,128,255,142,123,255,
    137,115,255,131,106,255,128,101,255,123,90,247,117,84,239,112,
    79,231,106,73,223,101,68,215,95,62,207,90,57,203,87,54,
    191,84,51,179,79,49,171,76,46,163,73,43,155,68,40,143,
    65,38,135,60,35,127,57,32,119,54,30,107,49,27,95,46,
    24,83,43,21,75,38,19,63,32,16,51,30,13,43,24,10,
    239,164,164,231,159,159,223,153,153,219,150,150,211,145,145,203,
    139,139,199,137,137,191,131,131,183,126,126,179,123,123,171,117,
    117,167,115,115,159,109,109,151,104,104,147,101,101,139,95,95,
    131,90,90,127,87,87,119,82,82,111,76,76,107,73,73,99,
    68,68,91,62,62,87,60,60,79,54,54,71,49,49,67,46,
    46,59,40,40,55,38,38,47,32,32,39,27,27,35,24,24,
    119,175,76,111,164,71,103,153,65,95,142,60,91,131,54,83,
    120,49,75,109,43,67,101,38,63,90,32,55,79,30,47,68,
    24,39,57,19,31,46,16,23,35,10,19,24,8,11,16,5,
    191,115,98,183,109,93,175,104,87,167,98,82,159,93,76,155,
    87,73,147,84,68,139,79,62,131,73,60,123,68,54,119,65,
    51,111,60,46,103,57,43,95,51,38,87,46,35,83,43,32,
    159,90,68,143,82,57,131,73,51,119,65,43,103,57,35,91,
    49,30,79,40,24,67,35,19,123,87,68,111,79,60,103,73,
    54,91,68,49,83,60,40,71,54,35,63,49,30,55,43,27,
    255,175,79,235,150,60,215,128,46,195,106,32,175,84,21,155,
    62,13,135,46,5,115,30,0,255,175,175,255,150,150,255,128,
    128,255,106,106,255,84,84,255,65,65,255,43,43,255,21,21,
    255,0,0,239,0,0,227,0,0,215,0,0,203,0,0,191,
    0,0,179,0,0,167,0,0,155,0,0,139,0,0,127,0,
    0,115,0,0,103,0,0,91,0,0,79,0,0,67,0,0,
    231,159,175,199,137,175,171,117,175,143,98,175,115,79,175,83,
    57,175,55,38,175,27,19,175,0,0,175,0,0,156,0,0,
    139,0,0,123,0,0,106,0,0,90,0,0,73,0,0,57,
    255,175,175,255,161,150,255,148,128,255,137,106,255,123,84,255,
    112,62,255,98,40,255,87,19,243,79,16,235,76,10,223,71,
    10,215,65,8,203,60,5,195,54,0,183,49,0,175,46,0,
    255,175,175,255,175,148,255,175,123,255,175,98,255,175,73,255,
    175,49,255,175,24,255,175,0,167,43,0,159,38,0,147,32,
    0,135,24,0,79,40,27,67,32,19,55,24,13,47,19,8,
    0,0,57,0,0,49,0,0,40,0,0,32,0,0,24,0,
    0,16,0,0,8,0,0,0,255,109,46,255,159,51,255,84,
    175,255,0,175,207,0,142,159,0,106,111,0,73,167,73,73
};

const byte C_ECB866[] = {
    0,0,0,29,17,4,21,11,3,69,54,30,236,184,102,25,
    19,11,18,14,8,10,8,4,6,5,3,43,40,12,32,31,
    6,21,22,3,14,17,0,73,43,17,66,37,14,58,31,11,
    236,132,73,229,123,68,225,118,65,217,109,60,214,103,57,206,
    97,54,203,89,49,195,83,46,188,77,43,184,71,40,177,66,
    36,173,63,35,166,57,32,162,51,28,155,45,25,151,43,24,
    143,37,20,140,34,19,132,31,17,129,25,14,121,22,12,118,
    19,11,110,17,9,106,14,8,99,11,6,95,8,4,88,5,
    3,84,5,3,77,5,3,73,0,0,66,0,0,62,0,0,
    236,170,89,236,164,84,236,158,80,236,152,75,236,149,72,236,
    144,67,236,138,62,236,135,59,236,129,52,229,123,49,221,118,
    46,214,112,43,206,106,40,199,100,36,192,95,33,188,92,32,
    177,89,30,166,83,28,158,80,27,151,77,25,143,71,24,132,
    69,22,125,63,20,118,60,19,110,57,17,99,51,16,88,48,
    14,77,45,12,69,40,11,58,34,9,47,31,8,40,25,6,
    221,172,96,214,167,92,206,161,89,203,158,88,195,152,84,188,
    146,81,184,144,80,177,138,76,169,132,73,166,129,72,158,123,
    68,155,121,67,147,115,64,140,109,60,136,106,59,129,100,56,
    121,95,52,118,92,51,110,86,48,103,80,44,99,77,43,92,
    71,40,84,66,36,81,63,35,73,57,32,66,51,28,62,48,
    27,55,43,24,51,40,22,43,34,19,36,28,16,32,25,14,
    110,184,44,103,172,41,95,161,38,88,149,35,84,138,32,77,
    126,28,69,115,25,62,106,22,58,95,19,51,83,17,43,71,
    14,36,60,11,29,48,9,21,37,6,18,25,4,10,17,3,
    177,121,57,169,115,54,162,109,51,155,103,48,147,97,44,143,
    92,43,136,89,40,129,83,36,121,77,35,114,71,32,110,69,
    30,103,63,27,95,60,25,88,54,22,81,48,20,77,45,19,
    147,95,40,132,86,33,121,77,30,110,69,25,95,60,20,84,
    51,17,73,43,14,62,37,11,114,92,40,103,83,35,95,77,
    32,84,71,28,77,63,24,66,57,20,58,51,17,51,45,16,
    236,184,46,217,158,35,199,135,27,180,112,19,162,89,12,143,
    66,8,125,48,3,106,31,0,236,184,102,236,158,88,236,135,
    75,236,112,62,236,89,49,236,69,38,236,45,25,236,22,12,
    236,0,0,221,0,0,210,0,0,199,0,0,188,0,0,177,
    0,0,166,0,0,155,0,0,143,0,0,129,0,0,118,0,
    0,106,0,0,95,0,0,84,0,0,73,0,0,62,0,0,
    214,167,102,184,144,102,158,123,102,132,103,102,106,83,102,77,
    60,102,51,40,102,25,19,102,0,0,102,0,0,91,0,0,
    81,0,0,72,0,0,62,0,0,52,0,0,43,0,0,33,
    236,184,102,236,170,88,236,155,75,236,144,62,236,129,49,236,
    118,36,236,103,24,236,92,11,225,83,9,217,80,6,206,74,
    6,199,69,4,188,63,3,180,57,0,169,51,0,162,48,0,
    236,184,102,236,184,86,236,184,72,236,184,57,236,184,43,236,
    184,28,236,184,14,236,184,0,155,45,0,147,40,0,136,34,
    0,125,25,0,73,43,16,62,34,11,51,25,8,43,19,4,
    0,0,33,0,0,28,0,0,24,0,0,19,0,0,14,0,
    0,9,0,0,4,0,0,0,236,115,27,236,167,30,236,89,
    102,236,0,102,192,0,83,147,0,62,103,0,43,155,77,43
};

const byte C_C63F23[] = {
    0,0,0,24,6,2,18,4,1,58,19,10,198,63,35,21,
    7,4,15,5,3,9,3,2,5,2,1,36,14,4,27,11,
    2,18,8,1,12,6,0,61,15,6,55,13,5,49,11,4,
    198,45,25,192,42,23,189,40,22,182,37,21,179,35,20,173,
    33,19,170,30,17,164,28,16,158,26,15,155,24,14,148,22,
    12,145,21,12,139,20,11,136,18,10,130,16,9,127,15,8,
    120,13,7,117,12,6,111,11,6,108,9,5,102,8,4,99,
    7,4,92,6,3,89,5,3,83,4,2,80,3,2,74,2,
    1,71,2,1,64,2,1,61,0,0,55,0,0,52,0,0,
    198,58,31,198,56,29,198,54,27,198,52,26,198,51,25,198,
    49,23,198,47,21,198,46,20,198,44,18,192,42,17,186,40,
    16,179,38,15,173,36,14,167,34,12,161,32,11,158,31,11,
    148,30,10,139,28,10,133,27,9,127,26,9,120,24,8,111,
    23,8,105,21,7,99,21,6,92,20,6,83,18,5,74,17,
    5,64,16,4,58,14,4,49,12,3,40,11,3,33,9,2,
    186,59,33,179,57,32,173,55,31,170,54,30,164,52,29,158,
    50,28,155,49,27,148,47,26,142,45,25,139,44,25,133,42,
    23,130,41,23,123,39,22,117,37,21,114,36,20,108,34,19,
    102,32,18,99,31,17,92,29,16,86,27,15,83,26,15,77,
    24,14,71,22,12,68,21,12,61,20,11,55,18,10,52,17,
    9,46,15,8,43,14,8,36,12,6,30,10,5,27,9,5,
    92,63,15,86,59,14,80,55,13,74,51,12,71,47,11,64,
    43,10,58,39,9,52,36,8,49,32,6,43,28,6,36,24,
    5,30,21,4,24,17,3,18,13,2,15,9,2,9,6,1,
    148,41,20,142,39,19,136,37,17,130,35,16,123,33,15,120,
    31,15,114,30,14,108,28,12,102,26,12,96,24,11,92,23,
    10,86,21,9,80,21,9,74,19,8,68,17,7,64,16,6,
    123,32,14,111,29,11,102,26,10,92,23,9,80,21,7,71,
    18,6,61,15,5,52,13,4,96,31,14,86,28,12,80,26,
    11,71,24,10,64,21,8,55,20,7,49,18,6,43,16,5,
    198,63,16,182,54,12,167,46,9,151,38,6,136,30,4,120,
    22,3,105,17,1,89,11,0,198,63,35,198,54,30,198,46,
    26,198,38,21,198,30,17,198,23,13,198,16,9,198,8,4,
    198,0,0,186,0,0,176,0,0,167,0,0,158,0,0,148,
    0,0,139,0,0,130,0,0,120,0,0,108,0,0,99,0,
    0,89,0,0,80,0,0,71,0,0,61,0,0,52,0,0,
    179,57,35,155,49,35,133,42,35,111,35,35,89,28,35,64,
    21,35,43,14,35,21,7,35,0,0,35,0,0,31,0,0,
    28,0,0,25,0,0,21,0,0,18,0,0,15,0,0,11,
    198,63,35,198,58,30,198,53,26,198,49,21,198,44,17,198,
    40,12,198,35,8,198,31,4,189,28,3,182,27,2,173,25,
    2,167,23,2,158,21,1,151,20,0,142,18,0,136,17,0,
    198,63,35,198,63,30,198,63,25,198,63,20,198,63,15,198,
    63,10,198,63,5,198,63,0,130,16,0,123,14,0,114,12,
    0,105,9,0,61,15,5,52,12,4,43,9,3,36,7,2,
    0,0,11,0,0,10,0,0,8,0,0,6,0,0,5,0,
    0,3,0,0,2,0,0,0,198,39,9,198,57,10,198,30,
    35,198,0,35,161,0,28,123,0,21,86,0,15,130,26,15
};

const byte C_9BC8CD[] = {
    0,0,0,19,18,9,14,12,6,46,59,60,155,200,205,16,
    21,22,12,15,15,7,9,9,4,5,6,29,43,25,21,34,
    12,14,24,6,9,18,0,48,46,35,43,40,28,38,34,22,
    155,144,147,150,134,137,148,128,131,143,118,121,140,112,115,136,
    106,109,133,96,99,128,90,92,123,84,86,121,78,80,116,71,
    73,114,68,70,109,62,64,106,56,57,102,49,51,99,46,47,
    94,40,41,92,37,38,87,34,35,84,27,28,80,24,25,77,
    21,22,72,18,18,70,15,15,65,12,12,63,9,9,58,5,
    6,55,5,6,50,5,6,48,0,0,43,0,0,41,0,0,
    155,184,179,155,178,170,155,172,160,155,165,150,155,162,144,155,
    156,134,155,150,125,155,147,118,155,140,105,150,134,99,145,128,
    92,140,122,86,136,115,80,131,109,73,126,103,67,123,100,64,
    116,96,60,109,90,57,104,87,54,99,84,51,94,78,47,87,
    75,44,82,68,41,77,65,38,72,62,35,65,56,31,58,53,
    28,50,49,25,46,43,22,38,37,18,31,34,15,26,27,12,
    145,187,192,140,181,186,136,175,179,133,172,176,128,165,170,123,
    159,163,121,156,160,116,150,154,111,144,147,109,140,144,104,134,
    137,102,131,134,97,125,128,92,118,121,89,115,118,84,109,112,
    80,103,105,77,100,102,72,93,96,67,87,89,65,84,86,60,
    78,80,55,71,73,53,68,70,48,62,64,43,56,57,41,53,
    54,36,46,47,33,43,44,29,37,38,24,31,31,21,27,28,
    72,200,89,67,187,83,63,175,76,58,162,70,55,150,64,50,
    137,57,46,125,51,41,115,44,38,103,38,33,90,35,29,78,
    28,24,65,22,19,53,18,14,40,12,12,27,9,7,18,6,
    116,131,115,111,125,109,106,118,102,102,112,96,97,106,89,94,
    100,86,89,96,80,84,90,73,80,84,70,75,78,64,72,75,
    60,67,68,54,63,65,51,58,59,44,53,53,41,50,49,38,
    97,103,80,87,93,67,80,84,60,72,75,51,63,65,41,55,
    56,35,48,46,28,41,40,22,75,100,80,67,90,70,63,84,
    64,55,78,57,50,68,47,43,62,41,38,56,35,33,49,31,
    155,200,92,143,172,70,131,147,54,119,122,38,106,96,25,94,
    71,15,82,53,6,70,34,0,155,200,205,155,172,176,155,147,
    150,155,122,125,155,96,99,155,75,76,155,49,51,155,24,25,
    155,0,0,145,0,0,138,0,0,131,0,0,123,0,0,116,
    0,0,109,0,0,102,0,0,94,0,0,84,0,0,77,0,
    0,70,0,0,63,0,0,55,0,0,48,0,0,41,0,0,
    140,181,205,121,156,205,104,134,205,87,112,205,70,90,205,50,
    65,205,33,43,205,16,21,205,0,0,205,0,0,182,0,0,
    163,0,0,144,0,0,125,0,0,105,0,0,86,0,0,67,
    155,200,205,155,184,176,155,169,150,155,156,125,155,140,99,155,
    128,73,155,112,47,155,100,22,148,90,18,143,87,12,136,81,
    12,131,75,9,123,68,6,119,62,0,111,56,0,106,53,0,
    155,200,205,155,200,173,155,200,144,155,200,115,155,200,86,155,
    200,57,155,200,28,155,200,0,102,49,0,97,43,0,89,37,
    0,82,27,0,48,46,31,41,37,22,33,27,15,29,21,9,
    0,0,67,0,0,57,0,0,47,0,0,38,0,0,28,0,
    0,18,0,0,9,0,0,0,155,125,54,155,181,60,155,96,
    205,155,0,205,126,0,166,97,0,125,67,0,86,102,84,86
};