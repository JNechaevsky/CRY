//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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

#include <stdio.h>

#include "doomdef.h"
#include "g_game.h"
#include "i_system.h"
#include "p_local.h"
#include "s_sound.h"


//
// CHANGE THE TEXTURE OF A WALL SWITCH TO ITS OPPOSITE
//

// [crispy] remove MAXSWITCHES limit
int           *switchlist;
int            numswitches;
static size_t  maxswitches;

button_t      *buttonlist; // [crispy] remove MAXBUTTONS limit
int            maxbuttons; // [crispy] remove MAXBUTTONS limit

static const switchlist_t alphSwitchList_vanilla[] =
{
    // [JN] Jaguar: all available switches.
    {"SW1BRN1",  "SW2BRN1",  1},
    {"SW1GARG",  "SW2GARG",  1},
    {"SW1GSTON", "SW2GSTON", 1},
    {"SW1HOT",   "SW2HOT",   1},
    {"SW1HOT",   "SW2HOT",   1},
    {"SW1STAR",  "SW2STAR",  1},
    {"SW1WOOD",  "SW2WOOD",  1},
};

// -----------------------------------------------------------------------------
// P_InitSwitchList
// Only called at game initialization.
// -----------------------------------------------------------------------------

void P_InitSwitchList(void)
{
    int slindex = 0;

    for (int i = 0; alphSwitchList_vanilla[i].episode; i++)
    {
        // [crispy] remove MAXSWITCHES limit
        if (slindex + 1 >= maxswitches)
        {
            size_t newmax = maxswitches ? 2 * maxswitches : MAXSWITCHES;
            switchlist = I_Realloc(switchlist, newmax * sizeof(*switchlist));
            maxswitches = newmax;
        }

        {
            const char *name1 = alphSwitchList_vanilla[i].name1;
            const char *name2 = alphSwitchList_vanilla[i].name2;
            const int texture1 = R_CheckTextureNumForName(name1);
            const int texture2 = R_CheckTextureNumForName(name2);

            switchlist[slindex++] = texture1;
            switchlist[slindex++] = texture2;
        }
    }

    numswitches = slindex / 2;
    switchlist[slindex] = -1;

    // [crispy] pre-allocate some memory for the buttonlist[] array
    buttonlist = I_Realloc(NULL, sizeof(*buttonlist) * (maxbuttons = MAXBUTTONS));
    memset(buttonlist, 0, sizeof(*buttonlist) * maxbuttons);
}

// -----------------------------------------------------------------------------
// P_StartButton
// Start a button counting down till it turns off.
// -----------------------------------------------------------------------------

void P_StartButton (line_t *line, bwhere_e w, int texture, int time)
{
    int i;

    // See if button is already pressed
    for (i = 0 ; i < maxbuttons ; i++)
    {
        if (buttonlist[i].btimer && buttonlist[i].line == line)
        {
            // [crispy] register up to three buttons at once for lines
            // with more than one switch texture
            if (buttonlist[i].where == w)
            {
                return;
            }
        }
    }

    for (i = 0 ; i < maxbuttons ; i++)
    {
        if (!buttonlist[i].btimer)
        {
            buttonlist[i].line = line;
            buttonlist[i].where = w;
            buttonlist[i].btexture = texture;
            buttonlist[i].btimer = time;
            buttonlist[i].soundorg = &line->soundorg; // [crispy] corrected sound source
            return;
        }
    }

    // [crispy] remove MAXBUTTONS limit
    {
        maxbuttons = 2 * maxbuttons;
        buttonlist = I_Realloc(buttonlist, sizeof(*buttonlist) * maxbuttons);
        memset(buttonlist + maxbuttons/2, 0, sizeof(*buttonlist) * maxbuttons/2);
        P_StartButton(line, w, texture, time);
        // [JN] Separate return to fix -Wpedantic GCC compiler warning:
        // ISO C forbids 'return' with expression, in function returning void
        return;
    }

    I_Error("P_StartButton: no button slots left!");
}

