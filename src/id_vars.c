//
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2014-2017 RestlessRodent
// Copyright(C) 2015-2018 Fabian Greffrath
// Copyright(C) 2016-2024 Julia Nechaevskaya
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


#include "m_config.h"  // [JN] M_BindIntVariable

#include "id_vars.h"


// Game modes
int crl_spectating = 0;  // RestlessRodent -- CRL
int crl_freeze = 0;


// -----------------------------------------------------------------------------
// [JN] ID-specific config variables.
// -----------------------------------------------------------------------------

//
// Video options
//

int vid_resolution = 1;
int vid_widescreen = 0;
int vid_uncapped_fps = 0;
int vid_fpslimit = 0;
int vid_vsync = 1;
int vid_showfps = 0;
int vid_smooth_scaling = 0;
int vid_screenwipe = 1;

//
// Display options
//

int dp_screen_size = 10;
int dp_detail_level = 0;  // Blocky mode (0 = high, 1 = normal)
int dp_cry_palette = 1;
int vid_gamma = 10;
int vid_fov = 90;
int dp_menu_shading = 0;
int dp_level_brightness = 0;
// Messages Settings
int msg_show = 1;
int msg_alignment = 0;
int msg_text_shadows = 0;
int msg_local_time = 0;

//
// Sound options
//

int snd_monosfx = 0;
int snd_channels = 8;
int snd_mute_inactive = 0;

//
// Control settings
//

int mouse_look = 0;

//
// Widgets and automap
//

int widget_enable = 1;
int widget_location = 0;
int widget_kis = 0;
int widget_time = 0;
int widget_totaltime = 0;
int widget_levelname = 0;
int widget_coords = 0;
int widget_render = 0;
int widget_health = 0;
// Automap
int automap_smooth = 0;
int automap_secrets = 0;
int automap_rotate = 0;
int automap_overlay = 0;
int automap_shading = 0;

//
// Gameplay features
//

// Visual
int vis_brightmaps = 0;
int vis_translucency = 0;
int vis_improved_fuzz = 0;
int vis_colored_lighting = 0;
int vis_colored_blood = 0;
int vis_swirling_liquids = 0;
int vis_animated_sky = 0;
int vis_linear_sky = 0;
int vis_flip_corpses = 0;

// Crosshair
int xhair_draw = 0;
int xhair_color = 0;

// Status bar
int st_colored_stbar = 0;
int st_negative_health = 0;

// Physical
int phys_torque = 0;
int phys_toss_drop = 0;
int phys_floating_powerups = 0;
int phys_weapon_alignment = 2;
int phys_breathing = 0;

// Accuracy
int acc_jaguar_alert = 1;
int acc_jaguar_explosion = 1;

// Gameplay
int gp_default_skill = 2;
int gp_pistol_start = 0;
int gp_revealed_secrets = 0;
int gp_flip_levels = 0;
int gp_death_use_action = 0;

// Compatibility-breaking
int compat_blockmap_fix = 0;
int compat_vertical_aiming = 0;


// -----------------------------------------------------------------------------
// [JN] ID-specific config variables binding functions.
// -----------------------------------------------------------------------------

