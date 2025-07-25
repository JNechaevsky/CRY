//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2014-2017 RestlessRodent
// Copyright(C) 2015-2018 Fabian Greffrath
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


#include <stdio.h>

#include "v_trans.h"
#include "v_video.h"
#include "doomstat.h"
#include "m_menu.h"
#include "m_misc.h"
#include "p_local.h"

#include "id_vars.h"
#include "id_func.h"


// =============================================================================
//
//                        Render Counters and Widgets
//
// =============================================================================

ID_Render_t IDRender;
ID_Widget_t IDWidget;

char ID_Level_Time[64];
char ID_Total_Time[64];
char ID_Local_Time[64];

enum
{
    widget_kills,
    widget_items,
    widget_secret
} widgetcolor_t;

static byte *ID_WidgetColor (const int i)
{
    switch (i)
    {
        case widget_kills:
        {
            return
                IDWidget.totalkills == 0 ? cr[CR_GREEN] :
                IDWidget.kills == 0 ? cr[CR_RED] :
                IDWidget.kills < IDWidget.totalkills ? cr[CR_YELLOW] : cr[CR_GREEN];
            break;
        }
        case widget_items:
        {
            return
                IDWidget.totalitems == 0 ? cr[CR_GREEN] :
                IDWidget.items == 0 ? cr[CR_RED] :
                IDWidget.items < IDWidget.totalitems ? cr[CR_YELLOW] : cr[CR_GREEN];
            break;
        }
        case widget_secret:
        {
            return
                IDWidget.totalsecrets == 0 ? cr[CR_GREEN] :
                IDWidget.secrets == 0 ? cr[CR_RED] :
                IDWidget.secrets < IDWidget.totalsecrets ? cr[CR_YELLOW] : cr[CR_GREEN];
            break;
        }
    }
    return NULL;
}

// [JN/PN] Enum for widget type values.
enum
{
    widget_kis_kills,
    widget_kis_items,
    widget_kis_secrets,
} widget_kis_count_t;

// [PN] Function for safe division to prevent division by zero.
// Returns the percentage or 0 if the total is zero.
static int safe_percent (int value, int total)
{
    return (total == 0) ? 0 : (value * 100) / total;
}

// [PN/JN] Main function to format KIS counts based on format and widget type.
static void ID_WidgetKISCount (char *buffer, size_t buffer_size, const int i)
{
    int value = 0, total = 0;
    
    // [PN] Set values for kills, items, or secrets based on widget type
    switch (i)
    {
        case widget_kis_kills:
            value = IDWidget.kills;
            total = IDWidget.totalkills;
            break;
        
        case widget_kis_items:
            value = IDWidget.items;
            total = IDWidget.totalitems;
            break;
        
        case widget_kis_secrets:
            value = IDWidget.secrets;
            total = IDWidget.totalsecrets;
            break;
        
        default:
            // [PN] Default case for unsupported widget type
            snprintf(buffer, buffer_size, "N/A");
            return;
    }

    // [PN] Format based on widget_kis_format
    switch (widget_kis_format)
    {
        case 1: // Remaining
            snprintf(buffer, buffer_size, "%d", total - value);
            break;

        case 2: // Percent
            snprintf(buffer, buffer_size, "%d%%", 
                     safe_percent(value, total));
            break;

        default: // Ratio
            snprintf(buffer, buffer_size, "%d/%d", value, total);
            break;
    }
}

// -----------------------------------------------------------------------------
// ID_LeftWidgets.
//  [JN] Draw all the widgets and counters.
// -----------------------------------------------------------------------------

