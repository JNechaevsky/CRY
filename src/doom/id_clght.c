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
lighttable_t   *colormaps_EEC06B;
lighttable_t   *colormaps_FF7F7F;

// Visplanes data
lighttable_t ***zlight_EEC06B = NULL;
lighttable_t ***zlight_FF7F7F = NULL;

// Segments data
lighttable_t ***scalelight_EEC06B = NULL;
lighttable_t ***scalelight_FF7F7F = NULL;


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
    colormaps_FF7F7F = (lighttable_t*) Z_Malloc((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
}

// -----------------------------------------------------------------------------
// Colored visplanes initialization
// -----------------------------------------------------------------------------

void R_ColoredVisplanesFreeI (int i)
{
    free(zlight_EEC06B[i]);
    free(zlight_FF7F7F[i]);
}

void R_ColoredVisplanesFree (void)
{
    free(zlight_EEC06B);
    free(zlight_FF7F7F);
}

void R_ColoredVisplanesMalloc (void)
{
    zlight_EEC06B = malloc(LIGHTLEVELS * sizeof(*zlight));
    zlight_FF7F7F = malloc(LIGHTLEVELS * sizeof(*zlight));
}

void R_ColoredVisplanesMAXLIGHTZ (int i)
{
    zlight_EEC06B[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
    zlight_FF7F7F[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
}

void R_ColoredVisplanesIJLevel (int i, int j, int level)
{
    zlight_EEC06B[i][j] = colormaps_EEC06B + level*256;
    zlight_FF7F7F[i][j] = colormaps_FF7F7F + level*256;
}

// -----------------------------------------------------------------------------
// Colored segments initialization
// -----------------------------------------------------------------------------

void R_ColoredSegsFreeI (int i)
{
    free(scalelight_EEC06B[i]);
    free(scalelight_FF7F7F[i]);
}

void R_ColoredSegsFree (void)
{
    free(scalelight_EEC06B);
    free(scalelight_FF7F7F);
}

void R_ColoredSegsMalloc (void)
{
    scalelight_EEC06B = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelight_FF7F7F = malloc(LIGHTLEVELS * sizeof(*scalelight));
}

void R_ColoredSegsMAXLIGHTSCALE (int i)
{
    scalelight_EEC06B[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
    scalelight_FF7F7F[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
}

void R_ColoredSegsIJLevel (int i, int j, int level)
{
    scalelight_EEC06B[i][j] = colormaps_EEC06B + level*256;
    scalelight_FF7F7F[i][j] = colormaps_FF7F7F + level*256;
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
        case 0xFF7F7F:  return zlight_FF7F7F[light]; break;
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
        case 0xFF7F7F:  return scalelight_FF7F7F[BETWEEN(0, l, lightnum)];  break;
        default:        return scalelight[BETWEEN(0, l, lightnum)];         break;
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

    {    1,      0,    0xFF7F7F },
    {    1,      1,    0xFF7F7F },
    {    1,      2,    0xFF7F7F },
    {    1,     64,    0xEEC06B },
    {    1,     65,    0xEEC06B },
    {    1,     67,    0xFF7F7F },
    {    1,     68,    0xFF7F7F },
    {    1,     69,    0xFF7F7F },
    {    1,     70,    0xFF7F7F },
    {    1,     71,    0xFF7F7F },
    {    1,     72,    0xFF7F7F },
    {    1,     73,    0xFF7F7F },

    {   -1,      0,    0x000000 }
};

