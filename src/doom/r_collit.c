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
static lighttable_t   *colormaps_EEC06B;  // Bright yellow/gold
static lighttable_t   *colormaps_D97C45;  // Middle yellow/gold (also E29A56)
static lighttable_t   *colormaps_FF7F7F;  // Bright red
static lighttable_t   *colormaps_55B828;  // Bright green
static lighttable_t   *colormaps_BBE357;  // Slime green
static lighttable_t   *colormaps_949DB9;  // Bright desaturated blue (also 6B779E)
static lighttable_t   *colormaps_2A2F6B;  // Middle desaturated blue (also 032149)
static lighttable_t   *colormaps_50ADAC;  // Middle cyan (also 31A29F)
static lighttable_t   *colormaps_CCE4A5;  // Middle green-yellow
static lighttable_t   *colormaps_CCEA5F;  // Bright green-yellow
static lighttable_t   *colormaps_B30202;  // Middle red
static lighttable_t   *colormaps_B87A15;  // Middle orange
static lighttable_t   *colormaps_FFD000;  // Middle yellow
static lighttable_t   *colormaps_FFDE4C;  // Middle-bright yellow
static lighttable_t   *colormaps_FFF588;  // Bright yellow
static lighttable_t   *colormaps_043E8B;  // Un-darked cyanic blue
static lighttable_t   *colormaps_5B4318;  // Dark brown (also 74561F)
static lighttable_t   *colormaps_4F5D8B;  // Dark cyanic blue 2
static lighttable_t   *colormaps_D46D3D;  // Middle orange 2
static lighttable_t   *colormaps_04918B;  // Middle saturated cyan
static lighttable_t   *colormaps_FF3030;  // Bright saturated red
static lighttable_t   *colormaps_311A59;  // Un-darked magenta
static lighttable_t   *colormaps_FFAFAF;  // Brighter red (also FFCECE)
static lighttable_t   *colormaps_ECB866;  // Bright orange
static lighttable_t   *colormaps_C63F23;  // Middle orange 3
static lighttable_t   *colormaps_9BC8CD;  // Bright cyan
static lighttable_t   *colormaps_666666;  // Special green (final level)
static lighttable_t   *colormaps_777777;  // Special red (final level)

// Visplane light tables
static lighttable_t ***zlight_EEC06B = NULL;
static lighttable_t ***zlight_D97C45 = NULL;
static lighttable_t ***zlight_FF7F7F = NULL;
static lighttable_t ***zlight_55B828 = NULL;
static lighttable_t ***zlight_BBE357 = NULL;
static lighttable_t ***zlight_949DB9 = NULL;
static lighttable_t ***zlight_2A2F6B = NULL;
static lighttable_t ***zlight_50ADAC = NULL;
static lighttable_t ***zlight_CCE4A5 = NULL;
static lighttable_t ***zlight_CCEA5F = NULL;
static lighttable_t ***zlight_B30202 = NULL;
static lighttable_t ***zlight_B87A15 = NULL;
static lighttable_t ***zlight_FFD000 = NULL;
static lighttable_t ***zlight_FFDE4C = NULL;
static lighttable_t ***zlight_FFF588 = NULL;
static lighttable_t ***zlight_043E8B = NULL;
static lighttable_t ***zlight_5B4318 = NULL;
static lighttable_t ***zlight_4F5D8B = NULL;
static lighttable_t ***zlight_D46D3D = NULL;
static lighttable_t ***zlight_04918B = NULL;
static lighttable_t ***zlight_FF3030 = NULL;
static lighttable_t ***zlight_311A59 = NULL;
static lighttable_t ***zlight_FFAFAF = NULL;
static lighttable_t ***zlight_ECB866 = NULL;
static lighttable_t ***zlight_C63F23 = NULL;
static lighttable_t ***zlight_9BC8CD = NULL;
static lighttable_t ***zlight_666666 = NULL;
static lighttable_t ***zlight_777777 = NULL;

// Segment/sprite light tables
static lighttable_t ***scalelight_EEC06B = NULL;
static lighttable_t ***scalelight_D97C45 = NULL;
static lighttable_t ***scalelight_FF7F7F = NULL;
static lighttable_t ***scalelight_55B828 = NULL;
static lighttable_t ***scalelight_BBE357 = NULL;
static lighttable_t ***scalelight_949DB9 = NULL;
static lighttable_t ***scalelight_2A2F6B = NULL;
static lighttable_t ***scalelight_50ADAC = NULL;
static lighttable_t ***scalelight_CCE4A5 = NULL;
static lighttable_t ***scalelight_CCEA5F = NULL;
static lighttable_t ***scalelight_B30202 = NULL;
static lighttable_t ***scalelight_B87A15 = NULL;
static lighttable_t ***scalelight_FFD000 = NULL;
static lighttable_t ***scalelight_FFDE4C = NULL;
static lighttable_t ***scalelight_FFF588 = NULL;
static lighttable_t ***scalelight_043E8B = NULL;
static lighttable_t ***scalelight_5B4318 = NULL;
static lighttable_t ***scalelight_4F5D8B = NULL;
static lighttable_t ***scalelight_D46D3D = NULL;
static lighttable_t ***scalelight_04918B = NULL;
static lighttable_t ***scalelight_FF3030 = NULL;
static lighttable_t ***scalelight_311A59 = NULL;
static lighttable_t ***scalelight_FFAFAF = NULL;
static lighttable_t ***scalelight_ECB866 = NULL;
static lighttable_t ***scalelight_C63F23 = NULL;
static lighttable_t ***scalelight_9BC8CD = NULL;
static lighttable_t ***scalelight_666666 = NULL;
static lighttable_t ***scalelight_777777 = NULL;


// =============================================================================
//
//                             COMPOSING FUNCTIONS
//
// =============================================================================

void R_AllocateColoredColormaps (void)
{
    const size_t lighttable_size = (NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t);

    colormaps_EEC06B = malloc(lighttable_size);
    colormaps_D97C45 = malloc(lighttable_size);
    colormaps_FF7F7F = malloc(lighttable_size);
    colormaps_55B828 = malloc(lighttable_size);
    colormaps_BBE357 = malloc(lighttable_size);
    colormaps_949DB9 = malloc(lighttable_size);
    colormaps_2A2F6B = malloc(lighttable_size);
    colormaps_50ADAC = malloc(lighttable_size);
    colormaps_CCE4A5 = malloc(lighttable_size);
    colormaps_CCEA5F = malloc(lighttable_size);
    colormaps_B30202 = malloc(lighttable_size);
    colormaps_B87A15 = malloc(lighttable_size);
    colormaps_FFD000 = malloc(lighttable_size);
    colormaps_FFDE4C = malloc(lighttable_size);
    colormaps_FFF588 = malloc(lighttable_size);
    colormaps_043E8B = malloc(lighttable_size);
    colormaps_5B4318 = malloc(lighttable_size);
    colormaps_4F5D8B = malloc(lighttable_size);
    colormaps_D46D3D = malloc(lighttable_size);
    colormaps_04918B = malloc(lighttable_size);
    colormaps_FF3030 = malloc(lighttable_size);
    colormaps_311A59 = malloc(lighttable_size);
    colormaps_FFAFAF = malloc(lighttable_size);
    colormaps_ECB866 = malloc(lighttable_size);
    colormaps_C63F23 = malloc(lighttable_size);
    colormaps_9BC8CD = malloc(lighttable_size);
    colormaps_666666 = malloc(lighttable_size);
    colormaps_777777 = malloc(lighttable_size);
}

static int r_clrmp_color, g_clrmp_color, b_clrmp_color;

static void R_InitColoredColormap (const byte k, const float scale, const byte *lump_name, lighttable_t *colormap_name, const int j)
{
    r_clrmp_color = gammatable[vid_gamma][lump_name[3 * k + 0]] * (1. - scale) + gammatable[vid_gamma][0] * scale;
    g_clrmp_color = gammatable[vid_gamma][lump_name[3 * k + 1]] * (1. - scale) + gammatable[vid_gamma][0] * scale;
    b_clrmp_color = gammatable[vid_gamma][lump_name[3 * k + 2]] * (1. - scale) + gammatable[vid_gamma][0] * scale;
    
    colormap_name[j] = 0xff000000 | (r_clrmp_color << 16) | (g_clrmp_color << 8) | b_clrmp_color;
}

void R_GenerateColoredColormaps (const byte k, const float scale, const int j)
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
    R_InitColoredColormap(k, scale, C_666666, colormaps_666666, j);
    R_InitColoredColormap(k, scale, C_777777, colormaps_777777, j);
}

// =============================================================================
//
//                          INITIALIZATION FUNCTIONS
//
// =============================================================================

#define DISTMAP 2

static const int start_map[] = {
    60, 58, 56, 54, 52, 50, 48, 46, 44, 42, 40, 38, 36, 34, 32,
    30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6, 4, 2, 0, -2, 
};

