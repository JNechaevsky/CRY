//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2014-2017 RestlessRodent
// Copyright(C) 2015-2018 Fabian Greffrath
// Copyright(C) 2016-2026 Julia Nechaevskaya
// Copyright(C) 2024-2026 Polina "Aura" N.
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
#include "mn_menu.h"
#include "m_misc.h"
#include "p_local.h"
#include "ct_chat.h"

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

// [JN] Enum for widget strings and values.
enum
{
    widget_kis_str,
    widget_kills,
    widget_items,
    widget_secret,
    widget_plyr1,
    widget_plyr2,
    widget_plyr3,
    widget_plyr4,
    widget_time_str,
    widget_time_val,
    widget_render_str,
    widget_render_val,
    widget_coords_str,
    widget_coords_val,
    widget_speed_str,
    widget_speed_val,
} widgetcolor_t;

static byte *ID_WidgetColor (const int i)
{
    if (!widget_scheme)
    {
        return NULL;
    }

    static byte *player_colors[4];
    static const int plyr_indices[] = {widget_plyr1, widget_plyr2, widget_plyr3, widget_plyr4};

    player_colors[0] = cr[CR_GREEN];
    player_colors[1] = cr[CR_GRAY];
    player_colors[2] = cr[CR_BROWN];
    player_colors[3] = cr[CR_RED];

    switch (widget_scheme)
    {
        case 1: // Inter
            switch (i)
            {
                case widget_kis_str:
                case widget_time_str:
                case widget_render_str:
                case widget_coords_str:
                case widget_speed_str:
                    return cr[CR_GRAY];
                
                case widget_kills:
                    return
                        IDWidget.totalkills == 0 ? cr[CR_GREEN] :
                        IDWidget.kills == 0 ? cr[CR_RED] :
                        IDWidget.kills < IDWidget.totalkills ? cr[CR_YELLOW] : cr[CR_GREEN];
                case widget_items:
                    return
                        IDWidget.totalitems == 0 ? cr[CR_GREEN] :
                        IDWidget.items == 0 ? cr[CR_RED] :
                        IDWidget.items < IDWidget.totalitems ? cr[CR_YELLOW] : cr[CR_GREEN];
                case widget_secret:
                    return
                        IDWidget.totalsecrets == 0 ? cr[CR_GREEN] :
                        IDWidget.secrets == 0 ? cr[CR_RED] :
                        IDWidget.secrets < IDWidget.totalsecrets ? cr[CR_YELLOW] : cr[CR_GREEN];

                case widget_time_val:
                    return cr[CR_LIGHTGRAY];

                case widget_render_val:
                case widget_coords_val:
                case widget_speed_val:
                    return cr[CR_GREEN];

                default:
                    for (int j = 0; j < 4; j++)
                    {
                        if (i == plyr_indices[j])
                        {
                            return player_colors[j];
                        }
                    }
            }
        break;

        case 2: // Crispy
            switch (i)
            {
                case widget_kis_str:
                case widget_time_str:
                case widget_speed_str:
                    return cr[CR_RED];
                case widget_render_str:
                    return cr[CR_YELLOW];
                case widget_coords_str:
                    return cr[CR_GREEN];

                default:
                    for (int j = 0; j < 4; j++)
                    {
                        if (i == plyr_indices[j])
                        {
                            return player_colors[j];
                        }
                    }
                    return cr[CR_WHITE];
            }
        break;

        case 3: // Woof
            switch (i)
            {
                case widget_kis_str:
                case widget_time_str:
                    return cr[CR_RED];

                case widget_kills:
                    return
                        IDWidget.totalkills == 0 ? cr[CR_BLUE] :
                        IDWidget.kills < IDWidget.totalkills ? cr[CR_WHITE] : cr[CR_BLUE];
                
                case widget_items:
                    return
                        IDWidget.totalitems == 0 ? cr[CR_BLUE] :
                        IDWidget.items < IDWidget.totalitems ? cr[CR_WHITE] : cr[CR_BLUE];
                
                case widget_secret:
                    return
                        IDWidget.totalsecrets == 0 ? cr[CR_BLUE] :
                        IDWidget.secrets < IDWidget.totalsecrets ? cr[CR_WHITE] : cr[CR_BLUE];

                case widget_render_str:
                case widget_render_val:
                case widget_coords_str:
                case widget_speed_str:
                    return cr[CR_GREEN];

                default:
                    for (int j = 0; j < 4; j++)
                    {
                        if (i == plyr_indices[j])
                        {
                            return player_colors[j];
                        }
                    }
                    return cr[CR_WHITE];
            }
        break;

        case 4: // DSDA
            switch (i)
            {
                case widget_kis_str:
                    return cr[CR_RED];

                case widget_kills:
                    return
                        IDWidget.totalkills == 0 ? cr[CR_BLUE] :
                        IDWidget.kills < IDWidget.totalkills ? cr[CR_YELLOW] : cr[CR_BLUE];
                
                case widget_items:
                    return
                        IDWidget.totalitems == 0 ? cr[CR_BLUE] :
                        IDWidget.items < IDWidget.totalitems ? cr[CR_YELLOW] : cr[CR_BLUE];
                
                case widget_secret:
                    return
                        IDWidget.totalsecrets == 0 ? cr[CR_BLUE] :
                        IDWidget.secrets < IDWidget.totalsecrets ? cr[CR_YELLOW] : cr[CR_BLUE];

                case widget_time_str:
                case widget_render_str:
                    return cr[CR_WHITE];

                case widget_render_val:
                    return cr[CR_YELLOW];

                case widget_time_val:
                case widget_coords_str:
                case widget_coords_val:
                case widget_speed_val:
                    return cr[CR_GREEN];

                default:
                    for (int j = 0; j < 4; j++)
                    {
                        if (i == plyr_indices[j])
                        {
                            return player_colors[j];
                        }
                    }
                    return cr[CR_WHITE];
            }
        break;
    }
    return NULL;
}

