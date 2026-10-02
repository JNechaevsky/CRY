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


#include <stdio.h>

#include "g_game.h"
#include "i_system.h"
#include "p_local.h"
#include "i_swap.h" // [crispy] SHORT()
#include "w_wad.h" // [crispy] W_CheckNumForName()
#include "z_zone.h" // [crispy] PU_STATIC
#include "s_sound.h"


/*================================================================== */
/* */
/*	CHANGE THE TEXTURE OF A WALL SWITCH TO ITS OPPOSITE */
/* */
/*================================================================== */

// [crispy] add support for SWITCHES lumps
static switchlist_t alphSwitchList_vanilla[] =
{
	{"SW1BRN1",		"SW2BRN1",	3},
	{"SW1GARG",		"SW2GARG",	3},
	{"SW1GSTON",	"SW2GSTON",	3},
	{"SW1HOT",		"SW2HOT",	3},
	{"SW1STAR",		"SW2STAR",	3},
	{"SW1WOOD",		"SW2WOOD",	3},

    // [crispy] SWITCHES lumps are supposed to end like this
    {"\0",		"\0",	0}
};

// [crispy] remove MAXSWITCHES limit
int			*switchlist;
int			numswitches;
static size_t	maxswitches;
button_t	*buttonlist; // [crispy] remove MAXBUTTONS limit
int		maxbuttons; // [crispy] remove MAXBUTTONS limit

/*
===============
=
= P_InitSwitchList
=
= Only called at game initialization
=
===============
*/

void P_InitSwitchList(void)
{
    int i, slindex, episode = 3;

    // [crispy] add support for SWITCHES lumps
    switchlist_t *alphSwitchList;
    boolean from_lump;

    if ((from_lump = (W_CheckNumForName("SWITCHES") != -1)))
    {
	alphSwitchList = W_CacheLumpName("SWITCHES", PU_STATIC);
    }
    else
    {
	alphSwitchList = alphSwitchList_vanilla;
    }

    // Note that this is called "episode" here but it's actually something
    // quite different. As we progress from Shareware->Registered->Doom II
    // we support more switch textures.
    // [JN] CRY - since we using MAPxx model, consider this as "commercial".
    /*
    switch (gamemode)
    {
        case registered:
        case retail:
            episode = 2;
            break;
        case commercial:
            episode = 3;
            break;
        default:
            episode = 1;
            break;
    }
    */

    slindex = 0;

    for (i = 0; alphSwitchList[i].episode; i++)
    {
	const short alphSwitchList_episode = from_lump ?
	    SHORT(alphSwitchList[i].episode) :
	    alphSwitchList[i].episode;

	// [crispy] remove MAXSWITCHES limit
	if (slindex + 1 >= maxswitches)
	{
	    size_t newmax = maxswitches ? 2 * maxswitches : MAXSWITCHES;
	    switchlist = I_Realloc(switchlist, newmax * sizeof(*switchlist));
	    maxswitches = newmax;
	}

	// [crispy] ignore switches referencing unknown texture names,
	// warn if either one is missing, but only add if both are valid
	if (alphSwitchList_episode <= episode)
	{
	    int texture1, texture2;
	    const char *name1 = alphSwitchList[i].name1;
	    const char *name2 = alphSwitchList[i].name2;

	    texture1 = R_CheckTextureNumForName(name1);
	    texture2 = R_CheckTextureNumForName(name2);

	    if (texture1 == -1 || texture2 == -1)
	    {
		fprintf(stderr, "P_InitSwitchList: could not add %s(%d)/%s(%d)\n",
		        name1, texture1, name2, texture2);
	    }
	    else
	    {
		switchlist[slindex++] = texture1;
		switchlist[slindex++] = texture2;
	    }
	}
    }

    numswitches = slindex / 2;
    switchlist[slindex] = -1;

    // [crispy] add support for SWITCHES lumps
    if (from_lump)
    {
	W_ReleaseLumpName("SWITCHES");
    }

    // [crispy] pre-allocate some memory for the buttonlist[] array
    maxbuttons = MAXBUTTONS;
    buttonlist = I_Realloc(NULL, sizeof(*buttonlist) * maxbuttons);
    memset(buttonlist, 0, sizeof(*buttonlist) * maxbuttons);
}

/*================================================================== */
/* */
/*	Start a button counting down till it turns off. */
/* */
/*================================================================== */
void P_StartButton(line_t *line,bwhere_e w,int texture,int time)
{
	int		i;
	
	for (i = 0;i < maxbuttons;i++)
		if (!buttonlist[i].btimer)
		{
			buttonlist[i].line = line;
			buttonlist[i].where = w;
			buttonlist[i].btexture = texture;
			buttonlist[i].btimer = time;
			buttonlist[i].soundorg = &line->frontsector->soundorg;
			return;
		}
		
	I_Error("P_StartButton: no button slots left!");
}

