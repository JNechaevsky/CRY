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

#include "i_swap.h"
#include "i_system.h"
#include "i_video.h"
#include "z_zone.h"
#include "m_misc.h"
#include "m_random.h"
#include "w_wad.h"
#include "deh_main.h"
#include "deh_misc.h"
#include "doomdef.h"
#include "doomkeys.h"
#include "g_game.h"
#include "st_stuff.h"
#include "st_lib.h"
#include "r_local.h"
#include "p_local.h"
#include "p_inter.h"
#include "am_map.h"
#include "m_cheat.h"
#include "m_menu.h"
#include "s_sound.h"
#include "v_video.h"
#include "doomstat.h"
#include "sounds.h"
#include "d_englsh.h"
#include "jn.h"


// =============================================================================
// STATUS BAR DATA
// =============================================================================


// Palette indices for damage/bonus red-/gold-shifts
#define STARTREDPALS        1
#define STARTBONUSPALS      9
#define NUMREDPALS          8
#define NUMBONUSPALS        4

#define RADIATIONPAL        13  // Radiation suit, green shift
#define INVULNERABILITYPAL  14  // [Julia] Cyan invulnerability palette

#define ST_X                0   // Location of status bar

// Number of status faces.
#define ST_NUMPAINFACES     5
#define ST_NUMSTRAIGHTFACES 3
#define ST_NUMTURNFACES     2
#define ST_NUMSPECIALFACES  3

#define ST_FACESTRIDE (ST_NUMSTRAIGHTFACES+ST_NUMTURNFACES+ST_NUMSPECIALFACES)

#define ST_NUMEXTRAFACES    2

// [Julia] Additional faces (Jaguar: +6 (exploded), PSX: +1 (squished))
#define ST_NUMFACES (ST_FACESTRIDE*ST_NUMPAINFACES+ST_NUMEXTRAFACES+7)

#define ST_TURNOFFSET       (ST_NUMSTRAIGHTFACES)
#define ST_OUCHOFFSET       (ST_TURNOFFSET + ST_NUMTURNFACES)
#define ST_EVILGRINOFFSET   (ST_OUCHOFFSET + 1)
#define ST_RAMPAGEOFFSET    (ST_EVILGRINOFFSET + 1)
#define ST_GODFACE          (ST_NUMPAINFACES*ST_FACESTRIDE)
#define ST_DEADFACE         (ST_GODFACE+1)
#define ST_EXPLFACE0        (ST_DEADFACE+1)
#define ST_EXPLFACE1        (ST_EXPLFACE0+1)
#define ST_EXPLFACE2        (ST_EXPLFACE1+1)
#define ST_EXPLFACE3        (ST_EXPLFACE2+1)
#define ST_EXPLFACE4        (ST_EXPLFACE3+1)
#define ST_EXPLFACE5        (ST_EXPLFACE4+1)
#define ST_CRSHFACE0        (ST_EXPLFACE5+1)

#define ST_FACESX           143
#define ST_FACESY           166

#define ST_EVILGRINCOUNT        (2*TICRATE)
#define ST_STRAIGHTFACECOUNT    (TICRATE/2)
#define ST_TURNCOUNT            (1*TICRATE)
#define ST_OUCHCOUNT            (1*TICRATE)
#define ST_RAMPAGEDELAY         (2*TICRATE)
#define ST_MUCHPAIN         20


// AMMO number pos.
// [Julia] Jaguar status bar: realigned
#define ST_AMMOWIDTH    3	
#define ST_AMMOX        51
#define ST_AMMOY        174

// HEALTH number pos.
// [Julia] Jaguar status bar: realigned
#define ST_HEALTHX      104
#define ST_HEALTHY      174

// Weapon pos.
// [Julia] Jaguar status bar: realigned
#define ST_ARMSX        249
#define ST_ARMSY        175
#define ST_ARMSXSPACE   12
#define ST_ARMSYSPACE   10

// ARMOR number pos.
// [Julia] Jaguar status bar: realigned
#define ST_ARMORX       225
#define ST_ARMORY       174

// Key icon positions.
// [Julia] Jaguar status bar: realigned, using Jaguar order
// KEY0 - blue
// KEY1 - yellow
// KEY2 - red
#define ST_KEYX         124 // Always same
#define ST_KEY0Y        175 // Blue
#define ST_KEY1Y        187 // Yellow
#define ST_KEY2Y        163 // Red

// [Julia] Jaguar: MAP number pos.
#define ST_MAPWIDTH_X1  1
#define ST_MAPWIDTH_X2  2
#define ST_MAP_X1       309
#define ST_MAP_X2       317
#define ST_MAPY         174

// Dimensions given in characters
#define ST_MSGWIDTH     52
#define ST_MAPTITLEX    (ORIGWIDTH - ST_MAPWIDTH * ST_CHATFONTWIDTH)


