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


#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "doomdef.h" 
#include "doomkeys.h"
#include "doomstat.h"
#include "deh_main.h"
#include "deh_misc.h"
#include "z_zone.h"
#include "f_finale.h"
#include "m_argv.h"
#include "m_controls.h"
#include "m_misc.h"
#include "m_menu.h"
#include "m_random.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_input.h"
#include "i_video.h"
#include "p_setup.h"
#include "p_saveg.h"
#include "p_tick.h"
#include "d_main.h"
#include "wi_stuff.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "am_map.h"
#include "v_video.h"
#include "w_wad.h"
#include "p_local.h" 
#include "s_sound.h"
#include "sounds.h"
#include "r_data.h"
#include "r_sky.h"
#include "g_game.h"
#include "d_englsh.h"
#include "jn.h"


#define SAVEGAMESIZE	0x2c000


void G_ReadDemoTiccmd (ticcmd_t* cmd);
void G_WriteDemoTiccmd (ticcmd_t* cmd);
void G_PlayerReborn (int player);
void G_DoReborn (int playernum);
void G_DoLoadLevel (void);
void G_DoNewGame (void);
void G_DoPlayDemo (void);
void G_DoCompleted (void);
void G_DoVictory (void);
void G_DoWorldDone (void);
void G_DoSaveGame (void);

// -----------------------------------------------------------------------------
// Gamestate the last time G_Ticker was called.
// -----------------------------------------------------------------------------

gamestate_t     oldgamestate;

gameaction_t    gameaction;
gamestate_t     gamestate;
skill_t         gameskill;
boolean         respawnmonsters;
int             gameepisode;
int             gamemap;

// -----------------------------------------------------------------------------
// If non-zero, exit the level after this number of minutes.
// -----------------------------------------------------------------------------

int             timelimit;

boolean         paused;
boolean         sendpause;  // send a pause event next tic
boolean         sendsave;   // send a save event next tic
boolean         usergame;   // ok to save / end game

boolean         timingdemo; // if true, exit with report on completion
boolean         nodrawers;  // for comparative timing purposes
int             starttime;  // for comparative timing purposes

boolean         viewactive;

int             deathmatch; // only if started as net death
boolean         netgame;    // only true if packets are broadcast
boolean         playeringame[MAXPLAYERS];
player_t        players[MAXPLAYERS];

boolean         turbodetected[MAXPLAYERS];

int             consoleplayer;      // player taking events and displaying 
int             displayplayer;      // view being displayed 
int             levelstarttic;      // gametic at level start 
int             totalkills, totalitems, totalsecret;    // for intermission
int             totalleveltimes;    // [crispy] CPhipps - total time for all completed levels
 
char           *demoname;
boolean         demorecording;
boolean         longtics;       // cph's doom 1.91 longtics hack
boolean         lowres_turn;    // low resolution turning for longtics
boolean         demoplayback;
boolean         netdemo;
byte*           demobuffer;
byte*           demo_p;
byte*           demoend;
boolean         singledemo;     // quit after playing a demo from cmdline 
 
boolean         precache = true;    // if true, load all graphics at start 

boolean         testcontrols = false;   // Invoked by setup to test controls
int             testcontrols_mousespeed;



wbstartstruct_t wminfo;         // parms for world map / intermission 
 
byte            consistancy[MAXPLAYERS][BACKUPTICS]; 
 
#define MAXPLMOVE   (forwardmove[1]) 
 
#define TURBOTHRESHOLD  0x32

fixed_t     forwardmove[2] = {0x19, 0x32}; 
fixed_t     sidemove[2] = {0x18, 0x28}; 
fixed_t     angleturn[3] = {640, 1280, 320};    // + slow turn 

// [JN] If true, activate maximum weapon bobbing
boolean max_bobbing;

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

#define SLOWTURNTICS    6 
 
#define NUMKEYS         256 
#define MAX_JOY_BUTTONS 20

static boolean  gamekeydown[NUMKEYS]; 
static int      turnheld;   // for accelerative turning

static boolean  mousearray[MAX_MOUSE_BUTTONS + 1];
static boolean *mousebuttons = &mousearray[1];  // allow [-1]

// mouse values are used once 
int             mousex;
int             mousey;

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
static boolean *joybuttons = &joyarray[1];  // allow [-1] 
 
static int      savegameslot; 
static char     savedescription[32]; 
 
#define	BODYQUESIZE	32

mobj_t*         bodyque[BODYQUESIZE]; 
int             bodyqueslot; 


// [Julia] Optional Gameplay Enhancements

// int translucency = 1; (d_main.c)
int hightlight_things = 0;
int brightmaps        = 1;
int swirling_liquids  = 1;
int colored_blood     = 1;
int weapon_bobbing    = 1;
int snd_monomode      = 0;

int crosshair_draw    = 0;
int crosshair_health  = 1;
int crosshair_scale   = 0;

 
int G_CmdChecksum (ticcmd_t* cmd) 
{ 
    size_t  i;
    int     sum = 0; 

    for (i=0 ; i< sizeof(*cmd)/4 - 1 ; i++)
    sum += ((int *)cmd)[i];

    return sum; 
}


// -----------------------------------------------------------------------------
// WeaponSelectable
// -----------------------------------------------------------------------------

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
        return (singleplayer);  // [crispy] yes, we can
    }

    return true;
}


