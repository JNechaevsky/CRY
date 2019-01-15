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
#include <ctype.h>

#include "doomdef.h"
#include "doomkeys.h"
#include "d_main.h"
#include "deh_main.h"
#include "i_swap.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "m_misc.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"
#include "r_local.h"
#include "hu_stuff.h"
#include "g_game.h"
#include "m_argv.h"
#include "m_controls.h"
#include "p_saveg.h"
#include "s_sound.h"
#include "doomstat.h"
#include "sounds.h"
#include "m_menu.h"
#include "d_englsh.h"
#include "jn.h"


// =============================================================================
// DEFAULT VALUES
// =============================================================================

#define SKULLXOFF   -32
#define LINEHEIGHT  16

int mouseSensitivity = 5;
int showMessages = 0;   // [Julia] Don't show any messages by default
int detailLevel = 0;    // Blocky mode, has default, 0 = high, 1 = normal
int screenblocks = 10;  // [Julia] No bezel by default

int screenSize;         // temp for screenblocks (0-9)
int quickSaveSlot;      // -1 = no quicksave slot picked!
int messageToPrint;     // 1 = message to be printed
char *messageString;    // ...and here is the message string!
int messx, messy;       // message x & y
int messageLastMenuActive;

int saveStringEnter;    // we are going to be entering a savegame string
int saveSlot;           // which slot to save in
int saveCharIndex;      // which char we're editing
char saveOldString[SAVESTRINGSIZE]; // old save description before edit
char savegamestrings[10][SAVESTRINGSIZE];
char endstring[160];

boolean messageNeedsInput;  // timed message = no input from user
boolean QuickSaveTitle;     // [Julia] For additional title "QUICK SAVING"
boolean inhelpscreens;
boolean menuactive;

extern int       st_palette;
extern patch_t  *hu_font[HU_FONTSIZE];
extern boolean   message_dontfuckwithme;
extern boolean   chat_on;   // in heads-up code
extern boolean   sendpause;

void (*messageRoutine)(int response);

char gammamsg[9][41] =
{
    GAMMA_OFF,
    GAMMA_05,
    GAMMA_1,
    GAMMA_15,
    GAMMA_2,
    GAMMA_25,
    GAMMA_3,
    GAMMA_35,
    GAMMA_4
};


// =============================================================================
// MENU TYPEDEFS
// =============================================================================

typedef struct
{
    // 0 = no cursor here, 1 = ok, 2 = arrows ok
    short   status;
    // [Julia] Increased from 10 to 20, needed for proper text sizes
    char    name[20];

    // choice = menu item #. 
    // if status = 2,
    //   choice=0:leftarrow,1:rightarrow
    void    (*routine)(int choice);

    // hotkey in menu
    char    alphaKey;			
} menuitem_t;


typedef struct menu_s
{
    short           numitems;       // # of menu items
    struct menu_s  *prevMenu;       // previous menu
    menuitem_t     *menuitems;      // menu items
    void            (*routine)();   // draw routine
    short           x;
    short           y;              // x,y of menu
    short           lastOn;         // last item user was on in menu
} menu_t;

short   itemOn;             // menu item skull is on
short   skullAnimCounter;   // skull animation counter
short   whichSkull;         // which skull to draw

// graphic name of skulls
// warning: initializer-string for array of chars is too long
char    *skullName[2] = {"M_SKULL1","M_SKULL2"};

// current menudef
menu_t  *currentMenu;                          


// =============================================================================
// PROTOTYPES
// =============================================================================

void M_NewGame(int choice);
void M_Episode(int choice);
void M_ChooseSkill(int choice);
void M_LoadGame(int choice);
void M_SaveGame(int choice);
void M_Options(int choice);
void M_EndGame(int choice);
void M_ReadThis(int choice);
void M_ReadThis2(int choice);
void M_QuitDOOM(int choice);

void M_ChangeMessages(int choice);
void M_ChangeSensitivity(int choice);
void M_SfxVol(int choice);
void M_MusicVol(int choice);
void M_ChangeDetail(int choice);
void M_SizeDisplay(int choice);
void M_StartGame(int choice);
void M_Sound(int choice);

void M_FinishReadThis(int choice);
void M_LoadSelect(int choice);
void M_SaveSelect(int choice);
void M_ReadSaveStrings(void);
void M_QuickSave(void);
void M_QuickLoad(void);

void M_DrawMainMenu(void);
void M_DrawReadThis1(void);
void M_DrawReadThis2(void);
void M_DrawNewGame(void);
void M_DrawEpisode(void);
void M_DrawOptions(void);
void M_DrawSound(void);
void M_DrawLoad(void);
void M_DrawSave(void);

void M_DrawSaveLoadBorder(int x,int y);
void M_SetupNextMenu(menu_t *menudef);
void M_DrawThermo(int x,int y,int thermWidth,int thermDot);
void M_WriteText(int x, int y, char *string);
int  M_StringWidth(char *string);
int  M_StringHeight(char *string);
void M_StartMessage(char *string,void *routine,boolean input);
void M_StopMessage(void);
void M_ClearMenus (void);


// =============================================================================
// DOOM MENU
// =============================================================================

enum
{
    newgame = 0,
    options,
    loadgame,
    savegame,
    readthis,
    quitdoom,
    main_end
} main_e;


