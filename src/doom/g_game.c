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
// DESCRIPTION:  none
//



#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>   // [JN] Local time.

#include "doomdef.h" 
#include "doomkeys.h"
#include "doomstat.h"

#include "z_zone.h"
#include "f_finale.h"
#include "m_argv.h"
#include "m_controls.h"
#include "m_misc.h"
#include "m_menu.h"
#include "m_random.h"
#include "i_joystick.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_input.h"
#include "i_swap.h"
#include "i_video.h"

#include "d_main.h"

#include "wi_stuff.h"
#include "st_bar.h"
#include "am_map.h"

// Needs access to LFB.
#include "v_video.h"

#include "w_wad.h"

#include "p_local.h" 

#include "s_sound.h"

// Data.
#include "d_englsh.h"
#include "sounds.h"
#include "ct_chat.h"

// SKY handling - still the wrong place.

#include "g_game.h"

#include "id_vars.h"
#include "id_func.h"



#include "memio.h"

#define SAVEGAMESIZE	0x2c000

 
// Gamestate the last time G_Ticker was called.

gamestate_t     oldgamestate; 
 
gameaction_t    gameaction; 
gamestate_t     gamestate; 
skill_t         gameskill; 
boolean		respawnmonsters;
int             gameepisode; 
int             gamemap; 

// If non-zero, exit the level after this number of minutes.

boolean         paused; 
boolean         sendpause;             	// send a pause event next tic 
boolean         sendsave;             	// send a save event next tic 
boolean         usergame;               // ok to save / end game 
 
boolean         nodrawers;              // for comparative timing purposes 
 
boolean         playeringame[MAXPLAYERS]; 
player_t        players[MAXPLAYERS]; 

int             consoleplayer;          // player taking events and displaying 
int             displayplayer;          // view being displayed 
int             levelstarttic;          // gametic at level start 
int             totalkills, totalitems, totalsecret;    // for intermission 
int             totalleveltimes;        // [crispy] CPhipps - total time for all completed levels
 
boolean         precache = true;        // if true, load all graphics at start 

boolean         testcontrols = false;    // Invoked by setup to test controls
int             testcontrols_mousespeed;
 

 
wbstartstruct_t wminfo;               	// parms for world map / intermission 
 
byte		consistancy[MAXPLAYERS][BACKUPTICS]; 
 
#define MAXPLMOVE		(forwardmove[1]) 
 
#define TURBOTHRESHOLD	0x32

fixed_t         forwardmove[2] = {0x19, 0x32}; 
fixed_t         sidemove[2] = {0x18, 0x28}; 
fixed_t         angleturn[3] = {640, 1280, 320};    // + slow turn 

static int *weapon_keys[] = {
    &key_weapon1,
    &key_weapon2,
    &key_weapon3,
    &key_weapon4,
    &key_weapon5,
    &key_weapon6,
    &key_weapon7,
    &key_weapon8
};

// Set to -1 or +1 to switch to the previous or next weapon.

static int next_weapon = 0;

// Used for prev/next weapon keys.

static const struct
{
    weapontype_t weapon;
    weapontype_t weapon_num;
} weapon_order_table[] = {
    { wp_fist,            wp_fist },
    { wp_chainsaw,        wp_fist },
    { wp_pistol,          wp_pistol },
    { wp_shotgun,         wp_shotgun },
    { wp_chaingun,        wp_chaingun },
    { wp_missile,         wp_missile },
    { wp_plasma,          wp_plasma },
    { wp_bfg,             wp_bfg }
};

#define SLOWTURNTICS	6 
 
#define NUMKEYS		256 
#define MAX_JOY_BUTTONS 20

static boolean  gamekeydown[NUMKEYS]; 
static int      turnheld;		// for accelerative turning 
 
static boolean  mousearray[MAX_MOUSE_BUTTONS + 1];
static boolean *mousebuttons = &mousearray[1];  // allow [-1]

// mouse values are used once 
int             mousex;
int             mousey;         

// [crispy] for rounding error
typedef struct carry_s
{
    double angle;
    double pitch;
    double side;
    double vert;
} carry_t;

static carry_t prevcarry;
static carry_t carry;

static int      dclicktime;
static boolean  dclickstate;
static int      dclicks; 
static int      dclicktime2;
static boolean  dclickstate2;
static int      dclicks2;

// joystick values are repeated 
static int      joyxmove;
static int      joyymove;
static int      joystrafemove;
static boolean  joyarray[MAX_JOY_BUTTONS + 1]; 
static boolean *joybuttons = &joyarray[1];		// allow [-1] 
 
char            savename[256]; // [crispy] moved here
static int      savegameslot; 
static char     savedescription[32]; 
 
static ticcmd_t basecmd; // [crispy]

// [JN] Determinates speed of camera Z-axis movement in spectator mode.
static int      crl_camzspeed;

 
static boolean WeaponSelectable(weapontype_t weapon)
{
    // Can't select a weapon if we don't own it.

    if (!players[consoleplayer].weaponowned[weapon])
    {
        return false;
    }

    // Can't select the fist if we have the chainsaw, unless
    // we also have the berserk pack.

    if (weapon == wp_fist
     && players[consoleplayer].weaponowned[wp_chainsaw]
     && !players[consoleplayer].powers[pw_strength])
    {
        return false;
    }

    return true;
}

static int G_NextWeapon(int direction)
{
    weapontype_t weapon;
    int start_i, i;

    // Find index in the table.

    if (players[consoleplayer].pendingweapon == wp_nochange)
    {
        weapon = players[consoleplayer].readyweapon;
    }
    else
    {
        weapon = players[consoleplayer].pendingweapon;
    }

    for (i=0; (size_t)i<arrlen(weapon_order_table); ++i)
    {
        if (weapon_order_table[i].weapon == weapon)
        {
            break;
        }
    }

    // Switch weapon. Don't loop forever.
    start_i = i;
    do
    {
        i += direction;
        i = (i + arrlen(weapon_order_table)) % arrlen(weapon_order_table);
    } while (i != start_i && !WeaponSelectable(weapon_order_table[i].weapon));

    return weapon_order_table[i].weapon_num;
}

// [crispy] holding down the "Run" key may trigger special behavior,
// e.g. quick exit, clean screenshots, resurrection from savegames
boolean speedkeydown (void)
{
    return (key_speed < NUMKEYS && gamekeydown[key_speed]) ||
           (mousebspeed < MAX_MOUSE_BUTTONS && mousebuttons[mousebspeed]) ||
           (joybspeed < MAX_JOY_BUTTONS && joybuttons[joybspeed]);
}

