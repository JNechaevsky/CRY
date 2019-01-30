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


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "deh_main.h"
#include "doomdef.h"
#include "doomstat.h"
#include "doomfeatures.h"
#include "sounds.h"
#include "d_iwad.h"
#include "z_zone.h"
#include "w_main.h"
#include "w_merge.h"
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
#include "p_saveg.h"
#include "i_input.h"
#include "i_joystick.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "wi_stuff.h"
#include "st_stuff.h"
#include "am_map.h"
#include "p_setup.h"
#include "r_local.h"

#include "d_main.h"
#include "d_englsh.h"
#include "jn.h"


// =============================================================================
// D-DoomLoop
//
// Not a globally visible function,
//  just included for source reference,
//  called by D_DoomMain, never exits.
// Manages timing and IO,
//  calls all ?_Responder, ?_Ticker, and ?_Drawer,
//  calls I_GetTime, I_StartFrame, and I_StartTic
// =============================================================================

void D_DoomLoop (void);
void D_CheckNetGame(void);
void R_ExecuteSetViewSize (void);


char wadfile[1024];     // primary wad file
char mapdir[1024];      // directory of development maps
char *savegamedir;      // Location where savegames are stored
char *iwadfile;         // location of IWAD and WAD files

boolean devparm;        // started game with -devparm
boolean fastparm;       // checkparm of -fast
boolean autostart;
boolean advancedemo;
boolean main_loop_started   = false;  // If true, the main game loop has started
boolean flip_levels_cmdline = false;

int startepisode;
int startmap;
int startloadgame;
int translucency = 1;      // [Julia] On by default

skill_t startskill;

extern boolean setsizeneeded;
extern int     showMessages;


// =============================================================================
// D_ProcessEvents
//
// Send all the events of the given timestamp down the responder chain
// =============================================================================

void D_ProcessEvents (void)
{
    event_t *ev;

    while ((ev = D_PopEvent()) != NULL)
    {
        if (M_Responder (ev))
        continue;   // menu ate the event

        G_Responder (ev);
    }
}


// =============================================================================
// D_Display
//
// Draw current display, possibly wiping it from the previous
// =============================================================================

// wipegamestate can be set to -1 to force a wipe on the next draw
gamestate_t wipegamestate = GS_DEMOSCREEN;