// graphics are drawn to a backing screen and blitted to the real screen
byte  *st_backing_screen;

static player_t *plyr;          // main player in game
static boolean st_firsttime;    // ST_Start() has just been called
static int lu_palette;          // lump number for PLAYPAL
static unsigned int	st_clock;   // used for timing
static int st_msgcounter=0;     // used for making messages go away

static st_chatstateenum_t  st_chatstate;    // used when in chat
static st_stateenum_t      st_gamestate;    // whether in automap or first-person

static boolean st_statusbaron;   // whether left-side main status bar is active
static boolean st_chat;          // whether status bar chat is active
static boolean st_oldchat;       // value of st_chat before message popped up
static boolean st_cursoron;      // whether chat window has the cursor on
static boolean st_notdeathmatch; // !deathmatch
static boolean st_armson;        // !deathmatch && st_statusbaron

static patch_t *sbar;                   // main bar left
static patch_t *tallnum[10];            // 0-9, tall numbers
static patch_t *tallpercent;            // tall % sign
static patch_t *shortnum[10];           // 0-9, short, yellow (,different!) numbers
static patch_t *keys[NUMCARDS];         // 3 key-cards, 3 skulls
static patch_t *faces[ST_NUMFACES * 2]; // face status patches (Julia - doubled)
static patch_t *arms[6][2];             // weapon ownership patches

static st_number_t w_ready;         // ready-weapon widget
static st_number_t w_currentmap;    // [Julia] current map widfet

static st_percent_t  w_health;      // health widget
static st_multicon_t w_arms[6];     // weapon ownership widgets
static st_multicon_t w_faces;       // face status widget
static st_multicon_t w_keyboxes[3]; // keycard widgets
static st_percent_t  w_armor;       // armor widget

static boolean oldweaponsowned[NUMWEAPONS]; // used for evil grin
static int st_oldhealth = -1;   // used to use appopriately pained face
static int st_facecount = 0;    // count until face changes
static int st_faceindex = 0;    // current face index, used by w_faces
static int keyboxes[3];         // holds key-type for each key box on bar
int st_keyorskull[3];           // [crispy] blinking key or skull in the status bar
static int st_randomnumber;     // a random number per tick

cheatseq_t cheat_mus               = CHEAT("idmus",  2);
cheatseq_t cheat_god               = CHEAT("iddqd",  0);
cheatseq_t cheat_ammo              = CHEAT("idkfa",  0);
cheatseq_t cheat_ammonokey         = CHEAT("idfa",   0);
cheatseq_t cheat_keys              = CHEAT("idka",   0);
cheatseq_t cheat_noclip            = CHEAT("idclip", 0);

cheatseq_t cheat_powerup[5] =
{
    CHEAT("idbeholdv", 0),
    CHEAT("idbeholds", 0),
    CHEAT("idbeholdr", 0),
    CHEAT("idbeholda", 0),
    CHEAT("idbehold",  0),
};

cheatseq_t cheat_choppers = CHEAT("idchoppers", 0);
cheatseq_t cheat_clev     = CHEAT("idclev",     2);
cheatseq_t cheat_mypos    = CHEAT("idmypos",    0);
cheatseq_t cheat_massacre = CHEAT("tntem",      0);


// =============================================================================
// STATUS BAR CODE
// =============================================================================

void ST_Stop(void);

// -----------------------------------------------------------------------------
// ST_refreshBackground
// -----------------------------------------------------------------------------

void ST_refreshBackground(void)
{
    if (screenblocks >= 11 && !automapactive)
    return;

    if (st_statusbaron)
    {
        V_UseBuffer(st_backing_screen);
        V_DrawPatch(ST_X, 0, sbar);

        V_RestoreBuffer();

        V_CopyRect(ST_X, 0, st_backing_screen, ST_WIDTH, ST_HEIGHT, ST_X, ST_Y);
    }
}


// -----------------------------------------------------------------------------
// ST_cheat_massacre
// [crispy] adapted from boom202s/M_CHEAT.C:467-498
// -----------------------------------------------------------------------------
static int ST_cheat_massacre()
{
    int killcount = 0;
    thinker_t *th;
    extern void A_PainDie(mobj_t *);

    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function.acp1 == (actionf_p1)P_MobjThinker)
        {
            mobj_t *mo = (mobj_t *)th;

            if (mo->flags & MF_COUNTKILL || mo->type == MT_SKULL)
            {
                if (mo->health > 0)
                {
                    P_DamageMobj(mo, NULL, NULL, 10000);
                    killcount++;
                }
            }
        }
    }

    return killcount;
} 


// -----------------------------------------------------------------------------
// ST_Responder
// Respond to keyboard input events, intercept cheats.
// -----------------------------------------------------------------------------