void R_InitColoredLightTables (void)
{
    int i, j;
    int level, scale;
    const size_t sclight_size     = LIGHTLEVELS * sizeof(*scalelight);
    const size_t sclight_size_max = MAXLIGHTSCALE * sizeof(**scalelight);
    const size_t zlight_size      = LIGHTLEVELS * sizeof(*zlight);
    const size_t zlight_size_max  = MAXLIGHTZ * sizeof(**zlight);

    if (scalelight_EEC06B)
    {
        for (i = 0 ; i < LIGHTLEVELS ; i++)
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

    if (zlight_EEC06B)
    {
        for (i = 0; i < LIGHTLEVELS; i++)
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
    
    scalelight_EEC06B = malloc(sclight_size);
    scalelight_D97C45 = malloc(sclight_size);
    scalelight_FF7F7F = malloc(sclight_size);
    scalelight_55B828 = malloc(sclight_size);
    scalelight_BBE357 = malloc(sclight_size);
    scalelight_949DB9 = malloc(sclight_size);
    scalelight_2A2F6B = malloc(sclight_size);
    scalelight_50ADAC = malloc(sclight_size);
    scalelight_CCE4A5 = malloc(sclight_size);
    scalelight_CCEA5F = malloc(sclight_size);
    scalelight_B30202 = malloc(sclight_size);
    scalelight_B87A15 = malloc(sclight_size);
    scalelight_FFD000 = malloc(sclight_size);
    scalelight_FFDE4C = malloc(sclight_size);
    scalelight_FFF588 = malloc(sclight_size);
    scalelight_043E8B = malloc(sclight_size);
    scalelight_5B4318 = malloc(sclight_size);
    scalelight_4F5D8B = malloc(sclight_size);
    scalelight_D46D3D = malloc(sclight_size);
    scalelight_04918B = malloc(sclight_size);
    scalelight_FF3030 = malloc(sclight_size);
    scalelight_311A59 = malloc(sclight_size);
    scalelight_FFAFAF = malloc(sclight_size);
    scalelight_ECB866 = malloc(sclight_size);
    scalelight_C63F23 = malloc(sclight_size);
    scalelight_9BC8CD = malloc(sclight_size);
    scalelight_666666 = malloc(sclight_size);
    scalelight_777777 = malloc(sclight_size);

    zlight_EEC06B = malloc(zlight_size);
    zlight_D97C45 = malloc(zlight_size);
    zlight_FF7F7F = malloc(zlight_size);
    zlight_55B828 = malloc(zlight_size);
    zlight_BBE357 = malloc(zlight_size);
    zlight_949DB9 = malloc(zlight_size);
    zlight_2A2F6B = malloc(zlight_size);
    zlight_50ADAC = malloc(zlight_size);
    zlight_CCE4A5 = malloc(zlight_size);
    zlight_CCEA5F = malloc(zlight_size);
    zlight_B30202 = malloc(zlight_size);
    zlight_B87A15 = malloc(zlight_size);
    zlight_FFD000 = malloc(zlight_size);
    zlight_FFDE4C = malloc(zlight_size);
    zlight_FFF588 = malloc(zlight_size);
    zlight_043E8B = malloc(zlight_size);
    zlight_5B4318 = malloc(zlight_size);
    zlight_4F5D8B = malloc(zlight_size);
    zlight_D46D3D = malloc(zlight_size);
    zlight_04918B = malloc(zlight_size);
    zlight_FF3030 = malloc(zlight_size);
    zlight_311A59 = malloc(zlight_size);
    zlight_FFAFAF = malloc(zlight_size);
    zlight_ECB866 = malloc(zlight_size);
    zlight_C63F23 = malloc(zlight_size);
    zlight_9BC8CD = malloc(zlight_size);
    zlight_666666 = malloc(zlight_size);
    zlight_777777 = malloc(zlight_size);

    // Calculate the light levels to use for each level / distance combination.
    for (i = 0 ; i < LIGHTLEVELS ; i++)
    {
        scalelight_EEC06B[i] = malloc(sclight_size_max);
        scalelight_D97C45[i] = malloc(sclight_size_max);
        scalelight_FF7F7F[i] = malloc(sclight_size_max);
        scalelight_55B828[i] = malloc(sclight_size_max);
        scalelight_BBE357[i] = malloc(sclight_size_max);
        scalelight_949DB9[i] = malloc(sclight_size_max);
        scalelight_2A2F6B[i] = malloc(sclight_size_max);
        scalelight_50ADAC[i] = malloc(sclight_size_max);
        scalelight_CCE4A5[i] = malloc(sclight_size_max);
        scalelight_CCEA5F[i] = malloc(sclight_size_max);
        scalelight_B30202[i] = malloc(sclight_size_max);
        scalelight_B87A15[i] = malloc(sclight_size_max);
        scalelight_FFD000[i] = malloc(sclight_size_max);
        scalelight_FFDE4C[i] = malloc(sclight_size_max);
        scalelight_FFF588[i] = malloc(sclight_size_max);
        scalelight_043E8B[i] = malloc(sclight_size_max);
        scalelight_5B4318[i] = malloc(sclight_size_max);
        scalelight_4F5D8B[i] = malloc(sclight_size_max);
        scalelight_D46D3D[i] = malloc(sclight_size_max);
        scalelight_04918B[i] = malloc(sclight_size_max);
        scalelight_FF3030[i] = malloc(sclight_size_max);
        scalelight_311A59[i] = malloc(sclight_size_max);
        scalelight_FFAFAF[i] = malloc(sclight_size_max);
        scalelight_ECB866[i] = malloc(sclight_size_max);
        scalelight_C63F23[i] = malloc(sclight_size_max);
        scalelight_9BC8CD[i] = malloc(sclight_size_max);
        scalelight_666666[i] = malloc(sclight_size_max);
        scalelight_777777[i] = malloc(sclight_size_max);

        zlight_EEC06B[i] = malloc(zlight_size_max);
        zlight_D97C45[i] = malloc(zlight_size_max);
        zlight_FF7F7F[i] = malloc(zlight_size_max);
        zlight_55B828[i] = malloc(zlight_size_max);
        zlight_BBE357[i] = malloc(zlight_size_max);
        zlight_949DB9[i] = malloc(zlight_size_max);
        zlight_2A2F6B[i] = malloc(zlight_size_max);
        zlight_50ADAC[i] = malloc(zlight_size_max);
        zlight_CCE4A5[i] = malloc(zlight_size_max);
        zlight_CCEA5F[i] = malloc(zlight_size_max);
        zlight_B30202[i] = malloc(zlight_size_max);
        zlight_B87A15[i] = malloc(zlight_size_max);
        zlight_FFD000[i] = malloc(zlight_size_max);
        zlight_FFDE4C[i] = malloc(zlight_size_max);
        zlight_FFF588[i] = malloc(zlight_size_max);
        zlight_043E8B[i] = malloc(zlight_size_max);
        zlight_5B4318[i] = malloc(zlight_size_max);
        zlight_4F5D8B[i] = malloc(zlight_size_max);
        zlight_D46D3D[i] = malloc(zlight_size_max);
        zlight_04918B[i] = malloc(zlight_size_max);
        zlight_FF3030[i] = malloc(zlight_size_max);
        zlight_311A59[i] = malloc(zlight_size_max);
        zlight_FFAFAF[i] = malloc(zlight_size_max);
        zlight_ECB866[i] = malloc(zlight_size_max);
        zlight_C63F23[i] = malloc(zlight_size_max);
        zlight_9BC8CD[i] = malloc(zlight_size_max);
        zlight_666666[i] = malloc(zlight_size_max);
        zlight_777777[i] = malloc(zlight_size_max);

        for (j = 0 ; j < MAXLIGHTZ ; j++)
        {
            scale = (FixedDiv ((ORIGWIDTH / 2 * FRACUNIT), (j + 1) << LIGHTZSHIFT)) >> LIGHTSCALESHIFT;
            level = BETWEEN(0, NUMCOLORMAPS - 1, start_map[i] - scale / DISTMAP) * 256;

            zlight_EEC06B[i][j] = colormaps_EEC06B + level;
            zlight_D97C45[i][j] = colormaps_D97C45 + level;
            zlight_FF7F7F[i][j] = colormaps_FF7F7F + level;
            zlight_55B828[i][j] = colormaps_55B828 + level;
            zlight_BBE357[i][j] = colormaps_BBE357 + level;
            zlight_949DB9[i][j] = colormaps_949DB9 + level;
            zlight_2A2F6B[i][j] = colormaps_2A2F6B + level;
            zlight_50ADAC[i][j] = colormaps_50ADAC + level;
            zlight_CCE4A5[i][j] = colormaps_CCE4A5 + level;
            zlight_CCEA5F[i][j] = colormaps_CCEA5F + level;
            zlight_B30202[i][j] = colormaps_B30202 + level;
            zlight_B87A15[i][j] = colormaps_B87A15 + level;
            zlight_FFD000[i][j] = colormaps_FFD000 + level;
            zlight_FFDE4C[i][j] = colormaps_FFDE4C + level;
            zlight_FFF588[i][j] = colormaps_FFF588 + level;
            zlight_043E8B[i][j] = colormaps_043E8B + level;
            zlight_5B4318[i][j] = colormaps_5B4318 + level;
            zlight_4F5D8B[i][j] = colormaps_4F5D8B + level;
            zlight_D46D3D[i][j] = colormaps_D46D3D + level;
            zlight_04918B[i][j] = colormaps_04918B + level;
            zlight_FF3030[i][j] = colormaps_FF3030 + level;
            zlight_311A59[i][j] = colormaps_311A59 + level;
            zlight_FFAFAF[i][j] = colormaps_FFAFAF + level;
            zlight_ECB866[i][j] = colormaps_ECB866 + level;
            zlight_C63F23[i][j] = colormaps_C63F23 + level;
            zlight_9BC8CD[i][j] = colormaps_9BC8CD + level;
            zlight_666666[i][j] = colormaps_666666 + level;
            zlight_777777[i][j] = colormaps_777777 + level;
        }
    }
}

void R_GenerateColoredSClights (const int width)
{
    int i, j;
    int level; 

    // Calculate the light levels to use for each level / scale combination.
    for (i = 0 ; i < LIGHTLEVELS ; i++)
    {
        for (j = 0 ; j < MAXLIGHTSCALE ; j++)
        {
            level = BETWEEN(0, NUMCOLORMAPS-1, start_map[i] - j * NONWIDEWIDTH / (width << detailshift) / DISTMAP) * 256;

            scalelight_EEC06B[i][j] = colormaps_EEC06B + level;
            scalelight_D97C45[i][j] = colormaps_D97C45 + level;
            scalelight_FF7F7F[i][j] = colormaps_FF7F7F + level;
            scalelight_55B828[i][j] = colormaps_55B828 + level;
            scalelight_BBE357[i][j] = colormaps_BBE357 + level;
            scalelight_949DB9[i][j] = colormaps_949DB9 + level;
            scalelight_2A2F6B[i][j] = colormaps_2A2F6B + level;
            scalelight_50ADAC[i][j] = colormaps_50ADAC + level;
            scalelight_CCE4A5[i][j] = colormaps_CCE4A5 + level;
            scalelight_CCEA5F[i][j] = colormaps_CCEA5F + level;
            scalelight_B30202[i][j] = colormaps_B30202 + level;
            scalelight_B87A15[i][j] = colormaps_B87A15 + level;
            scalelight_FFD000[i][j] = colormaps_FFD000 + level;
            scalelight_FFDE4C[i][j] = colormaps_FFDE4C + level;
            scalelight_FFF588[i][j] = colormaps_FFF588 + level;
            scalelight_043E8B[i][j] = colormaps_043E8B + level;
            scalelight_5B4318[i][j] = colormaps_5B4318 + level;
            scalelight_4F5D8B[i][j] = colormaps_4F5D8B + level;
            scalelight_D46D3D[i][j] = colormaps_D46D3D + level;
            scalelight_04918B[i][j] = colormaps_04918B + level;
            scalelight_FF3030[i][j] = colormaps_FF3030 + level;
            scalelight_311A59[i][j] = colormaps_311A59 + level;
            scalelight_FFAFAF[i][j] = colormaps_FFAFAF + level;
            scalelight_ECB866[i][j] = colormaps_ECB866 + level;
            scalelight_C63F23[i][j] = colormaps_C63F23 + level;
            scalelight_9BC8CD[i][j] = colormaps_9BC8CD + level;
            scalelight_666666[i][j] = colormaps_666666 + level;
            scalelight_777777[i][j] = colormaps_777777 + level;
        }
    }
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
    if (vis_colored_lighting)
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
        }
    }

    return colormaps;
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
    {    5,      8,    0x4F5D8B },
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
    {    7,     16,    0x4F5D8B },
    {    7,     17,    0x55B828 },
    {    7,     18,    0xBBE357 },
    {    7,     20,    0xBBE357 },
    {    7,     21,    0xBBE357 },
    {    7,     23,    0xBBE357 },
    {    7,     24,    0xBBE357 },
    {    7,     25,    0xBBE357 },
    {    7,     27,    0x55B828 },
    {    7,     29,    0xBBE357 },
    {    7,     28,    0x55B828 },
    {    7,     30,    0x55B828 },
    {    7,     31,    0x55B828 },
    {    7,     35,    0xFFF588 },
    {    7,     38,    0xFFF588 },
    {    7,     47,    0xFFF588 },
    {    7,     48,    0xFFF588 },
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
    {    7,    134,    0x311A59 },
    {    7,    136,    0x311A59 },
    {    7,    137,    0x311A59 },
    {    7,    140,    0xFF7F7F },
    {    7,    141,    0xFF7F7F },
    {    7,    142,    0xFFF588 },
    {    7,    143,    0xFFF588 },
    {    7,    144,    0x043E8B },
    {    7,    145,    0x043E8B },
    {    7,    146,    0xFFF588 },
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
    {    8,     16,    0xFFAFAF },
    {    8,     30,    0x043E8B },
    {    8,     51,    0xB30202 },
    {    8,     52,    0x4F5D8B },
    {    8,     53,    0xB30202 },
    {    8,     70,    0xB30202 },
    {    8,     76,    0xB30202 },
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
    {   10,      5,    0xFFAFAF },
    {   10,      7,    0xFFAFAF },
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
    {   10,     70,    0xFFF588 },
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
    {   11,     69,    0xEEC06B },
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
    {   12,     32,    0xFF7F7F },
    {   12,     33,    0xFF7F7F },
    {   12,     34,    0xFF7F7F },
    {   12,     35,    0xFF7F7F },
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
    {   12,    138,    0xFFF588 },
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
    {   13,     90,    0xFF7F7F },
    {   13,     91,    0xD97C45 },
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
    {   14,     85,    0xFF7F7F },
    {   14,     89,    0x50ADAC },
    {   14,     90,    0x50ADAC },
    {   14,     91,    0x50ADAC },
    {   14,     92,    0x50ADAC },
    {   14,     93,    0xFFAFAF },
    {   14,     96,    0x50ADAC },
    {   14,     97,    0x50ADAC },
    {   14,     99,    0xFFAFAF },
    {   14,    100,    0xFFAFAF },
    {   14,    103,    0xFFAFAF },
    {   14,    104,    0xFFAFAF },
    {   14,    105,    0x50ADAC },
    {   14,    109,    0xFFAFAF },
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
    {   18,     69,    0xFFAFAF },
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
    {   18,     94,    0xFFF588 },
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
    {   19,     34,    0xFFAFAF },
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
    {   21,     62,    0xFF7F7F },
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
    {   22,     76,    0xB30202 },
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
    {   22,    128,    0xFF3030 },
    {   22,    129,    0x043E8B },
    {   22,    130,    0x043E8B },
    {   22,    131,    0xFF3030 },
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
    {   24,    133,    0xFF7F7F },
    {   24,    136,    0xFF7F7F },
    {   24,    145,    0x043E8B },
    {   24,    146,    0xFF7F7F },
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
    0,0,0,2,0,0,0,0,0,41,36,19,238,192,107,2,
    2,1,0,0,0,0,0,0,0,0,0,15,15,5,7,8,
    1,0,2,0,0,0,0,37,23,9,29,17,6,24,12,4,
    238,181,97,238,167,89,238,158,85,238,142,76,236,132,70,225,
    120,64,217,106,57,205,96,51,194,87,46,185,78,42,173,69,
    37,167,65,34,155,55,29,149,49,26,135,40,21,131,38,20,
    119,31,16,112,28,15,102,23,13,96,18,10,84,15,8,79,
    13,7,71,10,5,65,8,4,58,5,3,53,2,1,46,0,
    0,42,0,0,36,0,0,32,0,0,25,0,0,24,0,0,
    238,192,107,238,192,107,238,192,107,238,192,102,237,192,98,237,
    192,91,238,181,83,238,177,79,237,166,70,238,156,63,235,145,
    57,221,134,52,210,123,47,199,112,41,187,102,37,180,96,34,
    163,90,31,147,81,28,138,75,26,125,69,24,116,62,21,98,
    55,19,90,47,16,80,44,14,72,40,13,58,35,11,46,29,
    9,36,23,7,30,18,5,22,14,4,12,9,1,8,6,1,
    238,192,107,238,192,107,238,192,107,238,192,107,231,192,107,219,
    192,102,212,186,99,200,175,94,189,165,88,183,160,86,169,148,
    79,163,143,76,152,133,71,140,123,65,133,117,62,121,106,57,
    109,96,51,104,90,48,91,81,43,81,72,38,77,67,36,66,
    58,31,58,51,27,53,47,25,46,40,21,38,34,18,34,29,
    16,26,23,12,24,21,11,17,15,8,11,10,5,8,8,4,
    124,192,55,109,192,48,97,187,42,84,170,37,76,148,31,64,
    131,26,53,110,21,46,96,18,39,78,13,30,62,11,23,47,
    8,14,35,5,8,23,3,1,13,0,0,5,0,0,0,0,
    183,142,67,170,132,61,159,120,55,147,110,51,133,99,45,129,
    92,42,116,86,38,104,75,33,95,67,29,83,58,25,78,54,
    23,68,47,20,61,43,18,51,36,14,45,29,12,40,26,10,
    128,95,40,104,77,31,91,66,26,75,51,20,55,40,15,44,
    30,11,35,22,7,25,16,5,89,81,37,74,67,29,63,58,
    25,53,50,21,43,38,16,35,32,13,27,26,9,21,21,7,
    221,192,69,200,167,53,170,134,40,146,103,28,120,75,18,100,
    49,10,77,30,4,62,15,0,238,192,107,238,192,107,238,186,
    99,238,149,80,238,116,62,238,87,46,238,55,29,238,32,17,
    231,13,7,209,11,6,195,10,5,181,8,5,163,8,4,150,
    7,4,133,5,3,120,3,2,105,2,1,86,0,0,75,0,
    0,63,0,0,50,0,0,41,0,0,32,0,0,24,0,0,
    238,192,107,226,192,107,191,168,107,156,137,107,121,106,107,84,
    74,107,56,50,107,33,29,107,14,13,107,11,10,91,8,8,
    76,6,5,63,2,2,49,0,0,37,0,0,26,0,0,16,
    238,192,107,238,192,107,238,192,102,235,186,85,235,165,67,233,
    143,48,235,117,32,228,97,20,213,87,16,201,77,13,189,71,
    12,176,63,10,162,53,8,150,45,5,135,38,4,125,35,4,
    238,192,107,238,192,107,235,192,98,228,192,81,219,192,65,212,
    186,50,202,178,37,188,167,26,118,32,2,107,26,1,95,22,
    0,81,14,0,37,23,8,25,15,5,15,8,1,12,2,0,
    0,0,16,0,0,12,0,0,8,0,0,5,0,0,2,0,
    0,0,0,0,0,0,0,0,231,133,38,213,176,52,238,116,
    107,231,13,106,169,8,78,109,2,49,58,0,26,147,79,42
};

const byte C_D97C45[] = {
    0,0,0,2,0,0,0,0,0,37,23,12,217,124,69,2,
    1,1,0,0,0,0,0,0,0,0,0,14,10,3,6,5,
    1,0,1,0,0,0,0,34,15,6,26,11,4,22,8,3,
    217,117,62,217,108,57,217,102,55,217,91,49,215,85,45,205,
    77,41,198,69,37,187,62,33,177,56,30,168,51,27,157,44,
    24,152,42,22,141,35,19,136,32,17,123,26,14,119,24,13,
    109,20,11,102,18,9,93,15,8,88,12,6,77,10,5,72,
    8,4,65,6,4,60,5,3,53,3,2,49,1,1,42,0,
    0,38,0,0,33,0,0,29,0,0,23,0,0,22,0,0,
    217,124,69,217,124,69,217,124,69,217,124,66,216,124,63,216,
    124,59,217,117,54,217,114,51,216,107,45,217,101,41,214,93,
    37,202,87,34,191,80,30,181,72,26,170,66,24,164,62,22,
    149,58,20,134,52,18,126,49,17,114,45,15,106,40,14,89,
    35,12,82,31,10,73,28,9,66,26,8,53,22,7,42,18,
    6,33,15,5,27,12,4,20,9,2,11,6,1,8,4,1,
    217,124,69,217,124,69,217,124,69,217,124,69,211,124,69,200,
    124,66,193,120,64,182,113,60,172,106,57,167,104,55,154,96,
    51,149,92,49,139,86,46,128,79,42,121,75,40,111,69,37,
    100,62,33,94,58,31,83,52,28,74,46,25,70,43,23,60,
    37,20,53,33,18,49,30,16,42,26,14,35,22,12,31,19,
    10,24,15,8,22,14,7,15,10,5,10,6,4,8,5,3,
    113,124,35,100,124,31,89,121,27,77,110,24,69,96,20,59,
    85,17,49,71,14,42,62,12,36,51,8,27,40,7,21,31,
    5,13,22,3,8,15,2,1,8,0,0,3,0,0,0,0,
    167,92,43,155,85,40,145,77,36,134,71,33,121,64,29,117,
    59,27,106,55,24,94,48,21,87,43,19,76,37,16,71,35,
    15,62,30,13,55,28,12,47,23,9,41,19,8,37,17,7,
    117,61,26,94,50,20,83,42,17,68,33,13,50,26,9,40,
    19,7,32,14,5,23,10,3,81,52,24,67,43,19,58,37,
    16,49,32,14,39,25,10,31,21,8,25,17,6,20,14,5,
    202,124,44,182,108,34,155,87,26,133,67,18,110,48,12,91,
    32,6,71,19,2,56,10,0,217,124,69,217,124,69,217,120,
    64,217,96,51,217,75,40,217,56,30,217,35,19,217,20,11,
    210,8,4,191,7,4,178,6,4,165,5,3,149,5,3,137,
    4,2,122,3,2,110,2,1,96,1,1,78,0,0,68,0,
    0,57,0,0,46,0,0,37,0,0,29,0,0,22,0,0,
    217,124,69,206,124,69,174,108,69,142,89,69,111,69,69,77,
    48,69,51,32,69,30,18,69,13,8,69,10,6,59,8,5,
    49,5,3,40,2,1,32,0,0,24,0,0,17,0,0,10,
    217,124,69,217,124,69,217,124,66,214,120,55,214,106,43,213,
    92,31,214,76,21,208,63,13,194,56,11,183,50,8,172,46,
    8,161,41,7,148,35,5,137,29,4,123,25,3,114,23,2,
    217,124,69,217,124,69,214,124,63,208,124,52,200,124,42,193,
    120,32,184,115,24,171,108,17,107,20,1,98,17,1,87,14,
    0,74,9,0,34,15,5,23,10,3,14,5,1,11,1,0,
    0,0,10,0,0,8,0,0,5,0,0,4,0,0,1,0,
    0,0,0,0,0,0,0,0,211,86,24,194,114,33,217,75,
    69,210,8,68,154,5,51,100,1,31,53,0,17,134,51,27
};

