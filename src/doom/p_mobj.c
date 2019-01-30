//
// Copyright(C) 1993-1996 Id Software, Inc.
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


#include <stdio.h>

#include "i_system.h"
#include "z_zone.h"
#include "m_random.h"
#include "doomdef.h"
#include "p_local.h"
#include "sounds.h"
#include "st_stuff.h"
#include "hu_stuff.h"
#include "s_sound.h"
#include "doomstat.h"
#include "jn.h"


void G_PlayerReborn (int player);
void P_SpawnMapThing (mapthing_t *mthing);


// -----------------------------------------------------------------------------
// P_SetMobjState
// Returns true if the mobj is still present.
// -----------------------------------------------------------------------------

int test;


// -----------------------------------------------------------------------------
// Use a heuristic approach to detect infinite state cycles: Count the number
// of times the loop in P_SetMobjState() executes and exit with an error once
// an arbitrary very large limit is reached.
// -----------------------------------------------------------------------------

#define MOBJ_CYCLE_LIMIT 1000000

boolean P_SetMobjState (mobj_t *mobj, statenum_t state)
{
    state_t *st;
    int      cycle_counter = 0;

    do
    {
        if (state == S_NULL)
        {
            mobj->state = (state_t *) S_NULL;
            P_RemoveMobj (mobj);
            return false;
        }

        st = &states[state];
        mobj->state = st;
        mobj->tics = st->tics;
        mobj->sprite = st->sprite;
        mobj->frame = st->frame;

        // Modified handling.
        // Call action functions when the state is set
        if (st->action.acp1)		
            st->action.acp1(mobj);	

        state = st->nextstate;

        if (cycle_counter++ > MOBJ_CYCLE_LIMIT)
        {
            I_Error("P_SetMobjState: Infinite state cycle detected!");
        }
    } while (!mobj->tics);

    return true;
}


// -----------------------------------------------------------------------------
// [crispy] return the latest "safe" state in a state sequence,
// so that no action pointer is ever called
// -----------------------------------------------------------------------------

static statenum_t P_LatestSafeState(statenum_t state)
{
    statenum_t safestate = S_NULL;
    static statenum_t laststate, lastsafestate;

    if (state == laststate)
    {
        return lastsafestate;
    }

    for (laststate = state; state != S_NULL; state = states[state].nextstate)
    {
        if (safestate == S_NULL)
        {
            safestate = state;
        }

        if (states[state].action.acp1)
        {
            safestate = S_NULL;
        }

        // [crispy] a state with -1 tics never changes
        if (states[state].tics == -1)
        {
            break;
        }
    }

    return lastsafestate = safestate;
}


// -----------------------------------------------------------------------------
// P_ExplodeMissile  
// -----------------------------------------------------------------------------

static void P_ExplodeMissileSafe (mobj_t *mo, boolean safe)
{
    mo->momx = mo->momy = mo->momz = 0;

    P_SetMobjState (mo, safe ? P_LatestSafeState(mobjinfo[mo->type].deathstate) : 
                                                 mobjinfo[mo->type].deathstate);

    mo->tics -= safe ? Crispy_Random()&3 : P_Random()&3;

    if (mo->tics < 1)
    mo->tics = 1;

    mo->flags &= ~MF_MISSILE;

    mo->flags |= MF_TRANSLUCENT;

    if (mo->info->deathsound)
    S_StartSound (mo, mo->info->deathsound);
}


// -----------------------------------------------------------------------------
// P_ExplodeMissile
// -----------------------------------------------------------------------------

void P_ExplodeMissile (mobj_t *mo)
{
    return P_ExplodeMissileSafe(mo, false);
}


// -----------------------------------------------------------------------------
// P_XYMovement  
// -----------------------------------------------------------------------------

#define STOPSPEED 0x1000
#define FRICTION  0xe800

