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
//	DOOM selection menu, options, episode etc.
//	Sliders and icons. Kinda widget stuff.
//


#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "doomdef.h"
#include "doomkeys.h"
#include "d_englsh.h"
#include "d_main.h"
#include "i_input.h"
#include "i_swap.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "m_controls.h"
#include "m_misc.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"
#include "r_local.h"
#include "g_game.h"
#include "m_argv.h"
#include "m_controls.h"
#include "s_sound.h"
#include "doomstat.h"
#include "sounds.h"
#include "m_menu.h"
#include "p_local.h"
#include "ct_chat.h"
#include "v_trans.h"
#include "am_map.h"
#include "st_bar.h"
#include "r_collit.h"

#include "id_vars.h"
#include "id_func.h"


#define SKULLXOFF		-32
#define LINEHEIGHT       16


boolean menuactive;

// -1 = no quicksave slot picked!
static int quickSaveSlot;

 // 1 = message to be printed
static int messageToPrint;
// ...and here is the message string!
static const char *messageString;

// message x & y
static int messageLastMenuActive;

// timed message = no input from user
static boolean messageNeedsInput;

static void (*messageRoutine)(int response);

static char *gammalvls[MAXGAMMA][2] =
{
    { GAMMALVL_N050,  "-0.50" },
    { GAMMALVL_N055,  "-0.55" },
    { GAMMALVL_N060,  "-0.60" },
    { GAMMALVL_N065,  "-0.65" },
    { GAMMALVL_N070,  "-0.70" },
    { GAMMALVL_N075,  "-0.75" },
    { GAMMALVL_N080,  "-0.80" },
    { GAMMALVL_N085,  "-0.85" },
    { GAMMALVL_N090,  "-0.90" },
    { GAMMALVL_N095,  "-0.95" },
    { GAMMALVL_OFF,   "OFF"   },
    { GAMMALVL_010,   "0.1"   },
    { GAMMALVL_020,   "0.2"   },
    { GAMMALVL_030,   "0.3"   },
    { GAMMALVL_040,   "0.4"   },
    { GAMMALVL_050,   "0.5"   },
    { GAMMALVL_060,   "0.6"   },
    { GAMMALVL_070,   "0.7"   },
    { GAMMALVL_080,   "0.8"   },
    { GAMMALVL_090,   "0.9"   },
    { GAMMALVL_100,   "1.0"   },
    { GAMMALVL_110,   "1.1"   },
    { GAMMALVL_120,   "1.2"   },
    { GAMMALVL_130,   "1.3"   },
    { GAMMALVL_140,   "1.4"   },
    { GAMMALVL_150,   "1.5"   },
    { GAMMALVL_160,   "1.6"   },
    { GAMMALVL_170,   "1.7"   },
    { GAMMALVL_180,   "1.8"   },
    { GAMMALVL_190,   "1.9"   },
    { GAMMALVL_200,   "2.0"   },
    { GAMMALVL_220,   "2.2"   },
    { GAMMALVL_240,   "2.4"   },
    { GAMMALVL_260,   "2.6"   },
    { GAMMALVL_280,   "2.8"   },
    { GAMMALVL_300,   "3.0"   },
    { GAMMALVL_320,   "3.2"   },
    { GAMMALVL_340,   "3.4"   },
    { GAMMALVL_360,   "3.6"   },
    { GAMMALVL_380,   "3.8"   },
    { GAMMALVL_400,   "4.0"   },
};

// we are going to be entering a savegame string
static int saveStringEnter;              
static int saveSlot;	   // which slot to save in
static int saveCharIndex;  // which char we're editing
static boolean joypadSave = false; // was the save action initiated by joypad?

// old save description before edit
static char saveOldString[SAVESTRINGSIZE];  

// [FG] support up to 8 pages of savegames
// int savepage = 0;
// static const int savepage_max = 7;

static char savegamestrings[10][SAVESTRINGSIZE];
static char endstring[160];


//
// MENU TYPEDEFS
//

typedef struct
{
    // 0 = no cursor here, 1 = ok, 2 = arrows ok
    short	status;

    // [JN] Menu item timer for glowing effect.
    short   tics;
    
    char	name[32];
    
    // choice = menu item #.
    // if status = 2,
    //   choice=0:leftarrow,1:rightarrow
    void	(*routine)(int choice);
    
    // hotkey in menu
    char	alphaKey;			
} menuitem_t;

typedef struct menu_s
{
    short		numitems;	// # of menu items ([PN] Automatic count via ITEMCOUNT() macro below)
    struct menu_s*	prevMenu;	// previous menu
    menuitem_t*		menuitems;	// menu items
    void		(*routine)(void);	// draw routine
    short		x;
    short		y;		// x,y of menu
    short		lastOn;		// last item user was on in menu
    boolean		smallFont;	// [JN] Menu is using small font
    boolean		ScrollAR;	// [JN] Menu can be scrolled by arrow keys
    boolean		ScrollPG;	// [JN] Menu can be scrolled by PGUP/PGDN keys
} menu_t;

#define ITEMCOUNT(items) (sizeof(items) / sizeof((items)[0]))

// [JN] For currentMenu->menuitems[itemOn].statuses:
#define STS_SKIP -1
#define STS_NONE  0
#define STS_SWTC  1
#define STS_MUL1  2
#define STS_MUL2  3
#define STS_SLDR  4

// [JN] Macro definitions for first two items of menuitem_t.
// Trailing zero initializes "tics" field.
#define M_SKIP    STS_SKIP,0  // Skippable, cursor can't get here.
#define M_SWTC    STS_SWTC,0  // On/off type or entering function.
#define M_MUL1    STS_MUL1,0  // Multichoice function: increase by wheel up, decrease by wheel down
#define M_MUL2    STS_MUL2,0  // Multichoice function: decrease by wheel up, increase by wheel down
#define M_SLDR    STS_SLDR,0  // Slider line.

// [JN] Small cursor timer for glowing effect.
static short   cursor_tics = 0;
static boolean cursor_direction = false;

static short itemOn;            // menu item skull is on (-1 = no selection)
static short skullAnimCounter;  // skull animation counter
static short whichSkull;        // which skull to draw

// graphic name of skulls
// warning: initializer-string for array of chars is too long
static char *skullName[2] = {"M_SKULL1","M_SKULL2"};

// current menudef
static menu_t *currentMenu;

// =============================================================================
// PROTOTYPES
// =============================================================================

static void M_NewGame(int choice);
static void M_ChooseSkill(int choice);
static void M_LoadGame(int choice);
static void M_SaveGame(int choice);
static void M_EndGame(int choice);
static void M_QuitDOOM(int choice);

static void M_Choose_ID_Main (int choice);
static menu_t ID_Def_Main;

static void M_ChangeMessages(int choice);
static void M_SfxVol(int choice);
static void M_MusicVol(int choice);
static void M_ChangeDetail(int choice);
static void M_SizeDisplay(int choice);

static void M_FinishReadThis(int choice);
static void M_LoadSelect(int choice);
static void M_SaveSelect(int choice);
static void M_ReadSaveStrings(void);
static void M_QuickSave(void);
static void M_QuickLoad(void);

static void M_DrawMainMenu(void);
static void M_DrawHelp(void);
static void M_DrawNewGame(void);
static void M_DrawSound(void);
static void M_DrawLoad(void);
static void M_DrawSave(void);

static void M_DrawSaveLoadBorder(int x,int y);
static void M_SetupNextMenu(menu_t *menudef);
static void M_DrawThermo(int x,int y,int thermWidth,int thermDot,int itemPos);
static int  M_StringHeight(const char *string);
static void M_StartMessage(const char *string, void (*routine)(int), boolean input);
static void M_ClearMenus (void);

static void M_ID_MenuMouseControl (void);
static void M_ID_HandleSliderMouseControl (int x, int y, int width, void *value, boolean is_float, float min, float max);

// =============================================================================
// DOOM MENU
// =============================================================================

enum
{
    newgame = 0,
    options,
    loadgame,
    savegame,
    quitdoom,
    main_end
} main_e;

static menuitem_t MainMenu[]=
{
	{ M_SWTC, "New game",   M_NewGame,        'n' },
	{ M_SWTC, "Options",    M_Choose_ID_Main, 'o' },
	{ M_SWTC, "Load game",  M_LoadGame,       'l' },
	{ M_SWTC, "Save game",  M_SaveGame,       's' },
	{ M_SWTC, "Quit game",  M_QuitDOOM,       'q' }
};

static menu_t MainDef =
{
    ITEMCOUNT(MainMenu),
    NULL,
    MainMenu,
    M_DrawMainMenu,
    97,72,
    0,
    false, false, false,
};

//
// NEW GAME
//

enum
{
    killthings,
    toorough,
    hurtme,
    violence,
    nightmare,
    newg_end
} newgame_e;

static menuitem_t NewGameMenu[]=
{
    { M_SWTC, "I'm a Wimp.",     M_ChooseSkill,  'i' },
    { M_SWTC, "Not too rough.",  M_ChooseSkill,  'h' },
    { M_SWTC, "Hurt me plenty.", M_ChooseSkill,  'h' },
    { M_SWTC, "Ultra-Violence.", M_ChooseSkill,  'u' },
    { M_SWTC, "",                M_ChooseSkill,  'n' }
};

static menu_t NewDef =
{
    ITEMCOUNT(NewGameMenu), // # of menu items
    &MainDef,       // previous menu
    NewGameMenu,    // menuitem_t ->
    M_DrawNewGame,  // drawing routine ->
    69,63,          // x,y
    hurtme,         // lastOn
    false, false, false,
};

//
// HELP Menu
//

enum
{
    helpempty,
    help_end
} read_e;

static menuitem_t HelpMenu[] =
{
    { M_SWTC, "", M_FinishReadThis, 0 }
};

static menu_t HelpDef =
{
    ITEMCOUNT(HelpMenu),
    &MainDef,
    HelpMenu,
    M_DrawHelp,
    330,165,
    0,
    false, false, false,
};

//
// SOUND VOLUME MENU
//

enum
{
    sfx_vol,
    sfx_empty1,
    music_vol,
    sfx_empty2,
    sound_end
} sound_e;

static menuitem_t SoundMenu[]=
{
    { M_SLDR, "Sfx Volume",   M_SfxVol,    's'  },
    { M_SKIP, "",             0,           '\0' },
    { M_SLDR, "Music Volume", M_MusicVol,  'm'  },
    { M_SKIP, "",             0,           '\0' }
};

static menu_t SoundDef =
{
    ITEMCOUNT(SoundMenu),
    &ID_Def_Main,
    SoundMenu,
    M_DrawSound,
    80,64,
    0,
    false, false, false,
};

//
// LOAD GAME MENU
//

enum
{
    load1,
    load2,
    load3,
    load4,
    load5,
    load6,
    load7,
    load8,
    load_end
} load_e;

static menuitem_t LoadMenu[]=
{
    { M_SWTC, "",  M_LoadSelect,  '1' },
    { M_SWTC, "",  M_LoadSelect,  '2' },
    { M_SWTC, "",  M_LoadSelect,  '3' },
    { M_SWTC, "",  M_LoadSelect,  '4' },
    { M_SWTC, "",  M_LoadSelect,  '5' },
    { M_SWTC, "",  M_LoadSelect,  '6' },
    { M_SWTC, "",  M_LoadSelect,  '7' },
    { M_SWTC, "",  M_LoadSelect,  '8' }
};

static menu_t LoadDef =
{
    ITEMCOUNT(LoadMenu),
    &MainDef,
    LoadMenu,
    M_DrawLoad,
    67,28,
    0,
    false, false, false,
};

//
// SAVE GAME MENU
//

static menuitem_t SaveMenu[]=
{
    { M_SWTC, "",  M_SaveSelect,  '1' },
    { M_SWTC, "",  M_SaveSelect,  '2' },
    { M_SWTC, "",  M_SaveSelect,  '3' },
    { M_SWTC, "",  M_SaveSelect,  '4' },
    { M_SWTC, "",  M_SaveSelect,  '5' },
    { M_SWTC, "",  M_SaveSelect,  '6' },
    { M_SWTC, "",  M_SaveSelect,  '7' },
    { M_SWTC, "",  M_SaveSelect,  '8' }
};

static menu_t SaveDef =
{
    ITEMCOUNT(SaveMenu),
    &MainDef,
    SaveMenu,
    M_DrawSave,
    67,28,
    0,
    false, false, false,
};

// =============================================================================
// [JN] Custom ID menu
// =============================================================================

#define ID_MENU_TOPOFFSET         (27)
#define ID_MENU_TOPOFFSET_SML     (18)
#define ID_MENU_LEFTOFFSET        (48)
#define ID_MENU_LEFTOFFSET_SML    (90)
#define ID_MENU_LEFTOFFSET_BIG    (32)
#define ID_MENU_LEFTOFFSET_LEVEL  (74)

#define ID_MENU_LINEHEIGHT_SMALL  (9)
#define ID_MENU_CURSOR_OFFSET     (10)

// Utility function to align menu item names by the right side.
static int M_ItemRightAlign (const char *text)
{
    return ORIGWIDTH - currentMenu->x - M_StringWidth(text);
}

static player_t *player;

static void M_Draw_ID_Main (void);

static void M_Choose_ID_Video (int choice);
static void M_Draw_ID_Video_1 (void);
static void M_ID_RenderingRes (int choice);
static void M_ID_Widescreen (int choice);
static void M_ID_ExclusiveFS (int choice);

static void M_ID_UncappedFPS (int choice);
static void M_ID_LimitFPS (int choice);
static void M_ID_VSync (int choice);
static void M_ID_ShowFPS (int choice);
static void M_ID_PixelScaling (int choice);
static void M_ID_ScreenWipe (int choice);

static void M_Draw_ID_Video_2 (void);
static void M_ID_SuperSmoothing (int choice);
static void M_ID_OverbrightGlow (int choice);
static void M_ID_SoftBloom (int choice);
static void M_ID_AnalogRGBDrift (int choice);
static void M_ID_VHSLineDistortion (int choice);
static void M_ID_ScreenVignette (int choice);
static void M_ID_FilmGrain (int choice);
static void M_ID_MotionBlur (int choice);
static void M_ID_DepthOfFieldBlur (int choice);
 
static void M_ScrollVideo (int choice);

static void M_Choose_ID_Display (int choice);
static void M_Draw_ID_Display (void);
static void M_ID_Gamma (int choice);
static void M_ID_CRYPalette (int choice);
static void M_ID_FOV (int choice);
static void M_ID_MenuShading (int choice);
static void M_ID_LevelBrightness (int choice);
static void M_ID_MessagesAlignment (int choice);
static void M_ID_TextShadows (int choice);
static void M_ID_LocalTime (int choice);

static void M_Choose_ID_Sound (int choice);
static void M_Draw_ID_Sound (void);
static void M_ID_SFXSystem (int choice);
static void M_ID_MusicSystem (int choice);
static void M_ID_SFXMode (int choice);
static void M_ID_SFXChannels (int choice);
static void M_ID_MuteInactive (int choice);
static void M_ID_PitchShift (int choice);

static void M_Choose_ID_Controls (int choice);
static void M_Draw_ID_Controls (void);
static void M_ID_Controls_Sensivity (int choice);
static void M_ID_Controls_Sensivity_y (int choice);
static void M_ID_Controls_Acceleration (int choice);
static void M_ID_Controls_Threshold (int choice);
static void M_ID_Controls_MLook (int choice);
static void M_ID_Controls_NoVert (int choice);
static void M_ID_Controls_InvertY (int choice);

static void M_Draw_ID_Keybinds_1 (void);
static void M_Bind_MoveForward (int choice);
static void M_Bind_MoveBackward (int choice);
static void M_Bind_TurnLeft (int choice);
static void M_Bind_TurnRight (int choice);
static void M_Bind_StrafeLeft (int choice);
static void M_Bind_StrafeRight (int choice);
static void M_Bind_SpeedOn (int choice);
static void M_Bind_StrafeOn (int choice);
static void M_Bind_180Turn (int choice);
static void M_Bind_FireAttack (int choice);
static void M_Bind_Use (int choice);

static void M_Draw_ID_Keybinds_2 (void);
static void M_Bind_AlwaysRun (int choice);
static void M_Bind_MouseLook (int choice);
static void M_Bind_NoVert (int choice);
static void M_Bind_RestartLevel (int choice);
static void M_Bind_NextLevel (int choice);
static void M_Bind_FlipLevels (int choice);
static void M_Bind_ExtendedHUD (int choice);
static void M_Bind_SpectatorMode (int choice);
static void M_Bind_FreezeMode (int choice);
static void M_Bind_NotargetMode (int choice);
static void M_Bind_BuddhaMode (int choice);

static void M_Draw_ID_Keybinds_3 (void);
static void M_Bind_Weapon1 (int choice);
static void M_Bind_Weapon2 (int choice);
static void M_Bind_Weapon3 (int choice);
static void M_Bind_Weapon4 (int choice);
static void M_Bind_Weapon5 (int choice);
static void M_Bind_Weapon6 (int choice);
static void M_Bind_Weapon7 (int choice);
static void M_Bind_Weapon8 (int choice);
static void M_Bind_PrevWeapon (int choice);
static void M_Bind_NextWeapon (int choice);

static void M_Draw_ID_Keybinds_4 (void);
static void M_Bind_ToggleMap (int choice);
static void M_Bind_ZoomIn (int choice);
static void M_Bind_ZoomOut (int choice);
static void M_Bind_MaxZoom (int choice);
static void M_Bind_FollowMode (int choice);
static void M_Bind_RotateMode (int choice);
static void M_Bind_OverlayMode (int choice);
static void M_Bind_ToggleGrid (int choice);
static void M_Bind_AddMark (int choice);
static void M_Bind_ClearMarks (int choice);

static void M_Draw_ID_Keybinds_5 (void);
static void M_Bind_HelpScreen (int choice);
static void M_Bind_SaveGame (int choice);
static void M_Bind_LoadGame (int choice);
static void M_Bind_SoundVolume (int choice);
static void M_Bind_ToggleDetail (int choice);
static void M_Bind_QuickSave (int choice);
static void M_Bind_EndGame (int choice);
static void M_Bind_ToggleMessages (int choice);
static void M_Bind_QuickLoad (int choice);
static void M_Bind_QuitGame (int choice);
static void M_Bind_ToggleGamma (int choice);
static void M_Bind_TogglePalette (int choice);

static void M_Draw_ID_Keybinds_6 (void);
static void M_Bind_Pause (int choice);
static void M_Bind_SaveScreenshot (int choice);
static void M_Bind_LastMessage (int choice);
static void M_Bind_Reset (int choice);

static void M_Choose_ID_MouseBinds (int choice);
static void M_Draw_ID_MouseBinds (void);
static void M_Bind_M_FireAttack (int choice);
static void M_Bind_M_MoveForward (int choice);
static void M_Bind_M_MoveBackward (int choice);
static void M_Bind_M_Use (int choice);
static void M_Bind_M_SpeedOn (int choice);
static void M_Bind_M_StrafeOn (int choice);
static void M_Bind_M_StrafeLeft (int choice);
static void M_Bind_M_StrafeRight (int choice);
static void M_Bind_M_PrevWeapon (int choice);
static void M_Bind_M_NextWeapon (int choice);
static void M_Bind_M_Reset (int choice);

static void M_Choose_ID_Widgets (int choice);
static void M_Draw_ID_Widgets (void);
static void M_ID_Widget_Placement (int choice);
static void M_ID_Widget_Alignment (int choice);
static void M_ID_Widget_KIS (int choice);
static void M_ID_Widget_KIS_Format (int choice);
static void M_ID_Widget_Time (int choice);
static void M_ID_Widget_TotalTime (int choice);
static void M_ID_Widget_LevelName (int choice);
static void M_ID_Widget_Coords (int choice);
static void M_ID_Widget_Render (int choice);
static void M_ID_Widget_Health (int choice);

static void M_Choose_ID_Automap (int choice);
static void M_Draw_ID_Automap (void);
static void M_ID_Automap_Smooth (int choice);
static void M_ID_Automap_Thick (int choice);
static void M_ID_Automap_Square (int choice);
static void M_ID_Automap_Secrets (int choice);
static void M_ID_Automap_Rotate (int choice);
static void M_ID_Automap_Overlay (int choice);
static void M_ID_Automap_Shading (int choice);

static void M_Draw_ID_Gameplay_1 (void);
static void M_ID_Brightmaps (int choice);
static void M_ID_Translucency (int choice);
static void M_ID_ImprovedFuzz (int choice);
static void M_ID_ColoredLighting (int choice);
static void M_ID_ColoredBlood (int choice);
static void M_ID_SwirlingLiquids (int choice);
static void M_ID_AnimatedSky (int choice);
static void M_ID_LinearSky (int choice);
static void M_ID_FlipCorpses (int choice);
static void M_ID_Crosshair (int choice);
static void M_ID_CrosshairColor (int choice);

static void M_Draw_ID_Gameplay_2 (void);
static void M_ID_ColoredSTBar (int choice);
static void M_ID_NegativeHealth (int choice);
static void M_ID_Torque (int choice);
static void M_ID_TossDrop (int choice);
static void M_ID_FloatingPowerups (int choice);
static void M_ID_WeaponAlignment (int choice);
static void M_ID_VerticalAiming (int choice);
static void M_ID_Breathing (int choice);
static void M_ID_SndOfCrushedCorpse (int choice);

static void M_Draw_ID_Gameplay_3 (void);
static void M_ID_DefaulSkill (int choice);
static void M_ID_PistolStart (int choice);
static void M_ID_RevealedSecrets (int choice);
static void M_ID_FlipLevels (int choice);
static void M_ID_OnDeathAction (int choice);
static void M_ID_JaguarMusic (int choice);
static void M_ID_JaguarAlert (int choice);
static void M_ID_JaguarExplosion (int choice);
static void M_ID_JaguarSkies (int choice);

static void M_ScrollGameplay (int choice);

static void M_Choose_ID_Reset (int choice);
static int  resetplaque_tics;

// Keyboard binding prototypes
static boolean KbdIsBinding;
static int     keyToBind;

static char   *M_NameBind (int itemSetOn, int key);
static void    M_StartBind (int keynum);
static void    M_CheckBind (int key);
static void    M_DoBind (int keynum, int key);
static void    M_ClearBind (int itemOn);
static void    M_ResetBinds (void);
static void    M_DrawBindKey (int itemNum, int yPos, int key);
static void    M_DrawBindFooter (char *pagenum, boolean drawPages);

// Mouse binding prototypes
static boolean MouseIsBinding;
static int     btnToBind;

static char   *M_NameMouseBind (int itemSetOn, int btn);
static void    M_StartMouseBind (int btn);
static void    M_CheckMouseBind (int btn);
static void    M_DoMouseBind (int btnnum, int btn);
static void    M_ClearMouseBind (int itemOn);
static void    M_DrawBindButton (int itemNum, int yPos, int btn);
static void    M_ResetMouseBinds (void);

// Forward declarations for scrolling and remembering last pages.
static menu_t ID_Def_Video_1;
static menu_t ID_Def_Video_2;
static menu_t ID_Def_Keybinds_1;
static menu_t ID_Def_Keybinds_2;
static menu_t ID_Def_Keybinds_3;
static menu_t ID_Def_Keybinds_4;
static menu_t ID_Def_Keybinds_5;
static menu_t ID_Def_Keybinds_6;
static menu_t ID_Def_Gameplay_1;
static menu_t ID_Def_Gameplay_2;
static menu_t ID_Def_Gameplay_3;

// Remember last keybindings page.
static int Keybinds_Cur;

static menu_t *KeybindsMenus[] =
{
    &ID_Def_Keybinds_1,
    &ID_Def_Keybinds_2,
    &ID_Def_Keybinds_3,
    &ID_Def_Keybinds_4,
    &ID_Def_Keybinds_5,
    &ID_Def_Keybinds_6,
};

static void M_Choose_ID_Keybinds (int choice)
{
    M_SetupNextMenu(KeybindsMenus[Keybinds_Cur]);
}

// Remember last gameplay page.
static int Gameplay_Cur;

static menu_t *GameplayMenus[] =
{
    &ID_Def_Gameplay_1,
    &ID_Def_Gameplay_2,
    &ID_Def_Gameplay_3,
};

static void M_Choose_ID_Gameplay (int choice)
{
    M_SetupNextMenu(GameplayMenus[Gameplay_Cur]);
}

// [JN/PN] Utility function for scrolling pages by arrows / PG keys.
static void M_ScrollPages (boolean direction)
{
    // "sfx_pstop" sound will be played only if menu will be changed.
    menu_t *nextMenu = NULL;

    // Remember cursor position.
    currentMenu->lastOn = itemOn;

    // Save/Load menu:
    /*
    if (currentMenu == &LoadDef || currentMenu == &SaveDef)
    {
        if (direction)
        {
            if (savepage < savepage_max)
            {
                savepage++;
                S_StartSound(NULL, sfx_pstop);
            }
        }
        else
        {
            if (savepage > 0)
            {
                savepage--;
                S_StartSound(NULL, sfx_pstop);
            }
        }
        quickSaveSlot = -1;
        M_ReadSaveStrings();
        return;
    }
    */

    // Video options:
         if (currentMenu == &ID_Def_Video_1) nextMenu = &ID_Def_Video_2;
    else if (currentMenu == &ID_Def_Video_2) nextMenu = &ID_Def_Video_1;

    // Keyboard bindings:
    else if (currentMenu == &ID_Def_Keybinds_1) nextMenu = (direction ? &ID_Def_Keybinds_2 : &ID_Def_Keybinds_6);
    else if (currentMenu == &ID_Def_Keybinds_2) nextMenu = (direction ? &ID_Def_Keybinds_3 : &ID_Def_Keybinds_1);
    else if (currentMenu == &ID_Def_Keybinds_3) nextMenu = (direction ? &ID_Def_Keybinds_4 : &ID_Def_Keybinds_2);
    else if (currentMenu == &ID_Def_Keybinds_4) nextMenu = (direction ? &ID_Def_Keybinds_5 : &ID_Def_Keybinds_3);
    else if (currentMenu == &ID_Def_Keybinds_5) nextMenu = (direction ? &ID_Def_Keybinds_6 : &ID_Def_Keybinds_4);
    else if (currentMenu == &ID_Def_Keybinds_6) nextMenu = (direction ? &ID_Def_Keybinds_1 : &ID_Def_Keybinds_5);

    // Gameplay features:
    else if (currentMenu == &ID_Def_Gameplay_1) nextMenu = (direction ? &ID_Def_Gameplay_2 : &ID_Def_Gameplay_3);
    else if (currentMenu == &ID_Def_Gameplay_2) nextMenu = (direction ? &ID_Def_Gameplay_3 : &ID_Def_Gameplay_1);
    else if (currentMenu == &ID_Def_Gameplay_3) nextMenu = (direction ? &ID_Def_Gameplay_1 : &ID_Def_Gameplay_2);

    // If a new menu was set up, play the navigation sound.
    if (nextMenu)
    {
        M_SetupNextMenu(nextMenu);
        S_StartSound(NULL, sfx_pstop);
    }
}

