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
// DESCRIPTION:
//	Handling interactions (i.e., collisions).
//


#include "d_englsh.h"
#include "doomstat.h"
#include "m_random.h"
#include "i_system.h"
#include "am_map.h"
#include "p_local.h"
#include "s_sound.h"
#include "ct_chat.h"

#include "id_vars.h"
#include "id_func.h"


#define BONUSADD	6

// a weapon is found with two clip loads,
// a big item has five clip loads
int maxammo[NUMAMMO] = {200, 50, 300, 50};
int clipammo[NUMAMMO] = {10, 4, 20, 1};


//
// GET STUFF
//

//
// P_GiveAmmo
// Num is the number of clip loads,
// not the individual count (0= 1/2 clip).
// Returns false if the ammo can't be picked up at all
//

boolean
P_GiveAmmo
( player_t*	player,
  ammotype_t	ammo,
  int		num )
{
    int		oldammo;
	
    if (ammo == am_noammo)
	return false;
		
    if (ammo >= NUMAMMO)
	I_Error ("P_GiveAmmo: bad type %i", ammo);
		
    if ( player->ammo[ammo] == player->maxammo[ammo]  )
	return false;
		
    if (num)
	num *= clipammo[ammo];
    else
	num = clipammo[ammo]/2;
    
    // [JN] Jaguar: no double ammo in Nightmare.
    if (gameskill == sk_baby
	/*|| gameskill == sk_nightmare*/)
    {
	// give double ammo in trainer mode,
	// you'll need in nightmare
	num <<= 1;
    }
    
		
    oldammo = player->ammo[ammo];
    player->ammo[ammo] += num;

    if (player->ammo[ammo] > player->maxammo[ammo])
	player->ammo[ammo] = player->maxammo[ammo];

    // If non zero ammo, 
    // don't change up weapons,
    // player was lower on purpose.
    if (oldammo)
	return true;	

    // We were down to zero,
    // so select a new weapon.
    // Preferences are not user selectable.
    switch (ammo)
    {
      case am_clip:
	if (player->readyweapon == wp_fist)
	{
	    if (player->weaponowned[wp_chaingun])
		player->pendingweapon = wp_chaingun;
	    else
		player->pendingweapon = wp_pistol;
	}
	break;
	
      case am_shell:
	if (player->readyweapon == wp_fist
	    || player->readyweapon == wp_pistol)
	{
	    if (player->weaponowned[wp_shotgun])
		player->pendingweapon = wp_shotgun;
	}
	break;
	
      case am_cell:
	if (player->readyweapon == wp_fist
	    || player->readyweapon == wp_pistol)
	{
	    if (player->weaponowned[wp_plasma])
		player->pendingweapon = wp_plasma;
	}
	break;
	
      case am_misl:
	if (player->readyweapon == wp_fist)
	{
	    if (player->weaponowned[wp_missile])
		player->pendingweapon = wp_missile;
	}
      default:
	break;
    }
	
    return true;
}

/* 
=================== 
= 
= P_GiveWeapon
=
= The weapon name may have a MF_DROPPED flag ored in
=================== 
*/ 

boolean P_GiveWeapon (player_t *player, weapontype_t weapon, boolean dropped)
{
    boolean	gaveammo;
    boolean	gaveweapon;
	
	if (weaponinfo[weapon].ammo != am_noammo)
	{	/* give one clip with a dropped weapon, two clips with a found weapon */
		if (dropped)
			gaveammo = P_GiveAmmo (player, weaponinfo[weapon].ammo, 1);
		else
			gaveammo = P_GiveAmmo (player, weaponinfo[weapon].ammo, 2);
	}
	else
		gaveammo = false;
	
	if (player->weaponowned[weapon])
		gaveweapon = false;
	else
	{
		gaveweapon = true;
		player->weaponowned[weapon] = true;
		player->pendingweapon = weapon;
	}
	
	return (gaveweapon || gaveammo);
}

/* 
=================== 
= 
= P_GiveBody
=
= Returns false if the body isn't needed at all
=================== 
*/ 

boolean P_GiveBody (player_t *player, int num)
{
	if (player->health >= MAXHEALTH)
		return false;
		
	player->health += num;
	if (player->health > MAXHEALTH)
		player->health = MAXHEALTH;
	player->mo->health = player->health;
	
	return true;
}

/* 
=================== 
= 
= P_GiveArmor
=
= Returns false if the armor is worse than the current armor
=================== 
*/ 