// [crispy] for carrying rounding error
static int CarryError(double value, const double *prevcarry, double *carry)
{
    const double desired = value + *prevcarry;
    const int actual = lround(desired);
    *carry = desired - actual;

    return actual;
}

static short CarryAngle(double angle)
{
    {
        return CarryError(angle, &prevcarry.angle, &carry.angle);
    }
}

static short CarryPitch(double pitch)
{
    return CarryError(pitch, &prevcarry.pitch, &carry.pitch);
}

static int CarryMouseVert(double vert)
{
    return CarryError(vert, &prevcarry.vert, &carry.vert);
}

static int CarryMouseSide(double side)
{
    const double desired = side + prevcarry.side;
    const int actual = lround(side * 0.5) * 2; // Even values only.
    carry.side = desired - actual;
    return actual;
}

static double CalcMouseAngle(int mousex)
{
    if (!mouseSensitivity)
        return 0.0;

    return (I_AccelerateMouse(mousex) * (mouseSensitivity + 5) * 8 / 10);
}

static double CalcMouseVert(int mousey)
{
    if (!mouseSensitivity)
        return 0.0;

    return (I_AccelerateMouseY(mousey) * (mouseSensitivity + 5) * 2 / 10);
}

//
// G_BuildTiccmd
// Builds a ticcmd from all of the available inputs
// or reads it from the demo buffer. 
// If recording a demo, write it out 
// 
void G_BuildTiccmd (ticcmd_t* cmd, int maketic) 
{ 
    int		i; 
    boolean	strafe;
    boolean	bstrafe; 
    int		speed;
    int		tspeed; 
    int		angle = 0; // [crispy]
    short	mousex_angleturn; // [crispy]
    int		forward;
    int		side;
    ticcmd_t spect;

    if (!crl_spectating)
    {
        // [crispy] For fast polling.
        G_PrepTiccmd();
        memcpy(cmd, &basecmd, sizeof(*cmd));
        memset(&basecmd, 0, sizeof(ticcmd_t));
    }
    else
    {
        // [JN] CRL - can't interpolate spectator.
        memset(cmd, 0, sizeof(ticcmd_t));
        // [JN] CRL - reset basecmd.angleturn for exact
        // position of jumping to the camera position.
        basecmd.angleturn = 0;
    }

	// needed for net games
    cmd->consistancy = consistancy[consoleplayer][maketic%BACKUPTICS]; 
 	
 	// RestlessRodent -- If spectating then the player loses all input
 	memmove(&spect, cmd, sizeof(spect));
 	// [JN] Allow saving and pausing while spectating.
 	if (crl_spectating && !sendsave && !sendpause)
 		cmd = &spect;
 	
    strafe = gamekeydown[key_strafe] || mousebuttons[mousebstrafe] 
	|| joybuttons[joybstrafe]; 

    // fraggle: support the old "joyb_speed = 31" hack which
    // allowed an autorun effect

    // [crispy] when "always run" is active,
    // pressing the "run" key will result in walking
    speed = (key_speed >= NUMKEYS
         || joybspeed >= MAX_JOY_BUTTONS);
    speed ^= speedkeydown();
    crl_camzspeed = speed;
 
    forward = side = 0;
    
    // use two stage accelerative turning
    // on the keyboard and joystick
    if (joyxmove < 0
	|| joyxmove > 0  
	|| gamekeydown[key_right]
	|| gamekeydown[key_left]) 
	turnheld += ticdup; 
    else 
	turnheld = 0; 

    if (turnheld < SLOWTURNTICS) 
	tspeed = 2;             // slow turn 
    else 
	tspeed = speed;
    
    // [crispy] add quick 180° reverse
    if (gamekeydown[key_180turn])
    {
        angle += ANG180 >> FRACBITS;
        gamekeydown[key_180turn] = false;
    }

    // [crispy] toggle "always run"
    if (gamekeydown[key_autorun])
    {
        static int joybspeed_old = 2;

        if (joybspeed >= MAX_JOY_BUTTONS)
        {
            joybspeed = joybspeed_old;
        }
        else
        {
            joybspeed_old = joybspeed;
            joybspeed = MAX_JOY_BUTTONS;
        }

        CT_SetMessage(&players[consoleplayer], joybspeed >= MAX_JOY_BUTTONS ?
                       ID_AUTORUN_ON : ID_AUTORUN_OFF, false, NULL);

        S_StartSound(NULL, sfx_swtchn);

        gamekeydown[key_autorun] = false;
    }

    // [JN] Toggle mouse look.
    if (gamekeydown[key_mouse_look])
    {
        mouse_look ^= 1;
        if (!mouse_look)
        {
            cmd->lookdir = players[consoleplayer].lookdir = 0;
        }
        CT_SetMessage(&players[consoleplayer], mouse_look ?
                       ID_MLOOK_ON : ID_MLOOK_OFF, false, NULL);
        S_StartSound(NULL, sfx_swtchn);
        gamekeydown[key_mouse_look] = false;
    }

    // [JN] Toggle vertical mouse movement.
    if (gamekeydown[key_novert])
    {
        mouse_novert ^= 1;
        CT_SetMessage(&players[consoleplayer], mouse_novert ?
                      ID_NOVERT_ON : ID_NOVERT_OFF, false, NULL);
        S_StartSound(NULL, sfx_swtchn);
        gamekeydown[key_novert] = false;
    }

    // let movement keys cancel each other out
    if (strafe) 
    { 
        if (!cmd->angleturn)
        {
            if (gamekeydown[key_right])
            {
                // fprintf(stderr, "strafe right\n");
                side += sidemove[speed];
            }
            if (gamekeydown[key_left])
            {
                //	fprintf(stderr, "strafe left\n");
                side -= sidemove[speed];
            }
            if (use_analog && joyxmove)
            {
                joyxmove = joyxmove * joystick_move_sensitivity / 10;
                joyxmove = (joyxmove > FRACUNIT) ? FRACUNIT : joyxmove;
                joyxmove = (joyxmove < -FRACUNIT) ? -FRACUNIT : joyxmove;
                side += FixedMul(sidemove[speed], joyxmove);
            }
            else if (joystick_move_sensitivity)
            {
                if (joyxmove > 0)
                    side += sidemove[speed];
                if (joyxmove < 0)
                    side -= sidemove[speed];
            }
        }
    } 
    else 
    { 
	if (gamekeydown[key_right])
	    angle -= angleturn[tspeed];
	if (gamekeydown[key_left])
	    angle += angleturn[tspeed];
        if (use_analog && joyxmove)
        {
            // Cubic response curve allows for finer control when stick
            // deflection is small.
            joyxmove = FixedMul(FixedMul(joyxmove, joyxmove), joyxmove);
            joyxmove = joyxmove * joystick_turn_sensitivity / 10;
            angle -= FixedMul(angleturn[1], joyxmove);
        }
        else if (joystick_turn_sensitivity)
        {
            if (joyxmove > 0)
                angle -= angleturn[tspeed];
            if (joyxmove < 0)
                angle += angleturn[tspeed];
        }
    } 
 
    if (gamekeydown[key_up]) 
    {
	// fprintf(stderr, "up\n");
	forward += forwardmove[speed]; 
    }
    if (gamekeydown[key_down]) 
    {
	// fprintf(stderr, "down\n");
	forward -= forwardmove[speed]; 
    }

    if (use_analog && joyymove)
    {
        joyymove = joyymove * joystick_move_sensitivity / 10;
        joyymove = (joyymove > FRACUNIT) ? FRACUNIT : joyymove;
        joyymove = (joyymove < -FRACUNIT) ? -FRACUNIT : joyymove;
        forward -= FixedMul(forwardmove[speed], joyymove);
    }
    else if (joystick_move_sensitivity)
    {
        if (joyymove < 0)
            forward += forwardmove[speed];
        if (joyymove > 0)
            forward -= forwardmove[speed];
    }

    if (gamekeydown[key_strafeleft]
     || joybuttons[joybstrafeleft]
     || mousebuttons[mousebstrafeleft])
    {
        side -= sidemove[speed];
    }

    if (gamekeydown[key_straferight]
     || joybuttons[joybstraferight]
     || mousebuttons[mousebstraferight])
    {
        side += sidemove[speed]; 
    }

    if (use_analog && joystrafemove)
    {
        joystrafemove = joystrafemove * joystick_move_sensitivity / 10;
        joystrafemove = (joystrafemove > FRACUNIT) ? FRACUNIT : joystrafemove;
        joystrafemove = (joystrafemove < -FRACUNIT) ? -FRACUNIT : joystrafemove;
        side += FixedMul(sidemove[speed], joystrafemove);
    }
    else if (joystick_move_sensitivity)
    {
        if (joystrafemove < 0)
            side -= sidemove[speed];
        if (joystrafemove > 0)
            side += sidemove[speed];
    }

    // buttons

    if (gamekeydown[key_fire] || mousebuttons[mousebfire] 
	|| joybuttons[joybfire]) 
	cmd->buttons |= BT_ATTACK; 
 
    if (gamekeydown[key_use]
     || joybuttons[joybuse]
     || mousebuttons[mousebuse])
    { 
	cmd->buttons |= BT_USE;
	// clear double clicks if hit use button 
	dclicks = 0;                   
    } 

    // If the previous or next weapon button is pressed, the
    // next_weapon variable is set to change weapons when
    // we generate a ticcmd.  Choose a new weapon.

    if (gamestate == GS_LEVEL && next_weapon != 0)
    {
        i = G_NextWeapon(next_weapon);
        cmd->buttons |= BT_CHANGE;
        cmd->buttons |= i << BT_WEAPONSHIFT;
    }
    else
    {
        // Check weapon keys.

        for (i=0; (size_t)i<arrlen(weapon_keys); ++i)
        {
            int key = *weapon_keys[i];

            if (gamekeydown[key])
            {
                cmd->buttons |= BT_CHANGE;
                cmd->buttons |= i<<BT_WEAPONSHIFT;
                break;
            }
        }
    }

    next_weapon = 0;

    if (gamekeydown[key_message_refresh])
    {
        players[consoleplayer].messageTics = MESSAGETICS;
    }

    // mouse
    if (mousebuttons[mousebforward]) 
    {
	forward += forwardmove[speed];
    }
    if (mousebuttons[mousebbackward])
    {
        forward -= forwardmove[speed];
    }

    if (mouse_dclick_use)
    {
        // forward double click
        if (mousebuttons[mousebforward] != dclickstate && dclicktime > 1 ) 
        { 
            dclickstate = mousebuttons[mousebforward]; 
            if (dclickstate) 
                dclicks++; 
            if (dclicks == 2) 
            { 
                cmd->buttons |= BT_USE; 
                dclicks = 0; 
            } 
            else 
                dclicktime = 0; 
        } 
        else 
        { 
            dclicktime += ticdup; 
            if (dclicktime > 20) 
            { 
                dclicks = 0; 
                dclickstate = 0; 
            } 
        }
        
        // strafe double click
        bstrafe =
            mousebuttons[mousebstrafe] 
            || joybuttons[joybstrafe]; 
        if (bstrafe != dclickstate2 && dclicktime2 > 1 ) 
        { 
            dclickstate2 = bstrafe; 
            if (dclickstate2) 
                dclicks2++; 
            if (dclicks2 == 2) 
            { 
                cmd->buttons |= BT_USE; 
                dclicks2 = 0; 
            } 
            else 
                dclicktime2 = 0; 
        } 
        else 
        { 
            dclicktime2 += ticdup; 
            if (dclicktime2 > 20) 
            { 
                dclicks2 = 0; 
                dclickstate2 = 0; 
            } 
        } 
    }

    // [crispy] mouse look
    if (mouse_look)
    {
        const double vert = CalcMouseVert(mousey);
        cmd->lookdir += mouse_y_invert ? CarryPitch(-vert) : CarryPitch(vert);
    }
    else
    if (!mouse_novert)
    {
    forward += CarryMouseVert(CalcMouseVert(mousey));
    }

    if (strafe) 
	side += mousex*2;
    else 
    {
        if (!crl_spectating)
        cmd->angleturn += CarryMouseSide(mousex);
        else
        angle -= mousex*0x8;
    }

    mousex_angleturn = cmd->angleturn;

    if (mousex_angleturn == 0)
    {
        // No movement in the previous frame

        testcontrols_mousespeed = 0;
    }
    
    if (angle)
    {
        if (!crl_spectating)
        {
        cmd->angleturn = CarryAngle(cmd->angleturn + angle);
        localview.ticangleturn = gp_flip_levels ?
            (mousex_angleturn - cmd->angleturn) :
            (cmd->angleturn - mousex_angleturn);
        }
        else
        {
        const short old_angleturn = cmd->angleturn;
        cmd->angleturn = CarryAngle(localview.rawangle + angle);
        localview.ticangleturn = gp_flip_levels ?
            (old_angleturn - cmd->angleturn) :
            (cmd->angleturn - old_angleturn);
        }
    }

    mousex = mousey = 0;
	 
    if (forward > MAXPLMOVE) 
	forward = MAXPLMOVE; 
    else if (forward < -MAXPLMOVE) 
	forward = -MAXPLMOVE; 
    if (side > MAXPLMOVE) 
	side = MAXPLMOVE; 
    else if (side < -MAXPLMOVE) 
	side = -MAXPLMOVE; 
 
    cmd->forwardmove += forward; 
    cmd->sidemove += side;

    // [crispy]
    localview.angle = 0;
    localview.rawangle = 0.0;
    prevcarry = carry;
    
    // special buttons
    if (sendpause) 
    { 
	sendpause = false; 

	if (gameaction != ga_loadgame)
	{
	cmd->buttons = BT_SPECIAL | BTS_PAUSE; 
	}
    } 
 
    if (sendsave) 
    { 
	sendsave = false; 
	cmd->buttons = BT_SPECIAL | BTS_SAVEGAME | (savegameslot<<BTS_SAVESHIFT); 
    } 

    if (gp_flip_levels)
    {
	mousex_angleturn = -mousex_angleturn;
	cmd->angleturn = -cmd->angleturn;
	cmd->sidemove = -cmd->sidemove;
    }

    // If spectating, send the movement commands instead
    if (crl_spectating && !menuactive)
    	CRL_ImpulseCamera(cmd->forwardmove, cmd->sidemove, cmd->angleturn); 
} 
 