// -----------------------------------------------------------------------------

// [JN] Delay before shading.
static int shade_wait;

// [JN] Shade background while in active menu.
static void M_ShadeBackground (void)
{
    if (dp_menu_shading)
    {
        pixel_t *dest = I_VideoBuffer;
        const int shade = dp_menu_shading;
        const int scr = SCREENAREA;
        
        for (int i = 0; i < scr; i++)
        {
            *dest = I_BlendDark(*dest, I_ShadeFactor[shade]);
            ++dest;
        }
    }
}

static void M_FillBackground (void)
{
    const byte *src = W_CacheLumpName("FLOOR4_8", PU_CACHE);
    pixel_t *dest = I_VideoBuffer;

    V_FillFlat(0, SCREENHEIGHT, 0, SCREENWIDTH, src, dest);
}

static int M_Line_Alpha (int tics)
{
    const int alpha_val[] = { 0, 50, 100, 150, 200, 255 };
    return alpha_val[tics];
}

#define LINE_ALPHA(i)   M_Line_Alpha(currentMenu->menuitems[(i)].tics)

static void M_Reset_Line_Glow (void)
{
    for (int i = 0 ; i < currentMenu->numitems ; i++)
    {
        currentMenu->menuitems[i].tics = 0;
    }

    // [JN] If menu is controlled by mouse, reset "last on" position
    // so this item won't blink upon reentering to the current menu.
    if (menu_mouse_allow)
    {
        currentMenu->lastOn = -1;
    }
}

static int M_INT_Slider (int val, int min, int max, int direction, boolean capped)
{
    // [PN] Adjust the slider value based on direction and handle min/max limits
    val += (direction == -1) ?  0 :     // [JN] Routine "-1" just reintializes value.
           (direction ==  0) ? -1 : 1;  // Otherwise, move either left "0" or right "1".

    if (val < min)
        val = capped ? min : max;
    else
    if (val > max)
        val = capped ? max : min;

    return val;
}

static float M_FLOAT_Slider (float val, float min, float max, float step,
                             int direction, boolean capped)
{
    // [PN] Adjust value based on direction
    val += (direction == -1) ? 0 :            // [JN] Routine "-1" just reintializes value.
           (direction ==  0) ? -step : step;  // Otherwise, move either left "0" or right "1".

    // [PN] Handle min/max limits
    if (val < min)
        val = capped ? min : max;
    else
    if (val > max)
        val = capped ? max : min;

    // [PN/JN] Do a float correction to get x.xxx000 values
    val = roundf(val * 1000.0f) / 1000.0f;

    return val;
}

static void M_DrawScrollPages (int x, int y, int itemOnGlow, const char *pagenum)
{
    char str[32];
    
    M_WriteTextGlow(x, y, "< SCROLL PAGES >",
                        cr[CR_LIGHTGRAY],
                            cr[CR_LIGHTGRAY_BRIGHT],
                                LINE_ALPHA(itemOnGlow));

    M_snprintf(str, 32, "%s", M_StringJoin("PAGE ", pagenum, NULL));
    M_WriteTextGlow(M_ItemRightAlign(str), y, str,
                        cr[CR_GRAY],
                            cr[CR_GRAY_BRIGHT],
                                LINE_ALPHA(itemOnGlow));
}

// -----------------------------------------------------------------------------
// Main ID Menu
// -----------------------------------------------------------------------------

static menuitem_t ID_Menu_Main[]=
{
    { M_SWTC, "Video",       M_Choose_ID_Video,    'v' },
    { M_SWTC, "Display",     M_Choose_ID_Display,  'd' },
    { M_SWTC, "Audio",       M_Choose_ID_Sound,    'a' },
    { M_SWTC, "Controls",    M_Choose_ID_Controls, 'c' },
    { M_SWTC, "Widgets",     M_Choose_ID_Widgets,  'w' },
    { M_SWTC, "Automap",     M_Choose_ID_Automap,  'a' },
    { M_SWTC, "Gameplay",    M_Choose_ID_Gameplay, 'g' },
    { M_SWTC, "End Game",    M_EndGame,            'e' },
    { M_SWTC, "Reset",       M_Choose_ID_Reset,    'r' },
};

static menu_t ID_Def_Main =
{
    ITEMCOUNT(ID_Menu_Main),
    &MainDef,
    ID_Menu_Main,
    M_Draw_ID_Main,
    97, 16/*ID_MENU_TOPOFFSET*/,
    0,
    false, false, false,
};

static void M_Choose_ID_Main (int choice)
{
    M_SetupNextMenu (&ID_Def_Main);
}

static void M_Draw_ID_Main (void)
{
    M_WriteTextBigCentered(1, "Options", cr[CR_YELLOW]);
}

// -----------------------------------------------------------------------------
// Video options 1
// -----------------------------------------------------------------------------

static menuitem_t ID_Menu_Video_1[]=
{
    { M_MUL1, "RENDERING RESOLUTION", M_ID_RenderingRes,  'r' },
    { M_MUL1, "WIDESCREEN MODE",      M_ID_Widescreen,    'w' },
    { M_MUL2, "EXCLUSIVE FULLSCREEN", M_ID_ExclusiveFS,   'e' },
    { M_MUL2, "UNCAPPED FRAMERATE",   M_ID_UncappedFPS,   'u' },
    { M_MUL1, "FRAMERATE LIMIT",      M_ID_LimitFPS,      'f' },
    { M_MUL2, "ENABLE VSYNC",         M_ID_VSync,         'e' },
    { M_MUL2, "SHOW FPS COUNTER",     M_ID_ShowFPS,       's' },
    { M_MUL2, "PIXEL SCALING",        M_ID_PixelScaling,  'p' },
    { M_MUL2, "SCREEN WIPE EFFECT",   M_ID_ScreenWipe,    's' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_MUL1, "", /* NEXT PAGE > */   M_ScrollVideo,      'n' },
};

static menu_t ID_Def_Video_1 =
{
    ITEMCOUNT(ID_Menu_Video_1),
    &ID_Def_Main,
    ID_Menu_Video_1,
    M_Draw_ID_Video_1,
    ID_MENU_LEFTOFFSET, ID_MENU_TOPOFFSET_SML,
    0,
    true, false, true,
};

static void M_Choose_ID_Video (int choice)
{
    M_SetupNextMenu (&ID_Def_Video_1);
}

static void M_Draw_ID_Video_1 (void)
{
    char str[32];

    M_WriteTextCentered(9, "VIDEO OPTIONS", cr[CR_YELLOW]);

    // Rendering resolution
    sprintf(str, vid_resolution == 1 ? "1X (200P)"  :
                 vid_resolution == 2 ? "2X (400P)"  :
                 vid_resolution == 3 ? "3X (600P)"  :
                 vid_resolution == 4 ? "4X (800P)"  :
                 vid_resolution == 5 ? "5X (1000P)" :
                 vid_resolution == 6 ? "6X (1200P)" :
                                       "CUSTOM");
    M_WriteTextGlow(M_ItemRightAlign(str), 18, str, 
                        vid_resolution == 1 ? cr[CR_DARKRED] :
                        vid_resolution == 2 ||
                        vid_resolution == 3 ? cr[CR_GREEN] :
                        vid_resolution == 4 ||
                        vid_resolution == 5 ? cr[CR_YELLOW] : cr[CR_ORANGE],
                            vid_resolution == 1 ? cr[CR_RED_BRIGHT] :
                            vid_resolution == 2 ||
                            vid_resolution == 3 ? cr[CR_GREEN_BRIGHT] :
                            vid_resolution == 4 ||
                            vid_resolution == 5 ? cr[CR_YELLOW_BRIGHT] : cr[CR_ORANGE_BRIGHT],
                                LINE_ALPHA(0));

    // Widescreen rendering
    sprintf(str, vid_widescreen == 1 ? "MATCH SCREEN" :
                 vid_widescreen == 2 ? "16:10" :
                 vid_widescreen == 3 ? "16:9" :
                 vid_widescreen == 4 ? "21:9" :
                 vid_widescreen == 5 ? "32:9" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 27, str, 
                        vid_widescreen ? cr[CR_GREEN] : cr[CR_DARKRED], 
                            vid_widescreen ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT], 
                                LINE_ALPHA(1));

    // Exclusive fullscreen
    sprintf(str, vid_fullscreen_exclusive ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 36, str, 
                        vid_fullscreen_exclusive ? cr[CR_GREEN] : cr[CR_DARKRED],
                            vid_fullscreen_exclusive ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT], 
                                LINE_ALPHA(2));

    // Uncapped framerate
    sprintf(str, vid_uncapped_fps ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 45, str, 
                        vid_uncapped_fps ? cr[CR_GREEN] : cr[CR_DARKRED], 
                            vid_uncapped_fps ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT], 
                                LINE_ALPHA(3));

    // Framerate limit
    sprintf(str, !vid_uncapped_fps ? "35" :
                 vid_fpslimit ? "%d" : "NONE", vid_fpslimit);
    M_WriteTextGlow(M_ItemRightAlign(str), 54, str, 
                        !vid_uncapped_fps ? cr[CR_DARKRED] :
                        vid_fpslimit == 0 ? cr[CR_RED] :
                        vid_fpslimit >= 500 ? cr[CR_YELLOW] : cr[CR_GREEN],
                            !vid_uncapped_fps ? cr[CR_RED_BRIGHT] :
                            vid_fpslimit == 0 ? cr[CR_RED_BRIGHT] :
                            vid_fpslimit >= 500 ? cr[CR_YELLOW_BRIGHT] : cr[CR_GREEN_BRIGHT],
                                LINE_ALPHA(4));

    // Enable vsync
    sprintf(str, vid_vsync ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 63, str, 
                    vid_vsync ? cr[CR_GREEN] : cr[CR_DARKRED],
                            vid_vsync ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(5));

    // Show FPS counter
    sprintf(str, vid_showfps ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 72, str, 
                    vid_showfps ? cr[CR_GREEN] : cr[CR_DARKRED],
                            vid_showfps ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(6));

    // Pixel scaling
    sprintf(str, vid_smooth_scaling ? "SMOOTH" : "SHARP");
    M_WriteTextGlow(M_ItemRightAlign(str), 81, str, 
                        vid_smooth_scaling ? cr[CR_GREEN] : cr[CR_DARKRED],
                            vid_smooth_scaling ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(7));

    // Screen wipe effect
    sprintf(str, vid_screenwipe == 1 ? "LOADING" :
                 vid_screenwipe == 2 ? "MELT" :
                 vid_screenwipe == 3 ? "CROSSFADE" :
                 vid_screenwipe == 4 ? "FIZZLE" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 90, str,
                        vid_screenwipe == 1 ? cr[CR_DARKRED] : cr[CR_GREEN],
                            vid_screenwipe == 1 ? cr[CR_RED_BRIGHT] : cr[CR_GREEN_BRIGHT],
                                LINE_ALPHA(8));

    // [JN] Print current resolution. Shamelessly taken from Nugget Doom!
    if (itemOn == 0 || itemOn == 1)
    {
        char  width[8];
        char  height[8];
        const char *resolution;

        M_snprintf(width, 8, "%d", (ORIGWIDTH + (WIDESCREENDELTA*2)) * vid_resolution);
        M_snprintf(height, 8, "%d", (ORIGHEIGHT * vid_resolution));
        resolution = M_StringJoin("CURRENT RESOLUTION: ", width, "x", height, NULL);

        M_WriteTextCentered(103, resolution, cr[CR_LIGHTGRAY_DARK]);
    }

    // < Scroll pages >
    M_DrawScrollPages(ID_MENU_LEFTOFFSET, 117, 11, "1/2");
}

static void M_ID_RenderingResHook (void)
{
    // [crispy] re-initialize framebuffers, textures and renderer
    I_ReInitGraphics(REINIT_FRAMEBUFFERS | REINIT_TEXTURES | REINIT_ASPECTRATIO);
    // [crispy] re-calculate framebuffer coordinates
    R_ExecuteSetViewSize();
    // [crispy] re-draw bezel
    R_FillBackScreen();
    // [JN] re-calculate status bar elements background buffers
    ST_InitElementsBackground();
    // [crispy] re-calculate automap coordinates
    AM_LevelInit(true);
    if (automapactive)
    {
        AM_Start();
    }
    // [JN] re-initialize mouse cursor position
    I_ReInitCursorPosition();
}

static void M_ID_RenderingRes (int choice)
{
    vid_resolution = M_INT_Slider(vid_resolution, 1, MAXHIRES, choice, false);
    post_rendering_hook = M_ID_RenderingResHook;
}

static void M_ID_WidescreenHook (void)
{
    // [crispy] re-initialize framebuffers, textures and renderer
    I_ReInitGraphics(REINIT_FRAMEBUFFERS | REINIT_TEXTURES | REINIT_ASPECTRATIO);
    // [crispy] re-calculate framebuffer coordinates
    R_ExecuteSetViewSize();
    // [crispy] re-draw bezel
    R_FillBackScreen();
    // [JN] re-calculate status bar elements background buffers
    ST_InitElementsBackground();
    // [crispy] re-calculate automap coordinates
    AM_LevelInit(true);
    if (automapactive)
    {
        AM_Start();
    }
}

static void M_ID_Widescreen (int choice)
{
    vid_widescreen = M_INT_Slider(vid_widescreen, 0, 5, choice, false);
    post_rendering_hook = M_ID_WidescreenHook;
}

static void M_ID_ExclusiveFSHook (void)
{
    vid_fullscreen_exclusive ^= 1;

    // [JN] Force to update, if running in fullscreen mode.
    if (vid_fullscreen)
    {
        I_UpdateExclusiveFullScreen();
    }
}

static void M_ID_ExclusiveFS (int choice)
{
    post_rendering_hook = M_ID_ExclusiveFSHook;
}

static void M_ID_UncappedFPS (int choice)
{
    vid_uncapped_fps ^= 1;
}

static void M_ID_LimitFPS (int choice)
{
    if (!vid_uncapped_fps)
    {
        return;  // Do not allow change value in capped framerate.
    }
    
    switch (choice)
    {
        case 0:
            if (vid_fpslimit)
            {
                if (speedkeydown())
                    vid_fpslimit -= 10;
                else
                    vid_fpslimit -= 1;
            }

            if (vid_fpslimit < TICRATE)
                vid_fpslimit = 0;

            break;
        case 1:
            if (vid_fpslimit < 501)
            {
                if (speedkeydown())
                    vid_fpslimit += 10;
                else
                    vid_fpslimit += 1;
            }

            if (vid_fpslimit < TICRATE)
                vid_fpslimit = TICRATE;
            if (vid_fpslimit > 500)
                vid_fpslimit = 500;

        default:
            break;
    }
}

static void M_ID_VSyncHook (void)
{
    I_ToggleVsync();
}

static void M_ID_VSync (int choice)
{
    vid_vsync ^= 1;
    post_rendering_hook = M_ID_VSyncHook;    
}

static void M_ID_ShowFPS (int choice)
{
    vid_showfps ^= 1;
}

static void M_ID_PixelScaling (int choice)
{
    vid_smooth_scaling ^= 1;

    // [crispy] re-calculate the zlight[][] array
    R_InitLightTables();
    // [crispy] re-calculate the scalelight[][] array
    R_ExecuteSetViewSize();
}

static void M_ID_ScreenWipe (int choice)
{
    vid_screenwipe = M_INT_Slider(vid_screenwipe, 0, 4, choice, false);
}

// -----------------------------------------------------------------------------
// Video options 2
// -----------------------------------------------------------------------------

static menuitem_t ID_Menu_Video_2[]=
{
    { M_MUL1, "SUPERSAMPLED SMOOTHING", M_ID_SuperSmoothing,    's' },
    { M_MUL2, "OVERBRIGHT GLOW",        M_ID_OverbrightGlow,    't' },
    { M_MUL2, "SOFT BLOOM",             M_ID_SoftBloom,         's' },
    { M_MUL1, "ANALOG RGB DRIFT",       M_ID_AnalogRGBDrift,    'a' },
    { M_MUL2, "VHS LINE DISTORTION",    M_ID_VHSLineDistortion, 'v' },
    { M_MUL1, "SCREEN VIGNETTE",        M_ID_ScreenVignette,    's' },
    { M_MUL1, "FILM GRAIN",             M_ID_FilmGrain,         'f' },
    { M_MUL1, "MOTION BLUR",            M_ID_MotionBlur,        'm' },
    { M_MUL2, "DEPTH OF FIELD BLUR",    M_ID_DepthOfFieldBlur,  'd' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_MUL1, "", /* PREV PAGE > */     M_ScrollVideo,          'p' },
};

static menu_t ID_Def_Video_2 =
{
    ITEMCOUNT(ID_Menu_Video_2),
    &ID_Def_Main,
    ID_Menu_Video_2,
    M_Draw_ID_Video_2,
    ID_MENU_LEFTOFFSET, ID_MENU_TOPOFFSET_SML,
    0,
    true, false, true,
};

static void M_Draw_ID_Video_2 (void)
{
    char str[32];
    const char *sample_factors[] = { "NONE", "1x", "2x", "3x", "4x", "5x", "6x" };

    M_WriteTextCentered(9, "POST-PROCESSING EFFECTS", cr[CR_YELLOW]);

    // Supersampled smoothing
    sprintf(str, "%s", sample_factors[post_supersample]);
    M_WriteTextGlow(M_ItemRightAlign(str), 18, str,
                        post_supersample ? cr[CR_GREEN] : cr[CR_DARKRED],
                            post_supersample ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(0));

    // Overbright glow
    sprintf(str, post_overglow ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 27, str, 
                        post_overglow ? cr[CR_GREEN] : cr[CR_DARKRED],
                            post_overglow ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(1));

    // Soft bloom
    sprintf(str, post_bloom ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 36, str, 
                        post_bloom ? cr[CR_GREEN] : cr[CR_DARKRED],
                            post_bloom ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(2));

    // Analog RGB drift
    sprintf(str, post_rgbdrift == 1 ? "SUBTLE" :
                 post_rgbdrift == 2 ? "STRONG" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 45, str, 
                        post_rgbdrift ? cr[CR_GREEN] : cr[CR_DARKRED],
                            post_rgbdrift ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(3));

    // VHS line distortion
    sprintf(str, post_vhsdist ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 54, str, 
                        post_vhsdist ? cr[CR_GREEN] : cr[CR_DARKRED],
                            post_vhsdist ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(4));

    // Screen vignette
    sprintf(str, post_vignette == 1 ? "SUBTLE" :
                 post_vignette == 2 ? "SOFT"   : 
                 post_vignette == 3 ? "STRONG" : 
                 post_vignette == 4 ? "DARK"   : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 63, str, 
                        post_vignette ? cr[CR_GREEN] : cr[CR_DARKRED],
                            post_vignette ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(5));

    // Film grain
    sprintf(str, post_filmgrain == 1 ? "SOFT"      :
                 post_filmgrain == 2 ? "LIGHT"     : 
                 post_filmgrain == 3 ? "MEDIUM"    : 
                 post_filmgrain == 4 ? "HEAVY"     : 
                 post_filmgrain == 5 ? "NIGHTMARE" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 72, str, 
                        post_filmgrain ? cr[CR_GREEN] : cr[CR_DARKRED],
                            post_filmgrain ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(6));

    // Motion blur
    sprintf(str, post_motionblur == 1 ? "SOFT"   :
                 post_motionblur == 2 ? "LIGHT"  : 
                 post_motionblur == 3 ? "MEDIUM" : 
                 post_motionblur == 4 ? "HEAVY"  : 
                 post_motionblur == 5 ? "GHOST"  : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 81, str, 
                        post_motionblur ? cr[CR_GREEN] : cr[CR_DARKRED],
                            post_motionblur ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(7));

    // Depth if field blur
    sprintf(str, post_dofblur ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 90, str, 
                        post_dofblur ? cr[CR_GREEN] : cr[CR_DARKRED],
                            post_dofblur ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(8));

    // < Scroll pages >
    M_DrawScrollPages(ID_MENU_LEFTOFFSET, 117, 11, "2/2");
}

static void M_ID_SuperSmoothing (int choice)
{
    post_supersample = M_INT_Slider(post_supersample, 0, 6, choice, false);
}

static void M_ID_OverbrightGlow (int choice)
{
    post_overglow ^= 1;
}

static void M_ID_SoftBloom (int choice)
{
    post_bloom ^= 1;
}

static void M_ID_AnalogRGBDrift (int choice)
{
    post_rgbdrift = M_INT_Slider(post_rgbdrift, 0, 2, choice, false);
}

static void M_ID_VHSLineDistortion (int choice)
{
    post_vhsdist ^= 1;
}

static void M_ID_ScreenVignette (int choice)
{
    post_vignette = M_INT_Slider(post_vignette, 0, 4, choice, false);
}

static void M_ID_FilmGrain (int choice)
{
    post_filmgrain = M_INT_Slider(post_filmgrain, 0, 5, choice, false);
}

static void M_ID_MotionBlur (int choice)
{
    post_motionblur = M_INT_Slider(post_motionblur, 0, 5, choice, false);
}

static void M_ID_DepthOfFieldBlur (int choice)
{
    post_dofblur ^= 1;
}

static void M_ScrollVideo (int choice)
{
         if (currentMenu == &ID_Def_Video_1) { M_SetupNextMenu(&ID_Def_Video_2); }
    else if (currentMenu == &ID_Def_Video_2) { M_SetupNextMenu(&ID_Def_Video_1); }
    itemOn = 11;
}

// -----------------------------------------------------------------------------
// Display options
// -----------------------------------------------------------------------------

static menuitem_t ID_Menu_Display[]=
{
    { M_SLDR, "GAMMA-CORRECTION",        M_ID_Gamma,             'g' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_MUL2, "EMULATE CRY PALETTE",     M_ID_CRYPalette,        'e' },
    { M_MUL1, "FIELD OF VIEW",           M_ID_FOV,               'f' },
    { M_MUL1, "MENU BACKGROUND SHADING", M_ID_MenuShading,       'm' },
    { M_MUL1, "EXTRA LEVEL BRIGHTNESS",  M_ID_LevelBrightness,   'e' },
    { M_SKIP, "", 0, '\0' },
    { M_MUL2, "MESSAGES ENABLED",        M_ChangeMessages,       'm' },
    { M_MUL2, "MESSAGES ALIGNMENT",      M_ID_MessagesAlignment, 'm' },
    { M_MUL2, "TEXT CASTS SHADOWS",      M_ID_TextShadows,       't' },
    { M_MUL2, "LOCAL TIME",              M_ID_LocalTime,         'l' },
};

static menu_t ID_Def_Display =
{
    ITEMCOUNT(ID_Menu_Display),
    &ID_Def_Main,
    ID_Menu_Display,
    M_Draw_ID_Display,
    ID_MENU_LEFTOFFSET, ID_MENU_TOPOFFSET_SML,
    0,
    true, false, false,
};

static void M_Choose_ID_Display (int choice)
{
    M_SetupNextMenu (&ID_Def_Display);
}

static void M_Draw_ID_Display (void)
{
    char str[32];

    M_WriteTextCentered(9, "DISPLAY OPTIONS", cr[CR_YELLOW]);

    // Gamma-correction slider and num
    M_DrawThermo(46, 27, 19, vid_gamma/2.1f, 0);
    M_ID_HandleSliderMouseControl(52, 28, 156, &vid_gamma, false, 0, 40);
    M_WriteTextGlow(216, 30, gammalvls[vid_gamma][1],
                        NULL, cr[CR_MENU_BRIGHT5],
                            LINE_ALPHA(0));

    // Emulate CRY palette
    sprintf(str, dp_cry_palette ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 45, str,
                        dp_cry_palette ? cr[CR_GREEN] : cr[CR_DARKRED],
                            dp_cry_palette ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(3));

    // Field of View
    sprintf(str, "%d", vid_fov);
    M_WriteTextGlow(M_ItemRightAlign(str), 54, str,
                        vid_fov == 135 || vid_fov == 45 ? cr[CR_YELLOW] :
                        vid_fov == 90 ? cr[CR_DARKRED] : cr[CR_GREEN],
                            vid_fov == 135 || vid_fov == 45 ? cr[CR_YELLOW_BRIGHT] :
                            vid_fov == 90 ? cr[CR_RED_BRIGHT] : cr[CR_GREEN_BRIGHT],
                                LINE_ALPHA(4));

    // Background shading
    sprintf(str, dp_menu_shading ? "%d" : "OFF", dp_menu_shading);
    M_WriteTextGlow(M_ItemRightAlign(str), 63, str,
                        dp_menu_shading == 12 ? cr[CR_YELLOW] :
                        dp_menu_shading  > 0  ? cr[CR_GREEN] : cr[CR_DARKRED],
                            dp_menu_shading == 12 ? cr[CR_YELLOW_BRIGHT] :
                            dp_menu_shading  > 0  ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(5));

    // Extra level brightness
    sprintf(str, dp_level_brightness ? "%d" : "OFF", dp_level_brightness);
    M_WriteTextGlow(M_ItemRightAlign(str), 72, str,
                        dp_level_brightness == 8 ? cr[CR_YELLOW] :
                        dp_level_brightness  > 0  ? cr[CR_GREEN] : cr[CR_DARKRED],
                            dp_level_brightness == 8 ? cr[CR_YELLOW_BRIGHT] :
                            dp_level_brightness  > 0  ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(6));

    M_WriteTextCentered(81, "MESSAGES SETTINGS", cr[CR_YELLOW]);

    // Messages enabled
    sprintf(str, msg_show ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 90, str,
                        msg_show ? cr[CR_GREEN] : cr[CR_DARKRED],
                            msg_show ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT], 
                                LINE_ALPHA(8));

    // Messages alignment
    sprintf(str, msg_alignment == 1 ? "STATUS BAR" :
                 msg_alignment == 2 ? "CENTERED" : "LEFT");
    M_WriteTextGlow(M_ItemRightAlign(str), 99, str,
                        msg_alignment ? cr[CR_GREEN] : cr[CR_DARKRED],
                            msg_alignment ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT], 
                                LINE_ALPHA(9));

    // Text casts shadows
    sprintf(str, msg_text_shadows ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 108, str, 
                        msg_text_shadows ? cr[CR_GREEN] : cr[CR_DARKRED],
                            msg_text_shadows ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT], 
                                LINE_ALPHA(10));

    // Local time
    sprintf(str, msg_local_time == 1 ? "12-HOUR FORMAT" :
                 msg_local_time == 2 ? "24-HOUR FORMAT" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 117, str, 
                        msg_local_time ? cr[CR_GREEN] : cr[CR_DARKRED],
                            msg_local_time ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT], 
                                LINE_ALPHA(11));
}

