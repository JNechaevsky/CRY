//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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

#include <stdio.h>

#include "doomtype.h"
#include "doomkeys.h"
#include "m_config.h"
#include "m_controls.h"
#include "m_misc.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>  // [JN] RegisterHotKey
#endif


//
// Keyboard controls
//

// Movement

int key_up          = 'w';            int key_up2          = 0;
int key_down        = 's';            int key_down2        = 0;
int key_right       = KEY_RIGHTARROW; int key_right2       = 0;
int key_left        = KEY_LEFTARROW;  int key_left2        = 0;
int key_strafeleft  = 'a';            int key_strafeleft2  = 0;
int key_straferight = 'd';            int key_straferight2 = 0;
int key_speed       = KEY_RSHIFT;     int key_speed2       = 0;
int key_strafe      = KEY_RALT;       int key_strafe2      = 0;
int key_180turn     = 0;              int key_180turn2     = 0; // [crispy]

// Action

int key_fire = KEY_RCTRL; int key_fire2 = 0;
int key_use  = ' ';       int key_use2  = 0;

// Advanced movement

int always_run     = 1;
int key_autorun    = KEY_CAPSLOCK; int key_autorun2    = 0; // [crispy]
int key_mouse_look = 0;            int key_mouse_look2 = 0; // [crispy]
int key_novert     = 0;            int key_novert2     = 0;

// Special keys

int key_reloadlevel   = 0; int key_reloadlevel2   = 0; // [crispy]
int key_nextlevel     = 0; int key_nextlevel2     = 0; // [crispy]
int key_flip_levels   = 0; int key_flip_levels2   = 0; // [crispy]
int key_widget_enable = 0; int key_widget_enable2 = 0;

// Special modes

int key_spectator   = 0; int key_spectator2   = 0;
int key_freeze      = 0; int key_freeze2      = 0;
int key_notarget    = 0; int key_notarget2    = 0;
int key_buddha      = 0; int key_buddha2      = 0;

// Weapons

int key_weapon1    = '1'; int key_weapon1_2   = 0;
int key_weapon2    = '2'; int key_weapon2_2   = 0;
int key_weapon3    = '3'; int key_weapon3_2   = 0;
int key_weapon4    = '4'; int key_weapon4_2   = 0;
int key_weapon5    = '5'; int key_weapon5_2   = 0;
int key_weapon6    = '6'; int key_weapon6_2   = 0;
int key_weapon7    = '7'; int key_weapon7_2   = 0;
int key_weapon8    = '8'; int key_weapon8_2   = 0;
int key_prevweapon = 0;   int key_prevweapon2 = 0;
int key_nextweapon = 0;   int key_nextweapon2 = 0;

// Automap

int key_map_toggle    = KEY_TAB; int key_map_toggle2    = 0;
int key_map_zoomin    = '=';     int key_map_zoomin2    = KEYP_PLUS;
int key_map_zoomout   = '-';     int key_map_zoomout2   = KEYP_MINUS;
int key_map_maxzoom   = '0';     int key_map_maxzoom2   = 0;
int key_map_follow    = 'f';     int key_map_follow2    = 0;
int key_map_rotate    = 'r';     int key_map_rotate2    = 0;
int key_map_overlay   = 'o';     int key_map_overlay2   = 0;
int key_map_mousepan  = 0;       int key_map_mousepan2  = 0;
int key_map_grid      = 'g';     int key_map_grid2      = 0;
int key_map_mark      = 'm';     int key_map_mark2      = 0;
int key_map_clearmark = 'c';     int key_map_clearmark2 = 0;
int key_map_north     = KEY_UPARROW;
int key_map_south     = KEY_DOWNARROW;
int key_map_east      = KEY_RIGHTARROW;
int key_map_west      = KEY_LEFTARROW;

// Function keys

int key_menu_help     = KEY_F1;  int key_menu_help2     = 0;
int key_menu_save     = KEY_F2;  int key_menu_save2     = 0;
int key_menu_load     = KEY_F3;  int key_menu_load2     = 0;
int key_menu_volume   = KEY_F4;  int key_menu_volume2   = 0;
int key_menu_detail   = KEY_F5;  int key_menu_detail2   = 0;
int key_menu_qsave    = KEY_F6;  int key_menu_qsave2    = 0;
int key_menu_endgame  = KEY_F7;  int key_menu_endgame2  = 0;
int key_menu_messages = KEY_F8;  int key_menu_messages2 = 0;
int key_menu_qload    = KEY_F9;  int key_menu_qload2    = 0;
int key_menu_quit     = KEY_F10; int key_menu_quit2     = 0;
int key_menu_gammad   = 0;       int key_menu_gammad2   = 0;
int key_menu_gamma    = KEY_F11; int key_menu_gamma2    = 0;
int key_menu_palette  = KEY_F12; int key_menu_palette2  = 0;

