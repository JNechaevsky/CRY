//
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2014 Fabian Greffrath
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
//
// Parses [STRINGS] sections in BEX files
//

// Russian Doom (C) 2016-2018 Julian Nechaevsky


#include <stdio.h>
#include <string.h>

#include "deh_defs.h"
#include "deh_io.h"
#include "deh_main.h"
#include "d_englsh.h"


typedef struct {
    char *macro;
    char *string;
} bex_string_t;

// mnemonic keys table
static const bex_string_t bex_stringtable[] = {
    // part 1 - general initialization and prompts
    {"D_DEVSTR", D_DEVSTR},
    {"D_CDROM", D_CDROM},
    {"QUITMSG", QUITMSG},
    {"QSAVESPOT", QSAVESPOT},
    {"SAVEDEAD", SAVEDEAD},
    {"MSGOFF", MSGOFF},
    {"MSGON", MSGON},
    {"ENDGAME", ENDGAME},
    {"DETAILHI", DETAILHI},
    {"DETAILLO", DETAILLO},
    //  [JN] Should not be translated
    // {"GAMMALVL0", GAMMALVL0},
    // {"GAMMALV02", GAMMALV02},
    // {"GAMMALV05", GAMMALV05},
    // {"GAMMALV07", GAMMALV07},
    // {"GAMMALVL1", GAMMALVL1},
    // {"GAMMALV12", GAMMALV12},
    // {"GAMMALV15", GAMMALV15},
    // {"GAMMALVL2", GAMMALVL2},
    // {"GAMMALV22", GAMMALV22},
    // {"GAMMALV25", GAMMALV25},
    // {"GAMMALV27", GAMMALV27},
    // {"GAMMALVL3", GAMMALVL3},
    // {"GAMMALV32", GAMMALV32},
    // {"GAMMALV35", GAMMALV35},
    // {"GAMMALV37", GAMMALV37},
    // {"GAMMALVL4", GAMMALVL4},
    {"EMPTYSTRING", EMPTYSTRING},
    {"GGSAVED", GGSAVED},
    {"GGLOADED", GGLOADED},
    {"SAVEGAMENAME", SAVEGAMENAME},
    // part 2 - messages when the player gets things
    {"GOTARMOR", GOTARMOR},
    {"GOTMEGA", GOTMEGA},
    {"GOTHTHBONUS", GOTHTHBONUS},
    {"GOTARMBONUS", GOTARMBONUS},
    {"GOTSTIM", GOTSTIM},
    {"GOTMEDINEED", GOTMEDINEED},
    {"GOTMEDIKIT", GOTMEDIKIT},
    {"GOTSUPER", GOTSUPER},
    {"GOTBLUECARD", GOTBLUECARD},
    {"GOTYELWCARD", GOTYELWCARD},
    {"GOTREDCARD", GOTREDCARD},
    {"GOTBLUESKUL", GOTBLUESKUL},
    {"GOTYELWSKUL", GOTYELWSKUL},
    {"GOTREDSKULL", GOTREDSKULL},
    {"GOTINVUL", GOTINVUL},
    {"GOTBERSERK", GOTBERSERK},
    {"GOTSUIT", GOTSUIT},
    {"GOTMAP", GOTMAP},
    {"GOTCLIP", GOTCLIP},
    {"GOTCLIPBOX", GOTCLIPBOX},
    {"GOTROCKET", GOTROCKET},
    {"GOTROCKET2", GOTROCKET2}, // [JN] Получено две ракеты.
    {"GOTROCKBOX", GOTROCKBOX},
    {"GOTCELL", GOTCELL},
    {"GOTCELLBOX", GOTCELLBOX},
    {"GOTSHELLS", GOTSHELLS},
    {"GOTSHELLS8", GOTSHELLS8}, // [JN] Получено 8 патронов для дробовика.
    {"GOTSHELLBOX", GOTSHELLBOX},
    {"GOTBACKPACK", GOTBACKPACK},
    {"GOTBFG9000", GOTBFG9000},
    {"GOTCHAINGUN", GOTCHAINGUN},
    {"GOTCHAINSAW", GOTCHAINSAW},
    {"GOTLAUNCHER", GOTLAUNCHER},
    {"GOTPLASMA", GOTPLASMA},
    {"GOTSHOTGUN", GOTSHOTGUN},
    // part 3 - messages when keys are needed
    // [Julia] TODO - fill with keys or skulls
    // part 4 - multiplayer messaging
    {"HUSTR_MSGU", HUSTR_MSGU},
    {"HUSTR_PLRGREEN", HUSTR_PLRGREEN},
    {"HUSTR_PLRINDIGO", HUSTR_PLRINDIGO},
    {"HUSTR_PLRBROWN", HUSTR_PLRBROWN},
    {"HUSTR_PLRRED", HUSTR_PLRRED},
    // part 5 - level names in the automap
    // ...
    // part 6 - messages as a result of toggling states
    {"AMSTR_FOLLOWON", AMSTR_FOLLOWON},
    {"AMSTR_FOLLOWOFF", AMSTR_FOLLOWOFF},
    {"AMSTR_GRIDON", AMSTR_GRIDON},
    {"AMSTR_GRIDOFF", AMSTR_GRIDOFF},
    {"AMSTR_MARKEDSPOT", AMSTR_MARKEDSPOT},
    {"AMSTR_MARKSCLEARED", AMSTR_MARKSCLEARED},
    {"STSTR_MUS", STSTR_MUS},
    {"STSTR_NOMUS", STSTR_NOMUS},
    {"STSTR_DQDON", STSTR_DQDON},
    {"STSTR_DQDOFF", STSTR_DQDOFF},
    {"STSTR_KFAADDED", STSTR_KFAADDED},
    {"STSTR_FAADDED", STSTR_FAADDED},
    {"STSTR_KAADDED", STSTR_KAADDED},
    {"STSTR_NCON", STSTR_NCON},
    {"STSTR_NCOFF", STSTR_NCOFF},
    {"STSTR_BEHOLD", STSTR_BEHOLD},
    {"STSTR_BEHOLDX", STSTR_BEHOLDX},
    {"STSTR_CHOPPERS", STSTR_CHOPPERS},
    {"STSTR_CLEV", STSTR_CLEV},
    // part 7 - episode intermission texts
    // ...
    // part 8 - creature names for the finale
    {"CC_ZOMBIE", CC_ZOMBIE},
    {"CC_SHOTGUN", CC_SHOTGUN},
    {"CC_IMP", CC_IMP},
    {"CC_DEMON", CC_DEMON},
    {"CC_LOST", CC_LOST},
    {"CC_CACO", CC_CACO},
    {"CC_BARON", CC_BARON},
    {"CC_HERO", CC_HERO},
    // part 9 - intermission tiled backgrounds
    {"BGFLATE1", "FLOOR4_8"},
    {"BGFLATE2", "SFLR6_1"},
    {"BGFLATE3", "MFLR8_4"},
    {"BGFLATE4", "MFLR8_3"},
    {"BGFLAT06", "SLIME16"},
    {"BGFLAT11", "RROCK14"},
    {"BGFLAT20", "RROCK07"},
    {"BGFLAT30", "RROCK17"},
    {"BGFLAT15", "RROCK13"},
    {"BGFLAT31", "RROCK19"},
    {"BGCASTCALL", "BOSSBACK"},
};

static void *DEH_BEXStrStart(deh_context_t *context, char *line)
{
    char s[10];

    if (sscanf(line, "%9s", s) == 0 || strncmp("[STRINGS]", s, sizeof(s)))
    {
	DEH_Warning(context, "Parse error on section start");
    }

    return NULL;
}

static void DEH_BEXStrParseLine(deh_context_t *context, char *line, void *tag)
{
    char *variable_name, *value;
    int i;

    if (!DEH_ParseAssignment(line, &variable_name, &value))
    {
	DEH_Warning(context, "Failed to parse assignment");
	return;
    }

    for (i = 0; i < arrlen(bex_stringtable); i++)
    {
	if (!strcmp(bex_stringtable[i].macro, variable_name))
	{
	    DEH_AddStringReplacement(bex_stringtable[i].string, value);
	}
    }
}

deh_section_t deh_section_bexstr =
{
    "[STRINGS]",
    NULL,
    DEH_BEXStrStart,
    DEH_BEXStrParseLine,
    NULL,
    NULL,
};