// [JN/PN] Enum for widget type values.
enum
{
    widgets_kis_kills,
    widgets_kis_items,
    widgets_kis_secrets,
} widgets_kis_count_t;

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
        case widgets_kis_kills:
            value = IDWidget.kills;
            total = IDWidget.totalkills;
            break;
        
        case widgets_kis_items:
            value = IDWidget.items;
            total = IDWidget.totalitems;
            break;
        
        case widgets_kis_secrets:
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
        {
            // [JN] Prevent negative values.
            const int total_value = (total - value > 0) ? (total - value) : 0;
            snprintf(buffer, buffer_size, "%d", total_value);
            break;
        }

        case 2: // Percent
        {
            snprintf(buffer, buffer_size, "%d%%", safe_percent(value, total));
            break;
        }

        case 3: // Count
        {
            snprintf(buffer, buffer_size, "%d", value);
            break;
        }

        default: // Ratio
        {
            snprintf(buffer, buffer_size, "%d/%d", value, total);
            break;
        }
    }
}

// -----------------------------------------------------------------------------
// ID_LeftWidgets.
//  [JN/PN] Draw all the widgets and counters.
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
            int yy = 0;

            // Kills:
            fontfunc(left_align, 9, "K:", cr[CR_GRAY]);
            char buf1[16];
            ID_WidgetKISCount(buf1, sizeof(buf1), widgets_kis_kills);
            fontfunc(left_align + widthfunc("K: "), 9, buf1, ID_WidgetColor(widget_kills));

            // Items:
            if (widget_kis_items == 1
            || (widget_kis_items == 2 && automapactive))
            {
            fontfunc(left_align, 18, "I:", cr[CR_GRAY]);
                    char buf2[16];
                    ID_WidgetKISCount(buf2, sizeof(buf2), widgets_kis_items);
                    fontfunc(left_align + widthfunc("I: "), 18, buf2, ID_WidgetColor(widget_items));
            }
            else
            {
            yy = 9;
            }

            // Secret:
            fontfunc(left_align, 27 - yy, "S:", cr[CR_GRAY]);
                char buf3[16];
                ID_WidgetKISCount(buf3, sizeof(buf3), widgets_kis_secrets);
                fontfunc(left_align + widthfunc("S: "), 27 - yy, buf3, ID_WidgetColor(widget_secret));
        }

        // Level timer. Time gathered in G_Ticker.
        if (widget_time == 1
        || (widget_time == 2 && automapactive))
        {
            fontfunc(left_align, 40, "TIME", ID_WidgetColor(widget_time_str));
            fontfunc(left_align, 49, ID_Level_Time, ID_WidgetColor(widget_time_val));
        }

        // Total time. Time gathered in G_Ticker.
        if (widget_totaltime == 1
        || (widget_totaltime == 2 && automapactive))
        {
            fontfunc(left_align, 58, "TOTAL", ID_WidgetColor(widget_time_str));
            fontfunc(left_align, 67, ID_Total_Time, ID_WidgetColor(widget_time_val));
        }

        // Player coords
        if (widget_coords == 1
        || (widget_coords == 2 && automapactive))
        {
            struct { const char *label; int y; int *value; } coords[] = {
                {"X: ", 80, &IDWidget.x},
                {"Y: ", 89, &IDWidget.y},
                {"ANG: ", 98, &IDWidget.ang}
            };
            for (int i = 0; i < 3; i++)
            {
                fontfunc(left_align, coords[i].y, coords[i].label,
                            ID_WidgetColor(widget_coords_str));
                char buf[32];
                sprintf(buf, "%d", *coords[i].value);
                fontfunc(left_align + widthfunc(coords[i].label), coords[i].y, buf, ID_WidgetColor(widget_coords_val));
            }
        }

        // Render counters
        if (widget_render)
        {
            struct { const char *label; int y; int value; } counters[] = {
                {"SPR: ", 111, IDRender.numsprites},
                {"SEG: ", 120, IDRender.numsegs},
                {"OPN: ", 129, IDRender.numopenings},
                {"PLN: ", 138, IDRender.numplanes}
            };
            for (int i = 0; i < 4; i++)
            {
                fontfunc(left_align, counters[i].y, counters[i].label,
                            ID_WidgetColor(widget_render_str));
                char buf[32];
                M_snprintf(buf, sizeof(buf), "%d", counters[i].value);
                fontfunc(left_align + widthfunc(counters[i].label), counters[i].y, buf, ID_WidgetColor(widget_render_val));
            }
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
            const int base_y = 46 + (widget_coords ? 0 : 34);
            struct { const char *label; int value; } counters[] = {
                {"SPR: ", IDRender.numsprites},
                {"SEG: ", IDRender.numsegs},
                {"OPN: ", IDRender.numopenings},
                {"PLN: ", IDRender.numplanes}
            };

            for (int i = 0; i < 4; i++)
            {
                const int y = base_y + 9 * i;
                fontfunc(left_align, y, counters[i].label, ID_WidgetColor(widget_render_str));
                char buf[32];
                M_snprintf(buf, sizeof(buf), "%d", counters[i].value);
                fontfunc(left_align + widthfunc(counters[i].label) /*32*/, y, buf, ID_WidgetColor(widget_render_val));
            }
        }

        // Player coords
        if (widget_coords == 1
        || (widget_coords == 2 && automapactive))
        {
            struct { const char *label; int y; int *value; } coords[] = {
                {"X: ", 89, &IDWidget.x},
                {"Y: ", 98, &IDWidget.y},
                {"ANG: ", 107, &IDWidget.ang}
            };
            for (int i = 0; i < 3; i++)
            {
                fontfunc(left_align, coords[i].y, coords[i].label, ID_WidgetColor(widget_coords_str));
                char buf[32];
                sprintf(buf, "%d", *coords[i].value);
                fontfunc(left_align + widthfunc(coords[i].label), coords[i].y, buf, ID_WidgetColor(widget_coords_val));
            }
        }

        if (automapactive)
        {
            yy -= 9;
        }

        // K/I/S stats
        if (widget_kis == 1
        || (widget_kis == 2 && automapactive))
        {
            const char *labels[] = {"K ", " I ", " S "};
            const int items_on = (widget_kis_items == 1 || (widget_kis_items == 2 && automapactive));
            const int active[] = {1, items_on, 1};  // kills, items (optional), secrets
            const int params[] = {widgets_kis_kills, widgets_kis_items, widgets_kis_secrets};
            const int colors[] = {widget_kills, widget_items, widget_secret};
            int x = left_align;
        
            for (int i = 0; i < 3; i++)
            {
                if (!active[i]) continue;
                fontfunc(x, 152 + yy, labels[i], ID_WidgetColor(widget_kis_str));
                x += widthfunc(labels[i]);
        
                char buf[16];
                ID_WidgetKISCount(buf, sizeof(buf), params[i]);
                fontfunc(x, 152 + yy, buf, ID_WidgetColor(colors[i]));
                x += widthfunc(buf);
            }
        }

        if (widget_kis)
        {
            yy -= 9;
        }

        // Level / DeathMatch and Total time. Time gathered in G_Ticker.
        struct { int cond; const char *label; const char *value; } times[] = {
            {widget_totaltime, "TOTAL ", ID_Total_Time},
            {widget_time,      "TIME ",  ID_Level_Time}
        };
        for (int i = 0; i < 2; i++)
        {
            const int c = times[i].cond;
            if (c == 1 || c == 3 || ((c == 2 || c == 4) && automapactive))
            {
                fontfunc(left_align, 152 + yy, times[i].label, ID_WidgetColor(widget_time_str));
                fontfunc(left_align + widthfunc(times[i].label), 152 + yy, 
                            times[i].value, ID_WidgetColor(widget_time_val));
            }
            if (c) yy -= 9;
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
    int yy = 9;

    // [JN/PN] FPS counter
    if (vid_showfps)
    {
        int  fps_x_pos;
        char fps[8];

        sprintf(fps, "%d", id_fps_value);
        fps_x_pos = ORIGWIDTH + WIDESCREENDELTA - 11 
                  - widthfunc(fps) - widthfunc("FPS");

        fontfunc(fps_x_pos, yy, fps, cr[CR_LIGHTGRAY_DARK]);
        fontfunc(fps_x_pos + widthfunc(fps) + 4, yy, "FPS", cr[CR_LIGHTGRAY_DARK]); // [PN] 4 for spacing

        yy += 9;
    }

    // [JN] Local time. Time gathered in G_Ticker.
    if (msg_local_time)
    {
        fontfunc(ORIGWIDTH + WIDESCREENDELTA - 7
                              - widthfunc(ID_Local_Time), yy, ID_Local_Time, cr[CR_GRAY]);
    }
}