static void M_ID_Gamma (int choice)
{
    shade_wait = I_GetTime() + TICRATE;
    vid_gamma = M_INT_Slider(vid_gamma, 0, MAXGAMMA-1, choice, true);

    I_SetPalette(st_palette);
    R_InitColormaps();
    R_FillBackScreen();
    st_fullupdate = true;
}

static void M_ID_CRYPalette (int choice)
{
    dp_cry_palette ^= 1;
    R_InitColormaps();
    R_ExecuteSetViewSize();
    R_FillBackScreen();
    st_fullupdate = true;
}

static void M_ID_FOV (int choice)
{
    vid_fov = M_INT_Slider(vid_fov, 45, 135, choice, true);

    // [crispy] re-calculate the zlight[][] array
    R_InitLightTables();
    // [crispy] re-calculate the scalelight[][] array
    R_ExecuteSetViewSize();
}

static void M_ID_MenuShading (int choice)
{
    dp_menu_shading = M_INT_Slider(dp_menu_shading, 0, 12, choice, true);
}

static void M_ID_LevelBrightness (int choice)
{
    dp_level_brightness = M_INT_Slider(dp_level_brightness, 0, 8, choice, true);
}

static void M_ID_MessagesAlignment (int choice)
{
    msg_alignment = M_INT_Slider(msg_alignment, 0, 2, choice, false);
}

static void M_ID_TextShadows (int choice)
{
    msg_text_shadows ^= 1;
}

static void M_ID_LocalTime (int choice)
{
    msg_local_time = M_INT_Slider(msg_local_time, 0, 2, choice, false);
}

// -----------------------------------------------------------------------------
// Sound options
// -----------------------------------------------------------------------------

static menuitem_t ID_Menu_Sound[]=
{
    { M_SLDR, "SFX VOLUME",                M_SfxVol,          's' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_SLDR, "MUSIC VOLUME",              M_MusicVol,        'm' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_MUL2, "SFX PLAYBACK",              M_ID_SFXSystem,    's' },
    { M_MUL2, "MUSIC PLAYBACK",            M_ID_MusicSystem,  'm' },
    { M_MUL1, "SOUND EFFECTS MODE",        M_ID_SFXMode,      's' },
    { M_MUL2, "JAGUAR SFX PITCH-SHIFTING", M_ID_PitchShift,   'j' },
    { M_MUL1, "NUMBER OF SFX TO MIX",      M_ID_SFXChannels,  'n' },
    { M_MUL2, "MUTE INACTIVE WINDOW",      M_ID_MuteInactive, 'm' },
};

static menu_t ID_Def_Sound =
{
    ITEMCOUNT(ID_Menu_Sound),
    &ID_Def_Main,
    ID_Menu_Sound,
    M_Draw_ID_Sound,
    ID_MENU_LEFTOFFSET, ID_MENU_TOPOFFSET_SML,
    0,
    true, false, false,
};

static void M_Choose_ID_Sound (int choice)
{
    M_SetupNextMenu (&ID_Def_Sound);
}

static void M_Draw_ID_Sound (void)
{
    char str[16];

    M_WriteTextCentered(9, "VOLUME", cr[CR_YELLOW]);

    M_DrawThermo(46, 27, 16, sfxVolume, 0);
    M_ID_HandleSliderMouseControl(52, 27, 132, &sfxVolume, false, 0, 15);
    sprintf(str,"%d", sfxVolume);
    M_WriteTextGlow(192, 30, str, NULL, cr[CR_MENU_BRIGHT5], LINE_ALPHA(0));

    M_DrawThermo(46, 54, 16, musicVolume, 3);
    M_ID_HandleSliderMouseControl(52, 54, 132, &musicVolume, false, 0, 15);
    sprintf(str,"%d", musicVolume);
    M_WriteTextGlow(192, 57, str, NULL, cr[CR_MENU_BRIGHT5], LINE_ALPHA(3));

    M_WriteTextCentered(72, "SOUND SYSTEM", cr[CR_YELLOW]);

    // SFX playback
    sprintf(str, snd_sfxdevice == 0 ? "DISABLED"    :
                 snd_sfxdevice == 3 ? "DIGITAL SFX" :
                                      "UNKNOWN");
    M_WriteTextGlow(M_ItemRightAlign(str), 81, str,
                        snd_sfxdevice ? cr[CR_GREEN] : cr[CR_RED],
                            snd_sfxdevice ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(7));

    // Music playback
    sprintf(str, snd_musicdevice == 0 ? "DISABLED" :
                (snd_musicdevice == 3 && !strcmp(snd_dmxoption, "")) ? "OPL2 SYNTH" : 
                (snd_musicdevice == 3 && !strcmp(snd_dmxoption, "-opl3")) ? "OPL3 SYNTH" : 
                 snd_musicdevice == 8 ? "NATIVE MIDI" :
                                        "UNKNOWN");
    M_WriteTextGlow(M_ItemRightAlign(str), 90, str,
                        snd_musicdevice ? cr[CR_GREEN] : cr[CR_RED],
                            snd_musicdevice ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(8));

    // Sound effects mode
    sprintf(str, snd_monosfx ? "MONO" : "STEREO");
    M_WriteTextGlow(M_ItemRightAlign(str), 99, str,
                        snd_monosfx ? cr[CR_RED] : cr[CR_GREEN],
                            snd_monosfx ? cr[CR_RED_BRIGHT] : cr[CR_GREEN_BRIGHT],
                                LINE_ALPHA(9));

    // Pitch-shifted sounds
    sprintf(str, snd_pitchshift ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 108, str,
                        snd_pitchshift ? cr[CR_GREEN] : cr[CR_RED],
                            snd_pitchshift ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(10));

    // Number of SFX to mix
    sprintf(str, "%i", snd_channels);
    M_WriteTextGlow(M_ItemRightAlign(str), 117, str,
                        snd_channels == 8 ? cr[CR_DARKRED] :
                        snd_channels == 1 || snd_channels == 16 ? cr[CR_YELLOW] : cr[CR_GREEN],
                            snd_channels == 8 ? cr[CR_RED_BRIGHT] :
                            snd_channels == 1 || snd_channels == 16 ? cr[CR_YELLOW_BRIGHT] : cr[CR_GREEN_BRIGHT],                        
                                LINE_ALPHA(11));

    // Mute inactive window
    sprintf(str, snd_mute_inactive ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 125, str,
                        snd_mute_inactive ? cr[CR_GREEN] : cr[CR_RED],
                            snd_mute_inactive ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(12));
}

static void M_ID_SFXSystem (int choice)
{
    // Select between "disabled" and "digital sfx"
    snd_sfxdevice = snd_sfxdevice == 0 ? 3 : 0;

    // Shut down current music
    S_StopMusic();

    // Free all sound channels/usefulness
    S_ChangeSFXSystem();

    // Shut down sound/music system
    I_ShutdownSound();

    // Start sound/music system
    I_InitSound(doom);

    // Re-generate SFX cache
    I_PrecacheSounds(S_sfx, NUMSFX);

    // Reinitialize sound volume
    S_SetSfxVolume(sfxVolume * 8);

    // Reinitialize music volume
    S_SetMusicVolume(musicVolume * 8);

    // Restart current music
    S_ChangeMusic(current_mus_num, usergame);
}

static void M_ID_MusicSystem (int choice)
{
    switch (choice)
    {
        case 0:
            if (snd_musicdevice == 0)
            {
                snd_musicdevice = 8;    // Set to Native MIDI
            }
            else if (snd_musicdevice == 8)
            {
                snd_musicdevice = 3;    // Set to OPL3
                snd_dmxoption = "-opl3";
            }
            else if (snd_musicdevice == 3  && !strcmp(snd_dmxoption, "-opl3"))
            {
                snd_musicdevice = 3;    // Set to OPL2
                snd_dmxoption = "";
            }
            else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, ""))
            {
                snd_musicdevice = 0;    // Disable
            }
            break;
        case 1:
            if (snd_musicdevice == 0)
            {
                snd_musicdevice  = 3;   // Set to OPL2
                snd_dmxoption = "";
            }
            else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, ""))
            {
                snd_musicdevice  = 3;   // Set to OPL3
                snd_dmxoption = "-opl3";
            }
            else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, "-opl3"))
            {
                snd_musicdevice  = 8;   // Set to Native MIDI
            }
            else if (snd_musicdevice == 8)
            {
                snd_musicdevice  = 0;   // Disable
            }
        default:
            break;
    }

    // Shut down current music
    S_StopMusic();

    // Shut down music system
    S_Shutdown();
    
    // Start music system
    I_InitSound(doom);

    // Reinitialize music volume
    S_SetMusicVolume(musicVolume * 8);

    // Restart current music
    S_ChangeMusic(current_mus_num, usergame);
}

static void M_ID_SFXMode (int choice)
{
    snd_monosfx ^= 1;

    // Update stereo separation
    S_UpdateStereoSeparation();
}

static void M_ID_PitchShift (int choice)
{
    snd_pitchshift ^= 1;
}

static void M_ID_SFXChannels (int choice)
{
    snd_channels = M_INT_Slider(snd_channels, 1, 16, choice, true);
}

static void M_ID_MuteInactive (int choice)
{
    snd_mute_inactive ^= 1;
}

// -----------------------------------------------------------------------------
// Control settings
// -----------------------------------------------------------------------------

static menuitem_t ID_Menu_Controls[]=
{
    { M_SWTC, "KEYBOARD BINDINGS",            M_Choose_ID_Keybinds,       'k' },
    { M_SWTC, "MOUSE BINDINGS",               M_Choose_ID_MouseBinds,     'm' },
    { M_SKIP, "", 0, '\0' },
    { M_SLDR, "HORIZONTAL SENSITIVITY",       M_ID_Controls_Sensivity,    'h' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_SLDR, "VERTICAL SENSITIVITY",         M_ID_Controls_Sensivity_y,  'v' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_MUL1, "ACCELERATION",                 M_ID_Controls_Acceleration, 'a' },
    { M_MUL1, "ACCELERATION THRESHOLD",       M_ID_Controls_Threshold,    'a' },
    { M_MUL2, "MOUSE LOOK",                   M_ID_Controls_MLook,        'm' },
    { M_MUL2, "VERTICAL MOUSE MOVEMENT",      M_ID_Controls_NoVert,       'v' },
    { M_MUL2, "INVERT VERTICAL AXIS",         M_ID_Controls_InvertY,      'v' },
};

static menu_t ID_Def_Controls =
{
    ITEMCOUNT(ID_Menu_Controls),
    &ID_Def_Main,
    ID_Menu_Controls,
    M_Draw_ID_Controls,
    ID_MENU_LEFTOFFSET, ID_MENU_TOPOFFSET - 9, // [JN] This menu is one line higher.
    0,
    true, false, false,
};

static void M_Choose_ID_Controls (int choice)
{
    M_SetupNextMenu (&ID_Def_Controls);
}

static void M_Draw_ID_Controls (void)
{
    char str[32];

    M_WriteTextCentered(9, "BINDINGS", cr[CR_YELLOW]);
    
    M_WriteTextCentered(36, "MOUSE CONFIGURATION", cr[CR_YELLOW]);

    M_DrawThermo(46, 54, 15, mouseSensitivity, 3);
    M_ID_HandleSliderMouseControl(52, 54, 124, &mouseSensitivity, false, 0, 14);
    sprintf(str,"%d", mouseSensitivity);
    M_WriteTextGlow(184, 57, str,
                        mouseSensitivity == 255 ? cr[CR_YELLOW] :
                        mouseSensitivity  >  14 ? cr[CR_GREEN] : NULL,
                            mouseSensitivity == 255 ? cr[CR_YELLOW_BRIGHT] :
                            mouseSensitivity  >  14 ? cr[CR_GREEN_BRIGHT] : cr[CR_MENU_BRIGHT5],
                                LINE_ALPHA(3));

    M_DrawThermo(46, 81, 15, mouse_sensitivity_y, 6);
    M_ID_HandleSliderMouseControl(52, 81, 124, &mouse_sensitivity_y, false, 0, 14);
    sprintf(str,"%d", mouse_sensitivity_y);
    M_WriteTextGlow(184, 84, str,
                        mouse_sensitivity_y == 255 ? cr[CR_YELLOW] :
                        mouse_sensitivity_y  >  14 ? cr[CR_GREEN] : NULL,
                            mouse_sensitivity_y == 255 ? cr[CR_YELLOW_BRIGHT] :
                            mouse_sensitivity_y  >  14 ? cr[CR_GREEN_BRIGHT] : cr[CR_MENU_BRIGHT5],
                                LINE_ALPHA(6));

    // Acceleration
    sprintf(str,"%.1f", mouse_acceleration);
    M_WriteTextGlow(M_ItemRightAlign(str), 99, str,
                        mouse_acceleration == 2.0f ? NULL :
                        mouse_acceleration == 1.0f ? cr[CR_DARKRED] :
                        mouse_acceleration  < 2.0f ? cr[CR_YELLOW] : cr[CR_GREEN],
                            mouse_acceleration == 2.0f ? cr[CR_MENU_BRIGHT5] :
                            mouse_acceleration == 1.0f ? cr[CR_RED_BRIGHT] :
                            mouse_acceleration  < 2.0f ? cr[CR_YELLOW_BRIGHT] : cr[CR_GREEN_BRIGHT],                        
                                LINE_ALPHA(9));

    // Acceleration threshold
    sprintf(str,"%d", mouse_threshold);
    M_WriteTextGlow(M_ItemRightAlign(str), 108, str,
                        mouse_threshold == 10 ? NULL :
                        mouse_threshold ==  0 ? cr[CR_DARKRED] :
                        mouse_threshold  < 10 ? cr[CR_YELLOW] : cr[CR_GREEN],
                            mouse_threshold == 10 ? cr[CR_MENU_BRIGHT5] :
                            mouse_threshold ==  0 ? cr[CR_RED_BRIGHT] :
                            mouse_threshold  < 10 ? cr[CR_YELLOW_BRIGHT] : cr[CR_GREEN_BRIGHT],
                                LINE_ALPHA(10));

    // Mouse look
    sprintf(str, mouse_look ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 117, str,
                        mouse_look ? cr[CR_GREEN] : cr[CR_RED],
                            mouse_look ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(11));

    // Vertical mouse movement
    sprintf(str, mouse_novert ? "OFF" : "ON");
    M_WriteTextGlow(M_ItemRightAlign(str), 126, str,
                        mouse_novert ? cr[CR_RED] : cr[CR_GREEN],
                            mouse_novert ? cr[CR_RED_BRIGHT] : cr[CR_GREEN_BRIGHT],
                                LINE_ALPHA(12));

    // Invert vertical axis
    sprintf(str, mouse_y_invert ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 135, str,
                        mouse_y_invert ? cr[CR_GREEN] : cr[CR_RED],
                            mouse_y_invert ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(13));
}

static void M_ID_Controls_Sensivity (int choice)
{
    // [crispy] extended range
    mouseSensitivity = M_INT_Slider(mouseSensitivity, 0, 255, choice, true);
}

static void M_ID_Controls_Sensivity_y (int choice)
{
    // [crispy] extended range
    mouse_sensitivity_y = M_INT_Slider(mouse_sensitivity_y, 0, 255, choice, true);
}

static void M_ID_Controls_Acceleration (int choice)
{
    mouse_acceleration = M_FLOAT_Slider(mouse_acceleration, 1.000000f, 5.000000f, 0.100000f, choice, true);
}

static void M_ID_Controls_Threshold (int choice)
{
    mouse_threshold = M_INT_Slider(mouse_threshold, 0, 32, choice, true);
}

static void M_ID_Controls_MLook (int choice)
{
    mouse_look ^= 1;
    if (!mouse_look)
    {
        players[consoleplayer].lookdir = 0;
    }
}

static void M_ID_Controls_NoVert (int choice)
{
    mouse_novert ^= 1;
}

static void M_ID_Controls_InvertY (int choice)
{
    mouse_y_invert ^= 1;
}

// -----------------------------------------------------------------------------
// Keybinds 1
// -----------------------------------------------------------------------------

static menuitem_t ID_Menu_Keybinds_1[]=
{
    { M_SWTC, "MOVE FORWARD",    M_Bind_MoveForward,  'm' },
    { M_SWTC, "MOVE BACKWARD",   M_Bind_MoveBackward, 'm' },
    { M_SWTC, "TURN LEFT",       M_Bind_TurnLeft,     't' },
    { M_SWTC, "TURN RIGHT",      M_Bind_TurnRight,    't' },
    { M_SWTC, "STRAFE LEFT",     M_Bind_StrafeLeft,   's' },
    { M_SWTC, "STRAFE RIGHT",    M_Bind_StrafeRight,  's' },
    { M_SWTC, "SPEED ON",        M_Bind_SpeedOn,      's' },
    { M_SWTC, "STRAFE ON",       M_Bind_StrafeOn,     's' },
    { M_SWTC, "180 DEGREE TURN", M_Bind_180Turn,      '1' },
    { M_SKIP, "", 0, '\0'},
    { M_SWTC, "FIRE/ATTACK",     M_Bind_FireAttack,   'f' },
    { M_SWTC, "USE",             M_Bind_Use,          'u' },
    { M_SKIP, "", 0, '\0'},
    { M_SKIP, "", 0, '\0'},
    { M_SKIP, "", 0, '\0'},
    { M_SKIP, "", 0, '\0'},
};

static menu_t ID_Def_Keybinds_1 =
{
    ITEMCOUNT(ID_Menu_Keybinds_1),
    &ID_Def_Controls,
    ID_Menu_Keybinds_1,
    M_Draw_ID_Keybinds_1,
    ID_MENU_LEFTOFFSET, ID_MENU_TOPOFFSET_SML,
    0,
    true, true, true,
};

static void M_Bind_MoveForward (int choice)
{
    M_StartBind(100);  // key_up
}

static void M_Bind_MoveBackward (int choice)
{
    M_StartBind(101);  // key_down
}

static void M_Bind_TurnLeft (int choice)
{
    M_StartBind(102);  // key_left
}

static void M_Bind_TurnRight (int choice)
{
    M_StartBind(103);  // key_right
}

static void M_Bind_StrafeLeft (int choice)
{
    M_StartBind(104);  // key_strafeleft
}

static void M_Bind_StrafeRight (int choice)
{
    M_StartBind(105);  // key_straferight
}

static void M_Bind_SpeedOn (int choice)
{
    M_StartBind(106);  // key_speed
}

static void M_Bind_StrafeOn (int choice)
{
    M_StartBind(107);  // key_strafe
}

static void M_Bind_180Turn (int choice)
{
    M_StartBind(108);  // key_180turn
}

static void M_Bind_FireAttack (int choice)
{
    M_StartBind(109);  // key_fire
}

static void M_Bind_Use (int choice)
{
    M_StartBind(110);  // key_use
}

static void M_Draw_ID_Keybinds_1 (void)
{
    st_fullupdate = true;
    Keybinds_Cur = 0;

    M_FillBackground();

    M_WriteTextCentered(9, "MOVEMENT", cr[CR_YELLOW]);

    M_DrawBindKey(0, 18, key_up);
    M_DrawBindKey(1, 27, key_down);
    M_DrawBindKey(2, 36, key_left);
    M_DrawBindKey(3, 45, key_right);
    M_DrawBindKey(4, 54, key_strafeleft);
    M_DrawBindKey(5, 63, key_straferight);
    M_DrawBindKey(6, 72, key_speed);
    M_DrawBindKey(7, 81, key_strafe);
    M_DrawBindKey(8, 90, key_180turn);

    M_WriteTextCentered(99, "ACTION", cr[CR_YELLOW]);

    M_DrawBindKey(10, 108, key_fire);
    M_DrawBindKey(11, 117, key_use);

    M_DrawBindFooter("1", true);
}

// -----------------------------------------------------------------------------
// Keybinds 2
// -----------------------------------------------------------------------------

static menuitem_t ID_Menu_Keybinds_2[]=
{
    { M_SWTC, "ALWAYS RUN",              M_Bind_AlwaysRun,      'a' },
    { M_SWTC, "MOUSE LOOK",              M_Bind_MouseLook,      'm' },
    { M_SWTC, "VERTICAL MOUSE MOVEMENT", M_Bind_NoVert,         'v' },
    { M_SKIP, "", 0, '\0'},
    { M_SWTC, "RESTART LEVEL",           M_Bind_RestartLevel,   'r' },
    { M_SWTC, "GO TO NEXT LEVEL",        M_Bind_NextLevel,      'g' },
    { M_SWTC, "FLIP LEVEL HORIZONTALLY", M_Bind_FlipLevels,     'f' },
    { M_SWTC, "TOGGLE EXTENDED HUD",     M_Bind_ExtendedHUD,    't' },
    { M_SKIP, "", 0, '\0'},
    { M_SWTC, "SPECTATOR MODE",           M_Bind_SpectatorMode, 's' },
    { M_SWTC, "FREEZE MODE",              M_Bind_FreezeMode,    'f' },
    { M_SWTC, "NOTARGET MODE",            M_Bind_NotargetMode,  'n' },
    { M_SWTC, "BUDDHA MODE",              M_Bind_BuddhaMode,    'b' },
    { M_SKIP, "", 0, '\0'},
    { M_SKIP, "", 0, '\0'},
    { M_SKIP, "", 0, '\0'},
};

static menu_t ID_Def_Keybinds_2 =
{
    ITEMCOUNT(ID_Menu_Keybinds_2),
    &ID_Def_Controls,
    ID_Menu_Keybinds_2,
    M_Draw_ID_Keybinds_2,
    ID_MENU_LEFTOFFSET, ID_MENU_TOPOFFSET_SML,
    0,
    true, true, true,
};

static void M_Bind_AlwaysRun (int choice)
{
    M_StartBind(200);  // key_autorun
}

static void M_Bind_MouseLook (int choice)
{
    M_StartBind(201);  // key_mouse_look
}

static void M_Bind_NoVert (int choice)
{
    M_StartBind(202);  // key_novert
}

static void M_Bind_RestartLevel (int choice)
{
    M_StartBind(203);  // key_reloadlevel
}

static void M_Bind_NextLevel (int choice)
{
    M_StartBind(204);  // key_nextlevel
}

static void M_Bind_FlipLevels (int choice)
{
    M_StartBind(205);  // key_flip_levels
}

static void M_Bind_ExtendedHUD (int choice)
{
    M_StartBind(206);  // key_widget_enable
}

static void M_Bind_SpectatorMode (int choice)
{
    M_StartBind(207);  // key_spectator
}

static void M_Bind_FreezeMode (int choice)
{
    M_StartBind(208);  // key_freeze
}

static void M_Bind_NotargetMode (int choice)
{
    M_StartBind(209);  // key_notarget
}

static void M_Bind_BuddhaMode (int choice)
{
    M_StartBind(210);  // key_buddha
}

static void M_Draw_ID_Keybinds_2 (void)
{
    st_fullupdate = true;
    Keybinds_Cur = 1;

    M_FillBackground();

    M_WriteTextCentered(9, "ADVANCED MOVEMENT", cr[CR_YELLOW]);

    M_DrawBindKey(0, 18, key_autorun);
    M_DrawBindKey(1, 27, key_mouse_look);
    M_DrawBindKey(2, 36, key_novert);

    M_WriteTextCentered(45, "SPECIAL KEYS", cr[CR_YELLOW]);

    M_DrawBindKey(4, 54, key_reloadlevel);
    M_DrawBindKey(5, 63, key_nextlevel);
    M_DrawBindKey(6, 72, key_flip_levels);
    M_DrawBindKey(7, 81, key_widget_enable);

    M_WriteTextCentered(90, "SPECIAL MODES", cr[CR_YELLOW]);

    M_DrawBindKey(9, 99, key_spectator);
    M_DrawBindKey(10, 108, key_freeze);
    M_DrawBindKey(11, 117, key_notarget);
    M_DrawBindKey(12, 126, key_buddha);

    M_DrawBindFooter("2", true);
}

// -----------------------------------------------------------------------------
// Keybinds 3
// -----------------------------------------------------------------------------