void P_XYMovement (mobj_t *mo) 
{ 	
    player_t *player;
    fixed_t   xmove;
    fixed_t   ymove;
			
    if (!mo->momx && !mo->momy)
    {
        if (mo->flags & MF_SKULLFLY)
        {
            // the skull slammed into something
            mo->flags &= ~MF_SKULLFLY;
            mo->momx = mo->momy = mo->momz = 0;

            // [Julia] Fix: https://doomwiki.org/wiki/Lost_soul_target_amnesia
            P_SetMobjState (mo, mo->info->seestate);

        }

        return;
    }

    player = mo->player;
		
    if (mo->momx > MAXMOVE)
        mo->momx = MAXMOVE;
    else 
    if (mo->momx < -MAXMOVE)
        mo->momx = -MAXMOVE;

    if (mo->momy > MAXMOVE)
        mo->momy = MAXMOVE;
    else 
    if (mo->momy < -MAXMOVE)
        mo->momy = -MAXMOVE;

    xmove = mo->momx;
    ymove = mo->momy;

    do
    {
        fixed_t ptryx, ptryy;

        // killough 8/9/98: fix bug in original Doom source:
        // Large negative displacements were never considered.
        // This explains the tendency for Mancubus fireballs
        // to pass through walls.

        // [Julia] Fix: https://doomwiki.org/wiki/Mancubus_fireball_clipping
        // Thanks to Jeff Doggett for simplifying!

        if ((xmove >  MAXMOVE/2 || ymove >  MAXMOVE/2) 
        ||  (xmove < -MAXMOVE/2 || ymove < -MAXMOVE/2))
        {
            ptryx = mo->x + xmove/2;
            ptryy = mo->y + ymove/2;
            xmove >>= 1;
            ymove >>= 1;
        }
        else
        {
            ptryx = mo->x + xmove;
            ptryy = mo->y + ymove;
            xmove = ymove = 0;
        }

        if (!P_TryMove (mo, ptryx, ptryy))
        {
            // blocked move
            if (mo->player)
            {	// try to slide along it
                P_SlideMove (mo);
            }
            else if (mo->flags & MF_MISSILE)
            {
                boolean safe = false;
                // explode a missile
                if (ceilingline &&
                    ceilingline->backsector &&
                    ceilingline->backsector->ceilingpic == skyflatnum)
                {
                    if (mo->z > ceilingline->backsector->ceilingheight)
                    {
                        // Hack to prevent missiles exploding
                        // against the sky.
                        // Does not handle sky floors.
                        P_RemoveMobj (mo);
                        return;
                    }
                    else
                    {
                        safe = true;
                    }
                }

                P_ExplodeMissileSafe (mo, safe);
            }
            else
            {
                mo->momx = mo->momy = 0;
            }
        }
    } while (xmove || ymove);

    if (mo->flags & (MF_MISSILE | MF_SKULLFLY) )
    return;     // no friction for missiles ever

    if (mo->z > mo->floorz)
    return;     // no friction when airborne

    if (mo->flags & MF_CORPSE)
    {
        // do not stop sliding
        //  if halfway off a step with some momentum
        if (mo->momx >  FRACUNIT/4
        ||  mo->momx < -FRACUNIT/4
        ||  mo->momy >  FRACUNIT/4
        ||  mo->momy < -FRACUNIT/4)
        {
            if (mo->floorz != mo->subsector->sector->floorheight)
            return;
        }
    }

    // killough 8/11/98: add bouncers
    // killough 9/15/98: add objects falling off ledges
    // killough 11/98: only include bouncers hanging off ledges
    if ((mo->flags & MF_CORPSE || mo->intflags & MIF_FALLING) 
    &&  (mo->momx > FRACUNIT/4 || mo->momx < -FRACUNIT/4
    ||   mo->momy > FRACUNIT/4 || mo->momy < -FRACUNIT/4) 
    &&   mo->floorz != mo->subsector->sector->floorheight)
    {
        return;  // do not stop sliding if halfway off a step with some momentum
    }

    if (mo->momx > -STOPSPEED
    &&  mo->momx <  STOPSPEED
    &&  mo->momy > -STOPSPEED
    &&  mo->momy <  STOPSPEED
    && (!player || (player->cmd.forwardmove== 0 && player->cmd.sidemove == 0)))
    {
        // if in a walking frame, stop moving
        if ( player&&(unsigned)((player->mo->state - states)- S_PLAY_RUN1) < 4)
        P_SetMobjState (player->mo, S_PLAY);

        mo->momx = 0;
        mo->momy = 0;
    }
    else
    {
        mo->momx = FixedMul (mo->momx, FRICTION);
        mo->momy = FixedMul (mo->momy, FRICTION);
    }
}


