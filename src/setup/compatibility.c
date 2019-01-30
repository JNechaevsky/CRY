//
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2019 Julia Nechaevskaya
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


// -----------------------------------------------------------------------------
// [Julia] Gameplay features
// -----------------------------------------------------------------------------

int hightlight_things = 0;
int brightmaps        = 1;
int translucency      = 1;
int swirling_liquids  = 1;
int colored_blood     = 1;
int weapon_bobbing    = 1;
int crosshair_draw    = 0;


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
    M_BindIntVariable("hightlight_things",  &hightlight_things);
    M_BindIntVariable("brightmaps",         &brightmaps);
    M_BindIntVariable("translucency",       &translucency);
    M_BindIntVariable("swirling_liquids",   &swirling_liquids);
    M_BindIntVariable("colored_blood",      &colored_blood);
    M_BindIntVariable("weapon_bobbing",     &weapon_bobbing);
    M_BindIntVariable("crosshair_draw",     &crosshair_draw);
}