static menuitem_t ID_Menu_Keybinds_3[]=
{
    { M_SWTC, "WEAPON 1",        M_Bind_Weapon1,    'w' },
    { M_SWTC, "WEAPON 2",        M_Bind_Weapon2,    'w' },
    { M_SWTC, "WEAPON 3",        M_Bind_Weapon3,    'w' },
    { M_SWTC, "WEAPON 4",        M_Bind_Weapon4,    'w' },
    { M_SWTC, "WEAPON 5",        M_Bind_Weapon5,    'w' },
    { M_SWTC, "WEAPON 6",        M_Bind_Weapon6,    'w' },
    { M_SWTC, "WEAPON 7",        M_Bind_Weapon7,    'w' },
    { M_SWTC, "WEAPON 8",        M_Bind_Weapon8,    'w' },
    { M_SWTC, "PREVIOUS WEAPON", M_Bind_PrevWeapon, 'p' },
    { M_SWTC, "NEXT WEAPON",     M_Bind_NextWeapon, 'n' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
};

static menu_t ID_Def_Keybinds_3 =
{
    ITEMCOUNT(ID_Menu_Keybinds_3),
    &ID_Def_Controls,
    ID_Menu_Keybinds_3,
    M_Draw_ID_Keybinds_3,
    ID_MENU_LEFTOFFSET, ID_MENU_TOPOFFSET_SML,
    0,
    true, true, true,
};

static void M_Bind_Weapon1 (int choice)
{
    M_StartBind(300);  // key_weapon1
}

static void M_Bind_Weapon2 (int choice)
{
    M_StartBind(301);  // key_weapon2
}

static void M_Bind_Weapon3 (int choice)
{
    M_StartBind(302);  // key_weapon3
}

static void M_Bind_Weapon4 (int choice)
{
    M_StartBind(303);  // key_weapon4
}

static void M_Bind_Weapon5 (int choice)
{
    M_StartBind(304);  // key_weapon5
}

static void M_Bind_Weapon6 (int choice)
{
    M_StartBind(305);  // key_weapon6
}

static void M_Bind_Weapon7 (int choice)
{
    M_StartBind(306);  // key_weapon7
}

static void M_Bind_Weapon8 (int choice)
{
    M_StartBind(307);  // key_weapon8
}

static void M_Bind_PrevWeapon (int choice)
{
    M_StartBind(308);  // key_prevweapon
}

static void M_Bind_NextWeapon (int choice)
{
    M_StartBind(309);  // key_nextweapon
}

static void M_Draw_ID_Keybinds_3 (void)
{
    st_fullupdate = true;
    Keybinds_Cur = 2;

    M_FillBackground();

    M_WriteTextCentered(9, "WEAPONS", cr[CR_YELLOW]);

    M_DrawBindKey(0, 18, key_weapon1);
    M_DrawBindKey(1, 27, key_weapon2);
    M_DrawBindKey(2, 36, key_weapon3);
    M_DrawBindKey(3, 45, key_weapon4);
    M_DrawBindKey(4, 54, key_weapon5);
    M_DrawBindKey(5, 63, key_weapon6);
    M_DrawBindKey(6, 72, key_weapon7);
    M_DrawBindKey(7, 81, key_weapon8);
    M_DrawBindKey(8, 90, key_prevweapon);
    M_DrawBindKey(9, 99, key_nextweapon);

    M_DrawBindFooter("3", true);
}

// -----------------------------------------------------------------------------
// Keybinds 4
// -----------------------------------------------------------------------------

static menuitem_t ID_Menu_Keybinds_4[]=
{
    { M_SWTC, "TOGGLE MAP",       M_Bind_ToggleMap,   't' },
    { M_SWTC, "ZOOM IN",          M_Bind_ZoomIn,      'z' },
    { M_SWTC, "ZOOM OUT",         M_Bind_ZoomOut,     'z' },
    { M_SWTC, "MAXIMUM ZOOM OUT", M_Bind_MaxZoom,     'm' },
    { M_SWTC, "FOLLOW MODE",      M_Bind_FollowMode,  'f' },
    { M_SWTC, "ROTATE MODE",      M_Bind_RotateMode,  'r' },
    { M_SWTC, "OVERLAY MODE",     M_Bind_OverlayMode, 'o' },
    { M_SWTC, "TOGGLE GRID",      M_Bind_ToggleGrid,  't' },
    { M_SWTC, "MARK LOCATION",    M_Bind_AddMark,     'm' },
    { M_SWTC, "CLEAR ALL MARKS",  M_Bind_ClearMarks,  'c' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
};

static menu_t ID_Def_Keybinds_4 =
{
    ITEMCOUNT(ID_Menu_Keybinds_4),
    &ID_Def_Controls,
    ID_Menu_Keybinds_4,
    M_Draw_ID_Keybinds_4,
    ID_MENU_LEFTOFFSET, ID_MENU_TOPOFFSET_SML,
    0,
    true, true, true,
};

static void M_Bind_ToggleMap (int choice)
{
    M_StartBind(400);  // key_map_toggle
}

static void M_Bind_ZoomIn (int choice)
{
    M_StartBind(401);  // key_map_zoomin
}

static void M_Bind_ZoomOut (int choice)
{
    M_StartBind(402);  // key_map_zoomout
}

static void M_Bind_MaxZoom (int choice)
{
    M_StartBind(403);  // key_map_maxzoom
}

static void M_Bind_FollowMode (int choice)
{
    M_StartBind(404);  // key_map_follow
}

static void M_Bind_RotateMode (int choice)
{
    M_StartBind(405);  // key_map_rotate
}

static void M_Bind_OverlayMode (int choice)
{
    M_StartBind(406);  // key_map_overlay
}

static void M_Bind_ToggleGrid (int choice)
{
    M_StartBind(407);  // key_map_grid
}

static void M_Bind_AddMark (int choice)
{
    M_StartBind(408);  // key_map_mark
}

static void M_Bind_ClearMarks (int choice)
{
    M_StartBind(409);  // key_map_clearmark
}

static void M_Draw_ID_Keybinds_4 (void)
{
    st_fullupdate = true;
    Keybinds_Cur = 3;

    M_FillBackground();

    M_WriteTextCentered(9, "AUTOMAP", cr[CR_YELLOW]);

    M_DrawBindKey(0, 18, key_map_toggle);
    M_DrawBindKey(1, 27, key_map_zoomin);
    M_DrawBindKey(2, 36, key_map_zoomout);
    M_DrawBindKey(3, 45, key_map_maxzoom);
    M_DrawBindKey(4, 54, key_map_follow);
    M_DrawBindKey(5, 63, key_map_rotate);
    M_DrawBindKey(6, 72, key_map_overlay);
    M_DrawBindKey(7, 81, key_map_grid);
    M_DrawBindKey(8, 90, key_map_mark);
    M_DrawBindKey(9, 99, key_map_clearmark);

    M_DrawBindFooter("4", true);
}

// -----------------------------------------------------------------------------
// Keybinds 5
// -----------------------------------------------------------------------------

static menuitem_t ID_Menu_Keybinds_5[]=
{
    { M_SWTC, "HELP SCREEN",     M_Bind_HelpScreen,     'h' },
    { M_SWTC, "SAVE GAME",       M_Bind_SaveGame,       's' },
    { M_SWTC, "LOAD GAME",       M_Bind_LoadGame,       'l' },
    { M_SWTC, "SOUND VOLUME",    M_Bind_SoundVolume,    's' },
    { M_SWTC, "TOGGLE DETAIL",   M_Bind_ToggleDetail,   't' },
    { M_SWTC, "QUICK SAVE",      M_Bind_QuickSave,      'q' },
    { M_SWTC, "END GAME",        M_Bind_EndGame,        'e' },
    { M_SWTC, "TOGGLE MESSAGES", M_Bind_ToggleMessages, 't' },
    { M_SWTC, "QUICK LOAD",      M_Bind_QuickLoad,      'q' },
    { M_SWTC, "QUIT GAME",       M_Bind_QuitGame,       'q' },
    { M_SWTC, "TOGGLE GAMMA",    M_Bind_ToggleGamma,    't' },
    { M_SWTC, "TOGGLE PALETTE",  M_Bind_TogglePalette,  't' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
};

static menu_t ID_Def_Keybinds_5 =
{
    ITEMCOUNT(ID_Menu_Keybinds_5),
    &ID_Def_Controls,
    ID_Menu_Keybinds_5,
    M_Draw_ID_Keybinds_5,
    ID_MENU_LEFTOFFSET, ID_MENU_TOPOFFSET_SML,
    0,
    true, true, true,
};

static void M_Bind_HelpScreen (int choice)
{
    M_StartBind(500);  // key_menu_help
}

static void M_Bind_SaveGame (int choice)
{
    M_StartBind(501);  // key_menu_save
}

static void M_Bind_LoadGame (int choice)
{
    M_StartBind(502);  // key_menu_load
}

static void M_Bind_SoundVolume (int choice)
{
    M_StartBind(503);  // key_menu_volume
}

static void M_Bind_ToggleDetail (int choice)
{
    M_StartBind(504);  // key_menu_detail
}

static void M_Bind_QuickSave (int choice)
{
    M_StartBind(505);  // key_menu_qsave
}

static void M_Bind_EndGame (int choice)
{
    M_StartBind(506);  // key_menu_endgame
}

static void M_Bind_ToggleMessages (int choice)
{
    M_StartBind(507);  // key_menu_messages
}

static void M_Bind_QuickLoad (int choice)
{
    M_StartBind(508);  // key_menu_qload
}

static void M_Bind_QuitGame (int choice)
{
    M_StartBind(509);  // key_menu_quit
}

static void M_Bind_ToggleGamma (int choice)
{
    M_StartBind(510);  // key_menu_gamma
}

static void M_Bind_TogglePalette (int choice)
{
    M_StartBind(511);  // key_menu_palette
}
static void M_Draw_ID_Keybinds_5 (void)
{
    st_fullupdate = true;
    Keybinds_Cur = 4;

    M_FillBackground();

    M_WriteTextCentered(9, "FUNCTION KEYS", cr[CR_YELLOW]);

    M_DrawBindKey(0, 18, key_menu_help);
    M_DrawBindKey(1, 27, key_menu_save);
    M_DrawBindKey(2, 36, key_menu_load);
    M_DrawBindKey(3, 45, key_menu_volume);
    M_DrawBindKey(4, 54, key_menu_detail);
    M_DrawBindKey(5, 63, key_menu_qsave);
    M_DrawBindKey(6, 72, key_menu_endgame);
    M_DrawBindKey(7, 81, key_menu_messages);
    M_DrawBindKey(8, 90, key_menu_qload);
    M_DrawBindKey(9, 99, key_menu_quit);
    M_DrawBindKey(10, 108, key_menu_gamma);
    M_DrawBindKey(11, 117, key_menu_palette);

    M_DrawBindFooter("5", true);
}

// -----------------------------------------------------------------------------
// Keybinds 6
// -----------------------------------------------------------------------------

static menuitem_t ID_Menu_Keybinds_6[]=
{
    { M_SWTC, "PAUSE GAME",                M_Bind_Pause,          'p' },
    { M_SWTC, "SAVE A SCREENSHOT",         M_Bind_SaveScreenshot, 's' },
    { M_SWTC, "DISPLAY LAST MESSAGE",      M_Bind_LastMessage,    'd' },
    { M_SKIP, "", 0, '\0' },
    { M_SWTC, "RESET BINDINGS TO DEFAULT", M_Bind_Reset,          'r' },
};

static menu_t ID_Def_Keybinds_6 =
{
    ITEMCOUNT(ID_Menu_Keybinds_6),
    &ID_Def_Controls,
    ID_Menu_Keybinds_6,
    M_Draw_ID_Keybinds_6,
    ID_MENU_LEFTOFFSET, ID_MENU_TOPOFFSET_SML,
    0,
    true, true, true,
};

static void M_Bind_Pause (int choice)
{
    M_StartBind(600);  // key_pause
}

static void M_Bind_SaveScreenshot (int choice)
{
    M_StartBind(601);  // key_menu_screenshot
}

static void M_Bind_LastMessage (int choice)
{
    M_StartBind(602);  // key_message_refresh
}

static void M_Bind_ResetResponse (int key)
{
    if (key != key_menu_confirm)
    {
        return;
    }
    
    M_ResetBinds();
}

static void M_Bind_Reset (int choice)
{
    const char *resetwarning =
	    M_StringJoin("RESET KEYBOARD BINDINGS TO DEFAULT VALUES?",
                     "\n\n", PRESSYN, NULL);

    M_StartMessage(resetwarning, M_Bind_ResetResponse, true);
}

static void M_Draw_ID_Keybinds_6 (void)
{
    st_fullupdate = true;
    Keybinds_Cur = 5;

    M_FillBackground();

    M_WriteTextCentered(9, "SHORTCUT KEYS", cr[CR_YELLOW]);

    M_DrawBindKey(0, 18, key_pause);
    M_DrawBindKey(1, 27, key_menu_screenshot);
    M_DrawBindKey(2, 36, key_message_refresh);

    M_WriteTextCentered(45, "RESET", cr[CR_YELLOW]);

    M_DrawBindFooter("6", true);
}

// -----------------------------------------------------------------------------
// Mouse bindings
// -----------------------------------------------------------------------------

static menuitem_t ID_Menu_MouseBinds[]=
{
    { M_SWTC, "FIRE/ATTACK",               M_Bind_M_FireAttack,   'f' },
    { M_SWTC, "MOVE FORWARD",              M_Bind_M_MoveForward,  'm' },
    { M_SWTC, "MOVE BACKWARD",             M_Bind_M_MoveBackward, 'm' },
    { M_SWTC, "USE",                       M_Bind_M_Use,          'u' },
    { M_SWTC, "SPEED ON",                  M_Bind_M_SpeedOn,      's' },
    { M_SWTC, "STRAFE ON",                 M_Bind_M_StrafeOn,     's' },
    { M_SWTC, "STRAFE LEFT",               M_Bind_M_StrafeLeft,   's' },
    { M_SWTC, "STRAFE RIGHT",              M_Bind_M_StrafeRight,  's' },
    { M_SWTC, "PREV WEAPON",               M_Bind_M_PrevWeapon,   'p' },
    { M_SWTC, "NEXT WEAPON",               M_Bind_M_NextWeapon,   'n' },
    { M_SKIP, "", 0, '\0' },
    { M_SWTC, "RESET BINDINGS TO DEFAULT", M_Bind_M_Reset,        'r' },
};

static menu_t ID_Def_MouseBinds =
{
    ITEMCOUNT(ID_Menu_MouseBinds),
    &ID_Def_Controls,
    ID_Menu_MouseBinds,
    M_Draw_ID_MouseBinds,
    ID_MENU_LEFTOFFSET, ID_MENU_TOPOFFSET_SML,
    0,
    true, false, false,
};

static void M_Choose_ID_MouseBinds (int choice)
{
    M_SetupNextMenu (&ID_Def_MouseBinds);
}

static void M_Bind_M_FireAttack (int choice)
{
    M_StartMouseBind(1000);  // mousebfire
}

static void M_Bind_M_MoveForward (int choice)
{
    M_StartMouseBind(1001);  // mousebforward
}

static void M_Bind_M_MoveBackward (int choice)
{
    M_StartMouseBind(1002);  // mousebbackward
}

static void M_Bind_M_Use (int choice)
{
    M_StartMouseBind(1003);  // mousebuse
}

static void M_Bind_M_SpeedOn (int choice)
{
    M_StartMouseBind(1004);  // mousebspeed
}

static void M_Bind_M_StrafeOn (int choice)
{
    M_StartMouseBind(1005);  // mousebstrafe
}

static void M_Bind_M_StrafeLeft (int choice)
{
    M_StartMouseBind(1006);  // mousebstrafeleft
}

static void M_Bind_M_StrafeRight (int choice)
{
    M_StartMouseBind(1007);  // mousebstraferight
}

static void M_Bind_M_PrevWeapon (int choice)
{
    M_StartMouseBind(1008);  // mousebprevweapon
}

static void M_Bind_M_NextWeapon (int choice)
{
    M_StartMouseBind(1009);  // mousebnextweapon
}

static void M_Bind_M_ResetResponse (int key)
{
    if (key != key_menu_confirm)
    {
        return;
    }

    M_ResetMouseBinds();
}

static void M_Bind_M_Reset (int choice)
{
    const char *resetwarning =
	    M_StringJoin("RESET MOUSE BINDINGS TO DEFAULT VALUES?",
                     "\n\n", PRESSYN, NULL);

    M_StartMessage(resetwarning, M_Bind_M_ResetResponse, true);
}

static void M_Draw_ID_MouseBinds (void)
{
    st_fullupdate = true;

    M_FillBackground();

    M_WriteTextCentered(9, "MOUSE BINDINGS", cr[CR_YELLOW]);

    M_DrawBindButton(0, 18, mousebfire);
    M_DrawBindButton(1, 27, mousebforward);
    M_DrawBindButton(2, 36, mousebbackward);
    M_DrawBindButton(3, 45, mousebuse);
    M_DrawBindButton(4, 54, mousebspeed);
    M_DrawBindButton(5, 63, mousebstrafe);
    M_DrawBindButton(6, 72, mousebstrafeleft);
    M_DrawBindButton(7, 81, mousebstraferight);
    M_DrawBindButton(8, 90, mousebprevweapon);
    M_DrawBindButton(9, 99, mousebnextweapon);

    M_WriteTextCentered(108, "RESET", cr[CR_YELLOW]);

    M_DrawBindFooter(NULL, false);
}

// -----------------------------------------------------------------------------
// Widgets
// -----------------------------------------------------------------------------

static menuitem_t ID_Menu_Widgets[]=
{
    { M_MUL2, "PLACEMENT",             M_ID_Widget_Placement, 'p' },
    { M_MUL2, "ALIGNMENT",             M_ID_Widget_Alignment, 'a' },
    { M_MUL2, "KIS STATS",             M_ID_Widget_KIS,       'k' },
    { M_MUL2, "- STATS FORMAT",        M_ID_Widget_KIS_Format,'s' },
    { M_MUL2, "LEVEL TIME",            M_ID_Widget_Time,      'l' },
    { M_MUL2, "TOTAL TIME",            M_ID_Widget_TotalTime, 't' },
    { M_MUL2, "LEVEL NAME",            M_ID_Widget_LevelName, 'l' },
    { M_MUL2, "PLAYER COORDS",         M_ID_Widget_Coords,    'p' },
    { M_MUL2, "RENDER COUNTERS",       M_ID_Widget_Render,    'r' },
    { M_MUL2, "TARGET'S HEALTH",       M_ID_Widget_Health,    't' },
};

static menu_t ID_Def_Widgets =
{
    ITEMCOUNT(ID_Menu_Widgets),
    &ID_Def_Main,
    ID_Menu_Widgets,
    M_Draw_ID_Widgets,
    ID_MENU_LEFTOFFSET, ID_MENU_TOPOFFSET_SML,
    0,
    true, false, false,
};

static void M_Choose_ID_Widgets (int choice)
{
    M_SetupNextMenu (&ID_Def_Widgets);
}

static void M_Draw_ID_Widgets (void)
{
    char str[32];

    M_WriteTextCentered(9, "WIDGETS", cr[CR_YELLOW]);

    // Placement
    sprintf(str, widget_location ? "TOP" : "BOTTOM");
    M_WriteTextGlow(M_ItemRightAlign(str), 18, str,
                        cr[CR_GREEN],
                            cr[CR_GREEN_BRIGHT],
                                LINE_ALPHA(0));

    // Alignment
    sprintf(str, widget_alignment == 1 ? "STATUS BAR" :
                 widget_alignment == 2 ? "AUTO" : "LEFT");
    M_WriteTextGlow(M_ItemRightAlign(str), 27, str,
                        widget_alignment ? cr[CR_GREEN] : cr[CR_DARKRED], 
                            widget_alignment ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT], 
                                LINE_ALPHA(1));

    // K/I/S stats
    sprintf(str, widget_kis == 1 ? "ALWAYS"  :
                 widget_kis == 2 ? "AUTOMAP" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 36, str,
                        widget_kis ? cr[CR_GREEN] : cr[CR_DARKRED], 
                            widget_kis ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT], 
                                LINE_ALPHA(2));

    // Stats format
    sprintf(str, widget_kis_format == 1 ? "REMAINING" :
                 widget_kis_format == 2 ? "PERCENT" : "RATIO");
    M_WriteTextGlow(M_ItemRightAlign(str), 45, str,
                        widget_kis_format ? cr[CR_GREEN] : cr[CR_DARKRED],
                            widget_kis_format ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(3));

    // Level time
    sprintf(str, widget_time == 1 ? "ALWAYS"  :
                 widget_time == 2 ? "AUTOMAP" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 54, str,
                        widget_time ? cr[CR_GREEN] : cr[CR_DARKRED],
                            widget_time ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(4));

    // Total time
    sprintf(str, widget_totaltime == 1 ? "ALWAYS"  :
                 widget_totaltime == 2 ? "AUTOMAP" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 63, str,
                        widget_totaltime ? cr[CR_GREEN] : cr[CR_DARKRED],
                            widget_totaltime ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(5));

    // Level name
    sprintf(str, widget_levelname ? "ALWAYS" : "AUTOMAP");
    M_WriteTextGlow(M_ItemRightAlign(str), 72, str,
                        widget_levelname ? cr[CR_GREEN] : cr[CR_DARKRED],
                            widget_levelname ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(6));

    // Player coords
    sprintf(str, widget_coords == 1 ? "ALWAYS"  :
                 widget_coords == 2 ? "AUTOMAP" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 81, str,
                        widget_coords ? cr[CR_GREEN] : cr[CR_DARKRED],
                            widget_coords ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(7));

    // Rendering counters
    sprintf(str, widget_render ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 90, str,
                        widget_render ? cr[CR_GREEN] : cr[CR_DARKRED],
                            widget_render ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(8));

    // Target's health
    sprintf(str, widget_health == 1 ? "TOP" :
                 widget_health == 2 ? "TOP+NAME" :
                 widget_health == 3 ? "BOTTOM" :
                 widget_health == 4 ? "BOTTOM+NAME" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 99, str,
                        widget_health ? cr[CR_GREEN] : cr[CR_DARKRED],
                            widget_health ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(9));
}

static void M_ID_Widget_Placement (int choice)
{
    widget_location ^= 1;
}

static void M_ID_Widget_Alignment (int choice)
{
    widget_alignment = M_INT_Slider(widget_alignment, 0, 2, choice, false);
}


static void M_ID_Widget_KIS (int choice)
{
    widget_kis = M_INT_Slider(widget_kis, 0, 2, choice, false);
}

static void M_ID_Widget_KIS_Format (int choice)
{
    widget_kis_format = M_INT_Slider(widget_kis_format, 0, 2, choice, false);
}

static void M_ID_Widget_Time (int choice)
{
    widget_time = M_INT_Slider(widget_time, 0, 2, choice, false);
}

static void M_ID_Widget_TotalTime (int choice)
{
    widget_totaltime = M_INT_Slider(widget_totaltime, 0, 2, choice, false);
}

static void M_ID_Widget_LevelName (int choice)
{
    widget_levelname ^= 1;
}

static void M_ID_Widget_Coords (int choice)
{
    widget_coords = M_INT_Slider(widget_coords, 0, 2, choice, false);
}

static void M_ID_Widget_Render (int choice)
{
    widget_render ^= 1;
}

static void M_ID_Widget_Health (int choice)
{
    widget_health = M_INT_Slider(widget_health, 0, 4, choice, false);
}

// -----------------------------------------------------------------------------
// Automap
// -----------------------------------------------------------------------------

static menuitem_t ID_Menu_Automap[]=
{
    { M_MUL2, "LINE SMOOTHING",        M_ID_Automap_Smooth,   'l' },
    { M_MUL1, "LINE THICKNESS",        M_ID_Automap_Thick,    'l' },
    { M_MUL2, "SQUARE ASPECT RATIO",   M_ID_Automap_Square,   's' },
    { M_MUL2, "MARK SECRET SECTORS",   M_ID_Automap_Secrets,  'm' },
    { M_MUL2, "ROTATE MODE",           M_ID_Automap_Rotate,   'r' },
    { M_MUL2, "OVERLAY MODE",          M_ID_Automap_Overlay,  'o' },
    { M_MUL1, "OVERLAY SHADING LEVEL", M_ID_Automap_Shading,  'o' },
};

static menu_t ID_Def_Automap =
{
    ITEMCOUNT(ID_Menu_Automap),
    &ID_Def_Main,
    ID_Menu_Automap,
    M_Draw_ID_Automap,
    ID_MENU_LEFTOFFSET, ID_MENU_TOPOFFSET_SML,
    0,
    true, false, false,
};

static void M_Choose_ID_Automap (int choice)
{
    M_SetupNextMenu (&ID_Def_Automap);
}

static void M_Draw_ID_Automap (void)
{
    char str[32];
    const char *thickness[] = {
        "DEFAULT","2X","3X","4X","5X","6X","AUTO"
    };

    M_WriteTextCentered(9, "AUTOMAP", cr[CR_YELLOW]);

    // Line smoothing
    sprintf(str, automap_smooth ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 18, str,
                        automap_smooth ? cr[CR_GREEN] : cr[CR_DARKRED],
                            automap_smooth ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(0));

    // Line thickness
    sprintf(str, "%s", thickness[automap_thick]);
    M_WriteTextGlow(M_ItemRightAlign(str), 27, str,
                        automap_thick ? cr[CR_GREEN] : cr[CR_DARKRED],
                            automap_thick ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(1));

    // Square aspect ratio
    sprintf(str, automap_square ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 36, str,
                        automap_square ? cr[CR_GREEN] : cr[CR_DARKRED],
                            automap_square ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(2));

    // Mark secret sectors
    sprintf(str, automap_secrets == 1 ? "REVEALED" :
                 automap_secrets == 2 ? "ALWAYS" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 45, str,
                        automap_secrets ? cr[CR_GREEN] : cr[CR_DARKRED],
                            automap_secrets ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(3));

    // Rotate mode
    sprintf(str, automap_rotate ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 54, str,
                        automap_rotate ? cr[CR_GREEN] : cr[CR_DARKRED],
                            automap_rotate ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(4));

    // Overlay mode
    sprintf(str, automap_overlay ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 63, str,
                        automap_overlay ? cr[CR_GREEN] : cr[CR_DARKRED],
                            automap_overlay ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(5));

    // Overlay shading level
    sprintf(str,"%d", automap_shading);
    M_WriteTextGlow(M_ItemRightAlign(str), 72, str,
                        !automap_overlay ? cr[CR_DARKRED] :
                         automap_shading ==  0 ? cr[CR_RED] :
                         automap_shading == 12 ? cr[CR_YELLOW] : cr[CR_GREEN],
                            !automap_overlay ? cr[CR_RED_BRIGHT] :
                             automap_shading ==  0 ? cr[CR_RED_BRIGHT] :
                             automap_shading == 12 ? cr[CR_YELLOW_BRIGHT] : cr[CR_GREEN_BRIGHT],
                                LINE_ALPHA(6));
}

static void M_ID_Automap_Smooth (int choice)
{
    automap_smooth ^= 1;
    AM_SetdrawFline();
}

static void M_ID_Automap_Thick (int choice)
{
    automap_thick = M_INT_Slider(automap_thick, 0, 6, choice, false);
}

static void M_ID_Automap_Square (int choice)
{
    automap_square ^= 1;
}

static void M_ID_Automap_Rotate (int choice)
{
    automap_rotate ^= 1;
}

static void M_ID_Automap_Overlay (int choice)
{
    automap_overlay ^= 1;
}

static void M_ID_Automap_Shading (int choice)
{
    automap_shading = M_INT_Slider(automap_shading, 0, 12, choice, true);
}

static void M_ID_Automap_Secrets (int choice)
{
    automap_secrets = M_INT_Slider(automap_secrets, 0, 2, choice, false);
}

// -----------------------------------------------------------------------------
// Gameplay features 1
// -----------------------------------------------------------------------------

static menuitem_t ID_Menu_Gameplay_1[]=
{
    { M_MUL1, "BRIGHTMAPS",                  M_ID_Brightmaps,        'b' },
    { M_MUL2, "TRANSLUCENCY",                M_ID_Translucency,      't' },
    { M_MUL1, "FUZZ EFFECT",                 M_ID_ImprovedFuzz,      'f' },
    { M_MUL1, "COLORED LIGHTING",            M_ID_ColoredLighting,   'c' },
    { M_MUL1, "COLORED BLOOD AND CORPSES",   M_ID_ColoredBlood,      'c' },
    { M_MUL1, "LIQUIDS ANIMATION",           M_ID_SwirlingLiquids,   'l' },
    { M_MUL1, "SKY ANIMATION",               M_ID_AnimatedSky,       's' },
    { M_MUL1, "SKY DRAWING MODE",            M_ID_LinearSky,         's' },
    { M_MUL1, "RANDOMLY MIRRORED CORPSES",   M_ID_FlipCorpses,       'r' },
    { M_SKIP, "", 0, '\0' },
    { M_MUL2, "SHAPE",                       M_ID_Crosshair,         's' },
    { M_MUL2, "INDICATION",                  M_ID_CrosshairColor,    'i' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_MUL2, "", /* < SCROLL PAGES >*/      M_ScrollGameplay,       's' },
};

static menu_t ID_Def_Gameplay_1 =
{
    ITEMCOUNT(ID_Menu_Gameplay_1),
    &ID_Def_Main,
    ID_Menu_Gameplay_1,
    M_Draw_ID_Gameplay_1,
    ID_MENU_LEFTOFFSET_BIG, ID_MENU_TOPOFFSET_SML,
    0,
    true, false, true,
};

static void M_Draw_ID_Gameplay_1 (void)
{
    char str[32];
    Gameplay_Cur = 0;

    M_WriteTextCentered(9, "VISUAL", cr[CR_YELLOW]);

    // Brightmaps
    sprintf(str, vis_brightmaps ? "ON" : "DEFAULT");
    M_WriteTextGlow(M_ItemRightAlign(str), 18, str,
                        vis_brightmaps ? cr[CR_GREEN] : cr[CR_DARKRED],
                            vis_brightmaps ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(0));

    // Translucency
    sprintf(str, vis_translucency == 1 ? "ADDITIVE" :
                 vis_translucency == 2 ? "BLENDING" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 27, str,
                        vis_translucency ? cr[CR_GREEN] : cr[CR_DARKRED],
                            vis_translucency ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(1));

    // Fuzz effect
    sprintf(str, vis_improved_fuzz == 1 ? "FUZZ" :
                 vis_improved_fuzz == 2 ? "TRANSLUCENT" :
                 vis_improved_fuzz == 3 ? "GRAYSCALE" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 36, str,
                        vis_improved_fuzz ? cr[CR_GREEN] : cr[CR_DARKRED],
                            vis_improved_fuzz ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(2));

    // Colored lighting
    sprintf(str, vis_colored_lighting ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 45, str,
                        vis_colored_lighting ? cr[CR_GREEN] : cr[CR_DARKRED],
                            vis_colored_lighting ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(3));

    // Colored blood and corpses
    sprintf(str, vis_colored_blood ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 54, str,
                        vis_colored_blood ? cr[CR_GREEN] : cr[CR_DARKRED],
                            vis_colored_blood ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(4));

    // Liquids animation
    sprintf(str, vis_swirling_liquids ? "IMPROVED" : "ORIGINAL");
    M_WriteTextGlow(M_ItemRightAlign(str), 63, str,
                        vis_swirling_liquids ? cr[CR_GREEN] : cr[CR_DARKRED],
                            vis_swirling_liquids ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(5));

    // Sky animation
    sprintf(str, vis_animated_sky ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 72, str,
                        vis_animated_sky ? cr[CR_GREEN] : cr[CR_DARKRED],
                            vis_animated_sky ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(6));

    // Sky drawing mode
    sprintf(str, vis_linear_sky ? "LINEAR" : "ORIGINAL");
    M_WriteTextGlow(M_ItemRightAlign(str), 81, str,
                        vis_linear_sky ? cr[CR_GREEN] : cr[CR_DARKRED],
                            vis_linear_sky ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(7));

    // Randomly mirrored corpses
    sprintf(str, vis_flip_corpses ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 90, str,
                        vis_flip_corpses ? cr[CR_GREEN] : cr[CR_DARKRED],
                            vis_flip_corpses ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(8));

    M_WriteTextCentered(99, "CROSSHAIR", cr[CR_YELLOW]);

    // Crosshair shape
    sprintf(str, xhair_draw == 1 ? "CROSS 1" :
                 xhair_draw == 2 ? "CROSS 2" :
                 xhair_draw == 3 ? "X" :
                 xhair_draw == 4 ? "CIRCLE" :
                 xhair_draw == 5 ? "ANGLE" :
                 xhair_draw == 6 ? "TRIANGLE" :
                 xhair_draw == 7 ? "DOT" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 108, str,
                        xhair_draw ? cr[CR_GREEN] : cr[CR_DARKRED],
                            xhair_draw ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(10));

    // Crosshair indication
    sprintf(str, xhair_color == 1 ? "HEALTH" :
                 xhair_color == 2 ? "TARGET HIGHLIGHT" :
                 xhair_color == 3 ? "TARGET HIGHLIGHT+HEALTH" : "STATIC");
    M_WriteTextGlow(M_ItemRightAlign(str), 117, str,
                        xhair_color ? cr[CR_GREEN] : cr[CR_DARKRED],
                            xhair_color ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(11));

    // < Scroll pages >
    M_DrawScrollPages(ID_MENU_LEFTOFFSET_BIG, 144, 14, "1/3");
}

static void M_ID_Brightmaps (int choice)
{
    vis_brightmaps ^= 1;
}

static void M_ID_ColoredLightingHook (void)
{
    vis_colored_lighting ^= 1;
    
    if (!vis_colored_lighting)
    {
        R_FreeColoredLights();
    }
    else
    {
        R_InitColoredLightTables();
    }
    R_ExecuteSetViewSize();
}

static void M_ID_ColoredLighting (int choice)
{
    post_rendering_hook = M_ID_ColoredLightingHook;
}

static void M_ID_Translucency (int choice)
{
    vis_translucency = M_INT_Slider(vis_translucency, 0, 2, choice, false);
}

static void M_ID_ImprovedFuzz (int choice)
{
    vis_improved_fuzz = M_INT_Slider(vis_improved_fuzz, 0, 3, choice, false);
}

static void M_ID_ColoredBlood (int choice)
{
    vis_colored_blood ^= 1;
}

static void M_ID_SwirlingLiquids (int choice)
{
    vis_swirling_liquids ^= 1;
    // [JN] Re-init animation sequences.
    P_InitPicAnims();
}

static void M_ID_AnimatedSky (int choice)
{
    vis_animated_sky ^= 1;

    // [JN] Reset clouds position when toggling feature off.
    if (!vis_animated_sky)
    {
        skysmoothdelta = 0;
    }
}

static void M_ID_LinearSky (int choice)
{
    vis_linear_sky ^= 1;
}

static void M_ID_FlipCorpses (int choice)
{
    vis_flip_corpses ^= 1;
}

static void M_ID_Crosshair (int choice)
{
    xhair_draw = M_INT_Slider(xhair_draw, 0, 7, choice, false);
}

static void M_ID_CrosshairColor (int choice)
{
    xhair_color = M_INT_Slider(xhair_color, 0, 3, choice, false);
}

// -----------------------------------------------------------------------------
// Gameplay features 2
// -----------------------------------------------------------------------------

static menuitem_t ID_Menu_Gameplay_2[]=
{
    { M_MUL1, "COLORED ELEMENTS",              M_ID_ColoredSTBar,      'c' },
    { M_MUL1, "SHOW NEGATIVE HEALTH",          M_ID_NegativeHealth,    's' },
    { M_SKIP, "", 0, '\0' },
    { M_MUL1, "CORPSES SLIDING FROM LEDGES",   M_ID_Torque,            'c' },
    { M_MUL1, "ITEMS ARE TOSSED WHEN DROPPED", M_ID_TossDrop,          'i' },
    { M_MUL2, "FLOATING POWERUPS AMPLITUDE",   M_ID_FloatingPowerups,  'f' },
    { M_MUL2, "WEAPON ATTACK ALIGNMENT",       M_ID_WeaponAlignment,   'w' },
    { M_MUL2, "VERTICAL AIMING",               M_ID_VerticalAiming,    'v' },
    { M_MUL1, "IMITATE PLAYER'S BREATHING",    M_ID_Breathing,         'i' },
    { M_SKIP, "", 0, '\0' },
    { M_MUL1, "SOUND OF CRUSHED CORPSE",       M_ID_SndOfCrushedCorpse,'s' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_MUL2, "", /* < SCROLL PAGES >*/        M_ScrollGameplay,       's' },
};

static menu_t ID_Def_Gameplay_2 =
{
    ITEMCOUNT(ID_Menu_Gameplay_2),
    &ID_Def_Main,
    ID_Menu_Gameplay_2,
    M_Draw_ID_Gameplay_2,
    ID_MENU_LEFTOFFSET_BIG, ID_MENU_TOPOFFSET_SML,
    0,
    true, false, true,
};

static void M_Draw_ID_Gameplay_2 (void)
{
    char str[32];
    Gameplay_Cur = 1;

    M_WriteTextCentered(9, "STATUS BAR", cr[CR_YELLOW]);

    // Colored elements
    sprintf(str, st_colored_stbar ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 18, str,
                        st_colored_stbar ? cr[CR_GREEN] : cr[CR_DARKRED],
                            st_colored_stbar ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(0));

    // Show negative health
    sprintf(str, st_negative_health ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 27, str,
                        st_negative_health ? cr[CR_GREEN] : cr[CR_DARKRED],
                            st_negative_health ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(1));

    M_WriteTextCentered(36, "PHYSICAL", cr[CR_YELLOW]);

    // Corpses sliding from ledges
    sprintf(str, phys_torque ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 45, str,
                        phys_torque ? cr[CR_GREEN] : cr[CR_DARKRED],
                            phys_torque ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(3));

    // Items are tossed when dropped
    sprintf(str, phys_toss_drop ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 54, str,
                        phys_toss_drop ? cr[CR_GREEN] : cr[CR_DARKRED],
                            phys_toss_drop ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(4));

    // Floating powerups amplitude
    sprintf(str, phys_floating_powerups == 1 ? "LOW" :
                 phys_floating_powerups == 2 ? "MIDDLE" :
                 phys_floating_powerups == 3 ? "HIGH" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 63, str,
                        phys_floating_powerups ? cr[CR_GREEN] : cr[CR_DARKRED],
                            phys_floating_powerups ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(5));

    // Weapon attack alignment
    sprintf(str, phys_weapon_alignment == 1 ? "BOBBING" :
                 phys_weapon_alignment == 2 ? "CENTERED" : "STATIC");
    M_WriteTextGlow(M_ItemRightAlign(str), 72, str,
                        phys_weapon_alignment ? cr[CR_GREEN] : cr[CR_DARKRED],
                            phys_weapon_alignment ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(6));

    // Vertical aiming
    sprintf(str, compat_vertical_aiming == 1 ? "DIRECT" :
                 compat_vertical_aiming == 2 ? "BOTH" : "AUTOAIM");
    M_WriteTextGlow(M_ItemRightAlign(str), 81, str,
                        compat_vertical_aiming ? cr[CR_GREEN] : cr[CR_DARKRED],
                            compat_vertical_aiming ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(7));

    // Imitate player's breathing
    sprintf(str, phys_breathing ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 90, str,
                        phys_breathing ? cr[CR_GREEN] : cr[CR_DARKRED],
                            phys_breathing ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(8));

    M_WriteTextCentered(99, "AUDIBLE", cr[CR_YELLOW]);

    // Sound of crushed corpse
    sprintf(str, aud_crushed_corpse ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 108, str,
                        aud_crushed_corpse ? cr[CR_GREEN] : cr[CR_DARKRED],
                            aud_crushed_corpse ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(10));

    // < Scroll pages >
    M_DrawScrollPages(ID_MENU_LEFTOFFSET_BIG, 144, 14, "2/3");
}

static void M_ID_ColoredSTBar (int choice)
{
    st_colored_stbar ^= 1;
}

static void M_ID_NegativeHealth (int choice)
{
    st_negative_health ^= 1;
}

static void M_ID_Torque (int choice)
{
    phys_torque ^= 1;
}

static void M_ID_TossDrop(int choice)
{
    phys_toss_drop ^= 1;
}

static void M_ID_FloatingPowerups(int choice)
{
    phys_floating_powerups = M_INT_Slider(phys_floating_powerups, 0, 3, choice, false);
}

static void M_ID_WeaponAlignment (int choice)
{
    phys_weapon_alignment = M_INT_Slider(phys_weapon_alignment, 0, 2, choice, false);
}

static void M_ID_VerticalAiming (int choice)
{
    compat_vertical_aiming = M_INT_Slider(compat_vertical_aiming, 0, 2, choice, false);
}

static void M_ID_Breathing (int choice)
{
    phys_breathing ^= 1;
}

static void M_ID_SndOfCrushedCorpse (int choice)
{
    aud_crushed_corpse ^= 1;
}

// -----------------------------------------------------------------------------
// Gameplay features 3
// -----------------------------------------------------------------------------

static menuitem_t ID_Menu_Gameplay_3[]=
{
    { M_MUL2, "DEFAULT SKILL LEVEL",           M_ID_DefaulSkill,       'd' },
    { M_MUL1, "PISTOL START GAME MODE",        M_ID_PistolStart,       'p' },
    { M_MUL2, "REPORT REVEALED SECRETS",       M_ID_RevealedSecrets,   'r' },
    { M_MUL1, "FLIP LEVELS HORIZONTALLY",      M_ID_FlipLevels,        'f' },
    { M_MUL2, "ON DEATH ACTION",               M_ID_OnDeathAction,     'o' },
    { M_SKIP, "", 0, '\0' },
    { M_MUL1, "MUSIC ARRANGEMENT",             M_ID_JaguarMusic,       'm' },
    { M_MUL1, "ALERTED MONSTERS BEHAVIOUR",    M_ID_JaguarAlert,       'a' },
    { M_MUL1, "EXPLOSION RADIUS IMPACT",       M_ID_JaguarExplosion,   'e' },
    { M_MUL1, "SKY TEXTURES",                  M_ID_JaguarSkies,       's' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_SKIP, "", 0, '\0' },
    { M_MUL2, "", /* < SCROLL PAGES >*/        M_ScrollGameplay,       's' },
};

static menu_t ID_Def_Gameplay_3 =
{
    ITEMCOUNT(ID_Menu_Gameplay_3),
    &ID_Def_Main,
    ID_Menu_Gameplay_3,
    M_Draw_ID_Gameplay_3,
    ID_MENU_LEFTOFFSET_BIG, ID_MENU_TOPOFFSET_SML,
    0,
    true, false, true,
};

static void M_Draw_ID_Gameplay_3 (void)
{
    char str[32];
    const char *const DefSkillName[5] = { "IAW", "NTR", "HMP", "UV", "NM" };

    Gameplay_Cur = 2;

    M_WriteTextCentered(9, "GAMEPLAY", cr[CR_YELLOW]);

    // Default skill level pitto
    snprintf(str, sizeof(str), "%s", DefSkillName[gp_default_skill]);
    M_WriteTextGlow(M_ItemRightAlign(str), 18, str, 
                        gp_default_skill == 0 ? cr[CR_OLIVE] :
                        gp_default_skill == 1 ? cr[CR_DARKGREEN] :
                        gp_default_skill == 2 ? cr[CR_GREEN] :
                        gp_default_skill == 3 ? cr[CR_YELLOW] :
                        gp_default_skill == 4 ? cr[CR_ORANGE] : cr[CR_RED],
                            gp_default_skill == 0 ? cr[CR_OLIVE_BRIGHT] :
                            gp_default_skill == 1 ? cr[CR_DARKGREEN_BRIGHT] :
                            gp_default_skill == 2 ? cr[CR_GREEN_BRIGHT] :
                            gp_default_skill == 3 ? cr[CR_YELLOW_BRIGHT] :
                            gp_default_skill == 4 ? cr[CR_ORANGE_BRIGHT] : cr[CR_RED_BRIGHT],    
                                LINE_ALPHA(0));

    // Pistol start game mode
    sprintf(str, gp_pistol_start ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 27, str,
                        gp_pistol_start ? cr[CR_GREEN] : cr[CR_DARKRED],
                            gp_pistol_start ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(1));

    // Report revealed secrets
    sprintf(str, gp_revealed_secrets == 1 ? "TOP" :
                 gp_revealed_secrets == 2 ? "CENTERED" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 36, str,
                        gp_revealed_secrets ? cr[CR_GREEN] : cr[CR_DARKRED],
                            gp_revealed_secrets ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(2));

    // Flip levels horizontally
    sprintf(str, gp_flip_levels ? "ON" : "OFF");
    M_WriteTextGlow(M_ItemRightAlign(str), 45, str,
                        gp_flip_levels ? cr[CR_GREEN] : cr[CR_DARKRED],
                            gp_flip_levels ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(3));

    // On death action
    sprintf(str, gp_death_use_action == 1 ? "LAST SAVE" :
                 gp_death_use_action == 2 ? "NOTHING" : "DEFAULT");
    M_WriteTextGlow(M_ItemRightAlign(str), 54, str,
                        gp_death_use_action ? cr[CR_GREEN] : cr[CR_DARKRED],
                            gp_death_use_action ? cr[CR_GREEN_BRIGHT] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(4));

    M_WriteTextCentered(63, "EMULATION ACCURACY", cr[CR_YELLOW]);

    // Music arrangement
    sprintf(str, emu_jaguar_music ? "JAGUAR" : "PC");
    M_WriteTextGlow(M_ItemRightAlign(str), 72, str,
                        emu_jaguar_music ? cr[CR_DARKRED] : cr[CR_GREEN],
                            emu_jaguar_music ? cr[CR_RED_BRIGHT] : cr[CR_GREEN_BRIGHT],
                                LINE_ALPHA(6));

    // Alerted monsters behaviour
    sprintf(str, emu_jaguar_alert ? "JAGUAR" : "PC");
    M_WriteTextGlow(M_ItemRightAlign(str), 81, str,
                        emu_jaguar_alert ? cr[CR_DARKRED] : cr[CR_GREEN],
                            emu_jaguar_alert ? cr[CR_RED_BRIGHT] : cr[CR_GREEN_BRIGHT],
                                LINE_ALPHA(7));

    // Explosion radius impact
    sprintf(str, emu_jaguar_explosion ? "JAGUAR" : "PC");
    M_WriteTextGlow(M_ItemRightAlign(str), 90, str,
                        emu_jaguar_explosion ? cr[CR_DARKRED] : cr[CR_GREEN],
                            emu_jaguar_explosion ? cr[CR_RED_BRIGHT] : cr[CR_GREEN_BRIGHT],
                                LINE_ALPHA(8));

    // Sky textures
    sprintf(str, emu_jaguar_skies ? "JAGUAR" : "PC");
    M_WriteTextGlow(M_ItemRightAlign(str), 99, str,
                        emu_jaguar_skies ? cr[CR_DARKRED] : cr[CR_GREEN],
                            emu_jaguar_skies ? cr[CR_RED_BRIGHT] : cr[CR_GREEN_BRIGHT],
                                LINE_ALPHA(9));

    // Print explanations of emulation accuracy features
    if (itemOn == 6 && emu_jaguar_music)
    {
        M_WriteTextCentered(117, "PLAY MUSIC ONLY ON TITLE,", cr[CR_GRAY]);
        M_WriteTextCentered(126, "INTERMISSION AND FINALE SCREENS", cr[CR_GRAY]);
    }
    if (itemOn == 7 && emu_jaguar_alert)
    {
        M_WriteTextCentered(117, "MONSTERS DON'T ATTACK", cr[CR_GRAY]);
        M_WriteTextCentered(126, "UNTIL PLAYER IS SIGHTED", cr[CR_GRAY]);
    }
    if (itemOn == 8 && emu_jaguar_explosion)
    {
        M_WriteTextCentered(117, "SOLID WALLS DON'T BLOCK", cr[CR_GRAY]);
        M_WriteTextCentered(126, "EXPLOSION RADIUS DAMAGE", cr[CR_GRAY]);
    }
    if (itemOn == 9 && emu_jaguar_skies)
    {
        M_WriteTextCentered(117, "AREA 17 USES DEIMOS SKY", cr[CR_GRAY]);
        M_WriteTextCentered(126, "AREA 24 USES HELLISH SKY", cr[CR_GRAY]);
    }

    // < Scroll pages >
    M_DrawScrollPages(ID_MENU_LEFTOFFSET_BIG, 144, 14, "3/3");
}

static void M_ID_DefaulSkill (int choice)
{
    gp_default_skill = M_INT_Slider(gp_default_skill, 0, 4, choice, false);
    // [JN] Set new skill in skill level menu.
    NewDef.lastOn = gp_default_skill;
}

static void M_ID_PistolStart (int choice)
{
    gp_pistol_start ^= 1;
}

static void M_ID_RevealedSecrets (int choice)
{
    gp_revealed_secrets = M_INT_Slider(gp_revealed_secrets, 0, 2, choice, false);
}

static void M_ID_FlipLevels (int choice)
{
    gp_flip_levels ^= 1;

    // Redraw game screen
    R_ExecuteSetViewSize();

    // Update stereo separation
    S_UpdateStereoSeparation();
}

static void M_ID_OnDeathAction (int choice)
{
    gp_death_use_action = M_INT_Slider(gp_death_use_action, 0, 2, choice, false);
}

static void M_ID_JaguarMusic (int choice)
{
    emu_jaguar_music ^= 1;

    // Shut down music
    S_StopMusic();

    // Restart music
    S_ChangeMusic(current_mus_num, usergame);
}

static void M_ID_JaguarAlert (int choice)
{
    emu_jaguar_alert ^= 1;
}

static void M_ID_JaguarExplosion (int choice)
{
    emu_jaguar_explosion ^= 1;
}

static void M_ID_JaguarSkies (int choice)
{
    emu_jaguar_skies ^= 1;

    // Reset Jaguar sky textures.
    G_InitSkyTextures();
    // Redefine color table for colored lighting.
    P_SetSectorColorTable(24);
    // Refill colored lighting values.
    P_ReloadSectorColorTable(24);
}

static void M_ScrollGameplay (int choice)
{
    if (choice) // Scroll right
    {
             if (currentMenu == &ID_Def_Gameplay_1) { M_SetupNextMenu(&ID_Def_Gameplay_2); }
        else if (currentMenu == &ID_Def_Gameplay_2) { M_SetupNextMenu(&ID_Def_Gameplay_3); }
        else if (currentMenu == &ID_Def_Gameplay_3) { M_SetupNextMenu(&ID_Def_Gameplay_1); }
    }
    else
    {           // Scroll left
             if (currentMenu == &ID_Def_Gameplay_1) { M_SetupNextMenu(&ID_Def_Gameplay_3); }
        else if (currentMenu == &ID_Def_Gameplay_2) { M_SetupNextMenu(&ID_Def_Gameplay_1); }
        else if (currentMenu == &ID_Def_Gameplay_3) { M_SetupNextMenu(&ID_Def_Gameplay_2); }
    }
    itemOn = 14;
}

// -----------------------------------------------------------------------------
// Reset settings
// -----------------------------------------------------------------------------

static void M_ID_ApplyResetHook (void)
{
    //
    // Video options
    //

    vid_resolution = 1;
    vid_widescreen = 0;
    vid_fullscreen_exclusive = 0;
    vid_uncapped_fps = 0;
    vid_fpslimit = 0;
    vid_vsync = 1;
    vid_showfps = 0;
    vid_smooth_scaling = 0;
    // Miscellaneous
    vid_screenwipe = 1;
    // Post-processing effects
    post_supersample = 0;
    post_overglow = 0;
    post_bloom = 0;
    post_rgbdrift = 0;
    post_vhsdist = 0;
    post_vignette = 0;
    post_filmgrain = 0;
    post_motionblur = 0;
    post_dofblur = 0;

    //
    // Display options
    //

    dp_screen_size = 10;
    dp_detail_level = 0;
    dp_cry_palette = 1;
    vid_gamma = 10;
    vid_fov = 90;
    dp_menu_shading = 0;
    dp_level_brightness = 0;
    // Messages settings
    msg_show = 1;
    msg_alignment = 0;
    msg_text_shadows = 0;
    msg_local_time = 0;

    //
    // Sound options
    //

    sfxVolume = 8;
    musicVolume = 8;
    snd_sfxdevice = 3;
    snd_musicdevice = 3;
    snd_dmxoption = "";
    snd_monosfx = 0;
    snd_pitchshift = 1;
    snd_channels = 8;
    snd_mute_inactive = 0;

    //
    // Widgets and automap
    //

    widget_location = 0;
    widget_alignment = 0;
    widget_kis = 0;
    widget_kis_format = 0;
    widget_time = 0;
    widget_totaltime = 0;
    widget_levelname = 0;
    widget_coords = 0;
    widget_render = 0;
    widget_health = 0;
    // Automap
    automap_smooth = 0;
    automap_thick = 0;
    automap_square = 0;
    automap_secrets = 0;
    automap_rotate = 0;
    automap_overlay = 0;
    automap_shading = 0;

    //
    // Gameplay features
    //

    // Visual
    vis_brightmaps = 0;
    vis_translucency = 0;
    vis_improved_fuzz = 0;
    vis_colored_lighting = 0; R_FreeColoredLights();
    vis_colored_blood = 0;
    vis_swirling_liquids = 0; P_InitPicAnims();
    vis_animated_sky = 0; skysmoothdelta = 0;
    vis_linear_sky = 0;
    vis_flip_corpses = 0;

    // Crosshair
    xhair_draw = 0;
    xhair_color = 0;

    // Status bar
    st_colored_stbar = 0;
    st_negative_health = 0;

    // Physical
    phys_torque = 0;
    phys_toss_drop = 0;
    phys_floating_powerups = 0;
    phys_weapon_alignment = 2;
    phys_breathing = 0;

    // Audible
    aud_crushed_corpse = 0;

    // Gameplay
    gp_default_skill = 2;
    gp_pistol_start = 0;
    gp_revealed_secrets = 0;
    gp_flip_levels = 0;
    gp_death_use_action = 0;

    // Emulation accuracy
    emu_jaguar_music = 0;
    emu_jaguar_alert = 1;
    emu_jaguar_explosion = 1;
    emu_jaguar_skies = 1;

    // Compatibility-breaking
    compat_vertical_aiming = 0;

    // Restart graphical systems
    I_ReInitGraphics(REINIT_FRAMEBUFFERS | REINIT_TEXTURES | REINIT_ASPECTRATIO);
    R_InitLightTables();
    R_SetViewSize(dp_screen_size, dp_detail_level);
    R_ExecuteSetViewSize();
    I_ToggleVsync();
    I_SetPalette(st_palette);
    R_InitColormaps();
    R_FillBackScreen();
    ST_InitElementsBackground();
    AM_LevelInit(true);
    if (automapactive)
    {
        AM_Start();
    }
    if (vid_fullscreen)
    {
        I_UpdateExclusiveFullScreen();
    }
    G_InitSkyTextures();

    // Restart audio systems
    S_StopMusic();
    S_ChangeSFXSystem();
    I_ShutdownSound();
    I_InitSound(doom);
    I_PrecacheSounds(S_sfx, NUMSFX);
    S_SetSfxVolume(sfxVolume * 8);
    S_SetMusicVolume(musicVolume * 8);
    S_ChangeMusic(current_mus_num, usergame);
    S_UpdateStereoSeparation();

    // [crispy] weapon sound sources
    for (int i = 0 ; i < MAXPLAYERS ; i++)
    {
        if (playeringame[i])
        {
            players[i].so = Crispy_PlayerSO(i);
        }
    }    
}

static void M_ID_ApplyReset (int key)
{
    if (key != key_menu_confirm)
    {
        return;
    }

    post_rendering_hook = M_ID_ApplyResetHook;
    // [JN] Arm "Defaults Restored" plaque timer for 1.5 seconds after reset.
    resetplaque_tics = 1.5 * TICRATE;
}

static void M_Choose_ID_Reset (int choice)
{
	M_StartMessage(ID_RESET, M_ID_ApplyReset, true);
}


// =============================================================================


//
// M_ReadSaveStrings
//  read the strings from the savegame files
//
static void M_ReadSaveStrings(void)
{
    FILE   *handle;
    int     i;
    char    name[256];

    for (i = 0;i < load_end;i++)
    {
        int retval;
        M_StringCopy(name, P_SaveGameFile(i), sizeof(name));

	handle = M_fopen(name, "rb");
        if (handle == NULL)
        {
            M_StringCopy(savegamestrings[i], EMPTYSTRING, SAVESTRINGSIZE);
            LoadMenu[i].status = 0;
            continue;
        }
        retval = fread(&savegamestrings[i], 1, SAVESTRINGSIZE, handle);
	fclose(handle);
        LoadMenu[i].status = retval == SAVESTRINGSIZE;
    }
}


// [FG] support up to 8 pages of savegames
static void M_DrawSaveLoadBottomLine (void)
{
/*
    char pagestr[16];

    if (savepage > 0)
    {
        M_WriteText(LoadDef.x, 151, "< PGUP", cr[CR_MENU_DARK2]);
    }
    if (savepage < savepage_max)
    {
        M_WriteText(LoadDef.x+(SAVESTRINGSIZE-6)*8, 151, "PGDN >", cr[CR_MENU_DARK2]);
    }

    M_snprintf(pagestr, sizeof(pagestr), "PAGE %d/%d", savepage + 1, savepage_max + 1);
    
    M_WriteTextCentered(151, pagestr, cr[CR_MENU_DARK1]);
*/

    // [JN] Print "modified" (or created initially) time of savegame file.
    if (LoadMenu[itemOn].status)
    {
        struct stat filestat;
        char filedate[32];

        if (M_stat(P_SaveGameFile(itemOn), &filestat) == 0)
        {
// [FG] suppress the most useless compiler warning ever
#if defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wformat-y2k"
#endif
            strftime(filedate, sizeof(filedate), "%x %X", localtime(&filestat.st_mtime));
#if defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif
            M_WriteTextCentered(152, filedate, cr[CR_MENU_DARK2]);
        }
    }
}


//
// M_LoadGame & Cie.
//
static void M_DrawLoad(void)
{
    int i;

	M_WriteTextBigCentered(8, "Load game", NULL);

	for (i = 0 ; i < load_end ; i++)
	{
        // [JN] Highlight selected item (itemOn == i) or apply fading effect.
        dp_translation = (itemOn == i) ? cr[CR_MENU_BRIGHT2] : NULL;
        M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i+7);
        dp_translation = false;

        M_WriteTextGlow(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i],
                            NULL,
                                (itemOn == i) ? cr[CR_MENU_BRIGHT5] : NULL, 
                                    LINE_ALPHA(i));
	}

    M_DrawSaveLoadBottomLine();
}



