//
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
//     Definitions for use in networking code.
//

#ifndef NET_DEFS_H
#define NET_DEFS_H 

#include <stdio.h>

#include "doomtype.h"
#include "d_ticcmd.h"


// The maximum number of players, multiplayer/networking.
// This is the maximum supported by the networking code; individual games
// have their own values for MAXPLAYERS that can be smaller.

#define NET_MAXPLAYERS 2

// Networking and tick handling related.

#define BACKUPTICS 128

// Game settings sent by client to server when initiating game start,
// and received from the server by clients when the game starts.

typedef struct
{
    int ticdup;
    int extratics;
    int deathmatch;
    int episode;
    int nomonsters;
    int fast_monsters;
    int respawn_monsters;
    int map;
    int skill;
    int gameversion;
    int lowres_turn;
    int new_sync;
    int loadgame;

    // These fields are only used by the server when sending a game
    // start message:

    int num_players;
    int consoleplayer;

} net_gamesettings_t;



#endif /* #ifndef NET_DEFS_H */
