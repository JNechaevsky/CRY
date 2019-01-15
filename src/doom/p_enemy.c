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
#include <stdlib.h>

#include "m_random.h"
#include "i_system.h"
#include "doomdef.h"
#include "p_local.h"
#include "s_sound.h"
#include "g_game.h"
#include "doomstat.h"
#include "r_state.h"
#include "sounds.h"
#include "jn.h"


typedef enum
{
    DI_EAST,
    DI_NORTHEAST,
    DI_NORTH,
    DI_NORTHWEST,
    DI_WEST,
    DI_SOUTHWEST,
    DI_SOUTH,
    DI_SOUTHEAST,
    DI_NODIR,
    NUMDIRS

} dirtype_t;


// -----------------------------------------------------------------------------
// P_NewChaseDir related LUT.
// -----------------------------------------------------------------------------
dirtype_t opposite[] =
{
    DI_WEST, DI_SOUTHWEST, DI_SOUTH, DI_SOUTHEAST,
    DI_EAST, DI_NORTHEAST, DI_NORTH, DI_NORTHWEST, DI_NODIR
};

dirtype_t diags[] =
{
    DI_NORTHWEST, DI_NORTHEAST, DI_SOUTHWEST, DI_SOUTHEAST
};


void A_Fall (mobj_t *actor);


// =============================================================================
// ENEMY THINKING
// Enemies are allways spawned
// with targetplayer = -1, threshold = 0
// Most monsters are spawned unaware of all players,
// but some can be made preaware
// =============================================================================


// -----------------------------------------------------------------------------
// Called by P_NoiseAlert.
// Recursively traverse adjacent sectors,
// sound blocking lines cut off traversal.
// -----------------------------------------------------------------------------

mobj_t *soundtarget;

void P_RecursiveSound (sector_t *sec, int soundblocks)
{
    int       i;
    line_t   *check;
    sector_t *other;

    // wake up all monsters in this sector
    if (sec->validcount == validcount
    && sec->soundtraversed <= soundblocks+1)
    {
        return; // already flooded
    }

    sec->validcount = validcount;
    sec->soundtraversed = soundblocks+1;
    sec->soundtarget = soundtarget;

    for (i=0 ; i<sec->linecount ; i++)
    {
        check = sec->lines[i];

        if (! (check->flags & ML_TWOSIDED) )
        continue;

        P_LineOpening (check);

        if (openrange <= 0)
        continue;   // closed door

        if ( sides[ check->sidenum[0] ].sector == sec)
        other = sides[ check->sidenum[1] ] .sector;
        else
        other = sides[ check->sidenum[0] ].sector;

        if (check->flags & ML_SOUNDBLOCK)
        {
            if (!soundblocks)
            P_RecursiveSound (other, 1);
        }
        else
            P_RecursiveSound (other, soundblocks);
        }
}


// -----------------------------------------------------------------------------
// P_NoiseAlert
//
// If a monster yells at a player,
// it will alert other monsters to the player.
// -----------------------------------------------------------------------------
void P_NoiseAlert (mobj_t *target, mobj_t *emmiter)
{
    soundtarget = target;
    validcount++;
    P_RecursiveSound (emmiter->subsector->sector, 0);
}


// -----------------------------------------------------------------------------
// P_CheckMeleeRange
// -----------------------------------------------------------------------------

boolean P_CheckMeleeRange (mobj_t *actor)
{
    mobj_t  *pl;
    fixed_t  dist;

    if (!actor->target)
    return false;

    pl = actor->target;
    dist = P_AproxDistance (pl->x-actor->x, pl->y-actor->y);

    if (dist >= MELEERANGE-20*FRACUNIT+pl->info->radius)
    return false;

    if (! P_CheckSight (actor, actor->target) )
    return false;

    return true;		
}


// -----------------------------------------------------------------------------
// P_CheckMissileRange
// -----------------------------------------------------------------------------