// -----------------------------------------------------------------------------
// G_NextWeapon
// -----------------------------------------------------------------------------

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

    for (i=0; i<arrlen(weapon_order_table); ++i)
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


// -----------------------------------------------------------------------------
// [crispy] holding down the "Run" key may trigger special behavior,
// e.g. quick exit, clean screenshots, resurrection from savegames
// -----------------------------------------------------------------------------

boolean speedkeydown (void)
{
    return (key_speed < NUMKEYS && gamekeydown[key_speed]) ||
           (joybspeed < MAX_JOY_BUTTONS && joybuttons[joybspeed]);
}


// -----------------------------------------------------------------------------
// G_BuildTiccmd
// Builds a ticcmd from all of the available inputs
// or reads it from the demo buffer. 
// If recording a demo, write it out 
// -----------------------------------------------------------------------------

void G_BuildTiccmd (ticcmd_t *cmd, int maketic) 
{ 
    int         i; 
    boolean     strafe;
    boolean     bstrafe; 
    int         speed;
    int         tspeed; 
    int         forward;
    int         side;
    int         look;
    static int  joybspeed_old = 2;

    memset(cmd, 0, sizeof(ticcmd_t));

    cmd->consistancy = consistancy[consoleplayer][maketic%BACKUPTICS]; 

    strafe = gamekeydown[key_strafe] || mousebuttons[mousebstrafe] || joybuttons[joybstrafe]; 

    // fraggle: support the old "joyb_speed = 31" hack which
    // allowed an autorun effect

    // [crispy] when "always run" is active,
    // pressing the "run" key will result in walking

    speed = key_speed >= NUMKEYS || joybspeed >= MAX_JOY_BUTTONS;
    speed ^= speedkeydown();    // [Julia] Speed button modifier

    forward = side = look = 0;

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
        tspeed = 2; // slow turn 
    else 
        tspeed = speed;

    // [crispy] toggle always run
    if (gamekeydown[key_toggleautorun])
    {
        static char autorunmsg[24];

        if (joybspeed >= MAX_JOY_BUTTONS)
        {
            joybspeed = joybspeed_old;
        }
        else
        {
            joybspeed_old = joybspeed;
            joybspeed = 29;
        }

        // [Julia] Added sound feedback
        M_snprintf(autorunmsg, sizeof(autorunmsg), STSTR_ALWAYSRUN "%s",
            (joybspeed >= MAX_JOY_BUTTONS) ? STSTR_ALWRUNON : STSTR_ALWRUNOFF);

        players[consoleplayer].message = autorunmsg;
        S_StartSound(NULL,sfx_swtchn);

        gamekeydown[key_toggleautorun] = false;
    }

    // let movement keys cancel each other out
    if (strafe) 
    { 
    if (gamekeydown[key_right]) 
    {
        side += sidemove[speed]; 
    }
    if (gamekeydown[key_left]) 
    {
        side -= sidemove[speed]; 
    }
    if (joyxmove > 0) 
        side += sidemove[speed]; 
    if (joyxmove < 0) 
        side -= sidemove[speed]; 
    } 

    else 
    { 
        if (gamekeydown[key_right]) 
            cmd->angleturn -= angleturn[tspeed]; 
        if (gamekeydown[key_left]) 
            cmd->angleturn += angleturn[tspeed]; 
        if (joyxmove > 0) 
            cmd->angleturn -= angleturn[tspeed]; 
        if (joyxmove < 0) 
            cmd->angleturn += angleturn[tspeed]; 
    } 

    if (gamekeydown[key_up]) 
    {
        forward += forwardmove[speed]; 
    }
    if (gamekeydown[key_down]) 
    {
        forward -= forwardmove[speed]; 
    }

    if (joyymove < 0) 
        forward += forwardmove[speed]; 
    if (joyymove > 0) 
        forward -= forwardmove[speed]; 

    if (gamekeydown[key_strafeleft]
    || joybuttons[joybstrafeleft]
    || mousebuttons[mousebstrafeleft]
    || joystrafemove < 0)
    {
        side -= sidemove[speed];
    }

    if (gamekeydown[key_straferight]
    || joybuttons[joybstraferight]
    || mousebuttons[mousebstraferight]
    || joystrafemove > 0)
    {
        side += sidemove[speed]; 
    }

    // buttons
    cmd->chatchar = HU_dequeueChatChar(); 

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

        for (i=0; i<arrlen(weapon_keys); ++i)
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

    // mouse
    if (mousebuttons[mousebforward]) 
    {
        forward += forwardmove[speed];
    }
    if (mousebuttons[mousebbackward])
    {
        forward -= forwardmove[speed];
    }

    if (dclick_use)
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

    if (strafe) 
        side += mousex*2; 
    else 
        cmd->angleturn -= mousex*0x8; 

    if (mousex == 0)
    {
        // No movement in the previous frame
        testcontrols_mousespeed = 0;
    }

    // [Julia] Mouselook: toggling
    if (gamekeydown[key_togglemlook])
    {
        static char mlookmsg[24];

        if (!mlook)
        {
            mlook = true;
        }
        else
        {
            mlook = false;
            look = TOCENTER;
        }

        M_snprintf(mlookmsg, sizeof(mlookmsg), STSRT_MOUSELOOK "%s",
            (mlook == true) ? STSTR_MLOOK_ON : STSTR_MLOOK_OFF);

        players[consoleplayer].message = mlookmsg;
        
        S_StartSound(NULL, sfx_swtchn);

        gamekeydown[key_togglemlook] = false;
    }

    // [Julia] Mouselook: handling
    if (!demoplayback && players[consoleplayer].playerstate == PST_LIVE && !paused && !menuactive)
    {
        cmd->lookdir += mousey;
        
        if (players[consoleplayer].lookdir > LOOKDIRMAX * MLOOKUNIT)
            players[consoleplayer].lookdir = LOOKDIRMAX * MLOOKUNIT;
        else
        if (players[consoleplayer].lookdir < -LOOKDIRMIN * MLOOKUNIT)
            players[consoleplayer].lookdir = -LOOKDIRMIN * MLOOKUNIT;
        
        if (look < 0)
        {
            look += 16;
        }
        cmd->lookfly = look;
    }

    mousex = mousey = 0; 

    // [Julia] "false" must be set as initial and returning condition.
    max_bobbing = false; 

    if (forward > MAXPLMOVE) 
    {
        forward = MAXPLMOVE; 
        max_bobbing = true;
    }
    else if (forward < -MAXPLMOVE) 
    {
        forward = -MAXPLMOVE; 
        max_bobbing = false;
    }
    if (side > MAXPLMOVE) 
    {
        side = MAXPLMOVE; 
        max_bobbing = true;
    }
    else if (side < -MAXPLMOVE) 
    {
        side = -MAXPLMOVE; 
        max_bobbing = false;        
    }

    cmd->forwardmove += forward; 
    cmd->sidemove += side;

    // [crispy] lookdir delta is stored in the lower 4 bits of the lookfly variable
    if (players[consoleplayer].playerstate == PST_LIVE)
    {
        if (look < 0)
        {
            look += 16;
        }
        cmd->lookfly = look;
    }

    // special buttons
    if (sendpause) 
    { 
        sendpause = false; 
        cmd->buttons = BT_SPECIAL | BTS_PAUSE; 
    } 

    if (sendsave) 
    { 
        sendsave = false; 
        cmd->buttons = BT_SPECIAL | BTS_SAVEGAME | (savegameslot<<BTS_SAVESHIFT); 
    } 
} 
 