// Shortcut keys

int key_pause              = KEY_PAUSE;  int key_pause2              = 0;
int key_menu_screenshot    = KEY_PRTSCR; int key_menu_screenshot2    = 0;
int key_message_refresh    = KEY_ENTER;  int key_message_refresh2    = 0;

// Special menu keys, not available for rebinding

int key_menu_activate  = KEY_ESCAPE;
int key_menu_up        = KEY_UPARROW;
int key_menu_down      = KEY_DOWNARROW;
int key_menu_left      = KEY_LEFTARROW;
int key_menu_right     = KEY_RIGHTARROW;
int key_menu_back      = KEY_BACKSPACE;
int key_menu_forward   = KEY_ENTER;
int key_menu_confirm   = 'y';
int key_menu_abort     = 'n';
int key_menu_incscreen = KEY_EQUALS;
int key_menu_decscreen = KEY_MINUS;
int key_menu_del       = KEY_DEL; // [crispy]

//
// Mouse controls
//

int mousebfire        = 0;  int mousebfire2        = -1;
int mousebforward     = 2;  int mousebforward2     = -1;
int mousebbackward    = -1; int mousebbackward2    = -1;
int mousebuse         = -1; int mousebuse2         = -1;
int mousebspeed       = -1; int mousebspeed2       = -1;
int mousebstrafe      = 1;  int mousebstrafe2      = -1;
int mousebstrafeleft  = -1; int mousebstrafeleft2  = -1;
int mousebstraferight = -1; int mousebstraferight2 = -1;
int mousebprevweapon  = 4;  int mousebprevweapon2  = -1;
int mousebnextweapon  = 3;  int mousebnextweapon2  = -1;


// Control whether if a mouse button is double clicked,
// it acts like "use" has been pressed.

int mouse_dclick_use = 1;

//
// Joystick controls
//

int joybfire = 0;
int joybstrafe = 1;
int joybuse = 3;
int joybspeed = 29;
int joybstrafeleft = -1;
int joybstraferight = -1;
int joybprevweapon = -1;
int joybnextweapon = -1;
int joybmenu = -1;
int joybautomap = -1;


// 
// Bind all of the common controls used by Doom and all other games.
//