boolean P_CheckMissileRange (mobj_t *actor)
{
    fixed_t dist;
	
    if (! P_CheckSight (actor, actor->target) )
    return false;

    if ( actor->flags & MF_JUSTHIT )
    {
        // the target just hit the enemy, so fight back!
        actor->flags &= ~MF_JUSTHIT;
        return true;
    }

    if (actor->reactiontime)
    return false;   // do not attack yet
		
    // OPTIMIZE: get this from a global checksight
    dist = P_AproxDistance (actor->x-actor->target->x,
                            actor->y-actor->target->y) - 64*FRACUNIT;

    if (!actor->info->meleestate)
    dist -= 128*FRACUNIT;   // no melee attack, so fire more

    dist >>= FRACBITS;

    if (actor->type == MT_SKULL)
    dist >>= 1;
    
    if (dist > 200)
	dist = 200;

    if (P_Random () < dist)
    return false;

    return true;
}


// -----------------------------------------------------------------------------
// P_Move
// Move in the current direction,
// returns false if the move is blocked.
// -----------------------------------------------------------------------------

fixed_t	xspeed[8] = {FRACUNIT,47000,0,-47000,-FRACUNIT,-47000,0,47000};
fixed_t yspeed[8] = {0,47000,FRACUNIT,47000,0,-47000,-FRACUNIT,-47000};

boolean P_Move (mobj_t *actor)
{
    fixed_t	 tryx;
    fixed_t	 tryy;
    line_t  *ld;
    line_t  *blockline;

    // warning: 'catch', 'throw', and 'try'
    // are all C++ reserved words
    boolean	try_ok;
    boolean	good;

    if (actor->movedir == DI_NODIR)
    return false;

    if ((unsigned)actor->movedir >= 8)
	I_Error ("Weird actor->movedir!");

    tryx = actor->x + actor->info->speed*xspeed[actor->movedir];
    tryy = actor->y + actor->info->speed*yspeed[actor->movedir];

    try_ok = P_TryMove (actor, tryx, tryy);

    if (!try_ok)
    {
        // open any specials
        if (actor->flags & MF_FLOAT && floatok)
        {
            // must adjust height
            if (actor->z < tmfloorz)
            actor->z += FLOATSPEED;
            else
            actor->z -= FLOATSPEED;

            actor->flags |= MF_INFLOAT;
            return true;
        }

        if (!numspechit)
        return false;

        actor->movedir = DI_NODIR;
        good = false;

    // [Julia] Fix: // https://doomwiki.org/wiki/Monsters_stuck_in_doortracks,_walls_or_hanging_off_lifts
    //
    // if the special is not a door that can be opened, return false
    //
    // killough 8/9/98: this is what caused monsters to get stuck in
    // doortracks, because it thought that the monster freed itself
    // by opening a door, even if it was moving towards the doortrack,
    // and not the door itself.
    //
    // killough 9/9/98: If a line blocking the monster is activated,
    // return true 90% of the time. If a line blocking the monster is
    // not activated, but some other line is, return false 90% of the
    // time. A bit of randomness is needed to ensure it's free from
    // lockups, but for most cases, it returns the correct result.
    //
    // Do NOT simply return false 1/4th of the time (causes monsters to
    // back out when they shouldn't, and creates secondary stickiness).
    blockline = spechit[numspechit];

    for (good = false; numspechit--;)
        if (P_UseSpecialLine(actor, spechit[numspechit], 0))
            good |= spechit[numspechit] == blockline ? 1 : 2;

    return (good && ((M_Random() >= 230) ^ (good & 1)));

    while (numspechit--)
    {
        ld = spechit[numspechit];
        // if the special is not a door that can be opened, return false
        if (P_UseSpecialLine (actor, ld,0))
        good = true;
    }
    return good;
    }
    else
    {
        actor->flags &= ~MF_INFLOAT;
    }
	
    if (! (actor->flags & MF_FLOAT) )	
    actor->z = actor->floorz;

    return true; 
}


// -----------------------------------------------------------------------------
// TryWalk
// Attempts to move actor on
// in its current (ob->moveangle) direction.
// If blocked by either a wall or an actor
// returns FALSE
// If move is either clear or blocked only by a door,
// returns TRUE and sets...
// If a door is in the way,
// an OpenDoor call is made to start it opening.
// -----------------------------------------------------------------------------

boolean P_TryWalk (mobj_t *actor)
{	
    if (!P_Move (actor))
    return false;

    actor->movecount = P_Random()&15;
    return true;
}