void ID_BindVariables (void)
{
    //
    // Video options
    //

    M_BindIntVariable("vid_resolution",                 &vid_resolution);
    M_BindIntVariable("vid_widescreen",                 &vid_widescreen);
    M_BindIntVariable("vid_uncapped_fps",               &vid_uncapped_fps);
    M_BindIntVariable("vid_fpslimit",                   &vid_fpslimit);
    M_BindIntVariable("vid_vsync",                      &vid_vsync);
    M_BindIntVariable("vid_showfps",                    &vid_showfps);
    M_BindIntVariable("vid_smooth_scaling",             &vid_smooth_scaling);
    M_BindIntVariable("vid_screenwipe",                 &vid_screenwipe);

    //
    // Display options
    //

    M_BindIntVariable("dp_screen_size",                 &dp_screen_size);
    M_BindIntVariable("dp_detail_level",                &dp_detail_level);
    M_BindIntVariable("dp_cry_palette",                 &dp_cry_palette);
    M_BindIntVariable("vid_gamma",                      &vid_gamma);
    M_BindIntVariable("vid_fov",                        &vid_fov);
    M_BindIntVariable("dp_menu_shading",                &dp_menu_shading);
    M_BindIntVariable("dp_level_brightness",            &dp_level_brightness);
    // Messages Settings
    M_BindIntVariable("msg_show",                       &msg_show);
    M_BindIntVariable("msg_alignment",                  &msg_alignment);
    M_BindIntVariable("msg_text_shadows",               &msg_text_shadows);
    M_BindIntVariable("msg_local_time",                 &msg_local_time);    

    //
    // Sound options
    //

    M_BindIntVariable("snd_monosfx",                    &snd_monosfx);
    M_BindIntVariable("snd_channels",                   &snd_channels);
    M_BindIntVariable("snd_mute_inactive",              &snd_mute_inactive);

    //
    // Control settings
    //

    M_BindIntVariable("mouse_look",                     &mouse_look);

    //
    // Widgets and automap
    //

    M_BindIntVariable("widget_enable",                  &widget_enable);
    M_BindIntVariable("widget_location",                &widget_location);
    M_BindIntVariable("widget_kis",                     &widget_kis);
    M_BindIntVariable("widget_time",                    &widget_time);
    M_BindIntVariable("widget_totaltime",               &widget_totaltime);
    M_BindIntVariable("widget_levelname",               &widget_levelname);
    M_BindIntVariable("widget_coords",                  &widget_coords);
    M_BindIntVariable("widget_render",                  &widget_render);
    M_BindIntVariable("widget_health",                  &widget_health);
    // Automap
    M_BindIntVariable("automap_smooth",                 &automap_smooth);
    M_BindIntVariable("automap_secrets",                &automap_secrets);
    M_BindIntVariable("automap_rotate",                 &automap_rotate);
    M_BindIntVariable("automap_overlay",                &automap_overlay);
    M_BindIntVariable("automap_shading",                &automap_shading);

    //
    // Gameplay features
    //

    // Visual
    M_BindIntVariable("vis_brightmaps",                 &vis_brightmaps);
    M_BindIntVariable("vis_translucency",               &vis_translucency);
    M_BindIntVariable("vis_improved_fuzz",              &vis_improved_fuzz);
    M_BindIntVariable("vis_colored_lighting",           &vis_colored_lighting);
    M_BindIntVariable("vis_colored_blood",              &vis_colored_blood);
    M_BindIntVariable("vis_swirling_liquids",           &vis_swirling_liquids);
    M_BindIntVariable("vis_animated_sky",               &vis_animated_sky);
    M_BindIntVariable("vis_linear_sky",                 &vis_linear_sky);
    M_BindIntVariable("vis_flip_corpses",               &vis_flip_corpses);
    
    // Crosshair
    M_BindIntVariable("xhair_draw",                     &xhair_draw);
    M_BindIntVariable("xhair_color",                    &xhair_color);
    
    // Monsters
    M_BindIntVariable("acc_jaguar_alert",               &acc_jaguar_alert);
    M_BindIntVariable("acc_jaguar_explosion",           &acc_jaguar_explosion);

    // Status bar
    M_BindIntVariable("st_colored_stbar",               &st_colored_stbar);
    M_BindIntVariable("st_negative_health",             &st_negative_health);
    
    // Physical
    M_BindIntVariable("phys_torque",                    &phys_torque);
    M_BindIntVariable("phys_toss_drop",                 &phys_toss_drop);
    M_BindIntVariable("phys_floating_powerups",         &phys_floating_powerups);
    M_BindIntVariable("phys_weapon_alignment",          &phys_weapon_alignment);
    M_BindIntVariable("phys_breathing",                 &phys_breathing);
    
    // Gameplay
    M_BindIntVariable("gp_default_skill",               &gp_default_skill);
    M_BindIntVariable("gp_pistol_start",                &gp_pistol_start);
    M_BindIntVariable("gp_revealed_secrets",            &gp_revealed_secrets);
    M_BindIntVariable("gp_flip_levels",                 &gp_flip_levels);
    M_BindIntVariable("gp_death_use_action",            &gp_death_use_action);
    
    M_BindIntVariable("compat_blockmap_fix",            &compat_blockmap_fix);
    M_BindIntVariable("compat_vertical_aiming",         &compat_vertical_aiming);
}