void ID_LeftWidgets (void)
{
    const int left_align = (widget_alignment == 0) ? -WIDESCREENDELTA :      // left
                           (widget_alignment == 1) ? 0                :      // status bar
                           (dp_screen_size   > 12  ? -WIDESCREENDELTA : 0);  // auto

    //
    // Located on the top
    //
    if (widget_location == 1)
    {
        // K/I/S stats
        if (widget_kis == 1
        || (widget_kis == 2 && automapactive))
        {
            char str1[16];  // kills
            char str2[16];  // items
            char str3[16];  // secret

            // Kills:
            M_WriteText(left_align, 9, "K:", cr[CR_GRAY]);
            ID_WidgetKISCount(str1, sizeof(str1), widget_kis_kills);
            M_WriteText(left_align + 16, 9, str1, ID_WidgetColor(widget_kills));

            // Items:
            M_WriteText(left_align, 18, "I:", cr[CR_GRAY]);
            ID_WidgetKISCount(str2, sizeof(str2), widget_kis_items);
            M_WriteText(left_align + 16, 18, str2, ID_WidgetColor(widget_items));

            // Secret:
            M_WriteText(left_align, 27, "S:", cr[CR_GRAY]);
            ID_WidgetKISCount(str3, sizeof(str3), widget_kis_secrets);
            M_WriteText(left_align + 16, 27, str3, ID_WidgetColor(widget_secret));
        }

        // Level timer. Time gathered in G_Ticker.
        if (widget_time == 1
        || (widget_time == 2 && automapactive))
        {
            M_WriteText(left_align, 40, "TIME", cr[CR_GRAY]);
            M_WriteText(left_align, 49, ID_Level_Time, cr[CR_LIGHTGRAY]);
        }

        // Total time. Time gathered in G_Ticker.
        if (widget_totaltime == 1
        || (widget_totaltime == 2 && automapactive))
        {
            M_WriteText(left_align, 58, "TOTAL", cr[CR_GRAY]);
            M_WriteText(left_align, 67, ID_Total_Time, cr[CR_LIGHTGRAY]);
        }

        // Player coords
        if (widget_coords == 1
        || (widget_coords == 2 && automapactive))
        {
            char str[128];

            M_WriteText(left_align, 80, "X:", cr[CR_GRAY]);
            M_WriteText(left_align, 89, "Y:", cr[CR_GRAY]);
            M_WriteText(left_align, 98, "ANG:", cr[CR_GRAY]);

            sprintf(str, "%d", IDWidget.x);
            M_WriteText(left_align + 16, 80, str, cr[CR_GREEN]);
            sprintf(str, "%d", IDWidget.y);
            M_WriteText(left_align + 16, 89, str, cr[CR_GREEN]);
            sprintf(str, "%d", IDWidget.ang);
            M_WriteText(left_align + 32, 98, str, cr[CR_GREEN]);
        }

        // Render counters
        if (widget_render)
        {
            char spr[32];
            char seg[32];
            char opn[64];
            char vis[32];

            // Sprites
            M_WriteText(left_align, 111, "SPR:", cr[CR_GRAY]);
            M_snprintf(spr, 16, "%d", IDRender.numsprites);
            M_WriteText(32 + left_align, 111, spr, cr[CR_GREEN]);

            // Segments (256 max)
            M_WriteText(left_align, 120, "SEG:", cr[CR_GRAY]);
            M_snprintf(seg, 16, "%d", IDRender.numsegs);
            M_WriteText(32 + left_align, 120, seg, cr[CR_GREEN]);

            // Openings
            M_WriteText(left_align, 129, "OPN:", cr[CR_GRAY]);
            M_snprintf(opn, 16, "%d", IDRender.numopenings);
            M_WriteText(32 + left_align, 129, opn, cr[CR_GREEN]);

            // Planes
            M_WriteText(left_align, 138, "PLN:", cr[CR_GRAY]);
            M_snprintf(vis, 32, "%d", IDRender.numplanes);
            M_WriteText(32 + left_align, 138, vis, cr[CR_GREEN]);
        }
    }
    //
    // Located at the bottom
    //
    else
    {
        int yy = 0;

        // Shift widgets one line up if Level Name widget is set to "always".
        if (widget_levelname && !automapactive)
        {
            yy -= 9;
        }

        // Render counters
        if (widget_render)
        {
            char spr[32];
            char seg[32];
            char opn[64];
            char vis[32];
            const int yy1 = widget_coords ? 0 : 34;

            // Sprites
            M_WriteText(left_align, 46 + yy1, "SPR:", cr[CR_GRAY]);
            M_snprintf(spr, 16, "%d", IDRender.numsprites);
            M_WriteText(32 + left_align, 46 + yy1, spr, cr[CR_GREEN]);

            // Segments (256 max)
            M_WriteText(left_align, 55 + yy1, "SEG:", cr[CR_GRAY]);
            M_snprintf(seg, 16, "%d", IDRender.numsegs);
            M_WriteText(32 + left_align, 55 + yy1, seg, cr[CR_GREEN]);

            // Openings
            M_WriteText(left_align, 64 + yy1, "OPN:", cr[CR_GRAY]);
            M_snprintf(opn, 16, "%d", IDRender.numopenings);
            M_WriteText(32 + left_align, 64 + yy1, opn, cr[CR_GREEN]);

            // Planes
            M_WriteText(left_align, 73 + yy1, "PLN:", cr[CR_GRAY]);
            M_snprintf(vis, 32, "%d", IDRender.numplanes);
            M_WriteText(32 + left_align, 73 + yy1, vis, cr[CR_GREEN]);
        }

        // Player coords
        if (widget_coords == 1
        || (widget_coords == 2 && automapactive))
        {
            char str[128];

            M_WriteText(left_align, 89, "X:", cr[CR_GRAY]);
            M_WriteText(left_align, 98, "Y:", cr[CR_GRAY]);
            M_WriteText(left_align, 107, "ANG:", cr[CR_GRAY]);

            sprintf(str, "%d", IDWidget.x);
            M_WriteText(16 + left_align, 89, str, cr[CR_GREEN]);
            sprintf(str, "%d", IDWidget.y);
            M_WriteText(16 + left_align, 98, str, cr[CR_GREEN]);
            sprintf(str, "%d", IDWidget.ang);
            M_WriteText(32 + left_align, 107, str, cr[CR_GREEN]);
        }

        if (automapactive)
        {
            yy -= 9;
        }

        // K/I/S stats
        if (widget_kis == 1
        || (widget_kis == 2 && automapactive))
        {
            char str1[8], str2[16];  // kills
            char str3[8], str4[16];  // items
            char str5[8], str6[16];  // secret
    
            // Kills:
            sprintf(str1, "K ");
            M_WriteText(left_align, 152 + yy, str1, cr[CR_GRAY]);
            ID_WidgetKISCount(str2, sizeof(str2), widget_kis_kills);
            M_WriteText(left_align + M_StringWidth(str1), 152 + yy, str2, ID_WidgetColor(widget_kills));
    
            // Items:
            sprintf(str3, " I ");
            M_WriteText(left_align + M_StringWidth(str1) + M_StringWidth(str2), 152 + yy, str3, cr[CR_GRAY]);
            
            ID_WidgetKISCount(str4, sizeof(str4), widget_kis_items);
            M_WriteText(left_align + M_StringWidth(str1) +
                        M_StringWidth(str2) +
                        M_StringWidth(str3), 152 + yy, str4, ID_WidgetColor(widget_items));
    
            // Secret:
            sprintf(str5, " S ");
            M_WriteText(left_align + M_StringWidth(str1) +
                        M_StringWidth(str2) +
                        M_StringWidth(str3) +
                        M_StringWidth(str4), 152 + yy, str5, cr[CR_GRAY]);
    
            ID_WidgetKISCount(str6, sizeof(str6), widget_kis_secrets);
            M_WriteText(left_align + M_StringWidth(str1) +
                        M_StringWidth(str2) + 
                        M_StringWidth(str3) +
                        M_StringWidth(str4) +
                        M_StringWidth(str5), 152 + yy, str6, ID_WidgetColor(widget_secret));
        }

        if (widget_kis)
        {
            yy -= 9;
        }

        // Total time. Time gathered in G_Ticker.
        if (widget_totaltime == 1
        || (widget_totaltime == 2 && automapactive))
        {
            char stra[8];

            sprintf(stra, "TOTAL ");
            M_WriteText(left_align, 152 + yy, stra, cr[CR_GRAY]);
            M_WriteText(left_align + M_StringWidth(stra), 152 + yy, ID_Total_Time, cr[CR_LIGHTGRAY]);
        }

        if (widget_totaltime)
        {
            yy -= 9;
        }

        // Level timer. Time gathered in G_Ticker.
        if (widget_time == 1
        || (widget_time == 2 && automapactive))
        {
            char stra[8];

            sprintf(stra, "TIME ");
            M_WriteText(left_align, 152 + yy, stra, cr[CR_GRAY]);
            M_WriteText(left_align + M_StringWidth(stra), 152 + yy, ID_Level_Time, cr[CR_LIGHTGRAY]);
        }
    }
}

