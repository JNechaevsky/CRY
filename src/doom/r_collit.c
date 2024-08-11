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
static lighttable_t   *colormaps_2B3BFF;  // Bright saturated blue
static lighttable_t   *colormaps_50ADAC;  // Middle cyan (also 31A29F)
static lighttable_t   *colormaps_CCE4A5;  // Middle green-yellow
static lighttable_t   *colormaps_CCEA5F;  // Bright green-yellow
static lighttable_t   *colormaps_B30202;  // Middle red
static lighttable_t   *colormaps_B87A15;  // Middle orange
static lighttable_t   *colormaps_FFD000;  // Middle yellow
static lighttable_t   *colormaps_FFDE4C;  // Middle-bright yellow
static lighttable_t   *colormaps_FFF588;  // Bright yellow
static lighttable_t   *colormaps_3089FF;  // Bright cyanic blue
static lighttable_t   *colormaps_A88139;  // Middle brown
static lighttable_t   *colormaps_7084C4;  // Dark cyanic blue 2
static lighttable_t   *colormaps_D46D3D;  // Middle orange 2
static lighttable_t   *colormaps_05A8A0;  // Middle saturated cyan
static lighttable_t   *colormaps_FF3030;  // Bright saturated red
static lighttable_t   *colormaps_6435B5;  // Un-darked magenta
static lighttable_t   *colormaps_FFAFAF;  // Brighter red (also FFCECE)
static lighttable_t   *colormaps_ECB866;  // Bright orange
static lighttable_t   *colormaps_C63F23;  // Middle orange 3
static lighttable_t   *colormaps_9BC8CD;  // Bright cyan (also 4F5D8B)
static lighttable_t   *colormaps_666666;  // Special green (00FF00, overlay 33%)
static lighttable_t   *colormaps_777777;  // Special red (FF0000, overlay 55%)

// Doom II
static lighttable_t   *colormaps_CEA98B;  // Brownie starport sky
static lighttable_t   *colormaps_FFCD5A;  // Bright yellow
static lighttable_t   *colormaps_AC785B;  // Middle brownie


// Visplane light tables
static lighttable_t ***zlight_EEC06B = NULL;
static lighttable_t ***zlight_D97C45 = NULL;
static lighttable_t ***zlight_FF7F7F = NULL;
static lighttable_t ***zlight_55B828 = NULL;
static lighttable_t ***zlight_BBE357 = NULL;
static lighttable_t ***zlight_949DB9 = NULL;
static lighttable_t ***zlight_2B3BFF = NULL;
static lighttable_t ***zlight_50ADAC = NULL;
static lighttable_t ***zlight_CCE4A5 = NULL;
static lighttable_t ***zlight_CCEA5F = NULL;
static lighttable_t ***zlight_B30202 = NULL;
static lighttable_t ***zlight_B87A15 = NULL;
static lighttable_t ***zlight_FFD000 = NULL;
static lighttable_t ***zlight_FFDE4C = NULL;
static lighttable_t ***zlight_FFF588 = NULL;
static lighttable_t ***zlight_3089FF = NULL;
static lighttable_t ***zlight_A88139 = NULL;
static lighttable_t ***zlight_7084C4 = NULL;
static lighttable_t ***zlight_D46D3D = NULL;
static lighttable_t ***zlight_05A8A0 = NULL;
static lighttable_t ***zlight_FF3030 = NULL;
static lighttable_t ***zlight_6435B5 = NULL;
static lighttable_t ***zlight_FFAFAF = NULL;
static lighttable_t ***zlight_ECB866 = NULL;
static lighttable_t ***zlight_C63F23 = NULL;
static lighttable_t ***zlight_9BC8CD = NULL;
static lighttable_t ***zlight_666666 = NULL;
static lighttable_t ***zlight_777777 = NULL;

static lighttable_t ***zlight_CEA98B = NULL;
static lighttable_t ***zlight_FFCD5A = NULL;
static lighttable_t ***zlight_AC785B = NULL;

// Segment/sprite light tables
static lighttable_t ***scalelight_EEC06B = NULL;
static lighttable_t ***scalelight_D97C45 = NULL;
static lighttable_t ***scalelight_FF7F7F = NULL;
static lighttable_t ***scalelight_55B828 = NULL;
static lighttable_t ***scalelight_BBE357 = NULL;
static lighttable_t ***scalelight_949DB9 = NULL;
static lighttable_t ***scalelight_2B3BFF = NULL;
static lighttable_t ***scalelight_50ADAC = NULL;
static lighttable_t ***scalelight_CCE4A5 = NULL;
static lighttable_t ***scalelight_CCEA5F = NULL;
static lighttable_t ***scalelight_B30202 = NULL;
static lighttable_t ***scalelight_B87A15 = NULL;
static lighttable_t ***scalelight_FFD000 = NULL;
static lighttable_t ***scalelight_FFDE4C = NULL;
static lighttable_t ***scalelight_FFF588 = NULL;
static lighttable_t ***scalelight_3089FF = NULL;
static lighttable_t ***scalelight_A88139 = NULL;
static lighttable_t ***scalelight_7084C4 = NULL;
static lighttable_t ***scalelight_D46D3D = NULL;
static lighttable_t ***scalelight_05A8A0 = NULL;
static lighttable_t ***scalelight_FF3030 = NULL;
static lighttable_t ***scalelight_6435B5 = NULL;
static lighttable_t ***scalelight_FFAFAF = NULL;
static lighttable_t ***scalelight_ECB866 = NULL;
static lighttable_t ***scalelight_C63F23 = NULL;
static lighttable_t ***scalelight_9BC8CD = NULL;
static lighttable_t ***scalelight_666666 = NULL;
static lighttable_t ***scalelight_777777 = NULL;