const byte C_FF7F7F[] = {
    0,0,0,2,0,0,0,0,0,44,24,23,255,127,127,2,
    1,1,0,0,0,0,0,0,0,0,0,16,10,6,7,5,
    1,0,1,0,0,0,0,40,15,10,31,11,7,26,8,5,
    255,120,115,255,111,106,255,105,101,255,94,90,253,87,84,241,
    79,76,233,70,68,220,63,61,208,57,55,198,52,49,185,45,
    43,179,43,41,166,36,35,160,32,31,145,26,25,140,25,24,
    128,20,19,120,18,17,109,15,15,103,12,11,90,10,9,85,
    8,8,76,6,6,70,5,5,62,3,3,57,1,1,49,0,
    0,45,0,0,39,0,0,34,0,0,27,0,0,26,0,0,
    255,127,127,255,127,127,255,127,127,255,127,122,254,127,116,254,
    127,108,255,120,99,255,117,94,254,110,83,255,103,75,252,96,
    68,237,89,62,225,82,55,213,74,48,200,67,43,193,64,40,
    175,60,37,158,53,33,148,50,31,134,46,28,124,41,25,105,
    36,22,96,31,19,86,29,17,77,26,15,62,23,13,49,19,
    10,39,15,8,32,12,6,24,9,4,13,6,1,9,4,1,
    255,127,127,255,127,127,255,127,127,255,127,127,248,127,127,235,
    127,122,227,123,118,214,116,111,202,109,105,196,106,102,181,98,
    94,175,95,91,163,88,85,150,81,78,142,77,74,130,70,68,
    117,63,61,111,60,57,98,53,51,87,47,45,82,44,42,71,
    38,37,62,34,32,57,31,30,49,26,25,41,22,21,36,19,
    18,28,15,14,26,14,13,18,10,9,12,6,6,9,5,5,
    133,127,65,117,127,57,104,124,50,90,113,43,81,98,36,69,
    87,31,57,73,25,49,63,21,42,52,15,32,41,13,25,31,
    9,15,23,5,9,15,3,1,8,0,0,3,0,0,0,0,
    196,94,80,182,87,73,170,79,66,157,73,60,142,65,53,138,
    61,49,124,57,45,111,49,39,102,44,34,89,38,30,84,36,
    27,73,31,23,65,28,21,55,24,16,48,19,14,43,17,12,
    137,63,47,111,51,37,97,43,30,80,34,24,59,26,17,47,
    20,13,38,14,8,27,10,6,95,53,43,79,44,35,68,38,
    30,57,33,25,46,25,18,37,21,15,29,17,11,23,14,8,
    237,127,82,214,111,63,182,89,48,156,68,33,129,49,21,107,
    32,11,83,20,4,66,10,0,255,127,127,255,127,127,255,123,
    118,255,99,95,255,77,73,255,57,55,255,36,35,255,21,20,
    247,8,8,224,7,7,209,6,6,194,5,5,175,5,5,161,
    4,4,143,3,3,129,2,2,113,1,1,92,0,0,80,0,
    0,67,0,0,54,0,0,44,0,0,34,0,0,26,0,0,
    255,127,127,242,127,127,205,111,127,167,91,127,130,70,127,90,
    49,127,60,33,127,35,19,127,15,8,127,12,6,109,9,5,
    91,6,3,74,2,1,58,0,0,43,0,0,30,0,0,19,
    255,127,127,255,127,127,255,127,122,252,123,101,252,109,79,250,
    95,57,252,78,38,244,64,23,228,57,19,215,51,15,202,47,
    14,189,42,12,174,35,9,161,30,6,145,25,5,134,23,4,
    255,127,127,255,127,127,252,127,117,244,127,97,235,127,78,227,
    123,59,216,118,43,201,111,30,126,21,2,115,17,1,102,14,
    0,87,9,0,40,15,10,27,10,5,16,5,1,13,1,0,
    0,0,19,0,0,14,0,0,9,0,0,6,0,0,2,0,
    0,0,0,0,0,0,0,0,248,88,45,228,117,61,255,77,
    127,247,8,126,181,5,93,117,1,58,62,0,30,157,52,50
};

const byte C_55B828[] = {
    0,0,0,1,0,0,0,0,0,15,35,7,85,184,40,1,
    1,0,0,0,0,0,0,0,0,0,0,5,14,2,2,7,
    0,0,1,0,0,0,0,13,22,3,10,17,2,9,12,2,
    85,174,36,85,160,33,85,152,32,85,136,28,84,126,26,80,
    115,24,78,102,21,73,92,19,69,83,17,66,75,16,62,66,
    14,60,62,13,55,53,11,53,47,10,48,38,8,47,36,8,
    43,30,6,40,27,5,36,22,5,34,17,4,30,14,3,28,
    12,3,25,9,2,23,7,2,21,5,1,19,1,0,16,0,
    0,15,0,0,13,0,0,11,0,0,9,0,0,9,0,0,
    85,184,40,85,184,40,85,184,40,85,184,38,85,184,37,85,
    184,34,85,174,31,85,170,30,85,159,26,85,149,24,84,139,
    21,79,128,19,75,118,17,71,108,15,67,97,14,64,92,13,
    58,87,12,53,77,11,49,72,10,45,66,9,41,59,8,35,
    53,7,32,45,6,29,42,5,26,38,5,21,33,4,16,27,
    3,13,22,3,11,17,2,8,13,1,4,9,0,3,6,0,
    85,184,40,85,184,40,85,184,40,85,184,40,83,184,40,78,
    184,38,76,178,37,71,168,35,67,158,33,65,154,32,60,142,
    30,58,137,29,54,128,27,50,118,24,47,112,23,43,102,21,
    39,92,19,37,87,18,33,77,16,29,69,14,27,64,13,24,
    56,12,21,49,10,19,45,9,16,38,8,14,32,7,12,28,
    6,9,22,5,9,20,4,6,14,3,4,9,2,3,7,2,
    44,184,21,39,184,18,35,179,16,30,163,14,27,142,11,23,
    126,10,19,105,8,16,92,7,14,75,5,11,59,4,8,45,
    3,5,33,2,3,22,1,0,12,0,0,5,0,0,0,0,
    65,136,25,61,126,23,57,115,21,52,105,19,47,95,17,46,
    88,16,41,82,14,37,71,12,34,64,11,30,56,9,28,52,
    9,24,45,7,22,41,7,18,35,5,16,28,4,14,25,4,
    46,91,15,37,74,12,32,63,10,27,49,8,20,38,5,16,
    29,4,13,21,3,9,15,2,32,77,14,26,64,11,23,56,
    9,19,48,8,15,37,6,12,31,5,10,25,3,8,20,3,
    79,184,26,71,160,20,61,128,15,52,99,10,43,71,7,36,
    47,4,28,29,1,22,14,0,85,184,40,85,184,40,85,178,
    37,85,143,30,85,111,23,85,83,17,85,53,11,85,30,6,
    82,12,3,75,10,2,70,9,2,65,8,2,58,7,2,54,
    6,1,48,5,1,43,3,1,38,1,0,31,0,0,27,0,
    0,22,0,0,18,0,0,15,0,0,11,0,0,9,0,0,
    85,184,40,81,184,40,68,161,40,56,131,40,43,102,40,30,
    71,40,20,48,40,12,27,40,5,12,40,4,9,34,3,7,
    29,2,5,23,1,1,18,0,0,14,0,0,10,0,0,6,
    85,184,40,85,184,40,85,184,38,84,178,32,84,158,25,83,
    137,18,84,113,12,81,93,7,76,83,6,72,74,5,67,68,
    5,63,61,4,58,51,3,54,43,2,48,37,2,45,34,1,
    85,184,40,85,184,40,84,184,37,81,184,30,78,184,24,76,
    178,19,72,170,14,67,160,10,42,30,1,38,25,0,34,21,
    0,29,14,0,13,22,3,9,14,2,5,7,0,4,1,0,
    0,0,6,0,0,4,0,0,3,0,0,2,0,0,1,0,
    0,0,0,0,0,0,0,0,83,128,14,76,169,19,85,111,
    40,82,12,40,60,8,29,39,1,18,21,0,10,52,76,16
};

const byte C_BBE357[] = {
    0,0,0,1,0,0,0,0,0,32,43,16,187,227,87,1,
    2,1,0,0,0,0,0,0,0,0,0,12,18,4,5,9,
    1,0,2,0,0,0,0,29,27,7,23,20,5,19,14,3,
    187,215,78,187,198,72,187,187,69,187,167,61,186,156,57,177,
    142,52,171,126,46,161,113,42,153,102,38,145,93,34,136,81,
    30,131,77,28,122,65,24,117,58,21,106,47,17,103,45,16,
    94,36,13,88,33,12,80,28,10,76,21,8,66,18,6,62,
    15,5,56,12,4,51,9,3,45,6,2,42,2,1,36,0,
    0,33,0,0,29,0,0,25,0,0,20,0,0,19,0,0,
    187,227,87,187,227,87,187,227,87,187,227,83,186,227,79,186,
    227,74,187,215,68,187,209,64,186,196,57,187,184,51,185,171,
    47,174,158,42,165,146,38,156,133,33,147,120,30,142,114,28,
    128,107,26,116,95,23,109,89,21,98,82,19,91,73,17,77,
    65,15,70,56,13,63,52,12,56,47,11,45,41,9,36,34,
    7,29,28,6,23,21,4,18,16,3,10,11,1,7,7,1,
    187,227,87,187,227,87,187,227,87,187,227,87,182,227,87,172,
    227,83,166,220,81,157,207,76,148,195,72,144,190,70,133,175,
    64,128,169,62,120,158,58,110,145,53,104,138,50,95,126,46,
    86,113,42,81,107,39,72,95,35,64,85,31,60,79,29,52,
    69,25,45,61,22,42,55,20,36,47,17,30,40,15,26,35,
    13,21,27,10,19,25,9,13,18,6,9,12,4,7,9,3,
    98,227,45,86,227,39,76,221,34,66,201,30,59,175,25,51,
    155,21,42,130,17,36,113,15,31,93,11,23,73,9,18,56,
    6,11,41,4,7,27,2,1,15,0,0,6,0,0,0,0,
    144,168,55,133,156,50,125,142,45,115,130,41,104,117,37,101,
    109,34,91,101,31,81,88,27,75,79,24,65,69,20,62,64,
    19,54,55,16,48,51,15,40,43,11,35,35,10,32,31,9,
    100,112,32,81,91,25,71,77,21,59,61,16,43,47,12,34,
    36,9,28,26,6,20,19,4,70,95,30,58,79,24,50,69,
    20,42,59,17,34,45,13,27,38,10,21,31,8,17,25,6,
    174,227,56,157,198,43,133,158,33,114,122,23,95,88,15,78,
    58,8,61,36,3,48,18,0,187,227,87,187,227,87,187,220,
    81,187,176,65,187,137,50,187,102,38,187,65,24,187,37,14,
    181,15,5,164,12,5,153,12,4,142,10,4,128,9,3,118,
    8,3,105,6,2,95,4,1,83,2,1,67,0,0,59,0,
    0,49,0,0,40,0,0,32,0,0,25,0,0,19,0,0,
    187,227,87,177,227,87,150,199,87,122,162,87,95,126,87,66,
    87,87,44,59,87,26,34,87,11,15,87,9,12,74,7,9,
    62,4,6,51,1,2,40,0,0,30,0,0,21,0,0,13,
    187,227,87,187,227,87,187,227,83,185,220,69,185,195,54,183,
    169,39,185,139,26,179,115,16,167,102,13,158,91,11,148,84,
    10,139,75,9,128,63,6,118,53,4,106,45,3,98,42,3,
    187,227,87,187,227,87,185,227,80,179,227,66,172,227,53,166,
    220,41,158,210,30,147,198,21,92,37,2,84,31,1,75,26,
    0,64,17,0,29,27,7,20,18,4,12,9,1,10,2,0,
    0,0,13,0,0,10,0,0,6,0,0,4,0,0,1,0,
    0,0,0,0,0,0,0,0,182,158,31,167,208,42,187,137,
    87,181,15,86,133,10,64,86,2,40,45,0,21,115,93,34
};

const byte C_949DB9[] = {
    0,0,0,1,0,0,0,0,0,26,30,33,148,157,185,1,
    1,1,0,0,0,0,0,0,0,0,0,9,12,9,4,6,
    1,0,1,0,0,0,0,23,18,15,18,14,10,15,10,7,
    148,148,167,148,137,154,148,129,147,148,116,131,147,108,122,140,
    98,111,135,87,99,128,78,89,121,71,80,115,64,72,107,56,
    63,104,53,59,96,45,51,93,40,45,84,33,37,81,31,35,
    74,25,28,70,23,25,63,19,22,60,15,17,52,12,14,49,
    10,12,44,8,9,41,6,7,36,4,4,33,1,1,28,0,
    0,26,0,0,23,0,0,20,0,0,16,0,0,15,0,0,
    148,157,185,148,157,185,148,157,185,148,157,177,147,157,169,147,
    157,157,148,148,144,148,145,137,147,135,120,148,127,109,146,118,
    99,138,110,90,131,101,81,124,92,70,116,83,63,112,79,59,
    102,74,54,92,66,49,86,62,45,78,57,41,72,50,36,61,
    45,33,56,39,28,50,36,25,45,33,22,36,28,19,28,23,
    15,23,19,12,19,15,9,14,11,7,8,7,2,5,5,1,
    148,157,185,148,157,185,148,157,185,148,157,185,144,157,185,136,
    157,177,132,152,172,124,143,162,117,135,152,114,131,148,105,121,
    137,102,117,132,95,109,123,87,100,113,82,95,107,75,87,99,
    68,78,89,64,74,83,57,66,74,50,58,66,48,55,62,41,
    47,54,36,42,47,33,38,44,28,33,37,24,28,31,21,24,
    27,16,18,21,15,17,20,10,12,14,7,8,9,5,6,7,
    77,157,95,68,157,83,60,153,73,52,139,63,47,121,53,40,
    107,45,33,90,37,28,78,31,24,64,22,19,50,20,15,39,
    13,9,28,8,5,18,4,1,10,0,0,4,0,0,0,0,
    114,116,116,106,108,106,99,98,96,91,90,88,82,81,78,80,
    75,72,72,70,65,64,61,57,59,55,50,52,47,44,49,44,
    40,42,38,34,38,35,31,32,30,24,28,24,20,25,22,18,
    80,78,69,64,63,54,56,54,44,46,42,35,34,33,25,27,
    25,20,22,18,12,16,13,9,55,66,63,46,55,51,39,47,
    44,33,41,37,27,31,27,21,26,22,17,22,16,13,17,12,
    138,157,119,124,137,91,106,110,70,91,84,48,75,61,31,62,
    40,17,48,25,7,38,12,1,148,157,185,148,157,185,148,152,
    172,148,122,138,148,95,107,148,71,80,148,45,51,148,26,30,
    143,10,12,130,9,10,121,8,9,113,7,8,102,6,7,93,
    6,7,83,4,4,75,2,3,66,1,1,53,0,0,46,0,
    0,39,0,0,31,0,0,26,0,0,20,0,0,15,0,0,
    148,157,185,140,157,185,119,137,185,97,112,185,75,87,185,52,
    60,185,35,41,185,20,23,185,9,10,185,7,8,158,5,6,
    132,3,4,108,1,1,85,0,0,63,0,0,44,0,0,28,
    148,157,185,148,157,185,148,157,177,146,152,147,146,135,115,145,
    117,83,146,96,55,142,79,34,132,71,28,125,63,22,117,58,
    21,110,52,18,101,44,13,93,37,9,84,31,7,78,29,7,
    148,157,185,148,157,185,146,157,170,142,157,141,136,157,113,132,
    152,86,125,145,63,117,137,44,73,26,4,67,22,2,59,18,
    1,50,12,0,23,18,15,16,12,8,9,6,1,8,1,0,
    0,0,28,0,0,20,0,0,14,0,0,9,0,0,3,0,
    0,0,0,0,0,0,0,0,144,109,65,132,144,89,148,95,
    185,143,10,183,105,7,136,68,1,84,36,0,44,91,65,73
};

const byte C_2A2F6B[] = {
    0,0,0,0,0,0,0,0,0,9,10,44,51,51,242,0,
    0,2,0,0,0,0,0,0,0,0,0,3,4,11,1,2,
    2,0,0,0,0,0,0,8,6,20,6,5,13,5,3,9,
    51,48,218,51,44,201,51,42,192,51,38,171,51,35,159,48,
    32,145,47,28,129,44,25,116,42,23,104,40,21,94,37,18,
    83,36,17,78,33,15,66,32,13,59,29,11,48,28,10,46,
    26,8,37,24,7,33,22,6,28,21,5,22,18,4,18,17,
    3,15,15,3,12,14,2,9,12,1,6,11,0,2,10,0,
    0,9,0,0,8,0,0,7,0,0,5,0,0,5,0,0,
    51,51,242,51,51,242,51,51,242,51,51,232,51,51,221,51,
    51,206,51,48,188,51,47,179,51,44,158,51,41,142,50,38,
    130,47,36,118,45,33,105,43,30,92,40,27,83,39,26,77,
    35,24,71,32,21,64,30,20,59,27,18,54,25,16,47,21,
    15,43,19,13,36,17,12,32,15,11,29,12,9,25,10,8,
    20,8,6,16,6,5,12,5,4,9,3,2,3,2,2,2,
    51,51,242,51,51,242,51,51,242,51,51,242,50,51,242,47,
    51,232,45,49,225,43,47,212,40,44,199,39,43,194,36,39,
    179,35,38,173,33,35,161,30,33,148,28,31,140,26,28,129,
    23,25,116,22,24,109,20,21,97,17,19,86,16,18,81,14,
    15,70,12,14,62,11,12,57,10,11,48,8,9,41,7,8,
    35,6,6,28,5,6,26,4,4,18,2,3,12,2,2,9,
    27,51,124,23,51,108,21,50,95,18,45,83,16,39,69,14,
    35,59,11,29,48,10,25,41,8,21,29,6,16,26,5,13,
    17,3,9,10,2,6,6,0,3,0,0,1,0,0,0,0,
    39,38,152,36,35,139,34,32,125,31,29,115,28,26,102,28,
    24,94,25,23,85,22,20,74,20,18,65,18,15,57,17,14,
    52,15,12,45,13,11,41,11,10,31,10,8,27,9,7,24,
    27,25,90,22,20,70,19,17,58,16,14,46,12,11,33,9,
    8,26,8,6,16,5,4,11,19,21,83,16,18,66,14,15,
    57,11,13,48,9,10,35,7,9,28,6,7,21,5,6,16,
    47,51,156,43,44,120,36,36,91,31,27,63,26,20,41,21,
    13,22,17,8,9,13,4,1,51,51,242,51,51,242,51,49,
    225,51,40,180,51,31,140,51,23,104,51,15,66,51,8,39,
    49,3,15,45,3,13,42,3,12,39,2,10,35,2,9,32,
    2,9,29,1,6,26,1,4,23,0,2,18,0,0,16,0,
    0,13,0,0,11,0,0,9,0,0,7,0,0,5,0,0,
    51,51,242,48,51,242,41,45,242,33,36,242,26,28,242,18,
    20,242,12,13,242,7,8,242,3,3,242,2,3,207,2,2,
    173,1,1,141,0,0,111,0,0,83,0,0,58,0,0,36,
    51,51,242,51,51,242,51,51,232,50,49,192,50,44,151,50,
    38,109,50,31,72,49,26,45,46,23,37,43,20,29,40,19,
    28,38,17,24,35,14,17,32,12,12,29,10,9,27,9,9,
    51,51,242,51,51,242,50,51,222,49,51,184,47,51,148,45,
    49,113,43,47,83,40,44,58,25,8,5,23,7,3,20,6,
    1,17,4,0,8,6,19,5,4,10,3,2,2,3,0,0,
    0,0,36,0,0,27,0,0,18,0,0,12,0,0,4,0,
    0,0,0,0,0,0,0,0,50,35,85,46,47,117,51,31,
    242,49,3,239,36,2,177,23,0,110,12,0,58,31,21,95
};