// [Julia] Now writing menu elements with text, don't using graphical patches.
menuitem_t MainMenu[]=
{
    {1,"New game",  M_NewGame,  'n'},
    {1,"Options",   M_Options,  'o'},
    {1,"Load game", M_LoadGame, 'l'},
    {1,"Save game", M_SaveGame, 's'},
    // Another hickup with Special edition.
    {1,"Read this", M_ReadThis, 'r'},
    {1,"Quit game", M_QuitDOOM, 'q'}
};


menu_t  MainDef =
{
    main_end,
    NULL,
    MainMenu,
    M_DrawMainMenu,
    97,70,
    0
};


// =============================================================================
// NEW GAME
// =============================================================================

enum
{
    killthings,
    toorough,
    hurtme,
    violence,
    nightmare,
    ultra_nm,
    newg_end
} newgame_e;

menuitem_t NewGameMenu[]=
{
    {1,"M_WIMP",    M_ChooseSkill, 'i'},
    {1,"M_ROUGH",   M_ChooseSkill, 'h'},
    {1,"M_HURT",    M_ChooseSkill, 'h'},
    {1,"M_ULTRA",   M_ChooseSkill, 'u'},
    {1,"M_NMARE",   M_ChooseSkill, 'n'},
    {1,"M_UNMARE",  M_ChooseSkill, 'z'}
};

menu_t  NewDef =
{
    newg_end,       // # of menu items
    &MainDef,        // previous menu
    NewGameMenu,    // menuitem_t ->
    M_DrawNewGame,  // drawing routine ->
    48,63,          // x,y
    hurtme          // lastOn
};


// =============================================================================
// OPTIONS MENU
// =============================================================================

enum
{
    endgame,
    messages,
    detail,
    scrnsize,
    option_empty1,
    mousesens,
    option_empty2,
    soundvol,
    opt_end
} options_e;


// [Julia] Now writing menu elements with text, don't using graphical patches.
menuitem_t OptionsMenu[]=
{
    {1,"End game", M_EndGame,'e'},
    {1,"Messages:", M_ChangeMessages,'m'},
    {1,"Detail level:", M_ChangeDetail,'g'},
    {2,"Screen size", M_SizeDisplay,'s'},
    {-1,"",0,'\0'},
    {20,"Mouse sensivity", M_ChangeSensitivity,'m'},
    {-1,"",0,'\0'},
    {1,"Sound volume", M_Sound,'s'}
};

menu_t  OptionsDef =
{
    opt_end,
    &MainDef,
    OptionsMenu,
    M_DrawOptions,
    60,37,
    0
};


// =============================================================================
// Read This! MENU 1 & 2
// =============================================================================

enum
{
    rdthsempty1,
    read1_end
} read_e;

menuitem_t ReadMenu1[] =
{
    {1,"",M_ReadThis2,0}
};

menu_t  ReadDef1 =
{
    read1_end,
    &MainDef,
    ReadMenu1,
    M_DrawReadThis1,
    280,185,
    0
};

enum
{
    rdthsempty2,
    read2_end
} read_e2;

menuitem_t ReadMenu2[]=
{
    {1,"",M_FinishReadThis,0}
};

menu_t  ReadDef2 =
{
    read2_end,
    &ReadDef1,
    ReadMenu2,
    M_DrawReadThis2,
    330,175,
    0
};


// =============================================================================
// SOUND VOLUME MENU
// =============================================================================

enum
{
    sfx_vol,
    sfx_empty1,
    music_vol,
    sfx_empty2,
    sound_end
} sound_e;

menuitem_t SoundMenu[]=
{
    {2,"Sfx Volume",M_SfxVol,'s'},
    {-1,"",0,'\0'},
    {2,"Music Volume",M_MusicVol,'m'},
    {-1,"",0,'\0'}
};

menu_t  SoundDef =
{
    sound_end,
    &OptionsDef,
    SoundMenu,
    M_DrawSound,
    88,64,
    0
};


// =============================================================================
// LOAD GAME MENU
// =============================================================================

enum
{
    load1,
    load2,
    load3,
    load4,
    load5,
    load6,
    load7,
    load_end
} load_e;

menuitem_t LoadMenu[]=
{
    {1,"", M_LoadSelect,'1'},
    {1,"", M_LoadSelect,'2'},
    {1,"", M_LoadSelect,'3'},
    {1,"", M_LoadSelect,'4'},
    {1,"", M_LoadSelect,'5'},
    {1,"", M_LoadSelect,'6'},
    {1,"", M_LoadSelect,'7'}
};

menu_t  LoadDef =
{
    load_end,
    &MainDef,
    LoadMenu,
    M_DrawLoad,
    67,38,  // [JN] Отцентрированы и скорректированы поля ввода текста
    0
};


// =============================================================================
// SAVE GAME MENU
// =============================================================================

menuitem_t SaveMenu[]=
{
    {1,"", M_SaveSelect,'1'},
    {1,"", M_SaveSelect,'2'},
    {1,"", M_SaveSelect,'3'},
    {1,"", M_SaveSelect,'4'},
    {1,"", M_SaveSelect,'5'},
    {1,"", M_SaveSelect,'6'},
    {1,"", M_SaveSelect,'7'}
};

menu_t  SaveDef =
{
    load_end,
    &MainDef,
    SaveMenu,
    M_DrawSave,
    67,38,  // [JN] Отцентрированы и скорректированы поля ввода текста
    0
};


// -----------------------------------------------------------------------------
// M_ReadSaveStrings
//
//  read the strings from the savegame files
// -----------------------------------------------------------------------------

