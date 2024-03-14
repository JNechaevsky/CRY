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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include "doomkeys.h"
#include "doomstat.h"
#include "dstrings.h"
#include "d_iwad.h"
#include "z_zone.h"
#include "w_main.h"
#include "w_wad.h"
#include "s_sound.h"
#include "v_video.h"
#include "f_finale.h"
#include "f_wipe.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_controls.h"
#include "m_misc.h"
#include "m_menu.h"
#include "p_local.h"
#include "i_input.h"
#include "i_joystick.h"
#include "i_system.h"
#include "g_game.h"
#include "wi_stuff.h"
#include "st_bar.h"
#include "am_map.h"
#include "net_client.h"
#include "net_dedicated.h"
#include "net_query.h"
#include "d_main.h"
#include "ct_chat.h"
#include "r_local.h"
#include "v_trans.h"

#include "icon.c"

#include "id_vars.h"
#include "id_func.h"


// Location where savegames are stored
char *savegamedir;

// location of IWAD and WAD files
static char *iwadfile;

boolean nomonsters;   // checkparm of -nomonsters
boolean respawnparm;  // checkparm of -respawn
boolean fastparm;     // checkparm of -fast

int     startepisode;
int     startmap;
int     startloadgame;
skill_t startskill;
boolean autostart;

boolean advancedemo;

static int   pagetic;

// If true, the main game loop has started.
boolean main_loop_started = false;

// wipegamestate can be set to -1 to force a wipe on the next draw
gamestate_t wipegamestate = GS_DEMOSCREEN;

// [JN] Check for available SSG from Crispy Doom.
boolean havessg = false;


// -----------------------------------------------------------------------------
// D_ProcessEvents
// Send all the events of the given timestamp down the responder chain
// -----------------------------------------------------------------------------

void D_ProcessEvents (void)
{
    event_t *ev;

    while ((ev = D_PopEvent()) != NULL)
    {
        if (M_Responder (ev))
        {
            continue;  // menu ate the event
        }
        G_Responder (ev);
    }
}

// -----------------------------------------------------------------------------
// ID_DrawMessage
// [JN] Draws message on the screen.
// -----------------------------------------------------------------------------

static void ID_DrawMessage (void)
{
    player_t *player = &players[displayplayer];

    if (player->messageTics <= 0 || !player->message)
    {
        return;  // No message
    }

    // Draw message, where?
    if (msg_alignment == 0)
    {
        // Left
        M_WriteText(0 - WIDESCREENDELTA, 0, player->message, player->messageColor);
    }
    else
    if (msg_alignment == 1)
    {
        // Status bar
        M_WriteText(0, 0, player->message, player->messageColor);
    }
    else
    {
        // Centered
        M_WriteTextCentered(0, player->message, player->messageColor);
    }
}

// -----------------------------------------------------------------------------
// ID_DrawMessage
// [JN] Draws message on the screen.
// -----------------------------------------------------------------------------

static void ID_DrawMessageCentered (void)
{
    player_t *player = &players[displayplayer];

    if (player->messageCenteredTics <= 0 || !player->messageCentered)
    {
        return;  // No message
    }

    // Always centered
    M_WriteTextCentered(63, player->messageCentered, player->messageCenteredColor);
}

// -----------------------------------------------------------------------------
// D_Display
//  draw current display, possibly wiping it from the previous
// -----------------------------------------------------------------------------

