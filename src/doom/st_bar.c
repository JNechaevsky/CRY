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
//	Status bar code.
//	Does the face/direction indicator animatin.
//	Does palette indicators as well (red pain/berserk, bright pickup)
//


#include <stdio.h>

#include "i_swap.h" // [crispy] SHORT()
#include "i_system.h"
#include "i_video.h"
#include "z_zone.h"
#include "m_controls.h"
#include "m_misc.h"
#include "m_random.h"
#include "w_wad.h"
#include "d_main.h"
#include "g_game.h"
#include "p_local.h"
#include "m_menu.h"
#include "s_sound.h"
#include "v_video.h"
#include "doomstat.h"
#include "d_englsh.h"
#include "v_trans.h"
#include "ct_chat.h"
#include "st_bar.h"
#include "am_map.h"

#include "id_vars.h"
#include "id_func.h"


// Palette indices.
// For damage/bonus red-/gold-shifts
#define STARTREDPALS		1
#define NUMREDPALS			16
// Bonus items, gold shift.
#define BONUSPAL			17
// Radiation suit, green shift.
#define RADIATIONPAL		18
// [JN] Bonus+radiation palette, mixed gold and green shift.
#define RADIATIONBONUSPAL	19


// Number of status faces.
#define NUMFACES            49

#define ST_NUMPAINFACES     5
#define ST_NUMSTRAIGHTFACES 3
#define ST_NUMTURNFACES     2
#define ST_NUMSPECIALFACES  3
#define ST_MUCHPAIN         20

#define ST_FACESTRIDE       (ST_NUMSTRAIGHTFACES+ST_NUMTURNFACES+ST_NUMSPECIALFACES)
#define ST_NUMEXTRAFACES    2
// [JN] Additional faces (Jaguar: +6 (exploded), PSX: +1 (squished))
#define ST_NUMFACES         (ST_FACESTRIDE*ST_NUMPAINFACES+ST_NUMEXTRAFACES+7)

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

#define ST_EVILGRINCOUNT        (TICRATE*2)
#define ST_STRAIGHTFACECOUNT    (TICRATE/2)
#define ST_TURNCOUNT            (TICRATE)
#define ST_OUCHCOUNT            (TICRATE)
#define ST_RAMPAGEDELAY         (TICRATE*2)


// graphics are drawn to a backing screen and blitted to the real screen
static pixel_t *st_backing_screen;

// main player in game
static player_t *plyr; 

// lump number for PLAYPAL
int st_palette = 0;

// used for evil grin
static int oldweaponsowned[NUMWEAPONS]; 

static patch_t *sbar;                // main bar background
static patch_t *tallnum[10];         // 0-9, tall numbers
static patch_t *tallpercent;         // tall % sign
static patch_t *tallminus;           // [JN] "minus" symbol
static patch_t *shortnum_y[10];      // 0-9, short, yellow numbers
static patch_t *keys[NUMCARDS];      // 3 key-cards, 3 skulls
static patch_t *faces[NUMFACES];     // face status patches
static patch_t *faceback;            // player face background

// [crispy] blinking key or skull in the status bar
int st_keyorskull[3];

static int st_oldhealth = -1;  // used to use appopriately pained face
static int st_facecount = 0;  // count until face changes
static int st_faceindex = 1;  // current face index, used by w_faces
static int st_randomnumber; // a random number per tick
static int faceindex; // [crispy] fix status bar face hysteresis

// [JN] Condition to redraw status bar background. 
boolean st_fullupdate = true;

// [JN] Array for holding buffered background of status bar.
static int stbar_bg[5];

// [JN] Is status bar background on?
static boolean st_background_on;

// [JN] Pointer to patch drawing functions to decide
// whether or not to use shadow casting for digits and keys.
static void (*drawpatchfunc)(int x, int y, patch_t *patch);

// =============================================================================
// CHEAT SEQUENCE PACKAGE
// =============================================================================

cheatseq_t cheat_wait = CHEAT("id", 0);
cheatseq_t cheat_mus = CHEAT("idmus", 2);
cheatseq_t cheat_god = CHEAT("iddqd", 0);
cheatseq_t cheat_ammo = CHEAT("idkfa", 0);
cheatseq_t cheat_ammonokey = CHEAT("idfa", 0);
cheatseq_t cheat_keys = CHEAT("idka", 0);
cheatseq_t cheat_noclip = CHEAT("idspispopd", 0);
cheatseq_t cheat_commercial_noclip = CHEAT("idclip", 0);
cheatseq_t cheat_choppers = CHEAT("idchoppers", 0);
cheatseq_t cheat_clev = CHEAT("idclev", 2);
cheatseq_t cheat_mypos = CHEAT("idmypos", 0);
// [JN] IDDT cheat, moved from am_map.c
cheatseq_t cheat_amap = CHEAT("iddt", 0);
// [JN] Jaguar: fixed "non-working" cheats:
cheatseq_t cheat_amap_1 = CHEAT("8002545465", 0); /* allmap cheat */
cheatseq_t cheat_amap_2 = CHEAT("8005778788", 0); /* show things cheat */