boolean P_GiveArmor (player_t *player, int armortype)
{
	int		hits;
	
	hits = armortype*100;
	if (player->armorpoints >= hits)
		return false;		/* don't pick up */
		
	player->armortype = armortype;
	player->armorpoints = hits;
	
	return true;
}

/* 
=================== 
= 
= P_GiveCard
=
=================== 
*/ 

void P_GiveCard (player_t *player, card_t card)
{
	if (player->cards[card])
		return;		
	player->bonuscount = BONUSADD;
	player->cards[card] = 1;
}

/* 
=================== 
= 
= P_GivePower
=
=================== 
*/ 
boolean P_GivePower (player_t *player, int /*powertype_t*/ power)
{
	if (power == pw_invulnerability)
	{
		player->powers[power] = INVULNTICS;
		return true;
	}
	if (power == pw_invisibility)
	{
		player->powers[power] = INVISTICS;
		player->mo->flags |= MF_SHADOW;
		return true;
	}
	if (power == pw_ironfeet)
	{
		player->powers[power] = IRONTICS;
		return true;
	}
	if (power == pw_strength)
	{
		P_GiveBody (player, 100);
		player->powers[power] = 1;
		return true;
	}
	
	if (player->powers[power])
		return false;		/* already got it */
		
	player->powers[power] = 1;
	return true;
}

// -----------------------------------------------------------------------------
// P_TouchSpecialThing
// -----------------------------------------------------------------------------