static void D_Display (void)
{
    int      nowtime;
    int      tics;
    int      wipestart;
    boolean  done;
    boolean  wipe;
    static   gamestate_t oldgamestate = -1;

    if (nodrawers)
    {
        return;  // for comparative timing / profiling
    }

    // [crispy] post-rendering function pointer to apply config changes
    // that affect rendering and that are better applied after the current
    // frame has finished rendering
    if (post_rendering_hook)
    {
        post_rendering_hook();
        post_rendering_hook = NULL;
    }

    // change the view size if needed
    if (setsizeneeded)
    {
        R_ExecuteSetViewSize();
        oldgamestate = -1;  // force background redraw
    }

    // save the current screen if about to wipe
    // [JN] Make screen wipe optional, use external config variable.
    if (gamestate != wipegamestate && vid_screenwipe)
    {
        wipe = true;
        wipe_StartScreen();
    }
    else
    {
        wipe = false;
    }

    // do buffered drawing
    switch (gamestate)
    {
        case GS_LEVEL:
        break;

        case GS_INTERMISSION:
        WI_Drawer();
        break;

        case GS_FINALE:
        F_Drawer();
        break;

        case GS_DEMOSCREEN:
		// [JN] Jaguar: always show white background on demo screen.
		// Swap big Doom logo with credits screen every 10 seconds,
		// but don't draw them while active menu.
        V_DrawPatchFullScreen(W_CacheLumpName("M_TITLE", PU_CACHE), false);
        if (!menuactive)
		{
			V_DrawPatch(0, 0, W_CacheLumpName((pagetic < 10 * TICRATE ?
			                                   "CREDITS" : "TITLE"), PU_CACHE));
		}
        break;
    }

    // draw the view directly
    if (gamestate == GS_LEVEL && gametic)
    {
        R_RenderPlayerView(&players[displayplayer]);
    }

    // [JN] Fail-safe: return earlier if post rendering hook is still active.
    if (post_rendering_hook)
    {
        return;
    }

    // clean up border stuff
    if (gamestate != oldgamestate && gamestate != GS_LEVEL)
	I_SetPalette (0);

    // see if the border needs to be initially drawn
    if (gamestate == GS_LEVEL && oldgamestate != GS_LEVEL)
    {
        R_FillBackScreen();  // draw the pattern into the back screen
    }

    // see if the border needs to be updated to the screen
    if (gamestate == GS_LEVEL && scaledviewwidth != SCREENWIDTH)
    {
        R_DrawViewBorder();  // erase old menu stuff
    }

    // [JN] Draw automap on top of player view and view border,
    // and update while playing. This also needed for widgets update.
    if (automapactive)
    {
        AM_Drawer();
    }

    // [JN] Allow to draw level name separately from automap.
    if (gamestate == GS_LEVEL && widget_levelname)
    {
        AM_LevelNameDrawer();
    }

    oldgamestate = wipegamestate = gamestate;

    // draw pause pic
    if (paused)
    {
		V_DrawShadowedPatchOptional(136, 72, 0, W_CacheLumpName ("M_PAUSE", PU_CACHE));
    }

    // [JN] Do not draw any widgets if not in game level.
    if (gamestate == GS_LEVEL)
    {
        // [JN] Left widgets are available while active game level.
        if (dp_screen_size < 15)
        {
            ID_LeftWidgets();
        }

        // [crispy] demo timer widget
        if (demoplayback && (demo_timer == 1 || demo_timer == 3))
        {
            ID_DemoTimer(demo_timerdir ? (deftotaldemotics - defdemotics) : defdemotics);
        }
        else if (demorecording && (demo_timer == 2 || demo_timer == 3))
        {
            ID_DemoTimer(leveltime);
        }

        // [JN] Target's health widget.
        // Actual health values are gathered in G_Ticker.
        if (widget_health)
        {
            ID_DrawTargetsHealth();
        }

        // [JN] Draw crosshair.
        if (xhair_draw && !automapactive)
        {
            ID_DrawCrosshair();
        }

        // [JN] Main status bar drawing function.
        if (dp_screen_size < 15 || (automapactive && !automap_overlay))
        {
            // [JN] Only forcefully update/redraw on...
            const boolean st_forceredraw = 
                             (oldgametic < gametic  // Every game tic
                          ||  dp_screen_size > 10   // Crispy HUD (no solid status bar background)
                          ||  setsizeneeded         // Screen size changing
                          || (menuactive && dp_menu_shading)); // Menu shading while non-capped game mode

            ST_Drawer(st_forceredraw);
        }

        // [JN] Chat drawer
        if (netgame && chatmodeon)
        {
            CT_Drawer();
        }
    }

    // [JN] Draw right widgets in any states except finale text screens.
    if (gamestate != GS_FINALE)
    {
        ID_RightWidgets();
    }

    // [crispy] Demo Timer widget
    if (demoplayback && demo_bar)
    {
        ID_DemoBar();
    }

    // Handle player messages
    ID_DrawMessage();

    // [JN] Handle centered player messages.
    ID_DrawMessageCentered();

    // menus go directly to the screen
    M_Drawer ();   // menu is drawn even on top of everything
    NetUpdate ();  // send out any new accumulation

    // normal update
    if (!wipe)
    {
        I_FinishUpdate();  // page flip or blit buffer
        return;
    }

    // wipe update
    wipe_EndScreen();
    wipestart = I_GetTime () - 1;

    do
    {
        do
        {
            nowtime = I_GetTime ();
            tics = nowtime - wipestart;
            I_Sleep(1);
        } while (tics <= 0);

        wipestart = nowtime;
        done = wipe_ScreenWipe(tics);
        M_Drawer();        // menu is drawn even on top of wipes
        I_FinishUpdate();  // page flip or blit buffer
        } while (!done);
}

