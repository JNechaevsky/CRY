//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2018-2019 Julia Nechaevskaya
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


#ifndef __R_SKY__
#define __R_SKY__


#define SKYFLATNAME "F_SKY1"    // SKY, store the number for name
#define ANGLETOSKYSHIFT 22      // The sky map is 256*128*4 maps

extern int skytexture;
extern int skytexturemid;

// Called whenever the view size changes.
void R_InitSkyMap (void);

#endif