void M_ReadSaveStrings (void)
{
    int    i;
    char   name[256];
    FILE  *handle;

    for (i = 0 ; i < load_end ; i++)
    {
        int retval;

        M_StringCopy(name, P_SaveGameFile(i), sizeof(name));
        handle = fopen(name, "rb");

        if (handle == NULL)
        {
            M_StringCopy(savegamestrings[i], EMPTYSTRING, SAVESTRINGSIZE);
            LoadMenu[i].status = 0;
            continue;
        }

        retval = fread(&savegamestrings[i], 1, SAVESTRINGSIZE, handle);
        fclose(handle);
        LoadMenu[i].status = retval = SAVESTRINGSIZE;
    }
}


// -----------------------------------------------------------------------------
// M_LoadGame & Cie.
// -----------------------------------------------------------------------------

void M_DrawLoad(void)
{
    int i;

    // [Julia] Write capitalized title "LOAD GAME"
    HU_WriteTextBig(97, 11, "LOAD GAME");
    
    for (i = 0 ; i < load_end ; i++)
    {
        M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
        M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }
}


// -----------------------------------------------------------------------------
// Draw border for the savegame description
// -----------------------------------------------------------------------------

void M_DrawSaveLoadBorder(int x,int y)
{
    int i;

    V_DrawShadowedPatch(x - 8, y + 8, 
                        W_CacheLumpName(DEH_String("M_LSLEFT"), PU_CACHE));

    for (i = 0 ; i < 24 ; i++)
    {
        V_DrawShadowedPatch(x, y + 8,
                            W_CacheLumpName(DEH_String("M_LSCNTR"), PU_CACHE));
        x += 8;
    }

    V_DrawShadowedPatch(x, y + 8,
                        W_CacheLumpName(DEH_String("M_LSRGHT"), PU_CACHE));
}


// -----------------------------------------------------------------------------
// User wants to load this game
// -----------------------------------------------------------------------------

void M_LoadSelect(int choice)
{
    char name[256];

    M_StringCopy(name, P_SaveGameFile(choice), sizeof(name));

    G_LoadGame (name);
    M_ClearMenus ();
}


// -----------------------------------------------------------------------------
// Selected from DOOM menu
// -----------------------------------------------------------------------------
void M_LoadGame (int choice)
{
    M_SetupNextMenu(&LoadDef);
    M_ReadSaveStrings();
}


// -----------------------------------------------------------------------------
//  M_SaveGame & Cie.
// -----------------------------------------------------------------------------

void M_DrawSave(void)
{
    int i;
	
    if (QuickSaveTitle)
    {
        // [Julia] Write capitalized title
        HU_WriteTextBig(84, 11, "QUICK SAVING");
    }
    else
    {
        // [Julia] Write capitalized title
        HU_WriteTextBig(99, 11, "SAVE GAME");
    }

    for (i = 0 ; i < load_end ; i++)
    {
        M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
        M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }

    if (saveStringEnter)
    {
        i = M_StringWidth(savegamestrings[saveSlot]);
        M_WriteText(LoadDef.x + i,LoadDef.y+LINEHEIGHT*saveSlot,"_");
    }
}


// -----------------------------------------------------------------------------
// M_Responder calls this when user is finished
// -----------------------------------------------------------------------------

void M_DoSave(int slot)
{
    G_SaveGame (slot,savegamestrings[slot]);
    M_ClearMenus ();

    // PICK QUICKSAVE SLOT YET?
    if (quickSaveSlot == -2)
    quickSaveSlot = slot;
}


// -----------------------------------------------------------------------------
// User wants to save. Start string input for M_Responder
// -----------------------------------------------------------------------------

void M_SaveSelect(int choice)
{
    // we are going to be intercepting all chars
    saveStringEnter = 1;

    saveSlot = choice;
    M_StringCopy(saveOldString,savegamestrings[choice], SAVESTRINGSIZE);

    if (!strcmp(savegamestrings[choice], EMPTYSTRING))
    savegamestrings[choice][0] = 0;

    saveCharIndex = strlen(savegamestrings[choice]);
}


// -----------------------------------------------------------------------------
// Selected from DOOM menu
// -----------------------------------------------------------------------------

void M_SaveGame (int choice)
{
    if (!usergame)
    {
        M_StartMessage(DEH_String(SAVEDEAD), NULL,false);
        return;
    }

    if (gamestate != GS_LEVEL)
    return;

    M_SetupNextMenu(&SaveDef);
    M_ReadSaveStrings();
}


// -----------------------------------------------------------------------------
// M_QuickSave
// -----------------------------------------------------------------------------

char tempstring[80];

void M_QuickSaveResponse(int key)
{
    if (key == key_menu_confirm)
    {
        M_DoSave(quickSaveSlot);
        S_StartSound(NULL,sfx_swtchx);
    }
}

// -----------------------------------------------------------------------------
// M_QuickSave
// -----------------------------------------------------------------------------

void M_QuickSave(void)
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

    M_DoSave(quickSaveSlot);
}


// -----------------------------------------------------------------------------
// M_QuickLoadResponse
// -----------------------------------------------------------------------------

void M_QuickLoadResponse(int key)
{
    if (key == key_menu_confirm)
    {
        M_LoadSelect(quickSaveSlot);
        S_StartSound(NULL,sfx_swtchx);
    }
}


// -----------------------------------------------------------------------------
// M_QuickLoad
// -----------------------------------------------------------------------------

void M_QuickLoad(void)
{
    if (quickSaveSlot < 0)
    {
        M_StartMessage(DEH_String(QSAVESPOT), NULL,false);
        return;
    }

    M_LoadSelect(quickSaveSlot);
}