// -----------------------------------------------------------------------------
// G_InitSkyTextures
// [JN] CRY: define sky textures with optional emulation. Jaguar skies are:
// AREA 17 (Hell Keep) is using Deimos sky.
// AREA 24 (Military base) is using hellish sky.
// -----------------------------------------------------------------------------

void G_InitSkyTextures (void)
{
    if (gamemap < 9 || (!emu_jaguar_skies && gamemap == 24))
    {
        skytexture = R_TextureNumForName("SKY1_1");
        skytexture2 = R_TextureNumForName("SKY1_2");
        skyscrollspeed = 40; // slow for Phobos levels
    }
    else if (gamemap < (emu_jaguar_skies ? 18 : 17))
    {
        skytexture = R_TextureNumForName("SKY2_1");
        skytexture2 = R_TextureNumForName("SKY2_2");
        skyscrollspeed = 60; // Middle for Deimos levels
    }
    else
    {
        skytexture = R_TextureNumForName("SKY3_1");
        skytexture2 = R_TextureNumForName("SKY3_2");
        skyscrollspeed = 75; // Fast for Hellish levels
    }
}

//
// G_DoLoadLevel 
//
void G_DoLoadLevel (void) 
{ 
    int i;

	// [JN] Properly remove paused state and resume music playing.
	// Fixes a bug when pausing intermission screen causes locking up sound.
	if (paused)
	{
		paused = false;
		S_ResumeSound ();
	}

    // Set the sky map.
    // First thing, we have a dummy sky texture name,
    //  a flat. The data is in the WAD only because
    //  we look for an actual index, instead of simply
    //  setting one.

    skyflatnum = R_FlatNumForName(SKYFLATNAME);

    // [JN] Set Jaguar sky textures.
    G_InitSkyTextures();
    
    levelstarttic = gametic;        // for time calculation
    
    if (wipegamestate == GS_LEVEL) 
	wipegamestate = -1;             // force a wipe 

    gamestate = GS_LEVEL; 

    for (i=0 ; i<MAXPLAYERS ; i++) 
    { 
	if (playeringame[i] && players[i].playerstate == PST_DEAD) 
	    players[i].playerstate = PST_REBORN; 
    } 
		 
    // [JN] Pistol start game mode.
    if (gp_pistol_start)
    {
        G_PlayerReborn(0);
    }

    P_SetupLevel (gameepisode, gamemap);    
    // [JN] Do not reset chosen player view across levels in multiplayer
    // demo playback. However, it must be reset when starting a new game.
    if (usergame)
    {
        displayplayer = consoleplayer;		// view the guy you are playing    
    } 
    gameaction = ga_nothing; 
    Z_CheckHeap ();
    
    // clear cmd building stuff

    memset (gamekeydown, 0, sizeof(gamekeydown));
    joyxmove = joyymove = joystrafemove = 0;
    mousex = mousey = 0;
    memset(&localview, 0, sizeof(localview)); // [crispy]
    memset(&carry, 0, sizeof(carry)); // [crispy]
    memset(&prevcarry, 0, sizeof(prevcarry)); // [crispy]
    memset(&basecmd, 0, sizeof(basecmd)); // [crispy]
    sendpause = sendsave = paused = false;
    memset(mousearray, 0, sizeof(mousearray));
    memset(joyarray, 0, sizeof(joyarray));

    if (testcontrols)
    {
        CT_SetMessage(&players[consoleplayer], "Press escape to quit.", false, NULL);
    }
} 