void M_BindControls (void)
{
    //
    // Keyboard controls
    //

    // Movement

    M_BindIntVariableKeybind("key_up",          &key_up,          "key_up2",          &key_up2);
    M_BindIntVariableKeybind("key_down",        &key_down,        "key_down2",        &key_down2);
    M_BindIntVariableKeybind("key_right",       &key_right,       "key_right2",       &key_right2);
    M_BindIntVariableKeybind("key_left",        &key_left,        "key_left2",        &key_left2);
    M_BindIntVariableKeybind("key_strafeleft",  &key_strafeleft,  "key_strafeleft2",  &key_strafeleft2);
    M_BindIntVariableKeybind("key_straferight", &key_straferight, "key_straferight2", &key_straferight2);
    M_BindIntVariableKeybind("key_speed",       &key_speed,       "key_speed2",       &key_speed2);
    M_BindIntVariableKeybind("key_strafe",      &key_strafe,      "key_strafe2",      &key_strafe2);
    M_BindIntVariableKeybind("key_180turn",     &key_180turn,     "key_180turn2",     &key_180turn2);

    // Action

    M_BindIntVariableKeybind("key_fire", &key_fire, "key_fire2", &key_fire2);
    M_BindIntVariableKeybind("key_use",  &key_use,  "key_use2",  &key_use2);

    // Advanced movement

    M_BindIntVariable("always_run", &always_run);
    M_BindIntVariableKeybind("key_autorun",    &key_autorun,    "key_autorun2",    &key_autorun2); // [crispy]
    M_BindIntVariableKeybind("key_mouse_look", &key_mouse_look, "key_mouse_look2", &key_mouse_look2);
    M_BindIntVariableKeybind("key_novert",     &key_novert,     "key_novert2",     &key_novert2);

    // Special keys

    M_BindIntVariableKeybind("key_reloadlevel",   &key_reloadlevel,   "key_reloadlevel2",   &key_reloadlevel2); // [crispy]
    M_BindIntVariableKeybind("key_nextlevel",     &key_nextlevel,     "key_nextlevel2",     &key_nextlevel2);   // [crispy]
    M_BindIntVariableKeybind("key_flip_levels",   &key_flip_levels,   "key_flip_levels2",   &key_flip_levels2); // [crispy]
    M_BindIntVariableKeybind("key_widget_enable", &key_widget_enable, "key_widget_enable2", &key_widget_enable2);

    // RestlessRodent -- CRL (Special modes)

    M_BindIntVariableKeybind("key_spectator",   &key_spectator,   "key_spectator2",   &key_spectator2); // RestlessRodent -- CRL 
    M_BindIntVariableKeybind("key_freeze",      &key_freeze,      "key_freeze2",      &key_freeze2);
    M_BindIntVariableKeybind("key_notarget",    &key_notarget,    "key_notarget2",    &key_notarget2);
    M_BindIntVariableKeybind("key_buddha",      &key_buddha,      "key_buddha2",      &key_buddha2);

    // Weapons

    M_BindIntVariableKeybind("key_weapon1",    &key_weapon1,    "key_weapon1_2",   &key_weapon1_2);
    M_BindIntVariableKeybind("key_weapon2",    &key_weapon2,    "key_weapon2_2",   &key_weapon2_2);
    M_BindIntVariableKeybind("key_weapon3",    &key_weapon3,    "key_weapon3_2",   &key_weapon3_2);
    M_BindIntVariableKeybind("key_weapon4",    &key_weapon4,    "key_weapon4_2",   &key_weapon4_2);
    M_BindIntVariableKeybind("key_weapon5",    &key_weapon5,    "key_weapon5_2",   &key_weapon5_2);
    M_BindIntVariableKeybind("key_weapon6",    &key_weapon6,    "key_weapon6_2",   &key_weapon6_2);
    M_BindIntVariableKeybind("key_weapon7",    &key_weapon7,    "key_weapon7_2",   &key_weapon7_2);
    M_BindIntVariableKeybind("key_weapon8",    &key_weapon8,    "key_weapon8_2",   &key_weapon8_2);
    M_BindIntVariableKeybind("key_prevweapon", &key_prevweapon, "key_prevweapon2", &key_prevweapon2);
    M_BindIntVariableKeybind("key_nextweapon", &key_nextweapon, "key_nextweapon2", &key_nextweapon2);

    // Automap

    M_BindIntVariableKeybind("key_map_toggle",    &key_map_toggle,    "key_map_toggle2",    &key_map_toggle2);
    M_BindIntVariableKeybind("key_map_zoomin",    &key_map_zoomin,    "key_map_zoomin2",    &key_map_zoomin2);
    M_BindIntVariableKeybind("key_map_zoomout",   &key_map_zoomout,   "key_map_zoomout2",   &key_map_zoomout2);
    M_BindIntVariableKeybind("key_map_maxzoom",   &key_map_maxzoom,   "key_map_maxzoom2",   &key_map_maxzoom2);
    M_BindIntVariableKeybind("key_map_follow",    &key_map_follow,    "key_map_follow2",    &key_map_follow2);
    M_BindIntVariableKeybind("key_map_rotate",    &key_map_rotate,    "key_map_rotate2",    &key_map_rotate2);
    M_BindIntVariableKeybind("key_map_overlay",   &key_map_overlay,   "key_map_overlay2",   &key_map_overlay2);
    M_BindIntVariableKeybind("key_map_mousepan",  &key_map_mousepan,  "key_map_mousepan2",  &key_map_mousepan2);
    M_BindIntVariableKeybind("key_map_grid",      &key_map_grid,      "key_map_grid2",      &key_map_grid2);
    M_BindIntVariableKeybind("key_map_mark",      &key_map_mark,      "key_map_mark2",      &key_map_mark2);
    M_BindIntVariableKeybind("key_map_clearmark", &key_map_clearmark, "key_map_clearmark2", &key_map_clearmark2);
    M_BindIntVariable("key_map_north",          &key_map_north);
    M_BindIntVariable("key_map_south",          &key_map_south);
    M_BindIntVariable("key_map_east",           &key_map_east);
    M_BindIntVariable("key_map_west",           &key_map_west);

    // Function keys

    M_BindIntVariableKeybind("key_menu_help",     &key_menu_help,     "key_menu_help2",     &key_menu_help2);
    M_BindIntVariableKeybind("key_menu_save",     &key_menu_save,     "key_menu_save2",     &key_menu_save2);
    M_BindIntVariableKeybind("key_menu_load",     &key_menu_load,     "key_menu_load2",     &key_menu_load2);
    M_BindIntVariableKeybind("key_menu_volume",   &key_menu_volume,   "key_menu_volume2",   &key_menu_volume2);
    M_BindIntVariableKeybind("key_menu_detail",   &key_menu_detail,   "key_menu_detail2",   &key_menu_detail2);
    M_BindIntVariableKeybind("key_menu_qsave",    &key_menu_qsave,    "key_menu_qsave2",    &key_menu_qsave2);
    M_BindIntVariableKeybind("key_menu_endgame",  &key_menu_endgame,  "key_menu_endgame2",  &key_menu_endgame2);
    M_BindIntVariableKeybind("key_menu_messages", &key_menu_messages, "key_menu_messages2", &key_menu_messages2);
    M_BindIntVariableKeybind("key_menu_qload",    &key_menu_qload,    "key_menu_qload2",    &key_menu_qload2);
    M_BindIntVariableKeybind("key_menu_quit",     &key_menu_quit,     "key_menu_quit2",     &key_menu_quit2);
    M_BindIntVariableKeybind("key_menu_gammad",   &key_menu_gammad,   "key_menu_gammad2",   &key_menu_gammad2);
    M_BindIntVariableKeybind("key_menu_gamma",    &key_menu_gamma,    "key_menu_gamma2",    &key_menu_gamma2);
    M_BindIntVariableKeybind("key_menu_palette",  &key_menu_palette,  "key_menu_palette2",  &key_menu_palette2);

    // Shortcut keys

    M_BindIntVariableKeybind("key_pause",           &key_pause,           "key_pause2",           &key_pause2);
    M_BindIntVariableKeybind("key_menu_screenshot", &key_menu_screenshot, "key_menu_screenshot2", &key_menu_screenshot2);
#ifdef _WIN32
    // [JN] Pressing PrintScreen on Windows 11 opens the Snipping Tool.
    // Re-register PrintScreen key pressing for port needs to avoid this.
    // Taken from DOOM Retro.
    if (key_menu_screenshot == KEY_PRTSCR)
    {
        RegisterHotKey(NULL, 1, MOD_ALT, VK_SNAPSHOT);
        RegisterHotKey(NULL, 2, 0, VK_SNAPSHOT);
    }
#endif

    // Special menu keys, not available for rebinding

    M_BindIntVariable("key_menu_activate",      &key_menu_activate);
    M_BindIntVariable("key_menu_up",            &key_menu_up);
    M_BindIntVariable("key_menu_down",          &key_menu_down);
    M_BindIntVariable("key_menu_left",          &key_menu_left);
    M_BindIntVariable("key_menu_right",         &key_menu_right);
    M_BindIntVariable("key_menu_back",          &key_menu_back);
    M_BindIntVariable("key_menu_forward",       &key_menu_forward);
    M_BindIntVariable("key_menu_confirm",       &key_menu_confirm);
    M_BindIntVariable("key_menu_abort",         &key_menu_abort);
    M_BindIntVariable("key_menu_incscreen",     &key_menu_incscreen);
    M_BindIntVariable("key_menu_decscreen",     &key_menu_decscreen);

    //
    // Mouse controls
    //

    M_BindIntVariableKeybind("mouseb_fire",        &mousebfire,        "mouseb_fire2",        &mousebfire2);
    M_BindIntVariableKeybind("mouseb_forward",     &mousebforward,     "mouseb_forward2",     &mousebforward2);
    M_BindIntVariableKeybind("mouseb_backward",    &mousebbackward,    "mouseb_backward2",    &mousebbackward2);
    M_BindIntVariableKeybind("mouseb_use",         &mousebuse,         "mouseb_use2",         &mousebuse2);
    M_BindIntVariableKeybind("mouseb_speed",       &mousebspeed,       "mouseb_speed2",       &mousebspeed2);
    M_BindIntVariableKeybind("mouseb_strafe",      &mousebstrafe,      "mouseb_strafe2",      &mousebstrafe2);
    M_BindIntVariableKeybind("mouseb_strafeleft",  &mousebstrafeleft,  "mouseb_strafeleft2",  &mousebstrafeleft2);
    M_BindIntVariableKeybind("mouseb_straferight", &mousebstraferight, "mouseb_straferight2", &mousebstraferight2);
    M_BindIntVariableKeybind("mouseb_prevweapon",  &mousebprevweapon,  "mouseb_prevweapon2",  &mousebprevweapon2);
    M_BindIntVariableKeybind("mouseb_nextweapon",  &mousebnextweapon,  "mouseb_nextweapon2",  &mousebnextweapon2);
    M_BindIntVariable("mouse_dclick_use", &mouse_dclick_use);

    //
    // Joystick controls
    //

    M_BindIntVariable("joyb_fire",              &joybfire);
    M_BindIntVariable("joyb_strafe",            &joybstrafe);
    M_BindIntVariable("joyb_use",               &joybuse);
    M_BindIntVariable("joyb_speed",             &joybspeed);
    M_BindIntVariable("joyb_strafeleft",        &joybstrafeleft);
    M_BindIntVariable("joyb_straferight",       &joybstraferight);
    M_BindIntVariable("joyb_prevweapon",        &joybprevweapon);
    M_BindIntVariable("joyb_nextweapon",        &joybnextweapon);
    M_BindIntVariable("joyb_menu_activate",     &joybmenu);
    M_BindIntVariable("joyb_toggle_automap",    &joybautomap);
}