const byte C_50ADAC[] = {
    0,0,0,1,0,0,0,0,0,14,33,31,80,173,172,1,
    1,1,0,0,0,0,0,0,0,0,0,5,14,8,2,7,
    1,0,1,0,0,0,0,13,20,14,10,16,9,8,11,7,
    80,164,155,80,151,143,80,142,136,80,128,121,79,119,113,76,
    108,103,73,96,92,69,86,82,65,78,74,62,71,67,58,62,
    59,56,58,55,52,50,47,50,44,42,45,36,34,44,34,32,
    40,28,26,38,25,24,34,21,20,32,16,16,28,14,13,27,
    12,11,24,9,9,22,7,7,19,5,4,18,1,1,15,0,
    0,14,0,0,12,0,0,11,0,0,8,0,0,8,0,0,
    80,173,172,80,173,172,80,173,172,80,173,165,80,173,157,80,
    173,146,80,164,134,80,159,127,80,149,112,80,140,101,79,130,
    92,74,121,84,71,111,75,67,101,65,63,92,59,61,87,55,
    55,81,51,50,73,45,46,68,42,42,62,38,39,56,34,33,
    50,30,30,43,26,27,39,23,24,36,21,19,31,18,15,26,
    14,12,21,11,10,16,9,8,12,6,4,8,2,3,5,1,
    80,173,172,80,173,172,80,173,172,80,173,172,78,173,172,74,
    173,165,71,168,160,67,158,150,63,149,142,61,145,138,57,134,
    127,55,129,123,51,120,115,47,111,105,45,105,100,41,96,92,
    37,86,82,35,81,78,31,73,69,27,64,61,26,60,57,22,
    52,50,19,46,44,18,42,40,15,36,34,13,31,29,11,26,
    25,9,20,20,8,19,18,6,14,13,4,9,9,3,7,7,
    42,173,88,37,173,77,33,168,67,28,153,59,25,134,49,22,
    118,42,18,99,34,15,86,29,13,71,21,10,56,18,8,43,
    12,5,31,7,3,20,4,0,12,0,0,5,0,0,0,0,
    61,128,108,57,119,98,53,108,89,49,99,82,45,89,72,43,
    83,67,39,77,61,35,67,53,32,60,47,28,52,40,26,49,
    37,23,42,32,20,39,29,17,33,22,15,26,19,13,24,17,
    43,85,64,35,69,50,30,59,41,25,46,32,19,36,24,15,
    27,18,12,20,11,8,14,8,30,73,59,25,60,47,21,52,
    40,18,45,34,14,35,25,12,29,20,9,24,15,7,19,11,
    74,173,111,67,151,85,57,121,65,49,93,45,40,67,29,34,
    44,16,26,27,6,21,14,1,80,173,172,80,173,172,80,168,
    160,80,134,128,80,104,99,80,78,74,80,50,47,80,28,28,
    77,12,11,70,9,9,66,9,9,61,7,7,55,7,7,51,
    6,6,45,5,4,40,3,3,35,1,1,29,0,0,25,0,
    0,21,0,0,17,0,0,14,0,0,11,0,0,8,0,0,
    80,173,172,76,173,172,64,151,172,52,123,172,41,96,172,28,
    66,172,19,45,172,11,26,172,5,12,172,4,9,147,3,7,
    123,2,5,101,1,1,79,0,0,59,0,0,41,0,0,26,
    80,173,172,80,173,172,80,173,165,79,168,136,79,149,107,78,
    129,78,79,106,51,77,88,32,72,78,26,67,69,21,63,64,
    20,59,57,17,55,48,12,51,41,9,45,35,7,42,32,6,
    80,173,172,80,173,172,79,173,158,77,173,131,74,173,105,71,
    168,80,68,160,59,63,151,41,40,28,3,36,24,2,32,20,
    1,27,13,0,13,20,13,8,14,7,5,7,1,4,1,0,
    0,0,26,0,0,19,0,0,13,0,0,9,0,0,3,0,
    0,0,0,0,0,0,0,0,78,120,61,72,159,83,80,104,
    172,77,12,170,57,7,126,37,1,78,19,0,41,49,71,67
};

const byte C_CCE4A5[] = {
    0,0,0,2,0,0,0,0,0,35,43,30,204,228,165,2,
    2,1,0,0,0,0,0,0,0,0,0,13,18,8,6,9,
    1,0,2,0,0,0,0,32,27,14,25,21,9,21,14,6,
    204,215,149,204,198,137,204,188,131,204,168,116,202,156,109,193,
    142,99,186,126,88,176,114,79,166,103,71,158,93,64,148,81,
    56,143,77,53,133,65,45,128,58,40,116,47,33,112,45,31,
    102,37,25,96,33,23,87,28,19,82,21,15,72,18,12,68,
    15,10,61,12,8,56,9,6,50,6,4,46,2,1,39,0,
    0,36,0,0,31,0,0,27,0,0,22,0,0,21,0,0,
    204,228,165,204,228,165,204,228,165,204,228,158,203,228,151,203,
    228,140,204,215,128,204,210,122,203,197,107,204,185,97,202,172,
    89,190,159,80,180,147,72,170,133,63,160,121,56,154,114,52,
    140,107,49,126,96,43,118,89,40,107,82,37,99,73,32,84,
    65,29,77,56,25,69,52,22,62,47,20,50,41,17,39,34,
    14,31,28,11,26,21,8,19,16,6,10,11,2,7,7,1,
    204,228,165,204,228,165,204,228,165,204,228,165,198,228,165,188,
    228,158,182,221,153,171,208,144,162,196,136,157,190,132,145,176,
    122,140,170,118,130,158,110,120,146,101,114,139,96,104,126,88,
    94,114,79,89,107,74,78,96,66,70,85,59,66,80,55,57,
    69,48,50,61,42,46,55,39,39,47,33,33,40,28,29,35,
    24,22,27,19,21,25,17,14,18,12,10,12,8,7,9,6,
    106,228,85,94,228,74,83,222,65,72,202,56,65,176,47,55,
    156,40,46,131,33,39,114,28,34,93,20,26,73,17,20,56,
    12,12,41,7,7,27,4,1,15,0,0,6,0,0,0,0,
    157,169,104,146,156,94,136,142,85,126,131,78,114,117,69,110,
    109,64,99,102,58,89,89,50,82,80,45,71,69,39,67,64,
    36,58,55,30,52,51,28,44,43,21,38,35,18,34,31,16,
    110,113,61,89,91,48,78,78,39,64,61,31,47,47,23,38,
    36,17,30,26,11,22,19,8,76,96,56,63,80,45,54,69,
    39,46,59,33,37,46,24,30,38,19,23,31,14,18,25,11,
    190,228,106,171,198,82,146,159,62,125,122,43,103,89,28,86,
    58,15,66,36,6,53,18,1,204,228,165,204,228,165,204,221,
    153,204,177,123,204,138,95,204,103,71,204,65,45,204,38,27,
    198,15,10,179,13,9,167,12,8,155,10,7,140,9,6,129,
    8,6,114,6,4,103,4,3,90,2,1,74,0,0,64,0,
    0,54,0,0,43,0,0,35,0,0,27,0,0,21,0,0,
    204,228,165,194,228,165,164,199,165,134,163,165,104,126,165,72,
    88,165,48,59,165,28,34,165,12,15,165,10,12,141,7,9,
    118,5,6,96,2,2,76,0,0,56,0,0,39,0,0,25,
    204,228,165,204,228,165,204,228,158,202,221,131,202,196,103,200,
    170,74,202,139,49,195,115,30,182,103,25,172,91,20,162,84,
    19,151,75,16,139,63,12,129,54,8,116,46,6,107,42,6,
    204,228,165,204,228,165,202,228,151,195,228,126,188,228,101,182,
    221,77,173,211,56,161,198,39,101,38,3,92,31,2,82,26,
    1,70,17,0,32,27,13,22,18,7,13,9,1,10,2,0,
    0,0,25,0,0,18,0,0,12,0,0,8,0,0,3,0,
    0,0,0,0,0,0,0,0,198,158,58,182,209,80,204,138,
    165,198,15,163,145,10,121,94,2,75,50,0,39,126,94,65
};

const byte C_CCEA5F[] = {
    0,0,0,2,0,0,0,0,0,35,44,17,204,234,95,2,
    2,1,0,0,0,0,0,0,0,0,0,13,18,4,6,9,
    1,0,2,0,0,0,0,32,28,8,25,21,5,21,15,4,
    204,221,86,204,204,79,204,193,75,204,173,67,202,161,63,193,
    146,57,186,129,51,176,117,45,166,106,41,158,95,37,148,84,
    32,143,79,31,133,67,26,128,60,23,116,49,19,112,46,18,
    102,38,15,96,34,13,87,28,11,82,22,9,72,18,7,68,
    16,6,61,12,5,56,9,4,50,6,2,46,2,1,39,0,
    0,36,0,0,31,0,0,27,0,0,22,0,0,21,0,0,
    204,234,95,204,234,95,204,234,95,204,234,91,203,234,87,203,
    234,81,204,221,74,204,216,70,203,202,62,204,190,56,202,176,
    51,190,163,46,180,150,41,170,137,36,160,124,32,154,117,30,
    140,110,28,126,98,25,118,92,23,107,84,21,99,75,19,84,
    67,17,77,58,14,69,53,13,62,49,12,50,42,10,39,35,
    8,31,28,6,26,22,5,19,17,3,10,11,1,7,7,1,
    204,234,95,204,234,95,204,234,95,204,234,95,198,234,95,188,
    234,91,182,227,88,171,214,83,162,201,78,157,195,76,145,181,
    70,140,174,68,130,162,63,120,150,58,114,142,55,104,129,51,
    94,117,45,89,110,43,78,98,38,70,87,34,66,82,32,57,
    71,28,50,62,24,46,57,22,39,49,19,33,41,16,29,36,
    14,22,28,11,21,26,10,14,18,7,10,12,5,7,9,4,
    106,234,49,94,234,42,83,228,37,72,207,32,65,181,27,55,
    160,23,46,134,19,39,117,16,34,95,12,26,75,10,20,58,
    7,12,42,4,7,28,2,1,16,0,0,6,0,0,0,0,
    157,173,60,146,161,54,136,146,49,126,134,45,114,120,40,110,
    112,37,99,105,34,89,91,29,82,82,26,71,71,22,67,66,
    20,58,57,18,52,52,16,44,44,12,38,36,10,34,32,9,
    110,116,35,89,94,28,78,80,23,64,62,18,47,49,13,38,
    37,10,30,27,6,22,19,4,76,98,32,63,82,26,54,71,
    22,46,61,19,37,47,14,30,39,11,23,32,8,18,26,6,
    190,234,61,171,204,47,146,163,36,125,126,25,103,91,16,86,
    60,9,66,37,3,53,18,0,204,234,95,204,234,95,204,227,
    88,204,182,71,204,141,55,204,106,41,204,67,26,204,39,15,
    198,16,6,179,13,5,167,12,5,155,10,4,140,9,4,129,
    8,3,114,6,2,103,4,1,90,2,1,74,0,0,64,0,
    0,54,0,0,43,0,0,35,0,0,27,0,0,21,0,0,
    204,234,95,194,234,95,164,205,95,134,167,95,104,129,95,72,
    90,95,48,61,95,28,35,95,12,16,95,10,12,81,7,9,
    68,5,6,56,2,2,44,0,0,32,0,0,23,0,0,14,
    204,234,95,204,234,95,204,234,91,202,227,75,202,201,59,200,
    174,43,202,143,28,195,118,18,182,106,15,172,94,12,162,86,
    11,151,77,9,139,65,7,129,55,5,116,47,4,107,43,3,
    204,234,95,204,234,95,202,234,87,195,234,72,188,234,58,182,
    227,44,173,217,32,161,204,23,101,39,2,92,32,1,82,27,
    0,70,17,0,32,28,7,22,18,4,13,9,1,10,2,0,
    0,0,14,0,0,10,0,0,7,0,0,5,0,0,1,0,
    0,0,0,0,0,0,0,0,198,162,34,182,215,46,204,141,
    95,198,16,94,145,10,70,94,2,43,50,0,23,126,96,37
};

const byte C_B30202[] = {
    0,0,0,1,0,0,0,0,0,31,0,0,179,2,2,1,
    0,0,0,0,0,0,0,0,0,0,0,11,0,0,5,0,
    0,0,0,0,0,0,0,28,0,0,22,0,0,18,0,0,
    179,2,2,179,2,2,179,2,2,179,1,1,178,1,1,169,
    1,1,164,1,1,154,1,1,146,1,1,139,1,1,130,1,
    1,126,1,1,117,1,1,112,1,0,102,0,0,98,0,0,
    90,0,0,84,0,0,77,0,0,72,0,0,63,0,0,60,
    0,0,53,0,0,49,0,0,44,0,0,40,0,0,34,0,
    0,32,0,0,27,0,0,24,0,0,19,0,0,18,0,0,
    179,2,2,179,2,2,179,2,2,179,2,2,178,2,2,178,
    2,2,179,2,2,179,2,1,178,2,1,179,2,1,177,2,
    1,166,1,1,158,1,1,150,1,1,140,1,1,135,1,1,
    123,1,1,111,1,1,104,1,0,94,1,0,87,1,0,74,
    1,0,67,0,0,60,0,0,54,0,0,44,0,0,34,0,
    0,27,0,0,22,0,0,17,0,0,9,0,0,6,0,0,
    179,2,2,179,2,2,179,2,2,179,2,2,174,2,2,165,
    2,2,159,2,2,150,2,2,142,2,2,138,2,2,127,2,
    1,123,1,1,114,1,1,105,1,1,100,1,1,91,1,1,
    82,1,1,78,1,1,69,1,1,61,1,1,58,1,1,50,
    1,1,44,1,1,40,0,0,34,0,0,29,0,0,25,0,
    0,20,0,0,18,0,0,13,0,0,8,0,0,6,0,0,
    93,2,1,82,2,1,73,2,1,63,2,1,57,2,1,48,
    1,0,40,1,0,34,1,0,29,1,0,22,1,0,18,0,
    0,11,0,0,6,0,0,1,0,0,0,0,0,0,0,0,
    138,1,1,128,1,1,119,1,1,110,1,1,100,1,1,97,
    1,1,87,1,1,78,1,1,72,1,1,62,1,0,59,1,
    0,51,0,0,46,0,0,39,0,0,34,0,0,30,0,0,
    96,1,1,78,1,1,68,1,0,56,1,0,41,0,0,33,
    0,0,27,0,0,19,0,0,67,1,1,55,1,1,48,1,
    0,40,1,0,32,0,0,26,0,0,20,0,0,16,0,0,
    166,2,1,150,2,1,128,1,1,110,1,1,91,1,0,75,
    1,0,58,0,0,46,0,0,179,2,2,179,2,2,179,2,
    2,179,2,1,179,1,1,179,1,1,179,1,1,179,0,0,
    173,0,0,157,0,0,147,0,0,136,0,0,123,0,0,113,
    0,0,100,0,0,91,0,0,79,0,0,65,0,0,56,0,
    0,47,0,0,38,0,0,31,0,0,24,0,0,18,0,0,
    179,2,2,170,2,2,144,2,2,117,1,2,91,1,2,63,
    1,2,42,1,2,25,0,2,11,0,2,8,0,2,6,0,
    1,4,0,1,1,0,1,0,0,1,0,0,0,0,0,0,
    179,2,2,179,2,2,179,2,2,177,2,2,177,2,1,175,
    1,1,177,1,1,171,1,0,160,1,0,151,1,0,142,1,
    0,133,1,0,122,1,0,113,0,0,102,0,0,94,0,0,
    179,2,2,179,2,2,177,2,2,171,2,2,165,2,1,159,
    2,1,152,2,1,141,2,0,88,0,0,81,0,0,72,0,
    0,61,0,0,28,0,0,19,0,0,11,0,0,9,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,174,1,1,160,2,1,179,1,
    2,173,0,2,127,0,1,82,0,1,44,0,0,110,1,1
};