//
// Draw border for the savegame description
//
static void M_DrawSaveLoadBorder(int x,int y)
{
    int             i;
	
    V_DrawShadowedPatchOptional(x - 8, y, W_CacheLumpName("M_LSLEFT", PU_CACHE));
	
    for (i = 0;i < 24;i++)
    {
	V_DrawShadowedPatchOptional(x, y, W_CacheLumpName("M_LSCNTR", PU_CACHE));
	x += 8;
    }

    V_DrawShadowedPatchOptional(x, y, W_CacheLumpName("M_LSRGHT", PU_CACHE));
}



//
// User wants to load this game
//
static void M_LoadSelect(int choice)
{
    char    name[256];
	
    M_StringCopy(name, P_SaveGameFile(choice), sizeof(name));

    G_LoadGame (name);
    M_ClearMenus ();

    // [crispy] allow quickload before quicksave
    if (quickSaveSlot == -2)
	quickSaveSlot = choice;
}

//
// Selected from DOOM menu
//
static void M_LoadGame (int choice)
{
    M_SetupNextMenu(&LoadDef);
    M_ReadSaveStrings();
}


//
//  M_SaveGame & Cie.
//
static void M_DrawSave(void)
{
	int i;

	M_WriteTextBigCentered(8, "Save game", NULL);

	for (i = 0 ; i < load_end ; i++)
	{
        // [JN] Highlight selected item (itemOn == i) or apply fading effect.
        dp_translation = (itemOn == i) ? cr[CR_MENU_BRIGHT2] : NULL;
        M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i+7);
        dp_translation = false;

        M_WriteTextGlow(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i],
                            NULL,
                                (itemOn == i) ? cr[CR_MENU_BRIGHT5] : NULL, 
                                    LINE_ALPHA(i));
	}

	if (saveStringEnter)
	{
		i = M_StringWidth(savegamestrings[saveSlot]);
		// [JN] Highlight "_" cursor, line is always active while typing.
		M_WriteText(LoadDef.x + i,LoadDef.y+LINEHEIGHT*saveSlot,"_", cr[CR_MENU_BRIGHT5]);
		// [JN] Forcefully hide the mouse cursor while typing.
		menu_mouse_allow = false;
	}

    M_DrawSaveLoadBottomLine();
}