boolean ST_Responder (event_t* ev)
{
    int i;

    // Filter automap on/off.
    if (ev->type == ev_keyup && ((ev->data1 & 0xffff0000) == AM_MSGHEADER))
    {
        switch(ev->data1)
        {
            case AM_MSGENTERED:
                st_gamestate = AutomapState;
                st_firsttime = true;
            break;

            case AM_MSGEXITED:
                st_gamestate = FirstPersonState;
            break;
        }
    }

    // if a user keypress...
    else if (ev->type == ev_keydown)
    {
        // [Julia] Dead player can't use cheats
        if (plyr->mo->health >= 1)
        {
            // 'dqd' cheat for toggleable god mode
            if (cht_CheckCheat(&cheat_god, ev->data2))
            {
                plyr->cheats ^= CF_GODMODE;

                // [Julia] No god mode healing in Jaguar
                if (plyr->cheats & CF_GODMODE)
                plyr->message = DEH_String(STSTR_DQDON);
                else 
                plyr->message = DEH_String(STSTR_DQDOFF);
            }

            // 'fa' cheat for killer fucking arsenal
            else if (cht_CheckCheat(&cheat_ammonokey, ev->data2))
            {
                plyr->armorpoints = deh_idfa_armor;
                plyr->armortype = deh_idfa_armor_class;

                for (i=0;i<NUMWEAPONS;i++)
                plyr->weaponowned[i] = true;

                // [Julia] Jaguar: happy 500 ammo for everything!
                for (i=0;i<NUMAMMO;i++)
                plyr->ammo[i] = plyr->maxammo[i] = 500;

                plyr->message = DEH_String(STSTR_FAADDED);
            }

            // 'kfa' cheat for key full ammo
            else if (cht_CheckCheat(&cheat_ammo, ev->data2))
            {
                plyr->armorpoints = deh_idkfa_armor;
                plyr->armortype = deh_idkfa_armor_class;

                for (i=0;i<NUMWEAPONS;i++)
                plyr->weaponowned[i] = true;

                // [Julia] Jaguar: happy 500 ammo for everything!
                for (i=0;i<NUMAMMO;i++)
                plyr->ammo[i] = plyr->maxammo[i] = 500;

                for (i=0;i<NUMCARDS;i++)
                plyr->cards[i] = true;

                plyr->message = DEH_String(STSTR_KFAADDED);
            }

            // [Julia] 'ka' cheat for keys only
            else if (cht_CheckCheat(&cheat_keys, ev->data2))
            {
                for (i=0;i<NUMCARDS;i++)
                plyr->cards[i] = true;

                plyr->message = DEH_String(STSTR_KAADDED);
            }

            // [crispy] implement Boom's "tntem" cheat
            else if (cht_CheckCheat(&cheat_massacre, ev->data2))
            {
                ST_cheat_massacre();
                plyr->message = DEH_String(STSTR_MASSACRE);
            }

            // 'mus' cheat for changing music
            else if (cht_CheckCheat(&cheat_mus, ev->data2))
            {
                char  buf[3];
                int   musnum;

                plyr->message = DEH_String(STSTR_MUS);
                cht_GetParam(&cheat_mus, buf);

                // [Julia] Jaguar music handling
                musnum = mus_map01 + (buf[0]-'0')*10 + buf[1]-'0' - 1;
	  
                if (((buf[0]-'0')*10 + buf[1]-'0') > 28)
                plyr->message = DEH_String(STSTR_NOMUS);
                else
                S_ChangeMusic(musnum, 1);
            }

            // [Julia] Noclip cheat
            else if (cht_CheckCheat(&cheat_noclip, ev->data2))
            {
                plyr->cheats ^= CF_NOCLIP;

                if (plyr->cheats & CF_NOCLIP)
                plyr->message = DEH_String(STSTR_NCON);
                else
                plyr->message = DEH_String(STSTR_NCOFF);
            }

            // 'behold?' power-up cheats
            // [Julia] "i"nvisibility and "l"ight visor aren't offered
            for (i=0 ; i<4 ; i++)
            {
                if (cht_CheckCheat(&cheat_powerup[i], ev->data2))
                {
                    if (!plyr->powers[i])
                    {
                        P_GivePower(plyr, i);   // Activated
                        plyr->message = DEH_String(STSTR_BEHOLDX);
                    }
                    else if (i!=pw_strength)
                    {
                        plyr->powers[i] = 1;    // Deactivated
                        plyr->message = DEH_String(STSTR_BEHOLDZ);
                    }
                    else
                    {
                        plyr->powers[i] = 0;    // Deactivated
                        plyr->message = DEH_String(STSTR_BEHOLDZ);
                    }
                }
            }

            // 'behold' power-up menu
            if (cht_CheckCheat(&cheat_powerup[4], ev->data2))
            {
                plyr->message = DEH_String(STSTR_BEHOLD);
            }

            // 'choppers' invulnerability & chainsaw
            else if (cht_CheckCheat(&cheat_choppers, ev->data2))
            {
                plyr->weaponowned[wp_chainsaw] = true;
                plyr->message = DEH_String(STSTR_CHOPPERS);
            }

            // 'mypos' for player position
            else if (cht_CheckCheat(&cheat_mypos, ev->data2))
            {
                static char buf[ST_MSGWIDTH];

                M_snprintf(buf, sizeof(buf), "ang=0x%x;x,y=(0x%x,0x%x)",
                    players[consoleplayer].mo->angle,
                    players[consoleplayer].mo->x,
                    players[consoleplayer].mo->y);

                plyr->message = buf;
            }
        }

        // 'clev' change-level cheat
        if (cht_CheckCheat(&cheat_clev, ev->data2))
        {
            char    buf[3];
            int     epsd;
            int     map;

            cht_GetParam(&cheat_clev, buf);

            epsd = 1;
            map = (buf[0] - '0')*10 + buf[1] - '0';

            // Catch invalid maps.
            if (map < 1)
            {
                return false;
            }
            // [Julia] Don't warp to map 25 and higher
            if (map > 24)
            {
                return false;
            }

            // So be it.
            plyr->message = DEH_String(STSTR_CLEV);
            G_DeferedInitNew(gameskill, epsd, map);
        }
    }

    return false;
}


