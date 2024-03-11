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
//	Game completion, final screen animation.
//


#include <stdio.h>
#include <ctype.h>

// Functions.
#include "ct_chat.h"
#include "i_system.h"
#include "i_swap.h"
#include "z_zone.h"
#include "v_video.h"
#include "w_wad.h"
#include "s_sound.h"
#include "m_menu.h"

// Data.
#include "d_main.h"
#include "dstrings.h"
#include "sounds.h"
#include "doomstat.h"
#include "r_local.h"
#include "m_misc.h" // [crispy] M_StringDuplicate()

#include "id_func.h"


#define	TEXTSPEED	4
#define	TEXTWAIT	250
#define	TEXTEND		25

#define JAGENDING               \
	"     id software\n"        \
	"     salutes you!\n"       \
	"\n"                        \
	"  the horrors of hell\n"   \
	"  could not kill you.\n"   \
	"  their most cunning\n"    \
	"  traps were no match\n"   \
	"  for you. you have\n"     \
	"  proven yourself the\n"   \
	"  best of all!\n"          \
	"\n"                        \
	"  congratulations!"


typedef enum
{
	F_STAGE_TEXT,
	F_STAGE_ARTSCREEN,
	F_STAGE_CAST,
} finalestage_t;

// Stage of animation:
finalestage_t finalestage;

unsigned int finalecount;
unsigned int finaleendcount;

// [JN] Do screen wipe only once after text skipping.
static boolean finale_wipe_done;

typedef struct
{
    GameMission_t mission;
    int episode, level;
    const char *background;
    const char *text;
} textscreen_t;

static textscreen_t textscreens[] =
{
	{ doom2, 1, 23, "ROCKS", JAGENDING},
};

const char *finaletext;
const char *finaleflat;
static char *finaletext_rw;

void	F_StartCast (void);
void	F_CastTicker (void);
boolean F_CastResponder (event_t *ev);
void	F_CastDrawer (void);



// -----------------------------------------------------------------------------
// F_StartFinale
// -----------------------------------------------------------------------------

