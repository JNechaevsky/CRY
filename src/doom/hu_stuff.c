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

// [Julia] TODO - more chat cleanup

#include <ctype.h>
#include <time.h>

#include "doomdef.h"
#include "doomkeys.h"
#include "z_zone.h"
#include "deh_main.h"
#include "i_input.h"
#include "i_swap.h"
#include "i_video.h"
#include "hu_stuff.h"
#include "hu_lib.h"
#include "m_controls.h"
#include "m_misc.h"
#include "w_wad.h"
#include "s_sound.h"
#include "doomstat.h"
#include "st_stuff.h" // [JN] ST_HEIGHT
#include "v_video.h"  // [JN] V_DrawPatch
#include "sounds.h"
#include "d_englsh.h"
#include "jn.h"


// =============================================================================
// Locally used constants, shortcuts.
// =============================================================================

// [Julia] Jaguar map names
#define HU_TITLEJ       (mapnames_jaguar[gamemap-1])

#define HU_TITLEHEIGHT  1
#define HU_TITLEX       0
#define HU_TITLEY       (159 - SHORT(hu_font[0]->height))

#define HU_INPUTTOGGLE  't'
#define HU_INPUTX       HU_MSGX
#define HU_INPUTY       (HU_MSGY + HU_MSGHEIGHT*(SHORT(hu_font[0]->height) +1))
#define HU_INPUTWIDTH   64
#define HU_INPUTHEIGHT  1

#define HU_COORDX       (ORIGWIDTH - 8 * hu_font['A'-HU_FONTSTART]->width)



char* player_names[] =
{
    HUSTR_PLRGREEN,
    HUSTR_PLRINDIGO,
    HUSTR_PLRBROWN,
    HUSTR_PLRRED
};


extern int showMessages;
static int message_counter;

char               chat_char; // remove later.
static             hu_textline_t w_title;
static             hu_textline_t w_kills;
static             hu_textline_t w_items;
static             hu_textline_t w_scrts;
static             hu_textline_t w_ltime;
static             hu_itext_t w_chat;
static hu_itext_t  w_inputbuffer[MAXPLAYERS];
static hu_stext_t  w_message;

boolean            chat_on;
boolean            message_dontfuckwithme;
static boolean     always_off = false;
static boolean     message_on;
static boolean     message_nottobefuckedwith;
static boolean     headsupactive = false;

static player_t   *plr;
patch_t           *hu_font[HU_FONTSIZE];
patch_t           *hu_font2[HU_FONTSIZE2];


// =============================================================================
// Builtin map names.
// The actual names can be found in DStrings.h.
// =============================================================================


// -----------------------------------------------------------------------------
// [Julia] List of Jaguar map names
// -----------------------------------------------------------------------------

char *mapnames_jaguar[] =
{
    JHUSTR_1,
    JHUSTR_2,
    JHUSTR_3,
    JHUSTR_4,
    JHUSTR_5,
    JHUSTR_6,
    JHUSTR_7,
    JHUSTR_8,
    JHUSTR_9,
    JHUSTR_10,
    JHUSTR_11,
    JHUSTR_12,
    JHUSTR_13,
    JHUSTR_14,
    JHUSTR_15,
    JHUSTR_16,
    JHUSTR_17,
    JHUSTR_18,
    JHUSTR_19,
    JHUSTR_20,
    JHUSTR_21,
    JHUSTR_22,
    JHUSTR_23,
    JHUSTR_24,
    JHUSTR_25,
    JHUSTR_26
};


// -----------------------------------------------------------------------------
// HU_Init
// -----------------------------------------------------------------------------