// -----------------------------------------------------------------------------
// ST_calcPainOffset
// -----------------------------------------------------------------------------

int ST_calcPainOffset(void)
{
    int         health;
    static int  lastcalc;
    static int  oldhealth = -1;

    health = plyr->health > 100 ? 100 : plyr->health;

    if (health != oldhealth)
    {
        lastcalc = ST_FACESTRIDE * (((100 - health) * ST_NUMPAINFACES) / 101);
        oldhealth = health;
    }

    return lastcalc;
}


// -----------------------------------------------------------------------------
// ST_updateFaceWidget
//
// This is a not-very-pretty routine which handles
//  the face states and their timing.
// the precedence of expressions is:
//  dead > evil grin > turned head > straight ahead
//
// [Julia] OUCH face fixes added, thank Fabian Greffrath and Brad Harding!
// -----------------------------------------------------------------------------

void ST_updateFaceWidget(void)
{
    int         i;
    angle_t     badguyangle;
    angle_t     diffang;
    static int  lastattackdown = -1;
    static int  priority = 0;
    boolean     doevilgrin;

    int         painoffset;
    static int  faceindex;

    painoffset = ST_calcPainOffset();

    if (priority < 10)
    {
        // dead
        // [crispy] negative player health
        if (plyr->health <= 0)
        {
            priority = 9;
            painoffset = 0;
            faceindex = ST_DEADFACE;
            st_facecount = 1;
        }
    }

    if (priority < 9)
    {
        if (plyr->bonuscount)
        {
            // picking up bonus
            doevilgrin = false;

            for (i=0;i<NUMWEAPONS;i++)
            {
                if (oldweaponsowned[i] != plyr->weaponowned[i])
                {
                    doevilgrin = true;
                    oldweaponsowned[i] = plyr->weaponowned[i];
                }
            }
            if (doevilgrin)
            {
                // evil grin if just picked up weapon
                priority = 8;
                st_facecount = ST_EVILGRINCOUNT;
                faceindex = ST_EVILGRINOFFSET;
            }
        }
    }

    if (priority < 8)
    {
        if (plyr->damagecount && plyr->attacker && plyr->attacker != plyr->mo)
        {
            // being attacked
            priority = 7;
	    
            // [crispy] show "Ouch Face" as intended
            if (st_oldhealth - plyr->health > ST_MUCHPAIN)
            {
                priority = 8;   // [BH] keep ouch-face visible
                st_facecount = ST_TURNCOUNT;
                faceindex = ST_OUCHOFFSET;
            }
            else
            {
                badguyangle = R_PointToAngle2(plyr->mo->x,
                                plyr->mo->y,
                                plyr->attacker->x,
                                plyr->attacker->y);
		
                if (badguyangle > plyr->mo->angle)
                {
                    // whether right or left
                    diffang = badguyangle - plyr->mo->angle;
                    i = diffang > ANG180; 
                }
                else
                {
                    // whether left or right
                    diffang = plyr->mo->angle - badguyangle;
                    i = diffang <= ANG180; 
                } // confusing, aint it?

                st_facecount = ST_TURNCOUNT;

                if (diffang < ANG45)
                {
                    // head-on    
                    faceindex = ST_RAMPAGEOFFSET;
                }
                else if (i)
                {
                    // turn face right
                    faceindex = ST_TURNOFFSET;
                }
                else
                {
                    // turn face left
                    faceindex = ST_TURNOFFSET+1;
                }
            }
        }
    }

    if (priority < 7)
    {
        // getting hurt because of your own damn stupidity
        if (plyr->damagecount)
        {
            // [crispy] show "Ouch Face" as intended
            if (st_oldhealth - plyr->health > ST_MUCHPAIN)
            {
                priority = 7;
                st_facecount = ST_TURNCOUNT;
                faceindex = ST_OUCHOFFSET;
            }
            else
            {
                priority = 6;
                st_facecount = ST_TURNCOUNT;
                faceindex = ST_RAMPAGEOFFSET;
            }
        }
    }

    if (priority < 6)
    {
        // rapid firing
        if (plyr->attackdown)
        {
            if (lastattackdown==-1)
            {
                lastattackdown = ST_RAMPAGEDELAY;
            }
            else if (!--lastattackdown)
            {
                priority = 5;
                faceindex = ST_RAMPAGEOFFSET;
                st_facecount = 1;
                lastattackdown = 1;
            }
        }
        else
        {
            lastattackdown = -1;
        }
    }

    if (priority < 5)
    {
        // invulnerability
        if ((plyr->cheats & CF_GODMODE) || plyr->powers[pw_invulnerability])
        {
            priority = 4;
            faceindex = ST_GODFACE;
            st_facecount = 0;   // [Julia] Activate extra bloody god mode faces
        }
    }

    // look left or look right if the facecount has timed out
    if (!st_facecount)
    {
        faceindex = st_randomnumber % 3;
        st_facecount = ST_STRAIGHTFACECOUNT;
        priority = 0;
    }

    st_facecount--;

    // [crispy] fix status bar face hysteresis
    st_faceindex = painoffset + faceindex;

    // [Julia] Activate additional cycle in god mode
    if ((plyr->powers[pw_invulnerability]) || (plyr->cheats & CF_GODMODE))
    {
        st_faceindex = painoffset + faceindex + ST_NUMFACES;
    }
}