static lighttable_t ***scalelight_CEA98B = NULL;
static lighttable_t ***scalelight_FFCD5A = NULL;
static lighttable_t ***scalelight_AC785B = NULL;

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
    colormaps_2B3BFF = malloc(lighttable_size);
    colormaps_50ADAC = malloc(lighttable_size);
    colormaps_CCE4A5 = malloc(lighttable_size);
    colormaps_CCEA5F = malloc(lighttable_size);
    colormaps_B30202 = malloc(lighttable_size);
    colormaps_B87A15 = malloc(lighttable_size);
    colormaps_FFD000 = malloc(lighttable_size);
    colormaps_FFDE4C = malloc(lighttable_size);
    colormaps_FFF588 = malloc(lighttable_size);
    colormaps_3089FF = malloc(lighttable_size);
    colormaps_A88139 = malloc(lighttable_size);
    colormaps_7084C4 = malloc(lighttable_size);
    colormaps_D46D3D = malloc(lighttable_size);
    colormaps_05A8A0 = malloc(lighttable_size);
    colormaps_FF3030 = malloc(lighttable_size);
    colormaps_6435B5 = malloc(lighttable_size);
    colormaps_FFAFAF = malloc(lighttable_size);
    colormaps_ECB866 = malloc(lighttable_size);
    colormaps_C63F23 = malloc(lighttable_size);
    colormaps_9BC8CD = malloc(lighttable_size);
    colormaps_666666 = malloc(lighttable_size);
    colormaps_777777 = malloc(lighttable_size);
    
    colormaps_CEA98B = malloc(lighttable_size);
    colormaps_FFCD5A = malloc(lighttable_size);
    colormaps_AC785B = malloc(lighttable_size);
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
    const byte *C_EEC06B;
    const byte *C_D97C45;
    const byte *C_FF7F7F;
    const byte *C_55B828;
    const byte *C_BBE357;
    const byte *C_949DB9;
    const byte *C_2B3BFF;
    const byte *C_50ADAC;
    const byte *C_CCE4A5;
    const byte *C_CCEA5F;
    const byte *C_B30202;
    const byte *C_B87A15;
    const byte *C_FFD000;
    const byte *C_FFDE4C;
    const byte *C_FFF588;
    const byte *C_3089FF;
    const byte *C_A88139;
    const byte *C_7084C4;
    const byte *C_D46D3D;
    const byte *C_05A8A0;
    const byte *C_FF3030;
    const byte *C_6435B5;
    const byte *C_FFAFAF;
    const byte *C_ECB866;
    const byte *C_C63F23;
    const byte *C_9BC8CD;
    const byte *C_666666;
    const byte *C_777777;
    
    const byte *C_CEA98B;
    const byte *C_FFCD5A;
    const byte *C_AC785B;

    if (dp_cry_palette)
    {
        C_EEC06B = C_EEC06B_CRY;
        C_D97C45 = C_D97C45_CRY;
        C_FF7F7F = C_FF7F7F_CRY;
        C_55B828 = C_55B828_CRY;
        C_BBE357 = C_BBE357_CRY;
        C_949DB9 = C_949DB9_CRY;
        C_2B3BFF = C_2B3BFF_CRY;
        C_50ADAC = C_50ADAC_CRY;
        C_CCE4A5 = C_CCE4A5_CRY;
        C_CCEA5F = C_CCEA5F_CRY;
        C_B30202 = C_B30202_CRY;
        C_B87A15 = C_B87A15_CRY;
        C_FFD000 = C_FFD000_CRY;
        C_FFDE4C = C_FFDE4C_CRY;
        C_FFF588 = C_FFF588_CRY;
        C_3089FF = C_3089FF_CRY;
        C_A88139 = C_A88139_CRY;
        C_7084C4 = C_7084C4_CRY;
        C_D46D3D = C_D46D3D_CRY;
        C_05A8A0 = C_05A8A0_CRY;
        C_FF3030 = C_FF3030_CRY;
        C_6435B5 = C_6435B5_CRY;
        C_FFAFAF = C_FFAFAF_CRY;
        C_ECB866 = C_ECB866_CRY;
        C_C63F23 = C_C63F23_CRY;
        C_9BC8CD = C_9BC8CD_CRY;
        C_666666 = C_666666_CRY;
        C_777777 = C_777777_CRY;
        
        C_CEA98B = C_CEA98B_CRY;
        C_FFCD5A = C_FFCD5A_CRY;
        C_AC785B = C_AC785B_CRY;
    }
    else
    {
        C_EEC06B = C_EEC06B_DOOM;
        C_D97C45 = C_D97C45_DOOM;
        C_FF7F7F = C_FF7F7F_DOOM;
        C_55B828 = C_55B828_DOOM;
        C_BBE357 = C_BBE357_DOOM;
        C_949DB9 = C_949DB9_DOOM;
        C_2B3BFF = C_2B3BFF_DOOM;
        C_50ADAC = C_50ADAC_DOOM;
        C_CCE4A5 = C_CCE4A5_DOOM;
        C_CCEA5F = C_CCEA5F_DOOM;
        C_B30202 = C_B30202_DOOM;
        C_B87A15 = C_B87A15_DOOM;
        C_FFD000 = C_FFD000_DOOM;
        C_FFDE4C = C_FFDE4C_DOOM;
        C_FFF588 = C_FFF588_DOOM;
        C_3089FF = C_3089FF_DOOM;
        C_A88139 = C_A88139_DOOM;
        C_7084C4 = C_7084C4_DOOM;
        C_D46D3D = C_D46D3D_DOOM;
        C_05A8A0 = C_05A8A0_DOOM;
        C_FF3030 = C_FF3030_DOOM;
        C_6435B5 = C_6435B5_DOOM;
        C_FFAFAF = C_FFAFAF_DOOM;
        C_ECB866 = C_ECB866_DOOM;
        C_C63F23 = C_C63F23_DOOM;
        C_9BC8CD = C_9BC8CD_DOOM;
        C_666666 = C_666666_DOOM;
        C_777777 = C_777777_DOOM;
        
        C_CEA98B = C_CEA98B_DOOM;
        C_FFCD5A = C_FFCD5A_DOOM;
        C_AC785B = C_AC785B_DOOM;
    }
    
    R_InitColoredColormap(k, scale, C_EEC06B, colormaps_EEC06B, j);
    R_InitColoredColormap(k, scale, C_D97C45, colormaps_D97C45, j);
    R_InitColoredColormap(k, scale, C_FF7F7F, colormaps_FF7F7F, j);
    R_InitColoredColormap(k, scale, C_55B828, colormaps_55B828, j);
    R_InitColoredColormap(k, scale, C_BBE357, colormaps_BBE357, j);
    R_InitColoredColormap(k, scale, C_949DB9, colormaps_949DB9, j);
    R_InitColoredColormap(k, scale, C_2B3BFF, colormaps_2B3BFF, j);
    R_InitColoredColormap(k, scale, C_50ADAC, colormaps_50ADAC, j);
    R_InitColoredColormap(k, scale, C_CCE4A5, colormaps_CCE4A5, j);
    R_InitColoredColormap(k, scale, C_CCEA5F, colormaps_CCEA5F, j);
    R_InitColoredColormap(k, scale, C_B30202, colormaps_B30202, j);
    R_InitColoredColormap(k, scale, C_B87A15, colormaps_B87A15, j);
    R_InitColoredColormap(k, scale, C_FFD000, colormaps_FFD000, j);
    R_InitColoredColormap(k, scale, C_FFDE4C, colormaps_FFDE4C, j);
    R_InitColoredColormap(k, scale, C_FFF588, colormaps_FFF588, j);
    R_InitColoredColormap(k, scale, C_3089FF, colormaps_3089FF, j);
    R_InitColoredColormap(k, scale, C_A88139, colormaps_A88139, j);
    R_InitColoredColormap(k, scale, C_7084C4, colormaps_7084C4, j);
    R_InitColoredColormap(k, scale, C_D46D3D, colormaps_D46D3D, j);
    R_InitColoredColormap(k, scale, C_05A8A0, colormaps_05A8A0, j);
    R_InitColoredColormap(k, scale, C_FF3030, colormaps_FF3030, j);
    R_InitColoredColormap(k, scale, C_6435B5, colormaps_6435B5, j);
    R_InitColoredColormap(k, scale, C_FFAFAF, colormaps_FFAFAF, j);
    R_InitColoredColormap(k, scale, C_ECB866, colormaps_C63F23, j);
    R_InitColoredColormap(k, scale, C_C63F23, colormaps_ECB866, j);
    R_InitColoredColormap(k, scale, C_9BC8CD, colormaps_9BC8CD, j);
    R_InitColoredColormap(k, scale, C_666666, colormaps_666666, j);
    R_InitColoredColormap(k, scale, C_777777, colormaps_777777, j);
    
    R_InitColoredColormap(k, scale, C_CEA98B, colormaps_CEA98B, j);
    R_InitColoredColormap(k, scale, C_FFCD5A, colormaps_FFCD5A, j);
    R_InitColoredColormap(k, scale, C_AC785B, colormaps_AC785B, j);
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
            free(scalelight_2B3BFF[i]);
            free(scalelight_50ADAC[i]);
            free(scalelight_CCE4A5[i]);
            free(scalelight_CCEA5F[i]);
            free(scalelight_B30202[i]);
            free(scalelight_B87A15[i]);
            free(scalelight_FFD000[i]);
            free(scalelight_FFDE4C[i]);
            free(scalelight_FFF588[i]);
            free(scalelight_3089FF[i]);
            free(scalelight_A88139[i]);
            free(scalelight_7084C4[i]);
            free(scalelight_D46D3D[i]);
            free(scalelight_05A8A0[i]);
            free(scalelight_FF3030[i]);
            free(scalelight_6435B5[i]);
            free(scalelight_FFAFAF[i]);
            free(scalelight_ECB866[i]);
            free(scalelight_C63F23[i]);
            free(scalelight_9BC8CD[i]);
            free(scalelight_666666[i]);
            free(scalelight_777777[i]);
            
            free(scalelight_CEA98B[i]);
            free(scalelight_FFCD5A[i]);
            free(scalelight_AC785B[i]);
        }
        free(scalelight_EEC06B);
        free(scalelight_D97C45);
        free(scalelight_FF7F7F);
        free(scalelight_55B828);
        free(scalelight_BBE357);
        free(scalelight_949DB9);
        free(scalelight_2B3BFF);
        free(scalelight_50ADAC);
        free(scalelight_CCE4A5);
        free(scalelight_CCEA5F);
        free(scalelight_B30202);
        free(scalelight_B87A15);
        free(scalelight_FFD000);
        free(scalelight_FFDE4C);
        free(scalelight_FFF588);
        free(scalelight_3089FF);
        free(scalelight_A88139);
        free(scalelight_7084C4);
        free(scalelight_D46D3D);
        free(scalelight_05A8A0);
        free(scalelight_FF3030);
        free(scalelight_6435B5);
        free(scalelight_FFAFAF);
        free(scalelight_ECB866);
        free(scalelight_C63F23);
        free(scalelight_9BC8CD);
        free(scalelight_666666);
        free(scalelight_777777);
        
        free(scalelight_CEA98B);
        free(scalelight_FFCD5A);
        free(scalelight_AC785B);
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
            free(zlight_2B3BFF[i]);
            free(zlight_50ADAC[i]);
            free(zlight_CCE4A5[i]);
            free(zlight_CCEA5F[i]);
            free(zlight_B30202[i]);
            free(zlight_B87A15[i]);
            free(zlight_FFD000[i]);
            free(zlight_FFDE4C[i]);
            free(zlight_FFF588[i]);
            free(zlight_3089FF[i]);
            free(zlight_A88139[i]);
            free(zlight_7084C4[i]);
            free(zlight_D46D3D[i]);
            free(zlight_05A8A0[i]);
            free(zlight_FF3030[i]);
            free(zlight_6435B5[i]);
            free(zlight_FFAFAF[i]);
            free(zlight_ECB866[i]);
            free(zlight_C63F23[i]);
            free(zlight_9BC8CD[i]);
            free(zlight_666666[i]);
            free(zlight_777777[i]);
            
            free(zlight_CEA98B[i]);
            free(zlight_FFCD5A[i]);
            free(zlight_AC785B[i]);
        }
        free(zlight_EEC06B);
        free(zlight_D97C45);
        free(zlight_FF7F7F);
        free(zlight_55B828);
        free(zlight_BBE357);
        free(zlight_949DB9);
        free(zlight_2B3BFF);
        free(zlight_50ADAC);
        free(zlight_CCE4A5);
        free(zlight_CCEA5F);
        free(zlight_B30202);
        free(zlight_B87A15);
        free(zlight_FFD000);
        free(zlight_FFDE4C);
        free(zlight_FFF588);
        free(zlight_3089FF);
        free(zlight_A88139);
        free(zlight_7084C4);
        free(zlight_D46D3D);
        free(zlight_05A8A0);
        free(zlight_FF3030);
        free(zlight_6435B5);
        free(zlight_FFAFAF);
        free(zlight_ECB866);
        free(zlight_C63F23);
        free(zlight_9BC8CD);
        free(zlight_666666);
        free(zlight_777777);
        
        free(zlight_CEA98B);
        free(zlight_FFCD5A);
        free(zlight_AC785B);
    }
    
    scalelight_EEC06B = malloc(sclight_size);
    scalelight_D97C45 = malloc(sclight_size);
    scalelight_FF7F7F = malloc(sclight_size);
    scalelight_55B828 = malloc(sclight_size);
    scalelight_BBE357 = malloc(sclight_size);
    scalelight_949DB9 = malloc(sclight_size);
    scalelight_2B3BFF = malloc(sclight_size);
    scalelight_50ADAC = malloc(sclight_size);
    scalelight_CCE4A5 = malloc(sclight_size);
    scalelight_CCEA5F = malloc(sclight_size);
    scalelight_B30202 = malloc(sclight_size);
    scalelight_B87A15 = malloc(sclight_size);
    scalelight_FFD000 = malloc(sclight_size);
    scalelight_FFDE4C = malloc(sclight_size);
    scalelight_FFF588 = malloc(sclight_size);
    scalelight_3089FF = malloc(sclight_size);
    scalelight_A88139 = malloc(sclight_size);
    scalelight_7084C4 = malloc(sclight_size);
    scalelight_D46D3D = malloc(sclight_size);
    scalelight_05A8A0 = malloc(sclight_size);
    scalelight_FF3030 = malloc(sclight_size);
    scalelight_6435B5 = malloc(sclight_size);
    scalelight_FFAFAF = malloc(sclight_size);
    scalelight_ECB866 = malloc(sclight_size);
    scalelight_C63F23 = malloc(sclight_size);
    scalelight_9BC8CD = malloc(sclight_size);
    scalelight_666666 = malloc(sclight_size);
    scalelight_777777 = malloc(sclight_size);
    
    scalelight_CEA98B = malloc(sclight_size);
    scalelight_FFCD5A = malloc(sclight_size);
    scalelight_AC785B = malloc(sclight_size);

    zlight_EEC06B = malloc(zlight_size);
    zlight_D97C45 = malloc(zlight_size);
    zlight_FF7F7F = malloc(zlight_size);
    zlight_55B828 = malloc(zlight_size);
    zlight_BBE357 = malloc(zlight_size);
    zlight_949DB9 = malloc(zlight_size);
    zlight_2B3BFF = malloc(zlight_size);
    zlight_50ADAC = malloc(zlight_size);
    zlight_CCE4A5 = malloc(zlight_size);
    zlight_CCEA5F = malloc(zlight_size);
    zlight_B30202 = malloc(zlight_size);
    zlight_B87A15 = malloc(zlight_size);
    zlight_FFD000 = malloc(zlight_size);
    zlight_FFDE4C = malloc(zlight_size);
    zlight_FFF588 = malloc(zlight_size);
    zlight_3089FF = malloc(zlight_size);
    zlight_A88139 = malloc(zlight_size);
    zlight_7084C4 = malloc(zlight_size);
    zlight_D46D3D = malloc(zlight_size);
    zlight_05A8A0 = malloc(zlight_size);
    zlight_FF3030 = malloc(zlight_size);
    zlight_6435B5 = malloc(zlight_size);
    zlight_FFAFAF = malloc(zlight_size);
    zlight_ECB866 = malloc(zlight_size);
    zlight_C63F23 = malloc(zlight_size);
    zlight_9BC8CD = malloc(zlight_size);
    zlight_666666 = malloc(zlight_size);
    zlight_777777 = malloc(zlight_size);
    
    zlight_CEA98B = malloc(zlight_size);
    zlight_FFCD5A = malloc(zlight_size);
    zlight_AC785B = malloc(zlight_size);

    // Calculate the light levels to use for each level / distance combination.
    for (i = 0 ; i < LIGHTLEVELS ; i++)
    {
        scalelight_EEC06B[i] = malloc(sclight_size_max);
        scalelight_D97C45[i] = malloc(sclight_size_max);
        scalelight_FF7F7F[i] = malloc(sclight_size_max);
        scalelight_55B828[i] = malloc(sclight_size_max);
        scalelight_BBE357[i] = malloc(sclight_size_max);
        scalelight_949DB9[i] = malloc(sclight_size_max);
        scalelight_2B3BFF[i] = malloc(sclight_size_max);
        scalelight_50ADAC[i] = malloc(sclight_size_max);
        scalelight_CCE4A5[i] = malloc(sclight_size_max);
        scalelight_CCEA5F[i] = malloc(sclight_size_max);
        scalelight_B30202[i] = malloc(sclight_size_max);
        scalelight_B87A15[i] = malloc(sclight_size_max);
        scalelight_FFD000[i] = malloc(sclight_size_max);
        scalelight_FFDE4C[i] = malloc(sclight_size_max);
        scalelight_FFF588[i] = malloc(sclight_size_max);
        scalelight_3089FF[i] = malloc(sclight_size_max);
        scalelight_A88139[i] = malloc(sclight_size_max);
        scalelight_7084C4[i] = malloc(sclight_size_max);
        scalelight_D46D3D[i] = malloc(sclight_size_max);
        scalelight_05A8A0[i] = malloc(sclight_size_max);
        scalelight_FF3030[i] = malloc(sclight_size_max);
        scalelight_6435B5[i] = malloc(sclight_size_max);
        scalelight_FFAFAF[i] = malloc(sclight_size_max);
        scalelight_ECB866[i] = malloc(sclight_size_max);
        scalelight_C63F23[i] = malloc(sclight_size_max);
        scalelight_9BC8CD[i] = malloc(sclight_size_max);
        scalelight_666666[i] = malloc(sclight_size_max);
        scalelight_777777[i] = malloc(sclight_size_max);
        
        scalelight_CEA98B[i] = malloc(sclight_size_max);
        scalelight_FFCD5A[i] = malloc(sclight_size_max);
        scalelight_AC785B[i] = malloc(sclight_size_max);

        zlight_EEC06B[i] = malloc(zlight_size_max);
        zlight_D97C45[i] = malloc(zlight_size_max);
        zlight_FF7F7F[i] = malloc(zlight_size_max);
        zlight_55B828[i] = malloc(zlight_size_max);
        zlight_BBE357[i] = malloc(zlight_size_max);
        zlight_949DB9[i] = malloc(zlight_size_max);
        zlight_2B3BFF[i] = malloc(zlight_size_max);
        zlight_50ADAC[i] = malloc(zlight_size_max);
        zlight_CCE4A5[i] = malloc(zlight_size_max);
        zlight_CCEA5F[i] = malloc(zlight_size_max);
        zlight_B30202[i] = malloc(zlight_size_max);
        zlight_B87A15[i] = malloc(zlight_size_max);
        zlight_FFD000[i] = malloc(zlight_size_max);
        zlight_FFDE4C[i] = malloc(zlight_size_max);
        zlight_FFF588[i] = malloc(zlight_size_max);
        zlight_3089FF[i] = malloc(zlight_size_max);
        zlight_A88139[i] = malloc(zlight_size_max);
        zlight_7084C4[i] = malloc(zlight_size_max);
        zlight_D46D3D[i] = malloc(zlight_size_max);
        zlight_05A8A0[i] = malloc(zlight_size_max);
        zlight_FF3030[i] = malloc(zlight_size_max);
        zlight_6435B5[i] = malloc(zlight_size_max);
        zlight_FFAFAF[i] = malloc(zlight_size_max);
        zlight_ECB866[i] = malloc(zlight_size_max);
        zlight_C63F23[i] = malloc(zlight_size_max);
        zlight_9BC8CD[i] = malloc(zlight_size_max);
        zlight_666666[i] = malloc(zlight_size_max);
        zlight_777777[i] = malloc(zlight_size_max);
        
        zlight_CEA98B[i] = malloc(zlight_size_max);
        zlight_FFCD5A[i] = malloc(zlight_size_max);
        zlight_AC785B[i] = malloc(zlight_size_max);

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
            zlight_2B3BFF[i][j] = colormaps_2B3BFF + level;
            zlight_50ADAC[i][j] = colormaps_50ADAC + level;
            zlight_CCE4A5[i][j] = colormaps_CCE4A5 + level;
            zlight_CCEA5F[i][j] = colormaps_CCEA5F + level;
            zlight_B30202[i][j] = colormaps_B30202 + level;
            zlight_B87A15[i][j] = colormaps_B87A15 + level;
            zlight_FFD000[i][j] = colormaps_FFD000 + level;
            zlight_FFDE4C[i][j] = colormaps_FFDE4C + level;
            zlight_FFF588[i][j] = colormaps_FFF588 + level;
            zlight_3089FF[i][j] = colormaps_3089FF + level;
            zlight_A88139[i][j] = colormaps_A88139 + level;
            zlight_7084C4[i][j] = colormaps_7084C4 + level;
            zlight_D46D3D[i][j] = colormaps_D46D3D + level;
            zlight_05A8A0[i][j] = colormaps_05A8A0 + level;
            zlight_FF3030[i][j] = colormaps_FF3030 + level;
            zlight_6435B5[i][j] = colormaps_6435B5 + level;
            zlight_FFAFAF[i][j] = colormaps_FFAFAF + level;
            zlight_ECB866[i][j] = colormaps_ECB866 + level;
            zlight_C63F23[i][j] = colormaps_C63F23 + level;
            zlight_9BC8CD[i][j] = colormaps_9BC8CD + level;
            zlight_666666[i][j] = colormaps_666666 + level;
            zlight_777777[i][j] = colormaps_777777 + level;
            
            zlight_CEA98B[i][j] = colormaps_CEA98B + level;
            zlight_FFCD5A[i][j] = colormaps_FFCD5A + level;
            zlight_AC785B[i][j] = colormaps_AC785B + level;
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
            scalelight_2B3BFF[i][j] = colormaps_2B3BFF + level;
            scalelight_50ADAC[i][j] = colormaps_50ADAC + level;
            scalelight_CCE4A5[i][j] = colormaps_CCE4A5 + level;
            scalelight_CCEA5F[i][j] = colormaps_CCEA5F + level;
            scalelight_B30202[i][j] = colormaps_B30202 + level;
            scalelight_B87A15[i][j] = colormaps_B87A15 + level;
            scalelight_FFD000[i][j] = colormaps_FFD000 + level;
            scalelight_FFDE4C[i][j] = colormaps_FFDE4C + level;
            scalelight_FFF588[i][j] = colormaps_FFF588 + level;
            scalelight_3089FF[i][j] = colormaps_3089FF + level;
            scalelight_A88139[i][j] = colormaps_A88139 + level;
            scalelight_7084C4[i][j] = colormaps_7084C4 + level;
            scalelight_D46D3D[i][j] = colormaps_D46D3D + level;
            scalelight_05A8A0[i][j] = colormaps_05A8A0 + level;
            scalelight_FF3030[i][j] = colormaps_FF3030 + level;
            scalelight_6435B5[i][j] = colormaps_6435B5 + level;
            scalelight_FFAFAF[i][j] = colormaps_FFAFAF + level;
            scalelight_ECB866[i][j] = colormaps_ECB866 + level;
            scalelight_C63F23[i][j] = colormaps_C63F23 + level;
            scalelight_9BC8CD[i][j] = colormaps_9BC8CD + level;
            scalelight_666666[i][j] = colormaps_666666 + level;
            scalelight_777777[i][j] = colormaps_777777 + level;
            
            scalelight_CEA98B[i][j] = colormaps_CEA98B + level;
            scalelight_FFCD5A[i][j] = colormaps_FFCD5A + level;
            scalelight_AC785B[i][j] = colormaps_AC785B + level;
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
            case 0x2B3BFF:  return zlight_2B3BFF[light]; break;
            case 0x50ADAC:  return zlight_50ADAC[light]; break;
            case 0xCCE4A5:  return zlight_CCE4A5[light]; break;
            case 0xCCEA5F:  return zlight_CCEA5F[light]; break;
            case 0xB30202:  return zlight_B30202[light]; break;
            case 0xB87A15:  return zlight_B87A15[light]; break;
            case 0xFFD000:  return zlight_FFD000[light]; break;
            case 0xFFDE4C:  return zlight_FFDE4C[light]; break;
            case 0xFFF588:  return zlight_FFF588[light]; break;
            case 0x3089FF:  return zlight_3089FF[light]; break;
            case 0xA88139:  return zlight_A88139[light]; break;
            case 0x7084C4:  return zlight_7084C4[light]; break;
            case 0xD46D3D:  return zlight_D46D3D[light]; break;
            case 0x05A8A0:  return zlight_05A8A0[light]; break;
            case 0xFF3030:  return zlight_FF3030[light]; break;
            case 0x6435B5:  return zlight_6435B5[light]; break;
            case 0xFFAFAF:  return zlight_FFAFAF[light]; break;
            case 0xECB866:  return zlight_C63F23[light]; break;
            case 0xC63F23:  return zlight_ECB866[light]; break;
            case 0x9BC8CD:  return zlight_9BC8CD[light]; break;
            case 0x666666:  return zlight_666666[light]; break;
            case 0x777777:  return zlight_777777[light]; break;
            
            case 0xCEA98B:  return zlight_CEA98B[light]; break;
            case 0xFFCD5A:  return zlight_FFCD5A[light]; break;
            case 0xAC785B:  return zlight_AC785B[light]; break;
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
            case 0x2B3BFF:  return scalelight_2B3BFF[BETWEEN(0, l, lightnum)];  break;
            case 0x50ADAC:  return scalelight_50ADAC[BETWEEN(0, l, lightnum)];  break;
            case 0xCCE4A5:  return scalelight_CCE4A5[BETWEEN(0, l, lightnum)];  break;
            case 0xCCEA5F:  return scalelight_CCEA5F[BETWEEN(0, l, lightnum)];  break;
            case 0xB30202:  return scalelight_B30202[BETWEEN(0, l, lightnum)];  break;
            case 0xB87A15:  return scalelight_B87A15[BETWEEN(0, l, lightnum)];  break;
            case 0xFFD000:  return scalelight_FFD000[BETWEEN(0, l, lightnum)];  break;
            case 0xFFDE4C:  return scalelight_FFDE4C[BETWEEN(0, l, lightnum)];  break;
            case 0xFFF588:  return scalelight_FFF588[BETWEEN(0, l, lightnum)];  break;
            case 0x3089FF:  return scalelight_3089FF[BETWEEN(0, l, lightnum)];  break;
            case 0xA88139:  return scalelight_A88139[BETWEEN(0, l, lightnum)];  break;
            case 0x7084C4:  return scalelight_7084C4[BETWEEN(0, l, lightnum)];  break;
            case 0xD46D3D:  return scalelight_D46D3D[BETWEEN(0, l, lightnum)];  break;
            case 0x05A8A0:  return scalelight_05A8A0[BETWEEN(0, l, lightnum)];  break;
            case 0xFF3030:  return scalelight_FF3030[BETWEEN(0, l, lightnum)];  break;
            case 0x6435B5:  return scalelight_6435B5[BETWEEN(0, l, lightnum)];  break;
            case 0xFFAFAF:  return scalelight_FFAFAF[BETWEEN(0, l, lightnum)];  break;
            case 0xECB866:  return scalelight_C63F23[BETWEEN(0, l, lightnum)];  break;
            case 0xC63F23:  return scalelight_ECB866[BETWEEN(0, l, lightnum)];  break;
            case 0x9BC8CD:  return scalelight_9BC8CD[BETWEEN(0, l, lightnum)];  break;
            case 0x666666:  return scalelight_666666[BETWEEN(0, l, lightnum)];  break;
            case 0x777777:  return scalelight_777777[BETWEEN(0, l, lightnum)];  break;
            
            case 0xCEA98B:  return scalelight_CEA98B[BETWEEN(0, l, lightnum)];  break;
            case 0xFFCD5A:  return scalelight_FFCD5A[BETWEEN(0, l, lightnum)];  break;
            case 0xAC785B:  return scalelight_AC785B[BETWEEN(0, l, lightnum)];  break;
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
            case 0x2B3BFF:  return colormaps_2B3BFF;  break;
            case 0x50ADAC:  return colormaps_50ADAC;  break;
            case 0xCCE4A5:  return colormaps_CCE4A5;  break;
            case 0xCCEA5F:  return colormaps_CCEA5F;  break;
            case 0xB30202:  return colormaps_B30202;  break;
            case 0xB87A15:  return colormaps_B87A15;  break;
            case 0xFFD000:  return colormaps_FFD000;  break;
            case 0xFFDE4C:  return colormaps_FFDE4C;  break;
            case 0xFFF588:  return colormaps_FFF588;  break;
            case 0x3089FF:  return colormaps_3089FF;  break;
            case 0xA88139:  return colormaps_A88139;  break;
            case 0x7084C4:  return colormaps_7084C4;  break;
            case 0xD46D3D:  return colormaps_D46D3D;  break;
            case 0x05A8A0:  return colormaps_05A8A0;  break;
            case 0xFF3030:  return colormaps_FF3030;  break;
            case 0x6435B5:  return colormaps_6435B5;  break;
            case 0xFFAFAF:  return colormaps_FFAFAF;  break;
            case 0xECB866:  return colormaps_C63F23;  break;
            case 0xC63F23:  return colormaps_ECB866;  break;
            case 0x9BC8CD:  return colormaps_9BC8CD;  break;
            case 0x666666:  return colormaps_666666;  break;
            case 0x777777:  return colormaps_777777;  break;
            
            case 0xCEA98B:  return colormaps_CEA98B;  break;
            case 0xFFCD5A:  return colormaps_FFCD5A;  break;
            case 0xAC785B:  return colormaps_AC785B;  break;
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
    {    1,      3,    0x2B3BFF },
    {    1,      4,    0x2B3BFF },
    {    1,      5,    0x2B3BFF },
    {    1,      6,    0x2B3BFF },
    {    1,      7,    0x2B3BFF },
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
    {    2,     30,    0x3089FF },
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
    {    3,      7,    0x7084C4 },
    {    3,     10,    0x7084C4 },
    {    3,     11,    0x7084C4 },
    {    3,     12,    0x7084C4 },
    {    3,     13,    0x7084C4 },
    {    3,     22,    0xFFF588 },
    {    3,     26,    0x3089FF },
    {    3,     28,    0x949DB9 },
    {    3,     33,    0x7084C4 },
    {    3,     34,    0x7084C4 },
    {    3,     35,    0x7084C4 },
    {    3,     36,    0x7084C4 },
    {    3,     37,    0x7084C4 },
    {    3,     38,    0x7084C4 },
    {    3,     39,    0x7084C4 },
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
    {    3,     77,    0x3089FF },
    {    3,     78,    0xBBE357 },
    {    3,     79,    0x3089FF },
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
    {    3,    123,    0xA88139 },
    {    3,    126,    0x3089FF },
    {    3,    128,    0x3089FF },
    {    3,    132,    0xA88139 },
    {    3,    133,    0xA88139 },
    {    3,    134,    0xA88139 },
    {    3,    135,    0xA88139 },
    {    3,    136,    0xA88139 },
    {    3,    137,    0xA88139 },
    {    3,    138,    0xBBE357 },
    {    3,    139,    0xBBE357 },
    {    3,    140,    0xA88139 },
    {    3,    151,    0xBBE357 },
    {    3,    157,    0x3089FF },
    {    3,    158,    0xFFF588 },
    {    3,    159,    0xFFF588 },
    SECTORCOLOR_END
};

