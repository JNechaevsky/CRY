//
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

#include "m_config.h"
#include "textscreen.h"
#include "mode.h"
#include "compatibility.h"

#define WINDOW_HELP_URL "http://jnechaevsky.users.sourceforge.net/projects/rusdoom/setup/gameplay.html"


// [JN] Дополнительные параметры игры

// Графика
int hightlight_things = 0;
int brightmaps = 1;
int translucency = 1;
int swirling_liquids = 1;
int invul_sky = 1;
int colored_blood = 1;
int draw_shadowed_text = 1;

// Звук
int crushed_corpses_sfx = 1;
int blazing_door_fix_sfx = 1;
int correct_endlevel_sfx = 0;
int play_exit_sfx = 1;
int noise_alert_sfx = 0;

// Тактика
int automap_stats = 1;
int secret_notification = 1;
int negative_health = 0;
int infragreen_visor = 0;

// Физика
int over_under = 0;
int torque = 1;
int weapon_bobbing = 1;
int ssg_blast_enemies = 1;
int randomly_flipcorpses = 1;
int floating_powerups = 0;

// Геймплей
int fix_map_errors = 1;
int flip_levels = 0;
int extra_player_faces = 1;
int unlimited_lost_souls = 1;
int agressive_lost_souls = 0;
int fast_quickload = 1;
int no_internal_demos = 0;

// Прицел
int crosshair_draw = 0;


void CompatibilitySettings(void)
{
    txt_window_t *window;
    txt_table_t  *window_features;

    window = TXT_NewWindow("Optional Gameplay Enhacements");
    window_features = TXT_NewTable(1);

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, TXT_NewWindowAbortAction(window));
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, TXT_NewWindowSelectAction(window));

    TXT_AddWidget(window, TXT_NewScrollPane(47, 6, window_features));
    
    TXT_AddWidgets(window_features,

        TXT_NewCheckBox("Hightlighted things",          &hightlight_things),
        TXT_NewCheckBox("Brightmaps",                   &brightmaps),
        TXT_NewCheckBox("Translucency",                 &translucency),
        TXT_NewCheckBox("Swirling liquids",             &swirling_liquids),
        TXT_NewCheckBox("Colored blood and corpses",    &colored_blood),
        TXT_NewCheckBox("Improved weapon bobbing",      &weapon_bobbing),

    NULL);
}

void BindCompatibilityVariables(void)
{
    // Графика
    M_BindIntVariable("hightlight_things",      &hightlight_things);
    M_BindIntVariable("brightmaps",             &brightmaps);
    M_BindIntVariable("translucency",           &translucency);
    M_BindIntVariable("swirling_liquids",       &swirling_liquids);
    M_BindIntVariable("invul_sky",              &invul_sky);
    M_BindIntVariable("colored_blood",          &colored_blood);
    M_BindIntVariable("draw_shadowed_text",     &draw_shadowed_text);

    // Звук
    M_BindIntVariable("crushed_corpses_sfx",    &crushed_corpses_sfx);
    M_BindIntVariable("blazing_door_fix_sfx",   &blazing_door_fix_sfx);
    M_BindIntVariable("play_exit_sfx",          &play_exit_sfx);
    M_BindIntVariable("correct_endlevel_sfx",   &correct_endlevel_sfx);
    M_BindIntVariable("noise_alert_sfx",        &noise_alert_sfx);

    // Тактика
    M_BindIntVariable("automap_stats",          &automap_stats);
    M_BindIntVariable("secret_notification",    &secret_notification);
    M_BindIntVariable("negative_health",        &negative_health);
    M_BindIntVariable("infragreen_visor",       &infragreen_visor);

    // Физика
    M_BindIntVariable("over_under",             &over_under);
    M_BindIntVariable("torque",                 &torque);
    M_BindIntVariable("weapon_bobbing",         &weapon_bobbing);
    M_BindIntVariable("ssg_blast_enemies",      &ssg_blast_enemies);
    M_BindIntVariable("randomly_flipcorpses",   &randomly_flipcorpses);
    M_BindIntVariable("floating_powerups",      &floating_powerups);

    // Геймплей
    M_BindIntVariable("fix_map_errors",         &fix_map_errors);
    M_BindIntVariable("flip_levels",            &flip_levels);
    M_BindIntVariable("extra_player_faces",     &extra_player_faces);
    M_BindIntVariable("unlimited_lost_souls",   &unlimited_lost_souls);
    M_BindIntVariable("agressive_lost_souls",   &agressive_lost_souls);
    M_BindIntVariable("fast_quickload",         &fast_quickload);
    M_BindIntVariable("no_internal_demos",      &no_internal_demos);

    // Прицел
    M_BindIntVariable("crosshair_draw",         &crosshair_draw);
}

