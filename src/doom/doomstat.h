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


#ifndef __D_STATE__
#define __D_STATE__


#include "doomdata.h"
#include "d_loop.h"
#include "d_player.h"
#include "d_mode.h"
#include "net_defs.h"


// -----------------------------------------------------------------------------
// Command line parameters.
// -----------------------------------------------------------------------------

extern boolean nomonsters;  // checkparm of -nomonsters
extern boolean respawnparm; // checkparm of -respawn
extern boolean fastparm;    // checkparm of -fast
extern boolean devparm;     // DEBUG: launched with -devparm


// -----------------------------------------------------------------------------
// Game Mode - identify IWAD as shareware, retail etc.
// -----------------------------------------------------------------------------
extern GameMode_t     gamemode;
extern GameMission_t  gamemission;
extern GameVersion_t  gameversion;
extern GameVariant_t  gamevariant;
extern char          *gamedescription;


// -----------------------------------------------------------------------------
// Convenience macro.
// 'gamemission' can be equal to pack_chex or pack_hacx, but these are
// just modified versions of doom and doom2, and should be interpreted
// as the same most of the time.
// -----------------------------------------------------------------------------

#define logical_gamemission                             \
    (gamemission == pack_chex ? doom :                  \
     gamemission == pack_hacx ? doom2 : gamemission)


// -----------------------------------------------------------------------------
// Set if homebrew PWAD stuff has been added.
// -----------------------------------------------------------------------------
extern boolean modifiedgame;


// -----------------------------------------------------------------------------
// Selected skill type, map etc.
// -----------------------------------------------------------------------------

// Defaults for menu, methinks.
extern skill_t startskill;
extern int     startepisode;
extern int     startmap;


// -----------------------------------------------------------------------------
// Savegame slot to load on startup.  This is the value provided to
// the -loadgame option.  If this has not been provided, this is -1.
// -----------------------------------------------------------------------------

extern int      startloadgame;
extern int      gameepisode;
extern int      gamemap;
extern int      deathmatch;     // 0=Cooperative; 1=Deathmatch; 2=Altdeath
extern boolean  player_is_cheater; // [Julia] True = player was using cheats.
extern boolean  autostart;
extern skill_t  gameskill;      // Selected by user. 
extern boolean  respawnmonsters; // Nightmare mode flag, single player.
extern boolean  netgame;         // Netgame? Only true if >1 player.


// -----------------------------------------------------------------------------
// Internal parameters for sound rendering.
// These have been taken from the DOS version,
//  but are not (yet) supported with Linux
//  (e.g. no sound volume adjustment with menu.
//
// From m_menu.c:
//  Sound FX volume has default, 0 - 15
//  Music volume has default, 0 - 15
// These are multiplied by 8.
// -----------------------------------------------------------------------------

extern int sfxVolume;
extern int musicVolume;
extern int snd_MusicDevice;
extern int snd_SfxDevice;
extern int snd_DesiredMusicDevice;
extern int snd_DesiredSfxDevice;


// -----------------------------------------------------------------------------
// Status flags for refresh.
//
// Depending on view size - no status bar?
// Note that there is no way to disable the
//  status bar explicitely.
// -----------------------------------------------------------------------------

extern boolean statusbaractive;
extern boolean automapactive;   // In AutoMap mode?
extern boolean menuactive;      // Menu overlayed?
extern boolean paused;          // Game Pause?
extern boolean viewactive;
extern boolean nodrawers;
extern boolean testcontrols;
extern boolean inhelpscreens;

extern int screenblocks;
extern int testcontrols_mousespeed;
extern int viewangleoffset;
extern int consoleplayer;	
extern int displayplayer;


// -----------------------------------------------------------------------------
// Scores, rating.
// Statistics on a given map, for intermission.
// -----------------------------------------------------------------------------

extern int totalkills;
extern int totalitems;
extern int totalsecret;

// Timer, for scores.
extern int levelstarttic;   // gametic at level start
extern int leveltime;       // tics in game play for par
extern int totalleveltimes; // [crispy] CPhipps - total time for all completed levels


// -----------------------------------------------------------------------------
// DEMO playback/recording related stuff.
// No demo, there is a human player in charge?
// Disable save/end game?
// -----------------------------------------------------------------------------

extern boolean usergame;
extern boolean demoplayback;
extern boolean demorecording;
extern boolean singledemo;      // Quit after playing a demo from cmdline.
extern gamestate_t gamestate;


// -----------------------------------------------------------------------------
// Internal parameters, fixed.
// These are set by the engine, and not changed
//  according to user inputs. Partly load from
//  WAD, partly set at startup time.
// -----------------------------------------------------------------------------

#define MAX_DM_STARTS 10    // Player spawn spots for deathmatch.

extern player_t players[MAXPLAYERS];        // Bookkeeping on players - state.
extern boolean playeringame[MAXPLAYERS];    // Alive? Disconnected?

extern mapthing_t  deathmatchstarts[MAX_DM_STARTS];
extern mapthing_t *deathmatch_p;
extern mapthing_t  playerstarts[MAXPLAYERS];
extern boolean     playerstartsingame[MAXPLAYERS];

extern wbstartstruct_t wminfo;	


// -----------------------------------------------------------------------------
// Internal parameters, used for engine.
// -----------------------------------------------------------------------------

extern int mouseSensitivity;
extern int bodyqueslot;
extern int skyflatnum;
extern int rndindex;

extern char *savegamedir;       // File handling stuff.
extern char  basedefault[1024];

extern boolean precache;    // if true, load all graphics at level load

extern gamestate_t  wipegamestate;
extern ticcmd_t     *netcmds;


#endif