//
// Area 4: Command Control
//

static const sectorcolor_t sectorcolor_map04[] =
{
    {    4,     11,    0x3089FF },
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
    {    4,     42,    0x3089FF },
    {    4,     43,    0x3089FF },
    {    4,     46,    0xBBE357 },
    {    4,     47,    0xBBE357 },
    {    4,     48,    0xBBE357 },
    {    4,     51,    0xBBE357 },
    {    4,     60,    0xBBE357 },
    {    4,     61,    0x3089FF },
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
    {    4,    120,    0x3089FF },
    SECTORCOLOR_END
};

//
// Area 5: Phobos Lab
//

static const sectorcolor_t sectorcolor_map05[] =
{
    {    5,      0,    0x7084C4 },
    {    5,      3,    0xBBE357 },
    {    5,      6,    0x3089FF },
    {    5,      7,    0x3089FF },
    {    5,      8,    0x7084C4 },
    {    5,      9,    0x7084C4 },
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
    {    5,     69,    0x3089FF },
    {    5,     71,    0xB30202 },
    {    5,     72,    0xB30202 },
    {    5,     78,    0x55B828 },
    {    5,     81,    0xFFF588 },
    {    5,     99,    0x55B828 },
    {    5,    101,    0x55B828 },
    {    5,    111,    0x55B828 },
    {    5,    120,    0xFFF588 },
    {    5,    121,    0xFFF588 },
    {    5,    122,    0x3089FF },
    {    5,    123,    0x3089FF },
    SECTORCOLOR_END
};

//
// Area 6: Central Processing
//

static const sectorcolor_t sectorcolor_map06[] =
{
    {    6,      0,    0x55B828 },
    {    6,      1,    0x55B828 },
    {    6,      2,    0x2B3BFF },
    {    6,      3,    0x55B828 },
    {    6,      4,    0x55B828 },
    {    6,      5,    0x55B828 },
    {    6,      6,    0x55B828 },
    {    6,      7,    0x55B828 },
    {    6,      8,    0x2B3BFF },
    {    6,      9,    0x55B828 },
    {    6,     10,    0x3089FF },
    {    6,     12,    0x3089FF },
    {    6,     17,    0xBBE357 },
    {    6,     18,    0xBBE357 },
    {    6,     25,    0x949DB9 },
    {    6,     26,    0x949DB9 },
    {    6,     29,    0x7084C4 },
    {    6,     30,    0x7084C4 },
    {    6,     31,    0x7084C4 },
    {    6,     32,    0x7084C4 },
    {    6,     34,    0xBBE357 },
    {    6,     35,    0x55B828 },
    {    6,     36,    0x55B828 },
    {    6,     39,    0x3089FF },
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
    {    6,     99,    0x7084C4 },
    {    6,    100,    0x7084C4 },
    {    6,    101,    0x7084C4 },
    {    6,    104,    0x55B828 },
    {    6,    105,    0x55B828 },
    {    6,    112,    0x55B828 },
    {    6,    113,    0x55B828 },
    {    6,    132,    0x7084C4 },
    {    6,    133,    0x7084C4 },
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
    {    6,    180,    0x3089FF },
    SECTORCOLOR_END
};

//
// Area 7: Computer Station
//

static const sectorcolor_t sectorcolor_map07[] =
{
    {    7,      3,    0x55B828 },
    {    7,      4,    0xBBE357 },
    {    7,      7,    0x7084C4 },
    {    7,      8,    0x7084C4 },
    {    7,     11,    0x7084C4 },
    {    7,     12,    0x7084C4 },
    {    7,     13,    0x7084C4 },
    {    7,     14,    0x6435B5 },
    {    7,     15,    0x6435B5 },
    {    7,     16,    0x7084C4 },
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
    {    7,     73,    0x3089FF },
    {    7,     74,    0x3089FF },
    {    7,     84,    0x05A8A0 },
    {    7,     88,    0x3089FF },
    {    7,     93,    0x3089FF },
    {    7,     98,    0x7084C4 },
    {    7,    101,    0xFFF588 },
    {    7,    102,    0xFFF588 },
    {    7,    107,    0x6435B5 },
    {    7,    113,    0xFF7F7F },
    {    7,    114,    0xFF7F7F },
    {    7,    115,    0xFF7F7F },
    {    7,    116,    0xFF7F7F },
    {    7,    119,    0x3089FF },
    {    7,    133,    0x6435B5 },
    {    7,    134,    0x6435B5 },
    {    7,    136,    0x6435B5 },
    {    7,    137,    0x6435B5 },
    {    7,    140,    0xFF7F7F },
    {    7,    141,    0xFF7F7F },
    {    7,    142,    0xFFF588 },
    {    7,    143,    0xFFF588 },
    {    7,    144,    0x3089FF },
    {    7,    145,    0x3089FF },
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
    {    8,      5,    0x7084C4 },
    {    8,      7,    0xFF3030 },
    {    8,     12,    0xD97C45 },
    {    8,     13,    0xD97C45 },
    {    8,     16,    0xFFAFAF },
    {    8,     30,    0x3089FF },
    {    8,     51,    0xB30202 },
    {    8,     52,    0x7084C4 },
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
    {    9,      1,    0x3089FF },
    {    9,      3,    0x3089FF },
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
    {    9,     43,    0x7084C4 },
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
    {    9,     80,    0x3089FF },
    {    9,     82,    0x3089FF },
    {    9,     83,    0xB30202 },
    {    9,     84,    0x7084C4 },
    {    9,     85,    0xB30202 },
    {    9,     88,    0x3089FF },
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
    {   10,     54,    0x3089FF },
    {   10,     63,    0x3089FF },
    {   10,     64,    0x3089FF },
    {   10,     65,    0x3089FF },
    {   10,     68,    0x50ADAC },
    {   10,     70,    0xFFF588 },
    {   10,     78,    0x9BC8CD },
    {   10,     79,    0x9BC8CD },
    {   10,     81,    0xFF7F7F },
    {   10,     83,    0x3089FF },
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
    {   10,    121,    0x7084C4 },
    {   10,    126,    0xA88139 },
    {   10,    127,    0xA88139 },
    {   10,    128,    0x3089FF },
    {   10,    129,    0x3089FF },
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
    {   11,     34,    0x3089FF },
    {   11,     35,    0x3089FF },
    {   11,     36,    0x55B828 },
    {   11,     38,    0xBBE357 },
    {   11,     40,    0xBBE357 },
    {   11,     41,    0xBBE357 },
    {   11,     42,    0xBBE357 },
    {   11,     60,    0x55B828 },
    {   11,     62,    0x55B828 },
    {   11,     67,    0xFF7F7F },
    {   11,     69,    0xEEC06B },
    {   11,     83,    0x3089FF },
    {   11,     84,    0xFFAFAF },
    {   11,     90,    0xD97C45 },
    {   11,     97,    0x3089FF },
    {   11,    100,    0x3089FF },
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
    {   12,     28,    0x3089FF },
    {   12,     29,    0x3089FF },
    {   12,     30,    0x3089FF },
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
    {   12,    165,    0x3089FF },
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
    {   13,     77,    0x2B3BFF },
    {   13,     78,    0x2B3BFF },
    {   13,     79,    0x2B3BFF },
    {   13,     80,    0x2B3BFF },
    {   13,     81,    0x2B3BFF },
    {   13,     82,    0x2B3BFF },
    {   13,     83,    0x2B3BFF },
    {   13,     84,    0x2B3BFF },
    {   13,     85,    0x2B3BFF },
    {   13,     86,    0x2B3BFF },
    {   13,     87,    0x2B3BFF },
    {   13,     88,    0xD97C45 },
    {   13,     89,    0xD97C45 },
    {   13,     90,    0xFF7F7F },
    {   13,     91,    0xD97C45 },
    {   13,     95,    0x2B3BFF },
    {   13,     96,    0x2B3BFF },
    {   13,     97,    0x2B3BFF },
    {   13,     98,    0x2B3BFF },
    {   13,     99,    0x2B3BFF },
    {   13,    100,    0x2B3BFF },
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
    {   14,     12,    0x3089FF },
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
    {   14,    111,    0x3089FF },
    {   14,    112,    0x3089FF },
    {   14,    113,    0xFF7F7F },
    {   14,    114,    0xFF7F7F },
    {   14,    115,    0xFFF588 },
    {   14,    116,    0x3089FF },
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
    {   17,     19,    0x3089FF },
    {   17,     20,    0x3089FF },
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
    {   18,     81,    0x7084C4 },
    {   18,     82,    0xD97C45 },
    {   18,     83,    0xD97C45 },
    {   18,     85,    0xEEC06B },
    {   18,     86,    0x7084C4 },
    {   18,     87,    0x7084C4 },
    {   18,     88,    0x7084C4 },
    {   18,     89,    0xD97C45 },
    {   18,     90,    0x7084C4 },
    {   18,     91,    0x7084C4 },
    {   18,     92,    0x7084C4 },
    {   18,     93,    0x7084C4 },
    {   18,     94,    0xFFF588 },
    {   18,     97,    0x3089FF },
    {   18,     98,    0x3089FF },
    {   18,     99,    0x3089FF },
    {   18,    100,    0xFF7F7F },
    {   18,    101,    0xFF7F7F },
    {   18,    104,    0xBBE357 },
    {   18,    106,    0x7084C4 },
    {   18,    111,    0xC63F23 },
    {   18,    112,    0xFFF588 },
    {   18,    114,    0xC63F23 },
    {   18,    115,    0xC63F23 },
    {   18,    117,    0x7084C4 },
    {   18,    118,    0x7084C4 },
    {   18,    119,    0x7084C4 },
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
    {   19,     64,    0x3089FF },
    {   19,     66,    0x3089FF },
    {   19,     65,    0xFF7F7F },
    {   19,     73,    0xFFAFAF },
    {   19,     77,    0xFF7F7F },
    {   19,     78,    0xFF7F7F },
    {   19,     80,    0xFFAFAF },
    {   19,     81,    0xFFAFAF },
    {   19,     86,    0x3089FF },
    {   19,     87,    0x3089FF },
    {   19,     88,    0x3089FF },
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
    {   20,    109,    0x3089FF },
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
    {   20,    228,    0x3089FF },
    {   20,    229,    0x3089FF },
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
    {   21,     15,    0x2B3BFF },
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
    {   21,    112,    0x2B3BFF },
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
    {   22,     47,    0x7084C4 },
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
    {   22,    113,    0x3089FF },
    {   22,    117,    0x3089FF },
    {   22,    118,    0x3089FF },
    {   22,    121,    0xFF7F7F },
    {   22,    122,    0xFF7F7F },
    {   22,    123,    0xFF7F7F },
    {   22,    125,    0x3089FF },
    {   22,    128,    0xFF3030 },
    {   22,    129,    0x3089FF },
    {   22,    130,    0x3089FF },
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
    {   23,      6,    0x3089FF },
    {   23,      7,    0x3089FF },
    {   23,      8,    0xFF7F7F },
    {   23,      9,    0x50ADAC },
    {   23,     10,    0x3089FF },
    {   23,     11,    0xFFF588 },
    {   23,     12,    0xB87A15 },
    {   23,     13,    0x666666 },
    {   23,     14,    0x50ADAC },
    {   23,     15,    0x3089FF },
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
    {   24,     24,    0x3089FF },
    {   24,     25,    0x3089FF },
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
    {   24,    145,    0x3089FF },
    {   24,    146,    0xFF7F7F },
    {   24,    147,    0xFFF588 },
    {   24,    148,    0xFF7F7F },
    {   24,    149,    0xFFF588 },
    SECTORCOLOR_END
};

//
// Area 25: Entryway
//

static const sectorcolor_t sectorcolor_map25[] =
{
    {   25,      0,    0xAC785B },
    {   25,      1,    0xCEA98B },
    {   25,      2,    0xCEA98B },
    {   25,      3,    0xCEA98B },
    {   25,      4,    0xFFCD5A },
    {   25,      5,    0xFFCD5A },
    {   25,      6,    0xFFF588 },
    {   25,      7,    0xFFCD5A },
    {   25,      8,    0xAC785B },
    {   25,      9,    0xFFCD5A },
    {   25,     10,    0xAC785B },
    {   25,     12,    0x9BC8CD },
    {   25,     18,    0xFF7F7F },
    {   25,     19,    0xFFF588 },
    {   25,     20,    0xFFCD5A },
    {   25,     21,    0xFFCD5A },
    {   25,     22,    0xFFCD5A },
    {   25,     23,    0xFFCD5A },
    {   25,     24,    0xCEA98B },
    {   25,     25,    0xCEA98B },
    {   25,     26,    0xFFCD5A },
    {   25,     27,    0xFFCD5A },
    {   25,     28,    0xCEA98B },
    {   25,     29,    0xCEA98B },
    {   25,     30,    0xCEA98B },
    {   25,     31,    0xFFCD5A },
    {   25,     32,    0xCEA98B },
    {   25,     33,    0xCEA98B },
    {   25,     34,    0xFFCD5A },
    {   25,     35,    0xFFCD5A },
    {   25,     36,    0xFFCD5A },
    {   25,     39,    0x9BC8CD },
    {   25,     40,    0x9BC8CD },
    {   25,     47,    0x9BC8CD },
    {   25,     48,    0x50ADAC },
    {   25,     49,    0x9BC8CD },
    {   25,     52,    0xFFCD5A },
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
        
        case 25:  sectorcolor = sectorcolor_map25;  break;
        default:  sectorcolor = sectorcolor_dummy;  break;
    }
}

// =============================================================================
//
//                           COLORED COLORMAP LUMPS
//
// =============================================================================

