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
// DESCRIPTION:
//	Created by a sound utility.
//	Kept as a sample, DOOM2 sounds.
//


#include <stdlib.h>

#include "doomtype.h"
#include "sounds.h"

// =============================================================================
// Information about all the music
// =============================================================================

#define MUSIC(name) { name, 0, NULL, NULL }

musicinfo_t S_music[] =
{
    MUSIC(NULL),
    MUSIC("map01"),
    MUSIC("map02"),
    MUSIC("map03"),
    MUSIC("map04"),
    MUSIC("map05"),
    MUSIC("map06"),
    MUSIC("map07"),
    MUSIC("map08"),
    MUSIC("map09"),
    MUSIC("map10"),
    MUSIC("map11"),
    MUSIC("map12"),
    MUSIC("map13"),
    MUSIC("map14"),
    MUSIC("map15"),
    MUSIC("map16"),
    MUSIC("map17"),
    MUSIC("map18"),
    MUSIC("map19"),
    MUSIC("map20"),
    MUSIC("map21"),
    MUSIC("map22"),
    MUSIC("map23"),
    MUSIC("map24"),
    MUSIC("map25"),
    MUSIC("map26"),
    MUSIC("title"),
    MUSIC("inter"),
    MUSIC("ending"),
    MUSIC("extra")
};


// =============================================================================
// Information about all the sfx
// =============================================================================

#define SOUND(name, priority) \
  { NULL, name, priority, NULL, -1, -1, 0, 0, -1, NULL }
#define SOUND_LINK(name, priority, link_id, pitch, volume) \
  { NULL, name, priority, &S_sfx[link_id], pitch, volume, 0, 0, -1, NULL }

sfxinfo_t S_sfx[] =
{
    // S_sfx[0] needs to be a dummy for odd reasons.
    SOUND("none",     0),
    SOUND("pistol",   64),
    SOUND("shotgn",   64),
    SOUND("sgcock",   64),
    SOUND("plasma",   64),
    SOUND("bfg",      64),
    SOUND("sawup",    64),
    SOUND("sawidl",   128),
    SOUND("sawful",   64),
    SOUND("sawhit",   64),
    SOUND("rlaunc",   64),
    SOUND("rfly",     64),
    SOUND("rxplod",   70),
    SOUND("firsht",   70),
    SOUND("firbal",   70),
    SOUND("firxpl",   70),
    SOUND("pstart",   100),
    SOUND("pstop",    100),
    SOUND("doropn",   100),
    SOUND("dorcls",   100),
    SOUND("stnmov",   100),
    SOUND("swtchn",   78),
    SOUND("swtchx",   78),
    SOUND("plpain",   96),
    SOUND("dmpain",   96),
    SOUND("popain",   96),
    SOUND("slop",     78),
    SOUND("itemup",   78),
    SOUND("wpnup",    78),
    SOUND("oof",      96),
    SOUND("telept",   32),
    SOUND("posit1",   98),
    SOUND("posit2",   98),
    SOUND("posit3",   98),
    SOUND("bgsit1",   98),
    SOUND("bgsit2",   98),
    SOUND("sgtsit",   98),
    SOUND("cacsit",   98),
    SOUND("brssit",   94),
    SOUND("cybsit",   92),
    SOUND("spisit",   90),
    SOUND("sklatk",   70),
    SOUND("sgtatk",   70),
    SOUND("claw",     70),
    SOUND("pldeth",   32),
    SOUND("podth1",   70),
    SOUND("podth2",   70),
    SOUND("podth3",   70),
    SOUND("bgdth1",   70),
    SOUND("bgdth2",   70),
    SOUND("sgtdth",   70),
    SOUND("cacdth",   70),
    SOUND("skldth",   70),
    SOUND("brsdth",   32),
    SOUND("cybdth",   32),
    SOUND("spidth",   32),
    SOUND("posact",   120),
    SOUND("bgact",    120),
    SOUND("dmact",    120),
    SOUND("noway",    78),
    SOUND("barexp",   60),
    SOUND("punch",    64),
    SOUND("itmbk",    120),
    SOUND_LINK("chgun", 64, sfx_pistol, 150, 0),
};