static void SetJoyButtons(unsigned int buttons_mask)
{
    int i;

    for (i=0; i<MAX_JOY_BUTTONS; ++i)
    {
        int button_on = (buttons_mask & (1 << i)) != 0;

        // Detect button press:

        if (!joybuttons[i] && button_on)
        {
            // Weapon cycling:

            if (i == joybprevweapon)
            {
                next_weapon = -1;
            }
            else if (i == joybnextweapon)
            {
                next_weapon = 1;
            }
        }

        joybuttons[i] = button_on;
    }
}

static void SetMouseButtons(unsigned int buttons_mask)
{
    int i;

    for (i=0; i<MAX_MOUSE_BUTTONS; ++i)
    {
        unsigned int button_on = (buttons_mask & (1 << i)) != 0;

        // Detect button press:

        if (!mousebuttons[i] && button_on)
        {
            // [JN] CRL - move spectator camera up/down.
            if (crl_spectating && !menuactive)
            {
                if (i == 4)  // Hardcoded mouse wheel down
                {
                    CRL_ImpulseCameraVert(false, crl_camzspeed ? 64 : 32); 
                }
                else
                if (i == 3)  // Hardcoded Mouse wheel down
                {
                    CRL_ImpulseCameraVert(true, crl_camzspeed ? 64 : 32);
                }
            }
            else
            {
                if (i == mousebprevweapon)
                {
                    next_weapon = -1;
                }
                else
                if (i == mousebnextweapon)
                {
                    next_weapon = 1;
                }
            }
        }

	mousebuttons[i] = button_on;
    }
}

