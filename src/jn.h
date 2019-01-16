//
// Copyright(C) 2014 Fabian Greffrath
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


#ifndef __JN_H__
#define __JN_H__


// -----------------------------------------------------------------------------
// Code from Crispy Doom (crispy.h)
// -----------------------------------------------------------------------------

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef BETWEEN
#define BETWEEN(l,u,x) (((l)>(x))?(l):((x)>(u))?(u):(x))
#endif

#define singleplayer (!demorecording && !demoplayback && !netgame)


// -----------------------------------------------------------------------------
// Command line parameters
// -----------------------------------------------------------------------------

extern boolean vanillaparm;
extern boolean flip_levels_cmdline;


// -----------------------------------------------------------------------------
// Controls
// -----------------------------------------------------------------------------

extern int mlook;


// -----------------------------------------------------------------------------
// Rendering
// -----------------------------------------------------------------------------

extern int uncapped_fps;


// -----------------------------------------------------------------------------
// Optional gameplay features
// -----------------------------------------------------------------------------

extern int hightlight_things;
extern int brightmaps;
extern int translucency;
extern int swirling_liquids;
extern int colored_blood;
extern int weapon_bobbing;

extern int snd_monomode;

extern int crosshair_draw;
extern int crosshair_health;
extern int crosshair_scale;



#endif