// -----------------------------------------------------------------------------
// P_ZMovement
// -----------------------------------------------------------------------------

void P_ZMovement (mobj_t *mo)
{
    fixed_t dist;
    fixed_t delta;
    
    // check for smooth step up
    if (mo->player && mo->z < mo->floorz)
    {
        mo->player->viewheight -= mo->floorz-mo->z;
        mo->player->deltaviewheight = (VIEWHEIGHT - mo->player->viewheight)>>3;
    }

    // adjust height
    mo->z += mo->momz;

    if ( mo->flags & MF_FLOAT && mo->target)
    {
        // float down towards target if too close
        if ( !(mo->flags & MF_SKULLFLY) && !(mo->flags & MF_INFLOAT) )
        {
            dist = P_AproxDistance (mo->x - mo->target->x,
                                    mo->y - mo->target->y);

            delta =(mo->target->z + (mo->height>>1)) - mo->z;

            if (delta<0 && dist < -(delta*3) )
            mo->z -= FLOATSPEED;
            else if (delta>0 && dist < (delta*3) )
            mo->z += FLOATSPEED;			
        }
    }

    // clip movement
    if (mo->z <= mo->floorz)
    {
        // hit the floor
    if (mo->momz < 0)
    {
        if (mo->player && mo->momz < -GRAVITY*8)	
        {
            // Squat down.
            // Decrease viewheight for a moment
            // after hitting the ground (hard),
            // and utter appropriate sound.
            mo->player->deltaviewheight = mo->momz>>3;

            // [crispy] squat down weapon sprite as well
            // [Julia] Suqat BFG9000 heavier and slower
            if (mo->player->readyweapon == wp_bfg)
            mo->player->psp_dy_max = mo->momz>>1;
            else
            mo->player->psp_dy_max = mo->momz>>2;
        
            // [Julia] Only alive player making "oof" sound
            if (mo->health > 0)
            {
                // [Julia] Don't break "oofs" by firing sounds
                S_StartSound (NULL, sfx_oof);
            }
	    }
        mo->momz = 0;
    }
    mo->z = mo->floorz;

        if (mo->flags & MF_SKULLFLY)
            mo->momz = -mo->momz;

	if ( (mo->flags & MF_MISSILE)
	     && !(mo->flags & MF_NOCLIP) )
	{
	    P_ExplodeMissile (mo);
	    return;
	}
    }
    else if (! (mo->flags & MF_NOGRAVITY) )
    {
	if (mo->momz == 0)
	    mo->momz = -GRAVITY*2;
	else
	    mo->momz -= GRAVITY;
    }
	
    if (mo->z + mo->height > mo->ceilingz)
    {
	// hit the ceiling
	if (mo->momz > 0)
	    mo->momz = 0;
	{
	    mo->z = mo->ceilingz - mo->height;
	}

	if (mo->flags & MF_SKULLFLY)
	{	// the skull slammed into something
	    mo->momz = -mo->momz;
	}

	if ((mo->flags & MF_MISSILE) && !(mo->flags & MF_NOCLIP))
	{
        // [Julia] Fix projectiles explode on impact with "sky" ceilings
        if (mo->subsector->sector->ceilingpic == skyflatnum)
        {
            P_RemoveMobj(mo);
        }
        else
        {
            P_ExplodeMissile (mo);
        }
        return;
    }
    }
} 


