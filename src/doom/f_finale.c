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
#include <ctype.h>

#include "deh_main.h"
#include "i_system.h"
#include "i_swap.h"
#include "z_zone.h"
#include "v_video.h"
#include "w_wad.h"
#include "s_sound.h"
#include "d_main.h"
#include "sounds.h"
#include "doomstat.h"
#include "r_state.h"
#include "d_englsh.h"
#include "jn.h"


typedef enum
{
    F_STAGE_TEXT,
    F_STAGE_ARTSCREEN,
    F_STAGE_CAST,
} finalestage_t;


// Stage of animation:
finalestage_t finalestage;

unsigned int finalecount;

#define TEXTSPEED   4   // [Julia] Initially 3. Higher values are slower.
#define TEXTWAIT    250

typedef struct
{
    GameMission_t mission;
    int episode, level;
    char *background;
    char *text;
} textscreen_t;


static textscreen_t textscreens[] =
{
    { jaguar, 1, 23, "BLACK", JAGENDING},
};


char   *finaletext;
char   *finaleflat;

void    F_StartCast (void);
void    F_CastTicker (void);
boolean F_CastResponder (event_t *ev);
void    F_CastDrawer (void);


// -----------------------------------------------------------------------------
// F_StartFinale
// -----------------------------------------------------------------------------

void F_StartFinale (void)
{
    size_t i;

    gameaction = ga_nothing;
    gamestate = GS_FINALE;
    viewactive = false;
    automapactive = false;

    if (player_is_cheater)
    S_StopMusic();  // [Julia] Shut down music in cheated ending
    else
    S_ChangeMusic(mus_ending, true);

    // Find the right screen and set the text and background

    for (i=0; i<arrlen(textscreens); ++i)
    {
        textscreen_t *screen = &textscreens[i];

        if (logical_gamemission == screen->mission
        && (logical_gamemission != doom || gameepisode == screen->episode)
        && gamemap == screen->level)
        {
            finaletext = screen->text;
            finaleflat = screen->background;
        }
    }

    // Do dehacked substitutions of strings

    finaletext = DEH_String(finaletext);
    finaleflat = DEH_String(finaleflat);

    finalestage = F_STAGE_TEXT;
    finalecount = 0;
}


// -----------------------------------------------------------------------------
// F_Responder
// -----------------------------------------------------------------------------

boolean F_Responder (event_t *event)
{
    if (finalestage == F_STAGE_CAST)
    return F_CastResponder (event);

    return false;
}


// -----------------------------------------------------------------------------
// F_Ticker
// -----------------------------------------------------------------------------

void F_Ticker (void)
{
    size_t i;

    // [Julia] Make PAUSE working properly on text screen
    if (paused)
    return;
    
    // [Julia] Can't skip cheated ending!
    if (player_is_cheater)
    return;

    // check for skipping
    // [Julia] Don't allow skipping until whole text is written on the screen.
    if (finalecount > 810)
    {
        // go on to the next level
        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            // [Julia] Pressing PAUSE should not skip text screen
            if (players[i].cmd.buttons && !(players->cmd.buttons & BTS_PAUSE))
            break;
        }

        if (i < MAXPLAYERS)
        {
            if (gamemap == 23)  // [JN] Jaguar: final level
            F_StartCast ();
            else
            gameaction = ga_worlddone;
        }
    }

    // advance animation
    finalecount++;

    if (finalestage == F_STAGE_CAST)
    {
        F_CastTicker ();
        return;
    }
}


// -----------------------------------------------------------------------------
// F_TextWrite
// -----------------------------------------------------------------------------

#include "hu_stuff.h"
extern patch_t *hu_font[HU_FONTSIZE];
extern patch_t *hu_font2[HU_FONTSIZE2];


