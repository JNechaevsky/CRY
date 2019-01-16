//
// Copyright(C) 2005-2014 Simon Howard
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

// Russian Doom (C) 2016-2018 Julian Nechaevsky


#ifndef SETUP_COMPATIBILITY_H
#define SETUP_COMPATIBILITY_H

void CompatibilitySettings(void);
void BindCompatibilityVariables(void);


// [JN] Дополнительные параметры игры

// Графика
extern int hightlight_things;
extern int brightmaps;
extern int translucency;
extern int swirling_liquids;
extern int invul_sky;
extern int colored_blood;
extern int draw_shadowed_text;

// Звук
extern int crushed_corpses_sfx;
extern int blazing_door_fix_sfx;
extern int correct_endlevel_sfx;
extern int play_exit_sfx;
extern int noise_alert_sfx;

// Тактика
extern int automap_stats;
extern int secret_notification;
extern int negative_health;
extern int infragreen_visor;

// Физика
extern int over_under;
extern int torque;
extern int weapon_bobbing;
extern int ssg_blast_enemies;
extern int randomly_flipcorpses;
extern int floating_powerups;

// Геймплей
extern int fix_map_errors;
extern int flip_levels;
extern int extra_player_faces;
extern int unlimited_lost_souls;
extern int agressive_lost_souls;
extern int fast_quickload;
extern int no_internal_demos;

// Прицел
extern int crosshair_draw;
extern int crosshair_health;
extern int crosshair_scale;


#endif /* #ifndef SETUP_COMPATIBILITY_H */

