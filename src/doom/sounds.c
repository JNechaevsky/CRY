//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2024 Julia Nechaevskaya
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

//
// Information about all the music
//

#define MUSIC(name) \
    { name, 0, NULL, NULL }

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
	
	MUSIC("inter"),

    MUSIC("e2m1"),
    MUSIC("e2m2"),
    MUSIC("e2m3"),
    MUSIC("e2m4"),
    MUSIC("e2m5"),
    MUSIC("e2m6"),
    MUSIC("e2m7"),
    MUSIC("e2m8"),
    MUSIC("e2m9"),
    MUSIC("e3m1"),
    MUSIC("e3m2"),
    MUSIC("e3m3"),
    MUSIC("e3m4"),
    MUSIC("e3m5"),
    MUSIC("e3m6"),
    MUSIC("e3m7"),
    MUSIC("e3m8"),
    MUSIC("e3m9"),
    // [crispy] support dedicated music tracks for the 4th episode
    MUSIC("e4m1"),
    MUSIC("e4m2"),
    MUSIC("e4m3"),
    MUSIC("e4m4"),
    MUSIC("e4m5"),
    MUSIC("e4m6"),
    MUSIC("e4m7"),
    MUSIC("e4m8"),
    MUSIC("e4m9"),
    // [crispy] Sigil
    MUSIC("e5m1"),
    MUSIC("e5m2"),
    MUSIC("e5m3"),
    MUSIC("e5m4"),
    MUSIC("e5m5"),
    MUSIC("e5m6"),
    MUSIC("e5m7"),
    MUSIC("e5m8"),
    MUSIC("e5m9"),
    // [crispy] Sigil II
    MUSIC("e6m1"),
    MUSIC("e6m2"),
    MUSIC("e6m3"),
    MUSIC("e6m4"),
    MUSIC("e6m5"),
    MUSIC("e6m6"),
    MUSIC("e6m7"),
    MUSIC("e6m8"),
    MUSIC("e6m9"),
    MUSIC("sigint"),
    MUSIC("sg2int"),
    MUSIC("inter"),
    MUSIC("intro"),
    MUSIC("bunny"),
    MUSIC("victor"),
    MUSIC("introa"),
    MUSIC("runnin"),
    MUSIC("stalks"),
    MUSIC("countd"),
    MUSIC("betwee"),
    MUSIC("doom"),
    MUSIC("the_da"),
    MUSIC("shawn"),
    MUSIC("ddtblu"),
    MUSIC("in_cit"),
    MUSIC("dead"),
    MUSIC("stlks2"),
    MUSIC("theda2"),
    MUSIC("doom2"),
    MUSIC("ddtbl2"),
    MUSIC("runni2"),
    MUSIC("dead2"),
    MUSIC("stlks3"),
    MUSIC("romero"),
    MUSIC("shawn2"),
    MUSIC("messag"),
    MUSIC("count2"),
    MUSIC("ddtbl3"),
    MUSIC("ampie"),
    MUSIC("theda3"),
    MUSIC("adrian"),
    MUSIC("messg2"),
    MUSIC("romer2"),
    MUSIC("tense"),
    MUSIC("shawn3"),
    MUSIC("openin"),
    MUSIC("evil"),
    MUSIC("ultima"),
    MUSIC("read_m"),
    MUSIC("dm2ttl"),
    MUSIC("dm2int"),
    MUSIC(NULL),
    MUSIC("musinfo")
};


//
// Information about all the sfx
//

#define SOUND(name, priority) \
  { NULL, name, priority, NULL, -1, -1, 0, 0, -1, NULL }
#define SOUND_LINK(name, priority, link_id, pitch, volume) \
  { NULL, name, priority, &S_sfx[link_id], pitch, volume, 0, 0, -1, NULL }