// -----------------------------------------------------------------------------
// ID_RightWidgets.
//  [JN] Draw actual frames per second value.
//  Some M_StringWidth adjustments are needed for proper positioning
//  in case of custom font is thinner or thicker.
// -----------------------------------------------------------------------------

void ID_RightWidgets (void)
{
    int yy = 0;

    // [JN] FPS counter
    if (vid_showfps)
    {
        char fps[8];
        char fps_str[4];

        sprintf(fps, "%d", id_fps_value);
        sprintf(fps_str, "FPS");

        M_WriteText(ORIGWIDTH + WIDESCREENDELTA - 11 - M_StringWidth(fps) 
                              - M_StringWidth(fps_str), 9 + yy, fps, cr[CR_LIGHTGRAY_DARK]);

        M_WriteText(ORIGWIDTH + WIDESCREENDELTA - 7 - M_StringWidth(fps_str), 9 + yy, "FPS", cr[CR_LIGHTGRAY_DARK]);

        yy += 9;
    }

    // [JN] Local time. Time gathered in G_Ticker.
    if (msg_local_time)
    {
        M_WriteText(ORIGWIDTH + WIDESCREENDELTA - 7
                              - M_StringWidth(ID_Local_Time), 9 + yy, ID_Local_Time, cr[CR_GRAY]);
    }
}

