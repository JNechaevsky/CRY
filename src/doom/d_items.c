//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2019 Julia Nechaevskaya
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


#include "info.h"
#include "d_items.h"


// -----------------------------------------------------------------------------
// PSPRITE ACTIONS
// [Julia] Use exact Jaguar states. Weapon timing is set in info.c.
// -----------------------------------------------------------------------------

weaponinfo_t	weaponinfo[NUMWEAPONS] =
{
    {               // FIST
    am_noammo,      // ammo
    S_PUNCHUP,      // upstate
    S_PUNCHDOWN,    // downstate
    S_PUNCH,        // readystate
    S_PUNCH1,       // atkstate
    S_NULL          // flashstate
    },

    {               // PISTOL
    am_clip,        // ammo
    S_PISTOLUP,     // upstate
    S_PISTOLDOWN,   // downstate
    S_PISTOL,       // readystate
    S_PISTOL2,      // atkstate
    S_PISTOLFLASH   // flashstate
	},

	{               // SHOTGUN
    am_shell,       // ammo
    S_SGUNUP,       // upstate
    S_SGUNDOWN,     // downstate
    S_SGUN,         // readystate
    S_SGUN2,        // atkstate
    S_SGUNFLASH1    // flashstate
	},

	{               // CHAINGUN
    am_clip,        // ammo
    S_CHAINUP,      // upstate
    S_CHAINDOWN,    // downstate
    S_CHAIN,        // readystate
    S_CHAIN1,       // atkstate
    S_CHAINFLASH1   // flashstate
	},

	{               // ROCKET LAUNCHER
    am_misl,        // ammo
    S_MISSILEUP,    // upstate
    S_MISSILEDOWN,  // downstate
    S_MISSILE,      // readystate
    S_MISSILE1,     // atkstate
    S_MISSILEFLASH1 // flashstate
	},

	{               // PLASMA GUN
    am_cell,        // ammo
    S_PLASMAUP,     // upstate
    S_PLASMADOWN,   // downstate
    S_PLASMA,       // readystate
    S_PLASMA1,      // atkstate
    S_PLASMAFLASH1  // flashstate
	},

	{               // BFG9000
    am_cell,        // ammo
    S_BFGUP,        // upstate
    S_BFGDOWN,      // downstate
    S_BFG,          // readystate
    S_BFG1,         // atkstate
    S_BFGFLASH1     // flashstate
	},

	{               // CHAINSAW
    am_noammo,      // ammo
    S_SAWUP,        // upstate
    S_SAWDOWN,      // downstate
    S_SAW,          // readystate
    S_SAW1,         // atkstate
    S_NULL          // flashstate
	}
};
 