// -----------------------------------------------------------------------------
// Read This Menus
// Had a "quick hack to fix romero bug"
//
// [Julia] Jaguar HELP screen and skull placement. Same to Doom II.
// -----------------------------------------------------------------------------

void M_DrawReadThis1(void)
{
    inhelpscreens = true;
    
    V_DrawPatch (0, 0, W_CacheLumpName(DEH_String("M_TITLE"), PU_CACHE));
    V_DrawShadowedPatch (0, 0, W_CacheLumpName(DEH_String("HELP"), PU_CACHE));

    ReadDef1.x = 330;
    ReadDef1.y = 162;
}


//
// Read This Menus - optional second page.
//
void M_DrawReadThis2(void)
{
    // [Julia] TODO - REMOVE
}


// -----------------------------------------------------------------------------
// Change Sfx & Music volumes
// -----------------------------------------------------------------------------

void M_DrawSound(void)
{
    // [Julia] Write capitalized title
    HU_WriteTextBig(71, 39, "SOUND VOLUME");

    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(sfx_vol+1), 16,sfxVolume);
    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(music_vol+1), 16,musicVolume);
}


// -----------------------------------------------------------------------------
// M_Sound
// -----------------------------------------------------------------------------

void M_Sound(int choice)
{
    M_SetupNextMenu(&SoundDef);
}


// -----------------------------------------------------------------------------
// M_SfxVol
// -----------------------------------------------------------------------------

void M_SfxVol(int choice)
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


// -----------------------------------------------------------------------------
// M_MusicVol
// -----------------------------------------------------------------------------

void M_MusicVol(int choice)
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


// -----------------------------------------------------------------------------
// M_DrawMainMenu
// -----------------------------------------------------------------------------

void M_DrawMainMenu(void)
{
    V_DrawShadowedPatch(94, 2, W_CacheLumpName(DEH_String("M_DOOM"), PU_CACHE));
}


// -----------------------------------------------------------------------------
// M_NewGame
// -----------------------------------------------------------------------------

void M_DrawNewGame(void)
{
    // [Julia] Write capitalized titles
    HU_WriteTextBig(99, 13, "NEW GAME");
    HU_WriteTextBig(48, 38, "Choose Skill Level:");
}

// -----------------------------------------------------------------------------
// M_NewGame
// -----------------------------------------------------------------------------

void M_NewGame(int choice)
{
    // [Julia] Don't go to episode selection
    M_SetupNextMenu(&NewDef);
}


// -----------------------------------------------------------------------------
// M_ChooseSkill
// -----------------------------------------------------------------------------

void M_ChooseSkill(int choice)
{
    G_DeferedInitNew(choice,1,1);
    M_ClearMenus ();
}


// -----------------------------------------------------------------------------
// M_Options
// -----------------------------------------------------------------------------

void M_DrawOptions(void)
{
    // [Julia] Write capitalized titles
    HU_WriteTextBig(110, 11, "OPTIONS");

    HU_WriteTextBig(180, 53, showMessages == 1 ? "on" : "off");
    HU_WriteTextBig(202, 69, detailLevel == 1 ? "low" : "high");

    // [Julia] Draw sliders
    M_DrawThermo(OptionsDef.x, OptionsDef.y 
                + LINEHEIGHT * (mousesens + 1), 13, mouseSensitivity);
    M_DrawThermo(OptionsDef.x,OptionsDef.y
                + LINEHEIGHT * (scrnsize + 1), 12, screenSize);
}


// -----------------------------------------------------------------------------
// M_Options
// -----------------------------------------------------------------------------

void M_Options(int choice)
{
    M_SetupNextMenu(&OptionsDef);
}


// -----------------------------------------------------------------------------
// Toggle messages on/off
// -----------------------------------------------------------------------------

void M_ChangeMessages(int choice)
{
    choice = 0;
    showMessages = 1 - showMessages;

    if (!showMessages)
    players[consoleplayer].message = DEH_String(MSGOFF);
    else
    players[consoleplayer].message = DEH_String(MSGON);

    message_dontfuckwithme = true;
}


// -----------------------------------------------------------------------------
// M_EndGame
// -----------------------------------------------------------------------------

void M_EndGameResponse(int key)
{
    if (key != key_menu_confirm)
    return;

    currentMenu->lastOn = itemOn;
    M_ClearMenus ();
    D_StartTitle ();
}


// -----------------------------------------------------------------------------
// M_EndGame
// -----------------------------------------------------------------------------

void M_EndGame(int choice)
{
    choice = 0;

    if (!usergame)
    {
        S_StartSound(NULL,sfx_oof);
        return;
    }

    M_StartMessage(DEH_String(ENDGAME), M_EndGameResponse,true);
}


// -----------------------------------------------------------------------------
// M_ReadThis
// -----------------------------------------------------------------------------

void M_ReadThis(int choice)
{
    choice = 0;
    M_SetupNextMenu(&ReadDef1);
}

void M_ReadThis2(int choice)
{
    // Close the menu
    M_FinishReadThis(0);
}

void M_FinishReadThis(int choice)
{
    choice = 0;
    M_SetupNextMenu(&MainDef);
}


// -----------------------------------------------------------------------------
// M_QuitDOOM
// -----------------------------------------------------------------------------

void M_QuitResponse(int key)
{
    if (key != key_menu_confirm)
    return;

    I_Quit ();
}


// -----------------------------------------------------------------------------
// M_QuitDOOM
// -----------------------------------------------------------------------------