// -----------------------------------------------------------------------------
// ST_updateWidgets
// -----------------------------------------------------------------------------

void ST_updateWidgets(void)
{
    static int largeammo = 1994; // means "n/a"
    int        i;

    // must redirect the pointer if the ready weapon has changed.
    if (weaponinfo[plyr->readyweapon].ammo == am_noammo)
    w_ready.num = &largeammo;
    else
    w_ready.num = &plyr->ammo[weaponinfo[plyr->readyweapon].ammo];

    w_ready.data = plyr->readyweapon;
    
    // [Julia] Current map
    w_currentmap.data = gamemap;

    // update keycard multiple widgets
    for (i=0;i<3;i++)
    {
        keyboxes[i] = plyr->cards[i] ? i : -1;

        if (plyr->cards[i+3])
        keyboxes[i] = i+3;

        // [crispy] blinking key or skull in the status bar
        // [Julia] blink in any HUD size, except full screen (no HUD)
        if (plyr->tryopen[i])
        {
            if (!(plyr->tryopen[i] & (2*KEYBLINKMASK-1)))
            {
                S_StartSound(NULL, sfx_itemup);
            }

            if (screenblocks < 14 && !(plyr->tryopen[i] & (KEYBLINKMASK-1)))
            {
                st_firsttime = true;
            }
            
            plyr->tryopen[i]--;

            if (screenblocks < 14)
            {
                keyboxes[i] = (plyr->tryopen[i] & KEYBLINKMASK) ? i + st_keyorskull[i] : -1;
            }
        }
    }

    // refresh everything if this is him coming back to life
    ST_updateFaceWidget();

    // used by the w_armsbg widget
    st_notdeathmatch = !deathmatch;

    // used by w_arms[] widgets
    st_armson = st_statusbaron && !deathmatch; 

    // get rid of chat window if up because of message
    if (!--st_msgcounter)
    st_chat = st_oldchat;
}


// -----------------------------------------------------------------------------
// ST_Ticker
// -----------------------------------------------------------------------------

void ST_Ticker (void)
{
    st_clock++;
    st_randomnumber = M_Random();
    ST_updateWidgets();
    st_oldhealth = plyr->health;
}


// -----------------------------------------------------------------------------
// ST_doPaletteStuff
// -----------------------------------------------------------------------------

int st_palette = 0;

