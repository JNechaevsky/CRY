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


#include "doomstat.h"
#include "s_sound.h"
#include "p_local.h"


/*================================================================== */
/* */
/*						TELEPORTATION */
/* */
/*================================================================== */
int	EV_Teleport( line_t *line, int side, mobj_t *thing )
{
	int		i;
	int		tag;
	thinker_t	*thinker;
	mobj_t		*m,*fog;
	unsigned	an;
	sector_t	*sector;
	fixed_t		oldx, oldy, oldz;
	
	if (thing->flags & MF_MISSILE)
		return 0;			/* don't teleport missiles */
		
	if (side == 1)		/* don't teleport if hit back of line, */
		return 0;		/* so you can get out of teleporter */
	
	tag = line->tag;
	for (i = 0; i < numsectors; i++)
		if (sectors[ i ].tag == tag )
		{
			for (thinker = thinkercap.next ; thinker != &thinkercap ; thinker = thinker->next)
			{
				// not a mobj
				if (thinker->function.acp1 != (actionf_p1)P_MobjThinker)
					continue;	

				m = (mobj_t *)thinker;

				if (m->type != MT_TELEPORTMAN )
					continue;		/* not a teleportman */
				sector = m->subsector->sector;
				if (sector-sectors != i )
					continue;		/* wrong sector */

				oldx = thing->x;
				oldy = thing->y;
				oldz = thing->z;

				if (!P_TeleportMove (thing, m->x, m->y))
					return 0;	/* move is blocked */
				thing->z = thing->floorz;

				if (thing->player)
				{
					thing->player->viewz = thing->z+thing->player->viewheight;
					// [crispy] center view after teleporting
					// thing->player->centering = true;
					// [JN] Center view immediately.
					thing->player->lookdir = 0;
				}

/* spawn teleport fog at source and destination */
				fog = P_SpawnMobj (oldx, oldy, oldz, MT_TFOG);
				S_StartSound (fog, sfx_telept);
				an = m->angle >> ANGLETOFINESHIFT;
				fog = P_SpawnMobj (m->x+20*finecosine[an], m->y+20*finesine[an]
					, thing->z, MT_TFOG);
				S_StartSound (fog, sfx_telept);
				if (thing->player)
					thing->reactiontime = 18;	/* don't move for a bit */
				thing->angle = m->angle;
				thing->momx = thing->momy = thing->momz = 0;
				return 1;
			}	
		}
	return 0;
}