// -----------------------------------------------------------------------------
// P_ChangeSwitchTexture
// Function that changes wall texture.
// Tell it if switch is ok to use again (1=yes, it's a button).
// -----------------------------------------------------------------------------

void P_ChangeSwitchTexture (line_t *line, int useAgain)
{
    int     texTop;
    int     texMid;
    int     texBot;
    int     i;
    int     sound;
    boolean playsound = false;

    if (!useAgain)
    {
        line->special = 0;
    }

    texTop = sides[line->sidenum[0]].toptexture;
    texMid = sides[line->sidenum[0]].midtexture;
    texBot = sides[line->sidenum[0]].bottomtexture;

    sound = sfx_swtchn;

    // EXIT SWITCH?
    if (line->special == 11)
    {
        sound = sfx_swtchx;
    }

    for (i = 0 ; i < numswitches*2 ; i++)
    {
        if (switchlist[i] == texTop)
        {
            playsound = true;
            sides[line->sidenum[0]].toptexture = switchlist[i^1];
            if (useAgain)
            {
                P_StartButton(line,top,switchlist[i],BUTTONTIME);
            }
        }
        // [crispy] register up to three buttons at once for lines with
        // more than one switch texture
        if (switchlist[i] == texMid)
        {
            playsound = true;
            sides[line->sidenum[0]].midtexture = switchlist[i^1];
            if (useAgain)
            {
                P_StartButton(line, middle,switchlist[i],BUTTONTIME);
            }
        }
        // [crispy] register up to three buttons at once for lines with
        // more than one switch texture
        {
            if (switchlist[i] == texBot)
            {
                playsound = true;
                sides[line->sidenum[0]].bottomtexture = switchlist[i^1];
                if (useAgain)
                {
                    P_StartButton(line, bottom,switchlist[i],BUTTONTIME);
                }
            }
        }
    }

    // [crispy] corrected sound source
    if (playsound)
    {
        S_StartSound(&line->soundorg,sound);
    }
}

// -----------------------------------------------------------------------------
// P_UseSpecialLine
// Called when a thing uses a special line.
// Only the front sides of lines are usable.
// -----------------------------------------------------------------------------