void P_TouchSpecialThing (mobj_t *special, mobj_t *toucher)
{
    int       i;
    int       sound;
    fixed_t	  delta;
    player_t *player;

    delta = special->z - toucher->z;

    if (delta > toucher->height || delta < -8 * FRACUNIT)
    {
        // out of reach
        return;
    }
	
    sound = sfx_itemup;	
    player = toucher->player;

    // Dead thing touching.
    // Can happen with a sliding player corpse.
    if (toucher->health <= 0)
	return;

    // Identify by sprite.
    switch (special->sprite)
    {
        // armor
        case SPR_ARM1:
        if (!P_GiveArmor (player, 1))
            return;
        CT_SetMessage(player, GOTARMOR, false, NULL);
        break;

        case SPR_ARM2:
        if (!P_GiveArmor (player, 2))
            return;
        CT_SetMessage(player, GOTMEGA, false, NULL);
        break;

        // bonus items
        case SPR_BON1:
        // [JN] Jaguar: adds 2 points.
        player->health += 2;		// can go over 100%
        if (player->health > 200)
            player->health = 200;
        player->mo->health = player->health;
        CT_SetMessage(player, GOTHTHBONUS, false, NULL);
        break;
	
        case SPR_BON2:
        player->armorpoints += 2;		// can go over 100%
        // [JN] Jaguar: adds 2 points.
        if (player->armorpoints > 200)
            player->armorpoints = 200;
        if (!player->armortype)
            player->armortype = 1;
        CT_SetMessage(player, GOTARMBONUS, false, NULL);
        break;

        case SPR_SOUL:
        player->health += 100;
        if (player->health > 200)
            player->health = 200;
        player->mo->health = player->health;
        CT_SetMessage(player, GOTSUPER, false, NULL);
        break;
	
        case SPR_MEGA:
        player->health = 200;
        player->mo->health = player->health;
        P_GiveArmor (player, 2);
        CT_SetMessage(player, GOTMSPHERE, false, NULL);
        break;

        // cards
        // leave cards for everyone
        case SPR_BKEY:
        if (!player->cards[it_bluecard])
            CT_SetMessage(player, GOTBLUECARD, false, NULL);
        P_GiveCard (player, it_bluecard);
            break;
        return;
	
        case SPR_YKEY:
        if (!player->cards[it_yellowcard])
            CT_SetMessage(player, GOTYELWCARD, false, NULL);
        P_GiveCard (player, it_yellowcard);
            break;
        return;
	
        case SPR_RKEY:
        if (!player->cards[it_redcard])
            CT_SetMessage(player, GOTREDCARD, false, NULL);
        P_GiveCard (player, it_redcard);
            break;
        return;
	
        case SPR_BSKU:
        if (!player->cards[it_blueskull])
            CT_SetMessage(player, GOTBLUESKUL, false, NULL);
        P_GiveCard (player, it_blueskull);
            break;
        return;
	
        case SPR_YSKU:
        if (!player->cards[it_yellowskull])
            CT_SetMessage(player, GOTYELWSKUL, false, NULL);
        P_GiveCard (player, it_yellowskull);
            break;
        return;
	
        case SPR_RSKU:
        if (!player->cards[it_redskull])
            CT_SetMessage(player, GOTREDSKULL, false, NULL);
        P_GiveCard (player, it_redskull);
            break;
        return;
	
        // medikits, heals
        case SPR_STIM:
        if (!P_GiveBody (player, 10))
            return;
        CT_SetMessage(player, GOTSTIM, false, NULL);
        break;
	
        case SPR_MEDI:
        if (!P_GiveBody (player, 25))
            return;
        // [JN] Fix for "Medikit that you really needed!"
        if (player->health < 50)
            CT_SetMessage(player, GOTMEDINEED, false, NULL);
        else
            CT_SetMessage(player, GOTMEDIKIT, false, NULL);
        break;

	
        // power ups
        case SPR_PINV:
        if (!P_GivePower (player, pw_invulnerability))
            return;
        CT_SetMessage(player, GOTINVUL, false, NULL);
        break;
	
        case SPR_PSTR:
        if (!P_GivePower (player, pw_strength))
            return;
        CT_SetMessage(player, GOTBERSERK, false, NULL);
        if (player->readyweapon != wp_fist)
            player->pendingweapon = wp_fist;
        break;
	
        case SPR_PINS:
        if (!P_GivePower (player, pw_invisibility))
            return;
        CT_SetMessage(player, GOTINVIS, false, NULL);
        break;

        case SPR_SUIT:
        if (!P_GivePower (player, pw_ironfeet))
            return;
        CT_SetMessage(player, GOTSUIT, false, NULL);
        break;

        case SPR_PMAP:
        if (!P_GivePower (player, pw_allmap))
            return;
        CT_SetMessage(player, GOTMAP, false, NULL);
        break;
	
        case SPR_PVIS:
        break;
	
        // ammo
        case SPR_CLIP:
        if (special->flags & MF_DROPPED)
        {
            if (!P_GiveAmmo (player,am_clip,0))
            return;
        }
        else
        {
            if (!P_GiveAmmo (player,am_clip,1))
            return;
        }
        CT_SetMessage(player, GOTCLIP, false, NULL);
        break;
	
        case SPR_AMMO:
        if (!P_GiveAmmo (player, am_clip,5))
            return;
        CT_SetMessage(player, GOTCLIPBOX, false, NULL);
        break;
	
        case SPR_ROCK:
        if (!P_GiveAmmo (player, am_misl,1))
            return;
        CT_SetMessage(player, GOTROCKET, false, NULL);
        break;

        case SPR_BROK:
        if (!P_GiveAmmo (player, am_misl,5))
            return;
        CT_SetMessage(player, GOTROCKBOX, false, NULL);
        break;
	
        case SPR_CELL:
        if (!P_GiveAmmo (player, am_cell,1))
            return;
        CT_SetMessage(player, GOTCELL, false, NULL);
        break;
	
        case SPR_CELP:
        if (!P_GiveAmmo (player, am_cell,5))
            return;
        CT_SetMessage(player, GOTCELLBOX, false, NULL);
        break;
	
        case SPR_SHEL:
        if (!P_GiveAmmo (player, am_shell,1))
            return;
        CT_SetMessage(player, GOTSHELLS, false, NULL);
        break;
	
        case SPR_SBOX:
        if (!P_GiveAmmo (player, am_shell,5))
            return;
        CT_SetMessage(player, GOTSHELLBOX, false, NULL);
        break;
	
        case SPR_BPAK:
        if (!player->backpack)
        {
            for (i=0 ; i<NUMAMMO ; i++)
                player->maxammo[i] *= 2;
            player->backpack = true;
        }
        for (i=0 ; i<NUMAMMO ; i++)
            P_GiveAmmo (player, i, 1);
        CT_SetMessage(player, GOTBACKPACK, false, NULL);
        break;
	
        // weapons
        case SPR_BFUG:
        if (!P_GiveWeapon (player, wp_bfg, false) )
            return;
        CT_SetMessage(player, GOTBFG9000, false, NULL);
        sound = sfx_wpnup;	
        break;
	
        case SPR_MGUN:
            if (!P_GiveWeapon(player, wp_chaingun,
                            (special->flags & MF_DROPPED) != 0))
            return;
        CT_SetMessage(player, GOTCHAINGUN, false, NULL);
        sound = sfx_wpnup;	
        break;
	
        case SPR_CSAW:
        if (!P_GiveWeapon (player, wp_chainsaw, false) )
            return;
        CT_SetMessage(player, GOTCHAINSAW, false, NULL);
        sound = sfx_wpnup;	
        break;
	
        case SPR_LAUN:
        if (!P_GiveWeapon (player, wp_missile, false) )
            return;
        CT_SetMessage(player, GOTLAUNCHER, false, NULL);
        sound = sfx_wpnup;	
        break;
	
        case SPR_PLAS:
        if (!P_GiveWeapon (player, wp_plasma, false) )
            return;
        CT_SetMessage(player, GOTPLASMA, false, NULL);
        sound = sfx_wpnup;	
        break;
	
        case SPR_SHOT:
            if (!P_GiveWeapon(player, wp_shotgun,
                            (special->flags & MF_DROPPED) != 0))
                return;
        CT_SetMessage(player, GOTSHOTGUN, false, NULL);
        sound = sfx_wpnup;	
        break;
		
        case SPR_SGN2:
            if (!P_GiveWeapon(player, wp_supershotgun,
                            (special->flags & MF_DROPPED) != 0))
                return;
        CT_SetMessage(player, GOTSHOTGUN2, false, NULL);
        sound = sfx_wpnup;	
        break;
        
        default:
        I_Error ("P_SpecialThing: Unknown gettable thing");
    }

    if (special->flags & MF_COUNTITEM)
        player->itemcount++;
    P_RemoveMobj (special);

    player->bonuscount += BONUSADD;
    // [JN] Limit bonus palette duration to 4 seconds.
    if (player->bonuscount > 4 * TICRATE)
    player->bonuscount = 4 * TICRATE;

    if (player == &players[displayplayer])
    S_StartSound (NULL, sound);
}