void D_Display (void)
{
    static boolean      viewactivestate = false;
    static boolean      menuactivestate = false;
    static boolean      inhelpscreensstate = false;
    static boolean      fullscreen = false;
    static gamestate_t  oldgamestate = -1;
    static int          borderdrawcount;
    int                 nowtime;
    int                 tics;
    int                 wipestart;
    boolean             done;
    boolean             wipe;
    boolean             redrawsbar;

    if (nodrawers)
    return; // for comparative timing / profiling

    redrawsbar = false;

    // change the view size if needed
    if (setsizeneeded)
    {
        R_ExecuteSetViewSize ();
        oldgamestate    = -1; // force background redraw
        borderdrawcount = 3;
    }

    // save the current screen if about to wipe
    // [Julia] No wiping while -devparm
    if (gamestate != wipegamestate && !devparm)
    {
        wipe = true;
        wipe_StartScreen(0, 0, SCREENWIDTH, SCREENHEIGHT);
    }
    else
    {
        wipe = false;
    }

    if (gamestate == GS_LEVEL && gametic)
    HU_Erase();

    // do buffered drawing
    switch (gamestate)
    {
        case GS_LEVEL:
        if (!gametic)
        break;

        if (automapactive)
        {
            // [crispy] update automap while playing
            R_RenderPlayerView (&players[displayplayer]);
            AM_Drawer ();
        }

        if (wipe || (scaledviewheight != SCREENHEIGHT && fullscreen))
        redrawsbar = true;

        if (inhelpscreensstate && !inhelpscreens)
        redrawsbar = true; // just put away the help screen

        ST_Drawer (scaledviewheight == SCREENHEIGHT, redrawsbar );
        fullscreen = scaledviewheight == SCREENHEIGHT;
        break;

        case GS_INTERMISSION:
        WI_Drawer ();
        break;

        case GS_FINALE:
        F_Drawer ();
        break;

        case GS_DEMOSCREEN:
        D_PageDrawer ();
        break;
    }

    // draw the view directly
    if (gamestate == GS_LEVEL && !automapactive && gametic)
    {
        R_RenderPlayerView (&players[displayplayer]);

        if (screenblocks == 11 || screenblocks == 12 || screenblocks == 13)
        ST_Drawer(0, 0);
    }

    if (gamestate == GS_LEVEL && gametic)
    HU_Drawer ();

    // clean up border stuff
    if (gamestate != oldgamestate && gamestate != GS_LEVEL)
    {
        I_SetPalette (W_CacheLumpName(DEH_String("PLAYPAL"), PU_CACHE));
    }

    // see if the border needs to be initially drawn
    if (gamestate == GS_LEVEL && oldgamestate != GS_LEVEL)
    {
        viewactivestate = false;    // view was not active
        R_FillBackScreen ();        // draw the pattern into the back screen
    }

    // see if the border needs to be updated to the screen
    if (gamestate == GS_LEVEL && !automapactive && scaledviewwidth != (320 << hires))
    {
        if (menuactive || menuactivestate || !viewactivestate)
        borderdrawcount = 3;

        if (borderdrawcount)
        {
            R_DrawViewBorder (); // erase old menu stuff
            borderdrawcount--;
        }
    }

    if (testcontrols)
    {
        // Box showing current mouse speed
        V_DrawMouseSpeedBox(testcontrols_mousespeed);
    }

    menuactivestate = menuactive;
    viewactivestate = viewactive;
    inhelpscreensstate = inhelpscreens;
    oldgamestate = wipegamestate = gamestate;

    // draw pause pic
    if (paused)
    {
        // [Julia] draw PAUSE pic independently, offsets done in the sprite
        V_DrawShadowedPatch(0, 0, W_CacheLumpName (DEH_String("M_PAUSE"), PU_CACHE));
    }

    // menus go directly to the screen
    M_Drawer ();    // menu is drawn even on top of everything
    NetUpdate ();   // send out any new accumulation

    // normal update
    if (!wipe)
    {
        I_FinishUpdate ();  // page flip or blit buffer
        return;
    }

    // wipe update
    wipe_EndScreen(0, 0, SCREENWIDTH, SCREENHEIGHT);
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
    done = wipe_ScreenWipe(wipe_Melt, 0, 0, SCREENWIDTH, SCREENHEIGHT, tics);
    M_Drawer ();        // menu is drawn even on top of wipes
    I_FinishUpdate ();  // page flip or blit buffer
    } while (!done);
}


// -----------------------------------------------------------------------------
// D_BindVariables
//
// Add configuration file variable bindings.
// -----------------------------------------------------------------------------

void D_BindVariables(void)
{
    M_ApplyPlatformDefaults();

    I_BindInputVariables();
    I_BindVideoVariables();
    I_BindJoystickVariables();
    I_BindSoundVariables();

    M_BindBaseControls();
    M_BindWeaponControls();
    M_BindMapControls();
    M_BindMenuControls();

    M_BindIntVariable("uncapped_fps",           &uncapped_fps);
    M_BindIntVariable("mouse_sensitivity",      &mouseSensitivity);
    M_BindIntVariable("sfx_volume",             &sfxVolume);
    M_BindIntVariable("music_volume",           &musicVolume);
    M_BindIntVariable("snd_monomode",           &snd_monomode);
    M_BindIntVariable("show_messages",          &showMessages);
    M_BindIntVariable("screenblocks",           &screenblocks);
    M_BindIntVariable("detaillevel",            &detailLevel);
    M_BindIntVariable("snd_channels",           &snd_channels);
    M_BindIntVariable("mlook",                  &mlook);

    // [Julia] Gameplay Features
    
    // Common
    M_BindIntVariable("hightlight_things",      &hightlight_things);
    M_BindIntVariable("brightmaps",             &brightmaps);
    M_BindIntVariable("translucency",           &translucency);
    M_BindIntVariable("swirling_liquids",       &swirling_liquids);
    M_BindIntVariable("colored_blood",          &colored_blood);
    M_BindIntVariable("weapon_bobbing",         &weapon_bobbing);
  
    // Crosshair
    M_BindIntVariable("crosshair_draw",         &crosshair_draw);
}


