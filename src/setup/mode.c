//
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


#include <stdlib.h>
#include <string.h>

#include "doomtype.h"
#include "config.h"
#include "textscreen.h"
#include "doomtype.h"
#include "d_mode.h"
#include "d_iwad.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_controls.h"
#include "m_misc.h"
#include "compatibility.h"
#include "display.h"
#include "joystick.h"
#include "keyboard.h"
#include "mouse.h"
#include "sound.h"
#include "mode.h"


GameMission_t gamemission;
static const iwad_t **iwads;

typedef struct
{
    char *label;
    GameMission_t mission;
    int mask;
    char *name;
    char *extra_config_file;
    char *executable;
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
    }
};

static GameSelectCallback game_selected_callback;

// Miscellaneous variables that aren't used in setup.

static int showMessages = 1;
static int screenblocks = 10;
static int detailLevel = 0;
static char *executable = NULL;
static char *game_title = "Doom";

static void BindMiscVariables(void)
{
    M_BindIntVariable("detaillevel",   &detailLevel);
    M_BindIntVariable("show_messages", &showMessages);
    M_BindIntVariable("screenblocks",   &screenblocks);
}

//
// Initialise all configuration file bindings.
//

void InitBindings(void)
{
    M_ApplyPlatformDefaults();

    // Keyboard, mouse, joystick controls

    M_BindBaseControls();
    M_BindWeaponControls();
    M_BindMapControls();
    M_BindMenuControls();

    // All other variables

    BindCompatibilityVariables();
    BindDisplayVariables();
    BindJoystickVariables();
    BindKeyboardVariables();
    BindMouseVariables();
    BindSoundVariables();
    BindMiscVariables();
}

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
    M_SetConfigFilenames(config->extra_config_file);
}

static void OpenGameSelectDialog(GameSelectCallback callback)
{
    txt_window_t *window;

    window = TXT_NewWindow("Yaguar Doom");

    TXT_AddWidget(window, TXT_NewLabel(" "));

    // [Julia] Don't lookup anything, select Doom as default game
    TXT_CloseWindow(window);
    SetMission(DEFAULT_MISSION);
    callback();
    return;

    game_selected_callback = callback;
}

void SetupMission(GameSelectCallback callback)
{
    OpenGameSelectDialog(callback);
}

char *GetExecutableName(void)
{
    return executable;
}

char *GetGameTitle(void)
{
    return game_title;
}

const iwad_t **GetIwads(void)
{
    return iwads;
}

