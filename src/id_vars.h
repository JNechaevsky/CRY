//
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2014-2017 RestlessRodent
// Copyright(C) 2015-2018 Fabian Greffrath
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


#pragma once


// -----------------------------------------------------------------------------
// [JN] ID-specific config variables.
// -----------------------------------------------------------------------------

// System and video
extern int vid_resolution;
extern int vid_widescreen;

extern int vid_uncapped_fps;
extern int vid_fpslimit;
extern int vid_vsync;
extern int vid_showfps;
extern int vid_gamma;
extern int vid_fov;
extern int dp_screen_size;
extern int vid_screenwipe;
extern int msg_text_shadows;

extern int post_supersample;
extern int post_overglow;
extern int post_bloom;
extern int post_rgbdrift;
extern int post_vhsdist;
extern int post_vignette;
extern int post_filmgrain;
extern int post_motionblur;
extern int post_dofblur;

// Display
extern int dp_detail_level;
extern int dp_cry_palette;
extern int dp_menu_shading;
extern int dp_level_brightness;

// Messages
extern int msg_show;
extern int msg_alignment;
extern int msg_local_time;

// Game modes
extern int crl_spectating;
extern int crl_freeze;

// Widgets
extern int widget_enable;
extern int widget_location;
extern int widget_alignment;
extern int widget_coords;
extern int widget_render;
extern int widget_kis;
extern int widget_kis_format;
extern int widget_time;
extern int widget_totaltime;
extern int widget_levelname;
extern int widget_health;

// Sound
extern int snd_monosfx;
extern int snd_channels;
extern int snd_mute_inactive;

// Automap
extern int automap_smooth;
extern int automap_thick;
extern int automap_square;
extern int automap_secrets;
extern int automap_rotate;
extern int automap_overlay;
extern int automap_shading;

// Gameplay features
extern int vis_brightmaps;
extern int vis_translucency;
extern int vis_improved_fuzz;
extern int vis_colored_lighting;
extern int vis_colored_blood;
extern int vis_swirling_liquids;
extern int vis_animated_sky;
extern int vis_linear_sky;
extern int vis_flip_corpses;

extern int xhair_draw;
extern int xhair_color;

extern int st_colored_stbar;
extern int st_negative_health;

extern int phys_torque;
extern int phys_toss_drop;
extern int phys_floating_powerups;
extern int phys_weapon_alignment;

extern int aud_crushed_corpse;

extern int gp_default_skill;
extern int gp_pistol_start;
extern int gp_revealed_secrets;
extern int phys_breathing;
extern int gp_flip_levels;
extern int gp_death_use_action;

extern int emu_jaguar_music;
extern int emu_jaguar_alert;
extern int emu_jaguar_explosion;
extern int emu_jaguar_skies;

// Compatibility-breaking
extern int compat_vertical_aiming;

// Mouse look
extern int mouse_look;

extern void ID_BindVariables (void);