//
// KillMobj
//
void
P_KillMobj
( mobj_t*	source,
  mobj_t*	target )
{
    mobjtype_t	item;
    mobj_t*	mo;
	
    target->flags &= ~(MF_SHOOTABLE|MF_FLOAT|MF_SKULLFLY);

    if (target->type != MT_SKULL)
	target->flags &= ~MF_NOGRAVITY;

    target->flags |= MF_CORPSE|MF_DROPOFF;
    target->height >>= 2;
    target->geartics = 15 * TICRATE;  // [JN] Limit torque to 15 seconds.

    if (source && source->player)
    {
        // count for intermission
        if (target->flags & MF_COUNTKILL)
        {
            source->player->killcount++;	
        }
    }
    else if (target->flags & MF_COUNTKILL)
    {
        // count all monster deaths,
        // even those caused by other monsters
        players[0].killcount++;	
    }
    
    if (target->player)
    {
			
	target->flags &= ~MF_SOLID;
	target->player->playerstate = PST_DEAD;
	P_DropWeapon (target->player);
	// [crispy] center view when dying
	target->player->centering = true;
	// [JN] & [crispy] Reset the yellow bonus palette when the player dies
	target->player->bonuscount = 0;
	// [JN] & [crispy] Remove the effect of the inverted palette when the player dies
	target->player->fixedcolormap = 0;

	if (target->player == &players[consoleplayer]
	    && automapactive)
	{
	    // don't die in auto map,
	    // switch view prior to dying
	    AM_Stop ();
	}
	
    }

    // [JN] Jaguar: player got forcefully gibbed with < -50 health, though in
    // original Jaguar this is a status bar only effect and it's a bit broken.
    if (target->player && target->health < -50)
    {
	P_SetMobjState (target, target->info->xdeathstate);
    }
    else
    if (target->health < -target->info->spawnhealth 
	&& target->info->xdeathstate)
    {
	P_SetMobjState (target, target->info->xdeathstate);
    }
    else
	P_SetMobjState (target, target->info->deathstate);
    target->tics -= P_Random()&3;

    // [crispy] randomly flip corpse, blood and death animation sprites
    if (target->flags & MF_FLIPPABLE)
    {
	target->health = (target->health & (int)~1) - (ID_RealRandom() & 1);
    }

    if (target->tics < 1)
	target->tics = 1;
		
    //	I_StartSound (&actor->r, actor->info->deathsound);

    // Drop stuff.
    // This determines the kind of object spawned
    // during the death frame of a thing.
    switch (target->type)
    {
      case MT_POSSESSED:
	item = MT_CLIP;
	break;
	
      case MT_SHOTGUY:
	item = MT_SHOTGUN;
	break;
	
      case MT_CHAINGUY:
	item = MT_CHAINGUN;
	break;
    
      default:
	return;
    }

    mo = P_SpawnMobj (target->x,target->y,ONFLOORZ, item);
    mo->flags |= MF_DROPPED;	// special versions of items
}