void F_StartFinale (void)
{
    size_t i;

    gameaction = ga_nothing;
    gamestate = GS_FINALE;
    automapactive = false;
    finale_wipe_done = false;
    players[consoleplayer].cheatTics = 1;
    players[consoleplayer].messageTics = 1;
    players[consoleplayer].message = NULL;
    players[consoleplayer].messageCenteredTics = 1;
    players[consoleplayer].messageCentered = NULL;

    S_ChangeMusic(mus_ending, true);

    // Find the right screen and set the text and background

    for (i = 0 ; i < arrlen(textscreens) ; ++i)
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

    // [JN] Count intermission/finale text lenght. Once it's fully printed, 
    // no extra "attack/use" button pressing is needed for skipping.
    finaleendcount = strlen(finaletext) * TEXTSPEED + TEXTEND;
    // [crispy] do the "char* vs. const char*" dance
    if (finaletext_rw)
    {
	free(finaletext_rw);
	finaletext_rw = NULL;
    }
    finaletext_rw = M_StringDuplicate(finaletext);
    
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

    //
    // [JN] If we are in single player mode, allow double skipping for 
    // intermission text. First skip printing all intermission text,
    // second is advancing to the next state.
    //
    if (singleplayer)
    {
        // [JN] Make PAUSE working properly on text screen
        if (paused)
        {
            return;
        }

        // [JN] Check for skipping. Allow double-press skiping, 
        // but don't skip immediately.
        if (finalecount > 10)
        {
            // go on to the next level
            for (i = 0 ; i < MAXPLAYERS ; i++)
            {
                // [JN] Don't allow to skip bunny screen,
                // and don't allow to skip by pressing "pause" button.
                if ((gameepisode == 3 && finalestage == F_STAGE_ARTSCREEN)
                || players[i].cmd.buttons == (BT_SPECIAL | BTS_PAUSE))
                continue;

                // [JN] Double-skip by pressing "attack" button.
                if (players[i].cmd.buttons & BT_ATTACK && !menuactive)
                {
                    if (!players[i].attackdown)
                    {
                        if (finalecount >= finaleendcount)
                        break;
    
                        finalecount += finaleendcount;
                        players[i].attackdown = true;
                    }
                    players[i].attackdown = true;
                }
                else
                {
                    players[i].attackdown = false;
                }
    
                // [JN] Double-skip by pressing "use" button.
                if (players[i].cmd.buttons & BT_USE && !menuactive)
                {
                    if (!players[i].usedown)
                    {
                        if (finalecount >= finaleendcount)
                        break;
    
                        finalecount += finaleendcount;
                        players[i].usedown = true;
                    }
                    players[i].usedown = true;
                }
                else
                {
                    players[i].usedown = false;
                }
            }

            if (i < MAXPLAYERS)
            {
                if (gamemode != commercial)
                {
                    
                    finalestage = F_STAGE_ARTSCREEN;
                    
                    if (!finale_wipe_done)
                    {
                        finale_wipe_done = true;
                        wipegamestate = -1; // force a wipe
                    }

                    return;
                }
    
                if (gamemap == 23)  // [JN] Jaguar: final level
                {
                    F_StartCast ();
                }
                else
                {
                    gameaction = ga_worlddone;
                }
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
    //
    // [JN] Standard Doom routine, safe for network game and demos.
    //        
    else
    {
    // check for skipping
    if ( (gamemode == commercial)
      && ( finalecount > 50) )
    {
      // go on to the next level
      for (i=0 ; i<MAXPLAYERS ; i++)
	if (players[i].cmd.buttons)
	  break;
				
      if (i < MAXPLAYERS)
      {	
	if (gamemap == 30)
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
	
    if ( gamemode == commercial)
	return;
		
    if (finalestage == F_STAGE_TEXT
     && finalecount>strlen (finaletext)*TEXTSPEED + TEXTWAIT)
    {
	finalecount = 0;
	finalestage = F_STAGE_ARTSCREEN;
	wipegamestate = -1;		// force a wipe
    }
    }
}

// -----------------------------------------------------------------------------
// F_TextWrite
// -----------------------------------------------------------------------------

void F_TextWrite (void)
{
    byte*	src;
    pixel_t*	dest;

    int		w;
    signed int	count;
    char *ch; // [crispy] un-const
    int		c;
    int		cx;
    int		cy;
    
    // erase the entire screen to a tiled background
    src = W_CacheLumpName ( finaleflat , PU_CACHE);
    dest = I_VideoBuffer;

    // [crispy] use unified flat filling function
    V_FillFlat(0, SCREENHEIGHT, 0, SCREENWIDTH, src, dest);
	
    V_MarkRect (0, 0, SCREENWIDTH, SCREENHEIGHT);

	// [JN] Draw special background
	if (gamemap == 23)
	{
		V_DrawPatch(0, 0, W_CacheLumpName ("ENDPIC1", PU_CACHE));
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
	if (c < 0 || c > HU_FONTSIZE_B)
	{
	    cx += 7;
	    continue;
	}
		
	w = SHORT (hu_font_b[c]->width);

	if (cx+w > ORIGWIDTH)
	break;

	V_DrawShadowedPatchOptional(cx, cy, 0, hu_font_b[c]);
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
    const char	*name;
    mobjtype_t	type;
} castinfo_t;

castinfo_t	castorder[] = {
	{"zombieman",		MT_POSSESSED},
	{"shotgun guy",		MT_SHOTGUY},
	{"imp",				MT_TROOP},
	{"demon",			MT_SERGEANT},
	{"lost soul",       MT_SKULL},
	{"cacodemon",       MT_HEAD},
	{"baron of hell",	MT_BRUISER},
	{"our hero",		MT_PLAYER},
	{NULL,          	0}
};

int		castnum;
int		casttics;
state_t*	caststate;
boolean		castdeath;
int		castframes;
int		castonmelee;
boolean		castattacking;

// -----------------------------------------------------------------------------
// F_StartCast
// -----------------------------------------------------------------------------

void F_StartCast (void)
{
    wipegamestate = -1;		// force a screen wipe
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


//
// F_CastTicker
//
void F_CastTicker (void)
{
    int		st;
    int		sfx;
	
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
		caststate=
		    &states[mobjinfo[castorder[castnum].type].meleestate];
	    else
		caststate=
		    &states[mobjinfo[castorder[castnum].type].missilestate];
	}
    }
	
    if (castattacking)
    {
	if (castframes == 24
	    ||	caststate == &states[mobjinfo[castorder[castnum].type].seestate] )
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


//
// F_CastResponder
//

boolean F_CastResponder (event_t* ev)
{
    if (ev->type != ev_keydown)
	return false;
		
    if (castdeath)
	return true;			// already in dying frames
		
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


//
// F_CastDrawer
//

void F_CastDrawer (void)
{
    spritedef_t*	sprdef;
    spriteframe_t*	sprframe;
    int			lump;
    boolean		flip;
    patch_t*		patch;
    
    // erase the entire screen to a background
    V_DrawPatchFullScreen(W_CacheLumpName("ENDPIC2", PU_CACHE), false);

    // [JN] Simplify to use common text drawing function.
    M_WriteTextBigCentered(15, castorder[castnum].name, NULL);
    
    // draw the current frame in the middle of the screen
    sprdef = &sprites[caststate->sprite];
    sprframe = &sprdef->spriteframes[ caststate->frame & FF_FRAMEMASK];
    lump = sprframe->lump[0];
    flip = (boolean)sprframe->flip[0];
			
    patch = W_CacheLumpNum (lump+firstspritelump, PU_CACHE);
	
	V_DrawPatchFinale(ORIGWIDTH/4, 90, patch);
}


//
// F_DrawPatchCol
//
static fixed_t dxi, dy, dyi;

void
F_DrawPatchCol
( int		x,
  patch_t*	patch,
  int		col )
{
    column_t*	column;
    byte*	source;
    pixel_t*	dest;
    pixel_t*	desttop;
    int		count;
	
    column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));
    desttop = I_VideoBuffer + x;

    // step through the posts in a column
    while (column->topdelta != 0xff )
    {
	int srccol = 0;
	source = (byte *)column + 3;
	dest = desttop + ((column->topdelta * dy) >> FRACBITS)*SCREENWIDTH;
	count = (column->length * dy) >> FRACBITS;
		
	while (count--)
	{
	    *dest = pal_color[source[srccol >> FRACBITS]];
	    srccol += dyi;
	    dest += SCREENWIDTH;
	}
	column = (column_t *)(  (byte *)column + column->length + 4 );
    }
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