// -----------------------------------------------------------------------------
// G_DoLoadLevel 
// -----------------------------------------------------------------------------

void G_DoLoadLevel (void) 
{ 
    int    i;
    char  *skytexturename;

    // [Julia] Properly remove paused state and resume music playing.
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

    skyflatnum = R_FlatNumForName(DEH_String(SKYFLATNAME));

    // [Julia] Set Jaguar skies
    if (gamemap < 9 || gamemap == 24)
    skytexturename = "SKY1";
    else if (gamemap < 17)
    skytexturename = "SKY2";
    else
    skytexturename = "SKY3";

    skytexture = R_TextureNumForName(skytexturename);

    levelstarttic = gametic;    // for time calculation
    
    if (wipegamestate == GS_LEVEL) 
        wipegamestate = -1;     // force a wipe 

    gamestate = GS_LEVEL; 

    for (i=0 ; i<MAXPLAYERS ; i++) 
    { 
        turbodetected[i] = false;
        if (playeringame[i] && players[i].playerstate == PST_DEAD) 
            players[i].playerstate = PST_REBORN; 
        memset (players[i].frags,0,sizeof(players[i].frags)); 
    } 

    P_SetupLevel (gameepisode, gamemap, 0, gameskill);    
    displayplayer = consoleplayer;		// view the guy you are playing    
    gameaction = ga_nothing; 
    Z_CheckHeap ();

    // clear cmd building stuff

    memset (gamekeydown, 0, sizeof(gamekeydown));
    joyxmove = joyymove = joystrafemove = 0;
    mousex = mousey = 0;
    sendpause = sendsave = paused = false;
    memset(mousearray, 0, sizeof(mousearray));
    memset(joyarray, 0, sizeof(joyarray));

    if (testcontrols)
    {
        players[consoleplayer].message = STSTR_TESTCTRLS;
    }
}

// -----------------------------------------------------------------------------
// SetJoyButtons
// -----------------------------------------------------------------------------

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


// -----------------------------------------------------------------------------
// SetMouseButtons
// -----------------------------------------------------------------------------

static void SetMouseButtons(unsigned int buttons_mask)
{
    int i;

    for (i=0; i<MAX_MOUSE_BUTTONS; ++i)
    {
        unsigned int button_on = (buttons_mask & (1 << i)) != 0;

        // Detect button press:

        if (!mousebuttons[i] && button_on)
        {
            if (i == mousebprevweapon)
            {
                next_weapon = -1;
            }
            else if (i == mousebnextweapon)
            {
                next_weapon = 1;
            }
        }

        mousebuttons[i] = button_on;
    }
}


// -----------------------------------------------------------------------------
// G_Responder
//
// Get info needed to make ticcmd_ts for the players.
// -----------------------------------------------------------------------------
 