// -----------------------------------------------------------------------------
// ID_HealthColor, ID_DrawTargetsHealth
//  [JN/PN] Indicates and colorizes current target's health.
// -----------------------------------------------------------------------------

static byte *ID_HealthColor (const int val1, const int val2)
{
    return
        !widget_scheme ? NULL          :
        val1 <= val2/4 ? cr[CR_RED]    :
        val1 <= val2/2 ? cr[CR_YELLOW] :
                         cr[CR_GREEN]  ;
}

void ID_DrawTargetsHealth (void)
{
    const player_t *player = &players[displayplayer];

    if (player->targetsheathTics <= 0 || !player->targetsheath)
    {
        return;  // No tics or target is dead, nothing to display.
    }

    const int yy = widget_speed ? 9 : 0;
    char  str[16];
    snprintf(str, sizeof(str), "%d/%d", player->targetsheath, player->targetsmaxheath);
    byte *color = ID_HealthColor(player->targetsheath, player->targetsmaxheath);

    switch (widget_health)
    {
        case 1:  // Top
            fontcenteredfunc(18, str, color);
            break;
        case 2:  // Top + name
            fontcenteredfunc(9, player->targetsname, color);
            fontcenteredfunc(18, str, color);
            break;
        case 3:  // Bottom
            fontcenteredfunc(152 - yy, str, color);
            break;
        case 4:  // Bottom + name
            fontcenteredfunc(144 - yy, player->targetsname, color);
            fontcenteredfunc(152 - yy, str, color);
            break;
    }
}