// -----------------------------------------------------------------------------
// P_MobjThinker
// -----------------------------------------------------------------------------

void P_MobjThinker (mobj_t *mobj)
{
    // [AM] Handle interpolation unless we're an active player.
    if (!(mobj->player != NULL && mobj == mobj->player->mo))
    {
        // Assume we can interpolate at the beginning
        // of the tic.
        mobj->interp = true;

        // Store starting position for mobj interpolation.
        mobj->oldx = mobj->x;
        mobj->oldy = mobj->y;
        mobj->oldz = mobj->z;
        mobj->oldangle = mobj->angle;
    }

    // momentum movement
    if (mobj->momx ||  mobj->momy || (mobj->flags&MF_SKULLFLY))
    {
        P_XYMovement (mobj);

        if (mobj->thinker.function.acv == (actionf_v) (-1))
        return;     // mobj was removed
    }

    if ((mobj->z != mobj->floorz) || mobj->momz)
    {
        P_ZMovement (mobj);

        if (mobj->thinker.function.acv == (actionf_v) (-1))
        return;     // mobj was removed
    }

	// killough 9/12/98: objects fall off ledges if they are hanging off
	// slightly push off of ledge if hanging more than halfway off
    if (mobj->z > mobj->dropoffz        // Only objects contacting dropoff
    && !(mobj->flags & MF_NOGRAVITY)    // Only objects which fall
    && (mobj->flags & MF_CORPSE))       // [Julia] And only for corpses
    {
        P_ApplyTorque(mobj);            // Apply torque
    }
    else
    {
        mobj->intflags &= ~MIF_FALLING, mobj->gear = 0;  // Reset torque
    }

    // cycle through states,
    // calling action functions at transitions
    if (mobj->tics != -1)
    {
        mobj->tics--;

        // you can cycle through multiple states in a tic
        if (!mobj->tics && !P_SetMobjState (mobj, mobj->state->nextstate) )
        return;     // freed itself
    }
}


// -----------------------------------------------------------------------------
// P_SpawnMobj
// -----------------------------------------------------------------------------
static mobj_t*
P_SpawnMobjSafe
( fixed_t     x,
  fixed_t     y,
  fixed_t     z,
  mobjtype_t  type,
  boolean     safe )
{
    mobj_t     *mobj;
    state_t    *st;
    mobjinfo_t *info;

    mobj = Z_Malloc (sizeof(*mobj), PU_LEVEL, NULL);
    memset (mobj, 0, sizeof (*mobj));
    info = &mobjinfo[type];

    mobj->type = type;
    mobj->info = info;
    mobj->x = x;
    mobj->y = y;
    mobj->radius = info->radius;
    mobj->height = info->height;
    mobj->flags = info->flags;
    mobj->health = info->spawnhealth;

    if (gameskill != sk_nightmare && gameskill != sk_ultranm)
    mobj->reactiontime = info->reactiontime;

    mobj->lastlook = safe ? Crispy_Random () % MAXPLAYERS : P_Random () % MAXPLAYERS;
    // do not set the state with P_SetMobjState,
    // because action routines can not be called yet
    st = &states[safe ? P_LatestSafeState(info->spawnstate) : info->spawnstate];

    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;

    // set subsector and/or block links
    P_SetThingPosition (mobj);

    mobj->floorz = mobj->subsector->sector->floorheight;
    mobj->ceilingz = mobj->subsector->sector->ceilingheight;

    if (z == ONFLOORZ)
    mobj->z = mobj->floorz;
    else if (z == ONCEILINGZ)
    mobj->z = mobj->ceilingz - mobj->info->height;
    else 
    mobj->z = z;

    // [AM] Do not interpolate on spawn.
    mobj->interp = false;

    // [AM] Just in case interpolation is attempted...
    mobj->oldx = mobj->x;
    mobj->oldy = mobj->y;
    mobj->oldz = mobj->z;
    mobj->oldangle = mobj->angle;

    mobj->thinker.function.acp1 = (actionf_p1)P_MobjThinker;

    P_AddThinker (&mobj->thinker);

    return mobj;
}


