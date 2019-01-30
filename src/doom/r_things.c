//
// Copyright(C) 1993-1996 Id Software, Inc.
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


#include <stdio.h>
#include <stdlib.h>

#include "deh_main.h"
#include "doomdef.h"
#include "i_swap.h"
#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"
#include "r_local.h"
#include "doomstat.h"
#include "v_trans.h"
#include "g_game.h"
#include "jn.h"


#define MINZ        (FRACUNIT*4)
#define BASEYCENTER (ORIGHEIGHT/2)


typedef struct
{
    int x1;
    int x2;
    int column;
    int topclip;
    int bottomclip;
} maskdraw_t;

extern boolean chainsaw_attack_swing;


//
// Sprite rotation 0 is facing the viewer,
//  rotation 1 is one angle turn CLOCKWISE around the axis.
// This is not the same as the angle,
//  which increases counter clockwise (protractor).
// There was a lot of stuff grabbed wrong, so I changed it...
//
fixed_t pspritescale;
fixed_t pspriteiscale;

lighttable_t  **spritelights;

// [Julia] Brightmaps
lighttable_t  **fullbrights_redonly;
lighttable_t  **fullbrights_dimmeditems;

// constant arrays used for psprite clipping and initializing clipping
int negonearray[SCREENWIDTH];
int screenheightarray[SCREENWIDTH];

// =============================================================================
// INITIALIZATION FUNCTIONS
// =============================================================================

// variables used to look up and range check thing_t sprites patches
spritedef_t*    sprites;
int             numsprites;

spriteframe_t   sprtemp[29];
int             maxframe;
char*           spritename;

// =============================================================================
// GAME FUNCTIONS
// =============================================================================

vissprite_t   vissprites[MAXVISSPRITES];
vissprite_t  *vissprite_p;

int	  newvissprite;
int  *mfloorclip;
int  *mceilingclip;

fixed_t      spryscale;
int64_t      sprtopscreen; // [crispy] WiggleFix
vissprite_t  vsprsortedhead;


// -------------------------------------------------------------------------
//
// R_InstallSpriteLump
// Local function for R_InitSprites.
//
// -------------------------------------------------------------------------

void R_InstallSpriteLump (int lump, unsigned frame, char rot, boolean flipped)
{
    int r;

    // [crispy] support 16 sprite rotations
    unsigned rotation = (rot >= 'A') ? rot - 'A' + 10 : 
                                      (rot >= '0') ? rot - '0' : 17;

    if (frame >= 29 || rotation > 16) // [crispy] support 16 sprite rotations
	I_Error("R_InstallSpriteLump: Bad frame characters in lump %i", lump);

    if ((int)frame > maxframe)
    maxframe = frame;

    if (rotation == 0)
    {
        // the lump should be used for all rotations
        if (sprtemp[frame].rotate == false)
        {
            I_Error ("R_InitSprites: Sprite %s frame %c has multip rot=0 lump",
                     spritename, 'A'+frame);
        }

        if (sprtemp[frame].rotate == true)
        {
            I_Error ("R_InitSprites: Sprite %s frame %c has rotations and a rot=0 lump\n",
                     spritename, 'A'+frame);
        }

        sprtemp[frame].rotate = false;

        for (r=0 ; r<16 ; r++) // [crispy] support 16 sprite rotations
        {
            sprtemp[frame].lump[r] = lump - firstspritelump;
            sprtemp[frame].flip[r] = (byte)flipped;
        }

        return;
    }

    // the lump is only used for one rotation
    if (sprtemp[frame].rotate == false)
    I_Error ("R_InitSprites: Sprite %s frame %c has rotations and a rot=0 lump",
             spritename, 'A'+frame);

    sprtemp[frame].rotate = true;

    // make 0 based
    rotation--;		
    if (sprtemp[frame].lump[rotation] != -1)
    I_Error ("R_InitSprites: Sprite %s : %c : %c has two lumps mapped to it",
             spritename, 'A'+frame, '1'+rotation);

    sprtemp[frame].lump[rotation] = lump - firstspritelump;
    sprtemp[frame].flip[rotation] = (byte)flipped;
}


// -------------------------------------------------------------------------
// R_InitSpriteDefs
//
// Pass a null terminated list of sprite names
//  (4 chars exactly) to be used.
// Builds the sprite rotation matrixes to account
//  for horizontally flipped sprites.
// Will report an error if the lumps are inconsistant. 
// Only called at startup.
//
// Sprite lump names are 4 characters for the actor,
//  a letter for the frame, and a number for the rotation.
// A sprite that is flippable will have an additional
//  letter/number appended.
// The rotation character can be 0 to signify no rotations.
//
// -------------------------------------------------------------------------