void ST_doPaletteStuff (void)
{
    int    palette;
    byte  *pal;
    int    cnt;
    int    bzc;

    cnt = plyr->damagecount;

    if (plyr->powers[pw_strength])
    {
        // slowly fade the berzerk out
        bzc = 12 - (plyr->powers[pw_strength]>>6);

        if (bzc > cnt)
        cnt = bzc;
    }

    if (cnt)
    {
        palette = (cnt+7)>>3;

        if (palette >= NUMREDPALS)
        palette = NUMREDPALS-1;

        palette += STARTREDPALS;
    }

    else if (plyr->bonuscount)
    {
        palette = (plyr->bonuscount+7)>>3;
        
        if (palette >= NUMBONUSPALS)
        palette = NUMBONUSPALS-1;
        palette += STARTBONUSPALS;
    }

    // [Julia] Don't replace CYAN palette with GREEN palette
    else if ( plyr->powers[pw_ironfeet] > 4*32 || plyr->powers[pw_ironfeet]&8)
    {
        palette = RADIATIONPAL;
    }
    else
    {
        palette = 0;
    }
    
    // [Julia] Use CYAN invulnerability palette for invulnerability,
    // unbreakable by other palettes
    if ((plyr->powers[pw_invulnerability] > 4*32)
    ||  (plyr->powers[pw_invulnerability]&8))
    {
        palette = INVULNERABILITYPAL;
    }

    if (palette != st_palette)
    {
        st_palette = palette;
        pal = (byte *) W_CacheLumpNum ((lu_palette), PU_CACHE) + palette * 768;
        I_SetPalette (pal);
    }
}


// -----------------------------------------------------------------------------
// ST_drawWidgets
// -----------------------------------------------------------------------------

void ST_drawWidgets (boolean refresh)
{
    int     i;

    // used by w_arms[] widgets
    st_armson = st_statusbaron && !deathmatch;

    STlib_updateNum(&w_ready, refresh);

    // [crispy] draw "special widgets" in the Crispy HUD
    if ((screenblocks == 11 || screenblocks == 12 || screenblocks == 13) 
    && !automapactive)
    {
        // [crispy] draw berserk pack instead of no ammo if appropriate
        if (plyr->readyweapon == wp_fist && plyr->powers[pw_strength])
        {
            static patch_t *patch;

            if (!patch)
            {
                const int lump = W_CheckNumForName(DEH_String("PSTRA0"));

                if (lump >= 0)
                patch = W_CacheLumpNum(lump, PU_STATIC);
            }

            if (patch)
            {
                // [crispy] (23,179) is the center of the Ammo widget
                V_DrawPatch(23 - SHORT(patch->width)/2 + SHORT(patch->leftoffset),
                            179 - SHORT(patch->height)/2 + SHORT(patch->topoffset),
                            patch);
            }
        }
    }

    // [Julia] Current map widget
    STlib_updateNum(&w_currentmap, refresh);

    // [Julia] Signed Crispy HUD: no STBAR backbround, with player's face/background
    if (screenblocks == 11 && !automapactive)
    {
        V_DrawPatch(0, 0, W_CacheLumpName(DEH_String("STPBG"), PU_CACHE));
    }
    
    // [Julia] Signed Crispy HUD: no STBAR backbround, without player's 
    // face/background. Also don't draw signs in automap.
    if ((screenblocks == 11 || screenblocks == 12) && !automapactive)
    {
        // [JN] Don't draw ammo for fist and chainsaw
        if (plyr->readyweapon != wp_fist && plyr->readyweapon != wp_chainsaw)
        V_DrawPatch(0, 0, W_CacheLumpName(DEH_String("STCHAMMO"), PU_CACHE));

        // [JN] Health, armor, ammo
        V_DrawPatch(0, 0, W_CacheLumpName(DEH_String("STCHNAMS"), PU_CACHE));
    }

    // Health widget
    STlib_updatePercent(&w_health, refresh || screenblocks == 11 || screenblocks == 12 || screenblocks == 13);

    // Armor widget
    STlib_updatePercent(&w_armor, refresh || screenblocks == 11 || screenblocks == 12 || screenblocks == 13);

    // ARMS widget
    for (i=0;i<6;i++)
    STlib_updateMultIcon(&w_arms[i], refresh || screenblocks == 11 || screenblocks == 12 || screenblocks == 13);

    // [Julia] Faces widet (don't draw in Traditional Crispy HUD / full screen)
    if (screenblocks < 12 || automapactive)
    STlib_updateMultIcon(&w_faces, refresh || screenblocks == 11);

    // Key boxes widget
    for (i=0;i<3;i++)
    STlib_updateMultIcon(&w_keyboxes[i], refresh || screenblocks == 11 || screenblocks == 12 || screenblocks == 13);
}


// -----------------------------------------------------------------------------
// ST_doRefresh
// -----------------------------------------------------------------------------

void ST_doRefresh(void)
{
    st_firsttime = false;

    // draw status bar background to off-screen buff
    ST_refreshBackground();

    // and refresh all widgets
    ST_drawWidgets(true);
}


// -----------------------------------------------------------------------------
// ST_diffDraw
// -----------------------------------------------------------------------------

void ST_diffDraw(void)
{
    // update all widgets
    ST_drawWidgets(false);
}


