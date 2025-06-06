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
//   Menu widget stuff, episode selection and such.
//    


#ifndef __M_MENU__
#define __M_MENU__



#include "d_event.h"

//
// MENUS
//
// Called by main loop,
// saves config file and calls I_Quit when user exits.
// Even when the menu is not displayed,
// this can resize the view and change game parameters.
// Does all the real work of the menu interaction.
boolean M_Responder (event_t *ev);


// Called by main loop,
// only used for menu (skull cursor) animation.
void M_Ticker (void);

// Called by main loop,
// draws the menus directly into the screen buffer.
void M_Drawer (void);

// Called by D_DoomMain,
// loads the config file.
void M_Init (void);

// Called by intro code to force menu up upon a keypress,
// does nothing if menu is already up.
void M_StartControlPanel (void);


extern void M_ConfirmDeleteGame (void);

extern void M_WriteText (int x, int y, const char *string, byte *table);
extern void M_WriteTextCentered (const int y, const char *string, byte *table);
extern void M_WriteTextGlow (int x, int y, const char *string, byte *table1, byte *table2, int alpha);
extern int  M_StringWidth (const char *string);
extern void M_WriteTextBig (int x, int y, const char *string, byte *table);
extern void M_WriteTextBigCentered (const int y, const char *string, byte *table);
extern void M_WriteTextBigGlow (int x, int y, const char *string, byte *table1, byte *table2, int alpha);

// [FG] support up to 8 pages of savegames
// extern int savepage;

#endif    