// -----------------------------------------------------------------------------
// D_GrabMouseCallback
//
// Called to determine whether to grab the mouse pointer
// -----------------------------------------------------------------------------

boolean D_GrabMouseCallback(void)
{
    // when menu is active or game is paused, release the mouse 
    if (menuactive || paused)
    return false;

    // only grab mouse when playing levels (but not demos)
    return (gamestate == GS_LEVEL) && !demoplayback && !advancedemo;
}


// -----------------------------------------------------------------------------
// D_DoomLoop
// -----------------------------------------------------------------------------

void D_DoomLoop (void)
{
    main_loop_started = true;

    I_SetWindowTitle(gamedescription);
    I_GraphicsCheckCommandLine();
    I_SetGrabMouseCallback(D_GrabMouseCallback);
    I_InitGraphics();

    TryRunTics();

    V_RestoreBuffer();
    R_ExecuteSetViewSize();

    D_StartGameLoop();

    if (testcontrols)
    {
        wipegamestate = gamestate;
    }

    while (1)
    {
        // will run at least one tic
        TryRunTics ();

        // move positional sounds
        S_UpdateSounds (players[consoleplayer].mo);

        // Update display, next frame, with current state.
        if (screenvisible)
        D_Display ();
    }
}


// =============================================================================
//  DEMO LOOP
// =============================================================================

int    demosequence;
int    pagetic;
char  *pagename1;   // [Julia] Background
char  *pagename2;   // [Julia] Foreground


// -----------------------------------------------------------------------------
// D_PageTicker
//
// Handles timing for warped projection
// -----------------------------------------------------------------------------

void D_PageTicker (void)
{
    if (--pagetic < 0)
    D_AdvanceDemo ();
}


// -----------------------------------------------------------------------------
// D_PageDrawer
// -----------------------------------------------------------------------------