//
// G_Responder  
// Get info needed to make ticcmd_ts for the players.
// 
boolean G_Responder (event_t* ev) 
{ 
    // any other key pops up menu if in demos
    if (gameaction == ga_nothing
	&&  gamestate == GS_DEMOSCREEN)
    { 
	if (ev->type == ev_keydown ||  
	    (ev->type == ev_mouse && ev->data1) || 
	    (ev->type == ev_joystick && ev->data1) ) 
	{ 
	    M_StartControlPanel (); 
	    return true; 
	} 
	return false; 
    } 

    if (gamestate == GS_LEVEL) 
    { 
	if (ST_Responder (ev)) 
	    return true;	// status window ate it 
	if (AM_Responder (ev)) 
	    return true;	// automap ate it 

    if (players[consoleplayer].cheatTics)
    {
        // [JN] Reset cheatTics if user have opened menu or moved/pressed mouse buttons.
	    if (menuactive || ev->type == ev_mouse)
	    players[consoleplayer].cheatTics = 0;

	    // [JN] Prevent other keys while cheatTics is running after typing "ID".
	    if (players[consoleplayer].cheatTics > 0)
	    return true;
    }
    } 
	 
    if (gamestate == GS_FINALE) 
    { 
	if (F_Responder (ev)) 
	    return true;	// finale ate the event 
    } 

    if (testcontrols && ev->type == ev_mouse)
    {
        // If we are invoked by setup to test the controls, save the 
        // mouse speed so that we can display it on-screen.
        // Perform a low pass filter on this so that the thermometer 
        // appears to move smoothly.

        testcontrols_mousespeed = abs(ev->data2);
    }

    // If the next/previous weapon keys are pressed, set the next_weapon
    // variable to change weapons when the next ticcmd is generated.

    if (ev->type == ev_keydown && ev->data1 == key_prevweapon)
    {
        next_weapon = -1;
    }
    else if (ev->type == ev_keydown && ev->data1 == key_nextweapon)
    {
        next_weapon = 1;
    }

    switch (ev->type) 
    { 
      case ev_keydown: 
	if (ev->data1 == key_pause) 
	{ 
	    sendpause = true; 
	}
        else if (ev->data1 <NUMKEYS) 
        {
	    gamekeydown[ev->data1] = true; 
        }

    // [JN] Flip level horizontally.
    if (ev->data1 == key_flip_levels)
    {
        gp_flip_levels ^= 1;
        // Redraw game screen
        R_ExecuteSetViewSize();
        // Update stereo separation
        S_UpdateStereoSeparation();
        // Audible feedback
        S_StartSound(NULL, sfx_swtchn);
    }   

    // [JN] CRL - Toggle extended HUD.
    if (ev->data1 == key_widget_enable)
    {
        widget_enable ^= 1;
        CT_SetMessage(&players[consoleplayer], widget_enable ?
                      ID_EXTHUD_ON : ID_EXTHUD_OFF, false, NULL);
        // Redraw status bar to possibly clean up 
        // remainings of demo progress bar.
        st_fullupdate = true;
    }

    // [JN] CRL - Toggle spectator mode.
    if (ev->data1 == key_spectator)
    {
        crl_spectating ^= 1;
        CT_SetMessage(&players[consoleplayer], crl_spectating ?
                       ID_SPECTATOR_ON : ID_SPECTATOR_OFF, false, NULL);
        pspr_interp = false;
    }        

    // [JN] CRL - Toggle freeze mode.
    if (ev->data1 == key_freeze)
    {
        crl_freeze ^= 1;
        CT_SetMessage(&players[consoleplayer], crl_freeze ?
                       ID_FREEZE_ON : ID_FREEZE_OFF, false, NULL);
    }    

    // [JN] CRL - Toggle notarget mode.
    if (ev->data1 == key_notarget)
    {
        player_t *player = &players[consoleplayer];

        player->cheats ^= CF_NOTARGET;
        P_ForgetPlayer(player);
        CT_SetMessage(player, player->cheats & CF_NOTARGET ?
                      ID_NOTARGET_ON : ID_NOTARGET_OFF, false, NULL);
    }

    // [JN] Woof - Toggle Buddha mode.
    if (ev->data1 == key_buddha)
    {
        player_t *player = &players[consoleplayer];

        player->cheats ^= CF_BUDDHA;
        CT_SetMessage(player, player->cheats & CF_BUDDHA ?
                      ID_BUDDHA_ON : ID_BUDDHA_OFF, false, NULL);
    }

	return true;    // eat key down events 
 
      case ev_keyup: 
	if (ev->data1 <NUMKEYS) 
	    gamekeydown[ev->data1] = false; 
	return false;   // always let key up events filter down 
		 
      case ev_mouse: 
        SetMouseButtons(ev->data1);
        mousex += ev->data2;
        mousey += ev->data3;
	return true;    // eat events 
 
      case ev_joystick: 
        SetJoyButtons(ev->data1);
	joyxmove = ev->data2; 
	joyymove = ev->data3; 
        joystrafemove = ev->data4;
	return true;    // eat events 
 
      default: 
	break; 
    } 
 
    return false; 
} 
 
// [crispy] For fast polling.
void G_FastResponder (void)
{
    if (newfastmouse)
    {
        mousex += fastmouse.data2;
        mousey += fastmouse.data3;

        newfastmouse = false;
    }
}
 
// [crispy]
void G_PrepTiccmd (void)
{
    const boolean strafe = gamekeydown[key_strafe] ||
        mousebuttons[mousebstrafe] || joybuttons[joybstrafe];

    if (mousex && !strafe)
    {
        localview.rawangle -= CalcMouseAngle(mousex);
        basecmd.angleturn = CarryAngle(localview.rawangle);
        localview.angle = gp_flip_levels ?
            -(basecmd.angleturn << 16) : (basecmd.angleturn << 16);
        mousex = 0;
    }

    if (mousey && mouse_look && !crl_spectating)
    {
        const double vert = CalcMouseVert(mousey);
        basecmd.lookdir += mouse_y_invert ?
                            CarryPitch(-vert): CarryPitch(vert);
        mousey = 0;
    }
}

// [crispy] re-read game parameters from command line
static void G_ReadGameParms (void)
{
    respawnparm = M_CheckParm ("-respawn");
    fastparm = M_CheckParm ("-fast");
    nomonsters = M_CheckParm ("-nomonsters");
}
 
