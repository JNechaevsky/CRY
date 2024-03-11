//
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

#include <stdlib.h>
#include <string.h>
#include "textscreen.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "multiplayer.h"
#include "mode.h"

#include "id_vars.h"

GameMission_t gamemission;
static const iwad_t **iwads;

typedef struct
{
    const char *label;
    GameMission_t mission;
    int mask;
    const char *name;
    const char *config_file;
    const char *executable;
} mission_config_t;

// Default mission to fall back on, if no IWADs are found at all:

#define DEFAULT_MISSION (&mission_configs[0])

static mission_config_t mission_configs[] =
{
    {
        "Doom",
        doom,
        IWAD_MASK_DOOM,
        "doom",
        PROGRAM_PREFIX "doom.ini",
        PROGRAM_PREFIX "doom"
    },
    {
        "Heretic",
        heretic,
        IWAD_MASK_HERETIC,
        "heretic",
        PROGRAM_PREFIX "heretic.ini",
        PROGRAM_PREFIX "heretic"
    },
    {
        "Hexen",
        hexen,
        IWAD_MASK_HEXEN,
        "hexen",
        PROGRAM_PREFIX "hexen.ini",
        PROGRAM_PREFIX "hexen"
    },
};

static GameSelectCallback game_selected_callback;

// Miscellaneous variables that aren't used in setup.

static char *executable = NULL;
static const char *game_title = "Doom";

// Set the name of the executable program to run the game:

static void SetExecutable(mission_config_t *config)
{
    char *extension;

    free(executable);

#ifdef _WIN32
    extension = ".exe";
#else
    extension = "";
#endif

    executable = M_StringJoin(config->executable, extension, NULL);
}

static void SetMission(mission_config_t *config)
{
    iwads = D_FindAllIWADs(config->mask);
    gamemission = config->mission;
    SetExecutable(config);
    game_title = config->label;
    M_SetConfigFilenames(config->config_file);
}

static mission_config_t *GetMissionForName(const char *name)
{
    int i;

    for (i=0; i<arrlen(mission_configs); ++i)
    {
        if (!strcmp(mission_configs[i].name, name))
        {
            return &mission_configs[i];
        }
    }

    return NULL;
}

// Check the name of the executable.  If it contains one of the game
// names (eg. chocolate-hexen-setup.exe) then use that game.

static boolean CheckExecutableName(GameSelectCallback callback)
{
    mission_config_t *config;
    const char *exe_name;
    int i;

    exe_name = M_GetExecutableName();

    for (i=0; i<arrlen(mission_configs); ++i)
    {
        config = &mission_configs[i];

        if (strstr(exe_name, config->name) != NULL)
        {
            SetMission(config);
            callback();
            return true;
        }
    }

    return false;
}

static void GameSelected(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(config))
{
    TXT_CAST_ARG(mission_config_t, config);

    SetMission(config);
    game_selected_callback();
}

static void OpenGameSelectDialog(GameSelectCallback callback)
{
    mission_config_t *mission = NULL;
    txt_window_t *window;
    const iwad_t **iwads;
    int num_games;
    int i;

    window = TXT_NewWindow("Select game");

    TXT_AddWidget(window, TXT_NewLabel("Select a game to configure:\n"));
    num_games = 0;

    // Add a button for each game.

    for (i=0; i<arrlen(mission_configs); ++i)
    {
        // Do we have any IWADs for this game installed?
        // If so, add a button.

        iwads = D_FindAllIWADs(mission_configs[i].mask);

        if (iwads[0] != NULL)
        {
            mission = &mission_configs[i];
            TXT_AddWidget(window, TXT_NewButton2(mission_configs[i].label,
                                                 GameSelected,
                                                 &mission_configs[i]));
            ++num_games;
        }

        free(iwads);
    }

    TXT_AddWidget(window, TXT_NewStrut(0, 1));

    // No IWADs found at all?  Fall back to doom, then.

    if (num_games == 0)
    {
        TXT_CloseWindow(window);
        SetMission(DEFAULT_MISSION);
        callback();
        return;
    }

    // Only one game? Use that game, and don't bother with a dialog.

    if (num_games == 1)
    {
        TXT_CloseWindow(window);
        SetMission(mission);
        callback();
        return;
    }

    game_selected_callback = callback;
}

void SetupMission(GameSelectCallback callback)
{
    mission_config_t *config;
    const char *mission_name;
    int p;

    //!
    // @arg <game>
    //
    // Specify the game to configure the settings for.  Valid
    // values are 'doom', 'heretic', 'hexen' and 'strife'.
    //

    p = M_CheckParm("-game");

    if (p > 0)
    {
        mission_name = myargv[p + 1];

        config = GetMissionForName(mission_name);

        if (config == NULL)
        {
            I_Error("Invalid parameter - '%s'", mission_name);
        }

        SetMission(config);
        callback();
    }
    else if (!CheckExecutableName(callback))
    {
        OpenGameSelectDialog(callback);
    }
}

const char *GetExecutableName(void)
{
    return executable;
}

const char *GetGameTitle(void)
{
    return game_title;
}

const iwad_t **GetIwads(void)
{
    return iwads;
}

