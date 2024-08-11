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
// DESCRIPTION: Door animation code (opening/closing)
//


#include "z_zone.h"
#include "p_local.h"
#include "i_system.h"
#include "s_sound.h"
#include "d_englsh.h"
#include "ct_chat.h"

/*================================================================== */
/*================================================================== */
/* */
/*							VERTICAL DOORS */
/* */
/*================================================================== */
/*================================================================== */

/*================================================================== */
/* */
/*	T_VerticalDoor */
/* */
/*================================================================== */

void T_VerticalDoor (vldoor_t *door)
{
	result_e	res;
	
	switch(door->direction)
	{
		case 0:		/* WAITING */
			if (!--door->topcountdown)
				switch(door->type)
				{
					case vld_blazeRaise:
						door->direction = -1; /* time to go back down */
						S_StartSound(&door->sector->soundorg, sfx_bdcls);
						break;
					case vld_normal:
						door->direction = -1; /* time to go back down */
						S_StartSound(&door->sector->soundorg,sfx_dorcls);
						break;
					case vld_close30ThenOpen:
						door->direction = 1;
						S_StartSound(&door->sector->soundorg,sfx_doropn);
						break;
					default:
						break;
				}
			break;
		case 2:		/*  INITIAL WAIT */
			if (!--door->topcountdown)
				switch(door->type)
				{
					case vld_raiseIn5Mins:
						door->direction = 1;
						door->type = vld_normal;
						S_StartSound(&door->sector->soundorg,sfx_doropn);
						break;
				default:
					break;
				}
			break;
		case -1:	/* DOWN */
			res = T_MovePlane(door->sector,door->speed,
				door->sector->floorheight,false,1,door->direction);
			if (res == pastdest)
				switch(door->type)
				{
					case vld_blazeRaise:
					case vld_blazeClose:
						door->sector->specialdata = NULL;
						P_RemoveThinker (&door->thinker);  /* unlink and free */
						S_StartSound(&door->sector->soundorg, sfx_bdcls);
						break;
					case vld_normal:
					case vld_close:
						door->sector->specialdata = NULL;
						P_RemoveThinker (&door->thinker);  /* unlink and free */
						break;
					case vld_close30ThenOpen:
						door->direction = 0;
						door->topcountdown = TICRATE*30;
						break;
					default:
						break;
				}
			else if (res == crushed)
			{
				switch(door->type)
				{
					case vld_blazeClose:
					case vld_close:		/* DO NOT GO BACK UP! */
						break;

					// [crispy] fix "fast doors reopening with wrong sound"
					case vld_blazeRaise:
						door->direction = 1;
						S_StartSound(&door->sector->soundorg, sfx_bdopn);
						break;

					default:
						door->direction = 1;
						S_StartSound(&door->sector->soundorg,sfx_doropn);
						break;
				}
			}
			break;
		case 1:		/* UP */
			res = T_MovePlane(door->sector,door->speed,
				door->topheight,false,1,door->direction);
			if (res == pastdest)
				switch(door->type)
				{
					case vld_blazeRaise:
					case vld_normal:
						door->direction = 0; /* wait at top */
						door->topcountdown = door->topwait;
						break;
					case vld_close30ThenOpen:
					case vld_blazeOpen:
					case vld_open:
						door->sector->specialdata = NULL;
						P_RemoveThinker (&door->thinker);  /* unlink and free */
						break;
					default:
						break;
				}
			break;
	}
}

/*================================================================== */
/* */
/*		EV_DoLockedDoor */
/*		Move a locked door up/down */
/* */
/*================================================================== */

int EV_DoLockedDoor (line_t *line, vldoor_e type, mobj_t *thing)
{
	player_t *p = thing->player;
	
	if (!p)
	return 0;
	
		switch(line->special)
		{
			case 99:	/* Blue Lock */
			case 133:
				if (!p->cards[it_bluecard] && !p->cards[it_blueskull])
				{
					CT_SetMessage(p, PD_BLUEO, false, NULL);
					// [crispy] blinking key or skull in the status bar
					p->tryopen[it_bluecard] = KEYBLINKTICS;
					if (PTR_NoWayAudible(line))
					S_StartSound(NULL,sfx_oof);
					return 0;
				}
				break;

			case 134:	/* Red Lock */
			case 135:
				if (!p->cards[it_redcard] && !p->cards[it_redskull])
				{
					CT_SetMessage(p, PD_REDO, false, NULL);
					// [crispy] blinking key or skull in the status bar
					p->tryopen[it_redcard] = KEYBLINKTICS;
					if (PTR_NoWayAudible(line))
					S_StartSound(NULL,sfx_oof);
					return 0;
				}
				break;

			case 136:	/* Yellow Lock */
			case 137:
				if (!p->cards[it_yellowcard] && !p->cards[it_yellowskull])
				{
					CT_SetMessage(p, PD_YELLOWO, false, NULL);
					// [crispy] blinking key or skull in the status bar
					p->tryopen[it_yellowcard] = KEYBLINKTICS;
					if (PTR_NoWayAudible(line))
					S_StartSound(NULL,sfx_oof);
					return 0;
				}
				break;	
    }

	return EV_DoDoor(line,type);
}

