//
// Copyright(C) 2017-2019 Julia Nechaevskaya
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


#include "doomdef.h"
#include "doomstat.h"
#include "r_bmaps.h"
#include "jn.h"


// Walls:
int bmaptexture1, bmaptexture2, bmaptexture3, bmaptexture4, bmaptexture5;

// Terminator:
int bmap_terminator;

//
// [Julia] Lookup and init all the textures for brightmapping.
// This function is called at startup, see R_Init.
//

void R_InitBrightmaps(void)
{
    // Texture lookup. There are many strict definitions,
    // for example, no need to lookup Doom 1 textures in TNT.

    // Print informative message
    printf("\nR_Init: Brightmapping initialization.");

    // All the textures we need in Atari Jaguar...
    // Red only:
    bmaptexture1 = R_TextureNumForName("EXITSIGN");
    bmaptexture2 = R_TextureNumForName("SW2WOOD");
    bmaptexture3 = R_TextureNumForName("SW2GSTON");
    bmaptexture4 = R_TextureNumForName("SW2HOT");
    // Bright tan:
    bmaptexture5 = R_TextureNumForName("SW2GARG");

    // We need to declare a "terminator" - standard game texture,
    // presented in all Doom series and using standard light formula.
    // Otherwise, non-defined textures will use latest brightmap.
    bmap_terminator = R_TextureNumForName("BIGDOOR2");

    // Don't look up any farther
    return;
}