void M_QuitDOOM(int choice)
{
    DEH_snprintf(endstring, sizeof(endstring),"%s\n\n" PRESSYN, DEH_String(QUITMSG));
    M_StartMessage(endstring,M_QuitResponse,true);
}


// -----------------------------------------------------------------------------
// M_ChangeSensitivity
// -----------------------------------------------------------------------------

void M_ChangeSensitivity(int choice)
{
    switch(choice)
    {
        case 0:
        if (mouseSensitivity)
            mouseSensitivity--;
        break;

        case 1:
        if (mouseSensitivity < 255) // [crispy] extended range
            mouseSensitivity++;
        break;
    }
}


// -----------------------------------------------------------------------------
// M_ChangeDetail
// -----------------------------------------------------------------------------

void M_ChangeDetail(int choice)
{
    choice = 0;
    detailLevel = 1 - detailLevel;

    R_SetViewSize (screenblocks, detailLevel);

    if (!detailLevel)
    players[consoleplayer].message = DEH_String(DETAILHI);
    else
    players[consoleplayer].message = DEH_String(DETAILLO);
}


// -----------------------------------------------------------------------------
// M_SizeDisplay
// -----------------------------------------------------------------------------

void M_SizeDisplay(int choice)
{
    switch(choice)
    {
        case 0:
        if (screenSize > 0)
        {
            screenblocks--;
            screenSize--;
        }
        break;

        case 1:
        if (screenSize < 11)
        {
            screenblocks++;
            screenSize++;
        }
        break;
    }

    R_SetViewSize (screenblocks, detailLevel);
}


// -----------------------------------------------------------------------------
// Menu Functions
// -----------------------------------------------------------------------------

void M_DrawThermo (int x, int y, int thermWidth, int thermDot )
{
    int   xx;
    int   i;
    char  num[4];

    xx = x;

    V_DrawShadowedPatch(xx, y, W_CacheLumpName(DEH_String("M_THERML"), PU_CACHE));

    xx += 8;

    for (i=0;i<thermWidth;i++)
    {
        V_DrawShadowedPatch(xx, y, W_CacheLumpName (DEH_String("M_THERMM"), PU_CACHE));
        xx += 8;
    }

    V_DrawShadowedPatch(xx, y, W_CacheLumpName (DEH_String("M_THERMR"), PU_CACHE));

    // [Julia] from Crispy Doom: also draw numerical representation of slider position.
    // Also don't draw it for "Screen size" slider, which is a 11 units in width.
    if (thermWidth > 12)
    {
        M_snprintf(num, 4, "%3d", thermDot);
        M_WriteText(xx + 8, y + 3, num);
    }

    // [crispy] do not crash anymore if value exceeds thermometer range

    // [Julia] If slider goes to the right out of bounds, color it green
    if (thermDot >= thermWidth)
    {
        thermDot = thermWidth - 1;
        V_DrawPatch((x + 8) + thermDot * 8, y, 
                    W_CacheLumpName(DEH_String("M_THERMW"),
                    PU_CACHE));
    }
    // [Julia] If slider goes is on most left position, color it red
    else if (thermDot == 0)
    {
        V_DrawPatch((x + 8) + thermDot * 8, y,
                    W_CacheLumpName(DEH_String("M_THERMD"),
                    PU_CACHE));
    }
    else
    {
        V_DrawPatch((x + 8) + thermDot * 8, y,
                    W_CacheLumpName(DEH_String("M_THERMO"), PU_CACHE));        
    }
}


// -----------------------------------------------------------------------------
// M_StartMessage
// -----------------------------------------------------------------------------

void M_StartMessage (char *string, void *routine, boolean input)
{
    messageLastMenuActive = menuactive;
    messageToPrint = 1;
    messageString = string;
    messageRoutine = routine;
    messageNeedsInput = input;
    menuactive = true;
    return;
}


// -----------------------------------------------------------------------------
// M_StopMessage
// -----------------------------------------------------------------------------

void M_StopMessage(void)
{
    menuactive = messageLastMenuActive;
    messageToPrint = 0;
}


// -----------------------------------------------------------------------------
// M_StringWidth
//
// Find string width from hu_font chars
// -----------------------------------------------------------------------------

int M_StringWidth (char *string)
{
    size_t  i;
    int     w = 0;
    int     c;

    for (i = 0 ; i < strlen(string) ; i++)
    {
        c = toupper(string[i]) - HU_FONTSTART;
        if (c < 0 || c >= HU_FONTSIZE)
            w += 4;
        else
            w += SHORT (hu_font[c]->width);
    }

    return w;
}


// -----------------------------------------------------------------------------
// Find string height from hu_font chars
// -----------------------------------------------------------------------------

int M_StringHeight(char *string)
{
    size_t  i;
    int     h;
    int     height = SHORT(hu_font[0]->height);

    h = height;

    for (i = 0;i < strlen(string);i++)
        if (string[i] == '\n')
            h += height;

    return h;
}


// -----------------------------------------------------------------------------
// Write a string using the hu_font
// -----------------------------------------------------------------------------

void M_WriteText (int x, int y, char *string)
{
    int    w;
    int    c;
    int    cx;
    int    cy;
    char  *ch;

    ch = string;
    cx = x;
    cy = y;

    while(1)
    {
        c = *ch++;
        if (!c)
            break;
        if (c == '\n')
        {
            cx = x;
            cy += 12;
            continue;
        }

        c = toupper(c) - HU_FONTSTART;
        if (c < 0 || c>= HU_FONTSIZE)
        {
            cx += 4;
            continue;
        }

        w = SHORT (hu_font[c]->width);
        if (cx+w > ORIGWIDTH)
            break;

        V_DrawShadowedPatch(cx, cy, hu_font[c]);

        cx+=w;
    }
}

