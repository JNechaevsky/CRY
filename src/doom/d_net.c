//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2025 Julia Nechaevskaya
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
//	DOOM Network game communication and protocol,
//	all OS independend parts.
//

#include <stdlib.h>

#include "d_main.h"
#include "m_argv.h"
#include "m_menu.h"
#include "m_misc.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "g_game.h"
#include "doomdef.h"
#include "doomstat.h"
#include "w_wad.h"

#include "d_loop.h"
#include "p_local.h"
#include "ct_chat.h"

#include "id_vars.h"

ticcmd_t *netcmds;

static void RunTic(ticcmd_t *cmds, boolean *ingame)
{
    netcmds = cmds;

    // check that there are players in the game.  if not, we cannot
    // run a tic.

    if (advancedemo)
        D_DoAdvanceDemo ();

    G_Ticker ();
}

static loop_interface_t doom_loop_interface = {
    D_ProcessEvents,
    G_BuildTiccmd,
    RunTic,
    M_Ticker
};


// Load game settings from the specified structure and
// set global variables.

static void LoadGameSettings(net_gamesettings_t *settings)
{
    unsigned int i;

    startepisode = settings->episode;
    startmap = settings->map;
    startskill = settings->skill;
    startloadgame = settings->loadgame;
    nomonsters = settings->nomonsters;
    fastparm = settings->fast_monsters;
    respawnparm = settings->respawn_monsters;
    consoleplayer = settings->consoleplayer;

    for (i = 0; i < MAXPLAYERS; ++i)
    {
        playeringame[i] = i < settings->num_players;
    }
}

// Save the game settings from global variables to the specified
// game settings structure.

static void SaveGameSettings(net_gamesettings_t *settings)
{
    // Fill in game settings structure with appropriate parameters
    // for the new game

    settings->episode = startepisode;
    settings->map = startmap;
    settings->skill = startskill;
    settings->loadgame = startloadgame;
    settings->nomonsters = nomonsters;
    settings->fast_monsters = fastparm;
    settings->respawn_monsters = respawnparm;
}

//
// D_CheckNetGame
// Works out player numbers among the net participants
//
void D_CheckNetGame (void)
{
    net_gamesettings_t settings;

    D_RegisterLoopCallbacks(&doom_loop_interface);

    SaveGameSettings(&settings);
    D_StartNetGame(&settings, NULL);
    LoadGameSettings(&settings);
}

