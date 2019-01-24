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


#ifndef __SOUNDS__
#define __SOUNDS__


#include "i_sound.h"


extern sfxinfo_t    S_sfx[];    // the complete set of sound effects
extern musicinfo_t  S_music[];  // the complete set of music

//
// Identifiers for all music in game.
//

typedef enum
{
    mus_None,
    mus_map01,
    mus_map02,
    mus_map03,
    mus_map04,
    mus_map05,
    mus_map06,
    mus_map07,
    mus_map08,
    mus_map09,
    mus_map10,
    mus_map11,
    mus_map12,
    mus_map13,
    mus_map14,
    mus_map15,
    mus_map16,
    mus_map17,
    mus_map18,
    mus_map19,
    mus_map20,
    mus_map21,
    mus_map22,
    mus_map23,
    mus_map24,
    mus_map25,
    mus_map26,
    mus_title,
    mus_inter,
    mus_ending,
    mus_extra,
    NUMMUSIC
} musicenum_t;


//
// Identifiers for all sfx in game.
//

typedef enum
{
    sfx_None,
    sfx_pistol,
    sfx_shotgn,
    sfx_sgcock,
    sfx_plasma,
    sfx_bfg,
    sfx_sawup,
    sfx_sawidl,
    sfx_sawful,
    sfx_sawhit,
    sfx_rlaunc,
    sfx_rfly,
    sfx_rxplod,
    sfx_firsht,
    sfx_firbal,
    sfx_firxpl,
    sfx_pstart,
    sfx_pstop,
    sfx_doropn,
    sfx_dorcls,
    sfx_stnmov,
    sfx_swtchn,
    sfx_swtchx,
    sfx_plpain,
    sfx_dmpain,
    sfx_popain,
    sfx_slop,
    sfx_itemup,
    sfx_wpnup,
    sfx_oof,
    sfx_telept,
    sfx_posit1,
    sfx_posit2,
    sfx_posit3,
    sfx_bgsit1,
    sfx_bgsit2,
    sfx_sgtsit,
    sfx_cacsit,
    sfx_brssit,
    sfx_cybsit,
    sfx_spisit,
    sfx_sklatk,
    sfx_sgtatk,
    sfx_claw,
    sfx_pldeth,
    sfx_podth1,
    sfx_podth2,
    sfx_podth3,
    sfx_bgdth1,
    sfx_bgdth2,
    sfx_sgtdth,
    sfx_cacdth,
    sfx_skldth,
    sfx_brsdth,
    sfx_cybdth,
    sfx_spidth,
    sfx_posact,
    sfx_bgact,
    sfx_dmact,
    sfx_noway,
    sfx_barexp,
    sfx_punch,
    sfx_itmbk,
    sfx_chgun,
    NUMSFX
} sfxenum_t;

#endif