//
// M_Responder calls this when user is finished
//
static void M_DoSave(int slot)
{
    G_SaveGame (slot,savegamestrings[slot]);
    M_ClearMenus ();

    // PICK QUICKSAVE SLOT YET?
    if (quickSaveSlot == -2)
	quickSaveSlot = slot;
}

//
// Generate a default save slot name when the user saves to
// an empty slot via the joypad.
//
static void SetDefaultSaveName(int slot)
{
    // [JN] Generate save name.
    M_snprintf(savegamestrings[itemOn], SAVESTRINGSIZE - 1, "AREA %d", gamemap);

    M_ForceUppercase(savegamestrings[itemOn]);
    joypadSave = false;
}

// [crispy] override savegame name if it already starts with a map identifier
static boolean StartsWithMapIdentifier (char *str)
{
    M_ForceUppercase(str);

    if (strlen(str) >= 6 &&
        str[0] == 'A' && str[1] == 'R' && str[2] == 'E' && str[3] == 'A' &&
        str[4] == ' ' && isdigit(str[5]))
    {
        return true;
    }

    return false;
}

//
// User wants to save. Start string input for M_Responder
//
static void M_SaveSelect(int choice)
{
    int x, y;

    // we are going to be intercepting all chars
    saveStringEnter = 1;

    // [crispy] load the last game you saved
    LoadDef.lastOn = choice;

    // We need to turn on text input:
    x = LoadDef.x - 11;
    y = LoadDef.y + choice * LINEHEIGHT - 4;
    I_StartTextInput(x, y, x + 8 + 24 * 8 + 8, y + LINEHEIGHT - 2);

    saveSlot = choice;
    M_StringCopy(saveOldString,savegamestrings[choice], SAVESTRINGSIZE);
    if (!strcmp(savegamestrings[choice], EMPTYSTRING) ||
        // [crispy] override savegame name if it already starts with a map identifier
        StartsWithMapIdentifier(savegamestrings[choice]))
    {
        savegamestrings[choice][0] = 0;

        if (joypadSave || true) // [crispy] always prefill empty savegame slot names
        {
            SetDefaultSaveName(choice);
        }
    }
    saveCharIndex = strlen(savegamestrings[choice]);
}

//
// Selected from DOOM menu
//
static void M_SaveGame (int choice)
{
    if (!usergame)
    {
	M_StartMessage(SAVEDEAD,NULL,false);
	return;
    }
	
    if (gamestate != GS_LEVEL)
	return;
	
    M_SetupNextMenu(&SaveDef);
    M_ReadSaveStrings();
}



//
//      M_QuickSave
//
static void M_QuickSave(void)
{
    if (!usergame)
    {
	S_StartSound(NULL,sfx_oof);
	return;
    }

    if (gamestate != GS_LEVEL)
	return;
	
    if (quickSaveSlot < 0)
    {
	M_StartControlPanel();
	M_ReadSaveStrings();
	M_SetupNextMenu(&SaveDef);
	quickSaveSlot = -2;	// means to pick a slot now
	return;
    }

	// [JN] Do not show confirmation dialog,
	// do a quick save immediately.
	M_DoSave(quickSaveSlot);
}



//
// M_QuickLoad
//
static void M_QuickLoad(void)
{
    if (quickSaveSlot < 0)
    {
	// [crispy] allow quickload before quicksave
	M_StartControlPanel();
	M_ReadSaveStrings();
	M_SetupNextMenu(&LoadDef);
	quickSaveSlot = -2;
	return;
    }

	// [JN] Do not show confirmation dialog,
	// do a quick load immediately.
	M_LoadSelect(quickSaveSlot);
}




//
// [JN] HELP Menu
//

static void M_DrawHelp (void)
{
    st_fullupdate = true;

    V_DrawPatchFullScreen(W_CacheLumpName("M_TITLE", PU_CACHE), false);
    V_DrawShadowedPatchOptional(0, 0, W_CacheLumpName("HELP", PU_CACHE));
}


//
// Change Sfx & Music volumes
//
static void M_DrawSound(void)
{
    char str[8];

	M_WriteTextBigCentered(38, "Sound Volume", NULL);

    M_DrawThermo(SoundDef.x, SoundDef.y + LINEHEIGHT * (sfx_vol + 1), 16, sfxVolume, 0);
    M_ID_HandleSliderMouseControl(86, 80, 132, &sfxVolume, false, 0, 15);
    sprintf(str,"%d", sfxVolume);
    M_WriteTextGlow(226, 83, str,
                        sfxVolume ? NULL : cr[CR_DARKRED],
                            sfxVolume ? cr[CR_MENU_BRIGHT5] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(0));

    M_DrawThermo(SoundDef.x, SoundDef.y + LINEHEIGHT * (music_vol + 1), 16, musicVolume, 2);
    M_ID_HandleSliderMouseControl(86, 112, 132, &musicVolume, false, 0, 15);
    sprintf(str,"%d", musicVolume);
    M_WriteTextGlow(226, 115, str,
                        musicVolume ? NULL : cr[CR_DARKRED],
                            musicVolume ? cr[CR_MENU_BRIGHT5] : cr[CR_RED_BRIGHT],
                                LINE_ALPHA(2));
}

static void M_SfxVol(int choice)
{
    switch(choice)
    {
      case 0:
	if (sfxVolume)
	    sfxVolume--;
	break;
      case 1:
	if (sfxVolume < 15)
	    sfxVolume++;
	break;
    }
	
    S_SetSfxVolume(sfxVolume * 8);
}

static void M_MusicVol(int choice)
{
    switch(choice)
    {
      case 0:
	if (musicVolume)
	    musicVolume--;
	break;
      case 1:
	if (musicVolume < 15)
	    musicVolume++;
	break;
    }
	
    S_SetMusicVolume(musicVolume * 8);
}




//
// M_DrawMainMenu
//
static void M_DrawMainMenu(void)
{
    V_DrawPatch(94, 2, W_CacheLumpName("M_DOOM", PU_CACHE));
}




//
// M_NewGame
//
static void M_DrawNewGame (void)
{
	M_WriteTextBigCentered(14, "New Game", NULL);
	M_WriteTextBigCentered(38, "Difficulty:", NULL);
	// [JN] Jaguar: draw "Nightmare!" as separated patches.
    // Base patch
    V_DrawShadowedPatchOptional(69, 127, W_CacheLumpName("M_NMARE", PU_CACHE));
    // Glowing overlay
    dp_translation = currentMenu->menuitems[4].tics > 0 ? cr[CR_MENU_BRIGHT3] : NULL;
    V_DrawFadePatch(69, 127, W_CacheLumpName("M_NMARE", PU_CACHE), LINE_ALPHA(4));
	dp_translation = NULL;
}

static void M_NewGame(int choice)
{
	M_SetupNextMenu(&NewDef);
}


//
//      M_Episode
//
static void M_ChooseSkill (int choice)
{
	G_DeferedInitNew(choice, 1, 1);
	M_ClearMenus();
}

//
//      Toggle messages on/off
//
static void M_ChangeMessages(int choice)
{
    msg_show ^= 1;
	
	CT_SetMessage(&players[consoleplayer], (msg_show ? MSGON : MSGOFF), true, NULL);
}


//
// M_EndGame
//
static void M_EndGameResponse(int key)
{
    if (key != key_menu_confirm && key != key_menu_forward)
	return;
		
    currentMenu->lastOn = itemOn;
    M_ClearMenus ();
    players[consoleplayer].messageTics = 1;
    players[consoleplayer].message = NULL;
    players[consoleplayer].messageCenteredTics = 1;
    players[consoleplayer].messageCentered = NULL;
    st_palette = 0;
    gamestate = GS_DEMOSCREEN;
    S_ChangeMusic(mus_intro, false);
    D_StartTitle ();
}

static void M_EndGame(int choice)
{
    if (!usergame)
    {
	S_StartSound(NULL,sfx_oof);
	return;
    }
	
    M_StartMessage(ENDGAME,M_EndGameResponse,true);
}




//
// M_ReadThis
//
static void M_FinishReadThis(int choice)
{
    M_SetupNextMenu(&MainDef);
}




//
// M_QuitDOOM
//

static void M_QuitResponse(int key)
{
    if (key != key_menu_confirm && key != key_menu_forward)
	return;

    I_Quit ();
}

static void M_QuitDOOM(int choice)
{
    snprintf(endstring, sizeof(endstring), "Are you sure want to quit?\n\n" PRESSYN);

    M_StartMessage(endstring,M_QuitResponse,true);
}





static void M_ChangeDetail(int choice)
{
    if (vid_resolution == 1)
    {
        CT_SetMessage(&players[consoleplayer], DETAILNA, false, NULL);
    }
    else
    {
        dp_detail_level ^= 1;

        R_SetViewSize (dp_screen_size, dp_detail_level);

        CT_SetMessage(&players[consoleplayer],
                      dp_detail_level ? DETAILLO : DETAILHI, false, NULL);
    }
}




static void M_SizeDisplay(int choice)
{
    switch(choice)
    {
      case 0:
	if (dp_screen_size > 3)
	{
	    dp_screen_size--;
	    // [JN] Skip wide status bar in non-wide screen mode.
	    if (!vid_widescreen)
	    {
	        if (dp_screen_size == 15)
	            dp_screen_size -= 3;
	        if (dp_screen_size == 13 || dp_screen_size == 12)
	            dp_screen_size -= 2;
	    }
	    R_SetViewSize (dp_screen_size, dp_detail_level);
	}
	break;
      case 1:
	if (dp_screen_size < 15)
	{
	    dp_screen_size++;
	    // [JN] Skip wide status bar in non-wide screen mode.
	    if (!vid_widescreen)
	    {
	        if (dp_screen_size == 13) dp_screen_size += 2;
	    }
	    R_SetViewSize (dp_screen_size, dp_detail_level);
	}
	break;
    }
}




//
//      Menu Functions
//
static void
M_DrawThermo
( int	x,
  int	y,
  int	thermWidth,
  int	thermDot,
  int	itemPos )
{
    int		xx;
    int		i;

    // [JN] Highlight active slider and gem.
    if (itemPos == itemOn)
    {
        dp_translation = cr[CR_MENU_BRIGHT2];
    }

    xx = x;
    V_DrawShadowedPatchOptional(xx, y, W_CacheLumpName("M_THERML", PU_CACHE));
    xx += 8;
    for (i=0;i<thermWidth;i++)
    {
	V_DrawShadowedPatchOptional(xx, y, W_CacheLumpName("M_THERMM", PU_CACHE));
	xx += 8;
    }
    V_DrawShadowedPatchOptional(xx, y, W_CacheLumpName("M_THERMR", PU_CACHE));

    // [crispy] do not crash anymore if value exceeds thermometer range
    if (thermDot >= thermWidth)
    {
        thermDot = thermWidth - 1;
    }

    V_DrawPatch((x + 8) + thermDot * 8, y, W_CacheLumpName("M_THERMO", PU_CACHE));

    dp_translation = NULL;
}

static void
M_StartMessage
( const char*	string,
  void		(*routine)(int),
  boolean	input )
{
    messageLastMenuActive = menuactive;
    messageToPrint = 1;
    messageString = string;
    messageRoutine = routine;
    messageNeedsInput = input;
    menuactive = true;
    return;
}

//
// Find string width from hu_font_s chars
//
int M_StringWidth(const char* string)
{
    size_t             i;
    int             w = 0;
    int             c;
	
    for (i = 0;i < strlen(string);i++)
    {
	c = toupper(string[i]) - HU_FONTSTART;
	if (c < 0 || c >= HU_FONTSIZE_S)
	    w += 4;
	else
	    w += SHORT (hu_font_s[c]->width);
    }
		
    return w;
}


//
//      Find string height from hu_font_s chars
//
static int M_StringHeight (const char *string)
{
    size_t             i;
    int             h;
    int             height = SHORT(hu_font_s[0]->height);
	
    h = height;
    for (i = 0;i < strlen(string);i++)
	if (string[i] == '\n')
	    h += height;
		
    return h;
}

// -----------------------------------------------------------------------------
// M_WriteText
// Write a string using the hu_font_s.
// -----------------------------------------------------------------------------

void M_WriteText (int x, int y, const char *string, byte *table)
{
    const char*	ch;
    int w, c, cx, cy;

    ch = string;
    cx = x;
    cy = y;

    dp_translation = table;

    while (ch)
    {
        c = *ch++;

        if (!c)
        {
            break;
        }

        if (c == '\n')
        {
            cx = x;
            cy += 12;
            continue;
        }

        c = toupper(c) - HU_FONTSTART;

        if (c < 0 || c >= HU_FONTSIZE_S)
        {
            cx += 4;
            continue;
        }

        w = SHORT (hu_font_s[c]->width);

        if (cx + w > SCREENWIDTH)
        {
            break;
        }

        V_DrawShadowedPatchOptional(cx, cy, hu_font_s[c]);
        cx+=w;
    }

    dp_translation = NULL;
}

// -----------------------------------------------------------------------------
// M_WriteTextCentered
// [JN] Write a centered string using the hu_font_s.
// -----------------------------------------------------------------------------

void M_WriteTextCentered (const int y, const char *string, byte *table)
{
    const char *ch;
    const int width = M_StringWidth(string);
    int w, c, cx, cy;

    ch = string;
    cx = ORIGWIDTH/2-width/2;
    cy = y;

    dp_translation = table;

    // find width
    while (ch)
    {
        c = *ch++;

        if (!c)
        {
            break;
        }

        c = c - HU_FONTSTART;

        if (c < 0 || c >= HU_FONTSIZE_S)
        {
            continue;
        }

        w = SHORT (hu_font_s[c]->width);
    }

    // draw it
    cx = ORIGWIDTH/2-width/2;
    ch = string;

    while (ch)
    {
        c = *ch++;

        if (!c)
        {
            break;
        }

        c = toupper(c) - HU_FONTSTART;

        if (c < 0 || c >= HU_FONTSIZE_S)
        {
            cx += 4;
            continue;
        }

        w = SHORT (hu_font_s[c]->width);

        if (cx+w > ORIGWIDTH)
        {
            break;
        }
        
        V_DrawShadowedPatchOptional(cx, cy, hu_font_s[c]);
        cx += w;
    }
    
    dp_translation = NULL;
}

// -----------------------------------------------------------------------------
// M_WriteTextGlow
// Write a string using the hu_font, with glowing effect.
// -----------------------------------------------------------------------------

void M_WriteTextGlow (int x, int y, const char *string, byte *table1, byte *table2, int alpha)
{
    const char*	ch;
    int w, c, cx, cy;

    ch = string;
    cx = x;
    cy = y;

    while (ch)
    {
        c = *ch++;

        if (!c)
        {
            break;
        }

        if (c == '\n')
        {
            cx = x;
            cy += 12;
            continue;
        }

        c = toupper(c) - HU_FONTSTART;

        if (c < 0 || c >= HU_FONTSIZE_S)
        {
            cx += 4;
            continue;
        }

        w = SHORT (hu_font_s[c]->width);

        if (cx + w > SCREENWIDTH)
        {
            break;
        }

        // Base patch
        dp_translation = table1;
        V_DrawShadowedPatchOptional(cx, cy, hu_font_s[c]);

        // Glowing overlay
        dp_translation = table2;
        if (alpha == 255)
            V_DrawPatch(cx, cy, hu_font_s[c]);
        else if (alpha > 0)
            V_DrawFadePatch(cx, cy, hu_font_s[c], alpha);

        cx+=w;
    }

    dp_translation = NULL;
}

// -----------------------------------------------------------------------------
// M_WriteTextBig
// [JN] Write a string using the hu_font_b.
// -----------------------------------------------------------------------------

void M_WriteTextBig (int x, int y, const char *string, byte *table)
{
    const char*	ch;
    int w, c, cx, cy;

    ch = string;
    cx = x;
    cy = y;

    dp_translation = table;

    while (ch)
    {
        c = *ch++;

        if (!c)
        {
            break;
        }

        if (c == '\n')
        {
            cx = x;
            cy += 12;
            continue;
        }

        c = c - HU_FONTSTART2;

        if (c < 0 || c >= HU_FONTSIZE_B)
        {
            cx += 7;
            continue;
        }

        w = SHORT (hu_font_b[c]->width);

        if (cx + w > SCREENWIDTH)
        {
            break;
        }

        V_DrawShadowedPatchOptional(cx, cy, hu_font_b[c]);

        // Place one char to another with one pixel
        cx += w-1;
    }

    dp_translation = NULL;
}

// -----------------------------------------------------------------------------
// M_WriteTextBigCentered
// [JN] Write a centered string using the hu_font_b.
// -----------------------------------------------------------------------------

void M_WriteTextBigCentered (const int y, const char *string, byte *table)
{
    const char *ch;
    int width;
    int w, c, cx, cy;

    ch = string;
	width = 0;
    cx = ORIGWIDTH/2-width/2;
    cy = y;

    dp_translation = table;

    // find width
    while (ch)
    {
        c = *ch++;

        if (!c)
        {
            break;
        }

        c = c - HU_FONTSTART2;

        if (c < 0 || c >= HU_FONTSIZE_B)
        {
			width += 7;
            continue;
        }

        w = SHORT (hu_font_b[c]->width);
		width += w;
    }

    // draw it
    cx = ORIGWIDTH/2-width/2;
    ch = string;

    while (ch)
    {
        c = *ch++;

        if (!c)
        {
            break;
        }

        c = c - HU_FONTSTART2;

        if (c < 0 || c >= HU_FONTSIZE_B)
        {
            cx += 7;
            continue;
        }

        w = SHORT (hu_font_b[c]->width);

        if (cx+w > ORIGWIDTH)
        {
            break;
        }
        
        V_DrawShadowedPatchOptional(cx, cy, hu_font_b[c]);

        // Place one char to another with one pixel
        cx += w-1;
    }
    
    dp_translation = NULL;
}

// -----------------------------------------------------------------------------
// M_WriteTextBig
// [JN] Write a string using the hu_font_b.
// -----------------------------------------------------------------------------

void M_WriteTextBigGlow (int x, int y, const char *string, byte *table1, byte *table2, int alpha)
{
    const char*	ch;
    int w, c, cx, cy;

    ch = string;
    cx = x;
    cy = y;

    while (ch)
    {
        c = *ch++;

        if (!c)
        {
            break;
        }

        if (c == '\n')
        {
            cx = x;
            cy += 12;
            continue;
        }

        c = c - HU_FONTSTART2;

        if (c < 0 || c >= HU_FONTSIZE_B)
        {
            cx += 7;
            continue;
        }

        w = SHORT (hu_font_b[c]->width);

        if (cx + w > SCREENWIDTH)
        {
            break;
        }

        // Base patch
        dp_translation = table1;
        V_DrawShadowedPatchOptional(cx, cy, hu_font_b[c]);

        // Glowing overlay
        dp_translation = table2;
        if (alpha == 255)
            V_DrawPatch(cx, cy, hu_font_b[c]);
        else if (alpha > 0)
            V_DrawFadePatch(cx, cy, hu_font_b[c], alpha);

        // Place one char to another with one pixel
        cx += w-1;
    }

    dp_translation = NULL;
}