// -----------------------------------------------------------------------------
// P_SpawnMobj
// -----------------------------------------------------------------------------

mobj_t *P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type)
{
    return P_SpawnMobjSafe(x, y, z, type, false);
} 


// -----------------------------------------------------------------------------
// P_RemoveMobj
// -----------------------------------------------------------------------------

void P_RemoveMobj (mobj_t *mobj)
{
    P_UnsetThingPosition (mobj);        // unlink from sector and block lists
    S_StopSound (mobj);                 // stop any playing sound
    P_RemoveThinker ((thinker_t*)mobj); // free block
}


// -----------------------------------------------------------------------------
// P_SpawnPlayer
// Called when a player is spawned on the level.
// Most of the player structure stays unchanged
//  between levels.
// -----------------------------------------------------------------------------

void P_SpawnPlayer (mapthing_t *mthing)
{
    fixed_t   x;
    fixed_t   y;
    fixed_t   z;
    player_t *p;
    mobj_t   *mobj;

    if (mthing->type == 0)
    return;

    // not playing?
    if (!playeringame[mthing->type-1])
    return;					

    p = &players[mthing->type-1];

    if (p->playerstate == PST_REBORN)
    G_PlayerReborn (mthing->type-1);

    x       = mthing->x << FRACBITS;
    y       = mthing->y << FRACBITS;
    z       = ONFLOORZ;
    mobj    = P_SpawnMobj (x,y,z, MT_PLAYER);

    // set color translations for player sprites
    if (mthing->type > 1)		
    mobj->flags |= (mthing->type-1)<<MF_TRANSSHIFT;

    mobj->angle	= ANG45 * (mthing->angle/45);
    mobj->player = p;
    mobj->health = p->health;

    p->mo = mobj;
    p->playerstate = PST_LIVE;	
    p->refire = 0;
    p->message = NULL;
    p->damagecount = 0;
    p->bonuscount = 0;
    p->extralight = 0;
    p->fixedcolormap = 0;
    p->viewheight = VIEWHEIGHT;
    p->lookdir = 0;

    // setup gun psprite
    P_SetupPsprites (p);

    if (mthing->type-1 == consoleplayer)
    {
        // wake up the status bar
        ST_Start ();
        // wake up the heads up text
        HU_Start ();		
    }
}


// -----------------------------------------------------------------------------
// P_SpawnMapThing
// The fields of the mapthing should
// already be in host byte order.
// -----------------------------------------------------------------------------