//
// G_Ticker
// Make ticcmd_ts for the players.
//
void G_Ticker (void) 
{ 
    int		i;
    ticcmd_t*	cmd;
    
    // do player reborns if needed
    for (i=0 ; i<MAXPLAYERS ; i++) 
	if (playeringame[i] && players[i].playerstate == PST_REBORN) 
	    G_DoReborn (i);
    
    // do things to change the game state
    while (gameaction != ga_nothing) 
    { 
	switch (gameaction) 
	{ 
	  case ga_loadlevel: 
	    G_DoLoadLevel (); 
	    break; 
	  case ga_newgame: 
	    // [crispy] re-read game parameters from command line
	    G_ReadGameParms();
	    G_DoNewGame (); 
	    break; 
	  case ga_loadgame: 
	    // [crispy] re-read game parameters from command line
	    G_ReadGameParms();
	    G_DoLoadGame (); 
	    // [JN] Reset looking direction if game is loaded without mouse look
	    if (!mouse_look)
	    players[consoleplayer].lookdir = 0;
	    break; 
	  case ga_savegame: 
	    G_DoSaveGame (); 
	    break; 
	  case ga_completed: 
	    G_DoCompleted (); 
	    break; 
	  case ga_worlddone: 
	    G_DoWorldDone (); 
	    break; 
	  case ga_screenshot: 
	    V_ScreenShot("DOOM%02i.%s"); 
	    gameaction = ga_nothing; 
	    break; 
	  case ga_nothing: 
	    break; 
	} 
    }
    
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
	if (playeringame[i]) 
	{ 
	    cmd = &players[i].cmd; 

	    memcpy(cmd, &netcmds[i], sizeof(ticcmd_t));
	}
    }
    
    // check for special buttons
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
	if (playeringame[i]) 
	{ 
	    if (players[i].cmd.buttons & BT_SPECIAL) 
	    { 
		switch (players[i].cmd.buttons & BT_SPECIALMASK) 
		{ 
		  case BTS_PAUSE: 
		    paused ^= 1; 
		    if (paused) 
			S_PauseSound (); 
		    else 
		    // [crispy] Fixed bug when music was hearable with zero volume
		    if (musicVolume)
			S_ResumeSound (); 
		    break; 
					 
		  case BTS_SAVEGAME: 
		    if (!savedescription[0]) 
                    {
                        M_StringCopy(savedescription, "NET GAME",
                                     sizeof(savedescription));
                    }

		    savegameslot =  
			(players[i].cmd.buttons & BTS_SAVEMASK)>>BTS_SAVESHIFT; 
		    gameaction = ga_savegame; 
		    break; 
		} 
	    } 
	}
    }

    oldgamestate = gamestate;
    oldleveltime = realleveltime;
    
    // do main actions
    switch (gamestate) 
    { 
      case GS_LEVEL: 
	P_Ticker (); 
	ST_Ticker (); 
	AM_Ticker (); 

	// [JN] Gather target's health for widget and/or crosshair.
	if (widget_health || (xhair_draw && xhair_color > 1))
	{
		player_t *player = &players[displayplayer];

		// Do an overflow-safe trace to gather target's health.
		P_AimLineAttack(player->mo, player->mo->angle, MISSILERANGE, true);
	}

	break; 
	 
      case GS_INTERMISSION: 
	WI_Ticker (); 
	break; 
			 
      case GS_FINALE: 
	F_Ticker (); 
	break; 
 
      case GS_DEMOSCREEN: 
	D_PageTicker (); 
	break;
    }        

    // [JN] Reduce message tics independently from framerate and game states.
    // Tics can't go negative.
    CT_Ticker();

    //
    // [JN] Query time for time-related widgets:
    //
    
    // Level timer
    if (widget_time)
    {
        const int time = leveltime / TICRATE;
    
        M_snprintf(ID_Level_Time, sizeof(ID_Level_Time),
                   "%02d:%02d:%02d", time/3600, (time%3600)/60, time%60);
    }
    // Total time
    if (widget_totaltime)
    {
        const int totaltime = (totalleveltimes / TICRATE) + (leveltime / TICRATE);

        M_snprintf(ID_Total_Time, sizeof(ID_Total_Time),
                   "%02d:%02d:%02d", totaltime/3600, (totaltime%3600)/60, totaltime%60);
    }
    // Local time
    if (msg_local_time)
    {
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);

        strftime(ID_Local_Time, sizeof(ID_Local_Time),
                 msg_local_time == 1 ? "%I:%M%p" :    // 12-hour (HH:MM designation)
                                       "%H:%M", tm);  // 24-hour (HH:MM)
    }
} 
 
 
//
// PLAYER STRUCTURE FUNCTIONS
// also see P_SpawnPlayer in P_Things
//

//
// G_InitPlayer 
// Called at the start.
// Called by the game initialization functions.
//
void G_InitPlayer (int player) 
{
    // clear everything else to defaults
    G_PlayerReborn (player); 
}
 
 

//
// G_PlayerFinishLevel
// Can when a player completes a level.
//
void G_PlayerFinishLevel (int player) 
{ 
    player_t*	p; 
	 
    p = &players[player]; 
	 
    memset (p->powers, 0, sizeof (p->powers)); 
    memset (p->cards, 0, sizeof (p->cards)); 
    memset (p->tryopen, 0, sizeof (p->tryopen)); // [crispy] blinking key or skull in the status bar
    p->cheatTics = 0;
    p->messageTics = 0;
    p->messageCenteredTics = 0;
    p->targetsheathTics = 0;
    p->mo->flags &= ~MF_SHADOW;		// cancel invisibility 
    p->extralight = 0;			// cancel gun flashes 
    p->invulcolormap = 0;		// [JN] cancel invulnerability palette
    p->damagecount = 0;			// no palette changes 
    p->bonuscount = 0; 
    // [crispy] reset additional player properties
    p->lookdir = p->oldlookdir = p->centering = 0;
    st_palette = 0;
    // [JN] Redraw status bar background.
    if (p == &players[consoleplayer])
    {
        st_fullupdate = true;
    }
    // [JN] Return controls to the player.
    crl_spectating = 0;
} 
 

