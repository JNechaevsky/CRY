//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2025 Julia Nechaevskaya
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


#include "doomstat.h"
#include "ct_chat.h"
#include "v_video.h"
#include "z_zone.h"

#include "id_vars.h"


patch_t *hu_font_s[HU_FONTSIZE_S];
patch_t *hu_font_b[HU_FONTSIZE_B];

boolean     ultimatemsg;
const char *lastmessage;


// -----------------------------------------------------------------------------
// CT_Init
// [JN] Load the heads-up fonts.
// -----------------------------------------------------------------------------

void CT_Init (void)
{
    int   j = HU_FONTSTART;  // Small font.
	int   n = HU_FONTSTART2; // Big font.
	int   i;
    char  buffer[9];

    for (i = 0 ; i < HU_FONTSIZE_S ; i++)
    {
        snprintf(buffer, 9, "STCFN%.3d", j++);
        hu_font_s[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
    }
    for (i = 0 ; i < HU_FONTSIZE_B ; i++)
    {
        snprintf(buffer, 9, "CHAR_%.3d", n++);
        hu_font_b[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
    }
}

// -----------------------------------------------------------------------------
// CT_SetMessage
// [JN] Sets message parameters.
// -----------------------------------------------------------------------------

void CT_SetMessage (player_t *player, const char *message, boolean ultmsg, byte *table)
{
    lastmessage = message;

    if ((ultimatemsg || !msg_show) && !ultmsg)
    {
        return;
    }
    player->message = message;
    player->messageTics = MESSAGETICS;
    player->messageColor = table;

    if (ultmsg)
    {
        ultimatemsg = true;
    }
}

// -----------------------------------------------------------------------------
// CT_SetMessageCentered
// [JN] Sets centered message parameters.
// -----------------------------------------------------------------------------

void CT_SetMessageCentered (player_t *player, const char *message, byte *table)
{
    player->messageCentered = message;
    player->messageCenteredTics = 5*TICRATE/2; // [crispy] 2.5 seconds
    player->messageCenteredColor = table;
}

// -----------------------------------------------------------------------------
// CT_Ticker
// [JN] Reduces message tics independently from framerate and game states.
// -----------------------------------------------------------------------------

void CT_Ticker (void)
{
    player_t *player = &players[displayplayer];

    if (player->messageTics > 0)
    {
        player->messageTics--;
    }
    if (!player->messageTics)
    {                           // Refresh the screen when a message goes away
        ultimatemsg = false;    // clear out any chat messages.
    }
    if (player->messageCenteredTics > 0)
    {
        player->messageCenteredTics--;
    }
}