// -----------------------------------------------------------------------------
// ID_DrawPlayerSpeed
//  [PN/JN] Draws player movement speed in map untits per second format.
//  Based on the implementation by ceski from the Woof source port.
// -----------------------------------------------------------------------------

void ID_DrawPlayerSpeed (void)
{
    static char str[8];
    static char val[16];
    static double speed = 0;
    const player_t *player = &players[displayplayer];

    // Calculate speed only every game tic, not every frame.
    if (oldgametic < gametic)
    {
        const double dx = (double)(player->mo->x - player->mo->oldx) / FRACUNIT;
        const double dy = (double)(player->mo->y - player->mo->oldy) / FRACUNIT;
        const double dz = (double)(player->mo->z - player->mo->oldz) / FRACUNIT;
        speed = sqrt(dx * dx + dy * dy + dz * dz) * TICRATE;
    }

    M_snprintf(str, sizeof(str), "SPD:");
    M_snprintf(val, sizeof(val), " %.0f", speed);

    const int x_val = (ORIGWIDTH / 2);
    const int x_str = x_val - widthfunc(str);

    fontfunc(x_str, 152, str, ID_WidgetColor(widget_speed_str));
    fontfunc(x_val, 152, val, ID_WidgetColor(widget_speed_val));
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
    patch_t *const crosshair_shapes[] = {
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

// [PN] Spectator mouse look in MLOOKUNIT units.
fixed_t CRL_camera_lookdir;
fixed_t CRL_camera_oldlookdir;

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
// [PN/JN] Spectator mouse look.
//  Clamp to the same range as player lookdir.
// -----------------------------------------------------------------------------

#define CLAMPLOOKDIR(x) BETWEEN(-LOOKDIRMIN * MLOOKUNIT, LOOKDIRMAX * MLOOKUNIT, (x))

void CRL_ReportLookdir (fixed_t lookdir)
{
    CRL_camera_oldlookdir = CRL_camera_lookdir;
    CRL_camera_lookdir = CLAMPLOOKDIR(lookdir);
}

void CRL_LimitLookdir (fixed_t delta)
{
    CRL_camera_lookdir = CLAMPLOOKDIR(CRL_camera_lookdir + delta);
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