void P_SpawnMapThing (mapthing_t *mthing)
{
    int      i;
    int      bit;
    fixed_t  x;
    fixed_t  y;
    fixed_t  z;
    mobj_t  *mobj;

    if (mthing->type <= 0)
    {
        // Thing type 0 is actually "player -1 start".  
        // For some reason, Vanilla Doom accepts/ignores this.
        return;
    }

    // check for players specially
    if (mthing->type <= 4)
    {
        // save spots for respawning in network games
        playerstarts[mthing->type-1] = *mthing;
        playerstartsingame[mthing->type-1] = true;
        P_SpawnPlayer (mthing);
        return;
    }

    // check for apropriate skill level
    if (!netgame && (mthing->options & 16) )
    return;

    if (gameskill == sk_baby)
    bit = 1;
    else if (gameskill == sk_nightmare || gameskill == sk_ultranm)
    bit = 4;
    else
    bit = 1<<(gameskill-1);

    if (!(mthing->options & bit) )
    return;

    // find which type to spawn
    for (i=0 ; i< NUMMOBJTYPES ; i++)
    if (mthing->type == mobjinfo[i].doomednum)
    break;

    if (i==NUMMOBJTYPES)
    {
        // [crispy] ignore unknown map things
        printf ("P_SpawnMapThing: Unknown type %i at (%i, %i)\n",
                mthing->type,
                mthing->x, mthing->y);
        return;
    }

    // spawn it
    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;

    if (mobjinfo[i].flags & MF_SPAWNCEILING)
    z = ONCEILINGZ;
    else
    z = ONFLOORZ;

    mobj = P_SpawnMobj (x,y,z, i);
    mobj->spawnpoint = *mthing;

    if (mobj->tics > 0)
    mobj->tics = 1 + (P_Random () % mobj->tics);
    if (mobj->flags & MF_COUNTKILL)
    totalkills++;
    if (mobj->flags & MF_COUNTITEM)
    totalitems++;

    mobj->angle = ANG45 * (mthing->angle/45);
    if (mthing->options & MTF_AMBUSH)
    mobj->flags |= MF_AMBUSH;

    // [crispy] randomly flip space marine corpse objects
    if (mobj->info->spawnstate == S_PLAY_DIE7 ||
        mobj->info->spawnstate == S_PLAY_XDIE9)
    {
        mobj->health -= Crispy_Random() & 1;
        // [crispy] randomly colorize space marine corpse objects
        mobj->flags |= (Crispy_Random() & 3) << MF_TRANSSHIFT;
    }

    // [crispy] blinking key or skull in the status bar
    if (mobj->sprite == SPR_BSKU)
    st_keyorskull[it_bluecard] = 3;
    else
    if (mobj->sprite == SPR_RSKU)
    st_keyorskull[it_redcard] = 3;
    else
    if (mobj->sprite == SPR_YSKU)
    st_keyorskull[it_yellowcard] = 3;
}


// =============================================================================
// GAME SPAWN FUNCTIONS
// =============================================================================


// -----------------------------------------------------------------------------
// P_SpawnPuff
// -----------------------------------------------------------------------------

extern fixed_t attackrange;

void P_SpawnPuff (fixed_t x, fixed_t y, fixed_t z)
{
    return P_SpawnPuffSafe(x, y, z, false);
}


// -----------------------------------------------------------------------------
// P_SpawnPuffSafe
// -----------------------------------------------------------------------------

void P_SpawnPuffSafe (fixed_t x, fixed_t y, fixed_t z, boolean safe)
{
    mobj_t *th;

    z += safe ? (Crispy_SubRandom() << 10) : (P_SubRandom() << 10);

    th = P_SpawnMobjSafe (x,y,z, MT_PUFF, safe);
    th->momz = FRACUNIT;
    th->tics -= safe ? Crispy_Random()&3 : P_Random()&3;

    if (th->tics < 1)
    th->tics = 1;

    // don't make punches spark on the wall
    if (attackrange == MELEERANGE)
    P_SetMobjState (th, safe ? P_LatestSafeState(S_PUFF3) : S_PUFF3);
}


// -----------------------------------------------------------------------------
// P_SpawnBlood
// -----------------------------------------------------------------------------

void P_SpawnBlood (fixed_t x, fixed_t y, fixed_t z, int damage, mobj_t *target)
{
    mobj_t *th;

    z += (P_SubRandom() << 10);

    th = P_SpawnMobj (x,y,z, MT_BLOOD);

    th->momz = FRACUNIT*2;
    th->tics -= P_Random()&3;

    if (th->tics < 1)
    th->tics = 1;

    if (damage <= 12 && damage >= 9)
    {
        P_SetMobjState (th,S_BLOOD2);
    }
    else if (damage < 9)
    {
        P_SetMobjState (th,S_BLOOD3);
    }

    // [crispy] connect blood object with the monster that bleeds it
    th->target = target;
}


// -----------------------------------------------------------------------------
// P_CheckMissileSpawn
// Moves the missile forward a bit
//  and possibly explodes it right there.
// -----------------------------------------------------------------------------