sfxinfo_t S_sfx[] =
{
  // S_sfx[0] needs to be a dummy for odd reasons.
  SOUND("none",   0),
  SOUND("pistol", 64),
  SOUND("shotgn", 64),
  SOUND("sgcock", 64),
  SOUND("dshtgn", 64),
  SOUND("dbopn",  64),
  SOUND("dbcls",  64),
  SOUND("dbload", 64),
  SOUND("plasma", 64),
  SOUND("bfg",    64),
  SOUND("sawup",  64),
  SOUND("sawidl", 118),
  SOUND("sawful", 64),
  SOUND("sawhit", 64),
  SOUND("rlaunc", 64),
  SOUND("rxplod", 70),
  SOUND("firsht", 70),
  SOUND("firxpl", 70),
  SOUND("pstart", 100),
  SOUND("pstop",  100),
  SOUND("doropn", 100),
  SOUND("dorcls", 100),
  SOUND("stnmov", 119),
  SOUND("swtchn", 78),
  SOUND("swtchx", 78),
  SOUND("plpain", 96),
  SOUND("dmpain", 96),
  SOUND("popain", 96),
  SOUND("vipain", 96),
  SOUND("mnpain", 96),
  SOUND("pepain", 96),
  SOUND("slop",   78),
  SOUND("itemup", 78),
  SOUND("wpnup",  78),
  SOUND("oof",    96),
  SOUND("telept", 32),
  SOUND("posit1", 98),
  SOUND("posit2", 98),
  SOUND("posit3", 98),
  SOUND("bgsit1", 98),
  SOUND("bgsit2", 98),
  SOUND("sgtsit", 98),
  SOUND("cacsit", 98),
  SOUND("brssit", 94),
  SOUND("cybsit", 92),
  SOUND("spisit", 90),
  SOUND("bspsit", 90),
  SOUND("kntsit", 90),
  SOUND("vilsit", 90),
  SOUND("mansit", 90),
  SOUND("pesit",  90),
  SOUND("sklatk", 70),
  SOUND("sgtatk", 70),
  SOUND("skepch", 70),
  SOUND("vilatk", 70),
  SOUND("claw",   70),
  SOUND("skeswg", 70),
  SOUND("pldeth", 32),
  SOUND("pdiehi", 32),
  SOUND("podth1", 70),
  SOUND("podth2", 70),
  SOUND("podth3", 70),
  SOUND("bgdth1", 70),
  SOUND("bgdth2", 70),
  SOUND("sgtdth", 70),
  SOUND("cacdth", 70),
  SOUND("skldth", 70),
  SOUND("brsdth", 32),
  SOUND("cybdth", 32),
  SOUND("spidth", 32),
  SOUND("bspdth", 32),
  SOUND("vildth", 32),
  SOUND("kntdth", 32),
  SOUND("pedth",  32),
  SOUND("skedth", 32),
  SOUND("posact", 120),
  SOUND("bgact",  120),
  SOUND("dmact",  120),
  SOUND("bspact", 100),
  SOUND("bspwlk", 100),
  SOUND("vilact", 100),
  SOUND("noway",  78),
  SOUND("barexp", 60),
  SOUND("punch",  64),
  SOUND("hoof",   70),
  SOUND("metal",  70),
  SOUND_LINK("chgun", 64, sfx_pistol, 150, 0),
  SOUND("tink",   60),
  SOUND("bdopn",  100),
  SOUND("bdcls",  100),
  SOUND("itmbk",  100),
  SOUND("flame",  32),
  SOUND("flamst", 32),
  SOUND("getpow", 60),
  SOUND("bospit", 70),
  SOUND("boscub", 70),
  SOUND("bossit", 70),
  SOUND("bospn",  70),
  SOUND("bosdth", 70),
  SOUND("manatk", 70),
  SOUND("mandth", 70),
  SOUND("sssit",  70),
  SOUND("ssdth",  70),
  SOUND("keenpn", 70),
  SOUND("keendt", 70),
  SOUND("skeact", 70),
  SOUND("skesit", 70),
  SOUND("skeatk", 70),
  SOUND("radio",  60),
  // [crispy] additional BOOM and MBF states, sprites and code pointers
  SOUND("dgsit",  98),
  SOUND("dgatk",  70),
  SOUND("dgact", 120),
  SOUND("dgdth",  70),
  SOUND("dgpain", 96),
  // [crispy] play DSSECRET if available
  SOUND("secret",  60),
};