const byte C_B87A15[] = {
    0,0,0,1,0,0,0,0,0,32,23,4,184,122,21,1,
    1,0,0,0,0,0,0,0,0,0,0,12,10,1,5,5,
    0,0,1,0,0,0,0,29,14,2,22,11,1,19,8,1,
    184,115,19,184,106,17,184,100,17,184,90,15,183,84,14,174,
    76,13,168,67,11,159,61,10,150,55,9,143,50,8,133,44,
    7,129,41,7,120,35,6,115,31,5,105,25,4,101,24,4,
    92,20,3,87,18,3,79,15,2,74,11,2,65,10,2,61,
    8,1,55,6,1,51,5,1,45,3,0,41,1,0,35,0,
    0,32,0,0,28,0,0,25,0,0,19,0,0,19,0,0,
    184,122,21,184,122,21,184,122,21,184,122,20,183,122,19,183,
    122,18,184,115,16,184,112,16,183,105,14,184,99,12,182,92,
    11,171,85,10,162,78,9,154,71,8,144,65,7,139,61,7,
    126,57,6,114,51,6,107,48,5,97,44,5,89,39,4,76,
    35,4,69,30,3,62,28,3,56,25,3,45,22,2,35,18,
    2,28,15,1,23,11,1,17,9,1,9,6,0,6,4,0,
    184,122,21,184,122,21,184,122,21,184,122,21,179,122,21,170,
    122,20,164,118,20,154,111,18,146,105,17,141,102,17,131,94,
    16,126,91,15,118,85,14,108,78,13,102,74,12,94,67,11,
    84,61,10,80,57,9,71,51,8,63,45,7,59,43,7,51,
    37,6,45,33,5,41,30,5,35,25,4,30,22,4,26,19,
    3,20,14,2,19,13,2,13,10,2,9,6,1,6,5,1,
    96,122,11,84,122,9,75,119,8,65,108,7,58,94,6,50,
    83,5,41,70,4,35,61,4,30,50,3,23,39,2,18,30,
    1,11,22,1,6,14,0,1,8,0,0,3,0,0,0,0,
    141,90,13,131,84,12,123,76,11,113,70,10,102,63,9,100,
    58,8,89,55,7,80,47,6,74,43,6,64,37,5,61,34,
    5,53,30,4,47,27,4,40,23,3,35,19,2,31,17,2,
    99,60,8,80,49,6,70,42,5,58,33,4,43,25,3,34,
    19,2,27,14,1,19,10,1,69,51,7,57,43,6,49,37,
    5,41,32,4,33,24,3,27,21,2,21,17,2,17,13,1,
    171,122,14,154,106,10,131,85,8,113,66,5,93,47,4,77,
    31,2,60,19,1,48,10,0,184,122,21,184,122,21,184,118,
    20,184,95,16,184,74,12,184,55,9,184,35,6,184,20,3,
    178,8,1,162,7,1,151,6,1,140,5,1,126,5,1,116,
    4,1,103,3,0,93,2,0,82,1,0,66,0,0,58,0,
    0,48,0,0,39,0,0,32,0,0,25,0,0,19,0,0,
    184,122,21,175,122,21,148,107,21,121,87,21,94,67,21,65,
    47,21,43,32,21,25,18,21,11,8,21,9,6,18,6,5,
    15,4,3,12,1,1,10,0,0,7,0,0,5,0,0,3,
    184,122,21,184,122,21,184,122,20,182,118,17,182,105,13,180,
    91,9,182,75,6,176,62,4,165,55,3,155,49,3,146,45,
    2,136,40,2,126,34,1,116,29,1,105,24,1,97,22,1,
    184,122,21,184,122,21,182,122,19,176,122,16,170,122,13,164,
    118,10,156,113,7,145,106,5,91,20,0,83,17,0,74,14,
    0,63,9,0,29,14,2,19,10,1,12,5,0,9,1,0,
    0,0,3,0,0,2,0,0,2,0,0,1,0,0,0,0,
    0,0,0,0,0,0,0,0,179,85,7,165,112,10,184,74,
    21,178,8,21,131,5,15,84,1,10,45,0,5,113,50,8
};

const byte C_FFD000[] = {
    0,0,0,2,0,0,0,0,0,44,39,0,255,208,0,2,
    2,0,0,0,0,0,0,0,0,0,0,16,16,0,7,8,
    0,0,2,0,0,0,0,40,24,0,31,19,0,26,13,0,
    255,197,0,255,181,0,255,171,0,255,153,0,253,143,0,241,
    130,0,233,115,0,220,104,0,208,94,0,198,85,0,185,74,
    0,179,70,0,166,60,0,160,53,0,145,43,0,140,41,0,
    128,33,0,120,30,0,109,25,0,103,20,0,90,16,0,85,
    14,0,76,11,0,70,8,0,62,6,0,57,2,0,49,0,
    0,45,0,0,39,0,0,34,0,0,27,0,0,26,0,0,
    255,208,0,255,208,0,255,208,0,255,208,0,254,208,0,254,
    208,0,255,197,0,255,192,0,254,179,0,255,169,0,252,157,
    0,237,145,0,225,134,0,213,122,0,200,110,0,193,104,0,
    175,98,0,158,87,0,148,82,0,134,75,0,124,67,0,105,
    60,0,96,51,0,86,47,0,77,43,0,62,38,0,49,31,
    0,39,25,0,32,20,0,24,15,0,13,10,0,9,7,0,
    255,208,0,255,208,0,255,208,0,255,208,0,248,208,0,235,
    208,0,227,201,0,214,190,0,202,179,0,196,174,0,181,161,
    0,175,155,0,163,144,0,150,133,0,142,126,0,130,115,0,
    117,104,0,111,98,0,98,87,0,87,77,0,82,73,0,71,
    63,0,62,55,0,57,51,0,49,43,0,41,37,0,36,32,
    0,28,24,0,26,23,0,18,16,0,12,11,0,9,8,0,
    133,208,0,117,208,0,104,202,0,90,184,0,81,161,0,69,
    142,0,57,119,0,49,104,0,42,85,0,32,67,0,25,51,
    0,15,38,0,9,24,0,1,14,0,0,6,0,0,0,0,
    196,154,0,182,143,0,170,130,0,157,119,0,142,107,0,138,
    100,0,124,93,0,111,81,0,102,73,0,89,63,0,84,59,
    0,73,51,0,65,46,0,55,39,0,48,32,0,43,29,0,
    137,103,0,111,83,0,97,71,0,80,55,0,59,43,0,47,
    33,0,38,24,0,27,17,0,95,87,0,79,73,0,68,63,
    0,57,54,0,46,42,0,37,35,0,29,29,0,23,23,0,
    237,208,0,214,181,0,182,145,0,156,112,0,129,81,0,107,
    53,0,83,33,0,66,16,0,255,208,0,255,208,0,255,201,
    0,255,162,0,255,126,0,255,94,0,255,60,0,255,34,0,
    247,14,0,224,11,0,209,11,0,194,9,0,175,8,0,161,
    7,0,143,6,0,129,3,0,113,2,0,92,0,0,80,0,
    0,67,0,0,54,0,0,44,0,0,34,0,0,26,0,0,
    255,208,0,242,208,0,205,182,0,167,148,0,130,115,0,90,
    80,0,60,54,0,35,31,0,15,14,0,12,11,0,9,8,
    0,6,6,0,2,2,0,0,0,0,0,0,0,0,0,0,
    255,208,0,255,208,0,255,208,0,252,201,0,252,179,0,250,
    155,0,252,127,0,244,105,0,228,94,0,215,83,0,202,77,
    0,189,69,0,174,58,0,161,49,0,145,42,0,134,38,0,
    255,208,0,255,208,0,252,208,0,244,208,0,235,208,0,227,
    201,0,216,193,0,201,181,0,126,34,0,115,29,0,102,24,
    0,87,15,0,40,24,0,27,16,0,16,8,0,13,2,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,248,144,0,228,191,0,255,126,
    0,247,14,0,181,9,0,117,2,0,62,0,0,157,86,0
};

const byte C_FFDE4C[] = {
    0,0,0,2,0,0,0,0,0,44,42,14,255,222,76,2,
    2,1,0,0,0,0,0,0,0,0,0,16,17,4,7,9,
    1,0,2,0,0,0,0,40,26,6,31,20,4,26,14,3,
    255,210,69,255,193,63,255,183,60,255,164,54,253,152,50,241,
    138,46,233,123,41,220,111,36,208,100,33,198,91,30,185,79,
    26,179,75,24,166,64,21,160,57,18,145,46,15,140,44,14,
    128,36,12,120,32,10,109,27,9,103,21,7,90,17,6,85,
    15,5,76,11,4,70,9,3,62,6,2,57,2,1,49,0,
    0,45,0,0,39,0,0,34,0,0,27,0,0,26,0,0,
    255,222,76,255,222,76,255,222,76,255,222,73,254,222,69,254,
    222,65,255,210,59,255,205,56,254,192,49,255,180,45,252,167,
    41,237,155,37,225,143,33,213,130,29,200,118,26,193,111,24,
    175,104,22,158,93,20,148,87,18,134,80,17,124,71,15,105,
    64,13,96,55,11,86,50,10,77,46,9,62,40,8,49,33,
    6,39,27,5,32,21,4,24,16,3,13,10,1,9,7,1,
    255,222,76,255,222,76,255,222,76,255,222,76,248,222,76,235,
    222,73,227,215,71,214,203,66,202,191,63,196,185,61,181,172,
    56,175,165,54,163,154,51,150,142,46,142,135,44,130,123,41,
    117,111,36,111,104,34,98,93,30,87,83,27,82,77,25,71,
    67,22,62,59,19,57,54,18,49,46,15,41,39,13,36,34,
    11,28,26,9,26,24,8,18,17,6,12,11,4,9,9,3,
    133,222,39,117,222,34,104,216,30,90,197,26,81,172,22,69,
    151,18,57,127,15,49,111,13,42,91,9,32,71,8,25,55,
    5,15,40,3,9,26,2,1,15,0,0,6,0,0,0,0,
    196,165,48,182,152,44,170,138,39,157,127,36,142,114,32,138,
    106,30,124,99,27,111,86,23,102,77,21,89,67,18,84,63,
    16,73,54,14,65,50,13,55,42,10,48,34,8,43,30,7,
    137,110,28,111,89,22,97,76,18,80,59,14,59,46,10,47,
    35,8,38,25,5,27,18,4,95,93,26,79,77,21,68,67,
    18,57,57,15,46,44,11,37,37,9,29,30,7,23,24,5,
    237,222,49,214,193,38,182,155,29,156,119,20,129,86,13,107,
    57,7,83,35,3,66,17,0,255,222,76,255,222,76,255,215,
    71,255,172,57,255,134,44,255,100,33,255,64,21,255,37,12,
    247,15,5,224,12,4,209,11,4,194,10,3,175,9,3,161,
    8,3,143,6,2,129,3,1,113,2,1,92,0,0,80,0,
    0,67,0,0,54,0,0,44,0,0,34,0,0,26,0,0,
    255,222,76,242,222,76,205,194,76,167,158,76,130,123,76,90,
    85,76,60,57,76,35,33,76,15,15,76,12,11,65,9,9,
    54,6,6,44,2,2,35,0,0,26,0,0,18,0,0,11,
    255,222,76,255,222,76,255,222,73,252,215,60,252,191,47,250,
    165,34,252,136,23,244,112,14,228,100,12,215,89,9,202,82,
    9,189,73,7,174,62,5,161,52,4,145,44,3,134,41,3,
    255,222,76,255,222,76,252,222,70,244,222,58,235,222,46,227,
    215,35,216,205,26,201,193,18,126,37,1,115,30,1,102,25,
    0,87,17,0,40,26,6,27,17,3,16,9,1,13,2,0,
    0,0,11,0,0,8,0,0,6,0,0,4,0,0,1,0,
    0,0,0,0,0,0,0,0,248,154,27,228,204,37,255,134,
    76,247,15,75,181,10,56,117,2,35,62,0,18,157,91,30
};

const byte C_FFF588[] = {
    0,0,0,2,0,0,0,0,0,44,46,18,255,242,100,2,
    2,1,0,0,0,0,0,0,0,0,0,16,19,5,7,9,
    1,0,2,0,0,0,0,40,28,8,31,22,5,26,15,4,
    255,229,90,255,211,83,255,199,79,255,178,71,253,166,66,241,
    151,60,233,134,53,220,121,48,208,109,43,198,99,39,185,86,
    34,179,82,32,166,69,27,160,62,24,145,50,20,140,47,19,
    128,39,15,120,35,14,109,29,12,103,23,9,90,19,7,85,
    16,6,76,12,5,70,9,4,62,7,2,57,2,1,49,0,
    0,45,0,0,39,0,0,34,0,0,27,0,0,26,0,0,
    255,242,100,255,242,100,255,242,100,255,242,96,254,242,91,254,
    242,85,255,229,78,255,223,74,254,209,65,255,196,59,252,182,
    54,237,169,49,225,156,44,213,141,38,200,128,34,193,121,32,
    175,114,29,158,102,26,148,95,24,134,87,22,124,78,20,105,
    69,18,96,60,15,86,55,13,77,50,12,62,44,10,49,36,
    8,39,29,7,32,23,5,24,17,4,13,11,1,9,8,1,
    255,242,100,255,242,100,255,242,100,255,242,100,248,242,100,235,
    242,96,227,234,93,214,221,87,202,208,82,196,202,80,181,187,
    74,175,180,71,163,168,67,150,155,61,142,147,58,130,134,53,
    117,121,48,111,114,45,98,102,40,87,90,36,82,84,33,71,
    73,29,62,65,25,57,59,24,49,50,20,41,43,17,36,37,
    15,28,28,11,26,27,11,18,19,7,12,12,5,9,9,4,
    133,242,51,117,242,45,104,235,39,90,214,34,81,187,29,69,
    165,24,57,139,20,49,121,17,42,99,12,32,78,11,25,60,
    7,15,44,4,9,28,2,1,16,0,0,7,0,0,0,0,
    196,179,63,182,166,57,170,151,52,157,139,47,142,124,42,138,
    116,39,124,108,35,111,94,31,102,84,27,89,73,24,84,68,
    22,73,59,18,65,54,17,55,46,13,48,37,11,43,33,10,
    137,120,37,111,97,29,97,83,24,80,65,19,59,50,14,47,
    38,11,38,28,7,27,20,5,95,102,34,79,84,27,68,73,
    24,57,63,20,46,48,15,37,41,12,29,33,9,23,27,7,
    237,242,64,214,211,49,182,169,38,156,130,26,129,94,17,107,
    62,9,83,38,4,66,19,0,255,242,100,255,242,100,255,234,
    93,255,188,75,255,146,58,255,109,43,255,69,27,255,40,16,
    247,16,6,224,13,5,209,12,5,194,10,4,175,9,4,161,
    9,4,143,7,2,129,4,2,113,2,1,92,0,0,80,0,
    0,67,0,0,54,0,0,44,0,0,34,0,0,26,0,0,
    255,242,100,242,242,100,205,212,100,167,173,100,130,134,100,90,
    93,100,60,63,100,35,36,100,15,16,100,12,12,85,9,9,
    71,6,7,58,2,2,46,0,0,34,0,0,24,0,0,15,
    255,242,100,255,242,100,255,242,96,252,234,79,252,208,62,250,
    180,45,252,148,30,244,122,18,228,109,15,215,97,12,202,89,
    11,189,80,10,174,67,7,161,57,5,145,48,4,134,45,4,
    255,242,100,255,242,100,252,242,92,244,242,76,235,242,61,227,
    234,47,216,224,34,201,211,24,126,40,2,115,33,1,102,28,
    0,87,18,0,40,28,8,27,19,4,16,9,1,13,2,0,
    0,0,15,0,0,11,0,0,7,0,0,5,0,0,2,0,
    0,0,0,0,0,0,0,0,248,168,35,228,222,48,255,146,
    100,247,16,99,181,10,73,117,2,45,62,0,24,157,100,39
};