//
// G_PlayerReborn
// Called after a player dies 
// almost everything is cleared and initialized 
//
void G_PlayerReborn (int player) 
{ 
    player_t*	p; 
    int		i; 
    int		killcount;
    int		itemcount;
    int		secretcount; 
	 
    killcount = players[player].killcount; 
    itemcount = players[player].itemcount; 
    secretcount = players[player].secretcount; 
	 
    p = &players[player]; 
    memset (p, 0, sizeof(*p)); 
 
    players[player].killcount = killcount; 
    players[player].itemcount = itemcount; 
    players[player].secretcount = secretcount; 
 
    p->usedown = p->attackdown = true;	// don't do anything immediately 
    p->playerstate = PST_LIVE;       
    p->health = MAXHEALTH;
    p->readyweapon = p->pendingweapon = wp_pistol; 
    p->weaponowned[wp_fist] = true; 
    p->weaponowned[wp_pistol] = true; 
    p->ammo[am_clip] = 50; 
    p->messageTics = 0;
    p->messageCenteredTics = 0;
    p->targetsheathTics = 0;
	 
    for (i=0 ; i<NUMAMMO ; i++) 
	p->maxammo[i] = maxammo[i]; 
		 
    // [JN] Redraw status bar background.
    if (p == &players[consoleplayer])
    {
        st_fullupdate = true;
    }
}

// [crispy] clear the "savename" variable,
// i.e. restart level from scratch upon resurrection
void G_ClearSavename (void)
{
    M_StringCopy(savename, "", sizeof(savename));
}

//
// G_DoReborn 
// 
void G_DoReborn (int playernum) 
{ 
	// reload the level from scratch
	gameaction = ga_loadlevel;  
} 
 
 
void G_ScreenShot (void) 
{ 
    gameaction = ga_screenshot; 
} 
 

//
// G_DoCompleted 
//
boolean		secretexit; 
 
void G_ExitLevel (void) 
{ 
    secretexit = false; 
    gameaction = ga_completed; 
} 

// -----------------------------------------------------------------------------
// G_SecretExitLevel
// -----------------------------------------------------------------------------

void G_SecretExitLevel (void) 
{ 
	secretexit = true; 
    gameaction = ga_completed; 
} 

// -----------------------------------------------------------------------------
// G_DoCompleted
// -----------------------------------------------------------------------------

void G_DoCompleted (void) 
{ 
	int i; 

	gameaction = ga_nothing; 

	for (i = 0 ; i < MAXPLAYERS ; i++) 
		if (playeringame[i]) 
			G_PlayerFinishLevel (i); // take away cards and stuff 

	if (automapactive) 
	AM_Stop (); 

    wminfo.didsecret = players[consoleplayer].didsecret; 
    wminfo.epsd = gameepisode -1; 
    wminfo.last = gamemap -1;

	// wminfo.next is 0 biased, unlike gamemap
	// [JN] Handling secret exit:
	if (secretexit)
	{
		switch(gamemap)
		{
			// Secret exit from Toxin Refinery (3) to Military Base (24-1)
			case 3:  wminfo.next = 23;
			break;
		}
	}
	else
	{
		switch(gamemap)
		{
			// After Military Base (24) go to Command Control (4-1)
			case 24: wminfo.next = 3;
			break;
			
			default: wminfo.next = gamemap;
		}
	}

	wminfo.maxkills = totalkills; 
	wminfo.maxitems = totalitems; 
	wminfo.maxsecret = totalsecret; 
    wminfo.pnum = consoleplayer; 
 
    for (i=0 ; i<MAXPLAYERS ; i++) 
    { 
	wminfo.plyr[i].in = playeringame[i]; 
	wminfo.plyr[i].skills = players[i].killcount;
	wminfo.plyr[i].sitems = players[i].itemcount; 
	wminfo.plyr[i].ssecret = players[i].secretcount; 
	wminfo.plyr[i].stime = leveltime; 
    } 
 
    // [crispy] CPhipps - total time for all completed levels
    // cph - modified so that only whole seconds are added to the totalleveltimes
    // value; so our total is compatible with the "naive" total of just adding
    // the times in seconds shown for each level. Also means our total time
    // will agree with Compet-n.
    wminfo.totaltimes = (totalleveltimes += (leveltime - leveltime % TICRATE));

    gamestate = GS_INTERMISSION; 
    automapactive = false; 
 
    WI_Start (&wminfo); 
} 


//
// G_WorldDone 
//
void G_WorldDone (void) 
{ 
    gameaction = ga_worlddone; 

    if (secretexit) 
	players[consoleplayer].didsecret = true; 

    if (gamemap == 23)
	F_StartFinale ();
} 
 
void G_DoWorldDone (void) 
{        
    idmusnum = -1;  // [JN] jff 3/17/98 allow new level's music to be loaded
    gamestate = GS_LEVEL; 
    gamemap = wminfo.next+1; 
    G_DoLoadLevel (); 
    gameaction = ga_nothing; 
    AM_clearMarks();  // [JN] jff 4/12/98 clear any marks on the automap
} 
 


//
// G_InitFromSavegame
// Can be called by the startup code or the menu task. 
//


void G_LoadGame (char* name) 
{ 
    M_StringCopy(savename, name, sizeof(savename));
    gameaction = ga_loadgame; 
} 

void G_DoLoadGame (void) 
{ 
    int savedleveltime;
	 
    gameaction = ga_nothing; 
	 
    save_stream = M_fopen(savename, "rb");

    if (save_stream == NULL)
    {
        return;
    }

    savegame_error = false;

    if (!P_ReadSaveGameHeader())
    {
        fclose(save_stream);
        return;
    }

    savedleveltime = leveltime;
    
    // load a base level 
    G_InitNew (gameskill, gameepisode, gamemap); 
 
    leveltime = savedleveltime;

    // dearchive all the modifications
    P_UnArchivePlayers (); 
    P_UnArchiveWorld (); 
    P_UnArchiveThinkers (); 
    P_UnArchiveSpecials (); 
 
    if (!P_ReadSaveGameEOF())
	I_Error ("Bad savegame");

    // [JN] Restore total level times.
    P_UnArchiveTotalTimes ();
    // [JN] Restore monster targets.
    P_RestoreTargets ();
    // [JN] Restore automap marks.
    P_UnArchiveAutomap ();
    // [plums] Restore old sector specials.
    P_UnArchiveOldSpecials ();

    fclose(save_stream);
    
    if (setsizeneeded)
	R_ExecuteSetViewSize ();
    
    // draw the pattern into the back screen
    R_FillBackScreen ();   

    // [crispy] if the player is dead in this savegame,
    // do not consider it for reload
    if (players[consoleplayer].health <= 0)
	G_ClearSavename();

    // [JN] If "On death action" is set to "last save",
    // then prevent holded "use" button to work for next few tics.
    // This fixes imidiate pressing on wall upon reloading
    // a save game, if "use" button is kept pressed.
    if (gp_death_use_action == 1)
	players[consoleplayer].usedown = true;
} 
 