// -----------------------------------------------------------------------------
// These keys evaluate to a "null" key in Vanilla Doom that allows weird
// jumping in the menus. Preserve this behavior for accuracy.
// -----------------------------------------------------------------------------

static boolean IsNullKey(int key)
{
    return key == KEY_PAUSE 
        || key == KEY_CAPSLOCK 
        || key == KEY_SCRLCK 
        || key == KEY_NUMLOCK;
}

// =============================================================================
// CONTROL PANEL
// =============================================================================


// -----------------------------------------------------------------------------
// M_Responder
// -----------------------------------------------------------------------------

boolean M_Responder (event_t *ev)
{
    int        ch;
    int        key;
    int        i;
    static int joywait = 0;
    static int mousewait = 0;
    static int mousey = 0;
    static int lasty = 0;
    static int mousex = 0;
    static int lastx = 0;

    // [Julia] Do not ask for quit in -devparm
    if (devparm && ev->data1 == key_menu_quit)
    {
        I_Quit();
        return true;
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

    if (ev->type == ev_joystick && joywait < I_GetTime())
    {
        if (ev->data3 < 0)
        {
            key = key_menu_up;
            joywait = I_GetTime() + 5;
        }
        else if (ev->data3 > 0)
        {
            key = key_menu_down;
            joywait = I_GetTime() + 5;
        }

        if (ev->data2 < 0)
        {
            key = key_menu_left;
            joywait = I_GetTime() + 2;
        }
        else if (ev->data2 > 0)
        {
            key = key_menu_right;
            joywait = I_GetTime() + 2;
        }

        if (ev->data1&1)
        {
            key = key_menu_forward;
            joywait = I_GetTime() + 5;
        }
        if (ev->data1&2)
        {
            key = key_menu_back;
            joywait = I_GetTime() + 5;
        }
        if (joybmenu >= 0 && (ev->data1 & (1 << joybmenu)) != 0)
        {
            key = key_menu_activate;
            joywait = I_GetTime() + 5;
        }
    }
    else
    {
        if (ev->type == ev_mouse && mousewait < I_GetTime())
        {
            mousey += ev->data3;
            if (mousey < lasty-30)
            {
                key = key_menu_down;
                mousewait = I_GetTime() + 5;
                mousey = lasty -= 30;
            }
            else if (mousey > lasty+30)
            {
                key = key_menu_up;
                mousewait = I_GetTime() + 5;
                mousey = lasty += 30;
            }

            mousex += ev->data2;

            if (mousex < lastx-30)
            {
                key = key_menu_left;
                mousewait = I_GetTime() + 5;
                mousex = lastx -= 30;
            }
            else if (mousex > lastx+30)
            {
                key = key_menu_right;
                mousewait = I_GetTime() + 5;
                mousex = lastx += 30;
            }

            if (ev->data1&1)
            {
                key = key_menu_forward;
                mousewait = I_GetTime() + 15;
            }

            if (ev->data1&2)
            {
                key = key_menu_back;
                mousewait = I_GetTime() + 15;
            }
            
            // [crispy] scroll menus with mouse wheel
            // [JN] it also affecting mouse side buttons (forward/backward)
            if (mousebprevweapon >= 0 && ev->data1 & (1 << mousebprevweapon))
            {
                key = key_menu_down;
                mousewait = I_GetTime() + 5;
            }
            else
            if (mousebnextweapon >= 0 && ev->data1 & (1 << mousebnextweapon))
            {
                key = key_menu_up;
                mousewait = I_GetTime() + 5;
            }
        }
        else
        {
            if (ev->type == ev_keydown)
            {
                key = ev->data1;
                ch = ev->data2;
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
            M_StringCopy(savegamestrings[saveSlot], saveOldString, SAVESTRINGSIZE);
            break;

            case KEY_ENTER:
            saveStringEnter = 0;
            if (savegamestrings[saveSlot][0])
            M_DoSave(saveSlot);
            break;

            default:
            // This is complicated.
            // Vanilla has a bug where the shift key is ignored when entering
            // a savegame name. If vanilla_keyboard_mapping is on, we want
            // to emulate this bug by using 'data1'. But if it's turned off,
            // it implies the user doesn't care about Vanilla emulation: just
            // use the correct 'data2'.

            if (vanilla_keyboard_mapping)
            {
                ch = key;
            }

            ch = toupper(ch);

            if (ch != ' ' && (ch - HU_FONTSTART < 0 || ch - HU_FONTSTART >= HU_FONTSIZE))
            {
                break;
            }

            if (ch >= 32 && ch <= 127 &&
            saveCharIndex < SAVESTRINGSIZE-1 &&
            M_StringWidth(savegamestrings[saveSlot]) <
            (SAVESTRINGSIZE-2)*8)
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
            if (key != ' ' && key != KEY_ESCAPE && key != key_menu_confirm && key != key_menu_abort)
            {
                return false;
            }
        }

    menuactive = messageLastMenuActive;
    messageToPrint = 0;
    if (messageRoutine)
        messageRoutine(key);

    menuactive = false;
    S_StartSound(NULL,sfx_swtchx);
    return true;
    }

    if ((devparm && key == key_menu_help) || (key != 0 && key == key_menu_screenshot))
    {
        G_ScreenShot ();
        return true;
    }

    // [JN] Toggling of crosshair
    if (key == key_togglecrosshair)
    {
        static char crosshairmsg[24];

        if (!crosshair_draw)
        {
            crosshair_draw = true;
        }
        else
        {
            crosshair_draw = false;
        }
        
        M_snprintf(crosshairmsg, sizeof(crosshairmsg), STSRT_CROSSHAIR "%s",
            crosshair_draw ? STSTR_CROSSHAIR_ON : STSTR_CROSSHAIR_OFF);

        players[consoleplayer].message = crosshairmsg;
        S_StartSound(NULL,sfx_swtchn);

        return true;
    }

    // F-Keys
    if (!menuactive)
    {
        if (key == key_menu_decscreen)      // Screen size down
        {
            if (automapactive || chat_on)
            return false;
            M_SizeDisplay(0);
            S_StartSound(NULL,sfx_stnmov);
            return true;
        }
        else if (key == key_menu_incscreen) // Screen size up
        {
            if (automapactive || chat_on)
            return false;
            M_SizeDisplay(1);
            S_StartSound(NULL,sfx_stnmov);
            return true;
        }
        else if (key == key_menu_help)     // Help key
        {
        M_StartControlPanel ();

        if ( gamemode == retail )
            currentMenu = &ReadDef2;
        else
            currentMenu = &ReadDef1;

        itemOn = 0;
        S_StartSound(NULL,sfx_swtchn);
        return true;
        }
        else if (key == key_menu_save)     // Save
        {
            QuickSaveTitle = false;
            M_StartControlPanel();
            S_StartSound(NULL,sfx_swtchn);
            M_SaveGame(0);
            return true;
        }
        else if (key == key_menu_load)     // Load
        {
            M_StartControlPanel();
            S_StartSound(NULL,sfx_swtchn);
            M_LoadGame(0);
            return true;
        }
        else if (key == key_menu_volume)   // Sound Volume
        {
            M_StartControlPanel ();
            currentMenu = &SoundDef;
            itemOn = sfx_vol;
            S_StartSound(NULL,sfx_swtchn);
            return true;
        }
        else if (key == key_menu_detail)   // Detail toggle
        {
            M_ChangeDetail(0);
            S_StartSound(NULL,sfx_swtchn);
            return true;
        }
        else if (key == key_menu_qsave)    // Quicksave
        {
            QuickSaveTitle = true;
            S_StartSound(NULL,sfx_swtchn);
            M_QuickSave();
            return true;
        }
        else if (key == key_menu_endgame)  // End game
        {
            S_StartSound(NULL,sfx_swtchn);
            M_EndGame(0);
            return true;
        }
        else if (key == key_menu_messages) // Toggle messages
        {
            M_ChangeMessages(0);
            S_StartSound(NULL,sfx_swtchn);
            return true;
        }
        else if (key == key_menu_qload)    // Quickload
        {
            S_StartSound(NULL,sfx_swtchn);
            M_QuickLoad();
            return true;
        }
        else if (key == key_menu_quit)     // Quit DOOM
        {
            S_StartSound(NULL,sfx_swtchn);
            M_QuitDOOM(0);
            return true;
        }
        else if (key == key_menu_gamma)    // gamma toggle
        {
            usegamma++;
            if (usegamma > 8)
                usegamma = 0;

            I_SetPalette ((byte *)W_CacheLumpName(DEH_String("PLAYPAL"),
                                                             PU_CACHE) + 
                                                             st_palette * 768);
            players[consoleplayer].message = DEH_String(gammamsg[usegamma]);
            return true;
        }
    }

    // Pop-up menu?
    if (!menuactive)
    {
        if (key == key_menu_activate)
        {
            M_StartControlPanel ();
            S_StartSound(NULL,sfx_swtchn);
            return true;
        }
        return false;
    }

    // Keys usable within menu

    if (key == key_menu_down)
    {
        // Move down to next item

        do
        {
            if (itemOn+1 > currentMenu->numitems-1)
            itemOn = 0;
            else itemOn++;
            S_StartSound(NULL,sfx_pstop);
        } while(currentMenu->menuitems[itemOn].status==-1);

        return true;
    }
    else if (key == key_menu_up)
    {
        // Move back up to previous item

        do
        {
            if (!itemOn)
            itemOn = currentMenu->numitems-1;
            else itemOn--;
            S_StartSound(NULL,sfx_pstop);
        } while(currentMenu->menuitems[itemOn].status==-1);

        return true;
    }
    else if (key == key_menu_left)
    {
        // Slide slider left

    if (currentMenu->menuitems[itemOn].routine && currentMenu->menuitems[itemOn].status == 2)
    {
        S_StartSound(NULL,sfx_stnmov);
        currentMenu->menuitems[itemOn].routine(0);
    }
    return true;
    }
    else if (key == key_menu_right)
    {
        // Slide slider right

        if (currentMenu->menuitems[itemOn].routine && currentMenu->menuitems[itemOn].status == 2)
        {
            S_StartSound(NULL,sfx_stnmov);
            currentMenu->menuitems[itemOn].routine(1);
        }
        return true;
    }
    else if (key == key_menu_forward)
    {
        // Activate menu item

        if (currentMenu->menuitems[itemOn].routine && currentMenu->menuitems[itemOn].status)
        {
            currentMenu->lastOn = itemOn;
            if (currentMenu->menuitems[itemOn].status == 2)
            {
                currentMenu->menuitems[itemOn].routine(1);      // right arrow
                S_StartSound(NULL,sfx_stnmov);
            }
            else
            {
                currentMenu->menuitems[itemOn].routine(itemOn);
                S_StartSound(NULL,sfx_pistol);
            }
        }
        return true;
    }
    else if (key == key_menu_activate)
    {
        // Deactivate menu

        currentMenu->lastOn = itemOn;
        M_ClearMenus ();
        S_StartSound(NULL,sfx_swtchx);
        return true;
    }
    else if (key == key_menu_back)
    {
        // Go back to previous menu

        currentMenu->lastOn = itemOn;
        if (currentMenu->prevMenu)
        {
            currentMenu = currentMenu->prevMenu;

            itemOn = currentMenu->lastOn;
            S_StartSound(NULL,sfx_swtchn);
        }
        return true;
    }

    // [crispy] delete a savegame
    else if (key == KEY_DEL)
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
    }

    // Keyboard shortcut?
    // Vanilla Doom has a weird behavior where it jumps to the scroll bars
    // when the certain keys are pressed, so emulate this.

    else if (ch != 0 || IsNullKey(key))
    {
        for (i = itemOn+1;i < currentMenu->numitems;i++)
        {
            if (currentMenu->menuitems[i].alphaKey == ch)
            {
                itemOn = i;
                S_StartSound(NULL,sfx_pstop);
                return true;
            }
        }

        for (i = 0;i <= itemOn;i++)
        {
            if (currentMenu->menuitems[i].alphaKey == ch)
            {
                itemOn = i;
                S_StartSound(NULL,sfx_pstop);
                return true;
            }
        }
    }

    return false;
}


// -----------------------------------------------------------------------------
// M_StartControlPanel
// -----------------------------------------------------------------------------

void M_StartControlPanel (void)
{
    // intro might call this repeatedly
    if (menuactive)
    return;

    menuactive = 1;
    currentMenu = &MainDef;         // JDC
    itemOn = currentMenu->lastOn;   // JDC
}


// -----------------------------------------------------------------------------
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
// -----------------------------------------------------------------------------

void M_Drawer (void)
{
    static short  x;
    static short  y;
    unsigned int  i;
    unsigned int  max;
    char          string[80];
    char         *name;
    int           start;

    inhelpscreens = false;

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
                    M_StringCopy(string, messageString + start, sizeof(string));
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

            x = 160 - M_StringWidth(string) / 2;
            M_WriteText(x, y, string);
            y += SHORT(hu_font[0]->height);
        }

        return;
    }

    if (!menuactive)
    return;

    if (currentMenu->routine)
    currentMenu->routine();     // call Draw routine

    // DRAW MENU
    x = currentMenu->x;
    y = currentMenu->y;
    max = currentMenu->numitems;

    for (i=0;i<max;i++)
    {
        name = DEH_String(currentMenu->menuitems[i].name);

        if (name[0])
        {
            // [Julia] Write menu items using big font. Yeah!
            if (currentMenu == &NewDef)
            V_DrawShadowedPatch(x, y, W_CacheLumpName(name, PU_CACHE));
            else
            HU_WriteTextBig(x, y, name);
        }

    y += LINEHEIGHT;
    }

    // DRAW SKULL
    V_DrawShadowedPatch(x + SKULLXOFF, currentMenu->y - 5 + itemOn*LINEHEIGHT,
        W_CacheLumpName(DEH_String(skullName[whichSkull]), PU_CACHE));
}