//
// P_DamageMobj
// Damages both enemies and players
// "inflictor" is the thing that caused the damage
//  creature or missile, can be NULL (slime, etc)
// "source" is the thing to target after taking damage
//  creature or NULL
// Source and inflictor are the same for melee attacks.
// Source can be NULL for slime, barrel explosions
// and other environmental stuff.
//
void
P_DamageMobj
( mobj_t*	target,
  mobj_t*	inflictor,
  mobj_t*	source,
  int 		damage )
{
    unsigned	ang;
    int		saved;
    player_t*	player;
    fixed_t	thrust;
	
    if ( !(target->flags & MF_SHOOTABLE) )
	return;	// shouldn't happen...
		
    if (target->health <= 0)
	return;

    if ( target->flags & MF_SKULLFLY )
    {
	target->momx = target->momy = target->momz = 0;
    }
	
    player = target->player;
    if (player && gameskill == sk_baby)
	damage >>= 1; 	// take half damage in trainer mode
		

    // Some close combat weapons should not
    // inflict thrust and push the victim out of reach,
    // thus kick away unless using the chainsaw.
    if (inflictor
	&& !(target->flags & MF_NOCLIP)
	&& (!source
	    || !source->player
	    || source->player->readyweapon != wp_chainsaw))
    {
	ang = R_PointToAngle2 ( inflictor->x,
				inflictor->y,
				target->x,
				target->y);
		
	thrust = damage*(FRACUNIT>>3)*100/target->info->mass;

	// make fall forwards sometimes
	if ( damage < 40
	     && damage > target->health
	     && target->z - inflictor->z > 64*FRACUNIT
	     && (P_Random ()&1) )
	{
	    ang += ANG180;
	    thrust *= 4;
	}
		
	ang >>= ANGLETOFINESHIFT;
	target->momx += FixedMul (thrust, finecosine[ang]);
	target->momy += FixedMul (thrust, finesine[ang]);
    }
    
    // player specific
    if (player)
    {
	// end of game hell hack
	if (target->subsector->sector->special == 11
	    && damage >= target->health)
	{
	    damage = target->health - 1;
	}
	

	// Below certain threshold,
	// ignore damage in GOD mode, or with INVUL power.
	if ( damage < 1000
	     && ( (player->cheats&CF_GODMODE)
		  || player->powers[pw_invulnerability] ) )
	{
	    return;
	}
	
	if (player->armortype)
	{
	    if (player->armortype == 1)
		saved = damage/3;
	    else
		saved = damage/2;
	    
	    if (player->armorpoints <= saved)
	    {
		// armor is used up
		saved = player->armorpoints;
		player->armortype = 0;
	    }
	    player->armorpoints -= saved;
	    damage -= saved;
	}
	player->health -= damage; 	// mirror mobj health here for Dave
	player->health_negative = player->health;  // [JN] Update negative health value.
    // [JN] BUDDHA cheat.
	if (player->cheats & CF_BUDDHA && player->health < 1)
        player->health = 1;
	else
	if (player->health < 0)
	    player->health = 0;
	
	player->attacker = source;
	player->damagecount += damage;	// add damage after armor / invuln

	if (player->damagecount > 100)
	    player->damagecount = 100;	// teleport stomp does 10k points...
    }
    
    // do the damage	
    target->health -= damage;	
    // [JN] BUDDHA cheat.
    if (player && player->cheats & CF_BUDDHA && target->health < 1)
    {
	    target->health = 1;
    }
    else
    if (target->health <= 0)
    {
	P_KillMobj (source, target);
	return;
    }

    if ( (P_Random () < target->info->painchance)
	 && !(target->flags&MF_SKULLFLY) )
    {
	target->flags |= MF_JUSTHIT;	// fight back!
	
	P_SetMobjState (target, target->info->painstate);
    }
			
    target->reactiontime = 0;		// we're awake now...	

	if (!target->threshold && source)
    {
	// if not intent on another player,
	// chase after this one
	target->target = source;
	target->threshold = BASETHRESHOLD;
	if (target->state == &states[target->info->spawnstate]
	    && target->info->seestate != S_NULL)
	    P_SetMobjState (target, target->info->seestate);
    }
			
}