void R_InitSpriteDefs (char **namelist)
{
    char  **check;
    int     i;
    int     l;
    int     frame;
    int     rotation;
    int     start;
    int     end;
    int     patched;

    // count the number of sprite names
    check = namelist;

    while (*check != NULL)
    check++;

    numsprites = check-namelist;

    if (!numsprites)
    return;

    sprites = Z_Malloc(numsprites *sizeof(*sprites), PU_STATIC, NULL);

    start = firstspritelump-1;
    end = lastspritelump+1;

    // scan all the lump names for each of the names,
    //  noting the highest frame letter.
    // Just compare 4 characters as ints
    for (i=0 ; i<numsprites ; i++)
    {
        spritename = DEH_String(namelist[i]);
        memset (sprtemp,-1, sizeof(sprtemp));

        maxframe = -1;

        // scan the lumps,
        //  filling in the frames for whatever is found
        for (l=start+1 ; l<end ; l++)
        {
            if (!strncasecmp(lumpinfo[l]->name, spritename, 4))
            {
                frame = lumpinfo[l]->name[4] - 'A';
                rotation = lumpinfo[l]->name[5];

                if (modifiedgame)
                    patched = W_GetNumForName (lumpinfo[l]->name);
                else
                    patched = l;

                R_InstallSpriteLump (patched, frame, rotation, false);

                if (lumpinfo[l]->name[6])
                {
                    frame = lumpinfo[l]->name[6] - 'A';
                    rotation = lumpinfo[l]->name[7];
                    R_InstallSpriteLump (l, frame, rotation, true);
                }
            }
        }

    // check the frames that were found for completeness
    if (maxframe == -1)
    {
        sprites[i].numframes = 0;
        continue;
    }

    maxframe++;

    for (frame = 0 ; frame < maxframe ; frame++)
    {
        switch ((int)sprtemp[frame].rotate)
        {
            case -1:
            // no rotations were found for that frame at all
            // [crispy] make non-fatal
            fprintf (stderr, "R_InitSprites: No patches found for %s frame %c\n",
                             spritename, frame+'A');
            break;

            case 0:
            // only the first rotation is needed
            break;

            case 1:
            // must have all 8 frames
            for (rotation=0 ; rotation<8 ; rotation++)
                if (sprtemp[frame].lump[rotation] == -1)
                I_Error ("R_InitSprites: Sprite %s frame %c is missing rotations",
                         spritename, frame+'A');

            // [crispy] support 16 sprite rotations
            sprtemp[frame].rotate = 2;
            for ( ; rotation<16 ; rotation++)
                if (sprtemp[frame].lump[rotation] == -1)
                {
                    sprtemp[frame].rotate = 1;
                    break;
                }

            break;
        }
    }

    // allocate space for the frames present and copy sprtemp to it
    sprites[i].numframes = maxframe;
    sprites[i].spriteframes = Z_Malloc (maxframe * sizeof(spriteframe_t), PU_STATIC, NULL);
    memcpy (sprites[i].spriteframes, sprtemp, maxframe*sizeof(spriteframe_t));
    }
}


// -------------------------------------------------------------------------
// R_InitSprites
//
// Called at program start.
// -------------------------------------------------------------------------

void R_InitSprites (char **namelist)
{
    int i;

    for (i=0 ; i<SCREENWIDTH ; i++)
    {
        negonearray[i] = -1;
    }

    R_InitSpriteDefs (namelist);
}


// -------------------------------------------------------------------------
// R_ClearSprites
//
// Called at frame start.
// -------------------------------------------------------------------------

void R_ClearSprites (void)
{
    vissprite_p = vissprites;
}

// -------------------------------------------------------------------------
// R_NewVisSprite
// -------------------------------------------------------------------------

vissprite_t  overflowsprite;
vissprite_t *R_NewVisSprite (void)
{
    if (vissprite_p == &vissprites[MAXVISSPRITES])
    return &overflowsprite;

    vissprite_p++;
    return vissprite_p-1;
}


// -------------------------------------------------------------------------
// R_DrawMaskedColumn
//
// Used for sprites and masked mid textures.
// Masked means: partly transparent, i.e. stored
//  in posts/runs of opaque pixels.
// -------------------------------------------------------------------------