/*================================================================== */
/* */
/*		EV_DoDoor */
/*		Move a door up/down and all around! */
/* */
/*================================================================== */

int EV_DoDoor (line_t *line, vldoor_e  type)
{
	int			secnum,rtn;
	sector_t		*sec;
	vldoor_t		*door;
	
	secnum = -1;
	rtn = 0;
    
	while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
	{
		sec = &sectors[secnum];
		if (sec->specialdata)
			continue;
		
		/* */
		/* new door thinker */
		/* */
	rtn = 1;
	door = Z_Malloc (sizeof(*door), PU_LEVSPEC, 0);
	P_AddThinker (&door->thinker);
	sec->specialdata = door;
	door->thinker.function.acp1 = (actionf_p1) T_VerticalDoor;
	door->sector = sec;
	door->type = type;
	door->topwait = VDOORWAIT;
	door->speed = VDOORSPEED;
		switch(type)
		{
			case vld_blazeClose:
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4*FRACUNIT;
				door->direction = -1;
				door->speed = VDOORSPEED * 4;
				S_StartSound(&door->sector->soundorg, sfx_bdcls);
				break;
			case vld_close:
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4*FRACUNIT;
				door->direction = -1;
				S_StartSound(&door->sector->soundorg,sfx_dorcls);
				break;
			case vld_close30ThenOpen:
				door->topheight = sec->ceilingheight;
				door->direction = -1;
				S_StartSound(&door->sector->soundorg,sfx_dorcls);
				break;
			case vld_blazeRaise:
			case vld_blazeOpen:
				door->direction = 1;
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4*FRACUNIT;
				door->speed = VDOORSPEED * 4;
				if (door->topheight != sec->ceilingheight)
				S_StartSound(&door->sector->soundorg, sfx_bdopn);
				break;
			case vld_normal:
			case vld_open:
				door->direction = 1;
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4*FRACUNIT;
				if (door->topheight != sec->ceilingheight)
					S_StartSound(&door->sector->soundorg,sfx_doropn);
				break;
			default:
				break;
		}
	}
	return rtn;
}

/*================================================================== */
/* */
/*	EV_VerticalDoor : open a door manually, no tag value */
/* */
/*================================================================== */