void P_CheckMissileSpawn (mobj_t *th)
{
    th->tics -= P_Random()&3;
    if (th->tics < 1)
    th->tics = 1;

    // move a little forward so an angle can
    // be computed if it immediately explodes
    th->x += (th->momx>>1);
    th->y += (th->momy>>1);
    th->z += (th->momz>>1);

    if (!P_TryMove (th, th->x, th->y))
    P_ExplodeMissile (th);
}


// -----------------------------------------------------------------------------
// Certain functions assume that a mobj_t pointer is non-NULL,
// causing a crash in some situations where it is NULL.  Vanilla
// Doom did not crash because of the lack of proper memory 
// protection. This function substitutes NULL pointers for
// pointers to a dummy mobj, to avoid a crash.
// -----------------------------------------------------------------------------

mobj_t *P_SubstNullMobj(mobj_t *mobj)
{
    if (mobj == NULL)
    {
        static mobj_t dummy_mobj;

        dummy_mobj.x = 0;
        dummy_mobj.y = 0;
        dummy_mobj.z = 0;
        dummy_mobj.flags = 0;

        mobj = &dummy_mobj;
    }

    return mobj;
}


// -----------------------------------------------------------------------------
// P_SpawnMissile
// -----------------------------------------------------------------------------

mobj_t *P_SpawnMissile (mobj_t *source, mobj_t *dest, mobjtype_t type)
{
    int      dist;
    angle_t  an;
    mobj_t  *th;

    th = P_SpawnMobj (source->x,
                      source->y,
                      source->z + 4*8*FRACUNIT, type);

    if (th->info->seesound)
    S_StartSound (th, th->info->seesound);

    th->target = source;	// where it came from
    an = R_PointToAngle2 (source->x, source->y, dest->x, dest->y);

    th->angle = an;
    an >>= ANGLETOFINESHIFT;
    th->momx = FixedMul (th->info->speed, finecosine[an]);
    th->momy = FixedMul (th->info->speed, finesine[an]);

    dist = P_AproxDistance (dest->x - source->x, dest->y - source->y);
    dist = dist / th->info->speed;

    if (dist < 1)
    dist = 1;

    th->momz = (dest->z - source->z) / dist;
    P_CheckMissileSpawn (th);

    return th;
}


// -----------------------------------------------------------------------------
// P_SpawnPlayerMissile
// Tries to aim at a nearby monster
// -----------------------------------------------------------------------------

void P_SpawnPlayerMissile (mobj_t *source, mobjtype_t type)
{
    mobj_t  *th;
    angle_t  an;
    fixed_t  x;
    fixed_t  y;
    fixed_t  z;
    fixed_t  slope;

    // see which target is to be aimed at
    an = source->angle;
    slope = P_AimLineAttack (source, an, 16*64*FRACUNIT);

    if (!linetarget)
    {
        an += 1<<26;
        slope = P_AimLineAttack (source, an, 16*64*FRACUNIT);

        if (!linetarget)
        {
            an -= 2<<26;
            slope = P_AimLineAttack (source, an, 16*64*FRACUNIT);
        }

        if (!linetarget)
        {
            an = source->angle;
            slope = 0;
        }

        // [Julia] Mouselook: also count vertical angles
        if (!linetarget && mlook)
        {
            an = source->angle;
            slope = ((source->player->lookdir / MLOOKUNIT) << FRACBITS) / 
                    (screenblocks <= 10 ? 160 : 146);
        }
    }

    x = source->x;
    y = source->y;
    z = source->z + 4*8*FRACUNIT;

    th = P_SpawnMobj (x,y,z, type);

    if (th->info->seesound)
    S_StartSound (th, th->info->seesound);

    th->target = source;
    th->angle = an;
    th->momx = FixedMul( th->info->speed, finecosine[an>>ANGLETOFINESHIFT]);
    th->momy = FixedMul( th->info->speed, finesine[an>>ANGLETOFINESHIFT]);
    th->momz = FixedMul( th->info->speed, slope);

    P_CheckMissileSpawn (th);
}