// -----------------------------------------------------------------------------
// ID_HealthColor, ID_DrawTargetsHealth
//  [JN/PN] Indicates and colorizes current target's health.
// -----------------------------------------------------------------------------

static byte *ID_HealthColor (const int val1, const int val2)
{
    return
        val1 <= val2/4 ? cr[CR_RED]    :
        val1 <= val2/2 ? cr[CR_YELLOW] :
                         cr[CR_GREEN]  ;
}

void ID_DrawTargetsHealth (void)
{
    char  str[16];
    const player_t *player = &players[displayplayer];
    byte *color;

    if (player->targetsheathTics <= 0 || !player->targetsheath)
    {
        return;  // No tics or target is dead, nothing to display.
    }

    snprintf(str, sizeof(str), "%d/%d", player->targetsheath, player->targetsmaxheath);
    color = ID_HealthColor(player->targetsheath, player->targetsmaxheath);

    switch (widget_health)
    {
        case 1:  // Top
            M_WriteTextCentered(18, str, color);
            break;
        case 2:  // Top + name
            M_WriteTextCentered(9, player->targetsname, color);
            M_WriteTextCentered(18, str, color);
            break;
        case 3:  // Bottom
            M_WriteTextCentered(152, str, color);
            break;
        case 4:  // Bottom + name
            M_WriteTextCentered(144, player->targetsname, color);
            M_WriteTextCentered(152, str, color);
            break;
    }
}

// =============================================================================
//
//                                 Crosshair
//
// =============================================================================

// -----------------------------------------------------------------------------
// [JN] Crosshair graphical patches in Doom GFX format.
// -----------------------------------------------------------------------------

static const byte xhair_cross1[] =
{
    0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 
    0x2A, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x31, 0x00, 0x00, 0x00, 
    0x3E, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x45, 0x00, 0x00, 0x00, 
    0x03, 0x01, 0x5A, 0x5A, 0x5A, 0xFF, 0x03, 0x01, 0x53, 0x53, 0x53, 0xFF, 
    0xFF, 0x00, 0x02, 0x5A, 0x5A, 0x53, 0x53, 0x05, 0x02, 0x53, 0x53, 0x5A, 
    0x5A, 0xFF, 0xFF, 0x03, 0x01, 0x53, 0x53, 0x53, 0xFF, 0x03, 0x01, 0x5A, 
    0x5A, 0x5A, 0xFF
};

static const byte xhair_cross2[] =
{
    0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 
    0x25, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x00, 0x00, 
    0x34, 0x00, 0x00, 0x00, 0x3A, 0x00, 0x00, 0x00, 0x3B, 0x00, 0x00, 0x00, 
    0xFF, 0xFF, 0x03, 0x01, 0x5A, 0x5A, 0x5A, 0xFF, 0x02, 0x03, 0x5A, 0x5A, 
    0x53, 0x5A, 0x5A, 0xFF, 0x03, 0x01, 0x5A, 0x5A, 0x5A, 0xFF, 0xFF, 0xFF
};

static const byte xhair_x[] =
{
    0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 
    0x25, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x3B, 0x00, 0x00, 0x00, 
    0x3C, 0x00, 0x00, 0x00, 0x47, 0x00, 0x00, 0x00, 0x52, 0x00, 0x00, 0x00, 
    0xFF, 0x01, 0x01, 0x5A, 0x5A, 0x5A, 0x05, 0x01, 0x5A, 0x5A, 0x5A, 0xFF, 
    0x02, 0x01, 0x53, 0x53, 0x53, 0x04, 0x01, 0x53, 0x53, 0x53, 0xFF, 0xFF, 
    0x02, 0x01, 0x53, 0x53, 0x53, 0x04, 0x01, 0x53, 0x53, 0x53, 0xFF, 0x01, 
    0x01, 0x5A, 0x5A, 0x5A, 0x05, 0x01, 0x5A, 0x5A, 0x5A, 0xFF, 0xFF
};