void EV_VerticalDoor (line_t *line, mobj_t *thing)
{
	player_t		*player;
	sector_t		*sec;
	vldoor_t		*door;
	int				side;
	
	side = 0;			/* only front sides can be used */
/* */
/*	Check for locks */
/* */
	player = thing->player;
		
	switch(line->special)
	{
		case 26:		/* Blue Card Lock */
		case 32:
			if ( player && !player->cards[it_bluecard] && !player->cards[it_blueskull])
			{
				CT_SetMessage(player, "You need a blue keycard to open this door", false, NULL);
				S_StartSound(thing,sfx_oof);
				// [crispy] blinking key or skull in the status bar
				player->tryopen[it_bluecard] = KEYBLINKTICS;
				return;
			}
			break;
		case 99:		/* Blue Skull Lock */
		case 106:
			if ( player && !player->cards[it_bluecard] && !player->cards[it_blueskull])
			{
				CT_SetMessage(player, "You need a blue skullkey to open this door", false, NULL);
				S_StartSound(thing,sfx_oof);
				// [crispy] blinking key or skull in the status bar
				player->tryopen[it_bluecard] = KEYBLINKTICS;
				return;
			}
			break;
		case 27:		/* Yellow Card Lock */
		case 34:
			if ( player && !player->cards[it_yellowcard] && !player->cards[it_yellowskull])
			{
				CT_SetMessage(player, "You need a yellow keycard to open this door", false, NULL);
				S_StartSound(thing,sfx_oof);
				// [crispy] blinking key or skull in the status bar
				player->tryopen[it_yellowcard] = KEYBLINKTICS;
				return;
			}
			break;
		case 105:		/* Yellow Skull Lock */
		case 108:
			if ( player && !player->cards[it_yellowcard] && !player->cards[it_yellowskull])
			{
				CT_SetMessage(player, "You need a yellow skullkey to open this door", false, NULL);
				S_StartSound(thing,sfx_oof);
				// [crispy] blinking key or skull in the status bar
				player->tryopen[it_yellowcard] = KEYBLINKTICS;
				return;
			}
			break;
		case 28:		/* Red Card Lock */
		case 33:
			if ( player && !player->cards[it_redcard] && !player->cards[it_redskull])
			{
				CT_SetMessage(player, "You need a red keycard to open this door", false, NULL);
				S_StartSound(thing,sfx_oof);
				// [crispy] blinking key or skull in the status bar
				player->tryopen[it_redcard] = KEYBLINKTICS;
				return;
			}
			break;
		case 100:		/* Red Skull Lock */
		case 107:
			if ( player && !player->cards[it_redcard] && !player->cards[it_redskull])
			{
				CT_SetMessage(player, "You need a red skullkey to open this door", false, NULL);
				S_StartSound(thing,sfx_oof);
				// [crispy] blinking key or skull in the status bar
				player->tryopen[it_redcard] = KEYBLINKTICS;
				return;
			}
			break;

    }
	
	/* if the sector has an active thinker, use it */
	sec = sides[ line->sidenum[side^1]] .sector;
	if (sec->specialdata)
	{
		door = sec->specialdata;
		switch(line->special)
		{
			case	1:		/* ONLY FOR "RAISE" DOORS, NOT "OPEN"s */
			case	26:		/* BLUE CARD */
			case	27:		/* YELLOW CARD */
			case	28:		/* RED CARD */
			case	106:	/* BLUE SKULL */
			case	108:	/* YELLOW SKULL */
			case	107:	/* RED SKULL */
				if (door->direction == -1)
					door->direction = 1;	/* go back up */
				else
				{
					if (!thing->player)
						return;				/* JDC: bad guys never close doors */
					door->direction = -1;	/* start going down immediately */
				}
				return;
		}
	}
	
	/* for proper sound */
	switch(line->special)
	{
		case 117:	/* BLAZING DOOR RAISE */
		case 118:	/* BLAZING DOOR OPEN */
			S_StartSound(&sec->soundorg,sfx_bdopn);
			break;

		case 1:		/* NORMAL DOOR SOUND */
		case 31:
			S_StartSound(&sec->soundorg,sfx_doropn);
			break;
		default:	/* LOCKED DOOR SOUND */
			S_StartSound(&sec->soundorg,sfx_doropn);
			break;
	}
	
	/* */
	/* new door thinker */
	/* */
	door = Z_Malloc (sizeof(*door), PU_LEVSPEC, 0);
	P_AddThinker (&door->thinker);
	sec->specialdata = door;
	door->thinker.function.acp1 = (actionf_p1) T_VerticalDoor;
	door->sector = sec;
	door->direction = 1;
	door->speed = VDOORSPEED;
	door->topwait = VDOORWAIT;
	switch(line->special)
	{
		case 1:
		case 26:
		case 27:
		case 28:
			door->type = vld_normal;
			break;
		case 31:
		case 32:
		case 33:
		case 34:
			door->type = vld_open;
			line->special = 0;
			break;

		case 117:	// blazing door raise
			door->type = vld_blazeRaise;
			door->speed = VDOORSPEED*4;
			break;
		case 118:	// blazing door open
			door->type = vld_blazeOpen;
			line->special = 0;
			door->speed = VDOORSPEED*4;
			break;
	}
	
	/* */
	/* find the top and bottom of the movement range */
	/* */
	door->topheight = P_FindLowestCeilingSurrounding(sec);
	door->topheight -= 4*FRACUNIT;
}

/*================================================================== */
/* */
/*	Spawn a door that closes after 30 seconds */
/* */
/*================================================================== */

void P_SpawnDoorCloseIn30 (sector_t *sec)
{
	vldoor_t	*door;
	
	door = Z_Malloc ( sizeof(*door), PU_LEVSPEC, 0);
	P_AddThinker (&door->thinker);
	sec->specialdata = door;
	sec->special = 0;
	door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
	door->sector = sec;
	door->direction = 0;
	door->type = vld_normal;
	door->speed = VDOORSPEED;
	door->topcountdown = 30 * TICRATE;
}

/*================================================================== */
/* */
/*	Spawn a door that opens after 5 minutes */
/* */
/*================================================================== */

void P_SpawnDoorRaiseIn5Mins (sector_t *sec, int secnum)
{
	vldoor_t	*door;
	
	door = Z_Malloc ( sizeof(*door), PU_LEVSPEC, 0);
	P_AddThinker (&door->thinker);
	sec->specialdata = door;
	sec->special = 0;
    door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
	door->sector = sec;
	door->direction = 2;
	door->type = vld_raiseIn5Mins;
	door->speed = VDOORSPEED;
	door->topheight = P_FindLowestCeilingSurrounding(sec);
	door->topheight -= 4*FRACUNIT;
	door->topwait = VDOORWAIT;
	door->topcountdown = 5 * 60 * TICRATE;
}