void R_DrawMaskedColumn (column_t *column)
{
    int64_t topscreen;     // [crispy] WiggleFix
    int64_t bottomscreen;  // [crispy] WiggleFix
    fixed_t basetexturemid;

    basetexturemid = dc_texturemid;
    dc_texheight = 0;

    for ( ; column->topdelta != 0xff ; ) 
    {
        // calculate unclipped screen coordinates for post
        topscreen = sprtopscreen + spryscale*column->topdelta;
        bottomscreen = topscreen + spryscale*column->length;

        dc_yl = (int)((topscreen+FRACUNIT-1)>>FRACBITS); // [crispy] WiggleFix
        dc_yh = (int)((bottomscreen-1)>>FRACBITS);       // [crispy] WiggleFix

        if (dc_yh >= mfloorclip[dc_x])
            dc_yh = mfloorclip[dc_x]-1;
        if (dc_yl <= mceilingclip[dc_x])
            dc_yl = mceilingclip[dc_x]+1;

        if (dc_yl <= dc_yh)
        {
            dc_source = (byte *)column + 3;
            dc_texturemid = basetexturemid - (column->topdelta<<FRACBITS);

            // Drawn by either R_DrawColumn
            //  or (SHADOW) R_DrawFuzzColumn.
            colfunc ();	
        }

        column = (column_t *)(  (byte *)column + column->length + 4);
    }

    dc_texturemid = basetexturemid;
}


// -------------------------------------------------------------------------
// R_DrawVisSprite
//
//  mfloorclip and mceilingclip should also be set.
// -------------------------------------------------------------------------

void R_DrawVisSprite (vissprite_t *vis, int x1, int x2)
{
    column_t  *column;
    int        texturecolumn;
    fixed_t    frac;
    patch_t   *patch;

    patch = W_CacheLumpNum (vis->patch+firstspritelump, PU_CACHE);

    dc_colormap = vis->colormap;

    if (vis->mobjflags & MF_TRANSLATION)
    {
        colfunc = transcolfunc;
        dc_translation = translationtables - 256 
                       + ((vis->mobjflags & MF_TRANSLATION) >> (MF_TRANSSHIFT-8));
    }
    else if (vis->translation)
    {
        colfunc = transcolfunc;
        dc_translation = vis->translation;
    }

    // [crispy] translucent sprites
    if (translucency && (vis->mobjflags & MF_TRANSLUCENT))
    {
        colfunc = tlcolfunc;
    }

    dc_iscale = abs(vis->xiscale)>>(detailshift && !hires);
    dc_texturemid = vis->texturemid;
    frac = vis->startfrac;
    spryscale = vis->scale;
    sprtopscreen = centeryfrac - FixedMul(dc_texturemid,spryscale);

    for (dc_x=vis->x1 ; dc_x<=vis->x2 ; dc_x++, frac += vis->xiscale)
    {
        texturecolumn = frac>>FRACBITS;
#ifdef RANGECHECK
        if (texturecolumn < 0 || texturecolumn >= SHORT(patch->width))
        I_Error ("R_DrawSpriteRange: bad texturecolumn");
#endif
        column = (column_t *) ((byte *)patch + LONG(patch->columnofs[texturecolumn]));
        R_DrawMaskedColumn (column);
    }

    colfunc = basecolfunc;
}


// -------------------------------------------------------------------------
// R_ProjectSprite
//
// Generates a vissprite for a thing
//  if it might be visible.
// -------------------------------------------------------------------------