void ST_Drawer (boolean fullscreen, boolean refresh)
{
    // [Julia] Also redraw whole status bar while in HELP screens.
    // Fixes a notable delay of HUD redraw after closing HELP screen.
    
    st_statusbaron = (!fullscreen) || automapactive || screenblocks == 11 || screenblocks == 12;
    st_firsttime = st_firsttime || refresh || inhelpscreens;

    // Do red-/gold-shifts from damage/items
    ST_doPaletteStuff();

    // [Julia] Doh. Always do a full redraw, necessary for 
    // a proper HUD update after missing blinking keys.
    ST_doRefresh();

    /*
    // If just after ST_Start(), refresh all
    // if (st_firsttime)
    //     ST_doRefresh();
    // Otherwise, update as little as possible
    // else 
    //     ST_diffDraw();
    */
}

typedef void (*load_callback_t)(char *lumpname, patch_t **variable);


// Iterates through all graphics to be loaded or unloaded, along with
// the variable they use, invoking the specified callback function.

static void ST_loadUnloadGraphics(load_callback_t callback)
{
    int     i;
    int     j;
    int     facenum;

    char    namebuf[9];

    // Load the numbers, tall and short
    for (i=0;i<10;i++)
    {
        DEH_snprintf(namebuf, 9, "STTNUM%d", i);
        callback(namebuf, &tallnum[i]);

        DEH_snprintf(namebuf, 9, "STYSNUM%d", i);
        callback(namebuf, &shortnum[i]);
    }

    // Load percent key.
    //Note: why not load STMINUS here, too?

    callback(DEH_String("STTPRCNT"), &tallpercent);

    // key cards
    for (i=0;i<NUMCARDS;i++)
    {
        DEH_snprintf(namebuf, 9, "STKEYS%d", i);
        callback(namebuf, &keys[i]);
    }

    // arms ownership widgets
    for (i=0; i<6; i++)
    {
        DEH_snprintf(namebuf, 9, "STGNUM%d", i+2);

        // gray #
        callback(namebuf, &arms[i][0]);

        // yellow #
        arms[i][1] = shortnum[i+2]; 
    }

    // status bar background bits
    callback(DEH_String("STBAR"), &sbar);

    // face states
    facenum = 0;
    for (i=0; i<ST_NUMPAINFACES; i++)
    {
        for (j=0; j<ST_NUMSTRAIGHTFACES; j++)
        {
            DEH_snprintf(namebuf, 9, "STFST%d%d", i, j);
            callback(namebuf, &faces[facenum]);
            ++facenum;
        }

        DEH_snprintf(namebuf, 9, "STFTR%d0", i);	// turn right
        callback(namebuf, &faces[facenum]);
        ++facenum;

        DEH_snprintf(namebuf, 9, "STFTL%d0", i);	// turn left
        callback(namebuf, &faces[facenum]);
        ++facenum;

        DEH_snprintf(namebuf, 9, "STFOUCH%d", i);	// ouch!
        callback(namebuf, &faces[facenum]);
        ++facenum;

        DEH_snprintf(namebuf, 9, "STFEVL%d", i);	// evil grin ;)
        callback(namebuf, &faces[facenum]);
        ++facenum;

        DEH_snprintf(namebuf, 9, "STFKILL%d", i);	// pissed off
        callback(namebuf, &faces[facenum]);
        ++facenum;
    }

    callback(DEH_String("STFGOD0"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFDEAD0"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFEXPL0"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFEXPL1"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFEXPL2"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFEXPL3"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFEXPL4"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFEXPL5"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFCRSH0"), &faces[facenum]);
    ++facenum;

    // [JN] Удвоение массива спрайтов лиц, необходимое для бессмертия.
    for (i = 0; i < ST_NUMPAINFACES; i++)
    {
        for (j = 0; j < ST_NUMSTRAIGHTFACES; j++)
        {
            M_snprintf(namebuf, 9, "STFST%i%iG", i, j);
            callback(namebuf, &faces[facenum++]);
        }

        M_snprintf(namebuf, 9, "STFTR%i0G", i);          // turn right
        callback(namebuf, &faces[facenum++]);

        M_snprintf(namebuf, 9, "STFTL%i0G", i);          // turn left
        callback(namebuf, &faces[facenum++]);

        M_snprintf(namebuf, 9, "STFOUC%iG", i);         // ouch!
        callback(namebuf, &faces[facenum++]);

        M_snprintf(namebuf, 9, "STFEVL%iG", i);          // evil grin ;)
        callback(namebuf, &faces[facenum++]);

        M_snprintf(namebuf, 9, "STFKIL%iG", i);         // pissed off
        callback(namebuf, &faces[facenum++]);
    }

    callback("STFGOD0G", &faces[facenum++]);
    callback("STFDEA0G", &faces[facenum++]);
    callback("STFEXP0G", &faces[facenum++]);
    callback("STFEXP1G", &faces[facenum++]);
    callback("STFEXP2G", &faces[facenum++]);
    callback("STFEXP3G", &faces[facenum++]);
    callback("STFEXP4G", &faces[facenum++]);
    callback("STFEXP5G", &faces[facenum++]);
    callback("STFCRS0G", &faces[facenum++]);
}