boolean G_Responder (event_t *ev) 
{ 
    // any other key pops up menu if in demos
    if (gameaction == ga_nothing && !singledemo && (demoplayback || gamestate == GS_DEMOSCREEN)) 
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
    if (HU_Responder (ev)) 
        return true;	// chat ate the event 
    if (ST_Responder (ev)) 
        return true;	// status window ate it 
    if (AM_Responder (ev)) 
        return true;	// automap ate it 
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

    return true;    // eat key down events 

    case ev_keyup: 
    if (ev->data1 <NUMKEYS) 
        gamekeydown[ev->data1] = false; 
    return false;   // always let key up events filter down 

    case ev_mouse: 
        SetMouseButtons(ev->data1);
    mousex = ev->data2*(mouseSensitivity+5)/10; 
    mousey = ev->data3*(mouseSensitivity+5)/10; 
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


// -----------------------------------------------------------------------------
// G_Ticker
//
// Make ticcmd_ts for the players.
// -----------------------------------------------------------------------------

void G_Ticker (void) 
{ 
    int         i;
    ticcmd_t   *cmd;

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
            G_DoNewGame ();
            break;

            case ga_loadgame:
            G_DoLoadGame ();
            // [Julia] Reset look direction if game is loaded without mouse look
            if (!mlook)
            players[consoleplayer].lookdir = 0;
            break;

            case ga_savegame:
            G_DoSaveGame ();
            break;

            case ga_playdemo:
            G_DoPlayDemo ();
            break;

            case ga_completed:
            G_DoCompleted ();
            break;

            case ga_victory:
            F_StartFinale ();
            break;

            case ga_worlddone:
            G_DoWorldDone ();
            break; 

            case ga_screenshot:
            V_ScreenShot("YAGUAR%02i.%s");
            if (devparm)
            {
                players[consoleplayer].message = STSTR_SCRNSHT;
            }
            S_StartSound(NULL,sfx_itemup); // [Julia] Sound feedback
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

            if (demoplayback) 
            G_ReadDemoTiccmd (cmd); 
            if (demorecording) 
            G_WriteDemoTiccmd (cmd);
	    
            // check for turbo cheats

            // check ~ 4 seconds whether to display the turbo message. 
            // store if the turbo threshold was exceeded in any tics
            // over the past 4 seconds.  offset the checking period
            // for each player so messages are not displayed at the
            // same time.

            if (cmd->forwardmove > TURBOTHRESHOLD)
            {
                turbodetected[i] = true;
            }

            if ((gametic & 31) == 0 
            && ((gametic >> 5) % MAXPLAYERS) == i
            && turbodetected[i])
            {
                static char turbomessage[80];
                extern char *player_names[4];

                M_snprintf(turbomessage, sizeof(turbomessage),
                               "%s is turbo!", player_names[i]);

                players[consoleplayer].message = turbomessage;
                turbodetected[i] = false;
            }
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
                    S_ResumeSound (); 
                    break; 
					 
                    case BTS_SAVEGAME: 
                    if (!savedescription[0]) 
                    {
                        M_StringCopy(savedescription, "NET GAME", sizeof(savedescription));
                    }

                savegameslot =  
                (players[i].cmd.buttons & BTS_SAVEMASK)>>BTS_SAVESHIFT; 
                gameaction = ga_savegame; 
                break; 
                } 
            } 
        }
    }

    // Have we just finished displaying an intermission screen?

    if (oldgamestate == GS_INTERMISSION && gamestate != GS_INTERMISSION)
    {
        WI_End();
    }

    oldgamestate = gamestate;
    
    // do main actions
    switch (gamestate) 
    { 
        case GS_LEVEL: 
        P_Ticker (); 
        ST_Ticker (); 
        AM_Ticker (); 
        HU_Ticker ();            
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
} 


// =============================================================================
// PLAYER STRUCTURE FUNCTIONS
// also see P_SpawnPlayer in P_Things
// =============================================================================


// -----------------------------------------------------------------------------
// G_InitPlayer 
//
// Called at the start.
// Called by the game initialization functions.
// -----------------------------------------------------------------------------

void G_InitPlayer (int player) 
{
    // clear everything else to defaults
    G_PlayerReborn (player); 
}


// -----------------------------------------------------------------------------
// G_PlayerFinishLevel
//
// Can when a player completes a level.
// -----------------------------------------------------------------------------

void G_PlayerFinishLevel (int player) 
{ 
    player_t *p; 

    p = &players[player]; 

    memset (p->powers, 0, sizeof (p->powers)); 
    memset (p->cards, 0, sizeof (p->cards)); 
    memset (p->tryopen, 0, sizeof (p->tryopen)); // [crispy] blinking key or skull in the status bar
    p->mo->flags &= ~MF_SHADOW;  // cancel invisibility 
    p->extralight = 0;      // cancel gun flashes 
    p->fixedcolormap = 0;   // cancel ir gogles 
    p->damagecount = 0;     // no palette changes 
    p->bonuscount = 0; 
}


// -----------------------------------------------------------------------------
// G_PlayerReborn
// Called after a player dies 
// almost everything is cleared and initialized 
// -----------------------------------------------------------------------------