void R_ProjectSprite (mobj_t *thing)
{
    int         x1, x2;
    int         lump;
    int         index;
    unsigned    rot;

    fixed_t     tr_x, tr_y;
    fixed_t     gxt, gyt;
    fixed_t     tx, tz;
    fixed_t     xscale;
    fixed_t     iscale;

    vissprite_t    *vis;
    spritedef_t    *sprdef;
    spriteframe_t  *sprframe;
    
    boolean     flip;
    angle_t     ang;    
    
    fixed_t     interpx;
    fixed_t     interpy;
    fixed_t     interpz;
    fixed_t     interpangle;

    // [AM] Interpolate between current and last position,
    //      if prudent.
    if (uncapped_fps &&
        // Don't interpolate if the mobj did something
        // that would necessitate turning it off for a tic.
        thing->interp == true &&
        // Don't interpolate during a paused state.
        !paused && (!menuactive || demoplayback || netgame))
    {
        interpx = thing->oldx + FixedMul(thing->x - thing->oldx, fractionaltic);
        interpy = thing->oldy + FixedMul(thing->y - thing->oldy, fractionaltic);
        interpz = thing->oldz + FixedMul(thing->z - thing->oldz, fractionaltic);
        interpangle = R_InterpolateAngle(thing->oldangle, thing->angle, fractionaltic);
    }
    else
    {
        interpx = thing->x;
        interpy = thing->y;
        interpz = thing->z;
        interpangle = thing->angle;
    }

    // transform the origin point
    tr_x = interpx - viewx;
    tr_y = interpy - viewy;

    gxt = FixedMul(tr_x,viewcos); 
    gyt = -FixedMul(tr_y,viewsin);

    tz = gxt-gyt; 

    // thing is behind view plane?
    if (tz < MINZ)
    return;

    xscale = FixedDiv(projection, tz);

    gxt = -FixedMul(tr_x,viewsin); 
    gyt = FixedMul(tr_y,viewcos); 
    tx = -(gyt+gxt); 

    // too far off the side?
    if (abs(tx)>(tz<<2))
    return;

    // decide which patch to use for sprite relative to player
#ifdef RANGECHECK
    if ((unsigned int) thing->sprite >= (unsigned int) numsprites)
    I_Error ("R_ProjectSprite: invalid sprite number %i", thing->sprite);
#endif
    sprdef = &sprites[thing->sprite];
#ifdef RANGECHECK
    if ( (thing->frame&FF_FRAMEMASK) >= sprdef->numframes )
    I_Error ("R_ProjectSprite: invalid sprite frame %i : %i ", 
             thing->sprite, thing->frame);
#endif
    sprframe = &sprdef->spriteframes[ thing->frame & FF_FRAMEMASK];

    if (sprframe->rotate)
    {
        // choose a different rotation based on player view
        ang = R_PointToAngle (interpx, interpy);
        // [crispy] support 16 sprite rotations
        if (sprframe->rotate == 2)
        {
            rot = (ang-interpangle+(unsigned)(ANG45/4)*17);
            rot = (rot>>29) + ((rot>>25)&8);
        }
        else
        {
            rot = (ang-interpangle+(unsigned)(ANG45/2)*9)>>29;
        }

        lump = sprframe->lump[rot];
        flip = (boolean)sprframe->flip[rot];
    }
    else
    {
        // use single rotation for all views
        lump = sprframe->lump[0];
        flip = (boolean)sprframe->flip[0];
    }

    // calculate edges of the shape
    tx -= spriteoffset[lump];	
    x1 = (centerxfrac + FixedMul (tx,xscale) ) >>FRACBITS;

    // off the right side?
    if (x1 > viewwidth)
    return;

    tx +=  spritewidth[lump];
    x2 = ((centerxfrac + FixedMul (tx,xscale) ) >>FRACBITS) - 1;

    // off the left side
    if (x2 < 0)
    return;

    // store information in a vissprite
    vis = R_NewVisSprite ();
    vis->translation = NULL;
    vis->mobjflags = thing->flags;
    vis->scale = xscale<<(detailshift && !hires);
    vis->gx = interpx;
    vis->gy = interpy;
    vis->gz = interpz;
    vis->gzt = interpz + spritetopoffset[lump];
    vis->texturemid = vis->gzt - viewz;
    vis->x1 = x1 < 0 ? 0 : x1;
    vis->x2 = x2 >= viewwidth ? viewwidth-1 : x2;	
    iscale = FixedDiv (FRACUNIT, xscale);

    // [crispy] flip death sprites and corpses randomly
    if ((thing->flags & MF_CORPSE &&
    thing->type != MT_BARREL) ||
    thing->info->spawnstate == S_PLAY_DIE7 ||
    thing->info->spawnstate == S_PLAY_XDIE9)
    {
        if (thing->health & 1)
        {
            flip = true;
        }
    }

    if (flip)
    {
        vis->startfrac = spritewidth[lump]-1;
        vis->xiscale = -iscale;
    }
    else
    {
        vis->startfrac = 0;
        vis->xiscale = iscale;
    }

    if (vis->x1 > x1)
    vis->startfrac += vis->xiscale*(vis->x1-x1);
    vis->patch = lump;

    // get light level
    if (thing->flags & MF_SHADOW)
    {
        // shadow draw
        vis->colormap = NULL;
    }
    else if (fixedcolormap)
    {
        // fixed map
        vis->colormap = fixedcolormap;
    }
    else if (thing->frame & FF_FULLBRIGHT)
    {
        // full bright
        vis->colormap = colormaps;
    }

    else
    {
        // diminished light
        index = xscale>>(LIGHTSCALESHIFT-detailshift+hires);

        if (index >= MAXLIGHTSCALE) 
        index = MAXLIGHTSCALE-1;

        vis->colormap = spritelights[index];

        // [Julia] Applying brightmaps to sprites...
        if (brightmaps)
        {
            // Armor Bonus
            if (thing->type == MT_MISC3)
            vis->colormap = fullbrights_dimmeditems[index];

            // Cell Charge
            else if (thing->type == MT_MISC20)
            vis->colormap = fullbrights_dimmeditems[index];

            // Cell Charge Pack
            else if (thing->type == MT_MISC21)
            vis->colormap = fullbrights_dimmeditems[index];

            // BFG9000
            else if (thing->type == MT_MISC25)
            vis->colormap = fullbrights_redonly[index];

            // Plasmagun
            else if (thing->type == MT_MISC28)
            vis->colormap = fullbrights_redonly[index];

            // Explosive Barrel
            else if (thing->type == MT_BARREL)
            vis->colormap = fullbrights_dimmeditems[index];
        }
    }	

    // [Julia] Jaguar: highlight everything
    if (hightlight_things)
    {
        vis->colormap = colormaps;
    }

    // [crispy] colored blood
    if (colored_blood &&
       (thing->type == MT_BLOOD || thing->state - states == S_GIBS) &&
       thing->target)
    {
        // [crispy] Barons of Hell bleed green blood
        if (thing->target->type == MT_BRUISER)
        {
            vis->translation = cr[CR_RED2GREEN];
        }
        else
        // [crispy] Cacodemons bleed blue blood
        if (thing->target->type == MT_HEAD)
        {
            vis->translation = cr[CR_RED2BLUE];
        }
    }
}