static void ST_loadCallback(char *lumpname, patch_t **variable)
{
    *variable = W_CacheLumpName(lumpname, PU_STATIC);
}


void ST_loadGraphics(void)
{
    ST_loadUnloadGraphics(ST_loadCallback);
}


void ST_loadData(void)
{
    lu_palette = W_GetNumForName (DEH_String("PLAYPAL"));

    ST_loadGraphics();
}


static void ST_unloadCallback(char *lumpname, patch_t **variable)
{
    W_ReleaseLumpName(lumpname);
    *variable = NULL;
}


void ST_unloadGraphics(void)
{
    ST_loadUnloadGraphics(ST_unloadCallback);
}


void ST_unloadData(void)
{
    ST_unloadGraphics();
}


void ST_initData(void)
{
    int	    i;

    st_firsttime = true;
    plyr = &players[consoleplayer];

    st_clock = 0;
    st_chatstate = StartChatState;
    st_gamestate = FirstPersonState;

    st_statusbaron = true;
    st_oldchat = st_chat = false;
    st_cursoron = false;

    st_faceindex = 0;
    st_palette = -1;

    st_oldhealth = -1;

    for (i=0;i<NUMWEAPONS;i++)
    oldweaponsowned[i] = plyr->weaponowned[i];

    for (i=0;i<3;i++)
    keyboxes[i] = -1;

    STlib_init();
}


void ST_createWidgets(void)
{
    int i;

    // ready weapon ammo
    STlib_initNum(&w_ready,
        ST_AMMOX,
        ST_AMMOY,
        tallnum,
        &plyr->ammo[weaponinfo[plyr->readyweapon].ammo],
        &st_statusbaron,
        ST_AMMOWIDTH );

    // the last weapon type
    w_ready.data = plyr->readyweapon; 

    // health percentage
    STlib_initPercent(&w_health,
        ST_HEALTHX,
        ST_HEALTHY,
        tallnum,
        &plyr->health,
        &st_statusbaron,
        tallpercent);

    // weapons owned
    for(i=0;i<6;i++)
    {
        STlib_initMultIcon(&w_arms[i],
            ST_ARMSX+(i%3)*ST_ARMSXSPACE,
            ST_ARMSY+(i/3)*ST_ARMSYSPACE,
            arms[i],
            &plyr->weaponowned[i+1],
            &st_armson);
    }

    // faces
    STlib_initMultIcon(&w_faces,
        ST_FACESX,
        ST_FACESY,
        faces,
        &st_faceindex,
        &st_statusbaron);

    // armor percentage - should be colored later
    STlib_initPercent(&w_armor,
        ST_ARMORX,
        ST_ARMORY,
        tallnum,
        &plyr->armorpoints,
        &st_statusbaron, tallpercent);

    // keyboxes 0-2
    // [Julia] Blue
    STlib_initMultIcon(&w_keyboxes[0],
        ST_KEYX,
        ST_KEY0Y,
        keys,
        &keyboxes[0],
        &st_statusbaron);

    // [Julia] Yellow
    STlib_initMultIcon(&w_keyboxes[1],
        ST_KEYX,
        ST_KEY1Y,
        keys,
        &keyboxes[1],
        &st_statusbaron);

    // [Julia] Red
    STlib_initMultIcon(&w_keyboxes[2],
        ST_KEYX,
        ST_KEY2Y,
        keys,
        &keyboxes[2],
        &st_statusbaron);

    // [Julia] current map
    STlib_initNum(&w_currentmap,
		  gamemap >= 10 ? ST_MAP_X2 : ST_MAP_X1,
		  ST_MAPY,
		  tallnum,
		  &gamemap,
		  &st_statusbaron,
		  gamemap >= 10 ? ST_MAPWIDTH_X2 : ST_MAPWIDTH_X1);
}

static boolean	st_stopped = true;


void ST_Start (void)
{
    if (!st_stopped)
    ST_Stop();

    ST_initData();
    ST_createWidgets();
    st_stopped = false;
}


void ST_Stop (void)
{
    if (st_stopped)
    return;

    I_SetPalette (W_CacheLumpNum (lu_palette, PU_CACHE));

    st_stopped = true;
}


void ST_Init (void)
{
    ST_loadData();
    st_backing_screen = (byte *) Z_Malloc((ST_WIDTH << hires) * (ST_HEIGHT << hires), PU_STATIC, 0);
}