/*================================================================== */
/* */
/*	Function that changes wall texture. */
/*	Tell it if switch is ok to use again (1=yes, it's a button). */
/* */
/*================================================================== */
void P_ChangeSwitchTexture(line_t *line,int useAgain)
{
	int	texTop;
	int	texMid;
	int	texBot;
	int	i;
	int	sound;
	
	if (!useAgain)
		line->special = 0;

	texTop = sides[line->sidenum[0]].toptexture;
	texMid = sides[line->sidenum[0]].midtexture;
	texBot = sides[line->sidenum[0]].bottomtexture;
	
	sound = sfx_swtchn;
	if (line->special == 11)		/* EXIT SWITCH? */
		sound = sfx_swtchx;
	
	for (i = 0;i < numswitches*2;i++)
		if (switchlist[i] == texTop)
		{
			S_StartSound(buttonlist->soundorg,sound);
			sides[line->sidenum[0]].toptexture = switchlist[i^1];
			if (useAgain)
				P_StartButton(line,top,switchlist[i],BUTTONTIME);
			return;
		}
		else
		if (switchlist[i] == texMid)
		{
			S_StartSound(buttonlist->soundorg,sound);
			sides[line->sidenum[0]].midtexture = switchlist[i^1];
			if (useAgain)
				P_StartButton(line, middle,switchlist[i],BUTTONTIME);
			return;
		}
		else
		if (switchlist[i] == texBot)
		{
			S_StartSound(buttonlist->soundorg,sound);
			sides[line->sidenum[0]].bottomtexture = switchlist[i^1];
			if (useAgain)
				P_StartButton(line, bottom,switchlist[i],BUTTONTIME);
			return;
		}
}

/*
==============================================================================
=
= P_UseSpecialLine
=
= Called when a thing uses a special line
= Only the front sides of lines are usable
===============================================================================
*/