const byte C_043E8B[] = {
    0,0,0,0,0,0,0,0,0,0,20,46,0,106,255,0,
    1,2,0,0,0,0,0,0,0,0,0,0,8,12,0,4,
    2,0,1,0,0,0,0,0,12,21,0,10,14,0,7,10,
    0,100,230,0,92,212,0,87,202,0,78,180,0,73,168,0,
    66,153,0,59,136,0,53,122,0,48,110,0,43,99,0,38,
    87,0,36,82,0,30,70,0,27,62,0,22,51,0,21,48,
    0,17,39,0,15,35,0,13,30,0,10,23,0,8,19,0,
    7,16,0,5,13,0,4,10,0,3,6,0,1,2,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,106,255,0,106,255,0,106,255,0,106,244,0,106,233,0,
    106,217,0,100,198,0,98,189,0,91,166,0,86,150,0,80,
    137,0,74,124,0,68,111,0,62,97,0,56,87,0,53,81,
    0,50,75,0,44,67,0,42,62,0,38,57,0,34,50,0,
    30,45,0,26,38,0,24,34,0,22,31,0,19,26,0,16,
    21,0,13,17,0,10,13,0,7,9,0,5,3,0,3,2,
    0,106,255,0,106,255,0,106,255,0,106,255,0,106,255,0,
    106,244,0,103,237,0,97,223,0,91,210,0,89,204,0,82,
    189,0,79,182,0,74,170,0,68,156,0,64,148,0,59,136,
    0,53,122,0,50,115,0,44,102,0,39,91,0,37,85,0,
    32,74,0,28,65,0,26,60,0,22,51,0,19,43,0,16,
    37,0,12,29,0,12,27,0,8,19,0,5,13,0,4,10,
    0,106,131,0,106,114,0,103,100,0,94,87,0,82,73,0,
    72,62,0,61,51,0,53,43,0,43,31,0,34,27,0,26,
    18,0,19,11,0,12,6,0,7,0,0,3,0,0,0,0,
    0,79,160,0,73,146,0,66,132,0,61,121,0,54,107,0,
    51,99,0,47,90,0,41,78,0,37,69,0,32,60,0,30,
    55,0,26,47,0,24,43,0,20,33,0,16,28,0,15,25,
    0,52,95,0,42,74,0,36,61,0,28,48,0,22,35,0,
    17,27,0,12,17,0,9,12,0,44,87,0,37,70,0,32,
    60,0,27,51,0,21,37,0,18,30,0,15,22,0,12,17,
    0,106,164,0,92,126,0,74,96,0,57,66,0,41,43,0,
    27,23,0,17,9,0,8,1,0,106,255,0,106,255,0,103,
    237,0,82,190,0,64,147,0,48,110,0,30,70,0,17,41,
    0,7,16,0,6,14,0,5,13,0,5,11,0,4,10,0,
    4,9,0,3,6,0,2,4,0,1,2,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,106,255,0,106,255,0,93,255,0,76,255,0,59,255,0,
    41,255,0,27,255,0,16,255,0,7,255,0,5,218,0,4,
    182,0,3,149,0,1,117,0,0,87,0,0,61,0,0,38,
    0,106,255,0,106,255,0,106,244,0,103,202,0,91,159,0,
    79,115,0,65,76,0,54,47,0,48,39,0,42,31,0,39,
    29,0,35,25,0,30,18,0,25,13,0,21,10,0,20,9,
    0,106,255,0,106,255,0,106,234,0,106,194,0,106,156,0,
    103,119,0,98,87,0,92,61,0,17,5,0,15,3,0,12,
    1,0,8,0,0,12,20,0,8,11,0,4,2,0,1,0,
    0,0,38,0,0,28,0,0,19,0,0,13,0,0,4,0,
    0,0,0,0,0,0,0,0,0,74,90,0,97,123,0,64,
    255,0,7,252,0,5,187,0,1,116,0,0,61,0,44,100
};

const byte C_5B4318[] = {
    0,0,0,1,0,0,0,0,0,29,23,6,168,121,34,1,
    1,0,0,0,0,0,0,0,0,0,0,11,9,2,5,5,
    0,0,1,0,0,0,0,26,14,3,20,11,2,17,8,1,
    168,114,31,168,105,28,168,100,27,168,89,24,167,83,22,159,
    75,20,154,67,18,145,60,16,137,55,15,130,49,13,122,43,
    12,118,41,11,109,35,9,105,31,8,96,25,7,92,24,6,
    84,19,5,79,18,5,72,15,4,68,11,3,59,9,3,56,
    8,2,50,6,2,46,5,1,41,3,1,38,1,0,32,0,
    0,30,0,0,26,0,0,22,0,0,18,0,0,17,0,0,
    168,121,34,168,121,34,168,121,34,168,121,33,167,121,31,167,
    121,29,168,114,26,168,112,25,167,104,22,168,98,20,166,91,
    18,156,84,17,148,78,15,140,71,13,132,64,12,127,61,11,
    115,57,10,104,51,9,98,47,8,88,44,8,82,39,7,69,
    35,6,63,30,5,57,28,5,51,25,4,41,22,3,32,18,
    3,26,15,2,21,11,2,16,9,1,9,6,0,6,4,0,
    168,121,34,168,121,34,168,121,34,168,121,34,163,121,34,155,
    121,33,150,117,32,141,111,30,133,104,28,129,101,27,119,93,
    25,115,90,24,107,84,23,99,77,21,94,74,20,86,67,18,
    77,60,16,73,57,15,65,51,14,57,45,12,54,42,11,47,
    37,10,41,32,9,38,29,8,32,25,7,27,21,6,24,19,
    5,18,14,4,17,13,4,12,9,3,8,6,2,6,5,1,
    88,121,17,77,121,15,69,118,13,59,107,12,53,93,10,45,
    83,8,38,69,7,32,60,6,28,49,4,21,39,4,16,30,
    2,10,22,1,6,14,1,1,8,0,0,3,0,0,0,0,
    129,90,21,120,83,19,112,75,18,103,69,16,94,62,14,91,
    58,13,82,54,12,73,47,10,67,42,9,59,37,8,55,34,
    7,48,29,6,43,27,6,36,23,4,32,19,4,28,17,3,
    90,60,13,73,48,10,64,41,8,53,32,6,39,25,5,31,
    19,4,25,14,2,18,10,2,63,51,12,52,42,9,45,37,
    8,38,31,7,30,24,5,24,20,4,19,17,3,15,13,2,
    156,121,22,141,105,17,120,84,13,103,65,9,85,47,6,70,
    31,3,55,19,1,43,9,0,168,121,34,168,121,34,168,117,
    32,168,94,25,168,73,20,168,55,15,168,35,9,168,20,5,
    163,8,2,148,7,2,138,6,2,128,5,1,115,5,1,106,
    4,1,94,3,1,85,2,1,74,1,0,61,0,0,53,0,
    0,44,0,0,36,0,0,29,0,0,22,0,0,17,0,0,
    168,121,34,159,121,34,135,106,34,110,86,34,86,67,34,59,
    47,34,40,31,34,23,18,34,10,8,34,8,6,29,6,5,
    24,4,3,20,1,1,16,0,0,12,0,0,8,0,0,5,
    168,121,34,168,121,34,168,121,33,166,117,27,166,104,21,165,
    90,15,166,74,10,161,61,6,150,55,5,142,48,4,133,45,
    4,125,40,3,115,34,2,106,28,2,96,24,1,88,22,1,
    168,121,34,168,121,34,166,121,31,161,121,26,155,121,21,150,
    117,16,142,112,12,132,105,8,83,20,1,76,17,0,67,14,
    0,57,9,0,26,14,3,18,9,1,11,5,0,9,1,0,
    0,0,5,0,0,4,0,0,3,0,0,2,0,0,1,0,
    0,0,0,0,0,0,0,0,163,84,12,150,111,16,168,73,
    34,163,8,34,119,5,25,77,1,15,41,0,8,103,50,13
};

const byte C_4F5D8B[] = {
    0,0,0,1,0,0,0,0,0,19,25,39,108,134,217,1,
    1,2,0,0,0,0,0,0,0,0,0,7,11,10,3,5,
    2,0,1,0,0,0,0,17,16,18,13,12,12,11,8,9,
    108,127,196,108,117,180,108,110,172,108,99,153,107,92,143,102,
    84,130,99,74,116,93,67,104,88,60,94,84,55,84,78,48,
    74,76,45,70,70,38,60,68,34,53,61,28,43,59,26,41,
    54,22,33,51,19,30,46,16,26,44,13,20,38,11,16,36,
    9,14,32,7,11,30,5,9,26,4,5,24,1,2,21,0,
    0,19,0,0,17,0,0,14,0,0,11,0,0,11,0,0,
    108,134,217,108,134,217,108,134,217,108,134,208,108,134,198,108,
    134,185,108,127,168,108,123,161,108,116,141,108,109,128,107,101,
    117,100,94,106,95,86,94,90,78,83,85,71,74,82,67,69,
    74,63,64,67,56,57,63,53,53,57,48,49,53,43,43,44,
    38,38,41,33,32,36,30,29,33,28,26,26,24,22,21,20,
    18,17,16,14,14,13,11,10,9,8,6,6,3,4,4,2,
    108,134,217,108,134,217,108,134,217,108,134,217,105,134,217,100,
    134,208,96,130,202,91,122,190,86,115,179,83,112,174,77,104,
    161,74,100,155,69,93,145,64,86,133,60,81,126,55,74,116,
    50,67,104,47,63,98,42,56,87,37,50,77,35,47,72,30,
    40,63,26,36,55,24,33,51,21,28,43,17,24,37,15,20,
    31,12,16,25,11,15,23,8,11,16,5,7,11,4,5,9,
    56,134,111,50,134,97,44,130,85,38,119,74,34,104,62,29,
    91,53,24,77,43,21,67,37,18,55,26,14,43,23,11,33,
    15,6,24,9,4,16,5,0,9,0,0,4,0,0,0,0,
    83,99,136,77,92,124,72,84,112,66,77,103,60,69,91,58,
    64,84,53,60,77,47,52,66,43,47,59,38,40,51,36,38,
    47,31,33,40,28,30,37,23,25,28,20,20,24,18,18,21,
    58,66,81,47,54,63,41,46,52,34,36,41,25,28,30,20,
    21,23,16,15,14,11,11,10,40,56,74,33,47,60,29,40,
    51,24,35,43,19,27,31,16,23,26,12,18,19,10,15,14,
    100,134,140,91,117,107,77,94,82,66,72,56,55,52,37,45,
    34,20,35,21,8,28,11,1,108,134,217,108,134,217,108,130,
    202,108,104,162,108,81,125,108,60,94,108,38,60,108,22,35,
    105,9,14,95,7,12,89,7,11,82,6,9,74,5,9,68,
    5,8,61,4,5,55,2,3,48,1,2,39,0,0,34,0,
    0,28,0,0,23,0,0,19,0,0,14,0,0,11,0,0,
    108,134,217,102,134,217,87,117,217,71,96,217,55,74,217,38,
    51,217,25,35,217,15,20,217,6,9,217,5,7,186,4,5,
    155,3,4,127,1,1,100,0,0,74,0,0,52,0,0,32,
    108,134,217,108,134,217,108,134,208,107,130,172,107,115,135,106,
    100,98,107,82,65,103,68,40,97,60,33,91,54,26,86,49,
    25,80,44,21,74,37,15,68,32,11,61,27,9,57,25,8,
    108,134,217,108,134,217,107,134,199,103,134,165,100,134,133,96,
    130,101,91,124,74,85,117,52,53,22,4,49,18,3,43,15,
    1,37,10,0,17,16,17,11,11,9,7,5,2,6,1,0,
    0,0,32,0,0,24,0,0,16,0,0,11,0,0,3,0,
    0,0,0,0,0,0,0,0,105,93,77,97,123,105,108,81,
    217,105,9,214,77,6,159,50,1,99,26,0,52,66,55,85
};

const byte C_D46D3D[] = {
    0,0,0,2,0,0,0,0,0,37,21,11,212,109,61,2,
    1,0,0,0,0,0,0,0,0,0,0,13,9,3,6,4,
    0,0,1,0,0,0,0,33,13,5,26,10,3,22,7,2,
    212,103,55,212,95,51,212,90,48,212,80,43,210,75,40,200,
    68,37,194,60,33,183,54,29,173,49,26,165,44,24,154,39,
    21,149,37,20,138,31,17,133,28,15,121,23,12,116,21,11,
    106,18,9,100,16,8,91,13,7,86,10,6,75,9,5,71,
    7,4,63,6,3,58,4,2,52,3,1,47,1,0,41,0,
    0,37,0,0,32,0,0,28,0,0,22,0,0,22,0,0,
    212,109,61,212,109,61,212,109,61,212,109,58,211,109,56,211,
    109,52,212,103,47,212,100,45,211,94,40,212,88,36,210,82,
    33,197,76,30,187,70,27,177,64,23,166,58,21,160,55,19,
    145,51,18,131,46,16,123,43,15,111,39,14,103,35,12,87,
    31,11,80,27,9,71,25,8,64,23,7,52,20,6,41,16,
    5,32,13,4,27,10,3,20,8,2,11,5,1,7,3,0,
    212,109,61,212,109,61,212,109,61,212,109,61,206,109,61,195,
    109,58,189,106,57,178,100,53,168,94,50,163,91,49,150,84,
    45,145,81,44,136,76,41,125,70,37,118,66,35,108,60,33,
    97,54,29,92,51,28,81,46,24,72,41,22,68,38,20,59,
    33,18,52,29,16,47,27,14,41,23,12,34,19,10,30,17,
    9,23,13,7,22,12,6,15,9,5,10,6,3,7,4,2,
    111,109,31,97,109,27,86,106,24,75,97,21,67,84,17,57,
    74,15,47,62,12,41,54,10,35,44,7,27,35,6,21,27,
    4,12,20,3,7,13,1,1,7,0,0,3,0,0,0,0,
    163,81,38,151,75,35,141,68,32,131,62,29,118,56,26,115,
    52,24,103,49,22,92,42,19,85,38,17,74,33,14,70,31,
    13,61,27,11,54,24,10,46,21,8,40,17,7,36,15,6,
    114,54,23,92,44,18,81,37,15,67,29,11,49,23,8,39,
    17,6,32,12,4,22,9,3,79,46,21,66,38,17,57,33,
    14,47,28,12,38,22,9,31,18,7,24,15,5,19,12,4,
    197,109,39,178,95,30,151,76,23,130,59,16,107,42,10,89,
    28,6,69,17,2,55,9,0,212,109,61,212,109,61,212,106,
    57,212,85,45,212,66,35,212,49,26,212,31,17,212,18,10,
    205,7,4,186,6,3,174,6,3,161,5,3,145,4,2,134,
    4,2,119,3,1,107,2,1,94,1,0,76,0,0,67,0,
    0,56,0,0,45,0,0,37,0,0,28,0,0,22,0,0,
    212,109,61,201,109,61,170,95,61,139,78,61,108,60,61,75,
    42,61,50,28,61,29,16,61,12,7,61,10,6,52,7,4,
    44,5,3,36,2,1,28,0,0,21,0,0,15,0,0,9,
    212,109,61,212,109,61,212,109,58,210,106,48,210,94,38,208,
    81,28,210,67,18,203,55,11,190,49,9,179,44,7,168,40,
    7,157,36,6,145,30,4,134,26,3,121,22,2,111,20,2,
    212,109,61,212,109,61,210,109,56,203,109,46,195,109,37,189,
    106,28,180,101,21,167,95,15,105,18,1,96,15,1,85,12,
    0,72,8,0,33,13,5,22,9,3,13,4,0,11,1,0,
    0,0,9,0,0,7,0,0,5,0,0,3,0,0,1,0,
    0,0,0,0,0,0,0,0,206,76,22,190,100,29,212,66,
    61,205,7,60,150,5,45,97,1,28,52,0,15,131,45,24
};