void F_TextWrite (void)
{
    byte      *src;
    byte      *dest;
    char      *ch;
    int        x,y,w;
    int        c, cx, cy;
    signed int count;

    // erase the entire screen to a tiled background
    src = W_CacheLumpName ( finaleflat , PU_CACHE);
    dest = I_VideoBuffer;

    for (y=0 ; y<SCREENHEIGHT ; y++)
    {
        for (x=0 ; x<SCREENWIDTH/64 ; x++)
        {
            memcpy (dest, src+((y&63)<<6), 64);
            dest += 64;
        }
        if (SCREENWIDTH&63)
        {
            memcpy (dest, src+((y&63)<<6), SCREENWIDTH&63);
            dest += (SCREENWIDTH&63);
        }
    }

    V_MarkRect (0, 0, SCREENWIDTH, SCREENHEIGHT);

    // [Julia] Draw special background
    if (gamemap == 23)
    {
        if (player_is_cheater)
        {
            // [Julia] Cheated ending! Console code is:
            /*
            ** CLS
            ** @ECHO OFF
            ** ECHO Runtime error 666 at 0000:029A 
            ** ECHO - user cheated
            ** ECHO.
            */

            V_DrawPatchUnscaled (1, 2, W_CacheLumpName (DEH_String("C_DOOM"), PU_CACHE));
            if (gametic & 4)
            V_DrawPatchUnscaled (65, 60, W_CacheLumpName (DEH_String("C_CURSOR"), PU_CACHE));

            // Don't go any farther
            return;
        }
        else
        {
            // [Julia] Normal ending
            V_DrawPatch (0, 0, W_CacheLumpName (DEH_String("ENDPIC1"), PU_CACHE));
        }
    }

    // draw some of the text onto the screen
    cx = 10;
    cy = 10;
    ch = finaletext;

    count = ((signed int) finalecount - 10) / TEXTSPEED;

    if (count < 0)
	count = 0;

    for ( ; count ; count-- )
    {
        c = *ch++;

        if (!c)
        break;

        if (c == '\n')
        {
            cx = 10;
            cy += 14;
            continue;
        }

        c = c - HU_FONTSTART2;
        if (c < 0 || c> HU_FONTSIZE2)
        {
            cx += 7;
            continue;
        }

        w = SHORT (hu_font2[c]->width);

        if (cx+w > ORIGWIDTH)
	    break;

        V_DrawShadowedPatch(cx, cy, hu_font2[c]);

        cx+=w;
    }
}


// =============================================================================
// Final DOOM 2 animation
// Casting by id Software.
//   in order of appearance
// =============================================================================


typedef struct
{
    char        *name;
    mobjtype_t  type;
} castinfo_t;


castinfo_t	castorder[] = {
    {CC_ZOMBIE,     MT_POSSESSED},
    {CC_SHOTGUN,    MT_SHOTGUY},
    {CC_IMP,        MT_TROOP},
    {CC_DEMON,      MT_SERGEANT},
    {CC_LOST,       MT_SKULL},
    {CC_CACO,       MT_HEAD},
    {CC_BARON,      MT_BRUISER},
    {CC_HERO,       MT_PLAYER},
    {NULL,          0}
};


int         castnum;
int         casttics;
state_t    *caststate;
boolean     castdeath;
int         castframes;
int         castonmelee;
boolean     castattacking;


// -----------------------------------------------------------------------------
// F_StartCast
// -----------------------------------------------------------------------------

void F_StartCast (void)
{
    wipegamestate = -1; // force a screen wipe
    castnum = 0;
    caststate = &states[mobjinfo[castorder[castnum].type].seestate];
    casttics = caststate->tics;
    castdeath = false;
    finalestage = F_STAGE_CAST;
    castframes = 0;
    castonmelee = 0;
    castattacking = false;
    S_ChangeMusic(mus_extra, true);
}


// -----------------------------------------------------------------------------
// F_CastTicker
// -----------------------------------------------------------------------------