static const byte xhair_circle[] =
{
    0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 
    0x25, 0x00, 0x00, 0x00, 0x2D, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 
    0x43, 0x00, 0x00, 0x00, 0x4E, 0x00, 0x00, 0x00, 0x56, 0x00, 0x00, 0x00, 
    0xFF, 0x02, 0x03, 0x5A, 0x5A, 0x53, 0x5A, 0x5A, 0xFF, 0x01, 0x01, 0x5A, 
    0x5A, 0x5A, 0x05, 0x01, 0x5A, 0x5A, 0x5A, 0xFF, 0x01, 0x01, 0x53, 0x53, 
    0x53, 0x05, 0x01, 0x53, 0x53, 0x53, 0xFF, 0x01, 0x01, 0x5A, 0x5A, 0x5A, 
    0x05, 0x01, 0x5A, 0x5A, 0x5A, 0xFF, 0x02, 0x03, 0x5A, 0x5A, 0x53, 0x5A, 
    0x5A, 0xFF, 0xFF
};

static const byte xhair_angle[] =
{
    0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 
    0x25, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00, 0x00, 
    0x2F, 0x00, 0x00, 0x00, 0x35, 0x00, 0x00, 0x00, 0x3B, 0x00, 0x00, 0x00, 
    0xFF, 0xFF, 0xFF, 0x03, 0x03, 0x53, 0x53, 0x5A, 0x60, 0x60, 0xFF, 0x03, 
    0x01, 0x5A, 0x5A, 0x5A, 0xFF, 0x03, 0x01, 0x60, 0x60, 0x60, 0xFF, 0xFF
};

static const byte xhair_triangle[] =
{
    0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 
    0x25, 0x00, 0x00, 0x00, 0x2B, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 
    0x3D, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00, 0x4A, 0x00, 0x00, 0x00, 
    0xFF, 0x05, 0x01, 0x60, 0x60, 0x60, 0xFF, 0x04, 0x02, 0x5A, 0x5A, 0x5A, 
    0x5A, 0xFF, 0x03, 0x01, 0x53, 0x53, 0x53, 0x05, 0x01, 0x53, 0x53, 0x53, 
    0xFF, 0x04, 0x02, 0x5A, 0x5A, 0x5A, 0x5A, 0xFF, 0x05, 0x01, 0x60, 0x60, 
    0x60, 0xFF, 0xFF
};

static const byte xhair_dot[] =
{
    0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 
    0x25, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00, 0x00, 
    0x2D, 0x00, 0x00, 0x00, 0x2E, 0x00, 0x00, 0x00, 0x2F, 0x00, 0x00, 0x00, 
    0xFF, 0xFF, 0xFF, 0x03, 0x01, 0x53, 0x53, 0x53, 0xFF, 0xFF, 0xFF, 0xFF
};

// -----------------------------------------------------------------------------
// ID_CrosshairShape
//  [JN] Decides which patch should be drawn, depending on "xhair_draw" variable.
// -----------------------------------------------------------------------------

static patch_t *ID_CrosshairShape (void)
{
    // [PN] Array of crosshair shapes with explicit type casting
    patch_t *crosshair_shapes[] = {
        NULL,                        // xhair_draw == 0 (no crosshair)
        (patch_t*) &xhair_cross1,    // xhair_draw == 1
        (patch_t*) &xhair_cross2,    // xhair_draw == 2
        (patch_t*) &xhair_x,         // xhair_draw == 3
        (patch_t*) &xhair_circle,    // xhair_draw == 4
        (patch_t*) &xhair_angle,     // xhair_draw == 5
        (patch_t*) &xhair_triangle,  // xhair_draw == 6
        (patch_t*) &xhair_dot,       // xhair_draw == 7
    };

    // [PN] Return the appropriate crosshair shape
    return crosshair_shapes[xhair_draw];
}

// -----------------------------------------------------------------------------
// ID_CrosshairColor
//  [JN/PN] Determines crosshair color depending on "xhair_color" variable.
//  Supports static, health-based, target highlight, and combined modes.
// -----------------------------------------------------------------------------