// -------------------------------------------------------------------------
// R_AddSprites
//
// During BSP traversal, this adds sprites by sector.
// -------------------------------------------------------------------------

void R_AddSprites (sector_t *sec)
{
    mobj_t  *thing;
    int      lightnum;

    // BSP is traversed by subsector.
    // A sector might have been split into several
    //  subsectors during BSP building.
    // Thus we check whether its already added.
    if (sec->validcount == validcount)
    return;		

    // Well, now it will be done.
    sec->validcount = validcount;

    lightnum = (sec->lightlevel >> LIGHTSEGSHIFT)+extralight;

    if (lightnum < 0)		
    {
        spritelights = scalelight[0];

        // [Julia] Calculating sprite brightmaps
        fullbrights_dimmeditems = fullbright_dimmeditems[0];
        fullbrights_redonly = fullbright_redonly[0];
    }
    else if (lightnum >= LIGHTLEVELS)
    {
        spritelights = scalelight[LIGHTLEVELS-1];

        // [Julia] Calculating sprite brightmaps
        fullbrights_dimmeditems = fullbright_dimmeditems[LIGHTLEVELS-1];
        fullbrights_redonly = fullbright_redonly[LIGHTLEVELS-1];
    }
    else
    {
        spritelights = scalelight[lightnum];

        // [Julia] Calculating sprite brightmaps
        fullbrights_dimmeditems = fullbright_dimmeditems[lightnum];
        fullbrights_redonly = fullbright_redonly[lightnum];
    }

    // Handle all things in sector.
    for (thing = sec->thinglist ; thing ; thing = thing->snext)
    R_ProjectSprite (thing);
}

// -------------------------------------------------------------------------
// [crispy] apply bobbing (or centering) to the player's weapon sprite
// -------------------------------------------------------------------------

static inline void R_ApplyWeaponBob (fixed_t *sx, boolean bobx, fixed_t *sy, boolean boby)
{
	const angle_t angle = (128 * leveltime) & FINEMASK;

	if (sx)
	{
		*sx = FRACUNIT;

		if (bobx)
		{
			 *sx += FixedMul(viewplayer->bob, finecosine[angle]);
		}
	}

	if (sy)
	{
		*sy = 32 * FRACUNIT; // [crispy] WEAPONTOP

		if (boby)
		{
			*sy += FixedMul(viewplayer->bob, finesine[angle & (FINEANGLES / 2 - 1)]);
		}
	}
}

// -------------------------------------------------------------------------
//
// [crispy] & [Julia] Halfed amplitube while firing in movement
//
// -------------------------------------------------------------------------