// [crispy] new cheats
static cheatseq_t cheat_massacre1 = CHEAT("tntem", 0);
static cheatseq_t cheat_massacre2 = CHEAT("killem", 0);
static cheatseq_t cheat_freeze = CHEAT("freeze", 0);
static cheatseq_t cheat_notarget = CHEAT("notarget", 0);
static cheatseq_t cheat_buddha = CHEAT("buddha", 0);

cheatseq_t cheat_powerup[5] =
{
    CHEAT("idbeholdv", 0),
    CHEAT("idbeholds", 0),
    CHEAT("idbeholdr", 0),
    CHEAT("idbeholda", 0),
    CHEAT("idbehold", 0),
};

// -----------------------------------------------------------------------------
// cht_CheckCheat
// Called in st_stuff module, which handles the input.
// Returns a 1 if the cheat was successful, 0 if failed.
// -----------------------------------------------------------------------------

static int cht_CheckCheat (cheatseq_t *cht, char key)
{
    // if we make a short sequence on a cheat with parameters, this 
    // will not work in vanilla doom.  behave the same.

    if (cht->parameter_chars > 0 && strlen(cht->sequence) < cht->sequence_len)
        return false;
    
    if (cht->chars_read < strlen(cht->sequence))
    {
        // still reading characters from the cheat code
        // and verifying.  reset back to the beginning 
        // if a key is wrong

        if (key == cht->sequence[cht->chars_read])
            ++cht->chars_read;
        else
            cht->chars_read = 0;
        
        cht->param_chars_read = 0;
    }
    else if (cht->param_chars_read < cht->parameter_chars)
    {
        // we have passed the end of the cheat sequence and are 
        // entering parameters now 
        
        cht->parameter_buf[cht->param_chars_read] = key;
        
        ++cht->param_chars_read;
    }

    if (cht->chars_read >= strlen(cht->sequence)
    &&  cht->param_chars_read >= cht->parameter_chars)
    {
        cht->chars_read = cht->param_chars_read = 0;

        return true;
    }
    
    // cheat not matched yet

    return false;
}

static void cht_GetParam (cheatseq_t *cht, char *buffer)
{
    memcpy(buffer, cht->parameter_buf, cht->parameter_chars);
}

