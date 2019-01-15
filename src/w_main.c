//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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
//     Common code to parse command line, identifying WAD files to load.
//

// Russian Doom (C) 2016-2018 Julian Nechaevsky


#include "config.h"
#include "doomfeatures.h"
#include "d_iwad.h"
#include "i_system.h"
#include "m_argv.h"
#include "w_main.h"
#include "w_merge.h"
#include "w_wad.h"
#include "z_zone.h"
#include "m_misc.h"
#include "jn.h"

// Lump names that are unique to particular game types. This lets us check
// the user is not trying to play with the wrong executable, eg.
// chocolate-doom -iwad hexen.wad.
static const struct
{
    GameMission_t mission;
    char *lumpname;
} unique_lumps[] = {
    { doom,    "POSSA1" },
    { heretic, "IMPXA1" },
    { hexen,   "ETTNA1" },
    { strife,  "AGRDA1" },
};

void W_CheckCorrectIWAD(GameMission_t mission)
{
    int i;
    lumpindex_t lumpnum;

    for (i = 0; i < arrlen(unique_lumps); ++i)
    {
        if (mission != unique_lumps[i].mission)
        {
            lumpnum = W_CheckNumForName(unique_lumps[i].lumpname);

            if (lumpnum >= 0)
            {
                if (english_language)
                {
                    I_Error("\nYou are trying to use a %s IWAD file with "
                            "the %s%s binary.\nThis isn't going to work.\n"
                            "You probably want to use the %s%s binary.",
                            D_SuggestGameName(unique_lumps[i].mission,
                                            indetermined),
                            PROGRAM_PREFIX,
                            D_GameMissionString(mission),
                            PROGRAM_PREFIX,
                            D_GameMissionString(unique_lumps[i].mission));
                }
                else
                {
                    I_Error("\nВы пытаетесь запусть IWAD-файл %s с исполняемым файлом %s%s.\n"
                            "Запуск невозможен. Вам необходимо использовать исполняемый файл %s%s.",
                            D_SuggestGameName(unique_lumps[i].mission,
                                            indetermined),
                            PROGRAM_PREFIX,
                            D_GameMissionString(mission),
                            PROGRAM_PREFIX,
                            D_GameMissionString(unique_lumps[i].mission));
                }
            }
        }
    }
}

