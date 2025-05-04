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
#include "d_englsh.h"
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
#include "d_main.h"
#include "ct_chat.h"
#include "r_local.h"
#include "v_postproc.h"
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

    if (vid_uncapped_fps)
    {
        I_StartDisplay();
        G_FastResponder();
        G_PrepTiccmd();
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
            if (!gametic)
            break;

            // draw the view directly
            R_RenderPlayerView(&players[displayplayer]);

            // [JN] Fail-safe: return earlier if post rendering hook is still active.
            if (post_rendering_hook)
            return;

            // see if the border needs to be initially drawn
            if (oldgamestate != GS_LEVEL)
            R_FillBackScreen();  // draw the pattern into the back screen

            // see if the border needs to be updated to the screen
            if (scaledviewwidth != SCREENWIDTH)
            R_DrawViewBorder();  // erase old menu stuff

            // [JN] Draw automap on top of player view and view border,
            // and update while playing. This also needed for widgets update.
            if (automapactive)
            AM_Drawer();

            // [JN] Allow to draw level name separately from automap.
            if (automapactive || (widget_levelname && widget_enable && dp_screen_size < 15))
            AM_LevelNameDrawer();

            // [JN] Do not draw any widgets if not in game level.
            if (widget_enable)
            {
                // [JN] Left widgets are available while active game level.
                if (dp_screen_size < 15)
                ID_LeftWidgets();

                // [JN] Target's health widget.
                // Actual health values are gathered in G_Ticker.
                if (widget_health)
                ID_DrawTargetsHealth();
            }

            // [JN] Draw crosshair.
            if (xhair_draw && !automapactive)
            ID_DrawCrosshair();

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

    // clean up border stuff
    if (gamestate != oldgamestate && gamestate != GS_LEVEL)
	I_SetPalette (0);

    oldgamestate = wipegamestate = gamestate;

    // draw pause pic
    if (paused)
    {
		V_DrawShadowedPatchOptional(136, 72, W_CacheLumpName ("PAUSED", PU_CACHE));
    }

    // [JN] Draw right widgets in any states except finale text screens.
    if (widget_enable)
    {
        if (gamestate != GS_FINALE)
        {
            ID_RightWidgets();
        }
    }

    // Handle player messages
    ID_DrawMessage();

    // [JN] Handle centered player messages.
    ID_DrawMessageCentered();

    // menus go directly to the screen
    M_Drawer ();   // menu is drawn even on top of everything

    // [JN] Apply post-processing effects and forcefully
    // update status bar if any effect is active.
    // Apply V_PProc_OverbrightGlow only on game level states,
    // and not while active non-overlayed automap.
    V_PProc_Display((gamestate != GS_LEVEL) || (automapactive && !automap_overlay));
    if (V_PProc_EffectsActive())
        st_fullupdate = true;

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
    I_BindInputVariables();
    I_BindVideoVariables();
    I_BindJoystickVariables();
    I_BindSoundVariables();

    M_BindControls();

    // [JN] Game-dependent variables:
    M_BindIntVariable("key_message_refresh",    &key_message_refresh);
    M_BindIntVariable("sfx_volume",             &sfxVolume);
    M_BindIntVariable("music_volume",           &musicVolume);
    
    M_BindStringVariable("savegames_path",      &SavePathConfig);
    M_BindStringVariable("screenshots_path",    &ShotPathConfig);

	// [JN] Bind ID-specific config variables.
	ID_BindVariables();
}

//
// D_GrabMouseCallback
//
// Called to determine whether to grab the mouse pointer
//

boolean D_GrabMouseCallback(void)
{
    // [JN] CRL - always grab mouse in spectator mode.
    // It's supposed to be controlled by hand, even while pause.
    // However, do not grab mouse while active game menu.

    if (crl_spectating)
        return menuactive ? false : true;

    // when menu is active or game is paused, release the mouse 
 
    if (menuactive || paused)
        return false;

    // [JN] Always grab the mouse on non-level game states.
    if (gamestate == GS_INTERMISSION || gamestate == GS_FINALE)
        return true;

    // only grab mouse when playing levels (but not demos)

    return (gamestate == GS_LEVEL);
}