// -----------------------------------------------------------------------------
// P_NewChaseDir
// -----------------------------------------------------------------------------

void P_NewChaseDir (mobj_t *actor)
{
    int        tdir;
    fixed_t    deltax;
    fixed_t    deltay;
    dirtype_t  d[3];
    dirtype_t  olddir;
    dirtype_t  turnaround;

    if (!actor->target)
	I_Error ("P_NewChaseDir: called with no target");

    olddir = actor->movedir;
    turnaround=opposite[olddir];

    deltax = actor->target->x - actor->x;
    deltay = actor->target->y - actor->y;

    if (deltax>10*FRACUNIT)
    d[1]= DI_EAST;
    else if (deltax<-10*FRACUNIT)
    d[1]= DI_WEST;
    else
    d[1]=DI_NODIR;

    if (deltay<-10*FRACUNIT)
    d[2]= DI_SOUTH;
    else if (deltay>10*FRACUNIT)
    d[2]= DI_NORTH;
    else
    d[2]=DI_NODIR;

    // try direct route
    if (d[1] != DI_NODIR && d[2] != DI_NODIR)
    {
        actor->movedir = diags[((deltay<0)<<1)+(deltax>0)];

        if (actor->movedir != (int) turnaround && P_TryWalk(actor))
        return;
    }

    // try other directions
    if (P_Random() > 200 || abs(deltay) > abs(deltax))
    {
        tdir=d[1];
        d[1]=d[2];
        d[2]=tdir;
    }

    if (d[1]==turnaround)
    d[1]=DI_NODIR;
    if (d[2]==turnaround)
    d[2]=DI_NODIR;

    if (d[1]!=DI_NODIR)
    {
        actor->movedir = d[1];

        if (P_TryWalk(actor))
        {
            // either moved forward or attacked
            return;
        }
    }

    if (d[2]!=DI_NODIR)
    {
        actor->movedir =d[2];

        if (P_TryWalk(actor))
        return;
    }

    // there is no direct path to the player, so pick another direction.
    if (olddir!=DI_NODIR)
    {
        actor->movedir =olddir;

        if (P_TryWalk(actor))
        return;
    }

    // randomly determine direction of search
    if (P_Random()&1) 	
    {
        for (tdir=DI_EAST ; tdir<=DI_SOUTHEAST ; tdir++)
        {
            if (tdir != (int) turnaround)
            {
                actor->movedir =tdir;

                if (P_TryWalk(actor))
                return;
            }
        }
    }
    else
    {
        for (tdir=DI_SOUTHEAST ; tdir != (DI_EAST-1) ; tdir--)
        {
            if (tdir != (int) turnaround)
            {
                actor->movedir = tdir;
		
                if ( P_TryWalk(actor) )
                return;
            }
        }
    }

    if (turnaround !=  DI_NODIR)
    {
        actor->movedir = turnaround;

        if (P_TryWalk(actor))
        return;
    }

    actor->movedir = DI_NODIR;  // can not move
}


// -----------------------------------------------------------------------------
// P_LookForPlayers
// If allaround is false, only look 180 degrees in front.
// Returns true if a player is targeted.
// -----------------------------------------------------------------------------

boolean P_LookForPlayers (mobj_t *actor, boolean allaround)
{
    int       c;
    int       stop;
    angle_t   an;
    fixed_t   dist;
    player_t *player;

    c = 0;
    stop = (actor->lastlook-1)&3;

    for ( ; ; actor->lastlook = (actor->lastlook+1)&3 )
    {
        if (!playeringame[actor->lastlook])
        continue;

        if (c++ == 2 || actor->lastlook == stop)
        {
            // done looking
            return false;	
        }

        player = &players[actor->lastlook];

        if (player->health <= 0)
        continue;       // dead

        if (!P_CheckSight (actor, player->mo))
        continue;       // out of sight

        if (!allaround)
        {
            an = R_PointToAngle2 (actor->x,
                                  actor->y, 
                                  player->mo->x,
                                  player->mo->y)
                                - actor->angle;

            if (an > ANG90 && an < ANG270)
            {
                dist = P_AproxDistance (player->mo->x - actor->x,
                                        player->mo->y - actor->y);
                // if real close, react anyway
                if (dist > MELEERANGE)
                continue;	// behind back
            }
        }

        actor->target = player->mo;
        return true;
    }
    return false;
}


