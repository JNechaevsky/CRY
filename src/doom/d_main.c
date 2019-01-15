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
// [Julia] TODO - remove unnecessary stuff

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
#include "net_client.h"
#include "net_dedicated.h"
#include "net_query.h"
#include "p_setup.h"
#include "r_local.h"

#include "d_main.h"
#include "d_englsh.h"
#include "jn.h"

// [JN] Сделана глобальной, нужна для функции автоподргузки 
// блоков DEHACKED, а также в цикле D_DoomMain.
int numiwadlumps; 


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
void D_ConnectNetGame(void);
void D_CheckNetGame(void);
void R_ExecuteSetViewSize (void);


char wadfile[1024];     // primary wad file
char mapdir[1024];      // directory of development maps
char *savegamedir;      // Location where savegames are stored
char *iwadfile;         // location of IWAD and WAD files

boolean devparm;        // started game with -devparm
boolean nomonsters;     // checkparm of -nomonsters
boolean respawnparm;    // checkparm of -respawn
boolean fastparm;       // checkparm of -fast
boolean autostart;
boolean advancedemo;
boolean main_loop_started   = false;  // If true, the main game loop has started
boolean flip_levels_cmdline = false;

int startepisode;
int startmap;
int startloadgame;
int english_language = 0;   // [Julia] TODO - REMOVE
int translucency  = 1;      // [Julia] Прозрачность объектов

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

    // [JN] Support for fallback to the English language.
    M_BindIntVariable("english_language",       &english_language);

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

    // [Julia] Optional Gameplay Enhancements
    
    // Common
    M_BindIntVariable("brightmaps",             &brightmaps);
    M_BindIntVariable("translucency",           &translucency);
    M_BindIntVariable("swirling_liquids",       &swirling_liquids);
    M_BindIntVariable("colored_blood",          &colored_blood);
    M_BindIntVariable("weapon_bobbing",         &weapon_bobbing);
  
    // Crosshair
    M_BindIntVariable("crosshair_draw",         &crosshair_draw);
    M_BindIntVariable("crosshair_health",       &crosshair_health);
    M_BindIntVariable("crosshair_scale",        &crosshair_scale);
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
    if (demorecording)
    G_BeginRecording ();

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
// This cycles through the demo sequences.
// [Julia] TODO - recreate Jaguar demos?
// -----------------------------------------------------------------------------