void F_CastTicker (void)
{
    int st;
    int sfx;

    if (--casttics > 0)
    return;			// not time to change state yet

    if (caststate->tics == -1 || caststate->nextstate == S_NULL)
    {
        // switch from deathstate to next monster
        castnum++;
        castdeath = false;

        if (castorder[castnum].name == NULL)
        castnum = 0;

        if (mobjinfo[castorder[castnum].type].seesound)
        S_StartSound (NULL, mobjinfo[castorder[castnum].type].seesound);

        caststate = &states[mobjinfo[castorder[castnum].type].seestate];
        castframes = 0;
    }
    else
    {
        // just advance to next state in animation
        if (caststate == &states[S_PLAY_ATK1])
        goto stopattack;	// Oh, gross hack!

        st = caststate->nextstate;
        caststate = &states[st];
        castframes++;

        // sound hacks....
        switch (st)
        {
            case S_PLAY_ATK1:	sfx = sfx_pistol; break;
            case S_POSS_ATK2:	sfx = sfx_pistol; break;
            case S_SPOS_ATK2:	sfx = sfx_shotgn; break;
            case S_TROO_ATK3:	sfx = sfx_claw;   break;
            case S_SARG_ATK2:	sfx = sfx_sgtatk; break;
            case S_BOSS_ATK2:
            case S_HEAD_ATK2:	sfx = sfx_firsht; break;
            case S_SKULL_ATK2:	sfx = sfx_sklatk; break;
            default: sfx = 0; break;
        }
		
        if (sfx)
        S_StartSound (NULL, sfx);
    }
	
    if (castframes == 12)
    {
        // go into attack frame
        castattacking = true;

        if (castonmelee)
        caststate=&states[mobjinfo[castorder[castnum].type].meleestate];
        else
        caststate=&states[mobjinfo[castorder[castnum].type].missilestate];

        castonmelee ^= 1;
        if (caststate == &states[S_NULL])
        {
            if (castonmelee)
            caststate = &states[mobjinfo[castorder[castnum].type].meleestate];
            else
            caststate = &states[mobjinfo[castorder[castnum].type].missilestate];
        }
    }

    if (castattacking)
    {
        if (castframes == 24 ||	caststate == &states[mobjinfo[castorder[castnum].type].seestate] )
        {
            stopattack:
            castattacking = false;
            castframes = 0;
            caststate = &states[mobjinfo[castorder[castnum].type].seestate];
        }
    }

    casttics = caststate->tics;

    if (casttics == -1)
    casttics = 15;
}


// -----------------------------------------------------------------------------
// F_CastResponder
// -----------------------------------------------------------------------------

boolean F_CastResponder (event_t *ev)
{
    if (ev->type != ev_keydown)
    return false;

    if (castdeath)
	return true;    // already in dying frames

    // go into death frame
    castdeath = true;
    caststate = &states[mobjinfo[castorder[castnum].type].deathstate];
    casttics = caststate->tics;
    castframes = 0;
    castattacking = false;

    if (mobjinfo[castorder[castnum].type].deathsound)
    S_StartSound (NULL, mobjinfo[castorder[castnum].type].deathsound);

    return true;
}


// -----------------------------------------------------------------------------
// F_CastPrint
// -----------------------------------------------------------------------------

void F_CastPrint (char *text)
{
    char  *ch;
    int	   c;
    int	   cx;
    int	   w;
    int	   width;

    // find width
    ch = text;
    width = 0;

    while (ch)
    {
        c = *ch++;

        if (!c)
        break;

        c = c - HU_FONTSTART2;

        if (c < 0 || c> HU_FONTSIZE2)
        {
            width += 4;
            continue;
        }

        w = SHORT (hu_font2[c]->width);
        width += w;
    }

    // draw it
    cx = ORIGWIDTH/2-width/2;
    ch = text;
    while (ch)
    {
        c = *ch++;

        if (!c)
        break;

        c = c - HU_FONTSTART2;

        if (c < 0 || c> HU_FONTSIZE2)
        {
            cx += 4;
            continue;
        }

        w = SHORT (hu_font2[c]->width);

        // [Julia] Jaguar: print casting name on top of the screen
        V_DrawShadowedPatch(cx, 15, hu_font2[c]);

        cx+=w;
    }
}


// -----------------------------------------------------------------------------
// F_CastDrawer
// -----------------------------------------------------------------------------

void F_CastDrawer (void)
{
    spritedef_t   *sprdef;
    spriteframe_t *sprframe;
    patch_t       *patch;
    int            lump;

    // [Julia] Draw special background
    V_DrawPatch (0, 0, W_CacheLumpName (DEH_String("ENDPIC2"), PU_CACHE));

    F_CastPrint (DEH_String(castorder[castnum].name));

    // draw the current frame in the middle of the screen
    sprdef = &sprites[caststate->sprite];
    sprframe = &sprdef->spriteframes[ caststate->frame & FF_FRAMEMASK];
    lump = sprframe->lump[0];

    patch = W_CacheLumpNum (lump+firstspritelump, PU_CACHE);

    V_DrawPatchFinale(ORIGWIDTH/4, 90, patch);
}


// -----------------------------------------------------------------------------
// F_Drawer
// -----------------------------------------------------------------------------

void F_Drawer (void)
{
    switch (finalestage)
    {
        case F_STAGE_CAST:
        F_CastDrawer();
        break;

        case F_STAGE_TEXT:
        F_TextWrite();
        break;

        case F_STAGE_ARTSCREEN:
        break;
    }
}