// =============================================================================
// ACTION ROUTINES
// =============================================================================


// -----------------------------------------------------------------------------
// A_Look
//
// Stay in state until a player is sighted.
// -----------------------------------------------------------------------------

void A_Look (mobj_t *actor)
{
    mobj_t *targ;

    actor->threshold = 0;   // any shot will wake up
    targ = actor->subsector->sector->soundtarget;

    if (targ && (targ->flags & MF_SHOOTABLE))
    {
        actor->target = targ;

        if ( actor->flags & MF_AMBUSH )
        {
            if (P_CheckSight (actor, actor->target))
            goto seeyou;
        }
        else
            goto seeyou;
        }
	
        if (!P_LookForPlayers (actor, false) )
        return;

        // go into chase state
        seeyou:
        if (actor->info->seesound)
        {
            int sound;

            switch (actor->info->seesound)
            {
                case sfx_posit1:
                case sfx_posit2:
                case sfx_posit3:
                sound = sfx_posit1+P_Random()%3;
                break;

                case sfx_bgsit1:
                case sfx_bgsit2:
                sound = sfx_bgsit1+P_Random()%2;
                break;

                default:
                sound = actor->info->seesound;
                break;
            }

            S_StartSound (actor, sound);
        }

    P_SetMobjState (actor, actor->info->seestate);
} 


// -----------------------------------------------------------------------------
// A_Chase
//
// Actor has a melee attack,
// so it tries to close as fast as possible
// -----------------------------------------------------------------------------

void A_Chase (mobj_t *actor)
{
    int delta;

    if (actor->reactiontime)
    actor->reactiontime--;

    // modify target threshold
    if  (actor->threshold)
    {
        if (!actor->target || actor->target->health <= 0)
        {
            actor->threshold = 0;
        }
        else
        {
            actor->threshold--;
        }
    }

    // turn towards movement direction if not there yet
    if (actor->movedir < 8)
    {
        actor->angle &= (7<<29);
        delta = actor->angle - (actor->movedir << 29);

        if (delta > 0)
        actor->angle -= ANG90/2;
        else if (delta < 0)
        actor->angle += ANG90/2;
    }

    if (!actor->target || !(actor->target->flags & MF_SHOOTABLE))
    {
        // look for a new target
        if (P_LookForPlayers(actor,true))
        return; // got a new target

        P_SetMobjState (actor, actor->info->spawnstate);
        return;
    }

    // do not attack twice in a row
    // [Julia] Jaguar: don't attack twice in Nightmare and Ultra-Nightmare
    if (actor->flags & MF_JUSTATTACKED)
    {
        actor->flags &= ~MF_JUSTATTACKED;
        // if (gameskill != sk_nightmare && gameskill != sk_ultranm && !fastparm)
        P_NewChaseDir (actor);
        return;
    }
    
    // check for melee attack
    if (actor->info->meleestate && P_CheckMeleeRange (actor))
    {
        if (actor->info->attacksound)
        S_StartSound (actor, actor->info->attacksound);

        P_SetMobjState (actor, actor->info->meleestate);
        return;
    }

    // check for missile attack
    if (actor->info->missilestate)
    {
        if (gameskill < sk_nightmare && actor->movecount)
        {
            goto nomissile;
        }
    
        if (!P_CheckMissileRange (actor))
        goto nomissile;
    
        P_SetMobjState (actor, actor->info->missilestate);
        actor->flags |= MF_JUSTATTACKED;
        return;
    }

    // ?
    nomissile:
    // chase towards player
    if (--actor->movecount < 0 || !P_Move (actor))
    {
        P_NewChaseDir (actor);
    }

    // make active sound
    if (actor->info->activesound && P_Random () < 3)
    {
        S_StartSound (actor, actor->info->activesound);
    }
}


// -----------------------------------------------------------------------------
// A_FaceTarget
// -----------------------------------------------------------------------------

void A_FaceTarget (mobj_t *actor)
{	
    if (!actor->target)
    return;

    actor->flags &= ~MF_AMBUSH;

    actor->angle = R_PointToAngle2 (actor->x,
                                    actor->y,
                                    actor->target->x,
                                    actor->target->y);
}