const byte C_04918B[] = {
    0,0,0,0,0,0,0,0,0,1,34,31,5,179,170,0,
    1,1,0,0,0,0,0,0,0,0,0,0,14,8,0,7,
    1,0,1,0,0,0,0,1,21,14,1,16,9,1,11,7,
    5,169,153,5,156,141,5,147,135,5,132,120,5,123,112,5,
    112,102,5,99,91,4,89,81,4,81,73,4,73,66,4,64,
    58,4,60,55,3,51,47,3,46,41,3,37,34,3,35,32,
    3,29,26,2,26,23,2,22,20,2,17,15,2,14,13,2,
    12,11,1,9,9,1,7,7,1,5,4,1,1,1,1,0,
    0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,
    5,179,170,5,179,170,5,179,170,5,179,163,5,179,155,5,
    179,145,5,169,132,5,165,126,5,154,111,5,145,100,5,135,
    91,5,125,83,4,115,74,4,105,65,4,95,58,4,90,54,
    3,84,50,3,75,45,3,70,41,3,65,38,2,58,33,2,
    51,30,2,44,25,2,41,23,2,37,21,1,32,17,1,27,
    14,1,22,11,1,17,9,0,13,6,0,8,2,0,6,1,
    5,179,170,5,179,170,5,179,170,5,179,170,5,179,170,5,
    179,163,4,173,158,4,164,149,4,154,140,4,150,136,4,138,
    126,3,133,121,3,124,113,3,114,104,3,109,99,3,99,91,
    2,89,81,2,84,77,2,75,68,2,67,61,2,62,57,1,
    54,49,1,48,43,1,44,40,1,37,34,1,32,29,1,27,
    25,1,21,19,1,20,18,0,14,13,0,9,9,0,7,7,
    3,179,87,2,179,76,2,174,67,2,159,58,2,138,49,1,
    122,41,1,102,34,1,89,29,1,73,21,1,58,18,0,44,
    12,0,32,7,0,21,4,0,12,0,0,5,0,0,0,0,
    4,133,107,4,123,97,3,112,88,3,102,81,3,92,71,3,
    86,66,2,80,60,2,69,52,2,62,46,2,54,40,2,51,
    37,1,44,31,1,40,29,1,34,22,1,27,19,1,25,17,
    3,88,63,2,72,49,2,61,41,2,48,32,1,37,23,1,
    28,18,1,20,11,1,15,8,2,75,58,2,62,47,1,54,
    40,1,46,34,1,36,25,1,30,20,1,25,15,0,20,11,
    5,179,109,4,156,84,4,125,64,3,96,44,3,69,29,2,
    46,15,2,28,6,1,14,1,5,179,170,5,179,170,5,173,
    158,5,139,127,5,108,98,5,81,73,5,51,47,5,29,27,
    5,12,11,4,10,9,4,9,9,4,8,7,3,7,7,3,
    6,6,3,5,4,3,3,3,2,1,1,2,0,0,2,0,
    0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,
    5,179,170,5,179,170,4,157,170,3,128,170,3,99,170,2,
    69,170,1,46,170,1,27,170,0,12,170,0,9,145,0,7,
    121,0,5,99,0,1,78,0,0,58,0,0,41,0,0,25,
    5,179,170,5,179,170,5,179,163,5,173,135,5,154,106,5,
    133,77,5,110,51,5,91,31,4,81,26,4,72,21,4,66,
    19,4,59,17,3,50,12,3,42,9,3,36,7,3,33,6,
    5,179,170,5,179,170,5,179,156,5,179,129,5,179,104,4,
    173,79,4,166,58,4,156,41,2,29,3,2,25,2,2,20,
    1,2,13,0,1,21,13,1,14,7,0,7,1,0,1,0,
    0,0,25,0,0,19,0,0,13,0,0,9,0,0,3,0,
    0,0,0,0,0,0,0,0,5,124,60,4,164,82,5,108,
    170,5,12,168,4,8,125,2,1,77,1,0,41,3,74,67
};

const byte C_FF3030[] = {
    0,0,0,2,0,0,0,0,0,44,9,9,255,48,48,2,
    0,0,0,0,0,0,0,0,0,0,0,16,4,2,7,2,
    0,0,0,0,0,0,0,40,6,4,31,4,3,26,3,2,
    255,45,43,255,42,40,255,40,38,255,35,34,253,33,32,241,
    30,29,233,27,26,220,24,23,208,22,21,198,20,19,185,17,
    16,179,16,15,166,14,13,160,12,12,145,10,10,140,9,9,
    128,8,7,120,7,7,109,6,6,103,5,4,90,4,4,85,
    3,3,76,2,2,70,2,2,62,1,1,57,0,0,49,0,
    0,45,0,0,39,0,0,34,0,0,27,0,0,26,0,0,
    255,48,48,255,48,48,255,48,48,255,48,46,254,48,44,254,
    48,41,255,45,37,255,44,36,254,41,31,255,39,28,252,36,
    26,237,34,23,225,31,21,213,28,18,200,25,16,193,24,15,
    175,23,14,158,20,13,148,19,12,134,17,11,124,15,9,105,
    14,8,96,12,7,86,11,6,77,10,6,62,9,5,49,7,
    4,39,6,3,32,5,2,24,3,2,13,2,1,9,2,0,
    255,48,48,255,48,48,255,48,48,255,48,48,248,48,48,235,
    48,46,227,46,45,214,44,42,202,41,40,196,40,38,181,37,
    36,175,36,34,163,33,32,150,31,29,142,29,28,130,27,26,
    117,24,23,111,23,22,98,20,19,87,18,17,82,17,16,71,
    14,14,62,13,12,57,12,11,49,10,10,41,8,8,36,7,
    7,28,6,5,26,5,5,18,4,4,12,2,2,9,2,2,
    133,48,25,117,48,21,104,47,19,90,43,16,81,37,14,69,
    33,12,57,27,10,49,24,8,42,20,6,32,15,5,25,12,
    3,15,9,2,9,6,1,1,3,0,0,1,0,0,0,0,
    196,36,30,182,33,27,170,30,25,157,27,23,142,25,20,138,
    23,19,124,21,17,111,19,15,102,17,13,89,14,11,84,14,
    10,73,12,9,65,11,8,55,9,6,48,7,5,43,7,5,
    137,24,18,111,19,14,97,16,11,80,13,9,59,10,7,47,
    8,5,38,5,3,27,4,2,95,20,16,79,17,13,68,14,
    11,57,12,10,46,10,7,37,8,6,29,7,4,23,5,3,
    237,48,31,214,42,24,182,34,18,156,26,12,129,19,8,107,
    12,4,83,8,2,66,4,0,255,48,48,255,48,48,255,46,
    45,255,37,36,255,29,28,255,22,21,255,14,13,255,8,8,
    247,3,3,224,3,3,209,2,2,194,2,2,175,2,2,161,
    2,2,143,1,1,129,1,1,113,0,0,92,0,0,80,0,
    0,67,0,0,54,0,0,44,0,0,34,0,0,26,0,0,
    255,48,48,242,48,48,205,42,48,167,34,48,130,27,48,90,
    18,48,60,12,48,35,7,48,15,3,48,12,2,41,9,2,
    34,6,1,28,2,0,22,0,0,16,0,0,11,0,0,7,
    255,48,48,255,48,48,255,48,46,252,46,38,252,41,30,250,
    36,22,252,29,14,244,24,9,228,22,7,215,19,6,202,18,
    5,189,16,5,174,13,3,161,11,2,145,10,2,134,9,2,
    255,48,48,255,48,48,252,48,44,244,48,37,235,48,29,227,
    46,22,216,44,16,201,42,11,126,8,1,115,7,1,102,5,
    0,87,4,0,40,6,4,27,4,2,16,2,0,13,0,0,
    0,0,7,0,0,5,0,0,4,0,0,2,0,0,1,0,
    0,0,0,0,0,0,0,0,248,33,17,228,44,23,255,29,
    48,247,3,47,181,2,35,117,0,22,62,0,11,157,20,19
};

const byte C_311A59[] = {
    0,0,0,1,0,0,0,0,0,21,8,39,122,44,218,1,
    0,2,0,0,0,0,0,0,0,0,0,8,3,10,3,2,
    2,0,0,0,0,0,0,19,5,18,15,4,12,12,3,9,
    122,42,197,122,38,181,122,36,173,122,32,154,121,30,144,115,
    27,131,111,24,116,105,22,104,100,20,94,95,18,85,89,16,
    74,86,15,70,79,13,60,77,11,53,69,9,44,67,9,41,
    61,7,33,57,6,30,52,5,26,49,4,20,43,3,16,41,
    3,14,36,2,11,33,2,9,30,1,5,27,0,2,23,0,
    0,22,0,0,19,0,0,16,0,0,13,0,0,12,0,0,
    122,44,218,122,44,218,122,44,218,122,44,209,122,44,199,122,
    44,186,122,42,169,122,41,162,122,38,142,122,36,128,121,33,
    117,113,31,106,108,28,95,102,26,83,96,23,74,92,22,69,
    84,21,64,76,18,57,71,17,53,64,16,49,59,14,43,50,
    13,38,46,11,32,41,10,29,37,9,27,30,8,22,23,7,
    18,19,5,15,15,4,11,11,3,8,6,2,3,4,1,2,
    122,44,218,122,44,218,122,44,218,122,44,218,119,44,218,112,
    44,209,109,43,203,102,40,191,97,38,180,94,37,174,87,34,
    162,84,33,156,78,31,145,72,28,133,68,27,127,62,24,116,
    56,22,104,53,21,98,47,18,87,42,16,78,39,15,73,34,
    13,63,30,12,56,27,11,51,23,9,44,20,8,37,17,7,
    32,13,5,25,12,5,23,9,3,16,6,2,11,4,2,9,
    64,44,112,56,44,97,50,43,85,43,39,74,39,34,62,33,
    30,53,27,25,44,23,22,37,20,18,27,15,14,23,12,11,
    15,7,8,9,4,5,5,0,3,0,0,1,0,0,0,0,
    94,33,137,87,30,125,81,27,113,75,25,103,68,23,91,66,
    21,85,59,20,77,53,17,67,49,15,59,43,13,51,40,12,
    47,35,11,40,31,10,37,26,8,28,23,7,24,21,6,21,
    66,22,81,53,18,63,46,15,52,38,12,41,28,9,30,22,
    7,23,18,5,15,13,4,10,45,18,74,38,15,60,33,13,
    51,27,11,44,22,9,32,18,7,26,14,6,19,11,5,15,
    113,44,140,102,38,108,87,31,82,75,24,56,62,17,37,51,
    11,20,40,7,8,32,3,1,122,44,218,122,44,218,122,43,
    203,122,34,162,122,27,126,122,20,94,122,13,60,122,7,35,
    118,3,14,107,2,12,100,2,11,93,2,9,84,2,9,77,
    2,8,68,1,5,62,1,3,54,0,2,44,0,0,38,0,
    0,32,0,0,26,0,0,21,0,0,16,0,0,12,0,0,
    122,44,218,116,44,218,98,38,218,80,31,218,62,24,218,43,
    17,218,29,11,218,17,7,218,7,3,218,6,2,186,4,2,
    156,3,1,127,1,0,100,0,0,74,0,0,52,0,0,32,
    122,44,218,122,44,218,122,44,209,121,43,173,121,38,136,120,
    33,98,121,27,65,117,22,40,109,20,33,103,18,27,97,16,
    25,90,14,21,83,12,15,77,10,11,69,9,9,64,8,8,
    122,44,218,122,44,218,121,44,200,117,44,166,112,44,133,109,
    43,102,103,41,74,96,38,52,60,7,4,55,6,3,49,5,
    1,42,3,0,19,5,17,13,3,9,8,2,2,6,0,0,
    0,0,32,0,0,24,0,0,16,0,0,11,0,0,3,0,
    0,0,0,0,0,0,0,0,119,31,77,109,40,105,122,27,
    218,118,3,215,87,2,160,56,0,99,30,0,52,75,18,85
};

const byte C_FFAFAF[] = {
    0,0,0,2,0,0,0,0,0,44,33,32,255,175,175,2,
    1,1,0,0,0,0,0,0,0,0,0,16,14,8,7,7,
    1,0,1,0,0,0,0,40,21,14,31,16,10,26,11,7,
    255,165,158,255,152,145,255,144,139,255,129,124,253,120,115,241,
    109,105,233,97,93,220,87,84,208,79,75,198,71,68,185,62,
    60,179,59,56,166,50,48,160,45,43,145,36,35,140,34,33,
    128,28,27,120,25,24,109,21,21,103,16,16,90,14,13,85,
    12,11,76,9,9,70,7,7,62,5,4,57,1,1,49,0,
    0,45,0,0,39,0,0,34,0,0,27,0,0,26,0,0,
    255,175,175,255,175,175,255,175,175,255,175,167,254,175,160,254,
    175,149,255,165,136,255,161,130,254,151,114,255,142,103,252,132,
    94,237,122,85,225,113,76,213,102,67,200,93,60,193,88,56,
    175,82,51,158,73,46,148,69,43,134,63,39,124,56,34,105,
    50,31,96,43,26,86,40,23,77,36,21,62,32,18,49,26,
    14,39,21,12,32,16,9,24,12,6,13,8,2,9,5,1,
    255,175,175,255,175,175,255,175,175,255,175,175,248,175,175,235,
    175,167,227,170,163,214,160,153,202,150,144,196,146,140,181,135,
    130,175,130,125,163,121,117,150,112,107,142,106,102,130,97,93,
    117,87,84,111,82,79,98,73,70,87,65,62,82,61,58,71,
    53,51,62,47,45,57,43,41,49,36,35,41,31,30,36,27,
    25,28,21,20,26,19,19,18,14,13,12,9,9,9,7,7,
    133,175,90,117,175,78,104,170,69,90,155,60,81,135,50,69,
    119,43,57,100,35,49,87,30,42,71,21,32,56,19,25,43,
    12,15,32,8,9,21,4,1,12,0,0,5,0,0,0,0,
    196,130,110,182,120,100,170,109,91,157,100,83,142,90,73,138,
    84,68,124,78,62,111,68,54,102,61,47,89,53,41,84,49,
    38,73,43,32,65,39,30,55,33,23,48,27,19,43,24,17,
    137,86,65,111,70,51,97,60,42,80,47,33,59,36,24,47,
    27,19,38,20,12,27,14,8,95,73,60,79,61,48,68,53,
    41,57,45,35,46,35,25,37,30,21,29,24,15,23,19,12,
    237,175,113,214,152,86,182,122,66,156,94,45,129,68,30,107,
    45,16,83,27,6,66,14,1,255,175,175,255,175,175,255,170,
    163,255,136,130,255,106,101,255,79,75,255,50,48,255,29,28,
    247,12,11,224,10,10,209,9,9,194,8,8,175,7,7,161,
    6,6,143,5,4,129,3,3,113,1,1,92,0,0,80,0,
    0,67,0,0,54,0,0,44,0,0,34,0,0,26,0,0,
    255,175,175,242,175,175,205,153,175,167,125,175,130,97,175,90,
    67,175,60,45,175,35,26,175,15,12,175,12,9,150,9,7,
    125,6,5,102,2,1,80,0,0,60,0,0,42,0,0,26,
    255,175,175,255,175,175,255,175,167,252,170,139,252,150,109,250,
    130,79,252,107,52,244,89,32,228,79,27,215,70,21,202,65,
    20,189,58,17,174,49,12,161,41,9,145,35,7,134,32,6,
    255,175,175,255,175,175,252,175,161,244,175,133,235,175,107,227,
    170,82,216,162,60,201,152,42,126,29,3,115,24,2,102,20,
    1,87,13,0,40,21,14,27,14,8,16,7,1,13,1,0,
    0,0,26,0,0,19,0,0,13,0,0,9,0,0,3,0,
    0,0,0,0,0,0,0,0,248,121,62,228,161,84,255,106,
    175,247,12,173,181,8,128,117,1,80,62,0,42,157,72,69
};

const byte C_ECB866[] = {
    0,0,0,2,0,0,0,0,0,41,35,18,236,184,102,2,
    1,1,0,0,0,0,0,0,0,0,0,15,14,5,6,7,
    1,0,1,0,0,0,0,37,22,8,29,17,6,24,12,4,
    236,174,92,236,160,85,236,152,81,236,136,72,234,126,67,223,
    115,61,216,102,54,204,92,49,193,83,44,183,75,40,171,66,
    35,166,62,33,154,53,28,148,47,25,134,38,20,130,36,19,
    118,30,16,111,27,14,101,22,12,95,17,9,83,14,8,79,
    12,6,70,9,5,65,7,4,57,5,2,53,1,1,45,0,
    0,42,0,0,36,0,0,31,0,0,25,0,0,24,0,0,
    236,184,102,236,184,102,236,184,102,236,184,98,235,184,93,235,
    184,87,236,174,79,236,170,76,235,159,66,236,149,60,233,139,
    55,219,128,50,208,118,44,197,108,39,185,97,35,179,92,32,
    162,87,30,146,77,27,137,72,25,124,66,23,115,59,20,97,
    53,18,89,45,15,80,42,14,71,38,12,57,33,10,45,27,
    8,36,22,7,30,17,5,22,13,4,12,9,1,8,6,1,
    236,184,102,236,184,102,236,184,102,236,184,102,230,184,102,217,
    184,98,210,178,95,198,168,89,187,158,84,181,154,82,168,142,
    76,162,137,73,151,128,68,139,118,62,131,112,59,120,102,54,
    108,92,49,103,87,46,91,77,41,81,69,36,76,64,34,66,
    56,30,57,49,26,53,45,24,45,38,20,38,32,17,33,28,
    15,26,22,12,24,20,11,17,14,8,11,9,5,8,7,4,
    123,184,52,108,184,46,96,179,40,83,163,35,75,142,29,64,
    126,25,53,105,20,45,92,17,39,75,12,30,59,11,23,45,
    7,14,33,4,8,22,2,1,12,0,0,5,0,0,0,0,
    181,136,64,168,126,58,157,115,53,145,105,48,131,95,43,128,
    88,40,115,82,36,103,71,31,94,64,28,82,56,24,78,52,
    22,68,45,19,60,41,17,51,35,13,44,28,11,40,25,10,
    127,91,38,103,74,30,90,63,24,74,49,19,55,38,14,43,
    29,11,35,21,7,25,15,5,88,77,35,73,64,28,63,56,
    24,53,48,20,43,37,15,34,31,12,27,25,9,21,20,7,
    219,184,66,198,160,50,168,128,38,144,99,26,119,71,17,99,
    47,9,77,29,4,61,14,0,236,184,102,236,184,102,236,178,
    95,236,143,76,236,111,59,236,83,44,236,53,28,236,30,16,
    229,12,6,207,10,6,193,9,5,180,8,4,162,7,4,149,
    6,4,132,5,2,119,3,2,105,1,1,85,0,0,74,0,
    0,62,0,0,50,0,0,41,0,0,31,0,0,24,0,0,
    236,184,102,224,184,102,190,161,102,155,131,102,120,102,102,83,
    71,102,56,48,102,32,27,102,14,12,102,11,9,87,8,7,
    73,6,5,60,2,1,47,0,0,35,0,0,24,0,0,15,
    236,184,102,236,184,102,236,184,98,233,178,81,233,158,64,231,
    137,46,233,113,30,226,93,19,211,83,16,199,74,12,187,68,
    12,175,61,10,161,51,7,149,43,5,134,37,4,124,34,4,
    236,184,102,236,184,102,233,184,94,226,184,78,217,184,62,210,
    178,48,200,170,35,186,160,24,117,30,2,106,25,1,94,21,
    0,81,14,0,37,22,8,25,14,4,15,7,1,12,1,0,
    0,0,15,0,0,11,0,0,8,0,0,5,0,0,2,0,
    0,0,0,0,0,0,0,0,230,128,36,211,169,49,236,111,
    102,229,12,101,168,8,75,108,1,46,57,0,24,145,76,40
};