const byte C_EEC06B_CRY[] = {
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

const byte C_D97C45_CRY[] = {
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

const byte C_FF7F7F_CRY[] = {
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

const byte C_55B828_CRY[] = {
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

const byte C_BBE357_CRY[] = {
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

const byte C_949DB9_CRY[] = {
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

const byte C_2B3BFF_CRY[] = {
    0,0,0,0,0,0,0,0,0,7,11,46,43,59,255,0,
    0,2,0,0,0,0,0,0,0,0,0,3,5,12,1,2,
    2,0,0,0,0,0,0,7,7,21,5,5,14,4,4,10,
    43,56,230,43,51,212,43,49,202,43,43,180,43,40,168,41,
    37,153,39,33,136,37,29,122,35,27,110,33,24,99,31,21,
    87,30,20,82,28,17,70,27,15,62,24,12,51,24,12,48,
    22,9,39,20,9,35,18,7,30,17,6,23,15,5,19,14,
    4,16,13,3,13,12,2,10,10,2,6,10,0,2,8,0,
    0,8,0,0,7,0,0,6,0,0,5,0,0,4,0,0,
    43,59,255,43,59,255,43,59,255,43,59,244,43,59,233,43,
    59,217,43,56,198,43,54,189,43,51,166,43,48,150,42,44,
    137,40,41,124,38,38,111,36,34,97,34,31,87,33,30,81,
    30,28,75,27,25,67,25,23,62,23,21,57,21,19,50,18,
    17,45,16,15,38,15,13,34,13,12,31,10,11,26,8,9,
    21,7,7,17,5,6,13,4,4,9,2,3,3,2,2,2,
    43,59,255,43,59,255,43,59,255,43,59,255,42,59,255,40,
    59,244,38,57,237,36,54,223,34,51,210,33,49,204,31,46,
    189,30,44,182,27,41,170,25,38,156,24,36,148,22,33,136,
    20,29,122,19,28,115,17,25,102,15,22,91,14,21,85,12,
    18,74,10,16,65,10,14,60,8,12,51,7,10,43,6,9,
    37,5,7,29,4,6,27,3,5,19,2,3,13,2,2,10,
    22,59,131,20,59,114,18,57,100,15,52,87,14,46,73,12,
    40,62,10,34,51,8,29,43,7,24,31,5,19,27,4,15,
    18,3,11,11,2,7,6,0,4,0,0,2,0,0,0,0,
    33,44,160,31,40,146,29,37,132,26,34,121,24,30,107,23,
    28,99,21,26,90,19,23,78,17,21,69,15,18,60,14,17,
    55,12,14,47,11,13,43,9,11,33,8,9,28,7,8,25,
    23,29,95,19,24,74,16,20,61,13,16,48,10,12,35,8,
    9,27,6,7,17,5,5,12,16,25,87,13,21,70,11,18,
    60,10,15,51,8,12,37,6,10,30,5,8,22,4,6,17,
    40,59,164,36,51,126,31,41,96,26,32,66,22,23,43,18,
    15,23,14,9,9,11,5,1,43,59,255,43,59,255,43,57,
    237,43,46,190,43,36,147,43,27,110,43,17,70,43,10,41,
    42,4,16,38,3,14,35,3,13,33,3,11,30,2,10,27,
    2,9,24,2,6,22,1,4,19,0,2,16,0,0,13,0,
    0,11,0,0,9,0,0,7,0,0,6,0,0,4,0,0,
    43,59,255,41,59,255,35,52,255,28,42,255,22,33,255,15,
    23,255,10,15,255,6,9,255,3,4,255,2,3,218,2,2,
    182,1,2,149,0,0,117,0,0,87,0,0,61,0,0,38,
    43,59,255,43,59,255,43,59,244,42,57,202,42,51,159,42,
    44,115,42,36,76,41,30,47,38,27,39,36,24,31,34,22,
    29,32,19,25,29,16,18,27,14,13,24,12,10,23,11,9,
    43,59,255,43,59,255,42,59,234,41,59,194,40,59,156,38,
    57,119,36,55,87,34,51,61,21,10,5,19,8,3,17,7,
    1,15,4,0,7,7,20,5,5,11,3,2,2,2,0,0,
    0,0,38,0,0,28,0,0,19,0,0,13,0,0,4,0,
    0,0,0,0,0,0,0,0,42,41,90,38,54,123,43,36,
    255,42,4,252,31,3,187,20,0,116,10,0,61,26,24,100
};

const byte C_50ADAC_CRY[] = {
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

const byte C_CCE4A5_CRY[] = {
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

const byte C_CCEA5F_CRY[] = {
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

const byte C_B30202_CRY[] = {
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

const byte C_B87A15_CRY[] = {
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

const byte C_FFD000_CRY[] = {
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

const byte C_FFDE4C_CRY[] = {
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

const byte C_FFF588_CRY[] = {
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

const byte C_3089FF_CRY[] = {
    0,0,0,0,0,0,0,0,0,8,26,46,48,137,255,0,
    1,2,0,0,0,0,0,0,0,0,0,3,11,12,1,5,
    2,0,1,0,0,0,0,8,16,21,6,12,14,5,9,10,
    48,129,230,48,119,212,48,113,202,48,101,180,48,94,168,45,
    85,153,44,76,136,41,68,122,39,62,110,37,56,99,35,49,
    87,34,46,82,31,39,70,30,35,62,27,28,51,26,27,48,
    24,22,39,23,20,35,21,17,30,19,13,23,17,11,19,16,
    9,16,14,7,13,13,5,10,12,4,6,11,1,2,9,0,
    0,8,0,0,7,0,0,6,0,0,5,0,0,5,0,0,
    48,137,255,48,137,255,48,137,255,48,137,244,48,137,233,48,
    137,217,48,129,198,48,126,189,48,118,166,48,111,150,47,103,
    137,45,96,124,42,88,111,40,80,97,38,73,87,36,69,81,
    33,64,75,30,57,67,28,54,62,25,49,57,23,44,50,20,
    39,45,18,34,38,16,31,34,14,28,31,12,25,26,9,20,
    21,7,17,17,6,13,13,5,10,9,2,6,3,2,4,2,
    48,137,255,48,137,255,48,137,255,48,137,255,47,137,255,44,
    137,244,43,133,237,40,125,223,38,118,210,37,114,204,34,106,
    189,33,102,182,31,95,170,28,88,156,27,83,148,24,76,136,
    22,68,122,21,64,115,18,57,102,16,51,91,15,48,85,13,
    41,74,12,37,65,11,33,60,9,28,51,8,24,43,7,21,
    37,5,16,29,5,15,27,3,11,19,2,7,13,2,5,10,
    25,137,131,22,137,114,20,133,100,17,121,87,15,106,73,13,
    93,62,11,78,51,9,68,43,8,56,31,6,44,27,5,34,
    18,3,25,11,2,16,6,0,9,0,0,4,0,0,0,0,
    37,102,160,34,94,146,32,85,132,30,78,121,27,70,107,26,
    66,99,23,61,90,21,53,78,19,48,69,17,41,60,16,39,
    55,14,33,47,12,31,43,10,26,33,9,21,28,8,19,25,
    26,68,95,21,55,74,18,47,61,15,37,48,11,28,35,9,
    21,27,7,16,17,5,11,12,18,57,87,15,48,70,13,41,
    60,11,35,51,9,27,37,7,23,30,5,19,22,4,15,17,
    45,137,164,40,119,126,34,96,96,29,74,66,24,53,43,20,
    35,23,16,21,9,12,11,1,48,137,255,48,137,255,48,133,
    237,48,106,190,48,83,147,48,62,110,48,39,70,48,23,41,
    46,9,16,42,8,14,39,7,13,37,6,11,33,5,10,30,
    5,9,27,4,6,24,2,4,21,1,2,17,0,0,15,0,
    0,13,0,0,10,0,0,8,0,0,6,0,0,5,0,0,
    48,137,255,46,137,255,39,120,255,31,98,255,24,76,255,17,
    53,255,11,35,255,7,20,255,3,9,255,2,7,218,2,5,
    182,1,4,149,0,1,117,0,0,87,0,0,61,0,0,38,
    48,137,255,48,137,255,48,137,244,47,133,202,47,118,159,47,
    102,115,47,84,76,46,69,47,43,62,39,40,55,31,38,51,
    29,36,45,25,33,38,18,30,32,13,27,27,10,25,25,9,
    48,137,255,48,137,255,47,137,234,46,137,194,44,137,156,43,
    133,119,41,127,87,38,119,61,24,23,5,22,19,3,19,16,
    1,16,10,0,8,16,20,5,11,11,3,5,2,2,1,0,
    0,0,38,0,0,28,0,0,19,0,0,13,0,0,4,0,
    0,0,0,0,0,0,0,0,47,95,90,43,126,123,48,83,
    255,46,9,252,34,6,187,22,1,116,12,0,61,30,56,100
};

const byte C_A88139_CRY[] = {
    0,0,0,1,0,0,0,0,0,29,24,10,168,129,57,1,
    1,0,0,0,0,0,0,0,0,0,0,11,10,3,5,5,
    0,0,1,0,0,0,0,26,15,5,20,12,3,17,8,2,
    168,122,51,168,112,47,168,106,45,168,95,40,167,89,38,159,
    80,34,154,71,30,145,64,27,137,58,25,130,53,22,122,46,
    19,118,44,18,109,37,16,105,33,14,96,27,11,92,25,11,
    84,21,9,79,19,8,72,16,7,68,12,5,59,10,4,56,
    9,4,50,7,3,46,5,2,41,4,1,38,1,0,32,0,
    0,30,0,0,26,0,0,22,0,0,18,0,0,17,0,0,
    168,129,57,168,129,57,168,129,57,168,129,55,167,129,52,167,
    129,49,168,122,44,168,119,42,167,111,37,168,105,34,166,97,
    31,156,90,28,148,83,25,140,75,22,132,68,19,127,65,18,
    115,61,17,104,54,15,98,51,14,88,47,13,82,41,11,69,
    37,10,63,32,8,57,29,8,51,27,7,41,23,6,32,19,
    5,26,16,4,21,12,3,16,9,2,9,6,1,6,4,0,
    168,129,57,168,129,57,168,129,57,168,129,57,163,129,57,155,
    129,55,150,125,53,141,118,50,133,111,47,129,108,46,119,100,
    42,115,96,41,107,90,38,99,82,35,94,78,33,86,71,30,
    77,64,27,73,61,26,65,54,23,57,48,20,54,45,19,47,
    39,17,41,34,15,38,31,13,32,27,11,27,23,10,24,20,
    8,18,15,6,17,14,6,12,10,4,8,7,3,6,5,2,
    88,129,29,77,129,25,69,125,22,59,114,19,53,100,16,45,
    88,14,38,74,11,32,64,10,28,53,7,21,41,6,16,32,
    4,10,23,2,6,15,1,1,9,0,0,4,0,0,0,0,
    129,96,36,120,89,33,112,80,30,103,74,27,94,66,24,91,
    62,22,82,58,20,73,50,17,67,45,15,59,39,13,55,36,
    12,48,31,11,43,29,10,36,24,7,32,20,6,28,18,6,
    90,64,21,73,52,17,64,44,14,53,34,11,39,27,8,31,
    20,6,25,15,4,18,11,3,63,54,19,52,45,16,45,39,
    13,38,33,11,30,26,8,24,22,7,19,18,5,15,14,4,
    156,129,37,141,112,28,120,90,21,103,69,15,85,50,10,70,
    33,5,55,20,2,43,10,0,168,129,57,168,129,57,168,125,
    53,168,100,42,168,78,33,168,58,25,168,37,16,168,21,9,
    163,9,4,148,7,3,138,7,3,128,6,2,115,5,2,106,
    5,2,94,4,1,85,2,1,74,1,0,61,0,0,53,0,
    0,44,0,0,36,0,0,29,0,0,22,0,0,17,0,0,
    168,129,57,159,129,57,135,113,57,110,92,57,86,71,57,59,
    50,57,40,33,57,23,19,57,10,9,57,8,7,49,6,5,
    41,4,4,33,1,1,26,0,0,19,0,0,14,0,0,8,
    168,129,57,168,129,57,168,129,55,166,125,45,166,111,36,165,
    96,26,166,79,17,161,65,11,150,58,9,142,52,7,133,48,
    6,125,42,6,115,36,4,106,30,3,96,26,2,88,24,2,
    168,129,57,168,129,57,166,129,52,161,129,43,155,129,35,150,
    125,27,142,119,19,132,112,14,83,21,1,76,18,1,67,15,
    0,57,10,0,26,15,4,18,10,2,11,5,0,9,1,0,
    0,0,8,0,0,6,0,0,4,0,0,3,0,0,1,0,
    0,0,0,0,0,0,0,0,163,90,20,150,118,27,168,78,
    57,163,9,56,119,6,42,77,1,26,41,0,14,103,53,22
};

const byte C_7084C4_CRY[] = {
    0,0,0,1,0,0,0,0,0,19,25,35,112,132,196,1,
    1,2,0,0,0,0,0,0,0,0,0,7,10,9,3,5,
    2,0,1,0,0,0,0,18,16,16,14,12,11,11,8,8,
    112,125,177,112,115,163,112,109,155,112,97,138,111,91,129,106,
    82,118,102,73,105,97,66,94,91,60,85,87,54,76,81,47,
    67,79,45,63,73,38,54,70,34,48,64,27,39,61,26,37,
    56,21,30,53,19,27,48,16,23,45,12,18,40,10,15,37,
    9,12,33,7,10,31,5,8,27,4,5,25,1,2,22,0,
    0,20,0,0,17,0,0,15,0,0,12,0,0,11,0,0,
    112,132,196,112,132,196,112,132,196,112,132,188,112,132,179,112,
    132,167,112,125,152,112,122,145,112,114,128,112,107,115,111,99,
    105,104,92,95,99,85,85,94,77,75,88,70,67,85,66,62,
    77,62,58,69,55,51,65,52,48,59,48,44,54,42,38,46,
    38,35,42,33,29,38,30,26,34,27,24,27,24,20,22,20,
    16,17,16,13,14,12,10,11,9,7,6,6,2,4,4,2,
    112,132,196,112,132,196,112,132,196,112,132,196,109,132,196,103,
    132,188,100,128,182,94,121,171,89,113,161,86,110,157,79,102,
    145,77,98,140,72,92,131,66,84,120,62,80,114,57,73,105,
    51,66,94,49,62,88,43,55,78,38,49,70,36,46,65,31,
    40,57,27,35,50,25,32,46,22,27,39,18,23,33,16,20,
    28,12,16,22,11,14,21,8,10,15,5,7,10,4,5,8,
    58,132,101,51,132,88,46,128,77,40,117,67,36,102,56,30,
    90,48,25,76,39,22,66,33,18,54,24,14,42,21,11,33,
    14,7,24,8,4,16,5,0,9,0,0,4,0,0,0,0,
    86,98,123,80,91,112,75,82,101,69,76,93,62,68,82,61,
    63,76,54,59,69,49,51,60,45,46,53,39,40,46,37,37,
    42,32,32,36,29,30,33,24,25,25,21,20,22,19,18,19,
    60,65,73,49,53,57,43,45,47,35,35,37,26,27,27,21,
    21,21,17,15,13,12,11,9,42,55,67,35,46,54,30,40,
    46,25,34,39,20,26,28,16,22,23,13,18,17,10,14,13,
    104,132,126,94,115,97,80,92,74,69,71,51,57,51,33,47,
    34,18,36,21,7,29,10,1,112,132,196,112,132,196,112,128,
    182,112,102,146,112,80,113,112,60,85,112,38,54,112,22,32,
    108,9,12,98,7,11,92,7,10,85,6,8,77,5,8,71,
    5,7,63,4,5,57,2,3,50,1,2,40,0,0,35,0,
    0,29,0,0,24,0,0,19,0,0,15,0,0,11,0,0,
    112,132,196,106,132,196,90,115,196,73,94,196,57,73,196,40,
    51,196,26,34,196,15,20,196,7,9,196,5,7,168,4,5,
    140,3,4,115,1,1,90,0,0,67,0,0,47,0,0,29,
    112,132,196,112,132,196,112,132,188,111,128,155,111,113,122,110,
    98,88,111,81,58,107,67,36,100,60,30,94,53,24,89,49,
    22,83,43,19,76,37,14,71,31,10,64,26,8,59,24,7,
    112,132,196,112,132,196,111,132,180,107,132,149,103,132,120,100,
    128,91,95,122,67,88,115,47,55,22,4,51,18,2,45,15,
    1,38,10,0,18,16,15,12,10,8,7,5,2,6,1,0,
    0,0,29,0,0,22,0,0,15,0,0,10,0,0,3,0,
    0,0,0,0,0,0,0,0,109,92,69,100,121,95,112,80,
    196,108,9,194,79,6,144,51,1,89,27,0,47,69,54,77
};

const byte C_D46D3D_CRY[] = {
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

const byte C_05A8A0_CRY[] = {
    0,0,0,0,0,0,0,0,0,1,32,29,5,168,160,0,
    1,1,0,0,0,0,0,0,0,0,0,0,13,8,0,7,
    1,0,1,0,0,0,0,1,20,13,1,15,9,1,11,6,
    5,159,144,5,146,133,5,138,127,5,124,113,5,115,105,5,
    105,96,5,93,85,4,84,77,4,76,69,4,69,62,4,60,
    55,4,57,51,3,48,44,3,43,39,3,35,32,3,33,30,
    3,27,24,2,24,22,2,20,19,2,16,14,2,13,12,2,
    11,10,1,9,8,1,7,6,1,5,4,1,1,1,1,0,
    0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,
    5,168,160,5,168,160,5,168,160,5,168,153,5,168,146,5,
    168,136,5,159,124,5,155,119,5,145,104,5,136,94,5,126,
    86,5,117,78,4,108,70,4,98,61,4,89,55,4,84,51,
    3,79,47,3,70,42,3,66,39,3,61,36,2,54,31,2,
    48,28,2,42,24,2,38,21,2,35,19,1,30,16,1,25,
    13,1,20,11,1,16,8,0,12,6,0,8,2,0,5,1,
    5,168,160,5,168,160,5,168,160,5,168,160,5,168,160,5,
    168,153,4,163,149,4,154,140,4,144,132,4,140,128,4,130,
    119,3,125,114,3,117,107,3,107,98,3,102,93,3,93,85,
    2,84,77,2,79,72,2,70,64,2,63,57,2,59,53,1,
    51,46,1,45,41,1,41,38,1,35,32,1,30,27,1,26,
    23,1,20,18,1,18,17,0,13,12,0,9,8,0,7,6,
    3,168,82,2,168,72,2,163,63,2,149,55,2,130,46,1,
    115,39,1,96,32,1,84,27,1,69,19,1,54,17,0,42,
    11,0,30,7,0,20,4,0,11,0,0,5,0,0,0,0,
    4,125,100,4,115,92,3,105,83,3,96,76,3,86,67,3,
    80,62,2,75,56,2,65,49,2,59,43,2,51,38,2,47,
    35,1,41,29,1,38,27,1,32,21,1,26,18,1,23,16,
    3,83,60,2,67,46,2,57,38,2,45,30,1,35,22,1,
    26,17,1,19,11,1,14,8,2,70,55,2,59,44,1,51,
    38,1,43,32,1,34,23,1,28,19,1,23,14,0,18,11,
    5,168,103,4,146,79,4,117,60,3,90,41,3,65,27,2,
    43,14,2,26,6,1,13,1,5,168,160,5,168,160,5,163,
    149,5,130,119,5,101,92,5,76,69,5,48,44,5,28,26,
    5,11,10,4,9,9,4,9,8,4,7,7,3,7,6,3,
    6,6,3,5,4,3,3,3,2,1,1,2,0,0,2,0,
    0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,
    5,168,160,5,168,160,4,147,160,3,120,160,3,93,160,2,
    65,160,1,43,160,1,25,160,0,11,160,0,9,137,0,7,
    114,0,5,93,0,1,73,0,0,55,0,0,38,0,0,24,
    5,168,160,5,168,160,5,168,153,5,163,127,5,144,100,5,
    125,72,5,103,48,5,85,29,4,76,24,4,67,19,4,62,
    18,4,55,16,3,47,11,3,40,8,3,34,6,3,31,6,
    5,168,160,5,168,160,5,168,147,5,168,122,5,168,98,4,
    163,75,4,155,55,4,146,38,2,28,3,2,23,2,2,19,
    1,2,13,0,1,20,13,1,13,7,0,7,1,0,1,0,
    0,0,24,0,0,18,0,0,12,0,0,8,0,0,3,0,
    0,0,0,0,0,0,0,0,5,117,56,4,154,77,5,101,
    160,5,11,158,4,7,117,2,1,73,1,0,38,3,69,63
};

const byte C_FF3030_CRY[] = {
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

const byte C_6435B5_CRY[] = {
    0,0,0,1,0,0,0,0,0,17,10,33,100,53,181,1,
    0,1,0,0,0,0,0,0,0,0,0,6,4,9,3,2,
    1,0,0,0,0,0,0,16,6,15,12,5,10,10,3,7,
    100,50,163,100,46,150,100,44,143,100,39,128,99,36,119,95,
    33,109,91,29,97,86,26,87,82,24,78,78,22,70,73,19,
    62,70,18,58,65,15,50,63,14,44,57,11,36,55,10,34,
    50,9,28,47,8,25,43,6,21,40,5,16,35,4,13,33,
    4,11,30,3,9,27,2,7,24,1,4,22,0,1,19,0,
    0,18,0,0,15,0,0,13,0,0,11,0,0,10,0,0,
    100,53,181,100,53,181,100,53,181,100,53,173,100,53,165,100,
    53,154,100,50,141,100,49,134,100,46,118,100,43,106,99,40,
    97,93,37,88,88,34,79,84,31,69,78,28,62,76,27,57,
    69,25,53,62,22,48,58,21,44,53,19,40,49,17,35,41,
    15,32,38,13,27,34,12,24,30,11,22,24,10,18,19,8,
    15,15,6,12,13,5,9,9,4,6,5,2,2,4,2,1,
    100,53,181,100,53,181,100,53,181,100,53,181,97,53,181,92,
    53,173,89,51,168,84,48,158,79,46,149,77,44,145,71,41,
    134,69,39,129,64,37,121,59,34,111,56,32,105,51,29,97,
    46,26,87,44,25,82,38,22,72,34,20,65,32,18,60,28,
    16,53,24,14,46,22,13,43,19,11,36,16,9,31,14,8,
    26,11,6,21,10,6,19,7,4,13,5,3,9,4,2,7,
    52,53,93,46,53,81,41,52,71,35,47,62,32,41,52,27,
    36,44,22,30,36,19,26,31,16,22,22,13,17,19,10,13,
    13,6,10,8,4,6,4,0,4,0,0,1,0,0,0,0,
    77,39,114,71,36,104,67,33,94,62,30,86,56,27,76,54,
    25,70,49,24,64,44,21,55,40,18,49,35,16,43,33,15,
    39,29,13,33,25,12,31,22,10,23,19,8,20,17,7,18,
    54,26,67,44,21,53,38,18,43,31,14,34,23,11,25,18,
    8,19,15,6,12,11,4,9,37,22,62,31,18,50,27,16,
    43,22,14,36,18,11,26,15,9,21,11,7,16,9,6,12,
    93,53,116,84,46,89,71,37,68,61,28,47,51,21,31,42,
    14,16,33,8,6,26,4,1,100,53,181,100,53,181,100,51,
    168,100,41,135,100,32,104,100,24,78,100,15,50,100,9,29,
    97,4,11,88,3,10,82,3,9,76,2,8,69,2,7,63,
    2,6,56,1,4,51,1,3,44,0,1,36,0,0,31,0,
    0,26,0,0,21,0,0,17,0,0,13,0,0,10,0,0,
    100,53,181,95,53,181,80,46,181,65,38,181,51,29,181,35,
    20,181,24,14,181,14,8,181,6,4,181,5,3,155,4,2,
    129,2,1,106,1,0,83,0,0,62,0,0,43,0,0,27,
    100,53,181,100,53,181,100,53,173,99,51,143,99,46,113,98,
    39,82,99,32,54,96,27,33,89,24,28,84,21,22,79,20,
    21,74,17,18,68,15,13,63,12,9,57,11,7,53,10,6,
    100,53,181,100,53,181,99,53,166,96,53,138,92,53,111,89,
    51,84,85,49,62,79,46,43,49,9,4,45,7,2,40,6,
    1,34,4,0,16,6,14,11,4,8,6,2,1,5,0,0,
    0,0,27,0,0,20,0,0,13,0,0,9,0,0,3,0,
    0,0,0,0,0,0,0,0,97,37,64,89,49,87,100,32,
    181,97,4,179,71,2,133,46,0,82,24,0,43,62,22,71
};

const byte C_FFAFAF_CRY[] = {
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

const byte C_ECB866_CRY[] = {
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

const byte C_C63F23_CRY[] = {
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

const byte C_9BC8CD_CRY[] = {
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

const byte C_666666_CRY[] = {
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

const byte C_777777_CRY[] = {
    0,0,0,3,0,0,0,0,0,68,22,21,255,255,255,3,
    1,1,0,0,0,0,0,0,0,0,0,25,9,5,11,5,
    1,0,1,0,0,0,0,62,14,9,48,10,6,40,7,5,
    255,233,216,255,204,188,255,185,173,255,151,139,254,131,120,249,
    106,97,245,78,71,239,57,55,234,52,50,229,47,45,223,41,
    39,221,39,37,215,33,32,212,29,28,205,24,23,203,23,22,
    198,18,18,186,17,16,169,14,14,160,11,10,139,9,9,132,
    8,7,118,6,6,108,5,5,96,3,3,88,1,1,76,0,
    0,70,0,0,60,0,0,53,0,0,42,0,0,40,0,0,
    255,255,255,255,255,255,255,255,255,255,255,238,255,255,221,255,
    255,196,255,233,167,255,224,153,255,201,117,255,181,92,254,157,
    72,247,136,56,241,114,50,236,91,44,230,69,39,227,58,37,
    219,54,34,211,48,30,207,45,28,200,41,26,192,37,23,163,
    33,20,149,28,17,133,26,15,119,24,14,96,21,12,76,17,
    9,60,14,8,50,11,6,37,8,4,20,5,1,14,4,1,
    255,255,255,255,255,255,255,255,255,255,255,255,252,255,255,246,
    255,238,242,243,227,237,221,205,231,199,185,228,190,176,222,165,
    153,219,154,142,214,134,123,208,112,102,204,100,89,199,78,71,
    181,57,55,172,54,52,152,48,46,135,43,41,127,40,38,110,
    35,33,96,31,29,88,28,27,76,24,23,64,20,19,56,18,
    17,43,14,13,40,13,12,28,9,9,19,6,6,14,5,5,
    200,255,63,181,255,51,161,244,45,139,210,39,125,165,33,107,
    130,28,88,86,23,76,57,19,65,47,14,50,37,12,39,28,
    8,23,21,5,14,14,3,2,8,0,0,3,0,0,0,0,
    228,153,108,222,131,86,217,106,64,211,86,55,204,63,48,202,
    55,45,192,51,41,172,45,35,158,40,31,138,35,27,130,32,
    25,113,28,21,101,26,19,85,22,15,74,18,13,67,16,11,
    202,57,43,172,46,33,150,39,28,124,31,22,91,24,16,73,
    18,12,59,13,8,42,9,5,147,48,39,122,40,32,105,35,
    27,88,30,23,71,23,17,57,19,14,45,16,10,36,13,8,
    247,255,114,237,204,57,222,136,43,210,72,30,198,45,19,166,
    29,10,129,18,4,102,9,0,255,255,255,255,255,255,255,243,
    227,255,167,154,255,99,88,255,52,50,255,33,32,255,19,18,
    251,8,7,241,6,6,234,6,6,227,5,5,219,5,5,213,
    4,4,204,3,3,198,2,2,175,1,1,143,0,0,124,0,
    0,104,0,0,84,0,0,68,0,0,53,0,0,40,0,0,
    255,255,255,249,255,255,232,205,255,215,142,255,199,78,255,139,
    44,255,93,30,255,54,17,255,23,8,255,19,6,198,14,5,
    142,9,3,91,3,1,53,0,0,39,0,0,28,0,0,17,
    255,255,255,255,255,255,255,255,238,254,243,173,254,199,106,253,
    154,52,254,102,34,250,60,21,243,52,18,237,46,14,231,42,
    13,225,38,11,218,32,8,213,27,6,205,23,5,200,21,4,
    255,255,255,255,255,255,254,255,222,250,255,161,246,255,102,242,
    243,54,237,226,39,231,204,28,195,19,2,178,16,1,158,13,
    0,135,9,0,62,14,9,42,9,5,25,5,1,20,1,0,
    0,0,17,0,0,13,0,0,9,0,0,6,0,0,2,0,
    0,0,0,0,0,0,0,0,252,134,41,243,222,55,255,99,
    255,251,8,250,222,5,150,181,1,52,96,0,28,211,47,45
};

const byte C_CEA98B_CRY[] = {
    0,0,0,4,1,0,2,0,0,45,38,31,206,169,139,4,
    3,3,1,1,1,0,0,0,0,0,0,20,20,9,11,11,
    2,2,3,0,0,1,0,42,26,15,35,21,11,29,15,8,
    206,152,123,206,141,114,206,135,108,206,122,99,202,115,93,193,
    106,86,187,95,77,178,87,70,169,80,64,162,74,59,152,66,
    53,148,62,51,138,55,45,134,50,40,124,42,34,120,40,32,
    111,34,27,105,30,25,97,27,22,92,21,17,82,18,15,78,
    16,13,71,13,10,67,10,8,61,7,6,57,3,3,49,1,
    1,46,1,1,40,1,1,36,0,0,30,0,0,28,0,0,
    206,169,139,206,169,138,206,169,134,206,166,128,206,164,122,206,
    160,114,206,152,105,206,148,101,206,140,89,206,133,82,203,124,
    76,192,116,70,183,107,64,174,99,57,165,91,52,159,87,49,
    147,83,45,134,76,41,128,71,38,117,66,35,110,60,32,96,
    55,29,90,48,25,82,45,23,74,42,21,63,36,17,52,31,
    14,43,27,11,36,21,9,28,17,7,19,13,2,14,9,2,
    206,169,139,206,169,139,206,169,139,205,168,138,198,164,135,188,
    160,129,183,156,126,174,148,119,164,140,113,160,136,110,149,127,
    102,145,123,100,136,115,93,126,107,87,121,103,83,112,95,77,
    103,87,70,99,84,67,89,76,61,80,68,55,76,64,52,68,
    58,46,61,52,41,57,48,39,49,42,34,42,36,29,38,32,
    26,31,26,21,28,24,19,21,18,15,15,13,10,12,10,8,
    115,169,75,103,166,67,93,156,60,82,143,53,75,127,46,66,
    114,40,57,98,34,49,87,29,43,74,22,35,60,19,27,48,
    14,19,37,9,12,26,5,3,16,1,2,7,0,0,1,0,
    162,123,88,152,115,81,143,106,74,133,98,69,123,89,62,120,
    85,58,110,80,53,100,71,47,93,64,44,83,58,38,79,54,
    35,71,48,31,65,44,28,57,38,23,49,32,20,45,29,18,
    120,86,56,102,73,44,90,63,38,77,52,31,61,42,23,50,
    33,18,41,25,13,31,19,8,88,78,52,75,66,43,67,59,
    38,57,52,32,48,42,25,40,36,20,32,30,15,27,25,13,
    200,169,84,184,148,64,161,121,49,140,95,35,118,72,23,99,
    50,14,80,33,5,64,18,1,206,169,139,206,169,139,206,156,
    126,206,128,103,206,103,82,206,80,64,206,55,45,204,34,28,
    197,16,13,180,13,11,170,13,10,158,11,9,145,10,8,135,
    9,7,122,7,6,111,5,4,100,3,3,84,2,2,74,1,
    1,65,1,1,54,1,1,45,0,0,36,0,0,28,0,0,
    206,169,139,194,162,139,166,142,139,139,119,139,112,95,139,82,
    70,139,59,50,139,37,31,137,19,16,135,15,13,117,12,10,
    100,9,7,83,4,3,68,2,2,53,1,1,39,0,0,27,
    206,169,139,206,169,139,206,168,128,205,156,106,205,139,86,204,
    123,64,205,103,45,199,87,28,187,79,24,177,72,19,166,67,
    18,158,60,16,146,53,12,137,46,9,124,40,7,116,38,7,
    206,169,139,206,169,139,205,169,122,202,169,100,199,169,79,196,
    166,59,190,162,40,179,154,23,111,34,4,102,29,3,92,25,
    2,80,17,2,42,26,15,31,18,8,21,10,2,16,3,1,
    0,0,27,0,0,21,0,0,15,0,0,10,0,0,4,0,
    0,1,0,0,0,0,0,0,204,115,50,196,156,61,206,103,
    139,197,16,134,149,11,102,103,3,68,61,1,39,132,74,60
};

const byte C_FFCD5A_CRY[] = {
    0,0,0,5,2,0,2,0,0,56,47,20,255,205,90,5,
    4,2,1,1,0,0,0,0,0,0,0,25,24,6,13,14,
    1,2,4,0,0,1,0,52,31,10,43,25,7,36,18,5,
    255,185,79,255,171,74,255,163,70,255,148,64,250,139,60,239,
    129,55,232,116,50,220,105,46,209,97,42,200,89,38,188,80,
    35,183,76,33,171,67,29,166,60,26,153,51,22,148,48,21,
    137,41,18,130,37,16,120,32,14,114,26,11,102,22,10,97,
    19,8,88,15,7,83,12,5,75,9,4,70,4,2,61,2,
    1,57,2,1,50,2,1,45,0,0,37,0,0,35,0,0,
    255,205,90,255,205,90,255,205,86,255,202,83,255,199,79,255,
    195,74,255,185,68,255,180,65,255,170,58,255,161,53,251,150,
    49,238,141,45,227,130,41,216,121,37,204,111,34,197,106,31,
    182,100,29,166,92,27,158,86,25,145,80,23,136,72,20,119,
    67,19,111,59,16,101,55,15,92,51,13,78,44,11,64,38,
    9,53,33,7,44,26,6,35,21,4,23,15,1,17,10,1,
    255,205,90,255,205,90,255,205,90,254,204,90,245,199,88,233,
    195,84,226,189,82,215,179,77,203,170,73,198,165,71,185,154,
    66,179,150,65,168,140,60,156,130,56,150,125,54,139,116,50,
    127,105,46,122,101,43,110,92,40,99,83,36,94,78,34,84,
    70,30,75,63,27,70,58,25,61,51,22,52,43,19,47,39,
    17,38,31,14,35,29,12,26,22,10,19,15,7,15,12,5,
    142,205,48,127,202,43,115,190,39,102,174,35,93,154,30,82,
    138,26,70,119,22,61,105,19,53,89,14,43,73,12,34,59,
    9,23,45,6,15,31,4,4,19,1,2,9,0,0,2,0,
    200,149,57,188,139,53,177,129,48,165,119,44,152,109,40,148,
    103,38,136,96,35,124,86,31,115,78,28,103,70,25,98,66,
    23,88,58,20,80,54,18,70,47,15,61,39,13,56,35,12,
    148,105,36,126,88,29,112,76,24,95,64,20,75,51,15,62,
    40,12,51,31,8,38,23,5,109,94,34,93,80,28,83,72,
    24,71,63,21,59,51,16,49,44,13,40,37,10,33,31,8,
    247,205,54,228,180,42,199,147,32,173,116,23,146,88,15,123,
    60,9,99,40,4,79,22,1,255,205,90,255,205,90,255,189,
    82,255,155,67,255,125,53,255,97,42,255,67,29,252,42,18,
    244,19,8,223,16,7,210,15,7,196,14,6,179,12,5,167,
    10,5,151,9,4,138,6,3,124,4,2,104,2,1,92,2,
    1,80,2,1,67,1,0,56,0,0,45,0,0,35,0,0,
    255,205,90,240,197,90,206,172,90,172,144,90,139,116,90,102,
    85,90,73,61,90,46,38,89,23,19,88,19,15,76,15,12,
    65,11,9,54,5,4,44,3,2,35,1,1,25,0,0,17,
    255,205,90,255,205,90,255,203,83,254,190,69,254,169,55,253,
    150,42,254,125,29,246,106,18,231,96,16,219,88,12,206,81,
    12,195,73,10,181,64,8,169,56,6,154,49,5,144,46,4,
    255,205,90,255,205,90,254,205,79,250,205,65,246,205,51,243,
    202,38,235,196,26,222,187,15,137,42,3,126,35,2,114,30,
    1,99,21,1,52,31,10,38,22,5,26,12,1,20,4,0,
    0,0,17,0,0,13,0,0,10,0,0,7,0,0,3,0,
    0,0,0,0,0,0,0,0,252,140,32,243,190,40,255,125,
    90,244,19,86,185,13,66,127,4,44,75,1,25,163,90,39
};

const byte C_AC785B_CRY[] = {
    0,0,0,3,1,0,1,0,0,38,27,20,172,120,91,3,
    2,2,1,0,0,0,0,0,0,0,0,17,14,6,9,8,
    1,1,2,0,0,0,0,35,18,10,29,15,7,24,11,5,
    172,108,80,172,100,75,172,96,71,172,87,65,169,81,61,161,
    75,56,156,68,51,148,62,46,141,57,42,135,52,39,127,47,
    35,123,44,33,115,39,29,112,35,26,103,30,22,100,28,21,
    92,24,18,88,22,16,81,19,14,77,15,11,69,13,10,65,
    11,8,59,9,7,56,7,5,51,5,4,47,2,2,41,1,
    1,38,1,1,34,1,1,30,0,0,25,0,0,24,0,0,
    172,120,91,172,120,91,172,120,87,172,118,84,172,117,80,172,
    114,75,172,108,69,172,105,66,172,99,59,172,94,54,169,88,
    50,161,82,46,153,76,42,146,71,37,138,65,34,133,62,32,
    123,59,30,112,54,27,107,50,25,98,47,23,92,42,21,80,
    39,19,75,34,16,68,32,15,62,30,14,53,26,11,43,22,
    9,36,19,7,30,15,6,24,12,4,16,9,1,11,6,1,
    172,120,91,172,120,91,172,120,91,171,120,91,165,117,89,157,
    114,85,152,111,82,145,105,78,137,99,74,134,96,72,125,90,
    67,121,88,65,113,82,61,105,76,57,101,73,54,94,68,51,
    86,62,46,82,59,44,74,54,40,67,48,36,63,46,34,57,
    41,30,51,37,27,47,34,25,41,30,22,35,25,19,32,23,
    17,26,18,14,24,17,12,18,13,10,13,9,7,10,7,5,
    96,120,49,86,118,44,78,111,39,69,102,35,63,90,30,55,
    81,26,47,70,22,41,62,19,36,52,15,29,43,12,23,34,
    9,16,26,6,10,18,4,3,11,1,1,5,0,0,1,0,
    135,87,57,127,81,53,119,75,49,111,70,45,103,64,41,100,
    60,38,92,56,35,84,50,31,78,46,29,69,41,25,66,39,
    23,59,34,20,54,32,19,47,27,15,41,23,13,38,21,12,
    100,61,36,85,52,29,76,45,25,64,37,20,51,30,15,42,
    24,12,34,18,8,26,14,5,74,55,34,63,47,28,56,42,
    25,48,37,21,40,30,16,33,26,13,27,22,10,22,18,8,
    167,120,55,154,105,42,134,86,32,117,68,23,98,51,15,83,
    35,9,67,24,4,53,13,1,172,120,91,172,120,91,172,111,
    82,172,91,67,172,73,54,172,57,42,172,39,29,170,24,18,
    165,11,8,150,9,7,142,9,7,132,8,6,121,7,5,113,
    6,5,102,5,4,93,4,3,84,2,2,70,1,1,62,1,
    1,54,1,1,45,0,0,38,0,0,30,0,0,24,0,0,
    172,120,91,162,115,91,139,101,91,116,84,91,94,68,91,69,
    50,91,49,36,91,31,22,90,16,11,89,13,9,76,10,7,
    65,7,5,55,3,2,45,2,1,35,1,0,26,0,0,17,
    172,120,91,172,120,91,172,119,84,171,111,70,171,99,56,171,
    88,42,171,73,29,166,62,18,156,56,16,148,51,12,139,48,
    12,132,43,10,122,38,8,114,33,6,104,29,5,97,27,4,
    172,120,91,172,120,91,171,120,80,169,120,66,166,120,52,164,
    118,39,159,115,26,150,110,15,92,24,3,85,21,2,77,17,
    1,67,12,1,35,18,10,26,13,5,18,7,1,13,2,0,
    0,0,17,0,0,14,0,0,10,0,0,7,0,0,3,0,
    0,0,0,0,0,0,0,0,170,82,33,164,111,40,172,73,
    91,165,11,87,125,8,67,86,2,45,51,0,26,110,53,39
};

// 
// DOOM
// 

const byte C_EEC06B_DOOM[] = {
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

const byte C_D97C45_DOOM[] = {
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

const byte C_FF7F7F_DOOM[] = {
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

const byte C_55B828_DOOM[] = {
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

const byte C_BBE357_DOOM[] = {
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

const byte C_949DB9_DOOM[] = {
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

const byte C_2B3BFF_DOOM[] = {
    0,0,0,5,5,11,4,3,7,13,17,75,43,59,255,5,
    6,27,3,4,19,2,3,11,1,2,7,8,13,31,6,10,
    15,4,7,7,3,5,0,13,14,43,12,12,35,11,10,27,
    43,42,183,42,40,171,41,38,163,40,35,151,39,33,143,38,
    31,135,37,28,123,36,27,115,34,25,107,34,23,99,32,21,
    91,32,20,87,30,18,79,30,16,71,28,15,63,27,14,59,
    26,12,51,25,11,47,24,10,43,23,8,35,22,7,31,21,
    6,27,20,5,23,19,4,19,18,3,15,17,3,11,16,2,
    7,15,2,7,14,2,7,13,0,0,12,0,0,11,0,0,
    43,54,223,43,53,211,43,51,199,43,49,187,43,48,179,43,
    46,167,43,44,155,43,43,147,43,41,131,42,40,123,40,38,
    115,39,36,107,38,34,99,36,32,91,35,30,83,34,29,79,
    32,28,75,30,27,71,29,26,67,27,25,63,26,23,59,24,
    22,55,23,20,51,21,19,47,20,18,43,18,16,39,16,16,
    35,14,15,31,13,13,27,11,11,23,9,10,19,7,8,15,
    40,55,239,39,53,231,38,52,223,37,51,219,36,49,211,34,
    47,203,34,46,199,32,44,191,31,42,183,30,41,179,29,40,
    171,28,39,167,27,37,159,25,35,151,25,34,147,23,32,139,
    22,30,131,21,29,127,20,28,119,19,26,111,18,25,107,17,
    23,99,15,21,91,15,20,87,13,18,79,12,16,71,11,16,
    67,10,14,59,9,13,55,8,11,47,7,9,39,6,8,35,
    20,59,111,19,55,103,17,52,95,16,48,87,15,44,79,14,
    40,71,13,37,63,11,34,55,11,30,47,9,27,43,8,23,
    35,7,19,27,5,16,23,4,12,15,3,8,11,2,5,7,
    32,39,143,31,37,135,30,35,127,28,33,119,27,31,111,26,
    29,107,25,28,99,23,27,91,22,25,87,21,23,79,20,22,
    75,19,20,67,17,19,63,16,17,55,15,16,51,14,15,47,
    27,30,99,24,28,83,22,25,75,20,22,63,17,19,51,15,
    16,43,13,14,35,11,12,27,21,29,99,19,27,87,17,25,
    79,15,23,71,14,20,59,12,18,51,11,16,43,9,15,39,
    43,59,115,40,51,87,36,43,67,33,36,47,30,28,31,26,
    21,19,23,16,7,19,10,0,43,59,255,43,51,219,43,43,
    187,43,36,155,43,28,123,43,22,95,43,15,63,43,7,31,
    43,0,0,40,0,0,38,0,0,36,0,0,34,0,0,32,
    0,0,30,0,0,28,0,0,26,0,0,23,0,0,21,0,
    0,19,0,0,17,0,0,15,0,0,13,0,0,11,0,0,
    39,53,255,34,46,255,29,40,255,24,33,255,19,27,255,14,
    19,255,9,13,255,5,6,255,0,0,255,0,0,227,0,0,
    203,0,0,179,0,0,155,0,0,131,0,0,107,0,0,83,
    43,59,255,43,54,219,43,50,187,43,46,155,43,41,123,43,
    38,91,43,33,59,43,29,27,41,27,23,40,26,15,38,24,
    15,36,22,11,34,20,7,33,18,0,31,16,0,30,16,0,
    43,59,255,43,59,215,43,59,179,43,59,143,43,59,107,43,
    59,71,43,59,35,43,59,0,28,15,0,27,13,0,25,11,
    0,23,8,0,13,14,39,11,11,27,9,8,19,8,6,11,
    0,0,83,0,0,71,0,0,59,0,0,47,0,0,35,0,
    0,23,0,0,11,0,0,0,43,37,67,43,53,75,43,28,
    255,43,0,255,35,0,207,27,0,155,19,0,107,28,25,107
};

const byte C_50ADAC_DOOM[] = {
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

const byte C_CCE4A5_DOOM[] = {
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

const byte C_CCEA5F_DOOM[] = {
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

const byte C_B30202_DOOM[] = {
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

const byte C_B87A15_DOOM[] = {
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

const byte C_FFD000_DOOM[] = {
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

const byte C_FFDE4C_DOOM[] = {
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

const byte C_FFF588_DOOM[] = {
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

const byte C_3089FF_DOOM[] = {
    0,0,0,6,12,11,4,8,7,14,40,75,48,137,255,5,
    15,27,4,10,19,2,6,11,1,4,7,9,30,31,7,23,
    15,4,17,7,3,12,0,15,32,43,13,27,35,12,23,27,
    48,98,183,46,92,171,46,88,163,44,81,151,43,77,143,42,
    73,135,41,66,123,40,62,115,38,57,107,37,53,99,36,49,
    91,35,47,87,34,42,79,33,38,71,31,34,63,31,32,59,
    29,27,51,28,25,47,27,23,43,26,19,35,25,17,31,24,
    15,27,22,12,23,22,10,19,20,8,15,19,6,11,18,4,
    7,17,4,7,16,4,7,15,0,0,13,0,0,13,0,0,
    48,126,223,48,122,211,48,118,199,48,113,187,48,111,179,48,
    107,167,48,103,155,48,100,147,48,96,131,46,92,123,45,88,
    115,43,83,107,42,79,99,40,75,91,39,70,83,38,68,79,
    36,66,75,34,62,71,32,60,67,31,57,63,29,53,59,27,
    51,55,25,47,51,24,45,47,22,42,43,20,38,39,18,36,
    35,16,34,31,14,30,27,12,25,23,10,23,19,8,19,15,
    45,128,239,43,124,231,42,120,223,41,118,219,40,113,211,38,
    109,203,37,107,199,36,103,191,34,98,183,34,96,179,32,92,
    171,31,90,167,30,85,159,28,81,151,28,79,147,26,75,139,
    25,70,131,24,68,127,22,64,119,21,60,111,20,57,107,19,
    53,99,17,49,91,16,47,87,15,42,79,13,38,71,13,36,
    67,11,32,59,10,30,55,9,25,47,7,21,39,7,19,35,
    22,137,111,21,128,103,19,120,95,18,111,87,17,103,79,16,
    94,71,14,85,63,13,79,55,12,70,47,10,62,43,9,53,
    35,7,45,27,6,36,23,4,27,15,4,19,11,2,12,7,
    36,90,143,34,85,135,33,81,127,31,77,119,30,73,111,29,
    68,107,28,66,99,26,62,91,25,57,87,23,53,79,22,51,
    75,21,47,67,19,45,63,18,40,55,16,36,51,16,34,47,
    30,70,99,27,64,83,25,57,75,22,51,63,19,45,51,17,
    38,43,15,32,35,13,27,27,23,68,99,21,62,87,19,57,
    79,17,53,71,16,47,59,13,42,51,12,38,43,10,34,39,
    48,137,115,44,118,87,40,100,67,37,83,47,33,66,31,29,
    49,19,25,36,7,22,23,0,48,137,255,48,118,219,48,100,
    187,48,83,155,48,66,123,48,51,95,48,34,63,48,17,31,
    48,0,0,45,0,0,43,0,0,40,0,0,38,0,0,36,
    0,0,34,0,0,31,0,0,29,0,0,26,0,0,24,0,
    0,22,0,0,19,0,0,17,0,0,15,0,0,13,0,0,
    43,124,255,37,107,255,32,92,255,27,77,255,22,62,255,16,
    45,255,10,30,255,5,15,255,0,0,255,0,0,227,0,0,
    203,0,0,179,0,0,155,0,0,131,0,0,107,0,0,83,
    48,137,255,48,126,219,48,116,187,48,107,155,48,96,123,48,
    88,91,48,77,59,48,68,27,46,62,23,44,60,15,42,55,
    15,40,51,11,38,47,7,37,42,0,34,38,0,33,36,0,
    48,137,255,48,137,215,48,137,179,48,137,143,48,137,107,48,
    137,71,48,137,35,48,137,0,31,34,0,30,30,0,28,25,
    0,25,19,0,15,32,39,13,25,27,10,19,19,9,15,11,
    0,0,83,0,0,71,0,0,59,0,0,47,0,0,35,0,
    0,23,0,0,11,0,0,0,48,85,67,48,124,75,48,66,
    255,48,0,255,39,0,207,30,0,155,21,0,107,31,57,107
};

const byte C_A88139_DOOM[] = {
    0,0,0,20,12,2,15,8,2,49,38,17,168,129,57,18,
    14,6,13,10,4,7,6,2,5,4,2,31,28,7,23,22,
    3,15,16,2,10,12,0,52,30,10,47,26,8,42,22,6,
    168,93,41,163,87,38,160,82,36,155,76,34,152,72,32,147,
    68,30,144,62,27,139,58,26,134,54,24,131,50,22,126,46,
    20,123,44,19,118,40,18,115,36,16,110,32,14,107,30,13,
    102,26,11,99,24,11,94,22,10,92,18,8,86,16,7,84,
    14,6,78,12,5,76,10,4,70,8,3,68,6,2,63,4,
    2,60,4,2,55,4,2,52,0,0,47,0,0,44,0,0,
    168,119,50,168,115,47,168,111,44,168,107,42,168,105,40,168,
    101,37,168,97,35,168,95,33,168,91,29,163,87,27,157,82,
    26,152,78,24,147,74,22,142,70,20,136,66,19,134,64,18,
    126,62,17,118,58,16,113,56,15,107,54,14,102,50,13,94,
    48,12,89,44,11,84,42,11,78,40,10,70,36,9,63,34,
    8,55,32,7,49,28,6,42,24,5,34,22,4,28,18,3,
    157,121,53,152,117,52,147,113,50,144,111,49,139,107,47,134,
    103,45,131,101,44,126,97,43,121,93,41,118,91,40,113,87,
    38,110,84,37,105,80,36,99,76,34,97,74,33,92,70,31,
    86,66,29,84,64,28,78,60,27,73,56,25,70,54,24,65,
    50,22,60,46,20,57,44,19,52,40,18,47,36,16,44,34,
    15,39,30,13,36,28,12,31,24,11,26,20,9,23,18,8,
    78,129,25,73,121,23,68,113,21,63,105,19,60,97,18,55,
    89,16,49,80,14,44,74,12,42,66,11,36,58,10,31,50,
    8,26,42,6,20,34,5,15,26,3,13,18,2,7,12,2,
    126,84,32,121,80,30,115,76,28,110,72,27,105,68,25,102,
    64,24,97,62,22,92,58,20,86,54,19,81,50,18,78,48,
    17,73,44,15,68,42,14,63,38,12,57,34,11,55,32,11,
    105,66,22,94,60,19,86,54,17,78,48,14,68,42,11,60,
    36,10,52,30,8,44,26,6,81,64,22,73,58,19,68,54,
    18,60,50,16,55,44,13,47,40,11,42,36,10,36,32,9,
    168,129,26,155,111,19,142,95,15,128,78,11,115,62,7,102,
    46,4,89,34,2,76,22,0,168,129,57,168,111,49,168,95,
    42,168,78,35,168,62,27,168,48,21,168,32,14,168,16,7,
    168,0,0,157,0,0,150,0,0,142,0,0,134,0,0,126,
    0,0,118,0,0,110,0,0,102,0,0,92,0,0,84,0,
    0,76,0,0,68,0,0,60,0,0,52,0,0,44,0,0,
    152,117,57,131,101,57,113,87,57,94,72,57,76,58,57,55,
    42,57,36,28,57,18,14,57,0,0,57,0,0,51,0,0,
    45,0,0,40,0,0,35,0,0,29,0,0,24,0,0,19,
    168,129,57,168,119,49,168,109,42,168,101,35,168,91,27,168,
    82,20,168,72,13,168,64,6,160,58,5,155,56,3,147,52,
    3,142,48,2,134,44,2,128,40,0,121,36,0,115,34,0,
    168,129,57,168,129,48,168,129,40,168,129,32,168,129,24,168,
    129,16,168,129,8,168,129,0,110,32,0,105,28,0,97,24,
    0,89,18,0,52,30,9,44,24,6,36,18,4,31,14,2,
    0,0,19,0,0,16,0,0,13,0,0,11,0,0,8,0,
    0,5,0,0,2,0,0,0,168,80,15,168,117,17,168,62,
    57,168,0,57,136,0,46,105,0,35,73,0,24,110,54,24
};

const byte C_7084C4_DOOM[] = {
    0,0,0,14,12,8,10,8,5,33,39,58,112,132,196,12,
    14,21,8,10,15,5,6,8,3,4,5,21,28,24,15,22,
    12,10,16,5,7,12,0,35,31,33,31,26,27,28,22,21,
    112,95,141,108,89,131,107,84,125,103,78,116,101,74,110,98,
    70,104,96,64,95,93,60,88,89,55,82,87,51,76,84,47,
    70,82,45,67,79,41,61,77,37,55,73,33,48,72,31,45,
    68,26,39,66,24,36,63,22,33,61,18,27,58,16,24,56,
    14,21,52,12,18,51,10,15,47,8,12,45,6,8,42,4,
    5,40,4,5,36,4,5,35,0,0,31,0,0,29,0,0,
    112,122,171,112,118,162,112,113,153,112,109,144,112,107,138,112,
    103,128,112,99,119,112,97,113,112,93,101,108,89,95,105,84,
    88,101,80,82,98,76,76,94,72,70,91,68,64,89,66,61,
    84,64,58,79,60,55,75,57,51,72,55,48,68,51,45,63,
    49,42,59,45,39,56,43,36,52,41,33,47,37,30,42,35,
    27,36,33,24,33,28,21,28,24,18,22,22,15,19,18,12,
    105,124,184,101,120,178,98,115,171,96,113,168,93,109,162,89,
    105,156,87,103,153,84,99,147,80,95,141,79,93,138,75,89,
    131,73,86,128,70,82,122,66,78,116,65,76,113,61,72,107,
    58,68,101,56,66,98,52,62,91,49,57,85,47,55,82,43,
    51,76,40,47,70,38,45,67,35,41,61,31,37,55,29,35,
    51,26,31,45,24,28,42,21,24,36,17,20,30,15,18,27,
    52,132,85,49,124,79,45,115,73,42,107,67,40,99,61,36,
    91,55,33,82,48,29,76,42,28,68,36,24,60,33,21,51,
    27,17,43,21,14,35,18,10,26,12,8,18,8,5,12,5,
    84,86,110,80,82,104,77,78,98,73,74,91,70,70,85,68,
    66,82,65,64,76,61,60,70,58,55,67,54,51,61,52,49,
    58,49,45,51,45,43,48,42,39,42,38,35,39,36,33,36,
    70,68,76,63,62,64,58,55,58,52,49,48,45,43,39,40,
    37,33,35,31,27,29,26,21,54,66,76,49,60,67,45,55,
    61,40,51,55,36,45,45,31,41,39,28,37,33,24,33,30,
    112,132,88,103,113,67,94,97,51,86,80,36,77,64,24,68,
    47,15,59,35,5,51,22,0,112,132,196,112,113,168,112,97,
    144,112,80,119,112,64,95,112,49,73,112,33,48,112,16,24,
    112,0,0,105,0,0,100,0,0,94,0,0,89,0,0,84,
    0,0,79,0,0,73,0,0,68,0,0,61,0,0,56,0,
    0,51,0,0,45,0,0,40,0,0,35,0,0,29,0,0,
    101,120,196,87,103,196,75,89,196,63,74,196,51,60,196,36,
    43,196,24,28,196,12,14,196,0,0,196,0,0,174,0,0,
    156,0,0,138,0,0,119,0,0,101,0,0,82,0,0,64,
    112,132,196,112,122,168,112,111,144,112,103,119,112,93,95,112,
    84,70,112,74,45,112,66,21,107,60,18,103,57,12,98,53,
    12,94,49,8,89,45,5,86,41,0,80,37,0,77,35,0,
    112,132,196,112,132,165,112,132,138,112,132,110,112,132,82,112,
    132,55,112,132,27,112,132,0,73,33,0,70,28,0,65,24,
    0,59,18,0,35,31,30,29,24,21,24,18,15,21,14,8,
    0,0,64,0,0,55,0,0,45,0,0,36,0,0,27,0,
    0,18,0,0,8,0,0,0,112,82,51,112,120,58,112,64,
    196,112,0,196,91,0,159,70,0,119,49,0,82,73,55,82
};

const byte C_D46D3D_DOOM[] = {
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

const byte C_05A8A0_DOOM[] = {
    0,0,0,1,15,7,0,10,4,1,49,47,5,168,160,1,
    18,17,0,13,12,0,7,7,0,5,4,1,36,19,1,28,
    9,0,20,4,0,15,0,2,39,27,1,34,22,1,28,17,
    5,121,115,5,113,107,5,107,102,5,99,95,5,94,90,4,
    89,85,4,81,77,4,76,72,4,70,67,4,65,62,4,60,
    57,4,57,55,4,52,50,3,47,45,3,42,40,3,39,37,
    3,34,32,3,31,29,3,28,27,3,23,22,3,20,19,2,
    18,17,2,15,14,2,13,12,2,10,9,2,7,7,2,5,
    4,2,5,4,2,5,4,2,0,0,1,0,0,1,0,0,
    5,155,140,5,150,132,5,144,125,5,139,117,5,136,112,5,
    131,105,5,126,97,5,123,92,5,118,82,5,113,77,5,107,
    72,5,102,67,4,97,62,4,92,57,4,86,52,4,84,50,
    4,81,47,4,76,45,3,73,42,3,70,40,3,65,37,3,
    63,35,3,57,32,2,55,29,2,52,27,2,47,24,2,44,
    22,2,42,19,1,36,17,1,31,14,1,28,12,1,23,9,
    5,157,150,5,152,145,4,147,140,4,144,137,4,139,132,4,
    134,127,4,131,125,4,126,120,4,121,115,4,118,112,3,113,
    107,3,110,105,3,105,100,3,99,95,3,97,92,3,92,87,
    3,86,82,2,84,80,2,78,75,2,73,70,2,70,67,2,
    65,62,2,60,57,2,57,55,2,52,50,1,47,45,1,44,
    42,1,39,37,1,36,35,1,31,29,1,26,24,1,23,22,
    2,168,70,2,157,65,2,147,60,2,136,55,2,126,50,2,
    115,45,1,105,40,1,97,35,1,86,29,1,76,27,1,65,
    22,1,55,17,1,44,14,0,34,9,0,23,7,0,15,4,
    4,110,90,4,105,85,3,99,80,3,94,75,3,89,70,3,
    84,67,3,81,62,3,76,57,3,70,55,2,65,50,2,63,
    47,2,57,42,2,55,40,2,49,35,2,44,32,2,42,29,
    3,86,62,3,78,52,3,70,47,2,63,40,2,55,32,2,
    47,27,2,39,22,1,34,17,2,84,62,2,76,55,2,70,
    50,2,65,45,2,57,37,1,52,32,1,47,27,1,42,24,
    5,168,72,5,144,55,4,123,42,4,102,29,3,81,19,3,
    60,12,3,44,4,2,28,0,5,168,160,5,144,137,5,123,
    117,5,102,97,5,81,77,5,63,60,5,42,40,5,20,19,
    5,0,0,5,0,0,4,0,0,4,0,0,4,0,0,4,
    0,0,4,0,0,3,0,0,3,0,0,3,0,0,2,0,
    0,2,0,0,2,0,0,2,0,0,2,0,0,1,0,0,
    5,152,160,4,131,160,3,113,160,3,94,160,2,76,160,2,
    55,160,1,36,160,1,18,160,0,0,160,0,0,142,0,0,
    127,0,0,112,0,0,97,0,0,82,0,0,67,0,0,52,
    5,168,160,5,155,137,5,142,117,5,131,97,5,118,77,5,
    107,57,5,94,37,5,84,17,5,76,14,5,73,9,4,68,
    9,4,63,7,4,57,4,4,52,0,4,47,0,3,44,0,
    5,168,160,5,168,135,5,168,112,5,168,90,5,168,67,5,
    168,45,5,168,22,5,168,0,3,42,0,3,36,0,3,31,
    0,3,23,0,2,39,24,1,31,17,1,23,12,1,18,7,
    0,0,52,0,0,45,0,0,37,0,0,29,0,0,22,0,
    0,14,0,0,7,0,0,0,5,105,42,5,152,47,5,81,
    160,5,0,160,4,0,130,3,0,97,2,0,67,3,70,67
};

const byte C_FF3030_DOOM[] = {
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

const byte C_6435B5_DOOM[] = {
    0,0,0,12,5,8,9,3,5,29,16,53,100,53,181,11,
    6,19,7,4,13,4,2,8,3,1,5,18,11,22,14,9,
    11,9,6,5,6,5,0,31,12,31,28,11,25,25,9,19,
    100,38,130,97,36,121,95,34,116,92,31,107,91,30,102,87,
    28,96,86,26,87,83,24,82,80,22,76,78,21,70,75,19,
    65,73,18,62,70,16,56,69,15,50,65,13,45,64,12,42,
    61,11,36,59,10,33,56,9,31,55,7,25,51,6,22,50,
    6,19,47,5,16,45,4,13,42,3,11,40,2,8,37,1,
    5,36,1,5,33,1,5,31,0,0,28,0,0,26,0,0,
    100,49,158,100,47,150,100,46,141,100,44,133,100,43,127,100,
    41,119,100,40,110,100,39,104,100,37,93,97,36,87,94,34,
    82,91,32,76,87,31,70,84,29,65,81,27,59,80,26,56,
    75,26,53,70,24,50,67,23,48,64,22,45,61,21,42,56,
    20,39,53,18,36,50,17,33,47,16,31,42,15,28,37,14,
    25,33,13,22,29,11,19,25,10,16,20,9,13,17,7,11,
    94,50,170,91,48,164,87,46,158,86,46,155,83,44,150,80,
    42,144,78,41,141,75,40,136,72,38,130,70,37,127,67,36,
    121,65,35,119,62,33,113,59,31,107,58,31,104,55,29,99,
    51,27,93,50,26,90,47,25,84,44,23,79,42,22,76,39,
    21,70,36,19,65,34,18,62,31,16,56,28,15,50,26,14,
    48,23,12,42,22,11,39,18,10,33,15,8,28,14,7,25,
    47,53,79,44,50,73,40,46,67,37,43,62,36,40,56,33,
    36,50,29,33,45,26,31,39,25,27,33,22,24,31,18,21,
    25,15,17,19,12,14,16,9,11,11,7,7,8,4,5,5,
    75,35,102,72,33,96,69,31,90,65,30,84,62,28,79,61,
    26,76,58,26,70,55,24,65,51,22,62,48,21,56,47,20,
    53,44,18,48,40,17,45,37,16,39,34,14,36,33,13,33,
    62,27,70,56,25,59,51,22,53,47,20,45,40,17,36,36,
    15,31,31,12,25,26,11,19,48,26,70,44,24,62,40,22,
    56,36,21,50,33,18,42,28,16,36,25,15,31,22,13,28,
    100,53,82,92,46,62,84,39,48,76,32,33,69,26,22,61,
    19,13,53,14,5,45,9,0,100,53,181,100,46,155,100,39,
    133,100,32,110,100,26,87,100,20,67,100,13,45,100,6,22,
    100,0,0,94,0,0,89,0,0,84,0,0,80,0,0,75,
    0,0,70,0,0,65,0,0,61,0,0,55,0,0,50,0,
    0,45,0,0,40,0,0,36,0,0,31,0,0,26,0,0,
    91,48,181,78,41,181,67,36,181,56,30,181,45,24,181,33,
    17,181,22,11,181,11,6,181,0,0,181,0,0,161,0,0,
    144,0,0,127,0,0,110,0,0,93,0,0,76,0,0,59,
    100,53,181,100,49,155,100,45,133,100,41,110,100,37,87,100,
    34,65,100,30,42,100,26,19,95,24,16,92,23,11,87,21,
    11,84,20,8,80,18,5,76,16,0,72,15,0,69,14,0,
    100,53,181,100,53,153,100,53,127,100,53,102,100,53,76,100,
    53,50,100,53,25,100,53,0,65,13,0,62,11,0,58,10,
    0,53,7,0,31,12,28,26,10,19,22,7,13,18,6,8,
    0,0,59,0,0,50,0,0,42,0,0,33,0,0,25,0,
    0,16,0,0,8,0,0,0,100,33,48,100,48,53,100,26,
    181,100,0,181,81,0,147,62,0,110,44,0,76,65,22,76
};

const byte C_FFAFAF_DOOM[] = {
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

const byte C_ECB866_DOOM[] = {
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

const byte C_C63F23_DOOM[] = {
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

const byte C_9BC8CD_DOOM[] = {
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

const byte C_666666_DOOM[] = {
    0,0,0,21,31,7,15,20,5,50,100,50,255,255,255,18,
    36,18,13,25,13,7,15,7,5,9,5,32,73,21,23,57,
    10,15,41,5,10,31,0,53,78,29,48,68,23,42,57,18,
    255,207,159,244,199,143,239,193,133,228,185,117,223,180,106,212,
    175,95,207,164,82,197,153,77,186,142,72,181,132,66,170,121,
    61,165,116,58,154,105,53,149,94,48,138,84,42,133,78,40,
    122,68,34,117,62,32,106,57,29,101,47,23,90,41,21,85,
    36,18,80,31,15,77,25,13,72,20,10,69,15,7,64,9,
    5,61,9,5,56,9,5,53,0,0,48,0,0,45,0,0,
    255,242,212,255,236,197,255,231,181,255,225,165,255,223,154,255,
    217,138,255,212,122,255,209,111,255,204,90,244,199,82,234,193,
    77,223,188,72,212,183,66,202,177,61,191,172,56,186,169,53,
    170,164,50,154,153,48,143,148,45,133,142,42,122,132,40,106,
    126,37,95,116,34,85,110,32,80,105,29,72,94,26,64,89,
    23,56,84,21,50,73,18,42,62,15,34,57,13,29,47,10,
    234,244,234,223,239,223,212,234,212,207,231,207,197,225,197,186,
    220,186,181,217,181,170,212,170,159,207,159,154,204,154,143,199,
    143,138,196,138,127,191,127,117,185,117,111,183,111,101,177,101,
    90,172,90,85,169,85,80,158,80,74,148,74,72,142,72,66,
    132,66,61,121,61,58,116,58,53,105,53,48,94,48,45,89,
    45,40,78,40,37,73,37,32,62,32,26,52,26,23,47,23,
    80,255,74,74,244,69,69,234,64,64,223,58,61,212,53,56,
    201,48,50,191,42,45,183,37,42,172,32,37,153,29,32,132,
    23,26,110,18,21,89,15,15,68,10,13,47,7,7,31,5,
    170,196,106,159,191,95,149,185,85,138,180,80,127,175,74,122,
    169,72,111,164,66,101,153,61,90,142,58,82,132,53,80,126,
    50,74,116,45,69,110,42,64,100,37,58,89,34,56,84,32,
    127,172,66,106,158,56,90,142,50,80,126,42,69,110,34,61,
    94,29,53,78,23,45,68,18,82,169,66,74,153,58,69,142,
    53,61,132,48,56,116,40,48,105,34,42,94,29,37,84,26,
    255,255,77,228,231,58,202,209,45,175,188,32,149,164,21,122,
    121,13,95,89,5,77,57,0,255,255,255,255,231,207,255,209,
    165,255,188,122,255,164,82,255,126,64,255,84,42,255,41,21,
    255,0,0,234,0,0,218,0,0,202,0,0,186,0,0,170,
    0,0,154,0,0,138,0,0,122,0,0,101,0,0,85,0,
    0,77,0,0,69,0,0,61,0,0,53,0,0,45,0,0,
    223,239,255,181,217,255,143,199,255,106,180,255,77,153,255,56,
    110,255,37,73,255,18,36,255,0,0,255,0,0,218,0,0,
    186,0,0,154,0,0,122,0,0,90,0,0,72,0,0,56,
    255,255,255,255,242,207,255,228,165,255,217,122,255,204,82,255,
    193,61,255,180,40,255,169,18,239,153,15,228,148,10,212,137,
    10,202,126,7,186,116,5,175,105,0,159,94,0,149,89,0,
    255,255,255,255,255,202,255,255,154,255,255,106,255,255,72,255,
    255,48,255,255,23,255,255,0,138,84,0,127,73,0,111,62,
    0,95,47,0,53,78,26,45,62,18,37,47,13,32,36,7,
    0,0,56,0,0,48,0,0,40,0,0,32,0,0,23,0,
    0,15,0,0,7,0,0,0,255,191,45,255,239,50,255,164,
    255,255,0,255,191,0,191,127,0,122,74,0,72,138,142,72
};

const byte C_777777_DOOM[] = {
    0,0,0,48,10,5,36,7,3,116,34,34,255,255,255,42,
    12,12,29,9,9,17,5,5,11,3,3,73,25,14,54,19,
    7,36,14,3,23,10,0,122,27,19,110,23,16,98,19,12,
    255,143,143,251,125,125,250,112,112,246,94,94,244,82,82,241,
    69,69,239,55,55,235,52,52,232,48,48,230,45,45,226,41,
    41,224,39,39,221,36,36,219,32,32,215,28,28,214,27,27,
    210,23,23,208,21,21,204,19,19,203,16,16,199,14,14,197,
    12,12,184,10,10,178,9,9,166,7,7,160,5,5,147,3,
    3,141,3,3,129,3,3,122,0,0,110,0,0,104,0,0,
    255,224,205,255,212,187,255,199,168,255,187,150,255,181,137,255,
    168,119,255,156,100,255,150,88,255,137,63,251,125,55,248,112,
    52,244,100,48,241,88,45,237,75,41,233,63,37,232,57,36,
    226,55,34,221,52,32,217,50,30,214,48,28,210,45,27,204,
    43,25,201,39,23,197,37,21,184,36,19,166,32,18,147,30,
    16,129,28,14,116,25,12,98,21,10,79,19,9,67,16,7,
    248,230,230,244,218,218,241,205,205,239,199,199,235,187,187,232,
    174,174,230,168,168,226,156,156,223,143,143,221,137,137,217,125,
    125,215,119,119,212,106,106,208,94,94,206,88,88,203,75,75,
    199,63,63,197,57,57,184,54,54,172,50,50,166,48,48,153,
    45,45,141,41,41,135,39,39,122,36,36,110,32,32,104,30,
    30,91,27,27,85,25,25,73,21,21,60,18,18,54,16,16,
    184,255,50,172,230,46,160,205,43,147,181,39,141,156,36,129,
    131,32,116,106,28,104,88,25,98,63,21,85,52,19,73,45,
    16,60,37,12,48,30,10,36,23,7,29,16,5,17,10,3,
    226,119,82,223,106,69,219,94,57,215,82,54,212,69,50,210,
    57,48,206,55,45,203,52,41,199,48,39,191,45,36,184,43,
    34,172,39,30,160,37,28,147,34,25,135,30,23,129,28,21,
    212,63,45,204,54,37,199,48,34,184,43,28,160,37,23,141,
    32,19,122,27,16,104,23,12,191,57,45,172,52,39,160,48,
    36,141,45,32,129,39,27,110,36,23,98,32,19,85,28,18,
    255,255,52,246,199,39,237,150,30,228,100,21,219,55,14,210,
    41,9,201,30,3,178,19,0,255,255,255,255,199,199,255,150,
    150,255,100,100,255,55,55,255,43,43,255,28,28,255,14,14,
    255,0,0,248,0,0,242,0,0,237,0,0,232,0,0,226,
    0,0,221,0,0,215,0,0,210,0,0,203,0,0,197,0,
    0,178,0,0,160,0,0,141,0,0,122,0,0,104,0,0,
    244,218,255,230,168,255,217,125,255,204,82,255,178,52,255,129,
    37,255,85,25,255,42,12,255,0,0,255,0,0,212,0,0,
    174,0,0,137,0,0,100,0,0,63,0,0,48,0,0,37,
    255,255,255,255,224,199,255,193,150,255,168,100,255,137,55,255,
    112,41,255,82,27,255,57,12,250,52,10,246,50,7,241,46,
    7,237,43,5,232,39,3,228,36,0,223,32,0,219,30,0,
    255,255,255,255,255,193,255,255,137,255,255,82,255,255,48,255,
    255,32,255,255,16,255,255,0,215,28,0,212,25,0,206,21,
    0,201,16,0,122,27,18,104,21,12,85,16,9,73,12,5,
    0,0,37,0,0,32,0,0,27,0,0,21,0,0,16,0,
    0,10,0,0,5,0,0,0,255,106,30,255,218,34,255,55,
    255,255,0,255,233,0,181,212,0,100,172,0,48,215,48,48
};

const byte C_CEA98B_DOOM[] = {
    0,0,0,25,15,6,19,10,4,61,50,41,206,169,139,22,
    18,15,15,13,10,9,7,6,6,5,4,38,36,17,28,28,
    8,19,21,4,12,15,0,64,39,23,57,34,19,51,28,15,
    206,121,100,200,113,93,196,108,89,190,100,82,187,95,78,180,
    89,74,177,82,67,170,76,63,164,71,58,161,66,54,154,60,
    50,151,58,47,145,52,43,141,47,39,135,42,34,132,39,32,
    125,34,28,122,31,26,116,28,23,112,23,19,106,21,17,103,
    18,15,96,15,13,93,13,10,86,10,8,83,7,6,77,5,
    4,74,5,4,67,5,4,64,0,0,57,0,0,54,0,0,
    206,156,122,206,150,115,206,145,108,206,140,102,206,137,98,206,
    132,91,206,127,84,206,124,80,206,119,71,200,113,67,193,108,
    63,187,103,58,180,97,54,174,92,50,167,87,45,164,84,43,
    154,82,41,145,76,39,138,74,37,132,71,34,125,66,32,116,
    63,30,109,58,28,103,55,26,96,52,23,86,47,21,77,44,
    19,67,42,17,61,36,15,51,31,13,41,28,10,35,23,8,
    193,158,130,187,153,126,180,148,122,177,145,119,170,140,115,164,
    135,111,161,132,108,154,127,104,148,121,100,145,119,98,138,113,
    93,135,111,91,128,105,87,122,100,82,119,97,80,112,92,76,
    106,87,71,103,84,69,96,79,65,90,74,61,86,71,58,80,
    66,54,74,60,50,70,58,47,64,52,43,57,47,39,54,44,
    37,48,39,32,44,36,30,38,31,26,32,26,21,28,23,19,
    96,169,61,90,158,56,83,148,52,77,137,47,74,127,43,67,
    116,39,61,105,34,54,97,30,51,87,26,44,76,23,38,66,
    19,32,55,15,25,44,13,19,34,8,15,23,6,9,15,4,
    154,111,78,148,105,74,141,100,69,135,95,65,128,89,61,125,
    84,58,119,82,54,112,76,50,106,71,47,99,66,43,96,63,
    41,90,58,37,83,55,34,77,50,30,70,44,28,67,42,26,
    128,87,54,116,79,45,106,71,41,96,63,34,83,55,28,74,
    47,23,64,39,19,54,34,15,99,84,54,90,76,47,83,71,
    43,74,66,39,67,58,32,57,52,28,51,47,23,44,42,21,
    206,169,63,190,145,47,174,124,37,158,103,26,141,82,17,125,
    60,10,109,44,4,93,28,0,206,169,139,206,145,119,206,124,
    102,206,103,84,206,82,67,206,63,52,206,42,34,206,21,17,
    206,0,0,193,0,0,183,0,0,174,0,0,164,0,0,154,
    0,0,145,0,0,135,0,0,125,0,0,112,0,0,103,0,
    0,93,0,0,83,0,0,74,0,0,64,0,0,54,0,0,
    187,153,139,161,132,139,138,113,139,116,95,139,93,76,139,67,
    55,139,44,36,139,22,18,139,0,0,139,0,0,124,0,0,
    111,0,0,98,0,0,84,0,0,71,0,0,58,0,0,45,
    206,169,139,206,156,119,206,142,102,206,132,84,206,119,67,206,
    108,50,206,95,32,206,84,15,196,76,13,190,74,8,180,68,
    8,174,63,6,164,58,4,158,52,0,148,47,0,141,44,0,
    206,169,139,206,169,117,206,169,98,206,169,78,206,169,58,206,
    169,39,206,169,19,206,169,0,135,42,0,128,36,0,119,31,
    0,109,23,0,64,39,21,54,31,15,44,23,10,38,18,6,
    0,0,45,0,0,39,0,0,32,0,0,26,0,0,19,0,
    0,13,0,0,6,0,0,0,206,105,37,206,153,41,206,82,
    139,206,0,139,167,0,113,128,0,84,90,0,58,135,71,58
};

const byte C_FFCD5A_DOOM[] = {
    0,0,0,31,18,4,23,12,2,75,60,26,255,205,90,27,
    22,10,19,15,7,11,9,4,7,6,2,47,44,11,35,35,
    5,23,25,2,15,18,0,79,47,15,71,41,12,63,35,10,
    255,147,65,247,137,60,243,131,58,235,121,53,231,115,50,223,
    109,48,219,99,43,211,92,41,203,86,38,199,80,35,191,73,
    32,187,70,31,179,64,28,175,57,25,167,51,22,163,47,21,
    155,41,18,151,38,17,143,35,15,139,28,12,131,25,11,127,
    22,10,119,18,8,115,15,7,107,12,5,103,9,4,95,6,
    2,91,6,2,83,6,2,79,0,0,71,0,0,67,0,0,
    255,189,79,255,182,74,255,176,70,255,170,66,255,166,63,255,
    160,59,255,154,55,255,150,52,255,144,46,247,137,43,239,131,
    41,231,125,38,223,118,35,215,112,32,207,105,29,203,102,28,
    191,99,26,179,92,25,171,89,24,163,86,22,155,80,21,143,
    76,19,135,70,18,127,67,17,119,64,15,107,57,14,95,54,
    12,83,51,11,75,44,10,63,38,8,51,35,7,43,28,5,
    239,192,84,231,186,82,223,179,79,219,176,77,211,170,74,203,
    163,72,199,160,70,191,154,67,183,147,65,179,144,63,171,137,
    60,167,134,59,159,128,56,151,121,53,147,118,52,139,112,49,
    131,105,46,127,102,45,119,96,42,111,89,39,107,86,38,99,
    80,35,91,73,32,87,70,31,79,64,28,71,57,25,67,54,
    24,59,47,21,55,44,19,47,38,17,39,31,14,35,28,12,
    119,205,39,111,192,36,103,179,34,95,166,31,91,154,28,83,
    141,25,75,128,22,67,118,19,63,105,17,55,92,15,47,80,
    12,39,67,10,31,54,8,23,41,5,19,28,4,11,18,2,
    191,134,50,183,128,48,175,121,45,167,115,42,159,109,39,155,
    102,38,147,99,35,139,92,32,131,86,31,123,80,28,119,76,
    26,111,70,24,103,67,22,95,60,19,87,54,18,83,51,17,
    159,105,35,143,96,29,131,86,26,119,76,22,103,67,18,91,
    57,15,79,47,12,67,41,10,123,102,35,111,92,31,103,86,
    28,91,80,25,83,70,21,71,64,18,63,57,15,55,51,14,
    255,205,41,235,176,31,215,150,24,195,125,17,175,99,11,155,
    73,7,135,54,2,115,35,0,255,205,90,255,176,77,255,150,
    66,255,125,55,255,99,43,255,76,34,255,51,22,255,25,11,
    255,0,0,239,0,0,227,0,0,215,0,0,203,0,0,191,
    0,0,179,0,0,167,0,0,155,0,0,139,0,0,127,0,
    0,115,0,0,103,0,0,91,0,0,79,0,0,67,0,0,
    231,186,90,199,160,90,171,137,90,143,115,90,115,92,90,83,
    67,90,55,44,90,27,22,90,0,0,90,0,0,80,0,0,
    72,0,0,63,0,0,55,0,0,46,0,0,38,0,0,29,
    255,205,90,255,189,77,255,173,66,255,160,55,255,144,43,255,
    131,32,255,115,21,255,102,10,243,92,8,235,89,5,223,83,
    5,215,76,4,203,70,2,195,64,0,183,57,0,175,54,0,
    255,205,90,255,205,76,255,205,63,255,205,50,255,205,38,255,
    205,25,255,205,12,255,205,0,167,51,0,159,44,0,147,38,
    0,135,28,0,79,47,14,67,38,10,55,28,7,47,22,4,
    0,0,29,0,0,25,0,0,21,0,0,17,0,0,12,0,
    0,8,0,0,4,0,0,0,255,128,24,255,186,26,255,99,
    90,255,0,90,207,0,73,159,0,55,111,0,38,167,86,38
};

const byte C_AC785B_DOOM[] = {
    0,0,0,21,11,4,16,7,2,51,35,27,172,120,91,18,
    13,10,13,9,7,7,5,4,5,3,2,32,26,11,24,20,
    5,16,15,2,10,11,0,53,28,15,48,24,12,42,20,10,
    172,86,65,167,80,61,164,77,58,159,71,54,156,67,51,150,
    64,48,148,58,44,142,54,41,137,50,38,134,47,35,129,43,
    32,126,41,31,121,37,28,118,33,25,113,30,22,110,28,21,
    105,24,18,102,22,17,96,20,15,94,16,12,88,15,11,86,
    13,10,80,11,8,78,9,7,72,7,5,69,5,4,64,3,
    2,61,3,2,56,3,2,53,0,0,48,0,0,45,0,0,
    172,111,80,172,107,75,172,103,71,172,99,67,172,97,64,172,
    94,60,172,90,55,172,88,52,172,84,47,167,80,44,161,77,
    41,156,73,38,150,69,35,145,65,32,140,62,30,137,60,28,
    129,58,27,121,54,25,115,52,24,110,50,22,105,47,21,96,
    45,20,91,41,18,86,39,17,80,37,15,72,33,14,64,32,
    12,56,30,11,51,26,10,42,22,8,34,20,7,29,16,5,
    161,112,85,156,109,82,150,105,80,148,103,78,142,99,75,137,
    96,72,134,94,71,129,90,68,123,86,65,121,84,64,115,80,
    61,113,79,60,107,75,57,102,71,54,99,69,52,94,65,50,
    88,62,47,86,60,45,80,56,42,75,52,40,72,50,38,67,
    47,35,61,43,32,59,41,31,53,37,28,48,33,25,45,32,
    24,40,28,21,37,26,20,32,22,17,26,18,14,24,16,12,
    80,120,40,75,112,37,69,105,34,64,97,31,61,90,28,56,
    82,25,51,75,22,45,69,20,42,62,17,37,54,15,32,47,
    12,26,39,10,21,32,8,16,24,5,13,16,4,7,11,2,
    129,79,51,123,75,48,118,71,45,113,67,42,107,64,40,105,
    60,38,99,58,35,94,54,32,88,50,31,83,47,28,80,45,
    27,75,41,24,69,39,22,64,35,20,59,32,18,56,30,17,
    107,62,35,96,56,30,88,50,27,80,45,22,69,39,18,61,
    33,15,53,28,12,45,24,10,83,60,35,75,54,31,69,50,
    28,61,47,25,56,41,21,48,37,18,42,33,15,37,30,14,
    172,120,41,159,103,31,145,88,24,132,73,17,118,58,11,105,
    43,7,91,32,2,78,20,0,172,120,91,172,103,78,172,88,
    67,172,73,55,172,58,44,172,45,34,172,30,22,172,15,11,
    172,0,0,161,0,0,153,0,0,145,0,0,137,0,0,129,
    0,0,121,0,0,113,0,0,105,0,0,94,0,0,86,0,
    0,78,0,0,69,0,0,61,0,0,53,0,0,45,0,0,
    156,109,91,134,94,91,115,80,91,96,67,91,78,54,91,56,
    39,91,37,26,91,18,13,91,0,0,91,0,0,81,0,0,
    72,0,0,64,0,0,55,0,0,47,0,0,38,0,0,30,
    172,120,91,172,111,78,172,101,67,172,94,55,172,84,44,172,
    77,32,172,67,21,172,60,10,164,54,8,159,52,5,150,48,
    5,145,45,4,137,41,2,132,37,0,123,33,0,118,32,0,
    172,120,91,172,120,77,172,120,64,172,120,51,172,120,38,172,
    120,25,172,120,12,172,120,0,113,30,0,107,26,0,99,22,
    0,91,16,0,53,28,14,45,22,10,37,16,7,32,13,4,
    0,0,30,0,0,25,0,0,21,0,0,17,0,0,12,0,
    0,8,0,0,4,0,0,0,172,75,24,172,109,27,172,58,
    91,172,0,91,140,0,74,107,0,55,75,0,38,113,50,38
};
