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
lighttable_t   *colormaps_50ADAC;  // Middle cyan

// Visplanes data
lighttable_t ***zlight_EEC06B = NULL;
lighttable_t ***zlight_D97C45 = NULL;
lighttable_t ***zlight_FF7F7F = NULL;
lighttable_t ***zlight_55B828 = NULL;
lighttable_t ***zlight_BBE357 = NULL;
lighttable_t ***zlight_949DB9 = NULL;
lighttable_t ***zlight_2A2F6B = NULL;
lighttable_t ***zlight_50ADAC = NULL;

// Segments data
lighttable_t ***scalelight_EEC06B = NULL;
lighttable_t ***scalelight_D97C45 = NULL;
lighttable_t ***scalelight_FF7F7F = NULL;
lighttable_t ***scalelight_55B828 = NULL;
lighttable_t ***scalelight_BBE357 = NULL;
lighttable_t ***scalelight_949DB9 = NULL;
lighttable_t ***scalelight_2A2F6B = NULL;
lighttable_t ***scalelight_50ADAC = NULL;


// =============================================================================
//
//                           INITIALIZATION FUNCTIONS
//
// =============================================================================

// -----------------------------------------------------------------------------
// Main colormaps
// -----------------------------------------------------------------------------

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
}

// -----------------------------------------------------------------------------
// Colored visplanes initialization
// -----------------------------------------------------------------------------

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
}

void R_ColoredVisplanesIJLevel (int i, int j, int level)
{
    zlight_EEC06B[i][j] = colormaps_EEC06B + level*256;
    zlight_D97C45[i][j] = colormaps_D97C45 + level*256;
    zlight_FF7F7F[i][j] = colormaps_FF7F7F + level*256;
    zlight_55B828[i][j] = colormaps_55B828 + level*256;
    zlight_BBE357[i][j] = colormaps_55B828 + level*256;
    zlight_949DB9[i][j] = colormaps_949DB9 + level*256;
    zlight_2A2F6B[i][j] = colormaps_2A2F6B + level*256;
    zlight_50ADAC[i][j] = colormaps_50ADAC + level*256;
}

// -----------------------------------------------------------------------------
// Colored segments initialization
// -----------------------------------------------------------------------------

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
}

void R_ColoredSegsIJLevel (int i, int j, int level)
{
    scalelight_EEC06B[i][j] = colormaps_EEC06B + level*256;
    scalelight_D97C45[i][j] = colormaps_EEC06B + level*256;
    scalelight_FF7F7F[i][j] = colormaps_FF7F7F + level*256;
    scalelight_55B828[i][j] = colormaps_55B828 + level*256;
    scalelight_BBE357[i][j] = colormaps_55B828 + level*256;
    scalelight_949DB9[i][j] = colormaps_949DB9 + level*256;
    scalelight_2A2F6B[i][j] = colormaps_2A2F6B + level*256;
    scalelight_50ADAC[i][j] = colormaps_50ADAC + level*256;
}

// =============================================================================
//
//                             VISPLANES COLORING
//
// =============================================================================

lighttable_t **R_ColoredVisplanesColorize (int light, int color)
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
        default:        return zlight[light];        break;
    }
}

// =============================================================================
//
//                              SEGMENTS COLORING
//
// =============================================================================

lighttable_t **R_ColoredSegsColorize (int lightnum, int color)
{
    const int l = LIGHTLEVELS - 1;

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
        default:        return scalelight[BETWEEN(0, l, lightnum)];         break;
    }
}

// =============================================================================
//
//                               SPRITES COLORING
//
// =============================================================================

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
        default:        return colormaps;         break;
    }
}

// =============================================================================
//
//                       COLORED LIGHTING INJECTION TABLE
//
// =============================================================================

const sectorcolor_t sectorcolor[] =
{
    // map, sector, color table

    // Area 1: Hangar
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

    {   -1,      0,    0x000000 }
};