// -----------------------------------------------------------------------------
// cht_CheckCheatSP
// [crispy] restrict cheat usage
// -----------------------------------------------------------------------------
static inline boolean cht_CheckCheatSP (cheatseq_t *cht, char key)
{
    if (!cht_CheckCheat(cht, key))
    {
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------
// WeaponAvailable
// [crispy] only give available weapons
// -----------------------------------------------------------------------------
static boolean WeaponAvailable (int w)
{
    if (w < 0 || w >= NUMWEAPONS)
    {
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------
// GiveBackpack
// [crispy] give or take backpack
// -----------------------------------------------------------------------------

static void GiveBackpack (boolean give)
{
    int i;
    
    if (give && !plyr->backpack)
    {
        for (i = 0; i < NUMAMMO; i++)
        {
            plyr->maxammo[i] *= 2;
        }
        plyr->backpack = true;
    }
    else
    if (!give && plyr->backpack)
    {
        for (i = 0; i < NUMAMMO; i++)
        {
            plyr->maxammo[i] /= 2;
        }
        plyr->backpack = false;
    }
}

// -----------------------------------------------------------------------------
// [crispy] adapted from boom202s/M_CHEAT.C:467-498
// -----------------------------------------------------------------------------

static int ST_cheat_massacre (boolean explode)
{
    int killcount = 0;
    thinker_t *th;

    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function.acp1 == (actionf_p1)P_MobjThinker)
        {
            mobj_t *mo = (mobj_t *)th;
            const int amount = explode ? 10000 : mo->health;

            if (mo->flags & MF_COUNTKILL || mo->type == MT_SKULL)
            {
                if (mo->health > 0)
                {
                    P_DamageMobj(mo, NULL, NULL, amount);
                    killcount++;
                }
            }
        }
    }

    // [crispy] disable brain spitters
    // numbraintargets = -1;

    return killcount;
}

// -----------------------------------------------------------------------------
// ST_Responder
// Respond to keyboard input events, intercept cheats.
// -----------------------------------------------------------------------------

boolean ST_Responder (event_t *ev)
{
    int i;

    // if a user keypress...
    if (ev->type == ev_keydown)
    {
        {
            // [JN] If user types "id", activate timer to prevent
            // other than typing actions in G_Responder.
            if (cht_CheckCheat(&cheat_wait, ev->data2))
            {
                plyr->cheatTics = TICRATE * 2;
            }

            // 'dqd' cheat for toggleable god mode
            if (cht_CheckCheatSP(&cheat_god, ev->data2))
            {
                // [crispy] dead players are first respawned at the current position
                mapthing_t mt = {0};

                if (plyr->playerstate == PST_DEAD)
                {
                    angle_t an;

                    mt.x = plyr->mo->x >> FRACBITS;
                    mt.y = plyr->mo->y >> FRACBITS;
                    mt.angle = (plyr->mo->angle + ANG45/2)*(uint64_t)45/ANG45;
                    mt.type = consoleplayer + 1;
                    P_SpawnPlayer(&mt);

                    // [crispy] spawn a teleport fog
                    an = plyr->mo->angle >> ANGLETOFINESHIFT;
                    P_SpawnMobj(plyr->mo->x+20*finecosine[an], plyr->mo->y+20*finesine[an], plyr->mo->z, MT_TFOG);
                    S_StartSound(NULL, sfx_telept);

                    // [crispy] fix reviving as "zombie" if god mode was already enabled
                    if (plyr->mo)
                    {
                        plyr->mo->health = 100;
                    }
                    plyr->health = 100;
                }

                plyr->cheats ^= CF_GODMODE;
                if (plyr->cheats & CF_GODMODE)
                {
                    // [JN] Jaguar: no healing by using god mode cheat.
                    /*
                    if (plyr->mo)
                    {
                        plyr->mo->health = 100;
                    }
                    plyr->health = 100;
                    */
                    CT_SetMessage(plyr, STSTR_DQDON, false, NULL);
                }
                else 
                {
                    CT_SetMessage(plyr, STSTR_DQDOFF, false, NULL);
                }
                plyr->cheatTics = 1;
            }
            // 'fa' cheat for killer fucking arsenal
            else if (cht_CheckCheatSP(&cheat_ammonokey, ev->data2))
            {
                plyr->armorpoints = 200;
                plyr->armortype = 2;

                // [crispy] give backpack
                GiveBackpack(true);

                for (i=0;i<NUMWEAPONS;i++)
                {
                    if (WeaponAvailable(i)) // [crispy] only give available weapons
                    {
                        plyr->weaponowned[i] = true;
                    }
                }
				// [JN] Jaguar: happy 500 ammo for everything!
                for (i=0;i<NUMAMMO;i++)
                {
                    plyr->ammo[i] = plyr->maxammo[i] = 500;
                }

                plyr->cheatTics = 1;
                CT_SetMessage(plyr, STSTR_FAADDED, false, NULL);
            }
            // 'kfa' cheat for key full ammo
            else if (cht_CheckCheatSP(&cheat_ammo, ev->data2))
            {
                plyr->armorpoints = 200;
                plyr->armortype = 2;

                // [crispy] give backpack
                GiveBackpack(true);

                for (i = 0 ; i < NUMWEAPONS ; i++)
                {
                    if (WeaponAvailable(i)) // [crispy] only give available weapons
                    {
                        plyr->weaponowned[i] = true;
                    }
                }
				// [JN] Jaguar: happy 500 ammo for everything!
                for (i = 0 ; i < NUMAMMO ; i++)
                {
                    plyr->ammo[i] = plyr->maxammo[i] = 500;
                }
                for (i = 0 ; i < NUMCARDS ; i++)
                {
                    plyr->cards[i] = true;
                }

                plyr->cheatTics = 1;
                CT_SetMessage(plyr, STSTR_KFAADDED, false, NULL);
            }
            // [JN] 'ka' for keys only
            else if (cht_CheckCheatSP(&cheat_keys, ev->data2))
            {
                for (i = 0; i < NUMCARDS ; i++)
                {
                    plyr->cards[i] = true;
                }

                plyr->cheatTics = 1;
                CT_SetMessage(plyr, STSTR_KAADDED, false, NULL);
            }
            // 'mus' cheat for changing music
            else if (cht_CheckCheat(&cheat_mus, ev->data2))
            {
                char buf[3];
                int  musnum;

                CT_SetMessage(plyr, STSTR_MUS, false, NULL);
                cht_GetParam(&cheat_mus, buf);

                // Note: The original v1.9 had a bug that tried to play back
                // the Doom II music regardless of gamemode.  This was fixed
                // in the Ultimate Doom executable so that it would work for
                // the Doom 1 music as well.
                // [JN] Fixed: using a proper IDMUS selection for shareware 
                // and registered game versions.
                {
                    musnum = mus_map01 + (buf[0]-'0')*10 + buf[1]-'0' - 1;

                    // [crispy] prevent crash with IDMUS00
                    if (musnum < mus_map01 || musnum >= NUMMUSIC)
                    {
                        CT_SetMessage(plyr, STSTR_NOMUS, false, NULL);
                    }
                    else
                    {
                        S_ChangeMusic(musnum, 1);
                        // [JN] jff 3/17/98 remember idmus number for restore
                        idmusnum = musnum;
                    }
                }

                plyr->cheatTics = 1;
            }
            // Noclip cheat.
            // For Doom 1, use the idspipsopd cheat; for all others, use idclip            
            // [crispy] allow both idspispopd and idclip cheats in all gamemissions
            else 
            if (cht_CheckCheatSP(&cheat_noclip, ev->data2)
            || (cht_CheckCheatSP(&cheat_commercial_noclip, ev->data2)))
            {	
                plyr->cheats ^= CF_NOCLIP;

                if (plyr->cheats & CF_NOCLIP)
                {
                    plyr->mo->flags |= MF_NOCLIP;
                    CT_SetMessage(plyr, STSTR_NCON, false, NULL);
                }
                else
                {
                    plyr->mo->flags &= ~MF_NOCLIP;
                    CT_SetMessage(plyr, STSTR_NCOFF, false, NULL);
                }

                plyr->cheatTics = 1;
            }

            // 'behold?' power-up cheats
            for (i = 0 ; i < 4 ; i++)
            {
                if (cht_CheckCheatSP(&cheat_powerup[i], ev->data2))
                {
                    if (!plyr->powers[i])
                    {
                        P_GivePower( plyr, i);
                    }
                    else if (i!=pw_strength)
                    {
                        plyr->powers[i] = 1;
                    }
                    else
                    {
                        plyr->powers[i] = 0;
                    }

                    plyr->cheatTics = 1;
                    CT_SetMessage(plyr, STSTR_BEHOLDX, false, NULL);
                }
            }
            // 'behold' power-up menu
            if (cht_CheckCheatSP(&cheat_powerup[4], ev->data2))
            {
                CT_SetMessage(plyr, STSTR_BEHOLD, false, NULL);
            }
            // 'choppers' invulnerability & chainsaw
            else if (cht_CheckCheatSP(&cheat_choppers, ev->data2))
            {
                plyr->weaponowned[wp_chainsaw] = true;
                plyr->powers[pw_invulnerability] = true;
                plyr->cheatTics = 1;
                CT_SetMessage(plyr, STSTR_CHOPPERS, false, NULL);
            }
            // 'mypos' for player position
            else if (cht_CheckCheat(&cheat_mypos, ev->data2))
            {
                static char buf[52];

                M_snprintf(buf, sizeof(buf), "ang=0x%x;x,y=(0x%x,0x%x)",
                           players[displayplayer].mo->angle,
                           players[displayplayer].mo->x,
                           players[displayplayer].mo->y);
                plyr->cheatTics = 1;
                CT_SetMessage(plyr, buf, false, NULL);
            }
            // [JN] IDDT cheating, moved from am_map.c
            else if (cht_CheckCheat(&cheat_amap, ev->data2))
            {
                iddt_cheating = (iddt_cheating + 1) % 3;
                plyr->cheatTics = 1;
            }
            // [JN] Jaguar cheat: allmap
            else if (cht_CheckCheat(&cheat_amap_1, ev->data2))
            {
                iddt_cheating = 1;
                plyr->cheatTics = 1;
            }
            // [JN] Jaguar cheat: show things
            else if (cht_CheckCheat(&cheat_amap_2, ev->data2))
            {
                iddt_cheating = 2;
                plyr->cheatTics = 1;
            }
            // [crispy] implement Boom's "tntem" cheat
            else if (cht_CheckCheatSP(&cheat_massacre1, ev->data2))
            {
                static char buf[52];
                const int killcount = ST_cheat_massacre(true);

                M_snprintf(buf, sizeof(buf), "Monsters killed: %d", killcount);
                
                plyr->cheatTics = 1;
                CT_SetMessage(plyr, buf, false, NULL);
            }
            // [JN] implement MBF's "killem" cheat, which kills, not explodes enemies.
            else if (cht_CheckCheatSP(&cheat_massacre2, ev->data2))
            {
                static char buf[52];
                const int killcount = ST_cheat_massacre(false);

                M_snprintf(buf, sizeof(buf), "Monsters killed: %d", killcount);
                
                plyr->cheatTics = 1;
                CT_SetMessage(plyr, buf, false, NULL);
            }
            // [JN] CRL - Freeze mode.
            else if (cht_CheckCheatSP(&cheat_freeze, ev->data2))
            {
                crl_freeze ^= 1;
                plyr->cheatTics = 1;
                CT_SetMessage(plyr, crl_freeze ?
                              ID_FREEZE_ON : ID_FREEZE_OFF, false, NULL);
            }
            // [JN] Implement Woof's "notarget" cheat.
            else if (cht_CheckCheatSP(&cheat_notarget, ev->data2))
            {
                plyr->cheats ^= CF_NOTARGET;
                P_ForgetPlayer(plyr);
                plyr->cheatTics = 1;
                CT_SetMessage(plyr, plyr->cheats & CF_NOTARGET ?
                               ID_NOTARGET_ON : ID_NOTARGET_OFF, false, NULL);
            }
            // [JN] Implement Woof's "buddha" cheat.
            else if (cht_CheckCheatSP(&cheat_buddha, ev->data2))
            {
                plyr->cheats ^= CF_BUDDHA;
                plyr->cheatTics = 1;
                CT_SetMessage(plyr, plyr->cheats & CF_BUDDHA ?
                               ID_BUDDHA_ON : ID_BUDDHA_OFF, false, NULL);
            }
        }

        // 'clev' change-level cheat
        if (cht_CheckCheat(&cheat_clev, ev->data2))
        {
            char  buf[3];
            int   epsd;
            int   map;

            cht_GetParam(&cheat_clev, buf);

            {
                epsd = 0;
                map = (buf[0] - '0')*10 + buf[1] - '0';
            }
            {
                if (map < 1)
                {
                    return false;
                }
                if (map > 24)
                {
                    return false;
                }
            }

            // So be it.
            {
                plyr->cheatTics = 1;
                CT_SetMessage(plyr, STSTR_CLEV, false, NULL);
                G_DeferedInitNew(gameskill, epsd, map);
            }
        }
    }

    return false;
}

// -----------------------------------------------------------------------------
// ST_calcPainOffset
// -----------------------------------------------------------------------------

static int ST_calcPainOffset (void)
{
    int        health;
    static int lastcalc;
    static int oldhealth = -1;

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
// This is a not-very-pretty routine which handles the face states 
// and their timing. // the precedence of expressions is:
//  dead > evil grin > turned head > straight ahead
// -----------------------------------------------------------------------------

static void ST_updateFaceWidget (void)
{
    int         i;
    static int  lastattackdown = -1;
    static int  priority = 0;
    angle_t     badguyangle;
    angle_t     diffang;
    boolean     doevilgrin;

    // [crispy] fix status bar face hysteresis
    int painoffset = ST_calcPainOffset();
    // [crispy] no evil grin or rampage face in god mode
    const boolean invul = (plyr->cheats & CF_GODMODE) || plyr->powers[pw_invulnerability];

    if (priority < 11)
    {
        // dead
        if (!plyr->health)
        {
            priority = 10;
            painoffset = 0;
            faceindex = ST_DEADFACE;
            st_facecount = 1;
        }

        // [JN] Exploded and squished faces
        if (plyr->health <= 0 
        &&  plyr->mo->state - states >= mobjinfo[plyr->mo->type].xdeathstate)
        {
            priority = 9;
            painoffset = 0;

            // Sync with actual player state:
            if (plyr->mo->state == &states[S_PLAY_XDIE1])
                faceindex = ST_EXPLFACE0;
            if (plyr->mo->state == &states[S_PLAY_XDIE2])
                faceindex = ST_EXPLFACE1;
            if (plyr->mo->state == &states[S_PLAY_XDIE3])
                faceindex = ST_EXPLFACE2;
            if (plyr->mo->state == &states[S_PLAY_XDIE4])
                faceindex = ST_EXPLFACE3;
            if (plyr->mo->state == &states[S_PLAY_XDIE5])
                faceindex = ST_EXPLFACE4;
            if ((plyr->mo->state == &states[S_PLAY_XDIE6])
            ||  (plyr->mo->state == &states[S_PLAY_XDIE7])
            ||  (plyr->mo->state == &states[S_PLAY_XDIE8])
            ||  (plyr->mo->state == &states[S_PLAY_XDIE9]))
                faceindex = ST_EXPLFACE5;

            if (plyr->mo->state == &states[S_GIBS])
                faceindex = ST_CRSHFACE0;
        }
    }

    if (priority < 10)
    {
        // [JN] invulnerability (moved up here)
        if (invul)
        {
            priority = 9;
            painoffset = 0;
            faceindex = ST_GODFACE;
            st_facecount = 1;
        }
    }

    if (priority < 9)
    {
        if (plyr->bonuscount)
        {
            // picking up bonus
            doevilgrin = false;

            for (i = 0 ; i < NUMWEAPONS ; i++)
            {
                if (oldweaponsowned[i] != plyr->weaponowned[i])
                {
                    doevilgrin = true;
                    oldweaponsowned[i] = plyr->weaponowned[i];
                }
            }
            // [crispy] no evil grin in god mode
            if (doevilgrin && !invul)
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
                // [crispy] raise "Ouch Face" priority
                priority = 8;
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
            // [crispy] no rampage face in god mode
            else if (!--lastattackdown && !invul)
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
}

// -----------------------------------------------------------------------------
// ST_doPaletteStuff
// -----------------------------------------------------------------------------

void ST_doPaletteStuff (void)
{
    int palette;
    int red = plyr->damagecount;
    int yel = plyr->bonuscount;
    int grn = (plyr->powers[pw_ironfeet] > 4*32 || plyr->powers[pw_ironfeet] & 8);

    if (plyr->powers[pw_strength])
    {
        // slowly fade the berzerk out
        // [JN] Jaguar: emulate faster fading.
        const int bzc = 42 - (plyr->powers[pw_strength]>>1);

        if (bzc > red)
        {
            red = bzc;
        }
    }

    if (red)
    {
        palette = red;

        if (palette >= NUMREDPALS)
        {
            palette = NUMREDPALS-1;
        }

        palette += STARTREDPALS;
    }
    else if (yel)
    {
        palette = BONUSPAL;
        
        // [JN] If rad palette is active, use special bonus+radiation palette.
        if (grn)
        {
            palette = RADIATIONBONUSPAL;
        }
    }
    else if (grn)
    {
        palette = RADIATIONPAL;
    }
    else
    {
        palette = 0;
    }

    if (palette != st_palette || yel)
    {
        st_palette = palette;
		I_SetPalette (palette);
    }
}

// -----------------------------------------------------------------------------
// ST_Ticker
// -----------------------------------------------------------------------------

void ST_Ticker (void)
{
    // refresh everything if this is him coming back to life
    ST_updateFaceWidget();

    st_background_on = dp_screen_size <= 10 || (automapactive && !automap_overlay);
    st_randomnumber = M_Random();
    st_oldhealth = plyr->health;

    // [JN] Update CRL_Widgets_t data.
    IDWidget.kills = plyr->killcount;
    IDWidget.totalkills = totalkills;
    IDWidget.items = plyr->itemcount;
    IDWidget.totalitems = totalitems;
    IDWidget.secrets = plyr->secretcount;
    IDWidget.totalsecrets = totalsecret;

    IDWidget.x = plyr->mo->x >> FRACBITS;
    IDWidget.y = plyr->mo->y >> FRACBITS;
    IDWidget.ang = plyr->mo->angle / ANG1;

    // [JN] Which patch drawing function to use for digits and keys?
    if (st_background_on)
    {
        drawpatchfunc = V_DrawPatch;
    }
    else
    {
        drawpatchfunc = V_DrawShadowedPatchOptional;
    }

    // [JN] Update blinking key or skull timer.
    for (int i = 0 ; i < 3 ; i++)
    {
        if (plyr->tryopen[i])
        {
            plyr->tryopen[i]--;
        }
    }

    // Do red-/gold-shifts from damage/items
    ST_doPaletteStuff();
}

// -----------------------------------------------------------------------------
// ST_WidgetColor
// [crispy] return ammo/health/armor widget color
// -----------------------------------------------------------------------------

enum
{
    hudcolor_ammo,
    hudcolor_health,
    hudcolor_armor
} hudcolor_t;

static byte *ST_WidgetColor (const int i)
{
    if (!st_colored_stbar)
    {
        return NULL;
    }

    switch (i)
    {
        case hudcolor_ammo:
        {
            if (weaponinfo[plyr->readyweapon].ammo == am_noammo)
            {
                return NULL;
            }
            else
            {
                int ammo =  plyr->ammo[weaponinfo[plyr->readyweapon].ammo];
                int fullammo = maxammo[weaponinfo[plyr->readyweapon].ammo];

                if (ammo < fullammo/4)
                    return cr[CR_RED];
                else if (ammo < fullammo/2)
                    return cr[CR_YELLOW];
                else if (ammo <= fullammo)
                    return cr[CR_GREEN];
                else
                    return cr[CR_BLUE2];
            }
            break;
        }
        case hudcolor_health:
        {
            int health = plyr->health;

            // [crispy] Invulnerability powerup and God Mode cheat turn Health values gray
            // [JN] I'm using different health values, represented by crosshair,
            // and thus a little bit different logic.
            if (plyr->cheats & CF_GODMODE || plyr->powers[pw_invulnerability])
                return cr[CR_WHITE];
            else if (health > 100)
                return cr[CR_BLUE2];
            else if (health >= 67)
                return cr[CR_GREEN];
            else if (health >= 34)
                return cr[CR_YELLOW];
            else
                return cr[CR_RED];
            break;
        }
        case hudcolor_armor:
        {
	    // [crispy] Invulnerability powerup and God Mode cheat turn Armor values gray
	    if (plyr->cheats & CF_GODMODE || plyr->powers[pw_invulnerability])
                return cr[CR_WHITE];
	    // [crispy] color by armor type
	    else if (plyr->armortype >= 2)
                return cr[CR_BLUE2];
	    else if (plyr->armortype == 1)
                return cr[CR_GREEN];
	    else if (plyr->armortype == 0)
                return cr[CR_RED];
            break;
        }
    }

    return NULL;
}

// -----------------------------------------------------------------------------
// ST_DrawBigNumber
// [JN] Draws a three digit big red number using NUM_* graphics.
// -----------------------------------------------------------------------------

static void ST_DrawBigNumber (int val, const int x, const int y, byte *table)
{
    int oldval = val;
    int xpos = x;

    dp_translation = table;

    // [JN] Support for negative values.
    if (val < 0)
    {
        val = -val;
        
        if (-val <= -99)
        {
            val = 99;
        }

        // [JN] Draw minus symbol with respection of digits placement.
        // However, values below -10 requires some correction in "x" placement.
        drawpatchfunc(xpos + (val <= 9 ? 20 : 5) - 4, y, tallminus);
    }
    if (val > 999)
    {
        val = 999;
    }

    if (val > 99)
    {
        drawpatchfunc(xpos - 4, y, tallnum[val / 100]);
    }

    val = val % 100;
    xpos += 14;

    if (val > 9 || oldval > 99)
    {
        drawpatchfunc(xpos - 4, y, tallnum[val / 10]);
    }

    val = val % 10;
    xpos += 14;

    drawpatchfunc(xpos - 4, y, tallnum[val]);
    
    dp_translation = NULL;
}

// -----------------------------------------------------------------------------
// ST_DrawPercent
// [JN] Draws big red percent sign.
// -----------------------------------------------------------------------------

static void ST_DrawPercent (const int x, const int y, byte *table)
{
    dp_translation = table;
    drawpatchfunc(x, y, tallpercent);
    dp_translation = NULL;
}

// -----------------------------------------------------------------------------
// ST_DrawSmallNumberY
// [JN] Draws a three digit yellow number using STYSNUM* graphics.
// -----------------------------------------------------------------------------

static void ST_DrawSmallNumberY (int val, const int x, const int y)
{
    int oldval = val;
    int xpos = x;

    if (val < 0)
    {
        val = 0;
    }
    if (val > 999)
    {
        val = 999;
    }

    if (val > 99)
    {
        V_DrawPatch(xpos - 4, y, shortnum_y[val / 100]);
    }

    val = val % 100;
    xpos += 4;

    if (val > 9 || oldval > 99)
    {
        V_DrawPatch(xpos - 4, y, shortnum_y[val / 10]);
    }

    val = val % 10;
    xpos += 4;

    V_DrawPatch(xpos - 4, y, shortnum_y[val]);
}

// -----------------------------------------------------------------------------
// ST_DrawSmallNumberFunc
// [JN] Jaguar: slightly different logics: if weapon is owned,
//      draw it's number using a yellow font. Else, don't draw at all.
// -----------------------------------------------------------------------------

static void ST_DrawWeaponNumberFunc (const int val, const int x, const int y, const boolean have_it)
{
    if (have_it)
    {
        ST_DrawSmallNumberY(val, x, y);
    }
}

// -----------------------------------------------------------------------------
// ST_UpdateElementsBackground
// [JN] Use V_CopyRect to draw/update background under elements.
//      This is notably faster than re-drawing entire background.
// -----------------------------------------------------------------------------

static void ST_UpdateElementsBackground (void)
{
    V_CopyRect(stbar_bg[0], stbar_bg[1], st_backing_screen,
               stbar_bg[2], stbar_bg[3],
               stbar_bg[0], stbar_bg[4]);
}

// -----------------------------------------------------------------------------
// ST_Drawer
// [JN] Main drawing function, totally rewritten.
// -----------------------------------------------------------------------------

void ST_Drawer (boolean force)
{
    if (force)
    {
    // [JN] Wide status bar.
    const int wide_x = dp_screen_size > 12 && (!automapactive || automap_overlay) ?
                       WIDESCREENDELTA : 0;

    plyr = &players[displayplayer];

    // Status bar background.
    if (st_background_on && st_fullupdate)
    {
        V_UseBuffer(st_backing_screen);

        // [crispy] this is our own local copy of R_FillBackScreen() to
        // fill the entire background of st_backing_screen with the bezel pattern,
        // so it appears to the left and right of the status bar in widescreen mode
        {
            byte *src;
            pixel_t *dest;
            const char *name = "FLOOR7_1";

            src = W_CacheLumpName(name, PU_CACHE);
            dest = st_backing_screen;

            // [crispy] use unified flat filling function
            V_FillFlat(SCREENHEIGHT-(ST_HEIGHT*vid_resolution), SCREENHEIGHT, 0, SCREENWIDTH, src, dest);

            // [crispy] preserve bezel bottom edge
            if (scaledviewwidth == SCREENWIDTH)
            {
                int x;
                patch_t *patch = W_CacheLumpName("brdr_b", PU_CACHE);

                for (x = 0 ; x < WIDESCREENDELTA ; x += 8)
                {
                    V_DrawPatch(x - WIDESCREENDELTA, 0, patch);
                    V_DrawPatch(ORIGWIDTH + WIDESCREENDELTA - x - 8, 0, patch);
                }
            }
        }

        // [crispy] center unity rerelease wide status bar
        if (SHORT(sbar->width) > ORIGWIDTH && SHORT(sbar->leftoffset) == 0)
        {
            V_DrawPatch((ORIGWIDTH - SHORT(sbar->width)) / 2, 0, sbar);
        }
        else
        {
            V_DrawPatch(0, 0, sbar);
        }

        V_RestoreBuffer();

        V_CopyRect(0, 0, st_backing_screen, SCREENWIDTH, ST_HEIGHT * vid_resolution, 0, ST_Y * vid_resolution);
    }

    st_fullupdate = false;

    if (st_background_on)
    {
        ST_UpdateElementsBackground();
    }

    // Ammo amount for current weapon
    {
        // [JN] Jaguar: draw "0" for fist and chainsaw.
		const int num = (weaponinfo[plyr->readyweapon].ammo == am_noammo ?
                        0 : plyr->ammo[weaponinfo[plyr->readyweapon].ammo]);

        ST_DrawBigNumber(num, 13 - wide_x, 174, ST_WidgetColor(hudcolor_ammo));
    }

    // Health, negative health
    {
        const boolean neghealth = st_negative_health && plyr->health <= 0;

        ST_DrawBigNumber(neghealth ? plyr->health_negative : plyr->health,
                         66 - wide_x, 174, ST_WidgetColor(hudcolor_health));
        ST_DrawPercent(104 - wide_x, 174, ST_WidgetColor(hudcolor_health));
    }

    // [crispy] blinking key or skull in the status bar
    for (int i = 0 ; i < 3 ; i++)
    {
        if (plyr->tryopen[i])
        {
            if (!(plyr->tryopen[i] & (2 * KEYBLINKMASK - 1)))
            {
                S_StartSound(NULL, sfx_itemup);
            }
            if (plyr->tryopen[i] & KEYBLINKMASK)
            {
                const int yy = plyr->tryopen[2] ? 163 :  // red key
                               plyr->tryopen[0] ? 175 :  // blue key
                                                  187 ;  // yellow key
                drawpatchfunc(124 - wide_x, yy, keys[i + st_keyorskull[i]]);
            }
        }
    }

    // Keys (in order of Jaguar Doom)
    if (plyr->cards[it_redskull])
    drawpatchfunc(124 - wide_x, 163, keys[5]);
    else if (plyr->cards[it_redcard])
    drawpatchfunc(124 - wide_x, 163, keys[2]);

    if (plyr->cards[it_blueskull])
    drawpatchfunc(124 - wide_x, 175, keys[3]);
    else if (plyr->cards[it_bluecard])
    drawpatchfunc(124 - wide_x, 175, keys[0]);

    if (plyr->cards[it_yellowskull])
    drawpatchfunc(124 - wide_x, 187, keys[4]);
    else if (plyr->cards[it_yellowcard])
    drawpatchfunc(124 - wide_x, 187, keys[1]);

    // Player face background
    if ((dp_screen_size == 11 || dp_screen_size == 13) && !st_background_on)
    {
        V_DrawPatch(143, 163, faceback);
    }
    // Player face
    if (dp_screen_size <= 11 || dp_screen_size == 13 || (automapactive && !automap_overlay))
    {
        V_DrawPatch(143, 166, faces[st_faceindex]);
    }

    // Armor
    ST_DrawBigNumber(plyr->armorpoints, 187 + wide_x, 174, ST_WidgetColor(hudcolor_armor));
    ST_DrawPercent(225 + wide_x, 174, ST_WidgetColor(hudcolor_armor));

    // Pistol
    ST_DrawWeaponNumberFunc(2, 245 + wide_x, 175, plyr->weaponowned[1]);
    // Shotgun
    ST_DrawWeaponNumberFunc(3, 257 + wide_x, 175, plyr->weaponowned[2]);
    // Chaingun
    ST_DrawWeaponNumberFunc(4, 269 + wide_x, 175, plyr->weaponowned[3]);
    // Rocket Launcher
    ST_DrawWeaponNumberFunc(5, 245 + wide_x, 185, plyr->weaponowned[4]);
    // Plasma Gun
    ST_DrawWeaponNumberFunc(6, 257 + wide_x, 185, plyr->weaponowned[5]);
    // BFG9000
    ST_DrawWeaponNumberFunc(7, 269 + wide_x, 185, plyr->weaponowned[6]);

    // Current map
    ST_DrawBigNumber(gamemap, 279 + wide_x, 174, NULL);
    }
}

void ST_Start (void)
{
    I_SetPalette (0);

    plyr = &players[displayplayer];
    faceindex = 1; // [crispy] fix status bar face hysteresis across level changes
    st_faceindex = 1;
    st_palette = -1;
    st_oldhealth = -1;

    for (int i = 0 ; i < NUMWEAPONS ; i++)
    {
        oldweaponsowned[i] = plyr->weaponowned[i];
    }
}

void ST_Init (void)
{
	int  i;
	char name[9];

	// Minus symbol
	tallminus = W_CacheLumpName("MINUS", PU_STATIC);

	// Load the numbers, tall and short
	for (i = 0 ; i < 10 ; i++)
	{
		snprintf(name, 9, "NUM_%d", i);
		tallnum[i] = W_CacheLumpName(name, PU_STATIC);

		snprintf(name, 9, "MICRO_%d", i);
		shortnum_y[i] = W_CacheLumpName(name, PU_STATIC);
	}

    // Load percent key
	tallpercent = W_CacheLumpName("PERCENT", PU_STATIC);

	// Key cards
	for (i=0;i<NUMCARDS;i++)
	{
		snprintf(name, 9, "CS_KEY%d", i);
		keys[i] = W_CacheLumpName(name, PU_STATIC);
	}

    // Face background
	faceback = W_CacheLumpName("STPBG", PU_STATIC);

    // Main status bar patch
	sbar = W_CacheLumpName("STBAR", PU_STATIC);

	// Face states
	for (i = 0 ; i < NUMFACES ; i++)
	{
        snprintf(name, 9, "FACE%.2d", i);
        faces[i] = W_CacheLumpName(name, PU_STATIC);
    }

	// Allocate status bar background.
	st_backing_screen = (pixel_t *) Z_Malloc(MAXWIDTH * (ST_HEIGHT * MAXHIRES)
					* sizeof(*st_backing_screen), PU_STATIC, 0);
}

// -----------------------------------------------------------------------------
// ST_InitElementsBackground
// [JN] Preallocate rectangle sizes for status bar buffered drawing 
//      to avoid some extra multiplying calculations while drawing.
// -----------------------------------------------------------------------------

void ST_InitElementsBackground (void)
{
    stbar_bg[0] = WIDESCREENDELTA * vid_resolution;
    stbar_bg[1] = vid_resolution;
    stbar_bg[2] = ORIGWIDTH * vid_resolution;
    stbar_bg[3] = 40 * vid_resolution;
    stbar_bg[4] = 161 * vid_resolution;
}