//
// Add configuration file variable bindings.
//

void D_BindVariables(void)
{
    int i;

    I_BindInputVariables();
    I_BindVideoVariables();
    I_BindJoystickVariables();
    I_BindSoundVariables();

    M_BindControls();
    M_BindChatControls(MAXPLAYERS);

    key_multi_msgplayer[0] = HUSTR_KEYGREEN;
    key_multi_msgplayer[1] = HUSTR_KEYINDIGO;
    key_multi_msgplayer[2] = HUSTR_KEYBROWN;
    key_multi_msgplayer[3] = HUSTR_KEYRED;

    NET_BindVariables();

    // [JN] Game-dependent variables:
    M_BindIntVariable("key_message_refresh",    &key_message_refresh);
    M_BindIntVariable("sfx_volume",             &sfxVolume);
    M_BindIntVariable("music_volume",           &musicVolume);

    // Multiplayer chat macros

    for (i=0; i<10; ++i)
    {
        char buf[12];

        M_snprintf(buf, sizeof(buf), "chatmacro%i", i);
        M_BindStringVariable(buf, &chat_macros[i]);
    }

	// [JN] Bind ID-specific config variables.
	ID_BindVariables(doom);
}

//
// D_GrabMouseCallback
//
// Called to determine whether to grab the mouse pointer
//

boolean D_GrabMouseCallback(void)
{
    // Drone players don't need mouse focus

    if (drone)
        return false;

    // [JN] CRL - always grab mouse in spectator mode.
    // It's supposed to be controlled by hand, even while pause.
    // However, do not grab mouse while active game menu.

    if (crl_spectating)
        return menuactive ? false : true;

    // when menu is active or game is paused, release the mouse 
 
    if (menuactive || paused)
        return false;

    // only grab mouse when playing levels (but not demos)

    return (gamestate == GS_LEVEL) && ((!demoplayback && !advancedemo));
}

//
//  D_DoomLoop
//
void D_DoomLoop (void)
{
    if (demorecording)
	G_BeginRecording ();

    main_loop_started = true;

    I_SetWindowTitle("Yaguar Doom");
    I_GraphicsCheckCommandLine();
    I_SetGrabMouseCallback(D_GrabMouseCallback);
    I_RegisterWindowIcon(doom_data, doom_w, doom_h);
    I_InitGraphics();
    // [JN] Calculate status bar elements background buffers.
    ST_InitElementsBackground();

    TryRunTics();

    V_RestoreBuffer();
    R_ExecuteSetViewSize();

    D_StartGameLoop();

    while (1)
    {
        // frame syncronous IO operations
        I_StartFrame ();

        // will run at least one tic
        TryRunTics ();

        // Update display, next frame, with current state.
        if (screenvisible)
        {
            D_Display();
        }

        // move positional sounds
        if (oldgametic < gametic)
        {
            // [JN] Mute and restore sound and music volume.
            if (snd_mute_inactive && volume_needs_update)
            {
                S_MuteUnmuteSound(!window_focused);
            }

            S_UpdateSounds (players[displayplayer].mo);
            oldgametic = gametic;
        }
    }
}


// =============================================================================
// DEMO LOOP
// =============================================================================


// -----------------------------------------------------------------------------
// D_PageTicker
// Handles timing for warped projection
// -----------------------------------------------------------------------------

void D_PageTicker (void)
{
    if (--pagetic < 0)
    {
        D_AdvanceDemo();
    }
}

// -----------------------------------------------------------------------------
// D_AdvanceDemo
// Called after each demo or intro demosequence finishes
// -----------------------------------------------------------------------------

void D_AdvanceDemo (void)
{
    advancedemo = true;
}

// -----------------------------------------------------------------------------
// This cycles through the demo sequences.
// FIXME - version dependend demo numbers?
// -----------------------------------------------------------------------------

void D_DoAdvanceDemo (void)
{
    players[consoleplayer].playerstate = PST_LIVE;  // not reborn
    advancedemo = false;
    usergame = false;               // no save / end game here
    paused = false;
    gameaction = ga_nothing;

    pagetic = TICRATE * 20;
    gamestate = GS_DEMOSCREEN;

    // S_StartMusic(mus_dm2ttl);
}