void D_PageDrawer (void)
{
    V_DrawPatch (0, 0, W_CacheLumpName(pagename1, PU_CACHE));
    V_DrawShadowedPatch (0, 0, W_CacheLumpName(pagename2, PU_CACHE));
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
// D_DoAdvanceDemo
//
// [Julia] Cycles through the screens (no internal demos):
//  1) Title 
//  2) id Software credits
//  3) Yaguar Doom credits
// -----------------------------------------------------------------------------

void D_DoAdvanceDemo (void)
{
    players[consoleplayer].playerstate = PST_LIVE;  // not reborn
    advancedemo = false;
    usergame = false;                               // no save / end game here
    paused = false;
    gameaction = ga_nothing;
    gamestate = GS_DEMOSCREEN;
    pagetic = TICRATE * 8;
    demosequence = (demosequence+1)%3;

    switch (demosequence)
    {
        case 0:
        {
            pagename1 = DEH_String("M_TITLE");
            pagename2 = DEH_String("TITLE");
            S_StartMusic (mus_title);
            break;
        }

        case 1:
        {
            pagename1 = DEH_String("M_TITLE");
            pagename2 = DEH_String("CREDIT1");
            break;
        }

        case 2:
        {
            pagename1 = DEH_String("M_TITLE");
            pagename2 = DEH_String("CREDIT2");
            break;
        }
    }
}


// -----------------------------------------------------------------------------
// D_StartTitle
// -----------------------------------------------------------------------------

void D_StartTitle (void)
{
    gameaction = ga_nothing;
    demosequence = -1;
    D_AdvanceDemo ();
}


static boolean D_AddFile(char *filename)
{
    wad_file_t *handle;

    printf(" adding: %s\n", filename);
    handle = W_AddFile(filename);

    return handle != NULL;
}



//
// D_DoomMain
//

void D_DoomMain (void)
{
    int     p;
    char    file[256];

    // [Julia] Print startup banner only once
    I_PrintDivider();
    I_PrintBanner(PACKAGE_STRING);
    I_PrintDivider();

    DEH_printf("Z_Init: Init zone memory allocation daemon. \n");
    Z_Init ();

    //! 
    // @vanilla
    //
    // Developer mode.  F1 saves a screenshot in the current working
    // directory.
    //

    devparm = M_CheckParm ("-devparm");
    I_DisplayFPSDots(devparm);

    if (devparm)
        DEH_printf(D_DEVSTR);
    
    // find which dir to use for config files

#ifdef _WIN32

    //!
    // @platform windows
    // @vanilla
    //
    // Save configuration data and savegames in c:\doomdata,
    // allowing play from CD.
    //

    if (M_ParmExists("-cdrom"))
    {
        printf(D_CDROM);
        M_SetConfigDir("c:\\doomdata\\");
    }
    else
#endif
    {
        // Auto-detect the configuration dir.
        M_SetConfigDir(NULL);
    }

    // init subsystems
    DEH_printf("V_Init: allocate screens.\n");
    V_Init ();

    // Load configuration files before initialising other subsystems.
    DEH_printf("M_LoadDefaults: Load system defaults.\n");
    M_SetConfigFilenames(PROGRAM_PREFIX "doom.ini");
    D_BindVariables();
    M_LoadDefaults();

    // Save configuration at exit.
    I_AtExit(M_SaveDefaults, false);

    // Find main IWAD file and load it.
    iwadfile = D_FindIWAD(IWAD_MASK_DOOM, &gamemission);

    // None found?

    if (iwadfile == NULL)
    {
        I_Error("Game mode indeterminate.  No IWAD file was found.  Try\n"
                "specifying one with the '-iwad' command line parameter.\n");
    }

    modifiedgame = false;

    DEH_printf("W_Init: Init WADfiles.\n");
    D_AddFile(iwadfile);

    W_CheckCorrectIWAD(doom);

    // [Julia] Set game version, mission and mode.
    gameversion = exe_doom_1_9;
    gamemission = jaguar;
    gamemode = commercial;

    // Generate the WAD hash table.  Speed things up a bit.
    W_GenerateHashTable();

    // [Julia] Set window title
    gamedescription = "Yaguar Doom";

#ifdef _WIN32
    // In -cdrom mode, we write savegames to c:\doomdata as well as configs.
    if (M_ParmExists("-cdrom"))
    {
        savegamedir = configdir;
    }
    else
#endif
    {
        savegamedir = M_GetSaveGameDir(D_SaveGameIWADName(gamemission));
    }

    DEH_printf("I_Init: Setting up machine state.\n");
    I_CheckIsScreensaver();
    I_InitTimer();
    I_InitJoystick();
    I_InitSound(true);
    I_InitMusic();

    // get skill / episode / map from parms
    startskill = sk_medium;
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
    // Start a game immediately, warping to ExMy (Doom 1) or MAPxy
    // (Doom 2)
    //

    p = M_CheckParmWithArgs("-warp", 1);

    if (p)
    {
        startmap = atoi (myargv[p+1]);
        gameaction = ga_newgame;
        autostart = true;
    }

    // Undocumented:
    // Invoked by setup to test the controls.

    p = M_CheckParm("-testcontrols");

    if (p > 0)
    {
        startepisode = 1;
        startmap = 1;
        autostart = true;
        testcontrols = true;
    }

    // [crispy] port level flipping feature over from Strawberry Doom

    p = M_CheckParm("-fliplevels");

    if (p > 0)
    {
        flip_levels_cmdline = !flip_levels_cmdline;
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

    DEH_printf("M_Init: Init miscellaneous info.\n");
    M_Init ();

    DEH_printf("R_Init: Init DOOM refresh daemon - ");
    R_Init ();

    DEH_printf("\nP_Init: Init Playloop state.\n");
    P_Init ();

    DEH_printf("S_Init: Setting up sound.\n");
    S_Init (sfxVolume * 8, musicVolume * 8);

    // [Julia] There is no network game, altrough D_CheckNetGame is needed.
    // DEH_printf("D_CheckNetGame: Checking network game status.\n");
    D_CheckNetGame ();

    DEH_printf("HU_Init: Setting up heads up display.\n");
    HU_Init ();

    DEH_printf("ST_Init: Init status bar.\n");
    ST_Init ();

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
            D_StartTitle ();    // start up intro loop
    }

    D_DoomLoop ();  // never returns
}