static inline void R_ApplyWeaponFiringBob (fixed_t *sx, boolean bobx, fixed_t *sy, boolean boby)
{
	const angle_t angle = (128 * leveltime) & FINEMASK;

	if (sx)
	{
		*sx = FRACUNIT;

		if (bobx)
		{
            if (chainsaw_attack_swing) // [Julia] Separate ampltitube for Chainsaw
            *sx += FixedMul(viewplayer->bob, finecosine[angle] / 16);

            else
			*sx += FixedMul(viewplayer->bob, finecosine[angle] / 2);
		}
	}

	if (sy)
	{
		*sy = 32 * FRACUNIT; // [crispy] WEAPONTOP

		if (boby)
		{
            if (chainsaw_attack_swing) // [Julia] Separate ampltitube for Chainsaw
            *sy += FixedMul(viewplayer->bob, finesine[angle*16 & (FINEANGLES / 2 - 1)] / 12);

            else
			*sy += FixedMul(viewplayer->bob, finesine[angle & (FINEANGLES / 2 - 1)] / 2);
		}
	}
}


// -------------------------------------------------------------------------
// R_DrawPSprite
// -------------------------------------------------------------------------

void R_DrawPSprite (pspdef_t *psp)
{
    int             x1, x2;
    int             lump;
    fixed_t         tx;
    fixed_t         psp_sx = psp->sx, psp_sy = psp->sy;
    spritedef_t    *sprdef;
    spriteframe_t  *sprframe;
    vissprite_t    *vis;
    vissprite_t     avis;
    boolean         flip;

    const int state = viewplayer->psprites[ps_weapon].state - states;

    // decide which patch to use
#ifdef RANGECHECK
    if ( (unsigned)psp->state->sprite >= (unsigned int) numsprites)
    I_Error ("R_ProjectSprite: invalid sprite number %i ",
             psp->state->sprite);
#endif
    sprdef = &sprites[psp->state->sprite];
#ifdef RANGECHECK
    if ( (psp->state->frame & FF_FRAMEMASK)  >= sprdef->numframes)
    I_Error ("R_ProjectSprite: invalid sprite frame %i : %i ",
             psp->state->sprite, psp->state->frame);
#endif
    sprframe = &sprdef->spriteframes[ psp->state->frame & FF_FRAMEMASK ];

    lump = sprframe->lump[0];
    flip = (boolean)sprframe->flip[0];

    // [crispy] Smoothen Chainsaw idle animation
    // [Julia] Also apply standard amplitude for some frames to get a smooth
    // transition between bobbing states.
    if (weapon_bobbing && (
    /* Chainsaw  */ state == S_SAW     || state == S_SAWB  ||
    /* Shotgun   */ state == S_SGUN8   || state == S_SGUN9 ||
    /* Plasmagun */ state == S_PLASMA2 ||
    /* BFG9000   */ state == S_BFG3    || state == S_BFG4))
    {
        R_ApplyWeaponBob(&psp_sx, true, &psp_sy, true);
    }

    // [Julia] Halfed amplitube while firing in movement 
    if (weapon_bobbing && (
    /* Fist      */ state == S_PUNCH1   || state == S_PUNCH2   || state == S_PUNCH3   || state == S_PUNCH4  || state == S_PUNCH5 ||
    /* Chainsaw  */ state == S_SAW1     || state == S_SAW2     ||
    /* Pistol    */ state == S_PISTOL1  || state == S_PISTOL2  || state == S_PISTOL3  || state == S_PISTOL4 ||
    /* Shotgun   */ state == S_SGUN1    || state == S_SGUN2    || state == S_SGUN3    || state == S_SGUN4   || state == S_SGUN5  || state == S_SGUN6 || state == S_SGUN7 ||
    /* Chaingun  */ state == S_CHAIN1   || state == S_CHAIN2   || state == S_CHAIN3   ||
    /* RLauncher */ state == S_MISSILE1 || state == S_MISSILE2 || state == S_MISSILE3 ||
    /* Plasmagun */ state == S_PLASMA1  ||
    /* BFG9000   */ state == S_BFG1     || state == S_BFG2 ))
    {
        R_ApplyWeaponFiringBob(&psp_sx, true, &psp_sy, true);
    }

    // [crispy] squat down weapon sprite a bit after hitting the ground
    if (weapon_bobbing)
    psp_sy += abs(viewplayer->psp_dy);

    // calculate edges of the shape
    tx = psp_sx-(ORIGWIDTH/2)*FRACUNIT;

    tx -= spriteoffset[lump];	
    x1 = (centerxfrac + FixedMul (tx,pspritescale) ) >>FRACBITS;

    // off the right side
    if (x1 > viewwidth)
    return;		

    tx +=  spritewidth[lump];
    x2 = ((centerxfrac + FixedMul (tx, pspritescale) ) >>FRACBITS) - 1;

    // off the left side
    if (x2 < 0)
    return;

    // store information in a vissprite
    vis = &avis;
    vis->translation = NULL;
    vis->mobjflags = 0;
    // [crispy] weapons drawn 1 pixel too high when player is idle
    // [Julia] Jaguar weapon placement -
    // Move weapon 10 px higher above STBAR, in not in full screen mode.
    if (screenblocks <= 10)
    {
        vis->texturemid = (BASEYCENTER<<FRACBITS) + FRACUNIT/4 + FRACUNIT*10 
                        - (psp_sy-spritetopoffset[lump]);
    }
    else
    {
        vis->texturemid = (BASEYCENTER<<FRACBITS) + FRACUNIT/4
                        - (psp_sy-spritetopoffset[lump]);
    }
    vis->x1 = x1 < 0 ? 0 : x1;
    vis->x2 = x2 >= viewwidth ? viewwidth-1 : x2;	
    vis->scale = pspritescale<<(detailshift && !hires);

    if (flip)
    {
        vis->xiscale = -pspriteiscale;
        vis->startfrac = spritewidth[lump]-1;
    }
    else
    {
        vis->xiscale = pspriteiscale;
        vis->startfrac = 0;
    }

    // [Julia] Mouselook: also move HUD weapons while mouse look
    vis->texturemid += FixedMul(((centery - viewheight / 2) << FRACBITS), vis->xiscale);

    if (vis->x1 > x1)
    vis->startfrac += vis->xiscale*(vis->x1-x1);

    vis->patch = lump;

    if (fixedcolormap)
    {
        // fixed color
        vis->colormap = fixedcolormap;
    }
    else if (psp->state->frame & FF_FULLBRIGHT)
    {
        // full bright
        vis->colormap = colormaps;
    }
    else
    {
        // local light
        vis->colormap = spritelights[MAXLIGHTSCALE-1];
    }
	
    R_DrawVisSprite (vis, vis->x1, vis->x2);
}