// -----------------------------------------------------------------------------
// M_ClearMenus
// -----------------------------------------------------------------------------

void M_ClearMenus (void)
{
    menuactive = 0;
}


// -----------------------------------------------------------------------------
// M_SetupNextMenu
// -----------------------------------------------------------------------------

void M_SetupNextMenu(menu_t *menudef)
{
    currentMenu = menudef;
    itemOn = currentMenu->lastOn;
}


// -----------------------------------------------------------------------------
// M_Ticker
// -----------------------------------------------------------------------------

void M_Ticker (void)
{
    if (--skullAnimCounter <= 0)
    {
        whichSkull ^= 1;
        skullAnimCounter = 8;
    }
}


// -----------------------------------------------------------------------------
// M_Init
// -----------------------------------------------------------------------------

void M_Init (void)
{
    currentMenu = &MainDef;
    menuactive = 0;
    itemOn = currentMenu->lastOn;
    whichSkull = 0;
    skullAnimCounter = 10;
    screenSize = screenblocks - 3;
    messageToPrint = 0;
    messageString = NULL;
    messageLastMenuActive = menuactive;
    quickSaveSlot = -1;

    // [Julia] Jaguar has no "read this" entry.
    MainMenu[readthis] = MainMenu[quitdoom];
    MainDef.numitems--;
    MainDef.y += 8;
    NewDef.prevMenu = &MainDef;
}


// -----------------------------------------------------------------------------
// [from crispy] Ability to delete save games
// -----------------------------------------------------------------------------

static char *savegwarning;

static void M_ConfirmDeleteGameResponse (int key)
{
    free(savegwarning);

    if (key == key_menu_confirm)
    {
        char name[256];

        M_StringCopy(name, P_SaveGameFile(itemOn), sizeof(name));
        remove(name);
        M_ReadSaveStrings();
    }
}

void M_ConfirmDeleteGame ()
{
    savegwarning =
    M_StringJoin("are you sure you want to\ndelete saved game\n\n\"",
        savegamestrings[itemOn], "\"?\n\n", PRESSYN, NULL);

    M_StartMessage(savegwarning, M_ConfirmDeleteGameResponse, true);
    messageToPrint = 2;
    S_StartSound(NULL,sfx_swtchn);
}