void G_PlayerReborn (int player) 
{ 
    player_t   *p; 
    int         i; 
    int         frags[MAXPLAYERS]; 
    int         killcount;
    int         itemcount;
    int         secretcount; 

    memcpy (frags,players[player].frags,sizeof(frags)); 
    killcount = players[player].killcount; 
    itemcount = players[player].itemcount; 
    secretcount = players[player].secretcount; 

    p = &players[player]; 
    memset (p, 0, sizeof(*p)); 

    memcpy (players[player].frags, frags, sizeof(players[player].frags)); 
    players[player].killcount = killcount; 
    players[player].itemcount = itemcount; 
    players[player].secretcount = secretcount; 

    p->usedown = p->attackdown = true;  // don't do anything immediately 
    p->playerstate = PST_LIVE;       
    p->health = deh_initial_health;     // Use dehacked value
    p->readyweapon = p->pendingweapon = wp_pistol; 
    p->weaponowned[wp_fist] = true; 
    p->weaponowned[wp_pistol] = true; 
    p->ammo[am_clip] = deh_initial_bullets; 

    for (i=0 ; i<NUMAMMO ; i++) 
	p->maxammo[i] = maxammo[i]; 
}


// -----------------------------------------------------------------------------
// G_CheckSpot  
// Returns false if the player cannot be respawned
// at the given mapthing_t spot  
// because something is occupying it 
// -----------------------------------------------------------------------------

void P_SpawnPlayer (mapthing_t *mthing); 
 
boolean G_CheckSpot (int playernum, mapthing_t *mthing)
{
    int           i;
    fixed_t       x;
    fixed_t       y; 
    subsector_t  *ss; 
    mobj_t       *mo; 

    if (!players[playernum].mo)
    {
        // first spawn of level, before corpses
        for (i=0 ; i<playernum ; i++)
        if (players[i].mo->x == mthing->x << FRACBITS
        && players[i].mo->y == mthing->y << FRACBITS)
            return false;	
        return true;
    }

    x = mthing->x << FRACBITS; 
    y = mthing->y << FRACBITS; 

    if (!P_CheckPosition (players[playernum].mo, x, y) ) 
    return false; 

    // flush an old corpse if needed 
    if (bodyqueslot >= BODYQUESIZE) 
    P_RemoveMobj (bodyque[bodyqueslot%BODYQUESIZE]); 
    bodyque[bodyqueslot%BODYQUESIZE] = players[playernum].mo; 
    bodyqueslot++; 

    // spawn a teleport fog
    ss = R_PointInSubsector (x,y);
    
    {
        fixed_t     xa, ya;
        signed int  an;

        // This calculation overflows in Vanilla Doom, but here we deliberately
        // avoid integer overflow as it is undefined behavior, so the value of
        // 'an' will always be positive.
        an = (ANG45 >> ANGLETOFINESHIFT) * ((signed int) mthing->angle / 45);

        // [Julia] Unh-uh. Let MT_TFOG always to be spawned,
        // in any direction of Deathmatch spawning spot.
        xa = finecosine[an];
        ya = finesine[an];

        mo = P_SpawnMobj(x + 20 * xa, y + 20 * ya, ss->sector->floorheight, MT_TFOG);
    }

    if (players[consoleplayer].viewz != 1) 
    S_StartSound (mo, sfx_telept);	// don't start sound on first frame 

    return true; 
}


// -----------------------------------------------------------------------------
// G_DoReborn 
// -----------------------------------------------------------------------------

void G_DoReborn (int playernum) 
{ 
    // reload the level from scratch
    gameaction = ga_loadlevel;  
} 


// -----------------------------------------------------------------------------
// G_ScreenShot
// -----------------------------------------------------------------------------

void G_ScreenShot (void) 
{ 
    gameaction = ga_screenshot; 
}


// -----------------------------------------------------------------------------
// G_DoCompleted 
// -----------------------------------------------------------------------------

boolean      secretexit; 
extern char *pagename; 


// -----------------------------------------------------------------------------
// G_ExitLevel
// -----------------------------------------------------------------------------

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

    for (i=0 ; i<MAXPLAYERS ; i++) 
        if (playeringame[i]) 
            G_PlayerFinishLevel (i);    // take away cards and stuff 

    if (automapactive) 
    AM_Stop (); 

    wminfo.didsecret = players[consoleplayer].didsecret; 
    wminfo.epsd = gameepisode -1; 
    wminfo.last = gamemap -1;

    // wminfo.next is 0 biased, unlike gamemap
    // [Julia] Handling secret exit
	if (secretexit)
	    switch(gamemap)
	    {
	      // [Julia] Secret exit from Toxin Refinery (04) to Military Base (23)
	      case 3:  wminfo.next = 23;
	      break;
	    }
	else
	    switch(gamemap)
	    {
	      // [Julia] After Military Base go to Command Control
	      case 24: wminfo.next = 3;
	      break;
	      default: wminfo.next = gamemap;
	    }

    wminfo.maxkills = totalkills; 
    wminfo.maxitems = totalitems; 
    wminfo.maxsecret = totalsecret; 
    wminfo.maxfrags = 0; 

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
    viewactive = false; 
    automapactive = false; 

    WI_Start (&wminfo); 
} 


// -----------------------------------------------------------------------------
// G_WorldDone 
// -----------------------------------------------------------------------------

void G_WorldDone (void) 
{ 
    gameaction = ga_worlddone; 

    if (secretexit) 
    players[consoleplayer].didsecret = true; 

    switch (gamemap)
    {
        case 23:
        F_StartFinale ();
        break;
    }
} 


// -----------------------------------------------------------------------------
// G_DoWorldDone
// -----------------------------------------------------------------------------

void G_DoWorldDone (void) 
{        
    gamestate = GS_LEVEL; 
    gamemap = wminfo.next+1; 
    G_DoLoadLevel (); 
    gameaction = ga_nothing; 
    viewactive = true; 
} 