void HU_Init(void)
{
    int     i;
    int     j;
    int     n;
    char    buffer[9];

    j = HU_FONTSTART;   // load the heads-up font
    n = HU_FONTSTART2;  // [Julia] load the big heads-up font

    // [Julia] Standard STCFN font
    for (i=0;i<HU_FONTSIZE;i++)
    {
        DEH_snprintf(buffer, 9, "STCFN%.3d", j++);
        hu_font[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
    }

    // [Julia] Load the big heads-up font
    for (i=0;i<HU_FONTSIZE2;i++)
    {
        DEH_snprintf(buffer, 9, "STCFB%.3d", n++);
        hu_font2[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
    }
}


// -----------------------------------------------------------------------------
// HU_Stop
// -----------------------------------------------------------------------------

void HU_Stop(void) 
{
    headsupactive = false;
}


// -----------------------------------------------------------------------------
// HU_Start
// -----------------------------------------------------------------------------

void HU_Start(void)
{
    int   i;
    char *s;

    if (headsupactive)
    HU_Stop();

    plr = &players[consoleplayer];
    message_on = false;
    message_dontfuckwithme = false;
    message_nottobefuckedwith = false;
    chat_on = false;

    // create the message widget
    HUlib_initSText(&w_message, HU_MSGX, HU_MSGY, HU_MSGHEIGHT, 
                                hu_font, HU_FONTSTART, &message_on);

    // create the map title widget
    HUlib_initTextLine(&w_title,
                HU_TITLEX, 
                HU_TITLEY,
                hu_font,
                HU_FONTSTART);

    HUlib_initTextLine(&w_kills,
                HU_TITLEX, HU_MSGY + 1 * 8,
                hu_font,
                HU_FONTSTART);

    HUlib_initTextLine(&w_items,
                HU_TITLEX, HU_MSGY + 2 * 8,
                hu_font,
                HU_FONTSTART);

    HUlib_initTextLine(&w_scrts,
                HU_TITLEX, HU_MSGY + 3 * 8,
                hu_font,
                HU_FONTSTART);

    HUlib_initTextLine(&w_ltime,
                HU_TITLEX, HU_MSGY + 5 * 8,
                hu_font,
                HU_FONTSTART);

    // [Julia] Set Jaguar map names
    s = HU_TITLEJ;

    // dehacked substitution to get modified level name
    s = DEH_String(s);

    while (*s)
    HUlib_addCharToTextLine(&w_title, *(s++));

    // create the chat widget
    HUlib_initIText(&w_chat, HU_INPUTX, HU_INPUTY, hu_font, HU_FONTSTART, &chat_on);

    // create the inputbuffer widgets
    for (i=0 ; i<MAXPLAYERS ; i++)
    HUlib_initIText(&w_inputbuffer[i], 0, 0, 0, 0, &always_off);

    headsupactive = true;
}


// -----------------------------------------------------------------------------
// HU_Drawer
// -----------------------------------------------------------------------------

void HU_Drawer(void)
{
    HUlib_drawSText(&w_message);
    HUlib_drawIText(&w_chat);

    if (automapactive)
    {
        static char str[32], *s;
        int time = leveltime / TICRATE;

        HUlib_drawTextLine(&w_title, false);

        // [from-crispy] Show level stats in automap
        // if (!vanillaparm && automap_stats)
        {
            sprintf(str, "Kills: %d/%d", 
                    players[consoleplayer].killcount, totalkills);
            HUlib_clearTextLine(&w_kills);
            s = str;
            while (*s)
                HUlib_addCharToTextLine(&w_kills, *(s++));
            HUlib_drawTextLine(&w_kills, false);
    
            sprintf(str, "Items: %d/%d",
                    players[consoleplayer].itemcount, totalitems);
            HUlib_clearTextLine(&w_items);
            s = str;
            while (*s)
                HUlib_addCharToTextLine(&w_items, *(s++));
            HUlib_drawTextLine(&w_items, false);
    
            sprintf(str, "Secret: %d/%d",
                    players[consoleplayer].secretcount, totalsecret);
            HUlib_clearTextLine(&w_scrts);
            s = str;
            while (*s)
                HUlib_addCharToTextLine(&w_scrts, *(s++));
            HUlib_drawTextLine(&w_scrts, false);
    
            sprintf(str, "%02d:%02d:%02d", time/3600, (time%3600)/60, time%60);
            HUlib_clearTextLine(&w_ltime);
            s = str;
            while (*s)
                HUlib_addCharToTextLine(&w_ltime, *(s++));
            HUlib_drawTextLine(&w_ltime, false);
        }
    }

    // [Julia] Draw crosshair. 
    // Thanks to Fabian Greffrath for ORIGWIDTH, ORIGHEIGHT and ST_HEIGHT values,
    // thanks to Zodomaniac for proper health values!
    if (!automapactive && crosshair_draw)
    {
        V_DrawPatchUnscaled(SCREENWIDTH/2,
            ((screenblocks <= 10) ? (SCREENHEIGHT-ST_HEIGHT-26)/2 : (SCREENHEIGHT+4)/2),
                W_CacheLumpName(DEH_String(plr->health >= 67 ?
                                           "XHAIRG" : // Green
                                           plr->health >= 34 ?
                                           "XHAIRY" : // Yellow
                                           "XHAIRR"), // Red
                                           PU_CACHE));
    }
}


// -----------------------------------------------------------------------------
// HU_Erase
// -----------------------------------------------------------------------------

void HU_Erase(void)
{
    HUlib_eraseSText(&w_message);
    HUlib_eraseIText(&w_chat);
    HUlib_eraseTextLine(&w_title);
}


// -----------------------------------------------------------------------------
// HU_Ticker
// -----------------------------------------------------------------------------

void HU_Ticker(void)
{
    // tick down message counter if message is up
    if (message_counter && !--message_counter)
    {
        message_on = false;
        message_nottobefuckedwith = false;
    }

    if (showMessages || message_dontfuckwithme)
    {
        // display message if necessary
        if ((plr->message && !message_nottobefuckedwith) 
        ||  (plr->message && message_dontfuckwithme))
        {
            HUlib_addMessageToSText(&w_message, 0, plr->message);
            plr->message = 0;
            message_on = true;
            message_counter = HU_MSGTIMEOUT;
            message_nottobefuckedwith = message_dontfuckwithme;
            message_dontfuckwithme = 0;
        }
    }
}


// -----------------------------------------------------------------------------
// HU_queueChatChar
// -----------------------------------------------------------------------------

#define QUEUESIZE 128

static char chatchars[QUEUESIZE];
static int  head = 0;
static int  tail = 0;


void HU_queueChatChar(char c)
{
    if (((head + 1) & (QUEUESIZE-1)) == tail)
    {
        plr->message = DEH_String(HUSTR_MSGU);
    }
    else
    {
        chatchars[head] = c;
        head = (head + 1) & (QUEUESIZE-1);
    }
}


// -----------------------------------------------------------------------------
// HU_dequeueChatChar
// -----------------------------------------------------------------------------

char HU_dequeueChatChar(void)
{
    char c;

    if (head != tail)
    {
        c = chatchars[tail];
        tail = (tail + 1) & (QUEUESIZE-1);
    }
    else
    {
        c = 0;
    }

    return c;
}


// -----------------------------------------------------------------------------
// StopChatInput
// -----------------------------------------------------------------------------

static void StopChatInput(void)
{
    chat_on = false;
    I_StopTextInput();
}


// -----------------------------------------------------------------------------
// HU_Responder
// -----------------------------------------------------------------------------

boolean HU_Responder(event_t *ev)
{
    int             i;
    int             numplayers;
    unsigned char   c;
    boolean         eatkey = false;
    static char     lastmessage[HU_MAXLINELENGTH+1];
    static boolean  altdown = false;

    numplayers = 0;

    for (i=0 ; i<MAXPLAYERS ; i++)
        numplayers += playeringame[i];

    if (ev->data1 == KEY_RSHIFT)
    {
        return false;
    }
    else if (ev->data1 == KEY_RALT || ev->data1 == KEY_LALT)
    {
        altdown = ev->type == ev_keydown;
        return false;
    }

    if (ev->type != ev_keydown)
    return false;

    if (!chat_on)
    {
        if (ev->data1 == key_message_refresh)
        {
            message_on = true;
            message_counter = HU_MSGTIMEOUT;
            eatkey = true;
        }
    }
    else
    {
        // send a macro
        if (altdown)
        {
            c = ev->data1 - '0';
            if (c > 9)
            return false;
            // fprintf(stderr, "got here\n");
            // [Julia] TODO - REMOVE
            // macromessage = chat_macros[c];

            // kill last message with a '\n'
            HU_queueChatChar(KEY_ENTER); // DEBUG!!!

            // leave chat mode and notify that it was sent
            StopChatInput();
            // [Julia] TODO - REMOVE
            // M_StringCopy(lastmessage, chat_macros[c], sizeof(lastmessage));
            plr->message = lastmessage;
            eatkey = true;
        }
        else
        {
            c = ev->data3;
        
            eatkey = HUlib_keyInIText(&w_chat, c);
            if (eatkey)
            {
                // static unsigned char buf[20]; // DEBUG
                HU_queueChatChar(c);
        
                // M_snprintf(buf, sizeof(buf), "KEY: %d => %d", ev->data1, c);
                //        plr->message = buf;
            }
            if (c == KEY_ENTER)
            {
                StopChatInput();
                if (w_chat.l.len)
                {
                    M_StringCopy(lastmessage, w_chat.l.l, sizeof(lastmessage));
                    plr->message = lastmessage;
                }
            }
            else if (c == KEY_ESCAPE)
            {
                StopChatInput();
            }
        }
    }

    return eatkey;
}


// -----------------------------------------------------------------------------
// HU_WriteTextBig
//
// [Julia] Write a string using the BIG hu_font2
// -----------------------------------------------------------------------------

void HU_WriteTextBig (int x, int y, char *string)
{
    int    w, c, cx, cy;
    char  *ch;

    ch = string;
    cx = x;
    cy = y;

    while(1)
    {
        c = *ch++;
        if (!c)
        break;

        if (c == '\n')
        {
            cx = x;
            cy += 12;
            continue;
        }

        // Spacing between characters
        // Note: DON'T use toupper here, it capitalizes font.
        // c = toupper(c) - HU_FONTSTART2;
        c = c - HU_FONTSTART2;
        if (c < 0 || c>= HU_FONTSIZE2)
        {
            cx += 7;
            continue;
        }

        w = SHORT (hu_font2[c]->width);
        if (cx+w > ORIGWIDTH)
        break;

        V_DrawShadowedPatch(cx, cy, hu_font2[c]);

        // Place one char to another with one pixel
        cx += w-1;
    }
}


// -----------------------------------------------------------------------------
// HU_WriteTextBigCentered
//
// [Julia] Write a centered string using the BIG hu_font2. Only Y coord is set.
// -----------------------------------------------------------------------------

void HU_WriteTextBigCentered (int y, char *string)
{
    int    w, c, cx, cy, width;
    char  *ch;

    ch = string;
    width = 0;
    cx = ORIGWIDTH/2-width/2;
    cy = y;

    // find width
    while(ch)
    {
        c = *ch++;

        if (!c)
        break;

        c = c - HU_FONTSTART2;

        if (c < 0 || c> HU_FONTSIZE2)
        {
            width += 7;
            continue;
        }

        w = SHORT (hu_font2[c]->width);
        width += w;
    }

    // draw it
    cx = ORIGWIDTH/2-width/2;
    ch = string;

    while (ch)
    {
        c = *ch++;

        if (!c)
        break;

        c = c - HU_FONTSTART2;

        if (c < 0 || c> HU_FONTSIZE2)
        {
            cx += 7;
            continue;
        }

        w = SHORT (hu_font2[c]->width);

        V_DrawShadowedPatch(cx, cy, hu_font2[c]);

        // Place one char to another with one pixel
        cx+=w-1;
    }
}