boolean P_UseSpecialLine (mobj_t *thing, line_t *line, int side)
{
    // Switches that other things can activate.
    if (!thing->player)
    {
        // never open secret doors
        if (line->flags & ML_SECRET)
        {
            return false;
        }

        switch(line->special)
        {
            case 1: 	// MANUAL DOOR RAISE
            case 32:	// MANUAL BLUE
            case 33:	// MANUAL RED
            case 34:	// MANUAL YELLOW
            break;

            default:
            return false;
            break;
        }
    }
    
    // do something  
    switch (line->special)
    {
        // MANUALS
        case 1:     // Vertical Door
        case 26:    // Blue Door/Locked
        case 27:    // Yellow Door /Locked
        case 28:    // Red Door /Locked

        case 31:    // Manual door open
        case 32:    // Blue locked door open
        case 33:    // Red locked door open
        case 34:    // Yellow locked door open

        case 117:   // Blazing door raise
        case 118:   // Blazing door open
            EV_VerticalDoor (line, thing);
        break;
	
        // [JN] Non-switches, just play an "oof" sound by pressing "use":
        case 24:    // Floor Raise to Lowest Ceiling
        case 46:    // Open Door Impact
        case 47:    // Floor Raise to Next Higher Floor (changes texture)
        case 48:    // Scroll Texture Left
        case 85:    // Scroll Texture Right (BOOM)
            if (PTR_NoWayAudible(line))
                S_StartSound(NULL, sfx_oof);
        break;

        // SWITCHES
        case 7:
        // Build Stairs
        if (EV_BuildStairs(line,build8))
            P_ChangeSwitchTexture(line,0);
        break;

        case 9:
        // Change Donut
        if (EV_DoDonut(line))
            P_ChangeSwitchTexture(line,0);
        break;
	
        case 11:
        // Exit level
        P_ChangeSwitchTexture(line,0);
        G_ExitLevel ();
        break;

        case 14:
        // Raise Floor 32 and change texture
        if (EV_DoPlat(line,raiseAndChange,32))
            P_ChangeSwitchTexture(line,0);
        break;

        case 15:
        // Raise Floor 24 and change texture
        if (EV_DoPlat(line,raiseAndChange,24))
            P_ChangeSwitchTexture(line,0);
        break;

        case 18:
        // Raise Floor to next highest floor
        if (EV_DoFloor(line, raiseFloorToNearest))
            P_ChangeSwitchTexture(line,0);
        break;

        case 20:
        // Raise Plat next highest floor and change texture
        if (EV_DoPlat(line,raiseToNearestAndChange,0))
            P_ChangeSwitchTexture(line,0);
        break;

        case 21:
        // PlatDownWaitUpStay
        if (EV_DoPlat(line,downWaitUpStay,0))
            P_ChangeSwitchTexture(line,0);
        break;

        case 23:
        // Lower Floor to Lowest
        if (EV_DoFloor(line,lowerFloorToLowest))
            P_ChangeSwitchTexture(line,0);
        break;

        case 29:
        // Raise Door
        if (EV_DoDoor(line,vld_normal))
            P_ChangeSwitchTexture(line,0);
        break;

        case 41:
        // Lower Ceiling to Floor
        if (EV_DoCeiling(line,lowerToFloor))
            P_ChangeSwitchTexture(line,0);
        break;

        case 71:
        // Turbo Lower Floor
        if (EV_DoFloor(line,turboLower))
            P_ChangeSwitchTexture(line,0);
        break;

        case 49:
        // Ceiling Crush And Raise
        if (EV_DoCeiling(line,crushAndRaise))
            P_ChangeSwitchTexture(line,0);
        break;

        case 50:
        // Close Door
        if (EV_DoDoor(line,vld_close))
            P_ChangeSwitchTexture(line,0);
        break;

        case 51:
        // Secret EXIT
        P_ChangeSwitchTexture(line,0);
        G_SecretExitLevel ();
        break;

        case 55:
        // Raise Floor Crush
        if (EV_DoFloor(line,raiseFloorCrush))
            P_ChangeSwitchTexture(line,0);
        break;

        case 101:
        // Raise Floor
        if (EV_DoFloor(line,raiseFloor))
            P_ChangeSwitchTexture(line,0);
        break;

        case 102:
        // Lower Floor to Surrounding floor height
        if (EV_DoFloor(line,lowerFloor))
            P_ChangeSwitchTexture(line,0);
        break;

        case 103:
        // Open Door
        if (EV_DoDoor(line,vld_open))
            P_ChangeSwitchTexture(line,0);
        break;

        case 111:
        // Blazing Door Raise (faster than TURBO!)
        if (EV_DoDoor (line,vld_blazeRaise))
            P_ChangeSwitchTexture(line,0);
        break;

        case 112:
        // Blazing Door Open (faster than TURBO!)
        if (EV_DoDoor (line,vld_blazeOpen))
            P_ChangeSwitchTexture(line,0);
        break;

        case 113:
        // Blazing Door Close (faster than TURBO!)
        if (EV_DoDoor (line,vld_blazeClose))
            P_ChangeSwitchTexture(line,0);
        break;

        case 122:
        // Blazing PlatDownWaitUpStay
        if (EV_DoPlat(line,blazeDWUS,0))
            P_ChangeSwitchTexture(line,0);
        break;

        case 127:
        // Build Stairs Turbo 16
        if (EV_BuildStairs(line,turbo16))
            P_ChangeSwitchTexture(line,0);
        break;

        case 131:
        // Raise Floor Turbo
        if (EV_DoFloor(line,raiseFloorTurbo))
            P_ChangeSwitchTexture(line,0);
        break;

        case 133:
        // BlzOpenDoor BLUE
        case 135:
        // BlzOpenDoor RED
        case 137:
        // BlzOpenDoor YELLOW
        if (EV_DoLockedDoor (line,vld_blazeOpen,thing))
            P_ChangeSwitchTexture(line,0);
        break;

        case 140:
        // Raise Floor 512
        if (EV_DoFloor(line,raiseFloor512))
            P_ChangeSwitchTexture(line,0);
        break;

        // BUTTONS
        case 42:
        // Close Door
        if (EV_DoDoor(line,vld_close))
            P_ChangeSwitchTexture(line,1);
        break;

        case 43:
        // Lower Ceiling to Floor
        if (EV_DoCeiling(line,lowerToFloor))
            P_ChangeSwitchTexture(line,1);
        break;

        case 45:
        // Lower Floor to Surrounding floor height
        if (EV_DoFloor(line,lowerFloor))
            P_ChangeSwitchTexture(line,1);
        break;

        case 60:
        // Lower Floor to Lowest
        if (EV_DoFloor(line,lowerFloorToLowest))
            P_ChangeSwitchTexture(line,1);
        break;

        case 61:
        // Open Door
        if (EV_DoDoor(line,vld_open))
            P_ChangeSwitchTexture(line,1);
        break;

        case 62:
        // PlatDownWaitUpStay
        if (EV_DoPlat(line,downWaitUpStay,1))
            P_ChangeSwitchTexture(line,1);
        break;

        case 63:
        // Raise Door
        if (EV_DoDoor(line,vld_normal))
            P_ChangeSwitchTexture(line,1);
        break;

        case 64:
        // Raise Floor to ceiling
        if (EV_DoFloor(line,raiseFloor))
            P_ChangeSwitchTexture(line,1);
        break;

        case 66:
        // Raise Floor 24 and change texture
        if (EV_DoPlat(line,raiseAndChange,24))
            P_ChangeSwitchTexture(line,1);
        break;

        case 67:
        // Raise Floor 32 and change texture
        if (EV_DoPlat(line,raiseAndChange,32))
            P_ChangeSwitchTexture(line,1);
        break;

        case 65:
        // Raise Floor Crush
        if (EV_DoFloor(line,raiseFloorCrush))
            P_ChangeSwitchTexture(line,1);
        break;

        case 68:
        // Raise Plat to next highest floor and change texture
        if (EV_DoPlat(line,raiseToNearestAndChange,0))
            P_ChangeSwitchTexture(line,1);
        break;

        case 69:
        // Raise Floor to next highest floor
        if (EV_DoFloor(line, raiseFloorToNearest))
            P_ChangeSwitchTexture(line,1);
        break;

        case 70:
        // Turbo Lower Floor
        if (EV_DoFloor(line,turboLower))
            P_ChangeSwitchTexture(line,1);
        break;

        case 114:
        // Blazing Door Raise (faster than TURBO!)
        if (EV_DoDoor (line,vld_blazeRaise))
            P_ChangeSwitchTexture(line,1);
        break;

        case 115:
        // Blazing Door Open (faster than TURBO!)
        if (EV_DoDoor (line,vld_blazeOpen))
            P_ChangeSwitchTexture(line,1);
        break;

        case 116:
        // Blazing Door Close (faster than TURBO!)
        if (EV_DoDoor (line,vld_blazeClose))
            P_ChangeSwitchTexture(line,1);
        break;

        case 123:
        // Blazing PlatDownWaitUpStay
        if (EV_DoPlat(line,blazeDWUS,0))
            P_ChangeSwitchTexture(line,1);
        break;

        case 132:
        // Raise Floor Turbo
        if (EV_DoFloor(line,raiseFloorTurbo))
            P_ChangeSwitchTexture(line,1);
        break;

        case 99:
        // BlzOpenDoor BLUE
        case 134:
        // BlzOpenDoor RED
        case 136:
        // BlzOpenDoor YELLOW
        if (EV_DoLockedDoor (line,vld_blazeOpen,thing))
            P_ChangeSwitchTexture(line,1);
        break;

        case 138:
        // Light Turn On
        EV_LightTurnOn(line,255);
        P_ChangeSwitchTexture(line,1);
        break;

        case 139:
        // Light Turn Off
        EV_LightTurnOn(line,35);
        P_ChangeSwitchTexture(line,1);
        break;
    }

    return true;
}