static byte *ID_CrosshairColor (int type)
{
    const player_t *player = &players[displayplayer];

    switch (type)
    {
        case 0:
            // Static/uncolored.
            return cr[CR_RED];

        case 1:
            // Health-based coloring.
            // Same logic as status bar coloring (ST_WidgetColor).
            return player->health >= 67 ? cr[CR_GREEN]  :
                   player->health >= 34 ? cr[CR_YELLOW] :
                                          cr[CR_RED];

        case 2:
            // Target highlight.
            // "linetarget" is set by intercept-safe P_AimLineAttack in G_Ticker.
            return linetarget ? cr[CR_BLUE2] : cr[CR_RED];

        case 3:
            // Target highlight+health.
            return linetarget           ? cr[CR_BLUE2]  :
                   player->health >= 67 ? cr[CR_GREEN]  :
                   player->health >= 34 ? cr[CR_YELLOW] :
                                          cr[CR_RED];
    }

    return NULL;
}

// -----------------------------------------------------------------------------
// ID_DrawCrosshair
//  [JN] Drawing routine, called via D_Display.
// -----------------------------------------------------------------------------

void ID_DrawCrosshair (void)
{
    const int xx = (ORIGWIDTH  / 2) - 3;
    const int yy = (ORIGHEIGHT / 2) - 3 - (dp_screen_size <= 10 ? 16 : 0);

    dp_translation = ID_CrosshairColor(xhair_color);
    V_DrawPatch(xx, yy, ID_CrosshairShape());
    dp_translation = NULL;
}

// =============================================================================
//
//                              Spectator Mode
//
// =============================================================================

// Camera position and orientation.
fixed_t CRL_camera_x, CRL_camera_y, CRL_camera_z;
angle_t CRL_camera_ang;

// [JN] An "old" position and orientation used for interpolation.
fixed_t CRL_camera_oldx, CRL_camera_oldy, CRL_camera_oldz;
angle_t CRL_camera_oldang;

// -----------------------------------------------------------------------------
// CRL_GetCameraPos
//  Returns the camera position.
// -----------------------------------------------------------------------------

void CRL_GetCameraPos (fixed_t *x, fixed_t *y, fixed_t *z, angle_t *a)
{
    *x = CRL_camera_x;
    *y = CRL_camera_y;
    *z = CRL_camera_z;
    *a = CRL_camera_ang;
}

// -----------------------------------------------------------------------------
// CRL_ReportPosition
//  Reports the position of the camera.
//  @param x The x position.
//  @param y The y position.
//  @param z The z position.
//  @param angle The angle used.
// -----------------------------------------------------------------------------

void CRL_ReportPosition (fixed_t x, fixed_t y, fixed_t z, angle_t angle)
{
	CRL_camera_oldx = x;
	CRL_camera_oldy = y;
	CRL_camera_oldz = z;
	CRL_camera_oldang = angle;
}

// -----------------------------------------------------------------------------
// CRL_ImpulseCamera
//  @param fwm Forward movement.
//  @param swm Sideways movement.
//  @param at Angle turning.
// -----------------------------------------------------------------------------

void CRL_ImpulseCamera (fixed_t fwm, fixed_t swm, angle_t at)
{
    // [JN/PN] Precalculate values:
    const fixed_t fwm_val = fwm * 32768;
    const fixed_t swm_val = swm * 32768;

    // Rotate camera first
    CRL_camera_ang += at << FRACBITS;

    // Forward movement
    at = CRL_camera_ang >> ANGLETOFINESHIFT;
    CRL_camera_x += FixedMul(fwm_val, finecosine[at]); 
    CRL_camera_y += FixedMul(fwm_val, finesine[at]);

    // Sideways movement
    at = (CRL_camera_ang - ANG90) >> ANGLETOFINESHIFT;
    CRL_camera_x += FixedMul(swm_val, finecosine[at]); 
    CRL_camera_y += FixedMul(swm_val, finesine[at]);
}

// -----------------------------------------------------------------------------
// CRL_ImpulseCameraVert
//  [JN/PN] Impulses the camera up/down.
//  @param direction: true = up, false = down.
//  @param intensity: 32 of 64 map unit, depending on player run mode.
// -----------------------------------------------------------------------------

void CRL_ImpulseCameraVert (boolean direction, fixed_t intensity)
{
    CRL_camera_z += (direction ? 1 : -1) * FRACUNIT * intensity;
}