boolean P_UseSpecialLine (mobj_t *thing, line_t *line, int side)
{		
	/* */
	/*	Switches that other things can activate */
	/* */
	if (!thing->player)
	{
		if (line->flags & ML_SECRET)
			return false;		/* never open secret doors */
		switch(line->special)
		{
			case 1:		/* MANUAL DOOR RAISE */
/*			case 32:	// MANUAL BLUE */
/*			case 33:	// MANUAL RED */
/*			case 34:	// MANUAL YELLOW */
				break;
			default:
				return false;
		}
	}
	
	/* */
	/* do something */
	/*	 */
	switch (line->special)
	{
		/*=============================================== */
		/*	MANUALS */
		/*=============================================== */
		case 1:			/* Vertical Door */
		case 31:		/* Manual door open */
		case 26:		/* Blue Card Door Raise */
		case 32:		/* Blue Card door open */
		case 99:		/* Blue Skull Door Open */
		case 106:		/* Blue Skull Door Raise */
		case 27:		/* Yellow Card Door Raise */
		case 34:		/* Yellow Card door open */
		case 105:		/* Yellow Skull Door Open */
		case 108:		/* Yellow Skull Door Raise */
		case 28:		/* Red Card Door Raise */
		case 33:		/* Red Card door open */
		case 100:		/* Red Skull Door Open */
		case 107:		/* Red Skull Door Raise */
			EV_VerticalDoor (line, thing);
			break;
		/*=============================================== */
		/*	BUTTONS */
		/*=============================================== */
		case 42:		/* Close Door */
			if (EV_DoDoor(line,vld_close))
				P_ChangeSwitchTexture(line,1);
			break;
		case 43:		/* Lower Ceiling to Floor */
			if (EV_DoCeiling(line,lowerToFloor))
				P_ChangeSwitchTexture(line,1);
			break;
		case 45:		/* Lower Floor to Surrounding floor height */
			if (EV_DoFloor(line,lowerFloor))
				P_ChangeSwitchTexture(line,1);
			break;
		case 60:		/* Lower Floor to Lowest */
			if (EV_DoFloor(line,lowerFloorToLowest))
				P_ChangeSwitchTexture(line,1);
			break;
		case 61:		/* Open Door */
			if (EV_DoDoor(line,vld_open))
				P_ChangeSwitchTexture(line,1);
			break;
		case 62:		/* PlatDownWaitUpStay */
			if (EV_DoPlat(line,downWaitUpStay,1))
				P_ChangeSwitchTexture(line,1);
			break;
		case 63:		/* Raise Door */
			if (EV_DoDoor(line,vld_normal))
				P_ChangeSwitchTexture(line,1);
			break;
		case 64:		/* Raise Floor to ceiling */
			if (EV_DoFloor(line,raiseFloor))
				P_ChangeSwitchTexture(line,1);
			break;
		case 66:		/* Raise Floor 24 and change texture */
			if (EV_DoPlat(line,raiseAndChange,24))
				P_ChangeSwitchTexture(line,1);
			break;
		case 67:		/* Raise Floor 32 and change texture */
			if (EV_DoPlat(line,raiseAndChange,32))
				P_ChangeSwitchTexture(line,1);
			break;
		case 65:		/* Raise Floor Crush */
			if (EV_DoFloor(line,raiseFloorCrush))
				P_ChangeSwitchTexture(line,1);
			break;
		case 68:		/* Raise Plat to next highest floor and change texture */
			if (EV_DoPlat(line,raiseToNearestAndChange,0))
				P_ChangeSwitchTexture(line,1);
			break;
		case 69:		/* Raise Floor to next highest floor */
			if (EV_DoFloor(line, raiseFloorToNearest))
				P_ChangeSwitchTexture(line,1);
			break;
		case 70:		/* Turbo Lower Floor */
			if (EV_DoFloor(line,turboLower))
				P_ChangeSwitchTexture(line,1);
			break;
		/*=============================================== */
		/*	SWITCHES */
		/*=============================================== */
		case 7:			/* Build Stairs */
			if (EV_BuildStairs(line))
				P_ChangeSwitchTexture(line,0);
			break;
		case 9:			/* Change Donut */
			if (EV_DoDonut(line))
				P_ChangeSwitchTexture(line,0);
			break;
		case 11:		/* Exit level */
			G_ExitLevel ();
			P_ChangeSwitchTexture(line,0);
			break;
		case 14:		/* Raise Floor 32 and change texture */
			if (EV_DoPlat(line,raiseAndChange,32))
				P_ChangeSwitchTexture(line,0);
			break;
		case 15:		/* Raise Floor 24 and change texture */
			if (EV_DoPlat(line,raiseAndChange,24))
				P_ChangeSwitchTexture(line,0);
			break;
		case 18:		/* Raise Floor to next highest floor */
			if (EV_DoFloor(line, raiseFloorToNearest))
				P_ChangeSwitchTexture(line,0);
			break;
		case 20:		/* Raise Plat next highest floor and change texture */
			if (EV_DoPlat(line,raiseToNearestAndChange,0))
				P_ChangeSwitchTexture(line,0);
			break;
		case 21:		/* PlatDownWaitUpStay */
			if (EV_DoPlat(line,downWaitUpStay,0))
				P_ChangeSwitchTexture(line,0);
			break;
		case 23:		/* Lower Floor to Lowest */
			if (EV_DoFloor(line,lowerFloorToLowest))
				P_ChangeSwitchTexture(line,0);
			break;
		case 29:		/* Raise Door */
			if (EV_DoDoor(line,vld_normal))
				P_ChangeSwitchTexture(line,0);
			break;
		case 41:		/* Lower Ceiling to Floor */
			if (EV_DoCeiling(line,lowerToFloor))
				P_ChangeSwitchTexture(line,0);
			break;
		case 71:		/* Turbo Lower Floor */
			if (EV_DoFloor(line,turboLower))
				P_ChangeSwitchTexture(line,0);
			break;
		case 49:		/* Lower Ceiling And Crush */
			if (EV_DoCeiling(line,lowerAndCrush))
				P_ChangeSwitchTexture(line,0);
			break;
		case 50:		/* Close Door */
			if (EV_DoDoor(line,vld_close))
				P_ChangeSwitchTexture(line,0);
			break;
		case 51:		/* Secret EXIT */
			G_SecretExitLevel ();
			P_ChangeSwitchTexture(line,0);
			break;
		case 55:		/* Raise Floor Crush */
			if (EV_DoFloor(line,raiseFloorCrush))
				P_ChangeSwitchTexture(line,0);
			break;
		case 101:		/* Raise Floor */
			if (EV_DoFloor(line,raiseFloor))
				P_ChangeSwitchTexture(line,0);
			break;
		case 102:		/* Lower Floor to Surrounding floor height */
			if (EV_DoFloor(line,lowerFloor))
				P_ChangeSwitchTexture(line,0);
			break;
		case 103:		/* Open Door */
			if (EV_DoDoor(line,vld_open))
				P_ChangeSwitchTexture(line,0);
			break;
	}
	
	return true;
}