// -------------------------------------------------------------------------
// R_DrawPlayerSprites
// -------------------------------------------------------------------------

void R_DrawPlayerSprites (void)
{
    int        i;
    int        lightnum;
    const int  state = viewplayer->psprites[ps_weapon].state - states;
    pspdef_t  *psp;    

    // get light level
    lightnum = (viewplayer->mo->subsector->sector->lightlevel >> LIGHTSEGSHIFT) + extralight;

    if (lightnum < 0)		
    {
        spritelights = scalelight[0];
    }
    else if (lightnum >= LIGHTLEVELS)
    {
        spritelights = scalelight[LIGHTLEVELS-1];
    }
    else
    {
        // [Julia] Standard formula first
        spritelights = scalelight[lightnum];

        // [Julia] Applying brightmaps to HUD weapons...
        if (brightmaps)
        {
            // BFG9000
            if (state == S_BFG1 || state == S_BFG2 || state == S_BFG3 || state == S_BFG4)
            spritelights = fullbright_redonly[lightnum];
        }
    }

    // clip to screen bounds
    mfloorclip = screenheightarray;
    mceilingclip = negonearray;

    // add all active psprites
    for (i=0, psp=viewplayer->psprites; i<NUMPSPRITES; i++,psp++)
    {
        if (psp->state)
        R_DrawPSprite (psp);
    }
}


// -------------------------------------------------------------------------
// R_SortVisSprites
// -------------------------------------------------------------------------

void R_SortVisSprites (void)
{
    int           i;
    int           count;
    vissprite_t  *ds;
    vissprite_t  *best;
    vissprite_t   unsorted;
    fixed_t       bestscale;

    count         = vissprite_p - vissprites;
    unsorted.next = unsorted.prev = &unsorted;

    if (!count)
    return;

    for (ds=vissprites ; ds<vissprite_p ; ds++)
    {
        ds->next = ds+1;
        ds->prev = ds-1;
    }

    vissprites[0].prev = &unsorted;
    unsorted.next = &vissprites[0];
    (vissprite_p-1)->next = &unsorted;
    unsorted.prev = vissprite_p-1;

    // pull the vissprites out by scale

    vsprsortedhead.next = vsprsortedhead.prev = &vsprsortedhead;
    for (i=0 ; i<count ; i++)
    {
        bestscale = INT_MAX;
        best = unsorted.next;

        for (ds=unsorted.next ; ds != &unsorted ; ds=ds->next)
        {
            if (ds->scale < bestscale)
            {
            bestscale = ds->scale;
            best = ds;
            }
        }

        best->next->prev = best->prev;
        best->prev->next = best->next;
        best->next = &vsprsortedhead;
        best->prev = vsprsortedhead.prev;
        vsprsortedhead.prev->next = best;
        vsprsortedhead.prev = best;
    }
}