void D_DoAdvanceDemo (void)
{
    players[consoleplayer].playerstate = PST_LIVE;  // not reborn
    advancedemo = false;
    usergame = false;                               // no save / end game here
    paused = false;
    gameaction = ga_nothing;
    demosequence = (demosequence+1)%6;

    switch (demosequence)
    {
        case 0:
        {
            pagetic = TICRATE * 11;
            gamestate = GS_DEMOSCREEN;
            pagename1 = DEH_String("M_TITLE");
            pagename2 = DEH_String("TITLE");
            S_StartMusic (mus_title);
            break;
        }

        case 1:
        {
            // G_DeferedPlayDemo(DEH_String("demo1"));
            break;
        }

        case 2:
        {
            pagetic = TICRATE * 11;
            gamestate = GS_DEMOSCREEN;
            pagename1 = DEH_String("M_TITLE");
            pagename2 = DEH_String("CREDIT");
            break;
        }

        case 3:
        {
            // G_DeferedPlayDemo(DEH_String("demo2"));
            break;
        }

        case 4:
        {
            gamestate = GS_DEMOSCREEN;
            pagetic = TICRATE * 11;
            pagename1 = DEH_String("M_TITLE");
            pagename2 = DEH_String("CREDIT2");
            S_StartMusic(mus_title);
            break;
        }

        case 5:
        {
            // G_DeferedPlayDemo(DEH_String("demo3"));
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

// Strings for dehacked replacements of the startup banner
//
// These are from the original source: some of them are perhaps
// not used in any dehacked patches

static char *banners[] =
{
    // doom2.wad
    "                         "
    "DOOM 2: Hell on Earth v%i.%i"
    "                           ",
    // doom2.wad v1.666
    "                         "
    "DOOM 2: Hell on Earth v%i.%i66"
    "                          ",
    // doom1.wad
    "                            "
    "DOOM Shareware Startup v%i.%i"
    "                           ",
    // doom.wad
    "                            "
    "DOOM Registered Startup v%i.%i"
    "                           ",
    // Registered DOOM uses this
    "                          "
    "DOOM System Startup v%i.%i"
    "                          ",
    // Doom v1.666
    "                          "
    "DOOM System Startup v%i.%i66"
    "                          "
    // doom.wad (Ultimate DOOM)
    "                         "
    "The Ultimate DOOM Startup v%i.%i"
    "                        ",
    // tnt.wad
    "                     "
    "DOOM 2: TNT - Evilution v%i.%i"
    "                           ",
    // plutonia.wad
    "                   "
    "DOOM 2: Plutonia Experiment v%i.%i"
    "                           ",
};


// -----------------------------------------------------------------------------
// Get game name: if the startup banner has been replaced, use that.
// Otherwise, use the name given
// -----------------------------------------------------------------------------

static char *GetGameName(char *gamename)
{
    size_t  i;
    char    *deh_sub;

    for (i=0; i<arrlen(banners); ++i)
    {
        // Has the banner been replaced?
        deh_sub = DEH_String(banners[i]);

        if (deh_sub != banners[i])
        {
            size_t gamename_size;
            int version;

            // Has been replaced.
            // We need to expand via printf to include the Doom version number
            // We also need to cut off spaces to get the basic name

            gamename_size = strlen(deh_sub) + 10;
            gamename = Z_Malloc(gamename_size, PU_STATIC, 0);
            version = G_VanillaVersionCode();
            M_snprintf(gamename, gamename_size, deh_sub, version / 100, version % 100);

            while (gamename[0] != '\0' && isspace(gamename[0]))
            {
                memmove(gamename, gamename + 1, gamename_size - 1);
            }

            while (gamename[0] != '\0' && isspace(gamename[strlen(gamename)-1]))
            {
                gamename[strlen(gamename) - 1] = '\0';
            }

            return gamename;
        }
    }

    return gamename;
}


static void SetMissionForPackName(char *pack_name)
{
    int i;
    static const struct
    {
        char *name;
        int mission;
    } packs[] = {
        { "doom2",    doom2 },
        { "tnt",      pack_tnt },
        { "plutonia", pack_plut },
    };

    for (i = 0; i < arrlen(packs); ++i)
    {
        if (!strcasecmp(pack_name, packs[i].name))
        {
            gamemission = packs[i].mission;
            return;
        }
    }

    printf("Valid mission packs are:\n");

    for (i = 0; i < arrlen(packs); ++i)
    {
        printf("\t%s\n", packs[i].name);
    }

    I_Error("Unknown mission pack name: %s", pack_name);
}


//
// Find out what version of Doom is playing.
//

void D_IdentifyVersion(void)
{
    // gamemission is set up by the D_FindIWAD function.  But if 
    // we specify '-iwad', we have to identify using 
    // IdentifyIWADByName.  However, if the iwad does not match
    // any known IWAD name, we may have a dilemma.  Try to 
    // identify by its contents.

    if (gamemission == none)
    {
        unsigned int i;

        for (i=0; i<numlumps; ++i)
        {
            if (!strncasecmp(lumpinfo[i]->name, "MAP01", 8))
            {
                gamemission = doom2;
                break;
            } 
            else if (!strncasecmp(lumpinfo[i]->name, "E1M1", 8))
            {
                gamemission = doom;
                break;
            }
        }

        if (gamemission == none)
        {
            // Still no idea.  I don't think this is going to work.
			// Unknown or invalid IWAD file.
            I_Error("Unknown or invalid IWAD file.");
        }
    }

    // Make sure gamemode is set up correctly

    if (logical_gamemission == doom)
    {
        // Doom 1.  But which version?

        if (W_CheckNumForName("E4M1") > 0)
            gamemode = retail;      // The Ultimate Doom

        else if (W_CheckNumForName("E3M1") > 0)
            gamemode = registered;  // Doom Registered

        else
            gamemode = shareware;   // Doom Shareware
    }
    else
    {
        int p;

        // Doom 2 of some kind.
        gamemode = commercial;

        // We can manually override the gamemission that we got from the
        // IWAD detection code. This allows us to eg. play Plutonia 2
        // with Freedoom and get the right level names.

        //!
        // @category compat
        // @arg <pack>
        //
        // Explicitly specify a Doom II "mission pack" to run as, instead of
        // detecting it based on the filename. Valid values are: "doom2",
        // "tnt" and "plutonia".
        //
        p = M_CheckParmWithArgs("-pack", 1);
        if (p > 0)
        {
            SetMissionForPackName(myargv[p + 1]);
        }
    }
}


// -----------------------------------------------------------------------------
// Set the gamedescription string
// [JN] На этом этапе указываем заголовок окна игры и подгрузку 
// необходимых файлов с локализованными ресурсами.

void D_SetGameDescription(void)
{
    gamedescription = GetGameName("DOOM for Atari Jaguar");
}


//      print title for every printed line
char    title[128];

static boolean D_AddFile(char *filename)
{
    wad_file_t *handle;

    printf(" adding: %s\n", filename);
    handle = W_AddFile(filename);

    return handle != NULL;
}

// Copyright message banners
// Some dehacked mods replace these.  These are only displayed if they are 
// replaced by dehacked.

static char *copyright_banners[] =
{
    "===========================================================================\n"
    "ВНИМАНИЕ:  Данная копия игры DOOM подверглась изменениям.  Для приобретения\n"
    "оригинальной игры позвоните 1-800-IDGAMES или ознакомьтесь с файлом ReadMe.\n"
    "          Мы не оказываем поддержки по модифицированным продуктам.\n"
    "                       Для продолжения нажите Enter.\n"
    "===========================================================================\n",

    "===========================================================================\n"
    "              Коммерческий продукт - распространение запрещено!\n"
    "  Просьба сообщать о фактах программного пиратства в SPA: 1-800-388-PIR8\n"
    "===========================================================================\n",

    "===========================================================================\n"
    "                          Демонстрационная версия!\n"
    "===========================================================================\n"
};

// Prints a message only if it has been modified by dehacked.

void PrintDehackedBanners(void)
{
    size_t i;

    for (i=0; i<arrlen(copyright_banners); ++i)
    {
        char *deh_s;

        deh_s = DEH_String(copyright_banners[i]);

        if (deh_s != copyright_banners[i])
        {
            printf("%s", deh_s);

            // Make sure the modified banner always ends in a newline character.
            // If it doesn't, add a newline.  This fixes av.wad.

            if (deh_s[strlen(deh_s) - 1] != '\n')
            {
                printf("\n");
            }
        }
    }
}

static struct 
{
    char *description;
    char *cmdline;
    GameVersion_t version;
} gameversions[] = {
    {"Doom 1.666",           "1.666",      exe_doom_1_666},
    {"Doom 1.7/1.7a",        "1.7",        exe_doom_1_7},
    {"Doom 1.8",             "1.8",        exe_doom_1_8},
    {"Doom 1.9",             "1.9",        exe_doom_1_9},
//  {"Hacx",                 "hacx",       exe_hacx},
    {"Ultimate Doom",        "ultimate",   exe_ultimate},
    {"Doom 1.9 SE",          "doomse",     exe_doom_se},
    {"Final Doom",           "final",      exe_final},
    {"Final Doom (alt)",     "final2",     exe_final2},
//  {"Chex Quest",           "chex",       exe_chex},
    { NULL,                  NULL,         0},
};

// Initialize the game version

static void InitGameVersion(void)
{
    byte    *demolump;
    char    demolumpname[6];
    int     demoversion;
    int     p;
    int     i;
    boolean status;

    //! 
    // @arg <version>
    // @category compat
    //
    // Emulate a specific version of Doom.  Valid values are "1.666",
    // "1.7", "1.8", "1.9", "ultimate", "final", "final2", "hacx" and
    // "chex".
    //

    p = M_CheckParmWithArgs("-gameversion", 1);

    if (p)
    {
        for (i=0; gameversions[i].description != NULL; ++i)
        {
            if (!strcmp(myargv[p+1], gameversions[i].cmdline))
            {
                gameversion = gameversions[i].version;
                break;
            }
        }

        if (gameversions[i].description == NULL) 
        {
            printf("Supported game versions:\n");

            for (i=0; gameversions[i].description != NULL; ++i)
            {
                printf("\t%s (%s)\n", gameversions[i].cmdline, gameversions[i].description);
            }

            I_Error("Unknown game version '%s'", myargv[p+1]);
        }
    }
    else
    {
        // Determine automatically

        if (gamemission == pack_chex)
        {
            // chex.exe - identified by iwad filename
            gameversion = exe_chex;
        }
        else if (gamemission == pack_hacx)
        {
            // hacx.exe: identified by iwad filename
            gameversion = exe_hacx;
        }
        else if (gamemode == shareware || gamemode == registered || (gamemode == commercial && gamemission == doom2))
        {
            // original
            gameversion = exe_doom_1_9;

            // Detect version from demo lump
            for (i = 1; i <= 3; ++i)
            {
                M_snprintf(demolumpname, 6, "demo%i", i);
                if (W_CheckNumForName(demolumpname) > 0)
                {
                    demolump = W_CacheLumpName(demolumpname, PU_STATIC);
                    demoversion = demolump[0];
                    W_ReleaseLumpName(demolumpname);
                    status = true;
                    switch (demoversion)
                    {
                        case 106:
                            gameversion = exe_doom_1_666;
                            break;
                        case 107:
                            gameversion = exe_doom_1_7;
                            break;
                        case 108:
                            gameversion = exe_doom_1_8;
                            break;
                        case 109:
                            gameversion = exe_doom_1_9;
                            break;
                        default:
                            status = false;
                            break;
                    }
                    if (status)
                    {
                        break;
                    }
                }
            }
        }
        else if (gamemode == retail)
        {
            gameversion = exe_ultimate;
        }
        else if (gamemode == commercial)
        {
            // Final Doom: tnt or plutonia
            // Defaults to emulating the first Final Doom executable,
            // which has the crash in the demo loop; however, having
            // this as the default should mean that it plays back
            // most demos correctly.

            gameversion = exe_final;
        }
    }

    // The original exe does not support retail - 4th episode not supported

    if (gameversion < exe_ultimate && gamemode == retail)
    {
        gamemode = registered;
    }

    // EXEs prior to the Final Doom exes do not support Final Doom.

    if (gameversion < exe_final && gamemode == commercial && (gamemission == pack_tnt || gamemission == pack_plut))
    {
        gamemission = doom2;
    }
}

void PrintGameVersion(void)
{
    int i;

    for (i=0; gameversions[i].description != NULL; ++i)
    {
        if (gameversions[i].version == gameversion)
        {
            printf("Emulating the behavior of the '%s' executable.\n",
                   gameversions[i].description);
            break;
        }
    }
}


// Load dehacked patches needed for certain IWADs.
static void LoadIwadDeh(void)
{
    // [JN] Doom Press Beta and Doom for Atari Jaguar 
    // have some info in DEHACKED lump, load it.
    if (gamemode == pressbeta || gamemission == jaguar)
    {
        DEH_LoadLumpByName("DEHACKED", false, true);
    }
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
    int     p;
    char    file[256];
    char    demolumpname[9];

    // print banner
    I_PrintBanner(PACKAGE_STRING);

    DEH_printf(english_language ?
               "Z_Init: Init zone memory allocation daemon. \n" :
               "Z_Init: Инициализация распределения памяти.\n");
    Z_Init ();

#ifdef FEATURE_MULTIPLAYER
    //!
    // @category net
    //
    // Start a dedicated server, routing packets but not participating
    // in the game itself.
    //

    if (M_CheckParm("-dedicated") > 0)
    {
        printf(english_language ?
               "Dedicated server mode.\n" :
               "Режим выделенного сервера.\n");
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

#endif

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
    // @vanilla
    //
    // Developer mode.  F1 saves a screenshot in the current working
    // directory.
    //

    devparm = M_CheckParm ("-devparm");

    I_DisplayFPSDots(devparm);

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

    //!
    // @arg <x>
    // @vanilla
    //
    // Turbo mode.  The player's speed is multiplied by x%.  If unspecified,
    // x defaults to 200.  Values are rounded up to 10 and down to 400.
    //

    if ( (p=M_CheckParm ("-turbo")) )
    {
        int         scale = 200;
        extern int  forwardmove[2];
        extern int  sidemove[2];

        if (p<myargc-1)
            scale = atoi (myargv[p+1]);
        if (scale < 10)
            scale = 10;
        if (scale > 400)
            scale = 400;
        
        DEH_printf(english_language ?
                   "turbo scale: %i%%\n" :
                   "турбо ускорение: %i%%\n",
                   scale);
        forwardmove[0] = forwardmove[0]*scale/100;
        forwardmove[1] = forwardmove[1]*scale/100;
        sidemove[0] = sidemove[0]*scale/100;
        sidemove[1] = sidemove[1]*scale/100;
    }

    // init subsystems
    DEH_printf(english_language ?
               "V_Init: allocate screens.\n" :
               "V_Init: Обнаружение экранов.\n");
    V_Init ();

    // Load configuration files before initialising other subsystems.
    DEH_printf(english_language ?
               "M_LoadDefaults: Load system defaults.\n" :
               "M_LoadDefaults: Загрузка системных стандартов.\n");
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
        if (english_language)
        {
            I_Error("Game mode indeterminate.  No IWAD file was found.  Try\n"
                    "specifying one with the '-iwad' command line parameter.\n");
        }
        else
        {
            I_Error("Невозможно определить игру из за отсутствующего IWAD-файла.\n"
                    "Попробуйте указать IWAD-файл командой '-iwad'.\n");
        }
    }

    modifiedgame = false;

    DEH_printf(english_language ?
               "W_Init: Init WADfiles.\n" :
               "W_Init: Инициализация WAD-файлов.\n");
    D_AddFile(iwadfile);
    numiwadlumps = numlumps;

    W_CheckCorrectIWAD(doom);

    // Now that we've loaded the IWAD, we can figure out what gamemission
    // we're playing and which version of Vanilla Doom we need to emulate.
    D_IdentifyVersion();
    InitGameVersion();

    // Check which IWAD variant we are using.

    if (W_CheckNumForName("FREEDOOM") >= 0)
    {
        if (W_CheckNumForName("FREEDM") >= 0)
            gamevariant = freedm;
        else
            gamevariant = freedoom;
    }
    else if (W_CheckNumForName("DMENUPIC") >= 0)
    {
        gamevariant = bfgedition;
    }
    // [JN] Checking for older sharewares
    else if (gamemode == shareware && W_CheckNumForName("STCHAT") >= 0)
    {
        gamevariant = old_shareware;
    }
    // [JN] Checking for Press Beta
    else if (W_CheckNumForName("DOOMPRES") >= 0)
    {
        gamemode = pressbeta;
    }
    // [JN] Checking for Atari Jaguar
    else if (W_CheckNumForName("RDJAGUAR") >= 0)
    {
        gamemission = jaguar;
    }

    //!
    // @category mod
    //
    // Disable automatic loading of Dehacked patches for certain
    // IWAD files.
    //
    if (!M_ParmExists("-nodeh"))
    {
        // Some IWADs have dehacked patches that need to be loaded for
        // them to be played properly.
        LoadIwadDeh();
    }

    // Doom 3: BFG Edition includes modified versions of the classic
    // IWADs which can be identified by an additional DMENUPIC lump.
    // Furthermore, the M_GDHIGH lumps have been modified in a way that
    // makes them incompatible to Vanilla Doom and the modified version
    // of doom2.wad is missing the TITLEPIC lump.
    // We specifically check for DMENUPIC here, before PWADs have been
    // loaded which could probably include a lump of that name.

#ifdef FEATURE_DEHACKED
    // Load Dehacked patches specified on the command line with -deh.
    // Note that there's a very careful and deliberate ordering to how
    // Dehacked patches are loaded. The order we use is:
    //  1. IWAD dehacked patches.
    //  2. Command line dehacked patches specified with -deh.
    //  3. PWAD dehacked patches in DEHACKED lumps.
    DEH_ParseCommandLine();
#endif

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
            DEH_snprintf(file, sizeof(file), "%s.lmp", myargv[p+1]);
        }

        free(uc_filename);

        if (D_AddFile(file))
        {
            M_StringCopy(demolumpname, lumpinfo[numlumps - 1]->name, sizeof(demolumpname));
        }
        else
        {
            // If file failed to load, still continue trying to play
            // the demo in the same way as Vanilla Doom.  This makes
            // tricks like "-playdemo demo1" possible.

            M_StringCopy(demolumpname, myargv[p + 1], sizeof(demolumpname));
        }

        printf(english_language ?
               "Playing demo %s.\n" :
               "Проигрывание демозаписи: %s.\n", file);
    }

    I_AtExit(G_CheckDemoStatusAtExit, true);

    // Generate the WAD hash table.  Speed things up a bit.
    W_GenerateHashTable();

    // [Julia] Set window title
    gamedescription = GetGameName("DOOM for Atari Jaguar");

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

    // Check for -file in shareware
    if (modifiedgame && (gamevariant != freedoom))
    {
        // These are the lumps that will be checked in IWAD,
        // if any one is not present, execution will be aborted.
        char name[23][8]=
        {
            "e2m1","e2m2","e2m3","e2m4","e2m5","e2m6","e2m7","e2m8","e2m9",
            "e3m1","e3m3","e3m3","e3m4","e3m5","e3m6","e3m7","e3m8","e3m9",
            "dphoof","bfgga0","heada1","cybra1","spida1d1"
        };

        int i;

        if ( gamemode == shareware)
            I_Error(DEH_String(english_language ?
                               "\nYou cannot -file with the shareware version. Register!" :
                               "\nВы не можете использовать -file в демонстрационной версии. Приобретите полную версию!"));

        // Check for fake IWAD with right name,
        // but w/o all the lumps of the registered version. 
        if (gamemode == registered)
            for (i = 0;i < 23; i++)
            if (W_CheckNumForName(name[i])<0)
                I_Error(DEH_String(english_language ?
                                   "\nThis is not the registered version." :
                                   "\nДанная версия не является зарегистрированной."));
    }

    I_PrintStartupBanner(gamedescription);
    PrintDehackedBanners();

    DEH_printf(english_language ?
               "I_Init: Setting up machine state.\n" :
               "I_Init: Инициализация состояния компьютера.\n");
    I_CheckIsScreensaver();
    I_InitTimer();
    I_InitJoystick();
    I_InitSound(true);
    I_InitMusic();

#ifdef FEATURE_MULTIPLAYER
    printf (english_language ?
            "NET_Init: Init network subsystem.\n" :
            "NET_Init: Инициализация сетевой подсистемы.\n");
    NET_Init ();
#endif

    // Initial netgame startup. Connect to server etc.
    D_ConnectNetGame();

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
    // @arg <n>
    // @vanilla
    //
    // Start playing on episode n (1-4)
    //

    p = M_CheckParmWithArgs("-episode", 1);

    if (p)
    {
        startepisode = myargv[p+1][0]-'0';
        startmap = 1;
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
        if (gamemode == commercial)
            startmap = atoi (myargv[p+1]);
        else
        {
            startepisode = myargv[p+1][0]-'0';

            if (p + 2 < myargc)
            {
                startmap = myargv[p+2][0]-'0';
            }
            else
            {
                startmap = 1;
            }
        }
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

    DEH_printf(english_language ?
               "M_Init: Init miscellaneous info.\n" :
               "M_Init: Инициализация внутренних данных.\n");
    M_Init ();

    DEH_printf(english_language ?
               "R_Init: Init DOOM refresh daemon - " :
               "R_Init: Инициализация процесса запуска DOOM - ");
    R_Init ();

    DEH_printf(english_language ?
               "\nP_Init: Init Playloop state.\n" :
               "\nP_Init: Инициализация игрового окружения.\n");
    P_Init ();

    DEH_printf(english_language ?
               "S_Init: Setting up sound.\n" :
               "S_Init: Активация звуковой системы.\n");
    S_Init (sfxVolume * 8, musicVolume * 8);

    DEH_printf(english_language ?
               "D_CheckNetGame: Checking network game status.\n" :
               "D_CheckNetGame: Проверка статуса сетевой игры.\n");
    D_CheckNetGame ();

    PrintGameVersion();

    DEH_printf(english_language ?
               "HU_Init: Setting up heads up display.\n" :
               "HU_Init: Настройка игрового дисплея.\n");
    HU_Init ();

    DEH_printf(english_language ?
               "ST_Init: Init status bar.\n" :
               "ST_Init: Инициализация строки состояния.\n");
    ST_Init ();

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
        singledemo = true;  // quit after one demo
        G_DeferedPlayDemo (demolumpname);
        D_DoomLoop ();      // never returns
    }

    p = M_CheckParmWithArgs("-timedemo", 1);
    if (p)
    {
        G_TimeDemo (demolumpname);
        D_DoomLoop ();      // never returns
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
            D_StartTitle ();    // start up intro loop
    }

    D_DoomLoop ();  // never returns
}