const byte C_C63F23[] = {
    0,0,0,2,0,0,0,0,0,34,12,6,198,63,35,2,
    0,0,0,0,0,0,0,0,0,0,0,12,5,2,5,2,
    0,0,0,0,0,0,0,31,7,3,24,6,2,20,4,1,
    198,60,32,198,55,29,198,52,28,198,46,25,196,43,23,187,
    39,21,181,35,19,171,31,17,162,28,15,154,26,14,144,22,
    12,139,21,11,129,18,10,124,16,9,113,13,7,109,12,7,
    99,10,5,93,9,5,85,8,4,80,6,3,70,5,3,66,
    4,2,59,3,2,54,2,1,48,2,1,44,0,0,38,0,
    0,35,0,0,30,0,0,26,0,0,21,0,0,20,0,0,
    198,63,35,198,63,35,198,63,35,198,63,33,197,63,32,197,
    63,30,198,60,27,198,58,26,197,54,23,198,51,21,196,47,
    19,184,44,17,175,41,15,165,37,13,155,33,12,150,32,11,
    136,30,10,123,26,9,115,25,9,104,23,8,96,20,7,82,
    18,6,75,16,5,67,14,5,60,13,4,48,11,4,38,9,
    3,30,8,2,25,6,2,19,4,1,10,3,0,7,2,0,
    198,63,35,198,63,35,198,63,35,198,63,35,193,63,35,182,
    63,33,176,61,33,166,58,31,157,54,29,152,53,28,141,49,
    26,136,47,25,127,44,23,116,40,21,110,38,20,101,35,19,
    91,31,17,86,30,16,76,26,14,68,23,12,64,22,12,55,
    19,10,48,17,9,44,15,8,38,13,7,32,11,6,28,10,
    5,22,7,4,20,7,4,14,5,3,9,3,2,7,2,1,
    103,63,18,91,63,16,81,61,14,70,56,12,63,49,10,54,
    43,9,44,36,7,38,31,6,33,26,4,25,20,4,19,16,
    2,12,11,2,7,7,1,1,4,0,0,2,0,0,0,0,
    152,47,22,141,43,20,132,39,18,122,36,17,110,32,15,107,
    30,14,96,28,12,86,24,11,79,22,9,69,19,8,65,18,
    8,57,15,6,50,14,6,43,12,5,37,10,4,33,9,3,
    106,31,13,86,25,10,75,21,8,62,17,7,46,13,5,36,
    10,4,30,7,2,21,5,2,74,26,12,61,22,10,53,19,
    8,44,16,7,36,13,5,29,11,4,23,9,3,18,7,2,
    184,63,23,166,55,17,141,44,13,121,34,9,100,24,6,83,
    16,3,64,10,1,51,5,0,198,63,35,198,63,35,198,61,
    33,198,49,26,198,38,20,198,28,15,198,18,10,198,10,6,
    192,4,2,174,3,2,162,3,2,151,3,2,136,2,1,125,
    2,1,111,2,1,100,1,1,88,0,0,71,0,0,62,0,
    0,52,0,0,42,0,0,34,0,0,26,0,0,20,0,0,
    198,63,35,188,63,35,159,55,35,130,45,35,101,35,35,70,
    24,35,47,16,35,27,9,35,12,4,35,9,3,30,7,2,
    25,5,2,20,2,0,16,0,0,12,0,0,8,0,0,5,
    198,63,35,198,63,35,198,63,33,196,61,28,196,54,22,194,
    47,16,196,39,10,189,32,6,177,28,5,167,25,4,157,23,
    4,147,21,3,135,18,2,125,15,2,113,13,1,104,12,1,
    198,63,35,198,63,35,196,63,32,189,63,27,182,63,21,176,
    61,16,168,58,12,156,55,8,98,10,1,89,9,0,79,7,
    0,68,5,0,31,7,3,21,5,2,12,2,0,10,0,0,
    0,0,5,0,0,4,0,0,3,0,0,2,0,0,1,0,
    0,0,0,0,0,0,0,0,193,44,12,177,58,17,198,38,
    35,192,4,35,141,3,26,91,0,16,48,0,8,122,26,14
};

const byte C_9BC8CD[] = {
    0,0,0,1,0,0,0,0,0,27,38,37,155,200,205,1,
    2,2,0,0,0,0,0,0,0,0,0,10,16,10,4,8,
    2,0,2,0,0,0,0,24,24,17,19,18,11,16,13,8,
    155,189,185,155,174,170,155,165,162,155,147,145,154,137,135,146,
    125,123,142,111,109,134,100,98,126,90,88,120,82,80,112,71,
    70,109,67,66,101,57,56,97,51,50,88,42,41,85,39,39,
    78,32,31,73,29,28,66,24,24,63,19,18,55,16,15,52,
    13,13,46,10,10,43,8,8,38,5,5,35,2,2,30,0,
    0,27,0,0,24,0,0,21,0,0,16,0,0,16,0,0,
    155,200,205,155,200,205,155,200,205,155,200,196,154,200,187,154,
    200,174,155,189,159,155,184,152,154,173,133,155,162,121,153,151,
    110,144,140,100,137,129,89,129,117,78,122,106,70,117,100,65,
    106,94,60,96,84,54,90,78,50,81,72,46,75,64,40,64,
    57,36,58,49,31,52,45,27,47,42,25,38,36,21,30,30,
    17,24,24,14,19,19,10,15,14,7,8,9,2,5,6,2,
    155,200,205,155,200,205,155,200,205,155,200,205,151,200,205,143,
    200,196,138,194,191,130,183,179,123,172,169,119,167,164,110,155,
    152,106,149,146,99,139,137,91,128,125,86,122,119,79,111,109,
    71,100,98,67,94,92,60,84,82,53,75,73,50,70,68,43,
    60,59,38,53,52,35,49,48,30,42,41,25,35,35,22,31,
    30,17,24,23,16,22,22,11,16,15,7,10,10,5,8,8,
    81,200,105,71,200,92,63,195,80,55,177,70,49,155,59,42,
    136,50,35,115,41,30,100,35,26,82,25,19,64,22,15,49,
    14,9,36,9,5,24,5,1,13,0,0,5,0,0,0,0,
    119,148,129,111,137,117,103,125,106,95,115,97,86,103,86,84,
    96,80,75,89,72,67,78,63,62,70,55,54,60,48,51,56,
    44,44,49,38,40,45,35,33,38,27,29,31,23,26,27,20,
    83,99,76,67,80,59,59,68,49,49,53,39,36,42,28,29,
    31,22,23,23,14,16,16,10,58,84,70,48,70,56,41,60,
    48,35,52,41,28,40,30,22,34,24,18,27,18,14,22,14,
    144,200,132,130,174,101,111,140,77,95,107,53,78,78,35,65,
    51,18,50,31,7,40,16,1,155,200,205,155,200,205,155,194,
    191,155,155,153,155,121,118,155,90,88,155,57,56,155,33,33,
    150,13,13,136,11,11,127,10,10,118,9,9,106,8,8,98,
    7,7,87,5,5,78,3,3,69,2,2,56,0,0,49,0,
    0,41,0,0,33,0,0,27,0,0,21,0,0,16,0,0,
    155,200,205,147,200,205,125,175,205,102,143,205,79,111,205,55,
    77,205,36,52,205,21,30,205,9,13,205,7,10,175,5,8,
    146,4,5,120,1,2,94,0,0,70,0,0,49,0,0,31,
    155,200,205,155,200,205,155,200,196,153,194,162,153,172,128,152,
    149,92,153,122,61,148,101,38,139,90,31,131,80,25,123,74,
    23,115,66,20,106,56,14,98,47,10,88,40,8,81,37,7,
    155,200,205,155,200,205,153,200,188,148,200,156,143,200,125,138,
    194,96,131,185,70,122,174,49,77,33,4,70,27,2,62,23,
    1,53,15,0,24,24,16,16,16,9,10,8,2,8,2,0,
    0,0,31,0,0,23,0,0,15,0,0,10,0,0,3,0,
    0,0,0,0,0,0,0,0,151,139,72,139,184,99,155,121,
    205,150,13,203,110,9,150,71,2,93,38,0,49,95,82,80
};

const byte C_666666[] = {
    0,0,0,1,0,0,0,0,0,30,64,31,255,255,255,1,
    3,1,0,0,0,0,0,0,0,0,0,11,27,8,5,13,
    1,0,3,0,0,0,0,27,40,14,21,31,9,17,21,7,
    255,246,222,255,233,198,255,225,185,255,210,155,252,201,139,236,
    191,119,226,179,97,208,169,82,193,153,74,179,138,66,162,121,
    58,154,114,55,137,97,47,129,86,42,109,70,34,102,66,32,
    86,55,26,80,49,23,73,41,20,69,32,15,60,27,13,57,
    23,11,51,17,9,47,13,7,42,9,4,38,3,1,33,0,
    0,30,0,0,26,0,0,23,0,0,18,0,0,17,0,0,
    255,255,255,255,255,255,255,255,255,255,255,240,254,255,226,254,
    255,204,255,246,179,255,242,167,254,232,137,255,223,115,251,213,
    98,231,203,83,215,194,74,199,184,65,182,175,58,173,170,54,
    149,160,50,126,142,45,113,133,42,94,122,38,83,109,34,70,
    97,30,64,84,25,58,77,23,52,70,21,42,61,17,33,51,
    14,26,41,11,21,32,9,16,24,6,9,16,2,6,11,1,
    255,255,255,255,255,255,255,255,255,255,255,255,246,255,255,228,
    255,240,218,250,231,200,240,212,185,231,195,177,227,187,157,216,
    167,149,211,158,133,203,142,115,193,123,105,188,113,89,179,97,
    78,169,82,74,160,77,66,142,68,58,126,61,55,118,57,48,
    102,50,42,90,44,38,82,40,33,70,34,27,60,29,24,52,
    25,19,40,19,17,37,18,12,27,13,8,17,9,6,13,7,
    93,255,90,78,255,76,70,250,67,60,236,58,54,216,49,46,
    201,42,38,182,34,33,169,29,28,138,21,21,109,18,17,84,
    12,10,61,7,6,40,4,1,23,0,0,9,0,0,0,0,
    177,211,129,158,201,110,142,191,91,125,182,81,105,172,72,99,
    162,66,83,152,60,74,132,52,68,118,46,60,102,40,56,96,
    37,49,82,32,44,76,29,37,64,22,32,52,19,29,47,17,
    98,168,64,74,136,50,65,116,41,54,90,32,40,70,23,32,
    53,18,25,39,11,18,28,8,64,142,58,53,118,47,46,102,
    40,38,88,34,31,68,25,25,57,20,19,47,15,15,37,11,
    231,255,134,200,233,84,158,203,64,123,176,44,87,132,29,72,
    86,15,56,53,6,44,27,1,255,255,255,255,255,255,255,250,
    231,255,217,169,255,187,111,255,153,74,255,97,47,255,56,27,
    244,23,11,214,19,9,194,17,9,174,15,7,149,13,7,130,
    12,6,106,9,4,87,5,3,76,3,1,62,0,0,54,0,
    0,45,0,0,36,0,0,30,0,0,23,0,0,17,0,0,
    255,255,255,238,255,255,189,234,255,138,206,255,89,179,255,60,
    130,255,40,88,255,23,51,255,10,23,255,8,17,206,6,13,
    158,4,9,114,1,3,78,0,0,58,0,0,41,0,0,25,
    255,255,255,255,255,255,255,255,240,251,250,185,251,231,127,248,
    211,77,251,189,51,240,171,32,219,153,26,202,136,21,185,125,
    19,167,112,17,147,94,12,130,80,9,109,68,7,94,62,6,
    255,255,255,255,255,255,251,255,227,240,255,174,228,255,123,218,
    250,80,203,242,58,183,233,41,84,56,3,77,47,2,68,39,
    1,58,25,0,27,40,13,18,27,7,11,13,1,9,3,0,
    0,0,25,0,0,19,0,0,13,0,0,9,0,0,3,0,
    0,0,0,0,0,0,0,0,246,203,60,219,241,82,255,187,
    255,244,23,251,157,15,165,78,3,78,42,0,41,125,140,67
};

const byte C_777777[] = {
    0,0,0,3,0,0,0,0,0,63,27,26,255,255,255,3,
    1,1,0,0,0,0,0,0,0,0,0,23,11,7,10,6,
    1,0,1,0,0,0,0,58,17,12,45,13,8,37,9,6,
    255,235,219,255,208,193,255,190,179,255,159,147,254,140,130,247,
    117,108,243,91,84,235,71,68,229,64,62,223,58,56,216,51,
    49,212,48,46,205,41,39,202,36,35,193,30,29,191,28,27,
    184,23,22,173,21,20,157,17,17,148,13,13,130,11,11,122,
    10,9,109,7,7,101,6,6,89,4,3,82,1,1,71,0,
    0,65,0,0,56,0,0,49,0,0,39,0,0,37,0,0,
    255,255,255,255,255,255,255,255,255,255,255,239,254,255,223,254,
    255,200,255,235,173,255,226,160,254,205,127,255,186,104,253,164,
    85,245,144,70,238,124,62,231,102,54,224,82,49,220,72,45,
    210,67,42,201,60,38,195,56,35,187,52,32,178,46,28,151,
    41,25,138,35,21,124,33,19,111,30,17,89,26,15,71,21,
    12,56,17,10,46,13,7,35,10,5,19,7,2,13,4,1,
    255,255,255,255,255,255,255,255,255,255,255,255,251,255,255,244,
    255,239,239,243,229,232,223,209,225,203,190,222,195,182,214,172,
    160,210,161,150,203,143,133,196,123,113,192,111,101,185,91,84,
    168,71,68,160,67,64,141,60,57,125,53,51,118,50,48,102,
    43,41,89,38,36,82,35,34,71,30,29,59,25,24,52,22,
    21,40,17,16,37,16,15,26,11,11,17,7,7,13,6,6,
    187,255,77,168,255,64,150,245,56,130,213,49,117,172,41,99,
    138,35,82,98,29,71,71,24,60,58,17,46,46,15,36,35,
    10,22,26,6,13,17,3,1,10,0,0,4,0,0,0,0,
    222,160,118,214,140,98,207,117,78,200,98,68,192,77,60,189,
    68,56,178,64,50,160,56,44,147,50,39,128,43,34,121,40,
    31,105,35,26,94,32,24,79,27,19,69,22,16,62,20,14,
    189,71,53,160,57,41,140,49,34,115,38,27,85,30,20,68,
    22,15,55,16,10,39,12,7,137,60,49,114,50,39,98,43,
    34,82,37,29,66,29,21,53,24,17,42,20,12,33,16,10,
    245,255,124,232,208,71,214,144,54,199,85,37,184,56,24,154,
    36,13,119,22,5,95,11,1,255,255,255,255,255,255,255,243,
    229,255,173,161,255,110,100,255,64,62,255,41,39,255,24,23,
    251,10,9,238,8,8,229,7,7,221,6,6,210,6,6,202,
    5,5,192,4,3,184,2,2,163,1,1,132,0,0,115,0,
    0,96,0,0,78,0,0,63,0,0,49,0,0,37,0,0,
    255,255,255,248,255,255,227,209,255,206,150,255,185,91,255,130,
    55,255,86,37,255,50,21,255,22,10,255,17,7,202,13,6,
    150,9,4,102,3,1,66,0,0,49,0,0,34,0,0,21,
    255,255,255,255,255,255,255,255,239,253,243,179,253,203,117,252,
    161,64,253,113,43,249,74,26,240,64,22,233,57,17,225,53,
    16,218,47,14,210,40,10,202,34,7,193,29,6,187,26,5,
    255,255,255,255,255,255,253,255,225,249,255,167,244,255,113,239,
    243,67,233,228,49,225,208,34,181,24,3,166,20,2,147,16,
    1,125,11,0,58,17,11,39,11,6,23,6,1,19,1,0,
    0,0,21,0,0,16,0,0,11,0,0,7,0,0,2,0,
    0,0,0,0,0,0,0,0,251,143,50,240,225,69,255,110,
    255,251,10,251,214,6,157,168,1,65,89,0,34,200,59,56
};