// -----------------------------------------------------------------------------
// G_InitFromSavegame
//
// Can be called by the startup code or the menu task. 
// -----------------------------------------------------------------------------

extern boolean setsizeneeded;
void R_ExecuteSetViewSize (void);
char savename[256];


// -----------------------------------------------------------------------------
// G_LoadGame
// -----------------------------------------------------------------------------

void G_LoadGame (char *name) 
{
    M_StringCopy(savename, name, sizeof(savename));
    gameaction = ga_loadgame; 
} 


// -----------------------------------------------------------------------------
// G_DoLoadGame
// -----------------------------------------------------------------------------

void G_DoLoadGame (void) 
{ 
    int savedleveltime;

    gameaction = ga_nothing; 

    save_stream = fopen(savename, "rb");

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
    P_RestoreTargets ();
 
    if (!P_ReadSaveGameEOF())
    I_Error ("Bad savegame");

    fclose(save_stream);
    
    if (setsizeneeded)
    R_ExecuteSetViewSize ();
    
    // [Julia] Additional HUD feedback: "Game loaded."
    players[consoleplayer].message = DEH_String(GGLOADED);
    
    // draw the pattern into the back screen
    R_FillBackScreen ();   
} 


// -----------------------------------------------------------------------------
// G_SaveGame
//
// Called by the menu task.
// Description is a 24 byte text string 
// -----------------------------------------------------------------------------

void G_SaveGame (int slot, char *description)
{
    savegameslot = slot;
    M_StringCopy(savedescription, description, sizeof(savedescription));
    sendsave = true;
}


// -----------------------------------------------------------------------------
// G_DoSaveGame
// -----------------------------------------------------------------------------

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
    save_stream = fopen(temp_savegame_file, "wb");

    if (save_stream == NULL)
    {
        // Failed to save the game, so we're going to have to abort. But
        // to be nice, save to somewhere else before we call I_Error().
        recovery_savegame_file = M_TempFile("recovery.dsg");
        save_stream = fopen(recovery_savegame_file, "wb");
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

    remove(savegame_file);
    rename(temp_savegame_file, savegame_file);

    gameaction = ga_nothing;
    M_StringCopy(savedescription, "", sizeof(savedescription));

    players[consoleplayer].message = DEH_String(GGSAVED);

    // draw the pattern into the back screen
    R_FillBackScreen ();
}
 

// -----------------------------------------------------------------------------
// G_InitNew
//
// Can be called by the startup code or the menu task,
// consoleplayer, displayplayer, playeringame[] should be set. 
// -----------------------------------------------------------------------------
skill_t d_skill; 
int     d_episode; 
int     d_map; 


// -----------------------------------------------------------------------------
// G_DeferedInitNew
// -----------------------------------------------------------------------------

void G_DeferedInitNew (skill_t skill, int episode, int map) 
{
    d_skill = skill; 
    d_episode = episode; 
    d_map = map; 
    gameaction = ga_newgame; 
} 


// -----------------------------------------------------------------------------
// G_DoNewGame
// -----------------------------------------------------------------------------