// -----------------------------------------------------------------------------
// A_PosAttack
// -----------------------------------------------------------------------------

void A_PosAttack (mobj_t *actor)
{
    int angle;
    int damage;
    int slope;

    if (!actor->target)
    return;

    A_FaceTarget (actor);
    angle = actor->angle;
    slope = P_AimLineAttack (actor, angle, MISSILERANGE);

    S_StartSound (actor, sfx_pistol);
    angle += P_SubRandom() << 20;

    if (sk_ultranm && !demorecording && !demoplayback)
    damage = ((P_Random()%5)+1)*4;
    else
    damage = ((P_Random()%5)+1)*3;

    P_LineAttack (actor, angle, MISSILERANGE, slope, damage);
}

// -----------------------------------------------------------------------------
// A_SPosAttack
// -----------------------------------------------------------------------------

void A_SPosAttack (mobj_t *actor)
{
    int i;
    int angle;
    int bangle;
    int damage;
    int slope;

    if (!actor->target)
    return;

    S_StartSound (actor, sfx_shotgn);
    A_FaceTarget (actor);
    bangle = actor->angle;
    slope = P_AimLineAttack (actor, bangle, MISSILERANGE);

    for (i=0 ; i<3 ; i++)
    {
    angle = bangle + (P_SubRandom() << 20);

    if (sk_ultranm)
    damage = ((P_Random()%5)+1)*4;
    else
    damage = ((P_Random()%5)+1)*3;

    P_LineAttack (actor, angle, MISSILERANGE, slope, damage);
    }   
}


// -----------------------------------------------------------------------------
// A_TroopAttack
// -----------------------------------------------------------------------------

void A_TroopAttack (mobj_t *actor)
{
    int damage;

    if (!actor->target)
    return;

    A_FaceTarget (actor);

    if (P_CheckMeleeRange (actor))
    {
        S_StartSound (actor, sfx_claw);
        damage = (P_Random()%8+1)*3;
        P_DamageMobj (actor->target, actor, actor, damage);
        return;
    }
    
    // launch a missile
    P_SpawnMissile (actor, actor->target, MT_TROOPSHOT);
}


// -----------------------------------------------------------------------------
// A_SargAttack
// -----------------------------------------------------------------------------

void A_SargAttack (mobj_t* actor)
{
    int damage;

    if (!actor->target)
    return;

    A_FaceTarget (actor);

    if (P_CheckMeleeRange (actor))
    {
        damage = ((P_Random()%10)+1) * 4;
        P_DamageMobj (actor->target, actor, actor, damage);
    }
}


// -----------------------------------------------------------------------------
// A_HeadAttack
// -----------------------------------------------------------------------------

void A_HeadAttack (mobj_t *actor)
{
    int damage;

    if (!actor->target)
    return;

    A_FaceTarget (actor);

    if (P_CheckMeleeRange (actor))
    {
        damage = (P_Random()%6+1)*10;
        P_DamageMobj (actor->target, actor, actor, damage);
        return;
    }

    // launch a missile
    P_SpawnMissile (actor, actor->target, MT_HEADSHOT);
}


// -----------------------------------------------------------------------------
// A_BruisAttack
// -----------------------------------------------------------------------------

void A_BruisAttack (mobj_t *actor)
{
    int damage;

    if (!actor->target)
    return;

    // [Julia] Fix: https://doomwiki.org/wiki/Baron_attacks_a_monster_behind_him
    A_FaceTarget (actor);

    if (P_CheckMeleeRange (actor))
    {
        S_StartSound (actor, sfx_claw);
        damage = (P_Random()%8+1)*10;
        P_DamageMobj (actor->target, actor, actor, damage);
        return;
    }

    // launch a missile
    P_SpawnMissile (actor, actor->target, MT_BRUISERSHOT);
}


// -----------------------------------------------------------------------------
// SkullAttack
// Fly at the player like a missile.
// -----------------------------------------------------------------------------

#define	SKULLSPEED (20*FRACUNIT)