// -----------------------------------------------------------------------------
// D_StartTitle
// -----------------------------------------------------------------------------

void D_StartTitle (void)
{
    gameaction = ga_nothing;
    automapactive = false; // [crispy] clear overlaid automap remainings
    D_AdvanceDemo ();
}

static boolean D_AddFile(char *filename)
{
    wad_file_t *handle;

    printf("  adding %s\n", filename);
    handle = W_AddFile(filename);

    return handle != NULL;
}

static void G_CheckDemoStatusAtExit (void)
{
    G_CheckDemoStatus();
}


//
// D_DoomMain
//
void D_DoomMain (void)
{
    int p;
    char file[256];
    char demolumpname[9];
    const int starttime = SDL_GetTicks();

#ifdef _WIN32
    // [JN] Print colorized title.
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BACKGROUND_BLUE
                           | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
                           | FOREGROUND_INTENSITY);

    for (p = 0 ; p < 26 ; p++) printf(" ");
    printf(PACKAGE_FULLNAME);
    for (p = 0 ; p < 27 ; p++) printf(" ");
    printf("\n");

    // [JN] Fallback to standard console colos.
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 
                            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
    // print banner
    I_PrintBanner(PACKAGE_FULLNAME);
#endif

    printf("Z_Init: Init zone memory allocation daemon. \n");
    Z_Init ();
    
    // Call I_ShutdownGraphics on quit
   	I_AtExit(I_ShutdownGraphics, true);


    //!
    // @category net
    //
    // Start a dedicated server, routing packets but not participating
    // in the game itself.
    //

    if (M_CheckParm("-dedicated") > 0)
    {
        printf("Dedicated server mode.\n");
        NET_DedicatedServer();

        // Never returns
    }

    //!
    // @category net
    //
    // Query the Internet master server for a global list of active
    // servers.
    //

    if (M_CheckParm("-search"))
    {
        NET_MasterQuery();
        exit(0);
    }

    //!
    // @arg <address>
    // @category net
    //
    // Query the status of the server running on the given IP
    // address.
    //

    p = M_CheckParmWithArgs("-query", 1);

    if (p)
    {
        NET_QueryAddress(myargv[p+1]);
        exit(0);
    }

    //!
    // @category net
    //
    // Search the local LAN for running servers.
    //

    if (M_CheckParm("-localsearch"))
    {
        NET_LANQuery();
        exit(0);
    }

    //!
    // @vanilla
    //
    // Disable monsters.
    //
	
    nomonsters = M_CheckParm ("-nomonsters");

    //!
    // @vanilla
    //
    // Monsters respawn after being killed.
    //

    respawnparm = M_CheckParm ("-respawn");

    //!
    // @vanilla
    //
    // Monsters move faster.
    //

    fastparm = M_CheckParm ("-fast");

    //!
    // @category net
    // @vanilla
    //
    // Start a deathmatch game.
    //

    if (M_CheckParm ("-deathmatch"))
	deathmatch = 1;

    //!
    // @category net
    // @vanilla
    //
    // Start a deathmatch 2.0 game.  Weapons do not stay in place and
    // all items respawn after 30 seconds.
    //

    if (M_CheckParm ("-altdeath"))
	deathmatch = 2;

    //!
    // @category net
    // @vanilla
    //
    // Start a deathmatch 3.0 game.  Weapons stay in place and
    // all items respawn after 30 seconds.
    //

    if (M_CheckParm ("-dm3"))
	deathmatch = 3;

   
    //!
    // @category game
    //
    // Start single player game with items spawns as in cooperative netgame.
    //

    coop_spawns = M_CheckParm ("-coop_spawns");

    // Auto-detect the configuration dir.

    M_SetConfigDir(NULL);

    //!
    // @arg <x>
    // @vanilla
    //
    // Turbo mode.  The player's speed is multiplied by x%.  If unspecified,
    // x defaults to 200.  Values are rounded up to 10 and down to 400.
    //

    if ( (p=M_CheckParm ("-turbo")) )
    {
	int     scale = 200;
	
	if (p<myargc-1)
	    scale = atoi (myargv[p+1]);
	if (scale < 10)
	    scale = 10;
	if (scale > 400)
	    scale = 400;
        printf("turbo scale: %i%%\n", scale);
	forwardmove[0] = forwardmove[0]*scale/100;
	forwardmove[1] = forwardmove[1]*scale/100;
	sidemove[0] = sidemove[0]*scale/100;
	sidemove[1] = sidemove[1]*scale/100;
    }
    
    // Load configuration files before initialising other subsystems.
    printf("M_LoadDefaults: Load system defaults.\n");
    M_SetConfigFilenames(PROGRAM_PREFIX "doom.ini");
    D_BindVariables();
    M_LoadDefaults();

    // [JN] Set screeenshot files dir.
    M_SetScreenshotDir();