//
//  D_DoomLoop
//
void D_DoomLoop (void)
{
    main_loop_started = true;

    I_SetWindowTitle("CRY");
    I_GraphicsCheckCommandLine();
    I_SetGrabMouseCallback(D_GrabMouseCallback);
    I_RegisterWindowIcon(doom_data, doom_w, doom_h);
    I_InitGraphics();

    TryRunTics();

    V_RestoreBuffer();
    R_ExecuteSetViewSize();

    D_StartGameLoop();

    while (1)
    {
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


//
// D_DoomMain
//
void D_DoomMain (void)
{
    int p;
    char file[256];
    const int starttime = SDL_GetTicks();

#ifdef _WIN32
    // [JN] Print colorized title.
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                            BACKGROUND_GREEN | BACKGROUND_BLUE);
    for (p = 0 ; p < 20 ; ++p) printf(" ");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                            BACKGROUND_RED);
    for (p = 0 ; p < 6 ; ++p) printf(" ");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                            BACKGROUND_RED |
                            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE |
                            FOREGROUND_INTENSITY);
    printf(PACKAGE_FULLNAME);
    for (p = 0 ; p < 7 ; ++p) printf(" ");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                            BACKGROUND_RED | BACKGROUND_GREEN);
    for (p = 0 ; p < 20 ; ++p) printf(" ");
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

    // Auto-detect the configuration dir.

    M_SetConfigDir(NULL);
    
    // Load configuration files before initialising other subsystems.
    printf("M_LoadDefaults: Load system defaults.\n");
    M_SetConfigFilenames(PROGRAM_PREFIX ".ini");
    D_BindVariables();
    M_LoadDefaults();

    // [JN] Set screeenshot files dir.
    M_SetScreenshotDir();

    // Save configuration at exit.
    I_AtExit(M_SaveDefaults, true); // [crispy] always save configuration at exit

    // Find main IWAD file and load it.
    iwadfile = D_FindWADByName("cry.wad");

    // None found?

    if (iwadfile == NULL)
    {
        i_error_safe = false;
        I_Error("IWAD file cry.wad not found.\n"
                "Make sure it's in the same folder as the CRY executable.");
    }

    modifiedgame = false;

    printf("W_Init: Init WADfiles.\n");
    D_AddFile(iwadfile);
	
    //!
    // @category mod
    //
    // Disable auto-loading of .wad and .deh files.
    //
    if (!M_ParmExists("-noautoload"))
    {
        char *autoload_dir;

        // common auto-loaded files for all Doom flavors

        // [JN] Since there are no IWADs, use straight "autoload" folder.
        {
            autoload_dir = M_GetAutoloadDir("");
            if (autoload_dir != NULL)
            {
                W_AutoLoadWADs(autoload_dir);
                free(autoload_dir);
            }
        }
    }
    // Load PWAD files.
    modifiedgame = W_ParseCommandLine();

    // Generate the WAD hash table.  Speed things up a bit.
    W_GenerateHashTable();

    // [JN] Set the default directory where savegames are saved.
    savegamedir = M_GetSaveGameDir("cry.wad");

    // [JN] Set the default directory where screenshots are saved.
    M_SetScreenshotDir();

    printf("I_Init: Setting up machine state.\n");
    I_CheckIsScreensaver();
    I_InitTimer();
    I_InitJoystick();
    I_InitSound(doom);
    I_InitMusic();

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

    //printf("D_CheckNetGame: Checking network game status.\n");
    D_CheckNetGame ();

    printf("CT_Init: Setting up messages system.\n");
    CT_Init ();

    printf("ST_Init: Init status bar.\n");
    ST_Init ();
    // [JN] Calculate status bar elements background buffers.
    ST_InitElementsBackground();

    printf("WI_Init: Init intermission screen data.\n");
    WI_Init ();

    // [JN] Predefine some automap variables at program startup.
    AM_Init ();

    // [JN] Show startup process time.
    printf("Startup process took %d ms.\n", SDL_GetTicks() - starttime);

    if (startloadgame >= 0)
    {
        M_StringCopy(file, P_SaveGameFile(startloadgame), sizeof(file));
	G_LoadGame(file);
    }
	
    if (gameaction != ga_loadgame )
    {
		if (autostart)
			G_InitNew (startskill, startepisode, startmap);
		else
		{
			gamestate = GS_DEMOSCREEN;
			S_ChangeMusic(mus_intro, false);
			D_StartTitle ();                // start up intro loop
		}
    }

    D_DoomLoop ();  // never returns
}
