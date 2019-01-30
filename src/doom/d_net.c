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


#include <stdlib.h>

#include "doomfeatures.h"
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
#include "w_checksum.h"
#include "w_wad.h"
#include "deh_main.h"
#include "d_loop.h"
#include "jn.h"

ticcmd_t *netcmds;


// -----------------------------------------------------------------------------
// PlayerQuitGame
//
// Called when a player leaves the game
// -----------------------------------------------------------------------------

static void PlayerQuitGame(player_t *player)
{
    static char exitmsg[80];
    unsigned int player_num;

    player_num = player - players;

    // Do this the same way as Vanilla Doom does, to allow dehacked
    // replacements of this message

    M_StringCopy(exitmsg, DEH_String("Player 1 left the game"), sizeof(exitmsg));

    exitmsg[7] += player_num;

    playeringame[player_num] = false;
    players[consoleplayer].message = exitmsg;
}


// -----------------------------------------------------------------------------
// RunTic
// -----------------------------------------------------------------------------

static void RunTic(ticcmd_t *cmds, boolean *ingame)
{
    extern boolean advancedemo;
    unsigned int i;

    // Check for player quits.

    for (i = 0; i < MAXPLAYERS; ++i)
    {
        if (!demoplayback && playeringame[i] && !ingame[i])
        {
            PlayerQuitGame(&players[i]);
        }
    }

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


// -----------------------------------------------------------------------------
// LoadGameSettings
//
// Load game settings from the specified structure and
// set global variables.
// -----------------------------------------------------------------------------

static void LoadGameSettings(net_gamesettings_t *settings)
{
    unsigned int i;

    startepisode = settings->episode;
    startmap = settings->map;
    startskill = settings->skill;
    startloadgame = settings->loadgame;
    fastparm = settings->fast_monsters;
    consoleplayer = settings->consoleplayer;

    for (i = 0; i < MAXPLAYERS; ++i)
    {
        playeringame[i] = i < settings->num_players;
    }
}


// -----------------------------------------------------------------------------
// SaveGameSettings
//
// Save the game settings from global variables to the specified
// game settings structure.
// -----------------------------------------------------------------------------

static void SaveGameSettings(net_gamesettings_t *settings)
{
    // Fill in game settings structure with appropriate parameters
    // for the new game

    settings->episode = startepisode;
    settings->map = startmap;
    settings->skill = startskill;
    settings->loadgame = startloadgame;
    settings->gameversion = gameversion;
    settings->fast_monsters = fastparm;
}


// -----------------------------------------------------------------------------
// D_CheckNetGame
//
// Works out player numbers among the net participants
// -----------------------------------------------------------------------------

void D_CheckNetGame (void)
{
    net_gamesettings_t settings;

    D_RegisterLoopCallbacks(&doom_loop_interface);

    SaveGameSettings(&settings);
    D_StartNetGame(&settings, NULL);
    LoadGameSettings(&settings);
}