#ifdef _WIN32
    // [JN] Pressing PrintScreen on Windows 11 is opening Snipping Tool.
    // Re-register PrintScreen key pressing for port needs to avoid this.
    // Taken from DOOM Retro.
    if (key_menu_screenshot == KEY_PRTSCR)
    {
        RegisterHotKey(NULL, 1, MOD_ALT, VK_SNAPSHOT);
        RegisterHotKey(NULL, 2, 0, VK_SNAPSHOT);
    }
#endif

    // Save configuration at exit.
    I_AtExit(M_SaveDefaults, true); // [crispy] always save configuration at exit

    // Find main IWAD file and load it.
    iwadfile = D_FindIWAD(IWAD_MASK_DOOM, &gamemission);

    // None found?

    if (iwadfile == NULL)
    {
        i_error_safe = true;
        I_Error("Game mode indeterminate, no IWAD file was found.\n"
                "Try to do following:\n"
                "- Copy IWAD file into the folder with executable file.\n"
                "- Drag and drop IWAD file onto executable file.\n"
                "- Specifying one with the '-iwad' command line parameter.\n\n"
                "Valid IWAD files are:\n"
                "doom1.wad, doom.wad, doom2.wad, plutonia.wad, tnt.wad,\n"
                "freedoom1.wad or freedoom2.wad");
    }

    modifiedgame = false;

    printf("W_Init: Init WADfiles.\n");
    D_AddFile(iwadfile);

    W_CheckCorrectIWAD(doom);

    // Now that we've loaded the IWAD, we can figure out what gamemission
    // we're playing and which version of Vanilla Doom we need to emulate.
    //D_IdentifyVersion();
	gamemode = commercial;
    gamemission = doom2;
	//InitGameVersion();
	gameversion = exe_doom_1_9;
	
    // Load PWAD files.
    modifiedgame = W_ParseCommandLine();

    //!
    // @arg <demo>
    // @category demo
    // @vanilla
    //
    // Play back the demo named demo.lmp.
    //

    p = M_CheckParmWithArgs ("-playdemo", 1);

    if (!p)
    {
        //!
        // @arg <demo>
        // @category demo
        // @vanilla
        //
        // Play back the demo named demo.lmp, determining the framerate
        // of the screen.
        //
	p = M_CheckParmWithArgs("-timedemo", 1);

    }

    if (p)
    {
        char *uc_filename = strdup(myargv[p + 1]);
        M_ForceUppercase(uc_filename);

        // With Vanilla you have to specify the file without extension,
        // but make that optional.
        if (M_StringEndsWith(uc_filename, ".LMP"))
        {
            M_StringCopy(file, myargv[p + 1], sizeof(file));
        }
        else
        {
            snprintf(file, sizeof(file), "%s.lmp", myargv[p+1]);
        }

        free(uc_filename);

        if (D_AddFile(file))
        {
            M_StringCopy(demolumpname, lumpinfo[numlumps - 1]->name,
                         sizeof(demolumpname));
        }
        else
        {
            // If file failed to load, still continue trying to play
            // the demo in the same way as Vanilla Doom.  This makes
            // tricks like "-playdemo demo1" possible.

            M_StringCopy(demolumpname, myargv[p + 1], sizeof(demolumpname));
        }

        printf("Playing demo %s.\n", file);
    }

    I_AtExit(G_CheckDemoStatusAtExit, true);

    // Generate the WAD hash table.  Speed things up a bit.
    W_GenerateHashTable();

    savegamedir = M_GetSaveGameDir(D_SaveGameIWADName(gamemission, gamevariant));

    printf("I_Init: Setting up machine state.\n");
    I_CheckIsScreensaver();
    I_InitTimer();
    I_InitJoystick();
    I_InitSound(doom);
    I_InitMusic();

    // [crispy] check for SSG resources
    havessg = false;
	/*
    (
        gamemode == commercial ||
        (
            W_CheckNumForName("sht2a0")         != -1 && // [crispy] wielding/firing sprite sequence
            I_GetSfxLumpNum(&S_sfx[sfx_dshtgn]) != -1 && // [crispy] firing sound
            I_GetSfxLumpNum(&S_sfx[sfx_dbopn])  != -1 && // [crispy] opening sound
            I_GetSfxLumpNum(&S_sfx[sfx_dbload]) != -1 && // [crispy] reloading sound
            I_GetSfxLumpNum(&S_sfx[sfx_dbcls])  != -1    // [crispy] closing sound
        )
    );
	*/

    printf ("NET_Init: Init network subsystem.\n");
    NET_Init ();

    // Initial netgame startup. Connect to server etc.
    D_ConnectNetGame();

    // get skill / episode / map from parms
    // [JN] Use chosen default skill level.
    startskill = gp_default_skill;
    startepisode = 1;
    startmap = 1;
    autostart = false;

    //!
    // @arg <skill>
    // @vanilla
    //
    // Set the game skill, 1-5 (1: easiest, 5: hardest).  A skill of
    // 0 disables all monsters.
    //

    p = M_CheckParmWithArgs("-skill", 1);

    if (p)
    {
	startskill = myargv[p+1][0]-'1';
	autostart = true;
    }

    timelimit = 0;

    //! 
    // @arg <n>
    // @category net
    // @vanilla
    //
    // For multiplayer games: exit each level after n minutes.
    //

    p = M_CheckParmWithArgs("-timer", 1);

    if (p)
    {
	timelimit = atoi(myargv[p+1]);
    }

    //!
    // @category net
    // @vanilla
    //
    // Austin Virtual Gaming: end levels after 20 minutes.
    //

    p = M_CheckParm ("-avg");

    if (p)
    {
	timelimit = 20;
    }

    //!
    // @arg [<x> <y> | <xy>]
    // @vanilla
    //
    // Start a game immediately, warping to MAPxy
    //

    p = M_CheckParmWithArgs("-warp", 1);

    if (p)
    {
        startmap = atoi (myargv[p+1]);
        autostart = true;
        // [crispy] if used with -playdemo, fast-forward demo up to the desired map
        demowarp = startmap;
    }

    // Check for load game parameter
    // We do this here and save the slot number, so that the network code
    // can override it or send the load slot to other players.

    //!
    // @arg <s>
    // @vanilla
    //
    // Load the game in slot s.
    //

    p = M_CheckParmWithArgs("-loadgame", 1);
    
    if (p)
    {
        startloadgame = atoi(myargv[p+1]);
    }
    else
    {
        // Not loading a game
        startloadgame = -1;
    }

    printf("M_Init: Init miscellaneous info.\n");
    M_Init ();

    printf("R_Init: Init DOOM refresh daemon - [");
    R_Init ();

    printf("\nP_Init: Init Playloop state.\n");
    P_Init ();

    printf("S_Init: Setting up sound.\n");
    S_Init (sfxVolume * 8, musicVolume * 8);

    printf("D_CheckNetGame: Checking network game status.\n");
    D_CheckNetGame ();

    printf("CT_Init: Setting up messages system.\n");
    CT_Init ();

    printf("ST_Init: Init status bar.\n");
    ST_Init ();

    printf("WI_Init: Init intermission screen data.\n");
    WI_Init ();

    // [JN] Predefine some automap variables at program startup.
    AM_Init ();

    // [JN] Show startup process time.
    printf("Startup process took %d ms.\n", SDL_GetTicks() - starttime);

    //!
    // @arg <x>
    // @category demo
    // @vanilla
    //
    // Record a demo named x.lmp.
    //

    p = M_CheckParmWithArgs("-record", 1);

    if (p)
    {
	G_RecordDemo (myargv[p+1]);
	autostart = true;
    }

    p = M_CheckParmWithArgs("-playdemo", 1);
    if (p)
    {
	singledemo = true;              // quit after one demo
	G_DeferedPlayDemo (demolumpname);
	D_DoomLoop ();  // never returns
    }
    demowarp = 0; // [crispy] we don't play a demo, so don't skip maps
	
    p = M_CheckParmWithArgs("-timedemo", 1);
    if (p)
    {
	G_TimeDemo (demolumpname);
	D_DoomLoop ();  // never returns
    }
	
    if (startloadgame >= 0)
    {
        M_StringCopy(file, P_SaveGameFile(startloadgame), sizeof(file));
	G_LoadGame(file);
    }
	
    if (gameaction != ga_loadgame )
    {
		if (autostart || netgame)
			G_InitNew (startskill, startepisode, startmap);
		else
		{
			D_StartTitle ();                // start up intro loop
		}
    }

    D_DoomLoop ();  // never returns
}