void A_SkullAttack (mobj_t *actor)
{
    int      dist;
    angle_t  an;
    mobj_t  *dest;

    if (!actor->target)
	return;

    dest = actor->target;	
    actor->flags |= MF_SKULLFLY;

    S_StartSound (actor, actor->info->attacksound);
    A_FaceTarget (actor);
    an = actor->angle >> ANGLETOFINESHIFT;
    actor->momx = FixedMul (SKULLSPEED, finecosine[an]);
    actor->momy = FixedMul (SKULLSPEED, finesine[an]);
    dist = P_AproxDistance (dest->x - actor->x, dest->y - actor->y);
    dist = dist / SKULLSPEED;

    if (dist < 1)
    dist = 1;
    actor->momz = (dest->z+(dest->height>>1) - actor->z) / dist;
}


// -----------------------------------------------------------------------------
// A_Scream
// -----------------------------------------------------------------------------

void A_Scream (mobj_t *actor)
{
    int sound;

    switch (actor->info->deathsound)
    {
        case 0:
        return;

        case sfx_podth1:
        case sfx_podth2:
        case sfx_podth3:
        sound = sfx_podth1 + P_Random ()%3;
        break;

        case sfx_bgdth1:
        case sfx_bgdth2:
        sound = sfx_bgdth1 + P_Random ()%2;
        break;
	
        default:
        sound = actor->info->deathsound;
        break;
    }

    S_StartSound (actor, sound);
}


// -----------------------------------------------------------------------------
// S_StartSound
// -----------------------------------------------------------------------------

void A_XScream (mobj_t *actor)
{
    S_StartSound (actor, sfx_slop);	
}


// -----------------------------------------------------------------------------
// A_Pain
// -----------------------------------------------------------------------------

void A_Pain (mobj_t *actor)
{
    // [Julia] Don't break player's firing sounds by pain sounds
    if (actor->info->doomednum == -1)
    S_StartSound (NULL, actor->info->painsound);

    else if (actor->info->painsound)
    S_StartSound (actor, actor->info->painsound);	
}


// -----------------------------------------------------------------------------
// A_Fall
// -----------------------------------------------------------------------------

void A_Fall (mobj_t *actor)
{
    // actor is on ground, it can be walked over
    actor->flags &= ~MF_SOLID;
}


// -----------------------------------------------------------------------------
// A_Explode
// -----------------------------------------------------------------------------

void A_Explode (mobj_t *thingy)
{
    P_RadiusAttack(thingy, thingy->target, 128);
}


// -----------------------------------------------------------------------------
// A_BossDeath
//
// Possibly trigger special effects
// if on first boss level
// -----------------------------------------------------------------------------

void A_BossDeath (mobj_t *mo)
{
    int         i;
    line_t      junk;
    thinker_t  *th;
    mobj_t     *mo2;
    
    // [Julia] Jaguar: bruisers apear on other levels
    if (gamemap != 8)
    return;

    if (mo->type != MT_BRUISER)
    return;        

    // make sure there is a player alive for victory
    for (i=0 ; i<MAXPLAYERS ; i++)
        if (playeringame[i] && players[i].health > 0)
            break;

    if (i==MAXPLAYERS)
    return;	// no one left alive, so do not end game

    // scan the remaining thinkers to see if all bosses are dead
    for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    {
        if (th->function.acp1 != (actionf_p1)P_MobjThinker)
        continue;

        mo2 = (mobj_t *)th;

        if (mo2 != mo && mo2->type == mo->type && mo2->health > 0)
        {
            // other boss not dead
            return;
        }
    }

    // victory!
    if (gamemap == 8)
    {
        if (mo->type == MT_BRUISER)
        {
            junk.tag = 666;
            EV_DoFloor (&junk, lowerFloorToLowest);
            return;
        }
    }

    G_ExitLevel ();
}

// -----------------------------------------------------------------------------
// A_ReFire
// -----------------------------------------------------------------------------

void A_ReFire (mobj_t *mobj, player_t *player, pspdef_t *psp );


// -----------------------------------------------------------------------------
// A_PlayerScream
// -----------------------------------------------------------------------------

void A_PlayerScream (mobj_t *mo)
{
    // [Julia] No commercial extra death sound
    S_StartSound (mo, sfx_pldeth);
}