// These keys evaluate to a "null" key in Vanilla Doom that allows weird
// jumping in the menus. Preserve this behavior for accuracy.

static boolean IsNullKey(int key)
{
    return key == KEY_PAUSE || key == KEY_CAPSLOCK
        || key == KEY_SCRLCK || key == KEY_NUMLOCK;
}

// -----------------------------------------------------------------------------
// G_ReloadLevel
// [crispy] reload current level / go to next level
// adapted from prboom-plus/src/e6y.c:369-449
// -----------------------------------------------------------------------------

static int G_ReloadLevel (void)
{
    int result = false;

    if (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION)
    {
        G_DeferedInitNew(gameskill, gameepisode, gamemap);
        result = true;
    }

    return result;
}

static int G_GotoNextLevel (void)
{
    byte jaguar_next[] = {
     2,  3,  4,  5,  6,  7,  8,  9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 1,
    };

    int changed = false;

    if (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION)
    {
        G_DeferedInitNew(gameskill, gameepisode, jaguar_next[gamemap-1]);
        changed = true;
    }

    return changed;
}


//
// CONTROL PANEL
//


//
// M_Responder
//
boolean M_Responder (event_t* ev)
{
    int             ch;
    int             key;
    int             i;
    static  int     mousewait = 0;
    static  int     mousey = 0;
    static  int     lasty = 0;
    // [JN] Disable menu left/right controls by mouse movement.
    /*
    static  int     mousex = 0;
    static  int     lastx = 0;
    */

    // In testcontrols mode, none of the function keys should do anything
    // - the only key is escape to quit.

    if (testcontrols)
    {
        if (ev->type == ev_quit
        || (ev->type == ev_keydown
        && (ev->data1 == key_menu_activate || ev->data1 == key_menu_quit)))
        {
            I_Quit();
            return true;
        }

        return false;
    }

    // "close" button pressed on window?
    if (ev->type == ev_quit)
    {
        // First click on close button = bring up quit confirm message.
        // Second click on close button = confirm quit

        if (menuactive && messageToPrint && messageRoutine == M_QuitResponse)
        {
            M_QuitResponse(key_menu_confirm);
        }
        else
        {
            S_StartSound(NULL,sfx_swtchn);
            M_QuitDOOM(0);
        }

        return true;
    }

    // key is the key pressed, ch is the actual character typed

    ch = 0;
    key = -1;

    if (ev->type == ev_joystick)
    {
        // Simulate key presses from joystick events to interact with the menu.

        if (ev->data3 < 0)
        {
            key = key_menu_up;
            joywait = I_GetTime() + 5;
        }
        else
        if (ev->data3 > 0)
        {
            key = key_menu_down;
            joywait = I_GetTime() + 5;
        }

        if (ev->data2 < 0)
        {
            key = key_menu_left;
            joywait = I_GetTime() + 2;
        }
        else
        if (ev->data2 > 0)
        {
            key = key_menu_right;
            joywait = I_GetTime() + 2;
        }

#define JOY_BUTTON_MAPPED(x) ((x) >= 0)
#define JOY_BUTTON_PRESSED(x) (JOY_BUTTON_MAPPED(x) && (ev->data1 & (1 << (x))) != 0)

        if (JOY_BUTTON_PRESSED(joybfire))
        {
            // Simulate a 'Y' keypress when Doom show a Y/N dialog with Fire button.
            if (messageToPrint && messageNeedsInput)
            {
                key = key_menu_confirm;
            }
            // Simulate pressing "Enter" when we are supplying a save slot name
            else if (saveStringEnter)
            {
                key = KEY_ENTER;
            }
            else
            {
                // if selecting a save slot via joypad, set a flag
                if (currentMenu == &SaveDef)
                {
                    joypadSave = true;
                }
                key = key_menu_forward;
            }
            joywait = I_GetTime() + 5;
        }
        if (JOY_BUTTON_PRESSED(joybuse))
        {
            // Simulate a 'N' keypress when Doom show a Y/N dialog with Use button.
            if (messageToPrint && messageNeedsInput)
            {
                key = key_menu_abort;
            }
            // If user was entering a save name, back out
            else if (saveStringEnter)
            {
                key = KEY_ESCAPE;
            }
            else
            {
                key = key_menu_back;
            }
            joywait = I_GetTime() + 5;
        }
        if (JOY_BUTTON_PRESSED(joybmenu))
        {
            key = key_menu_activate;
            joywait = I_GetTime() + 5;
        }
    }
    else
    {
        const int item_status = currentMenu->menuitems[itemOn].status;

        // [JN] Shows the mouse cursor when moved.
        {
            if (ev->data2 || ev->data3)
                menu_mouse_allow = true;
            menu_mouse_allow_click = false;
        }

        if (ev->type == ev_mouse && mousewait < I_GetTime())
        {
            // [crispy] mouse_novert disables controlling the menus with the mouse
            // [JN] Not needed, as menu is fully controllable by mouse wheel and buttons.
            /*
            if (!mouse_novert)
            {
                mousey += ev->data3;
            }
            */
            if (mousey < lasty - 30)
            {
                key = key_menu_down;
                mousewait = I_GetTime() + 5;
                mousey = lasty -= 30;
            }
            else
            if (mousey > lasty + 30)
            {
                key = key_menu_up;
                mousewait = I_GetTime() + 5;
                mousey = lasty += 30;
            }

            // [JN] Disable menu left/right controls by mouse movement.
            /*
            mousex += ev->data2;
            if (mousex < lastx - 30)
            {
                key = key_menu_left;
                mousewait = I_GetTime() + 5;
                mousex = lastx -= 30;
            }
            else if (mousex > lastx + 30)
            {
                key = key_menu_right;
                mousewait = I_GetTime() + 5;
                mousex = lastx += 30;
            }
            */

            // [JN] Handle mouse bindings before going any farther.
            // Catch only button pressing events, i.e. ev->data1.
            if (MouseIsBinding && ev->data1 && !ev->data2 && !ev->data3)
            {
                M_CheckMouseBind(SDL_mouseButton);
                M_DoMouseBind(btnToBind, SDL_mouseButton);
                btnToBind = 0;
                MouseIsBinding = false;
                mousewait = I_GetTime() + 5;
                return true;
            }

            if (ev->data1 & 1)
            {
                if (menuactive && item_status == STS_SLDR)
                {
                    // [JN] Allow repetitive on sliders to move it while mouse movement.
                    menu_mouse_allow_click = true;
                }
                else
                if (!ev->data2 && !ev->data3)   // [JN] Do not consider movement as pressing.
                {
                    if (!menuactive && !usergame)
                    {
                        M_StartControlPanel();  // [JN] Open the main menu if the game is not active.
                    }
                    else
                    if (messageToPrint && messageNeedsInput)
                    {
                        key = key_menu_confirm; // [JN] Confirm by left mouse button.
                    }
                    else
                    {
                        key = key_menu_forward;
                    }
                    mousewait = I_GetTime() + 1;
                }
            }

            if (ev->data1 & 2
            && !ev->data2 && !ev->data3)  // [JN] Do not consider movement as pressing.
            {
                if (!menuactive && !usergame)
                {
                    M_StartControlPanel();  // [JN] Open the main menu if the game is not active.
                }
                else
                if (messageToPrint && messageNeedsInput)
                {
                    key = key_menu_abort;  // [JN] Cancel by right mouse button.
                }
                else
                if (saveStringEnter)
                {
                    key = key_menu_abort;
                    saveStringEnter = 0;
                    M_ReadSaveStrings();  // [JN] Reread save strings after cancelation.
                }
                else
                {
                    key = key_menu_back;
                }
                mousewait = I_GetTime() + 1;
            }

            // [JN] Scrolls through menu item values or navigates between pages.
            if (ev->data1 & (1 << 4) && menuactive)  // Wheel down
            {
                if (itemOn == -1
                || (currentMenu->ScrollAR && !saveStringEnter && !KbdIsBinding))
                {
                    M_ScrollPages(1);
                }
                else
                if (item_status > 1)
                {
                    // Scroll menu item backward normally, or forward for STS_MUL2
                    currentMenu->menuitems[itemOn].routine(item_status != STS_MUL2 ? 0 : 1);
                    S_StartSound(NULL, sfx_stnmov);
                }
                mousewait = I_GetTime();
            }
            else
            if (ev->data1 & (1 << 3) && menuactive)  // Wheel up
            {
                if (itemOn == -1
                || (currentMenu->ScrollAR && !saveStringEnter && !KbdIsBinding))
                {
                    M_ScrollPages(0);
                }
                else
                if (item_status > 1)
                {
                    // Scroll menu item forward normally, or backward for STS_MUL2
                     currentMenu->menuitems[itemOn].routine(item_status != STS_MUL2 ? 1 : 0);
                    S_StartSound(NULL, sfx_stnmov);
                }
                mousewait = I_GetTime();
            }
        }
        else
        {
            if (ev->type == ev_keydown)
            {
                key = ev->data1;
                ch = ev->data2;
                // [JN] Hide mouse cursor by pressing a key.
                menu_mouse_allow = false;
            }
        }
    }

    if (key == -1)
        return false;

    // Save Game string input
    if (saveStringEnter)
    {
        switch(key)
        {
            case KEY_BACKSPACE:
                if (saveCharIndex > 0)
                {
                    saveCharIndex--;
                    savegamestrings[saveSlot][saveCharIndex] = 0;
                }
                break;

            case KEY_ESCAPE:
                saveStringEnter = 0;
                I_StopTextInput();
                M_StringCopy(savegamestrings[saveSlot], saveOldString, SAVESTRINGSIZE);
                break;

            case KEY_ENTER:
                saveStringEnter = 0;
                I_StopTextInput();
                if (savegamestrings[saveSlot][0])
                    M_DoSave(saveSlot);
                break;

            default:
                // Savegame name entry. This is complicated.
                // Vanilla has a bug where the shift key is ignored when entering
                // a savegame name. If vanilla_keyboard_mapping is on, we want
                // to emulate this bug by using ev->data1. But if it's turned off,
                // it implies the user doesn't care about Vanilla emulation:
                // instead, use ev->data3 which gives the fully-translated and
                // modified key input.
                if (ev->type != ev_keydown)
                {
                    break;
                }
                if (vanilla_keyboard_mapping)
                {
                    ch = ev->data1;
                }
                else
                {
                    ch = ev->data3;
                }

                ch = toupper(ch);

                if (ch != ' '
                && (ch - HU_FONTSTART < 0 || ch - HU_FONTSTART >= HU_FONTSIZE_S))
                {
                    break;
                }

                if (ch >= 32 && ch <= 127
                && saveCharIndex < SAVESTRINGSIZE - 1
                && M_StringWidth(savegamestrings[saveSlot]) < (SAVESTRINGSIZE - 2) * 8)
                {
                    savegamestrings[saveSlot][saveCharIndex++] = ch;
                    savegamestrings[saveSlot][saveCharIndex] = 0;
                }
                break;
        }
        return true;
    }

    // Take care of any messages that need input
    if (messageToPrint)
    {
        if (messageNeedsInput)
        {
             // [JN] Allow to exclusevely confirm quit game by pressing F10 again.
            if (key == key_menu_quit && messageRoutine == M_QuitResponse)
            {
                I_Quit ();
                return true;
            }

            if (key != ' ' && key != KEY_ESCAPE
            &&  key != key_menu_confirm && key != key_menu_abort
            // [JN] Allow to confirm nightmare, end game and quit by pressing Enter.
            &&  key != key_menu_forward)
            {
                return false;
            }
        }

        menuactive = messageLastMenuActive;
        messageToPrint = 0;
        if (messageRoutine)
            messageRoutine(key);

        // [JN] Do not close Save/Load menu after deleting a savegame.
        if (currentMenu != &SaveDef && currentMenu != &LoadDef
        // [JN] Do not close Options menu after pressing "N" in End Game.
        &&  currentMenu != &ID_Def_Main
        // [JN] Do not close bindings menu after keyboard/mouse binds reset.
        &&  currentMenu != &ID_Def_Keybinds_6 && currentMenu != &ID_Def_MouseBinds)
        {
            menuactive = false;
        }
        S_StartSound(NULL,sfx_swtchx);
        return true;
    }

    // [JN] Handle keyboard bindings:
    if (KbdIsBinding)
    {
        if (ev->type == ev_mouse)
        {
            // Reject mouse buttons, but keep binding active.
            return false;
        }

        if (key == KEY_ESCAPE)
        {
            // Pressing ESC will cancel binding and leave key unchanged.
            keyToBind = 0;
            KbdIsBinding = false;
            return false;
        }
        else
        {
            M_CheckBind(key);
            M_DoBind(keyToBind, key);
            keyToBind = 0;
            KbdIsBinding = false;
            return true;
        }
    }

    // [JN] Disallow keyboard pressing and stop binding
    // while mouse binding is active.
    if (MouseIsBinding)
    {
        if (ev->type != ev_mouse)
        {
            btnToBind = 0;
            MouseIsBinding = false;
            return false;
        }
    }

    if (key != 0 && key == key_menu_screenshot)
    {
        S_StartSound(NULL,sfx_itemup);    // [JN] Add audible feedback
        G_ScreenShot ();
        return true;
    }

    // F-Keys
    if (!menuactive)
    {
        if (key == key_menu_decscreen)      // Screen size down
        {
            if (automapactive)
                return false;
            M_SizeDisplay(0);
            S_StartSound(NULL, sfx_stnmov);
            return true;
        }
        else if (key == key_menu_incscreen) // Screen size up
        {
            if (automapactive)
                return false;
            M_SizeDisplay(1);
            S_StartSound(NULL, sfx_stnmov);
            return true;
        }
        else if (key == key_menu_help)     // Help key
        {
            M_StartControlPanel ();

            currentMenu = &HelpDef;
            itemOn = 0;
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_save)     // Save
        {
            M_StartControlPanel();
            S_StartSound(NULL, sfx_swtchn);
            M_SaveGame(0);
            return true;
        }
        else if (key == key_menu_load)     // Load
        {
            M_StartControlPanel();
            S_StartSound(NULL, sfx_swtchn);
            M_LoadGame(0);
            return true;
        }
        else if (key == key_menu_volume)   // Sound Volume
        {
            M_StartControlPanel ();
            currentMenu = &SoundDef;
            itemOn = currentMenu->lastOn;
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_detail)   // Detail toggle
        {
            M_ChangeDetail(0);
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_qsave)    // Quicksave
        {
            S_StartSound(NULL, sfx_swtchn);
            M_QuickSave();
            return true;
        }
        else if (key == key_menu_endgame)  // End game
        {
            S_StartSound(NULL, sfx_swtchn);
            M_EndGame(0);
            return true;
        }
        else if (key == key_menu_messages) // Toggle messages
        {
            M_ChangeMessages(0);
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_qload)    // Quickload
        {
            S_StartSound(NULL, sfx_swtchn);
            M_QuickLoad();
            return true;
        }
        else if (key == key_menu_quit)     // Quit DOOM
        {
            S_StartSound(NULL, sfx_swtchn);
            M_QuitDOOM(0);
            return true;
        }
        // [crispy] those two can be considered as shortcuts for the IDCLEV cheat
        // and should be treated as such, i.e. add "if (!netgame)"
        else if (key != 0 && key == key_reloadlevel)
        {
            if (G_ReloadLevel())
                return true;
        }
        else if (key != 0 && key == key_nextlevel)
        {
            if (G_GotoNextLevel())
                return true;
        }
    }

    // [JN] Allow to change gamma while active menu.
    if (key == key_menu_gamma)    // gamma toggle
    {
        vid_gamma = M_INT_Slider(vid_gamma, 0, MAXGAMMA-1, 1 /*right*/, false);
        CT_SetMessage(&players[consoleplayer], gammalvls[vid_gamma][0], true, NULL);
        I_SetPalette(st_palette);
        R_InitColormaps();
        R_FillBackScreen();
        st_fullupdate = true;
        return true;
    }

    // [JN] Allow to change palette while active menu.
    if (key == key_menu_palette)    // [JN] Palette toggle.
    {
        M_ID_CRYPalette(0);
        CT_SetMessage(&players[consoleplayer],
                      dp_cry_palette ? ID_CRYPAL_ON : ID_CRYPAL_OFF, true, NULL);
        return true;
    }

    // Pop-up menu?
    if (!menuactive)
    {
        if (key == key_menu_activate)
        {
            M_StartControlPanel();
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        return false;
    }

    //
    // Keys usable within menu
    //

    if (key == key_menu_down)
    {
        // [PN] Move down to the next available item
        do
        {
            itemOn = (itemOn + 1) % currentMenu->numitems;
        } while (currentMenu->menuitems[itemOn].status == STS_SKIP);

        S_StartSound(NULL, sfx_pstop);
        return true;
    }
    else if (key == key_menu_up)
    {
        // [JN] Current menu item was hidden while mouse controls,
        // so move cursor to last one menu item by pressing "up" key.
        if (itemOn == -1)
        {
            itemOn = currentMenu->numitems;
        }

        // [PN] Move back up to the previous available item
        do
        {
            itemOn = (itemOn == 0) ? currentMenu->numitems - 1 : itemOn - 1;
        } while (currentMenu->menuitems[itemOn].status == STS_SKIP);

        S_StartSound(NULL, sfx_pstop);
        return true;
    }
    else if (key == key_menu_activate)
    {
        // Deactivate menu

        currentMenu->lastOn = itemOn;
        M_ClearMenus ();
        S_StartSound(NULL, sfx_swtchx);
        return true;
    }
    else if (key == key_menu_back)
    {
        // Go back to previous menu

        currentMenu->lastOn = itemOn;
        if (currentMenu->prevMenu)
        {
            currentMenu = currentMenu->prevMenu;
            M_Reset_Line_Glow();
            itemOn = currentMenu->lastOn;
            S_StartSound(NULL, sfx_swtchn);
            // [JN] Update mouse cursor position.
            M_ID_MenuMouseControl();
        }
        // [JN] Close menu if pressed "back" in Doom main or CRL main menu.
        else
        if (currentMenu == &MainDef || currentMenu == &ID_Def_Main)
        {
            S_StartSound(NULL, sfx_swtchx);
            M_ClearMenus();
        }
        return true;
    }
    else if (key == key_menu_left)
    {
        // [JN] Go to previous-left menu by pressing Left Arrow.
        if (currentMenu->ScrollAR || itemOn == -1)
        {
            M_ScrollPages(false);
            return true;
        }

        // Slide slider left
        if (currentMenu->menuitems[itemOn].routine
        &&  currentMenu->menuitems[itemOn].status > STS_SWTC)
        {
            S_StartSound(NULL, sfx_stnmov);
            currentMenu->menuitems[itemOn].routine(0);
        }
        return true;
    }
    else if (key == key_menu_right)
    {
        // [JN] Go to next-right menu by pressing Right Arrow.
        if (currentMenu->ScrollAR || itemOn == -1)
        {
            M_ScrollPages(true);
            return true;
        }

        // Slide slider right
        if (currentMenu->menuitems[itemOn].routine
        &&  currentMenu->menuitems[itemOn].status > STS_SWTC)
        {
            S_StartSound(NULL, sfx_stnmov);
            currentMenu->menuitems[itemOn].routine(1);
        }
        return true;
    }
    else if (key == key_menu_forward && itemOn != -1)
    {
        // Activate menu item

        if (currentMenu->menuitems[itemOn].routine
        &&  currentMenu->menuitems[itemOn].status)
        {
            currentMenu->lastOn = itemOn;
            if (currentMenu->menuitems[itemOn].status == STS_MUL1
            ||  currentMenu->menuitems[itemOn].status == STS_MUL2)
            {
                currentMenu->menuitems[itemOn].routine(1);      // right arrow
                S_StartSound(NULL, sfx_stnmov);
            }
            else
            {
                currentMenu->menuitems[itemOn].routine(itemOn);
                S_StartSound(NULL, sfx_pistol);
            }
        }
        return true;
    }
    // [crispy] delete a savegame
    else if (key == key_menu_del)
    {
        if (currentMenu == &LoadDef || currentMenu == &SaveDef)
        {
            if (LoadMenu[itemOn].status)
            {
                currentMenu->lastOn = itemOn;
                M_ConfirmDeleteGame();
                return true;
            }
            else
            {
                return true;
            }
        }
        // [JN] ...or clear key bind.
        else
        if (currentMenu == &ID_Def_Keybinds_1 || currentMenu == &ID_Def_Keybinds_2
        ||  currentMenu == &ID_Def_Keybinds_3 || currentMenu == &ID_Def_Keybinds_4
        ||  currentMenu == &ID_Def_Keybinds_5 || currentMenu == &ID_Def_Keybinds_6)
        {
            M_ClearBind(itemOn);
            return true;
        }
        // [JN] ...or clear mouse bind.
        else
        if (currentMenu == &ID_Def_MouseBinds)
        {
            M_ClearMouseBind(itemOn);
            return true;
        }
    }
    // [JN] Go to previous-left menu by pressing Page Up key.
    else if (key == KEY_PGUP)
    {
        if (currentMenu->ScrollPG)
        {
            M_ScrollPages(false);
        }
    }
    // [JN] Go to next-right menu by pressing Page Down key.
    else if (key == KEY_PGDN)
    {
        if (currentMenu->ScrollPG)
        {
            M_ScrollPages(true);
        }
    }

    // Keyboard shortcut?
    // Vanilla Doom has a weird behavior where it jumps to the scroll bars
    // when the certain keys are pressed, so emulate this.

    else if (ch != 0 || IsNullKey(key))
    {
        for (i = itemOn + 1 ; i < currentMenu->numitems ; i++)
        {
            if (currentMenu->menuitems[i].alphaKey == ch)
            {
                itemOn = i;
                S_StartSound(NULL, sfx_pstop);
                return true;
            }
        }

        for (i = 0 ; i <= itemOn ; i++)
        {
            if (currentMenu->menuitems[i].alphaKey == ch)
            {
                itemOn = i;
                S_StartSound(NULL, sfx_pstop);
                return true;
            }
        }
    }

    return false;
}



//
// M_StartControlPanel
//
void M_StartControlPanel (void)
{
    // intro might call this repeatedly
    if (menuactive)
	return;
    
    menuactive = 1;
    currentMenu = &MainDef;         // JDC
    M_Reset_Line_Glow();
    itemOn = currentMenu->lastOn;   // JDC
    // [JN] Disallow menu items highlighting initially to prevent
    // cursor jumping. It will be allowed by mouse movement.
    menu_mouse_allow = false;
}

static void M_ID_MenuMouseControl (void)
{
    if (!menu_mouse_allow || KbdIsBinding || MouseIsBinding)
    {
        // [JN] Skip hovering if the cursor is disabled/hidden or a binding is active.
        return;
    }
    else
    {
        // [JN] Which line height should be used?
        const int line_height = currentMenu->smallFont ? ID_MENU_LINEHEIGHT_SMALL : LINEHEIGHT;

        // [JN] Reset current menu item, it will be set in a cycle below.
        itemOn = -1;

        // [PN] Check if the cursor is hovering over a menu item
        for (int i = 0; i < currentMenu->numitems; i++)
        {
            // [JN] Slider takes three lines.
            const int line_item = currentMenu->menuitems[i].status == STS_SLDR ? 3 : 1;

            if (menu_mouse_x >= (currentMenu->x + WIDESCREENDELTA) * vid_resolution
            &&  menu_mouse_x <= (ORIGWIDTH + WIDESCREENDELTA - currentMenu->x) * vid_resolution
            &&  menu_mouse_y >= (currentMenu->y + i * line_height) * vid_resolution
            &&  menu_mouse_y <= (currentMenu->y + (i + line_item) * line_height) * vid_resolution
            &&  currentMenu->menuitems[i].status != -1)
            {
                // [PN] Highlight the current menu item
                itemOn = i;
            }
        }
    }
}

static void M_ID_HandleSliderMouseControl (int x, int y, int width, void *value, boolean is_float, float min, float max)
{
    if (!menu_mouse_allow_click)
        return;

    // [JN/PN] Adjust slider boundaries
    const int adj_x = (x + WIDESCREENDELTA) * vid_resolution;
    const int adj_y = y * vid_resolution;
    const int adj_width = width * vid_resolution;
    const int adj_height = LINEHEIGHT * vid_resolution;

    // [PN] Check cursor position and item status
    if (menu_mouse_x < adj_x || menu_mouse_x > adj_x + adj_width
    ||  menu_mouse_y < adj_y || menu_mouse_y > adj_y + adj_height
    ||  currentMenu->menuitems[itemOn].status != STS_SLDR)
        return;

    // [PN] Calculate and update slider value
    const float normalized = (float)(menu_mouse_x - adj_x + 5) / adj_width;
    const float newValue = min + normalized * (max - min);
    if (is_float)
        *((float *)value) = newValue;
    else
        *((int *)value) = (int)newValue;

    // [JN/PN] Call related routine and reset mouse click allowance
    currentMenu->menuitems[itemOn].routine(-1);
    menu_mouse_allow_click = false;

    // Play sound
    S_StartSound(NULL, sfx_stnmov);
}

//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer (void)
{
    static short	x;
    static short	y;
    unsigned int	i;
    unsigned int	max;
    char		string[80];
    const char *name;
    int			start;

    // [JN] Shade background while active menu.
    if (menuactive)
    {
        // Temporary unshade while changing certain settings.
        if (shade_wait < I_GetTime())
        {
            M_ShadeBackground();
        }
        // Always redraw status bar background.
        st_fullupdate = true;
    }

    // Horiz. & Vertically center string and print it.
    if (messageToPrint)
    {
	start = 0;
	y = ORIGHEIGHT/2 - M_StringHeight(messageString) / 2;
	while (messageString[start] != '\0')
	{
	    int foundnewline = 0;

            for (i = 0; i < strlen(messageString + start); i++)
            {
                if (messageString[start + i] == '\n')
                {
                    M_StringCopy(string, messageString + start,
                                 sizeof(string));
                    if (i < sizeof(string))
                    {
                        string[i] = '\0';
                    }

                    foundnewline = 1;
                    start += i + 1;
                    break;
                }
            }

            if (!foundnewline)
            {
                M_StringCopy(string, messageString + start, sizeof(string));
                start += strlen(string);
            }

	    x = ORIGWIDTH/2 - M_StringWidth(string) / 2;
	    M_WriteText(x, y, string, NULL);
	    y += SHORT(hu_font_s[0]->height);
	}

	return;
    }

    if (!menuactive)
    {
	menu_mouse_allow = false;  // [JN] Disallow cursor if menu is not active.
	return;
    }

    if (currentMenu->routine)
	currentMenu->routine();         // call Draw routine
    
    // DRAW MENU
    x = currentMenu->x;
    y = currentMenu->y;
    max = currentMenu->numitems;

    if (currentMenu->smallFont)
    {
        // [JN] Draw glowing * symbol.
        if (itemOn != -1)
        M_WriteTextGlow(x - ID_MENU_CURSOR_OFFSET, y + itemOn * ID_MENU_LINEHEIGHT_SMALL, "*",
                            cr[CR_MENU_DARK4], cr[CR_MENU_BRIGHT5], (cursor_tics * 17));

        for (i = 0 ; i < max ; i++)
        {
            M_WriteTextGlow(x, y, currentMenu->menuitems[i].name, NULL, cr[CR_MENU_BRIGHT5], LINE_ALPHA(i));
            y += ID_MENU_LINEHEIGHT_SMALL;
        }
    }
    else
    {
        // DRAW SKULL
        if (itemOn != -1)
        V_DrawShadowedPatchOptional(x + SKULLXOFF, y - 5 + itemOn * LINEHEIGHT,
                                    W_CacheLumpName(skullName[whichSkull], PU_CACHE));

        for (i = 0 ; i < max ; i++)
        {
            name = currentMenu->menuitems[i].name;

            if (name[0])
            {
            M_WriteTextBigGlow(x, y, currentMenu->menuitems[i].name,
                                NULL, cr[CR_MENU_BRIGHT3], LINE_ALPHA(i));
            }
            y += LINEHEIGHT;
        }
    }

    // [JN] Draw "Defaults Restored" plaque while it's timer is active.
    if (resetplaque_tics)
    {
        V_DrawShadowedPatchOptional(116, 76, W_CacheLumpName("DEFAULTS", PU_CACHE));
    }

    // [JN] Call the menu control routine for mouse input.
    M_ID_MenuMouseControl();
}


//
// M_ClearMenus
//
static void M_ClearMenus (void)
{
    menuactive = 0;
    resetplaque_tics = 0;  // [JN] Forcefully reset "Defaults Restored" plaque timer.
    menu_mouse_allow = false;  // [JN] Hide cursor on closing menu.
}




//
// M_SetupNextMenu
//
static void M_SetupNextMenu(menu_t *menudef)
{
    currentMenu = menudef;
    M_Reset_Line_Glow();
    itemOn = currentMenu->lastOn;
    // [JN] Update mouse cursor position.
    M_ID_MenuMouseControl();
}


//
// M_Ticker
//
void M_Ticker (void)
{
    if (--skullAnimCounter <= 0)
    {
	whichSkull ^= 1;
	skullAnimCounter = 8;
    }

    // [JN] Cursor glowing animation:
    cursor_tics += cursor_direction ? -1 : 1;
    if (cursor_tics == 0 || cursor_tics == 15)
    {
        cursor_direction = !cursor_direction;
    }

    // [JN] Menu item fading effect:
    for (int i = 0 ; i < currentMenu->numitems ; i++)
    {
        currentMenu->menuitems[i].tics = (itemOn == i) ? 5 :
            (currentMenu->menuitems[i].tics > 0 ? currentMenu->menuitems[i].tics - 1 : 0);
    }
    
    // [JN] "Defaults Restored" plaque timer.
    if (resetplaque_tics)
    {
        resetplaque_tics--;
    }
}


//
// M_Init
//
void M_Init (void)
{
    currentMenu = &MainDef;
    menuactive = 0;
    itemOn = currentMenu->lastOn;
    whichSkull = 0;
    skullAnimCounter = 10;
    messageToPrint = 0;
    messageString = NULL;
    messageLastMenuActive = menuactive;
    quickSaveSlot = -1;

    // [JN] Player is always local, "console" player.
    player = &players[consoleplayer];

    // [JN] Set cursor position in skill menu to default skill level.
    NewDef.lastOn = gp_default_skill;
}

// [crispy] delete a savegame
static char *savegwarning;
static void M_ConfirmDeleteGameResponse (int key)
{
	free(savegwarning);

	if (key == key_menu_confirm)
	{
		char name[256];

		M_StringCopy(name, P_SaveGameFile(itemOn), sizeof(name));
		remove(name);

		if (itemOn == quickSaveSlot)
			quickSaveSlot = -1;

		M_ReadSaveStrings();
	}
}

void M_ConfirmDeleteGame (void)
{
	savegwarning =
	M_StringJoin("delete savegame\n\n", "\"", savegamestrings[itemOn], "\"", " ?\n\n",
	             PRESSYN, NULL);

	M_StartMessage(savegwarning, M_ConfirmDeleteGameResponse, true);
	messageToPrint = 2;
	S_StartSound(NULL,sfx_swtchn);
}


// =============================================================================
//
//                        [JN] Keyboard binding routines.
//                    Drawing, coloring, checking and binding.
//
// =============================================================================


// -----------------------------------------------------------------------------
// M_NameBind
//  [JN] Convert Doom key number into printable string.
// -----------------------------------------------------------------------------

static struct {
    int key;
    char *name;
} key_names[] = KEY_NAMES_ARRAY;

static char *M_NameBind (int itemSetOn, int key)
{
    if (itemOn == itemSetOn && KbdIsBinding)
    {
        return "?";  // Means binding now
    }
    else
    {
        for (int i = 0; (size_t)i < arrlen(key_names); ++i)
        {
            if (key_names[i].key == key)
            {
                return key_names[i].name;
            }
        }
    }
    return "---";  // Means empty
}

// -----------------------------------------------------------------------------
// M_StartBind
//  [JN] Indicate that key binding is started (KbdIsBinding), and
//  pass internal number (keyToBind) for binding a new key.
// -----------------------------------------------------------------------------

static void M_StartBind (int keynum)
{
    KbdIsBinding = true;
    keyToBind = keynum;
}

// -----------------------------------------------------------------------------
// M_CheckBind
//  [JN] Check if pressed key is already binded, clear previous bind if found.
// -----------------------------------------------------------------------------

static void M_CheckBind (int key)
{
    // Page 1
    if (key_up == key)               key_up               = 0;
    if (key_down == key)             key_down             = 0;
    if (key_left == key)             key_left             = 0;
    if (key_right == key)            key_right            = 0;
    if (key_strafeleft == key)       key_strafeleft       = 0;
    if (key_straferight == key)      key_straferight      = 0;
    if (key_speed == key)            key_speed            = 0;
    if (key_strafe == key)           key_strafe           = 0;
    if (key_180turn == key)          key_180turn          = 0;
    if (key_fire == key)             key_fire             = 0;
    if (key_use == key)              key_use              = 0;
    // Page 2
    if (key_autorun == key)          key_autorun          = 0;
    if (key_mouse_look == key)       key_mouse_look       = 0;
    if (key_novert == key)           key_novert           = 0;
    if (key_reloadlevel == key)      key_reloadlevel      = 0;
    if (key_nextlevel == key)        key_nextlevel        = 0;
    if (key_flip_levels == key)      key_flip_levels      = 0;
    if (key_widget_enable == key)    key_widget_enable    = 0;
    if (key_spectator == key)        key_spectator        = 0;
    if (key_freeze == key)           key_freeze           = 0;
    if (key_notarget == key)         key_notarget         = 0;
    if (key_buddha == key)           key_buddha           = 0;
    // Page 3
    if (key_weapon1 == key)          key_weapon1          = 0;
    if (key_weapon2 == key)          key_weapon2          = 0;
    if (key_weapon3 == key)          key_weapon3          = 0;
    if (key_weapon4 == key)          key_weapon4          = 0;
    if (key_weapon5 == key)          key_weapon5          = 0;
    if (key_weapon6 == key)          key_weapon6          = 0;
    if (key_weapon7 == key)          key_weapon7          = 0;
    if (key_weapon8 == key)          key_weapon8          = 0;
    if (key_prevweapon == key)       key_prevweapon       = 0;
    if (key_nextweapon == key)       key_nextweapon       = 0;
    // Page 4
    if (key_map_toggle == key)       key_map_toggle       = 0;
    // Do not override Automap binds in other pages.
    if (currentMenu == &ID_Def_Keybinds_4)
    {
        if (key_map_zoomin == key)     key_map_zoomin     = 0;
        if (key_map_zoomout == key)    key_map_zoomout    = 0;
        if (key_map_maxzoom == key)    key_map_maxzoom    = 0;
        if (key_map_follow == key)     key_map_follow     = 0;
        if (key_map_rotate == key)     key_map_rotate     = 0;
        if (key_map_overlay == key)    key_map_overlay    = 0;
        if (key_map_grid == key)       key_map_grid       = 0;
        if (key_map_mark == key)       key_map_mark       = 0;
        if (key_map_clearmark == key)  key_map_clearmark  = 0;
    }
    // Page 5
    if (key_menu_help == key)        key_menu_help        = 0;
    if (key_menu_save == key)        key_menu_save        = 0;
    if (key_menu_load == key)        key_menu_load        = 0;
    if (key_menu_volume == key)      key_menu_volume      = 0;
    if (key_menu_detail == key)      key_menu_detail      = 0;
    if (key_menu_qsave == key)       key_menu_qsave       = 0;
    if (key_menu_endgame == key)     key_menu_endgame     = 0;
    if (key_menu_messages == key)    key_menu_messages    = 0;
    if (key_menu_qload == key)       key_menu_qload       = 0;
    if (key_menu_quit == key)        key_menu_quit        = 0;
    if (key_menu_gamma == key)       key_menu_gamma       = 0;
    if (key_menu_palette == key)     key_menu_palette     = 0;
    // Page 6
    if (key_pause == key)            key_pause            = 0;
    if (key_menu_screenshot == key)  key_menu_screenshot  = 0;
    if (key_message_refresh == key)  key_message_refresh  = 0;
}

// -----------------------------------------------------------------------------
// M_DoBind
//  [JN] By catching internal bind number (keynum), do actual binding
//  of pressed key (key) to real keybind.
// -----------------------------------------------------------------------------

static void M_DoBind (int keynum, int key)
{
    switch (keynum)
    {
        // Page 1
        case 100:  key_up = key;                break;
        case 101:  key_down = key;              break;
        case 102:  key_left = key;              break;
        case 103:  key_right = key;             break;
        case 104:  key_strafeleft = key;        break;
        case 105:  key_straferight = key;       break;
        case 106:  key_speed = key;             break;
        case 107:  key_strafe = key;            break;
        case 108:  key_180turn = key;           break;
        case 109:  key_fire = key;              break;
        case 110:  key_use = key;               break;
        // Page 2  
        case 200:  key_autorun = key;           break;
        case 201:  key_mouse_look = key;        break;
        case 202:  key_novert = key;            break;
        case 203:  key_reloadlevel = key;       break;
        case 204:  key_nextlevel = key;         break;
        case 205:  key_flip_levels = key;       break;
        case 206:  key_widget_enable = key;     break;
        case 207:  key_spectator = key;         break;
        case 208:  key_freeze = key;            break;
        case 209:  key_notarget = key;          break;
        case 210:  key_buddha = key;            break;
        // Page 3  
        case 300:  key_weapon1 = key;           break;
        case 301:  key_weapon2 = key;           break;
        case 302:  key_weapon3 = key;           break;
        case 303:  key_weapon4 = key;           break;
        case 304:  key_weapon5 = key;           break;
        case 305:  key_weapon6 = key;           break;
        case 306:  key_weapon7 = key;           break;
        case 307:  key_weapon8 = key;           break;
        case 308:  key_prevweapon = key;        break;
        case 309:  key_nextweapon = key;        break;
        // Page 4  
        case 400:  key_map_toggle = key;        break;
        case 401:  key_map_zoomin = key;        break;
        case 402:  key_map_zoomout = key;       break;
        case 403:  key_map_maxzoom = key;       break;
        case 404:  key_map_follow = key;        break;
        case 405:  key_map_rotate = key;        break;
        case 406:  key_map_overlay = key;       break;
        case 407:  key_map_grid = key;          break;
        case 408:  key_map_mark = key;          break;
        case 409:  key_map_clearmark = key;     break;
        // Page 5  
        case 500:  key_menu_help = key;         break;
        case 501:  key_menu_save = key;         break;
        case 502:  key_menu_load = key;         break;
        case 503:  key_menu_volume = key;       break;
        case 504:  key_menu_detail = key;       break;
        case 505:  key_menu_qsave = key;        break;
        case 506:  key_menu_endgame = key;      break;
        case 507:  key_menu_messages = key;     break;
        case 508:  key_menu_qload = key;        break;
        case 509:  key_menu_quit = key;         break;
        case 510:  key_menu_gamma = key;        break;
        case 511:  key_menu_palette = key;      break;
        // Page 6  
        case 600:  key_pause = key;             break;
        case 601:  key_menu_screenshot = key;   break;
        case 602:  key_message_refresh = key;   break;
    }
}

// -----------------------------------------------------------------------------
// M_ClearBind
//  [JN] Clear key bind on the line where cursor is placed (itemOn).
// -----------------------------------------------------------------------------

static void M_ClearBind (int itemOn)
{
    if (currentMenu == &ID_Def_Keybinds_1)
    {
        switch (itemOn)
        {
            case 0:   key_up = 0;               break;
            case 1:   key_down = 0;             break;
            case 2:   key_left = 0;             break;
            case 3:   key_right = 0;            break;
            case 4:   key_strafeleft = 0;       break;
            case 5:   key_straferight = 0;      break;
            case 6:   key_speed = 0;            break;
            case 7:   key_strafe = 0;           break;
            case 8:   key_180turn = 0;          break;
            // Action title
            case 10:  key_fire = 0;             break;
            case 11:  key_use = 0;              break;
        }
    }
    if (currentMenu == &ID_Def_Keybinds_2)
    {
        switch (itemOn)
        {
            case 0:   key_autorun = 0;          break;
            case 1:   key_mouse_look = 0;       break;
            case 2:   key_novert = 0;           break;
            // Special keys title
            case 4:   key_reloadlevel = 0;      break;
            case 5:   key_nextlevel = 0;        break;
            case 6:   key_flip_levels = 0;      break;
            case 7:   key_widget_enable = 0;    break;
            // Special modes title
            case 9:   key_spectator = 0;        break;
            case 10:  key_freeze = 0;           break;
            case 11:  key_notarget = 0;         break;
            case 12:  key_buddha = 0;           break;
        }
    }
    if (currentMenu == &ID_Def_Keybinds_3)
    {
        switch (itemOn)
        {
            case 0:   key_weapon1 = 0;          break;
            case 1:   key_weapon2 = 0;          break;
            case 2:   key_weapon3 = 0;          break;
            case 3:   key_weapon4 = 0;          break;
            case 4:   key_weapon5 = 0;          break;
            case 5:   key_weapon6 = 0;          break;
            case 6:   key_weapon7 = 0;          break;
            case 7:   key_weapon8 = 0;          break;
            case 8:   key_prevweapon = 0;       break;
            case 9:   key_nextweapon = 0;       break;
        }
    }
    if (currentMenu == &ID_Def_Keybinds_4)
    {
        switch (itemOn)
        {
            case 0:   key_map_toggle = 0;       break;
            case 1:   key_map_zoomin = 0;       break;
            case 2:   key_map_zoomout = 0;      break;
            case 3:   key_map_maxzoom = 0;      break;
            case 4:   key_map_follow = 0;       break;
            case 5:   key_map_rotate = 0;       break;
            case 6:   key_map_overlay = 0;      break;
            case 7:   key_map_grid = 0;         break;
            case 8:   key_map_mark = 0;         break;
            case 9:   key_map_clearmark = 0;    break;
        }
    }
    if (currentMenu == &ID_Def_Keybinds_5)
    {
        switch (itemOn)
        {
            case 0:   key_menu_help = 0;        break;
            case 1:   key_menu_save = 0;        break;
            case 2:   key_menu_load = 0;        break;
            case 3:   key_menu_volume = 0;      break;
            case 4:   key_menu_detail = 0;      break;
            case 5:   key_menu_qsave = 0;       break;
            case 6:   key_menu_endgame = 0;     break;
            case 7:   key_menu_messages = 0;    break;
            case 8:   key_menu_qload = 0;       break;
            case 9:   key_menu_quit = 0;        break;
            case 10:  key_menu_gamma = 0;       break;
            case 11:  key_menu_palette = 0;     break;
        }
    }
    if (currentMenu == &ID_Def_Keybinds_6)
    {
        switch (itemOn)
        {
            case 0:   key_pause = 0;            break;
            case 1:   key_menu_screenshot = 0;  break;
            case 2:   key_message_refresh = 0;  break;
        }
    }
}

// -----------------------------------------------------------------------------
// M_ResetBinds
//  [JN] Reset all keyboard binding to it's defaults.
// -----------------------------------------------------------------------------

static void M_ResetBinds (void)
{
    // Page 1
    key_up = 'w';
    key_down = 's';
    key_left = KEY_LEFTARROW;
    key_right = KEY_RIGHTARROW;
    key_strafeleft = 'a';
    key_straferight = 'd';
    key_speed = KEY_RSHIFT;
    key_strafe = KEY_RALT;
    key_180turn = 0;
    key_fire = KEY_RCTRL;
    key_use = ' ';
    // Page 2
    key_autorun = KEY_CAPSLOCK;
    key_mouse_look = 0;
    key_novert = 0;
    key_reloadlevel = 0;
    key_nextlevel = 0;
    key_flip_levels = 0;
    key_widget_enable = 0;
    key_spectator = 0;
    key_freeze = 0;
    key_notarget = 0;
    key_buddha = 0;
    // Page 3
    key_weapon1 = '1';
    key_weapon2 = '2';
    key_weapon3 = '3';
    key_weapon4 = '4';
    key_weapon5 = '5';
    key_weapon6 = '6';
    key_weapon7 = '7';
    key_weapon8 = '8';
    key_prevweapon = 0;
    key_nextweapon = 0;
    // Page 4
    key_map_toggle = KEY_TAB;
    key_map_zoomin = '=';
    key_map_zoomout = '-';
    key_map_maxzoom = '0';
    key_map_follow = 'f';
    key_map_rotate = 'r';
    key_map_overlay = 'o';
    key_map_grid = 'g';
    key_map_mark = 'm';
    key_map_clearmark = 'c';
    // Page 5
    key_menu_help = KEY_F1;
    key_menu_save = KEY_F2;
    key_menu_load = KEY_F3;
    key_menu_volume = KEY_F4;
    key_menu_detail = KEY_F5;
    key_menu_qsave = KEY_F6;
    key_menu_endgame = KEY_F7;
    key_menu_messages = KEY_F8;
    key_menu_qload = KEY_F9;
    key_menu_quit = KEY_F10;
    key_menu_gamma = KEY_F11;
    key_menu_palette = KEY_F12;
    // Page 6
    key_pause = KEY_PAUSE;
    key_menu_screenshot = KEY_PRTSCR;
    key_message_refresh = KEY_ENTER;
}

// -----------------------------------------------------------------------------
// M_DrawBindKey
//  [JN] Do keyboard bind drawing.
// -----------------------------------------------------------------------------

static void M_DrawBindKey (int itemNum, int yPos, int key)
{
    M_WriteTextGlow(M_ItemRightAlign(M_NameBind(itemNum, key)), yPos, M_NameBind(itemNum, key),
                        itemOn == itemNum && KbdIsBinding ? cr[CR_YELLOW] :
                        key == 0 ? cr[CR_RED] : cr[CR_GREEN],
                            itemOn == itemNum && KbdIsBinding ? cr[CR_YELLOW_BRIGHT] :
                            key == 0 ? cr[CR_RED_BRIGHT] : cr[CR_GREEN_BRIGHT],
                                LINE_ALPHA(itemNum));
}

// -----------------------------------------------------------------------------
// M_DrawBindFooter
//  [JN] Draw footer in key binding pages with numeration.
// -----------------------------------------------------------------------------

static void M_DrawBindFooter (char *pagenum, boolean drawPages)
{
    const char *string = "PRESS ENTER TO BIND, DEL TO CLEAR";

    if (drawPages)
    {
        M_WriteTextCentered(171, string, cr[CR_MENU_DARK1]);
        M_WriteText(ID_MENU_LEFTOFFSET, 180, "< PGUP", cr[CR_MENU_DARK3]);
        M_WriteTextCentered(180, M_StringJoin("PAGE ", pagenum, "/6", NULL), cr[CR_MENU_DARK2]);
        M_WriteText(M_ItemRightAlign("PGDN >"), 180, "PGDN >", cr[CR_MENU_DARK3]);
    }
    else
    {
        M_WriteTextCentered(180, string, cr[CR_MENU_DARK1]);
    }
}


// =============================================================================
//
//                          [JN] Mouse binding routines.
//                    Drawing, coloring, checking and binding.
//
// =============================================================================


// -----------------------------------------------------------------------------
// M_NameBind
//  [JN] Draw mouse button number as printable string.
// -----------------------------------------------------------------------------


static char *M_NameMouseBind (int itemSetOn, int btn)
{
    if (itemOn == itemSetOn && MouseIsBinding)
    {
        return "?";  // Means binding now
    }
    else
    {
        char  num[8]; 
        char *other_button;

        M_snprintf(num, 8, "%d", btn + 1);
        other_button = M_StringJoin("BUTTON #", num, NULL);

        switch (btn)
        {
            case -1:  return  "---";            break;  // Means empty
            case  0:  return  "LEFT BUTTON";    break;
            case  1:  return  "RIGHT BUTTON";   break;
            case  2:  return  "MIDDLE BUTTON";  break;
            case  3:  return  "WHEEL UP";       break;
            case  4:  return  "WHEEL DOWN";     break;
            default:  return  other_button;     break;
        }
    }
}

// -----------------------------------------------------------------------------
// M_StartMouseBind
//  [JN] Indicate that mouse button binding is started (MouseIsBinding), and
//  pass internal number (btnToBind) for binding a new button.
// -----------------------------------------------------------------------------

static void M_StartMouseBind (int btn)
{
    MouseIsBinding = true;
    btnToBind = btn;
}

// -----------------------------------------------------------------------------
// M_CheckMouseBind
//  [JN] Check if pressed button is already binded, clear previous bind if found.
// -----------------------------------------------------------------------------

static void M_CheckMouseBind (int btn)
{
    if (mousebfire == btn)        mousebfire        = -1;
    if (mousebforward == btn)     mousebforward     = -1;
    if (mousebbackward == btn)    mousebbackward    = -1;
    if (mousebuse == btn)         mousebuse         = -1;
    if (mousebspeed == btn)       mousebspeed       = -1;
    if (mousebstrafe == btn)      mousebstrafe      = -1;
    if (mousebstrafeleft == btn)  mousebstrafeleft  = -1;
    if (mousebstraferight == btn) mousebstraferight = -1;
    if (mousebprevweapon == btn)  mousebprevweapon  = -1;
    if (mousebnextweapon == btn)  mousebnextweapon  = -1;
}

// -----------------------------------------------------------------------------
// M_DoMouseBind
//  [JN] By catching internal bind number (btnnum), do actual binding
//  of pressed button (btn) to real mouse bind.
// -----------------------------------------------------------------------------

static void M_DoMouseBind (int btnnum, int btn)
{
    switch (btnnum)
    {
        case 1000:  mousebfire = btn;         break;
        case 1001:  mousebforward = btn;      break;
        case 1002:  mousebbackward = btn;     break;
        case 1003:  mousebuse = btn;          break;
        case 1004:  mousebspeed = btn;        break;
        case 1005:  mousebstrafe = btn;       break;
        case 1006:  mousebstrafeleft = btn;   break;
        case 1007:  mousebstraferight = btn;  break;
        case 1008:  mousebprevweapon = btn;   break;
        case 1009:  mousebnextweapon = btn;   break;
        default:                              break;
    }
}

// -----------------------------------------------------------------------------
// M_ClearMouseBind
//  [JN] Clear mouse bind on the line where cursor is placed (itemOn).
// -----------------------------------------------------------------------------


static void M_ClearMouseBind (int itemOn)
{
    switch (itemOn)
    {
        case 0:  mousebfire = -1;         break;
        case 1:  mousebforward = -1;      break;
        case 2:  mousebbackward = -1;     break;
        case 3:  mousebuse = -1;          break;
        case 4:  mousebspeed = -1;        break;
        case 5:  mousebstrafe = -1;       break;
        case 6:  mousebstrafeleft = -1;   break;
        case 7:  mousebstraferight = -1;  break;
        case 8:  mousebprevweapon = -1;   break;
        case 9:  mousebnextweapon = -1;   break;
    }
}

// -----------------------------------------------------------------------------
// M_DrawBindButton
//  [JN] Do mouse button bind drawing.
// -----------------------------------------------------------------------------

static void M_DrawBindButton (int itemNum, int yPos, int btn)
{
    M_WriteTextGlow(M_ItemRightAlign(M_NameMouseBind(itemNum, btn)), yPos, M_NameMouseBind(itemNum, btn),
                        itemOn == itemNum && MouseIsBinding ? cr[CR_YELLOW] :
                        btn == - 1 ? cr[CR_RED] : cr[CR_GREEN],
                            itemOn == itemNum && MouseIsBinding ? cr[CR_YELLOW_BRIGHT] :
                            btn == - 1 ? cr[CR_RED_BRIGHT] : cr[CR_GREEN_BRIGHT],
                                LINE_ALPHA(itemNum));
}

// -----------------------------------------------------------------------------
// M_ResetBinds
//  [JN] Reset all mouse binding to it's defaults.
// -----------------------------------------------------------------------------

static void M_ResetMouseBinds (void)
{
    mousebfire = 0;
    mousebforward = 2;
    mousebbackward = -1;
    mousebuse = -1;
    mousebspeed = -1;
    mousebstrafe = 1;
    mousebstrafeleft = -1;
    mousebstraferight = -1;
    mousebprevweapon = 4;
    mousebnextweapon = 3;
}