// -------------------------------------------------------------------------
// R_DrawSprite
// -------------------------------------------------------------------------

void R_DrawSprite (vissprite_t *spr)
{
    int         clipbot[SCREENWIDTH];
    int         cliptop[SCREENWIDTH];
    int         x;
    int         r1;
    int         r2;
    fixed_t     scale;
    fixed_t     lowscale;
    drawseg_t  *ds;

    for (x = spr->x1 ; x<=spr->x2 ; x++)
    clipbot[x] = cliptop[x] = -2;

    // Scan drawsegs from end to start for obscuring segs.
    // The first drawseg that has a greater scale
    //  is the clip seg.

    // Modified by Lee Killough:
    // (pointer check was originally nonportable
    // and buggy, by going past LEFT end of array):

    // for (ds=ds_p-1 ; ds >= drawsegs ; ds--)    old buggy code
    for (ds=ds_p ; ds-- > drawsegs ; )  // new -- killough
    {   
        // determine if the drawseg obscures the sprite
        if (ds->x1 > spr->x2 || ds->x2 < spr->x1 || (!ds->silhouette && !ds->maskedtexturecol))
        continue;   // does not cover sprite

        r1 = ds->x1 < spr->x1 ? spr->x1 : ds->x1;
        r2 = ds->x2 > spr->x2 ? spr->x2 : ds->x2;

        if (ds->scale1 > ds->scale2)
        {
            lowscale = ds->scale2;
            scale = ds->scale1;
        }
        else
        {
            lowscale = ds->scale1;
            scale = ds->scale2;
        }

        if (scale < spr->scale || (lowscale < spr->scale 
        && !R_PointOnSegSide (spr->gx, spr->gy, ds->curline)))
        {
            if (ds->maskedtexturecol)   // masked mid texture?
            R_RenderMaskedSegRange(ds, r1, r2);

            continue;                   // seg is behind sprite
        }

        // clip this piece of the sprite
        // killough 3/27/98: optimized and made much shorter

        if (ds->silhouette&SIL_BOTTOM && spr->gz < ds->bsilheight)  // bottom sil
            for (x=r1 ; x<=r2 ; x++)
                if (clipbot[x] == -2)
                    clipbot[x] = ds->sprbottomclip[x];

        if (ds->silhouette&SIL_TOP && spr->gzt > ds->tsilheight)    // top sil
            for (x=r1 ; x<=r2 ; x++)
                if (cliptop[x] == -2)
                    cliptop[x] = ds->sprtopclip[x];
    }

    // all clipping has been performed, so draw the sprite

    // check for unclipped columns
    for (x = spr->x1 ; x<=spr->x2 ; x++)
    {
        if (clipbot[x] == -2)		
            clipbot[x] = viewheight;

        if (cliptop[x] == -2)
            cliptop[x] = -1;
    }

    mfloorclip = clipbot;
    mceilingclip = cliptop;
    R_DrawVisSprite (spr, spr->x1, spr->x2);
}


// -------------------------------------------------------------------------
// R_DrawMasked
// -------------------------------------------------------------------------

void R_DrawMasked (void)
{
    vissprite_t  *spr;
    drawseg_t    *ds;

    R_SortVisSprites();

    if (vissprite_p > vissprites)
    {
        // draw all vissprites back to front
        for (spr = vsprsortedhead.next ; 
            spr != &vsprsortedhead ;
            spr=spr->next)
        {
            R_DrawSprite (spr);
        }
    }

    // render any remaining masked mid textures

    // Modified by Lee Killough:
    // (pointer check was originally nonportable
    // and buggy, by going past LEFT end of array):

    // for (ds=ds_p-1 ; ds >= drawsegs ; ds--)    old buggy code
    for (ds=ds_p ; ds-- > drawsegs ; )  // new -- killough
        if (ds->maskedtexturecol)
            R_RenderMaskedSegRange (ds, ds->x1, ds->x2);

    // draw the psprites on top of everything
    //  but does not draw on side views
    if (!viewangleoffset)		
    R_DrawPlayerSprites ();
}