void G_DoNewGame (void) 
{
    demoplayback = false; 
    netdemo = false;
    netgame = false;
    deathmatch = false;
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


// -----------------------------------------------------------------------------
// G_InitNew
// -----------------------------------------------------------------------------

void G_InitNew (skill_t skill, int episode, int map)
{
    char  *skytexturename;
    int    i;
    // [crispy] make sure "fast" parameters are really only applied once
    static boolean fast_applied;

    if (paused)
    {
        paused = false;
        S_ResumeSound ();
    }

    if (skill > sk_ultranm)
    skill = sk_ultranm;

    // [Julia] Episode itself is never used, but still necessary
    episode = 1;

    // [Julia] Catch unexisting maps
    if (map < 1)
        map = 1;
    if (map > 24)
        map = 24;

    M_ClearRandom ();

    // [crispy] make sure "fast" parameters are really only applied once
    if ((fastparm || skill == sk_nightmare || skill == sk_ultranm) && !fast_applied)
    {
        for (i=S_SARG_RUN1 ; i<=S_SARG_PAIN2 ; i++)
	    // [crispy] Fix infinite loop caused by Demon speed bug
	    if (states[i].tics > 1)
	    {
	    states[i].tics >>= 1;
	    }

        mobjinfo[MT_BRUISERSHOT].speed = 20*FRACUNIT;
        mobjinfo[MT_HEADSHOT].speed = 20*FRACUNIT;
        mobjinfo[MT_TROOPSHOT].speed = 20*FRACUNIT;

        fast_applied = true;
    }
    else if (!fastparm && skill != sk_nightmare && skill != sk_ultranm && fast_applied)
    {
        for (i=S_SARG_RUN1 ; i<=S_SARG_PAIN2 ; i++)
        states[i].tics <<= 1;
        mobjinfo[MT_BRUISERSHOT].speed = 15*FRACUNIT;
        mobjinfo[MT_HEADSHOT].speed = 10*FRACUNIT;
        mobjinfo[MT_TROOPSHOT].speed = 10*FRACUNIT;
        fast_applied = false;
    }
    
    // [Julia] Ultra Nightmare definitions
    if (skill == sk_ultranm)
    {
        // Monster's speed                           Who (initial value)
        mobjinfo[MT_POSSESSED].speed = 12;           // Zombieman (8)
        mobjinfo[MT_SHOTGUY].speed = 12;             // Shotgun guy (8)
        mobjinfo[MT_TROOP].speed = 13;               // Imp (8)
        mobjinfo[MT_SKULL].speed = 14;               // Lost Soul (8)
        mobjinfo[MT_HEAD].speed = 14;                // Cacodemon (8)
        mobjinfo[MT_BRUISER].speed = 14;             // Baron of Hell (8)

        // Monster's damage                          What (initial value)
        mobjinfo[MT_TROOPSHOT].damage = 4;           // Imp (3)
        mobjinfo[MT_HEADSHOT].damage = 7;            // Cacodemon (5)
        mobjinfo[MT_BRUISERSHOT].damage = 10;        // Knight / Baron (8) 
    }
    // [JN] Fallback to standard values
    else
    {
        // Monster's speed                          Who
        mobjinfo[MT_POSSESSED].speed = 8;           // Zombieman
        mobjinfo[MT_SHOTGUY].speed = 8;             // Shotgun guy
        mobjinfo[MT_TROOP].speed = 8;               // Imp
        mobjinfo[MT_SKULL].speed = 8;               // Lost Soul
        mobjinfo[MT_HEAD].speed = 8;                // Cacodemon
        mobjinfo[MT_BRUISER].speed = 8;             // Baron of Hell

        // Monster's damage                         What
        mobjinfo[MT_TROOPSHOT].damage = 3;          // Imp fireball
        mobjinfo[MT_HEADSHOT].damage = 5;           // Cacodemon fireball
        mobjinfo[MT_BRUISERSHOT].damage = 8;        // Knight / Baron fireball 
    }

    // force players to be initialized upon first level load
    for (i=0 ; i<MAXPLAYERS ; i++)
    players[i].playerstate = PST_REBORN;

    usergame = true;                // will be set false if a demo
    paused = false;
    demoplayback = false;
    automapactive = false;
    viewactive = true;
    gameepisode = episode;
    gamemap = map;
    gameskill = skill;

    // [crispy] CPhipps - total time for all completed levels
    totalleveltimes = 0;

    viewactive = true;

    // [Julia] Set the Jaguar sky to use
    if (gamemap < 9 || gamemap == 24)
    skytexturename = "SKY1";
    else if (gamemap < 17)
    skytexturename = "SKY2";
    else
    skytexturename = "SKY3";

    skytexture = R_TextureNumForName(skytexturename);

    G_DoLoadLevel ();
}


// =============================================================================
// DEMO RECORDING 
// =============================================================================

#define DEMOMARKER  0x80


// -----------------------------------------------------------------------------
// G_ReadDemoTiccmd
// -----------------------------------------------------------------------------

void G_ReadDemoTiccmd (ticcmd_t *cmd) 
{ 
    if (*demo_p == DEMOMARKER) 
    {
        // end of demo data stream 
        G_CheckDemoStatus (); 
        return; 
    }

    cmd->forwardmove = ((signed char)*demo_p++); 
    cmd->sidemove = ((signed char)*demo_p++); 
    cmd->angleturn = ((unsigned char) *demo_p++)<<8; 
    cmd->buttons = (unsigned char)*demo_p++; 

    if (flip_levels_cmdline)
    {
        cmd->sidemove *= (const signed char) -1;
        cmd->angleturn *= (const short) -1;
    }
} 


// -----------------------------------------------------------------------------
// IncreaseDemoBuffer
//
// Increase the size of the demo buffer to allow unlimited demos
// -----------------------------------------------------------------------------

static void IncreaseDemoBuffer(void)
{
    int     current_length;
    byte    *new_demobuffer;
    byte    *new_demop;
    int     new_length;

    // Find the current size

    current_length = demoend - demobuffer;
    
    // Generate a new buffer twice the size
    new_length = current_length * 2;
    
    new_demobuffer = Z_Malloc(new_length, PU_STATIC, 0);
    new_demop = new_demobuffer + (demo_p - demobuffer);

    // Copy over the old data

    memcpy(new_demobuffer, demobuffer, current_length);

    // Free the old buffer and point the demo pointers at the new buffer.

    Z_Free(demobuffer);

    demobuffer = new_demobuffer;
    demo_p = new_demop;
    demoend = demobuffer + new_length;
}


// -----------------------------------------------------------------------------
// G_WriteDemoTiccmd
// -----------------------------------------------------------------------------

void G_WriteDemoTiccmd (ticcmd_t *cmd) 
{ 
    byte *demo_start;

    if (flip_levels_cmdline)
    {
        cmd->sidemove *= (const signed char) -1;
        cmd->angleturn *= (const short) -1;
    }

    if (gamekeydown[key_demo_quit]) // press q to end demo recording 
    G_CheckDemoStatus (); 

    demo_start = demo_p;

    *demo_p++ = cmd->forwardmove; 
    *demo_p++ = cmd->sidemove; 
    *demo_p++ = cmd->angleturn >> 8; 
    *demo_p++ = cmd->buttons; 

    // reset demo pointer back
    demo_p = demo_start;

    // [Julia] No limits for demo recording
    if (demo_p > demoend - 16)
    {
        IncreaseDemoBuffer();
    }
	
    G_ReadDemoTiccmd (cmd); // make SURE it is exactly the same 
}


// -----------------------------------------------------------------------------
// G_RecordDemo
// -----------------------------------------------------------------------------

void G_RecordDemo (char *name)
{
    size_t  demoname_size;
    int     maxsize;

    usergame = false;
    demoname_size = strlen(name) + 5;
    demoname = Z_Malloc(demoname_size, PU_STATIC, NULL);
    M_snprintf(demoname, demoname_size, "%s.lmp", name);
    maxsize = 0x20000;
    demobuffer = Z_Malloc (maxsize,PU_STATIC,NULL); 
    demoend = demobuffer + maxsize;

    demorecording = true; 
} 


// -----------------------------------------------------------------------------
// G_VanillaVersionCode
//
// Get the demo version code appropriate for the version set in gameversion.
// -----------------------------------------------------------------------------

int G_VanillaVersionCode(void)
{
    switch (gameversion)
    {
        // [Julia] TODO - cleanup, use only 1.9
        case exe_doom_1_2:
        I_Error("Doom 1.2 does not have a version code!");

        case exe_doom_1_666:
        return 106;

        case exe_doom_1_7:
        return 107;
        
        case exe_doom_1_8:
        return 108;

        case exe_doom_1_9:
        default:  // All other versions are variants on v1.9:
        return 109;
    }
}


// -----------------------------------------------------------------------------
// G_BeginRecording
// -----------------------------------------------------------------------------

void G_BeginRecording (void) 
{ 
    int i;

    demo_p = demobuffer;

    *demo_p++ = G_VanillaVersionCode();
    *demo_p++ = gameskill; 
    *demo_p++ = gameepisode; 
    *demo_p++ = gamemap; 
    *demo_p++ = deathmatch; 
    *demo_p++ = respawnparm;
    *demo_p++ = fastparm;
    *demo_p++ = nomonsters;
    *demo_p++ = consoleplayer;

    for (i=0 ; i<MAXPLAYERS ; i++) 
    *demo_p++ = playeringame[i]; 		 
} 


// -----------------------------------------------------------------------------
// G_PlayDemo 
// -----------------------------------------------------------------------------

char *defdemoname; 

void G_DeferedPlayDemo (char* name) 
{ 
    defdemoname = name; 
    gameaction = ga_playdemo;
}


// -----------------------------------------------------------------------------
// G_DoPlayDemo
// -----------------------------------------------------------------------------

void G_DoPlayDemo (void)
{
    skill_t skill;
    int     i, lumpnum, episode, map;

    lumpnum = W_GetNumForName(defdemoname);
    gameaction = ga_nothing;
    demobuffer = W_CacheLumpNum(lumpnum, PU_STATIC);
    demo_p = demobuffer;

    skill = *demo_p++; 
    episode = *demo_p++; 
    map = *demo_p++; 
    deathmatch = *demo_p++;
    respawnparm = *demo_p++;
    fastparm = *demo_p++;
    nomonsters = *demo_p++;
    consoleplayer = *demo_p++;

    for (i=0 ; i<MAXPLAYERS ; i++) 
	playeringame[i] = *demo_p++; 

    if (playeringame[1] || M_CheckParm("-solo-net") > 0 || M_CheckParm("-netdemo") > 0)
    {
        netgame = true;
        netdemo = true;
    }

    // don't spend a lot of time in loadlevel 
    precache = false;
    G_InitNew (skill, episode, map); 
    precache = true; 
    starttime = I_GetTime (); 

    usergame = false; 
    demoplayback = true; 
} 


// -----------------------------------------------------------------------------
// G_TimeDemo 
// -----------------------------------------------------------------------------

void G_TimeDemo (char *name) 
{
    timingdemo  = true; 
    singletics  = true; 
    defdemoname = name; 
    gameaction  = ga_playdemo; 
} 


// -----------------------------------------------------------------------------
// G_CheckDemoStatus 
//
// Called after a death or level completion to allow demos to be cleaned up 
// Returns true if a new demo loop action will take place 
// -----------------------------------------------------------------------------

boolean G_CheckDemoStatus (void) 
{ 
    int endtime; 

    if (timingdemo) 
    { 
        float fps;
        int   realtics;

        endtime = I_GetTime (); 
        realtics = endtime - starttime;
        fps = ((float) gametic * TICRATE) / realtics;

        // Prevent recursive calls
        timingdemo = false;
        demoplayback = false;
        I_Error ("timed %i gametics in %i realtics (%f fps)",
                 gametic, realtics, fps);
    } 

    if (demoplayback)
    {
        W_ReleaseLumpName(defdemoname);
        demoplayback = false; 
        netdemo = false;
        netgame = false;
        deathmatch = false;
        playeringame[1] = playeringame[2] = playeringame[3] = 0;
        respawnparm = false;
        fastparm = false;
        nomonsters = false;
        consoleplayer = 0;

        if (singledemo) 
            I_Quit (); 
        else 
            D_AdvanceDemo (); 

        return true; 
    } 

    if (demorecording) 
    { 
        *demo_p++ = DEMOMARKER; 

        M_WriteFile (demoname, demobuffer, demo_p - demobuffer); 
        Z_Free (demobuffer); 
        demorecording = false; 
        I_Error ("Demo %s recorded", demoname); 
    } 

    return false; 
}