//
// G_SaveGame
// Called by the menu task.
// Description is a 24 byte text string 
//
void
G_SaveGame
( int	slot,
  char*	description )
{
    savegameslot = slot;
    M_StringCopy(savedescription, description, sizeof(savedescription));
    sendsave = true;
}

void G_DoSaveGame (void) 
{ 
    char *savegame_file;
    char *temp_savegame_file;
    char *recovery_savegame_file;

    recovery_savegame_file = NULL;
    temp_savegame_file = P_TempSaveGameFile();
    savegame_file = P_SaveGameFile(savegameslot);

    // Open the savegame file for writing.  We write to a temporary file
    // and then rename it at the end if it was successfully written.
    // This prevents an existing savegame from being overwritten by
    // a corrupted one, or if a savegame buffer overrun occurs.
    save_stream = M_fopen(temp_savegame_file, "wb");

    if (save_stream == NULL)
    {
        // Failed to save the game, so we're going to have to abort. But
        // to be nice, save to somewhere else before we call I_Error().
        recovery_savegame_file = M_TempFile("recovery.sav");
        save_stream = M_fopen(recovery_savegame_file, "wb");
        if (save_stream == NULL)
        {
            I_Error("Failed to open either '%s' or '%s' to write savegame.",
                    temp_savegame_file, recovery_savegame_file);
        }
    }

    savegame_error = false;

    P_WriteSaveGameHeader(savedescription);

    P_ArchivePlayers ();
    P_ArchiveWorld ();
    P_ArchiveThinkers ();
    P_ArchiveSpecials ();

    P_WriteSaveGameEOF();

    // [JN] Write total level times after EOF terminator
    // to keep compatibility with vanilla save games.
    P_ArchiveTotalTimes ();

    P_ArchiveAutomap ();

    // [plums] write old sector specials (for revealed secrets) at the end
    // to keep save compatibility with previous versions
    P_ArchiveOldSpecials ();

    // Finish up, close the savegame file.

    fclose(save_stream);

    if (recovery_savegame_file != NULL)
    {
        // We failed to save to the normal location, but we wrote a
        // recovery file to the temp directory. Now we can bomb out
        // with an error.
        I_Error("Failed to open savegame file '%s' for writing.\n"
                "But your game has been saved to '%s' for recovery.",
                temp_savegame_file, recovery_savegame_file);
    }

    // Now rename the temporary savegame file to the actual savegame
    // file, overwriting the old savegame if there was one there.

    M_remove(savegame_file);
    M_rename(temp_savegame_file, savegame_file);

    gameaction = ga_nothing;
    M_StringCopy(savedescription, "", sizeof(savedescription));
    M_StringCopy(savename, savegame_file, sizeof(savename));

    CT_SetMessage(&players[consoleplayer], GGSAVED, false, NULL);

    // draw the pattern into the back screen
    R_FillBackScreen ();
}
 

//
// G_InitNew
// Can be called by the startup code or the menu task,
// consoleplayer, displayplayer, playeringame[] should be set. 
//
skill_t	d_skill; 
int     d_episode; 
int     d_map; 
 
void
G_DeferedInitNew
( skill_t	skill,
  int		episode,
  int		map) 
{ 
    d_skill = skill; 
    d_episode = episode; 
    d_map = map; 
    G_ClearSavename();
    gameaction = ga_newgame; 
} 


void G_DoNewGame (void) 
{
    idmusnum = -1;  // [JN] Andrey Budko: allow new level's music to be loaded
    playeringame[1] = playeringame[2] = playeringame[3] = 0;
    // [crispy] do not reset -respawn, -fast and -nomonsters parameters
    /*
    respawnparm = false;
    fastparm = false;
    nomonsters = false;
    */
    consoleplayer = 0;
    G_InitNew (d_skill, d_episode, d_map); 
    gameaction = ga_nothing; 
} 


void
G_InitNew
( skill_t	skill,
  int		episode,
  int		map )
{
	int             i;
	// [crispy] make sure "fast" parameters are really only applied once
	static boolean fast_applied;

	if (paused)
	{
		paused = false;
		S_ResumeSound ();
	}

	if (skill > sk_nightmare)
	skill = sk_nightmare;

	// [JN] Episode itself is never used, but still necessary.
	episode = 1;

	// [JN] Catch unexisting maps.
	if (map < 1)
	{
		map = 1;
	}
	if (map > 24)
	{
		map = 24;
	}

	M_ClearRandom ();

	// [JN] Jaguar: no respawning on Nightmare,
	// but keep command line parameter.
	if (/*skill == sk_nightmare ||*/ respawnparm)
	respawnmonsters = true;
	else
	respawnmonsters = false;

    // [crispy] make sure "fast" parameters are really only applied once
    if ((fastparm || skill == sk_nightmare) && !fast_applied)
    {
	// [JN] Jaguar: slightly different (slower) speeds for Demons and Spectres.
	states[S_SARG_ATK1].tics = 4;
	states[S_SARG_ATK2].tics = 4;
	states[S_SARG_ATK3].tics = 4;
	mobjinfo[MT_SERGEANT].speed = 15; 
	mobjinfo[MT_SHADOWS].speed = 15; 

	mobjinfo[MT_BRUISERSHOT].speed = 20*FRACUNIT;
	mobjinfo[MT_HEADSHOT].speed = 20*FRACUNIT;
	mobjinfo[MT_TROOPSHOT].speed = 20*FRACUNIT;
	fast_applied = true;
    }
    else if (!fastparm && skill != sk_nightmare && fast_applied)
    {
	states[S_SARG_ATK1].tics = 8;
	states[S_SARG_ATK2].tics = 8;
	states[S_SARG_ATK3].tics = 8;
	mobjinfo[MT_SERGEANT].speed = 10; 
	mobjinfo[MT_SHADOWS].speed = 10; 

	mobjinfo[MT_BRUISERSHOT].speed = 15*FRACUNIT;
	mobjinfo[MT_HEADSHOT].speed = 10*FRACUNIT;
	mobjinfo[MT_TROOPSHOT].speed = 10*FRACUNIT;
	fast_applied = false;
    }

	// force players to be initialized upon first level load
	for (i=0 ; i<MAXPLAYERS ; i++)
	players[i].playerstate = PST_REBORN;

	usergame = true;                // will be set false if a demo
	paused = false;
	automapactive = false;
	gameepisode = episode;
	gamemap = map;
	gameskill = skill;

	// [crispy] CPhipps - total time for all completed levels
	totalleveltimes = 0;

	// [JN] jff 4/16/98 force marks on automap cleared every new level start
	AM_clearMarks();

    G_DoLoadLevel ();
}
