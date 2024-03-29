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
//	DOOM graphics stuff for SDL.
//


#include <stdlib.h>
#include <string.h>

#include "SDL.h"
#include "SDL_opengl.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include "config.h"
#include "d_loop.h"
#include "doomtype.h"
#include "i_input.h"
#include "i_joystick.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "tables.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

#include "id_vars.h"


static int vid_startup_delay = 35;
static int vid_resize_delay = 35;

int SCREENWIDTH, SCREENHEIGHT, SCREENHEIGHT_4_3;
int NONWIDEWIDTH; // [crispy] non-widescreen SCREENWIDTH
int WIDESCREENDELTA; // [crispy] horizontal widescreen offset

void (*post_rendering_hook) (void); // [crispy]

// These are (1) the window (or the full screen) that our game is rendered to
// and (2) the renderer that scales the texture (see below) into this window.

static SDL_Window *screen;
static SDL_Renderer *renderer;

// Window title

static const char *window_title = "";

// [JN] Defines window title composition:
// 1 - only game name will appear.
// 0 - game name, port name and version will appear.

static int vid_window_title_short = 1;

// These are (1) the 320x200x8 paletted buffer that we draw to (i.e. the one
// that holds I_VideoBuffer), (2) the 320x200x32 RGBA intermediate buffer that
// we blit the former buffer to, (3) the intermediate 320x200 texture that we
// load the RGBA buffer to and that we render into another texture (4) which
// is upscaled by an integer factor UPSCALE using "nearest" scaling and which
// in turn is finally rendered to screen using "linear" scaling.

#ifndef CRISPY_TRUECOLOR
static SDL_Surface *screenbuffer = NULL;
#endif
static SDL_Surface *argbbuffer = NULL;
static SDL_Texture *texture = NULL;
static SDL_Texture *texture_upscaled = NULL;

#ifndef CRISPY_TRUECOLOR
static SDL_Rect blit_rect = {
    0,
    0,
    MAXWIDTH,
    MAXHEIGHT
};
#endif

static uint32_t pixel_format;

// palette

#ifdef CRISPY_TRUECOLOR
static SDL_Texture *curpane = NULL;
static SDL_Texture *palette_02 = NULL;
static SDL_Texture *palette_03 = NULL;
static SDL_Texture *palette_04 = NULL;
static SDL_Texture *palette_05 = NULL;
static SDL_Texture *palette_06 = NULL;
static SDL_Texture *palette_07 = NULL;
static SDL_Texture *palette_08 = NULL;
static SDL_Texture *palette_09 = NULL;

static SDL_Texture *palette_yel = NULL;

static SDL_Texture *palette_14 = NULL;

static int pane_alpha;

static unsigned int rmask, gmask, bmask, amask; // [crispy] moved up here
static const uint8_t blend_alpha = 184; // [JN] Increased opacity from 0xa8 (168).
static const uint8_t blend_alpha_tinttab = 0x60; // 96
static const uint8_t blend_alpha_alttinttab = 0x8E; // 142
extern pixel_t* pal_color; // [crispy] evil hack to get FPS dots working as in Vanilla
#else
static SDL_Color palette[256];
#endif
static boolean palette_to_set;

// display has been set up?

static boolean initialized = false;

// disable mouse?

static boolean nomouse = false;
int usemouse = 1;

// SDL video driver name

char *vid_video_driver = "";

// [JN] Allow to choose render driver to use.
// https://wiki.libsdl.org/SDL2/SDL_HINT_RENDER_DRIVER

char *vid_screen_scale_api = "";

// [JN] Window X and Y position to save and restore.

int vid_window_position_y = 0;
int vid_window_position_x = 0;

// SDL display number on which to run.

int vid_video_display = 0;

// Screen width and height, from configuration file.

int vid_window_width = 640;
int vid_window_height = 480;

// vid_fullscreen mode, 0x0 for SDL_WINDOW_FULLSCREEN_DESKTOP.

int vid_fullscreen_width = 0, vid_fullscreen_height = 0;

// Maximum number of pixels to use for intermediate scale buffer.

static int vid_max_scaling_buffer_pixels = 16000000;

// Run in full screen mode?  (int type for config code)

int vid_fullscreen = true;

// Aspect ratio correction mode

int vid_aspect_ratio_correct = true;
static int actualheight;

// Force integer scales for resolution-independent rendering

int vid_integer_scaling = false;

// VGA Porch palette change emulation

int vid_vga_porch_flash = false;

// Force software rendering, for systems which lack effective hardware
// acceleration

int vid_force_software_renderer = false;

// Grab the mouse? (int type for config code). nomouse_grab_override allows
// this to be temporarily disabled via the command line.

static int mouse_grab = true;
static boolean nomouse_grab_override = false;

// The screen buffer; this is modified to draw things to the screen

pixel_t *I_VideoBuffer = NULL;

// If true, game is running as a screensaver

boolean screensaver_mode = false;

// Flag indicating whether the screen is currently visible:
// when the screen isnt visible, don't render the screen

boolean screenvisible = true;

// [JN] Externalized FPS value used for FPS counter.

int id_fps_value;

// [JN] Moved to upper level to prevent following while demo warp:
// - prevent force frame rate uncapping after demo warp
// - disk icon drawing
// - palette changing
int demowarp;

// If this is true, the screen is rendered but not blitted to the
// video buffer.

static boolean noblit;

// Callback function to invoke to determine whether to grab the 
// mouse pointer.

static grabmouse_callback_t mouse_grab_callback = NULL;

// Does the window currently have focus?

boolean window_focused = true;

// [JN] Does the sound volume needs to be updated?
// Used for "snd_mute_inactive" feature.

boolean volume_needs_update = false;

// Window resize state.

static boolean need_resize = false;
static unsigned int last_resize_time;

// Joystick/gamepad hysteresis
unsigned int joywait = 0;

// Icon RGB data and dimensions
static const unsigned int *icon_data;
static int icon_w;
static int icon_h;

void *I_GetSDLWindow(void)
{
    return screen;
}

void *I_GetSDLRenderer(void)
{
    return renderer;
}

static boolean MouseShouldBeGrabbed(void)
{
    // never grab the mouse when in screensaver mode
   
    if (screensaver_mode)
        return false;

    // if the window doesn't have focus, never grab it

    if (!window_focused)
        return false;

    // always grab the mouse when full screen (dont want to 
    // see the mouse pointer)

    if (vid_fullscreen)
        return true;

    // Don't grab the mouse if mouse input is disabled

    if (!usemouse || nomouse)
        return false;

    // if we specify not to grab the mouse, never grab

    if (nomouse_grab_override || !mouse_grab)
        return false;

    // Invoke the mouse_grab callback function to determine whether
    // the mouse should be grabbed

    if (mouse_grab_callback != NULL)
    {
        return mouse_grab_callback();
    }
    else
    {
        return true;
    }
}

void I_SetGrabMouseCallback(grabmouse_callback_t func)
{
    mouse_grab_callback = func;
}

static void SetShowCursor(boolean show)
{
    if (!screensaver_mode)
    {
        // When the cursor is hidden, grab the input.
        // Relative mode implicitly hides the cursor.
        SDL_SetRelativeMouseMode(!show);
        SDL_GetRelativeMouseState(NULL, NULL);
    }
}

void I_ShutdownGraphics(void)
{
    if (initialized)
    {
        SetShowCursor(true);

        SDL_QuitSubSystem(SDL_INIT_VIDEO);

        initialized = false;
    }
}

void I_RenderReadPixels (byte **data, int *w, int *h)
{
    SDL_Rect rect;
    SDL_PixelFormat *format;
    int temp;
    uint32_t png_format;
    byte *pixels;

    // [crispy] adjust cropping rectangle if necessary
    rect.x = rect.y = 0;
    SDL_GetRendererOutputSize(renderer, &rect.w, &rect.h);
    if (vid_aspect_ratio_correct)
    {
        if (rect.w * actualheight > rect.h * SCREENWIDTH)
        {
            temp = rect.w;
            rect.w = rect.h * SCREENWIDTH / actualheight;
            rect.x = (temp - rect.w) / 2;
        }
        else
        if (rect.h * SCREENWIDTH > rect.w * actualheight)
        {
            temp = rect.h;
            rect.h = rect.w * actualheight / SCREENWIDTH;
            rect.y = (temp - rect.h) / 2;
        }
    }

    // [crispy] native PNG pixel format
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    png_format = SDL_PIXELFORMAT_ABGR8888;
#else
    png_format = SDL_PIXELFORMAT_RGBA8888;
#endif
    format = SDL_AllocFormat(png_format);
    temp = rect.w * format->BytesPerPixel; // [crispy] pitch

    // [crispy] As far as I understand the issue, SDL_RenderPresent()
    // may return early, i.e. before it has actually finished rendering the
    // current texture to screen -- from where we want to capture it.
    // However, it does never return before it has finished rendering the
    // *previous* texture.
    // Thus, we add a second call to SDL_RenderPresent() here to make sure
    // that it has at least finished rendering the previous texture, which
    // already contains the scene that we actually want to capture.
    if (post_rendering_hook)
    {
        SDL_RenderCopy(renderer, vid_smooth_scaling ? texture_upscaled : texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    // [crispy] allocate memory for screenshot image
    pixels = malloc(rect.h * temp);
    SDL_RenderReadPixels(renderer, &rect, format->format, pixels, temp);

    *data = pixels;
    *w = rect.w;
    *h = rect.h;

    SDL_FreeFormat(format);
}

//
// I_StartFrame
//
void I_StartFrame (void)
{
    // er?

}

// Adjust vid_window_width / vid_window_height variables to be an an aspect
// ratio consistent with the vid_aspect_ratio_correct variable.
static void AdjustWindowSize(void)
{
    if (vid_aspect_ratio_correct || vid_integer_scaling)
    {
        static int old_v_w, old_v_h;

        if (old_v_w > 0 && old_v_h > 0)
        {
          int rendered_height;

          // rendered height does not necessarily match window height
          if (vid_window_height * old_v_w > vid_window_width * old_v_h)
            rendered_height = (vid_window_width * old_v_h + old_v_w - 1) / old_v_w;
          else
            rendered_height = vid_window_height;

          vid_window_width = rendered_height * SCREENWIDTH / actualheight;
        }

        old_v_w = SCREENWIDTH;
        old_v_h = actualheight;
#if 0
        if (vid_window_width * actualheight <= vid_window_height * SCREENWIDTH)
        {
            // We round up vid_window_height if the ratio is not exact; this leaves
            // the result stable.
            vid_window_height = (vid_window_width * actualheight + SCREENWIDTH - 1) / SCREENWIDTH;
        }
        else
        {
            vid_window_width = vid_window_height * SCREENWIDTH / actualheight;
        }
#endif
    }
}

static void HandleWindowEvent(SDL_WindowEvent *event)
{
    int i;

    switch (event->event)
    {
#if 0 // SDL2-TODO
        case SDL_ACTIVEEVENT:
            // need to update our focus state
            UpdateFocus();
            break;
#endif
        case SDL_WINDOWEVENT_EXPOSED:
            palette_to_set = true;
            break;

        case SDL_WINDOWEVENT_RESIZED:
            need_resize = true;
            last_resize_time = SDL_GetTicks();
            break;

        // Don't render the screen when the window is minimized:

        case SDL_WINDOWEVENT_MINIMIZED:
            screenvisible = false;
            break;

        case SDL_WINDOWEVENT_MAXIMIZED:
        case SDL_WINDOWEVENT_RESTORED:
            screenvisible = true;
            break;

        // Update the value of window_focused when we get a focus event
        //
        // We try to make ourselves be well-behaved: the grab on the mouse
        // is removed if we lose focus (such as a popup window appearing),
        // and we dont move the mouse around if we aren't focused either.

        case SDL_WINDOWEVENT_FOCUS_GAINED:
            window_focused = true;
            volume_needs_update = true;
            break;

        case SDL_WINDOWEVENT_FOCUS_LOST:
            window_focused = false;
            volume_needs_update = true;
            break;

        // We want to save the user's preferred monitor to use for running the
        // game, so that next time we're run we start on the same display. So
        // every time the window is moved, find which display we're now on and
        // update the vid_video_display config variable.

        case SDL_WINDOWEVENT_MOVED:
            i = SDL_GetWindowDisplayIndex(screen);
            if (i >= 0)
            {
                vid_video_display = i;
            }
            // [JN] Get X and Y coordinates after moving a window.
            // But do not get in vid_fullscreen mode, since x and y becomes 0,
            // which will cause position reset to "centered" in SetVideoMode.
            if (!vid_fullscreen)
            {
                SDL_GetWindowPosition(screen, &vid_window_position_x, &vid_window_position_y);
            }
            break;

        default:
            break;
    }
}

// -----------------------------------------------------------------------------
// HandleWindowResize
// [JN] Updates window contents (SDL texture) on fly while resizing.
// SDL_WINDOWEVENT_RESIZED from above is still needed to get rid of 
// black borders after window size has been changed.
// -----------------------------------------------------------------------------

static int HandleWindowResize (void* data, SDL_Event *event) 
{
    if (event->type == SDL_WINDOWEVENT 
    &&  event->window.event == SDL_WINDOWEVENT_RESIZED)
    {
        // Redraw window contents.
        I_FinishUpdate();
    }
    return 0;
}

static boolean Togglevid_fullscreenKeyShortcut(SDL_Keysym *sym)
{
    Uint16 flags = (KMOD_LALT | KMOD_RALT);
#if defined(__MACOSX__)
    flags |= (KMOD_LGUI | KMOD_RGUI);
#endif
    return (sym->scancode == SDL_SCANCODE_RETURN || 
            sym->scancode == SDL_SCANCODE_KP_ENTER) && (sym->mod & flags) != 0;
}

static void I_Togglevid_fullscreen(void)
{
    unsigned int flags = 0;

    // TODO: Consider implementing vid_fullscreen toggle for SDL_WINDOW_FULLSCREEN
    // (mode-changing) setup. This is hard because we have to shut down and
    // restart again.
    if (vid_fullscreen_width != 0 || vid_fullscreen_height != 0)
    {
        return;
    }

    vid_fullscreen = !vid_fullscreen;

    if (vid_fullscreen)
    {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    SDL_SetWindowFullscreen(screen, flags);
    // [JN] Hack to fix missing window icon after starting in vid_fullscreen mode.
    I_InitWindowIcon();

    if (!vid_fullscreen)
    {
        AdjustWindowSize();
        SDL_SetWindowSize(screen, vid_window_width, vid_window_height);
    }
}

void I_GetEvent(void)
{
    SDL_Event sdlevent;

    SDL_PumpEvents();

    while (SDL_PollEvent(&sdlevent))
    {
        switch (sdlevent.type)
        {
            case SDL_KEYDOWN:
                if (Togglevid_fullscreenKeyShortcut(&sdlevent.key.keysym))
                {
                    I_Togglevid_fullscreen();
                    break;
                }
                // deliberate fall-though

            case SDL_KEYUP:
		I_HandleKeyboardEvent(&sdlevent);
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEWHEEL:
                if (usemouse && !nomouse && window_focused)
                {
                    I_HandleMouseEvent(&sdlevent);
                }
                break;

            case SDL_QUIT:
                if (screensaver_mode)
                {
                    I_Quit();
                }
                else
                {
                    event_t event;
                    event.type = ev_quit;
                    D_PostEvent(&event);
                }
                break;

            case SDL_WINDOWEVENT:
                if (sdlevent.window.windowID == SDL_GetWindowID(screen))
                {
                    HandleWindowEvent(&sdlevent.window);
                }
                break;

            default:
                break;
        }
    }
}

//
// I_StartTic
//
void I_StartTic (void)
{
    if (!initialized)
    {
        return;
    }

    I_GetEvent();

    if (usemouse && !nomouse && window_focused)
    {
        I_ReadMouse();
    }

    if (joywait < I_GetTime())
    {
        I_UpdateJoystick();
    }
}

static void UpdateGrab(void)
{
    static boolean currently_grabbed = false;
    boolean grab;

    grab = MouseShouldBeGrabbed();

    if (screensaver_mode)
    {
        // Hide the cursor in screensaver mode

        SetShowCursor(false);
    }
    else if (grab && !currently_grabbed)
    {
        SetShowCursor(false);
    }
    else if (!grab && currently_grabbed)
    {
        int screen_w, screen_h;

        SetShowCursor(true);

        // When releasing the mouse from grab, warp the mouse cursor to
        // the bottom-right of the screen. This is a minimally distracting
        // place for it to appear - we may only have released the grab
        // because we're at an end of level intermission screen, for
        // example.

        SDL_GetWindowSize(screen, &screen_w, &screen_h);
        SDL_WarpMouseInWindow(screen, screen_w - 16, screen_h - 16);
        SDL_GetRelativeMouseState(NULL, NULL);
    }

    currently_grabbed = grab;
}

static void LimitTextureSize(int *w_upscale, int *h_upscale)
{
    SDL_RendererInfo rinfo;
    int orig_w, orig_h;

    orig_w = *w_upscale;
    orig_h = *h_upscale;

    // Query renderer and limit to maximum texture dimensions of hardware:
    if (SDL_GetRendererInfo(renderer, &rinfo) != 0)
    {
        I_Error("CreateUpscaledTexture: SDL_GetRendererInfo() call failed: %s",
                SDL_GetError());
    }

    while (*w_upscale * SCREENWIDTH > rinfo.max_texture_width)
    {
        --*w_upscale;
    }
    while (*h_upscale * SCREENHEIGHT > rinfo.max_texture_height)
    {
        --*h_upscale;
    }

    if ((*w_upscale < 1 && rinfo.max_texture_width > 0) ||
        (*h_upscale < 1 && rinfo.max_texture_height > 0))
    {
        I_Error("CreateUpscaledTexture: Can't create a texture big enough for "
                "the whole screen! Maximum texture size %dx%d",
                rinfo.max_texture_width, rinfo.max_texture_height);
    }

    // We limit the amount of texture memory used for the intermediate buffer,
    // since beyond a certain point there are diminishing returns. Also,
    // depending on the hardware there may be performance problems with very
    // huge textures, so the user can use this to reduce the maximum texture
    // size if desired.

    if (vid_max_scaling_buffer_pixels < SCREENWIDTH * SCREENHEIGHT)
    {
        I_Error("CreateUpscaledTexture: vid_max_scaling_buffer_pixels too small "
                "to create a texture buffer: %d < %d",
                vid_max_scaling_buffer_pixels, SCREENWIDTH * SCREENHEIGHT);
    }

    while (*w_upscale * *h_upscale * SCREENWIDTH * SCREENHEIGHT
           > vid_max_scaling_buffer_pixels)
    {
        if (*w_upscale > *h_upscale)
        {
            --*w_upscale;
        }
        else
        {
            --*h_upscale;
        }
    }

    if (*w_upscale != orig_w || *h_upscale != orig_h)
    {
        printf("CreateUpscaledTexture: Limited texture size to %dx%d "
               "(max %d pixels, max texture size %dx%d)\n",
               *w_upscale * SCREENWIDTH, *h_upscale * SCREENHEIGHT,
               vid_max_scaling_buffer_pixels,
               rinfo.max_texture_width, rinfo.max_texture_height);
    }
}

static void CreateUpscaledTexture(boolean force)
{
    int w, h;
    int h_upscale, w_upscale;
    static int h_upscale_old, w_upscale_old;

    SDL_Texture *new_texture, *old_texture;

    // Get the size of the renderer output. The units this gives us will be
    // real world pixels, which are not necessarily equivalent to the screen's
    // window size (because of highdpi).
    if (SDL_GetRendererOutputSize(renderer, &w, &h) != 0)
    {
        I_Error("Failed to get renderer output size: %s", SDL_GetError());
    }

    // When the screen or window dimensions do not match the aspect ratio
    // of the texture, the rendered area is scaled down to fit. Calculate
    // the actual dimensions of the rendered area.

    if (w * actualheight < h * SCREENWIDTH)
    {
        // Tall window.

        h = w * actualheight / SCREENWIDTH;
    }
    else
    {
        // Wide window.

        w = h * SCREENWIDTH / actualheight;
    }

    // Pick texture size the next integer multiple of the screen dimensions.
    // If one screen dimension matches an integer multiple of the original
    // resolution, there is no need to overscale in this direction.

    w_upscale = (w + SCREENWIDTH - 1) / SCREENWIDTH;
    h_upscale = (h + SCREENHEIGHT - 1) / SCREENHEIGHT;

    // Minimum texture dimensions of 320x200.

    if (w_upscale < 1)
    {
        w_upscale = 1;
    }
    if (h_upscale < 1)
    {
        h_upscale = 1;
    }

    LimitTextureSize(&w_upscale, &h_upscale);

    // Create a new texture only if the upscale factors have actually changed.

    if (h_upscale == h_upscale_old && w_upscale == w_upscale_old && !force)
    {
        return;
    }

    h_upscale_old = h_upscale;
    w_upscale_old = w_upscale;

    // Set the scaling quality for rendering the upscaled texture to "linear",
    // which looks much softer and smoother than "nearest" but does a better
    // job at downscaling from the upscaled texture to screen.

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    new_texture = SDL_CreateTexture(renderer,
                                pixel_format,
                                SDL_TEXTUREACCESS_TARGET,
                                w_upscale*SCREENWIDTH,
                                h_upscale*SCREENHEIGHT);

    old_texture = texture_upscaled;
    texture_upscaled = new_texture;

    if (old_texture != NULL)
    {
        SDL_DestroyTexture(old_texture);
    }
}

// [AM] Fractional part of the current tic, in the half-open
//      range of [0.0, 1.0).  Used for interpolation.
fixed_t fractionaltic;

//
// I_FinishUpdate
//
void I_FinishUpdate (void)
{
    // static int lasttic;
    // int tics;
    int i;

    if (!initialized)
        return;

    if (noblit)
        return;

    if (need_resize)
    {
        if (SDL_GetTicks() > last_resize_time + vid_resize_delay)
        {
            int flags;
            // When the window is resized (we're not in vid_fullscreen mode),
            // save the new window size.
            flags = SDL_GetWindowFlags(screen);
            if ((flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == 0)
            {
                SDL_GetWindowSize(screen, &vid_window_width, &vid_window_height);

                // Adjust the window by resizing again so that the window
                // is the right aspect ratio.
                AdjustWindowSize();
                SDL_SetWindowSize(screen, vid_window_width, vid_window_height);
            }
            CreateUpscaledTexture(false);
            need_resize = false;
            palette_to_set = true;
        }
        else
        {
            return;
        }
    }

    UpdateGrab();

#if 0 // SDL2-TODO
    // Don't update the screen if the window isn't visible.
    // Not doing this breaks under Windows when we alt-tab away 
    // while vid_fullscreen.

    if (!(SDL_GetAppState() & SDL_APPACTIVE))
        return;
#endif

	// [crispy] [AM] Real FPS counter
	{
		static int lastmili;
		static int fpscount;
		int mili;

		fpscount++;

		i = SDL_GetTicks();
		mili = i - lastmili;

		// Update FPS counter every second
		if (mili >= 1000)
		{
			id_fps_value = (fpscount * 1000) / mili;
			fpscount = 0;
			lastmili = i;
		}
	}

#ifndef CRISPY_TRUECOLOR
    if (palette_to_set)
    {
        SDL_SetPaletteColors(screenbuffer->format->palette, palette, 0, 256);
        palette_to_set = false;

        if (vid_vga_porch_flash)
        {
            // "flash" the pillars/letterboxes with palette changes, emulating
            // VGA "porch" behaviour (GitHub issue #832)
            SDL_SetRenderDrawColor(renderer, palette[0].r, palette[0].g,
                palette[0].b, SDL_ALPHA_OPAQUE);
        }
    }

    // Blit from the paletted 8-bit screen buffer to the intermediate
    // 32-bit RGBA buffer that we can load into the texture.

    SDL_LowerBlit(screenbuffer, &blit_rect, argbbuffer, &blit_rect);
#endif

    // Update the intermediate texture with the contents of the RGBA buffer.

    SDL_UpdateTexture(texture, NULL, argbbuffer->pixels, argbbuffer->pitch);

    // Make sure the pillarboxes are kept clear each frame.

    SDL_RenderClear(renderer);

    if (vid_smooth_scaling && !vid_force_software_renderer)
    {
    // Render this intermediate texture into the upscaled texture
    // using "nearest" integer scaling.

    SDL_SetRenderTarget(renderer, texture_upscaled);
    SDL_RenderCopy(renderer, texture, NULL, NULL);

    // Finally, render this upscaled texture to screen using linear scaling.

    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture_upscaled, NULL, NULL);
    }
    else
    {
	SDL_SetRenderTarget(renderer, NULL);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
    }

#ifdef CRISPY_TRUECOLOR
    if (curpane)
    {
	SDL_SetTextureAlphaMod(curpane, pane_alpha);
	SDL_RenderCopy(renderer, curpane, NULL, NULL);
    }
#endif

    // Draw!

    SDL_RenderPresent(renderer);

    if (vid_uncapped_fps && !singletics)
    {
        // Limit framerate
        if (vid_fpslimit >= TICRATE)
        {
            uint64_t target_time = 1000000ull / vid_fpslimit;
            static uint64_t start_time;

            while (1)
            {
                uint64_t current_time = I_GetTimeUS();
                uint64_t elapsed_time = current_time - start_time;
                uint64_t remaining_time = 0;

                if (elapsed_time >= target_time)
                {
                    start_time = current_time;
                    break;
                }

                remaining_time = target_time - elapsed_time;

                if (remaining_time > 1000)
                {
                    I_Sleep((remaining_time - 1000) / 1000);
                }
            }
        }

        // [AM] Figure out how far into the current tic we're in as a fixed_t.
        fractionaltic = I_GetFracRealTime();
    }
}


//
// I_ReadScreen
//
void I_ReadScreen (pixel_t* scr)
{
    memcpy(scr, I_VideoBuffer, SCREENWIDTH*SCREENHEIGHT*sizeof(*scr));
}


//
// I_SetPalette
//
void I_SetPalette (int palette)
{
    // [JN] Don't change palette while demo warp.
    if (demowarp)
    {
        return;
    }
    
    switch (palette)
    {
		case 0:		curpane = NULL;			break;
		// Red palette
		case 1:		curpane = palette_02;	break;
		case 2:		curpane = palette_03;	break;
		case 3:		curpane = palette_04;	break;
		case 4:		curpane = palette_05;	break;
		case 5:		curpane = palette_06;	break;
		case 6:		curpane = palette_07;	break;
		case 7:		curpane = palette_08;	break;
		case 8:		curpane = palette_09;	break;
		// Yellow palette
		case 9:		curpane = palette_yel;	pane_alpha = 24;	break;
		case 10:	curpane = palette_yel;	pane_alpha = 48;	break;
		case 11:	curpane = palette_yel;	pane_alpha = 72;	break;
		case 12:	curpane = palette_yel;	pane_alpha = 96;	break;
		// Green palette
		case 13:	curpane = palette_14;	break;

		default:
			I_Error("Unknown palette: %d!\n", palette);
			break;
    }
}

// 
// Set the window title
//

void I_SetWindowTitle(const char *title)
{
    window_title = title;
}

//
// Call the SDL function to set the window title, based on 
// the title set with I_SetWindowTitle.
//

void I_InitWindowTitle(void)
{
    char *buf;

    // [JN] Leave only game name in window title.
    buf = M_StringJoin(window_title, vid_window_title_short ?
                       NULL : " - ", PACKAGE_FULLNAME, NULL);
    SDL_SetWindowTitle(screen, buf);
    free(buf);
}

void I_RegisterWindowIcon(const unsigned int *icon, int width, int height)
{
    icon_data = icon;
    icon_w = width;
    icon_h = height;
}

// Set the application icon

void I_InitWindowIcon(void)
{
    SDL_Surface *surface;

    surface = SDL_CreateRGBSurfaceFrom((void *) icon_data, icon_w, icon_h,
                                       32, icon_w * 4,
                                       0xffu << 24, 0xffu << 16,
                                       0xffu << 8, 0xffu << 0);

    SDL_SetWindowIcon(screen, surface);
    SDL_FreeSurface(surface);
}

// Set video size to a particular scale factor (1x, 2x, 3x, etc.)

static void SetScaleFactor(int factor)
{
    // Pick 320x200 or 320x240, depending on aspect ratio correct

    vid_window_width = factor * SCREENWIDTH;
    vid_window_height = factor * actualheight;
    vid_fullscreen = false;
}

void I_GraphicsCheckCommandLine(void)
{
    int i;

    //!
    // @category video
    // @vanilla
    //
    // Disable blitting the screen.
    //

    noblit = M_CheckParm ("-noblit");

    //!
    // @category video 
    //
    // Don't grab the mouse when running in windowed mode.
    //

    nomouse_grab_override = M_ParmExists("-nomouse_grab");

    // default to vid_fullscreen mode, allow override with command line
    // novid_fullscreen because we love prboom

    //!
    // @category video 
    //
    // Run in a window.
    //

    if (M_CheckParm("-window") || M_CheckParm("-novid_fullscreen"))
    {
        vid_fullscreen = false;
    }

    //!
    // @category video 
    //
    // Run in vid_fullscreen mode.
    //

    if (M_CheckParm("-vid_fullscreen"))
    {
        vid_fullscreen = true;
    }

    //!
    // @category video 
    //
    // Disable the mouse.
    //

    nomouse = M_CheckParm("-nomouse") > 0;

    //!
    // @category video
    // @arg <x>
    //
    // Specify the screen width, in pixels. Implies -window.
    //

    i = M_CheckParmWithArgs("-width", 1);

    if (i > 0)
    {
        vid_window_width = atoi(myargv[i + 1]);
        vid_window_height = vid_window_width * 2;
        AdjustWindowSize();
        vid_fullscreen = false;
    }

    //!
    // @category video
    // @arg <y>
    //
    // Specify the screen height, in pixels. Implies -window.
    //

    i = M_CheckParmWithArgs("-height", 1);

    if (i > 0)
    {
        vid_window_height = atoi(myargv[i + 1]);
        vid_window_width = vid_window_height * 2;
        AdjustWindowSize();
        vid_fullscreen = false;
    }

    //!
    // @category video
    // @arg <WxY>
    //
    // Specify the dimensions of the window. Implies -window.
    //

    i = M_CheckParmWithArgs("-geometry", 1);

    if (i > 0)
    {
        int w, h, s;

        s = sscanf(myargv[i + 1], "%ix%i", &w, &h);
        if (s == 2)
        {
            vid_window_width = w;
            vid_window_height = h;
            vid_fullscreen = false;
        }
    }

    //!
    // @category video
    //
    // Don't scale up the screen. Implies -window.
    //

    if (M_CheckParm("-1")) 
    {
        SetScaleFactor(1);
    }

    //!
    // @category video
    //
    // Double up the screen to 2x its normal size. Implies -window.
    //

    if (M_CheckParm("-2")) 
    {
        SetScaleFactor(2);
    }

    //!
    // @category video
    //
    // Double up the screen to 3x its normal size. Implies -window.
    //

    if (M_CheckParm("-3")) 
    {
        SetScaleFactor(3);
    }
}

// Check if we have been invoked as a screensaver by xscreensaver.

void I_CheckIsScreensaver(void)
{
    char *env;

    env = getenv("XSCREENSAVER_WINDOW");

    if (env != NULL)
    {
        screensaver_mode = true;
    }
}

static void SetSDLVideoDriver(void)
{
    // Allow a default value for the SDL video driver to be specified
    // in the configuration file.

    if (strcmp(vid_video_driver, "") != 0)
    {
        char *env_string;

        env_string = M_StringJoin("SDL_VIDEODRIVER=", vid_video_driver, NULL);
        putenv(env_string);
        free(env_string);
    }
}

// Check the display bounds of the display referred to by 'vid_video_display' and
// set x and y to a location that places the window in the center of that
// display.
void CenterWindow(int *x, int *y, int w, int h)
{
    SDL_Rect bounds;

    if (SDL_GetDisplayBounds(vid_video_display, &bounds) < 0)
    {
        fprintf(stderr, "CenterWindow: Failed to read display bounds "
                        "for display #%d!\n", vid_video_display);
        return;
    }

    *x = bounds.x + SDL_max((bounds.w - w) / 2, 0);
    *y = bounds.y + SDL_max((bounds.h - h) / 2, 0);
}

static void SetVideoMode(void)
{
    int w, h;
    int x = 0, y = 0;
#ifndef CRISPY_TRUECOLOR
    unsigned int rmask, gmask, bmask, amask;
#endif
    int bpp;
    int window_flags = 0, renderer_flags = 0;
    SDL_DisplayMode mode;

    w = vid_window_width;
    h = vid_window_height;

    // In windowed mode, the window can be resized while the game is
    // running.
    window_flags = SDL_WINDOW_RESIZABLE;

    // Set the highdpi flag - this makes a big difference on Macs with
    // retina displays, especially when using small window sizes.
    window_flags |= SDL_WINDOW_ALLOW_HIGHDPI;

    // [JN] Choose render driver to use.
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, vid_screen_scale_api);

    if (vid_fullscreen)
    {
        if (vid_fullscreen_width == 0 && vid_fullscreen_height == 0)
        {
            // This window_flags means "Never change the screen resolution!
            // Instead, draw to the entire screen by scaling the texture
            // appropriately".
            window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
        else
        {
            w = vid_fullscreen_width;
            h = vid_fullscreen_height;
            window_flags |= SDL_WINDOW_FULLSCREEN;
        }
    }

    // in vid_fullscreen mode, the window "position" still matters, because
    // we use it to control which display we run vid_fullscreen on.
    if (vid_fullscreen)
    {
        CenterWindow(&x, &y, w, h);
    }

    // [JN] If window X and Y coords was not set,
    // place game window in the center of the screen.
    if (vid_window_position_x == 0 || vid_window_position_y == 0)
    {
        vid_window_position_x = x/2 + w/2;
        vid_window_position_y = y/2 + h/2;
    }

    // Create window and renderer contexts. We set the window title
    // later anyway and leave the window position "undefined". If
    // "window_flags" contains the vid_fullscreen flag (see above), then
    // w and h are ignored.

    if (screen == NULL)
    {
        screen = SDL_CreateWindow(NULL, vid_window_position_x, vid_window_position_y,
                                  w, h, window_flags);

        if (screen == NULL)
        {
            I_Error("Error creating window for video startup: %s",
            SDL_GetError());
        }

        pixel_format = SDL_GetWindowPixelFormat(screen);

        SDL_SetWindowMinimumSize(screen, SCREENWIDTH, actualheight);

        I_InitWindowTitle();
        I_InitWindowIcon();
    }

    // The SDL_RENDERER_TARGETTEXTURE flag is required to render the
    // intermediate texture into the upscaled texture.
    renderer_flags = SDL_RENDERER_TARGETTEXTURE;
	
    if (SDL_GetCurrentDisplayMode(vid_video_display, &mode) != 0)
    {
        I_Error("Could not get display mode for video display #%d: %s",
        vid_video_display, SDL_GetError());
    }

    // Turn on vsync if we aren't in a -timedemo
    if ((!singletics && mode.refresh_rate > 0) || demowarp)
    {
        if (vid_vsync) // [crispy] uncapped vsync
        {
            renderer_flags |= SDL_RENDERER_PRESENTVSYNC;
        }
    }

	// Force software mode
    if (vid_force_software_renderer)
    {
        renderer_flags |= SDL_RENDERER_SOFTWARE;
        renderer_flags &= ~SDL_RENDERER_PRESENTVSYNC;
        vid_vsync = false;
    }

    if (renderer != NULL)
    {
        SDL_DestroyRenderer(renderer);
        // all associated textures get destroyed
        texture = NULL;
        texture_upscaled = NULL;
    }

    renderer = SDL_CreateRenderer(screen, -1, renderer_flags);

    // If we could not find a matching render driver,
    // try again without hardware acceleration.

    if (renderer == NULL && !vid_force_software_renderer)
    {
        renderer_flags |= SDL_RENDERER_SOFTWARE;
        renderer_flags &= ~SDL_RENDERER_PRESENTVSYNC;

        renderer = SDL_CreateRenderer(screen, -1, renderer_flags);

        // If this helped, save the setting for later.
        if (renderer != NULL)
        {
            vid_force_software_renderer = 1;
        }
    }

    if (renderer == NULL)
    {
        I_Error("Error creating renderer for screen window: %s",
                SDL_GetError());
    }

    // Important: Set the "logical size" of the rendering context. At the same
    // time this also defines the aspect ratio that is preserved while scaling
    // and stretching the texture into the window.

    if (vid_aspect_ratio_correct || vid_integer_scaling)
    {
        SDL_RenderSetLogicalSize(renderer,
                                 SCREENWIDTH,
                                 actualheight);
    }

    // Force integer scales for resolution-independent rendering.

    SDL_RenderSetIntegerScale(renderer, vid_integer_scaling);

    // Blank out the full screen area in case there is any junk in
    // the borders that won't otherwise be overwritten.

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

#ifndef CRISPY_TRUECOLOR
    // Create the 8-bit paletted and the 32-bit RGBA screenbuffer surfaces.

    if (screenbuffer != NULL)
    {
        SDL_FreeSurface(screenbuffer);
        screenbuffer = NULL;
    }

    if (screenbuffer == NULL)
    {
        screenbuffer = SDL_CreateRGBSurface(0,
                                            SCREENWIDTH, SCREENHEIGHT, 8,
                                            0, 0, 0, 0);
        SDL_FillRect(screenbuffer, NULL, 0);
    }
#endif

    // Format of argbbuffer must match the screen pixel format because we
    // import the surface data into the texture.

    if (argbbuffer != NULL)
    {
        SDL_FreeSurface(argbbuffer);
        argbbuffer = NULL;
    }

    if (argbbuffer == NULL)
    {
        SDL_PixelFormatEnumToMasks(pixel_format, &bpp,
                                   &rmask, &gmask, &bmask, &amask);
        argbbuffer = SDL_CreateRGBSurface(0,
                                          SCREENWIDTH, SCREENHEIGHT, bpp,
                                          rmask, gmask, bmask, amask);
#ifdef CRISPY_TRUECOLOR

		// Red palette
        SDL_FillRect(argbbuffer, NULL, I_MapRGB(255, 224, 224));
        palette_02 = SDL_CreateTextureFromSurface(renderer, argbbuffer);
        SDL_SetTextureBlendMode(palette_02, SDL_BLENDMODE_MOD); // SDL_BLENDMODE_MUL

        SDL_FillRect(argbbuffer, NULL, I_MapRGB(255, 192, 192));
        palette_03 = SDL_CreateTextureFromSurface(renderer, argbbuffer);
        SDL_SetTextureBlendMode(palette_03, SDL_BLENDMODE_MOD);

        SDL_FillRect(argbbuffer, NULL, I_MapRGB(255, 160, 160));
        palette_04 = SDL_CreateTextureFromSurface(renderer, argbbuffer);
        SDL_SetTextureBlendMode(palette_04, SDL_BLENDMODE_MOD);

        SDL_FillRect(argbbuffer, NULL, I_MapRGB(255, 128, 128));
        palette_05 = SDL_CreateTextureFromSurface(renderer, argbbuffer);
        SDL_SetTextureBlendMode(palette_05, SDL_BLENDMODE_MOD);

        SDL_FillRect(argbbuffer, NULL, I_MapRGB(255, 96, 96));
        palette_06 = SDL_CreateTextureFromSurface(renderer, argbbuffer);
        SDL_SetTextureBlendMode(palette_06, SDL_BLENDMODE_MOD);

        SDL_FillRect(argbbuffer, NULL, I_MapRGB(255, 64, 64));
        palette_07 = SDL_CreateTextureFromSurface(renderer, argbbuffer);
        SDL_SetTextureBlendMode(palette_07, SDL_BLENDMODE_MOD);

        SDL_FillRect(argbbuffer, NULL, I_MapRGB(255, 32, 32));
        palette_08 = SDL_CreateTextureFromSurface(renderer, argbbuffer);
        SDL_SetTextureBlendMode(palette_08, SDL_BLENDMODE_MOD);

        SDL_FillRect(argbbuffer, NULL, I_MapRGB(255, 0, 0));
        palette_09 = SDL_CreateTextureFromSurface(renderer, argbbuffer);
        SDL_SetTextureBlendMode(palette_09, SDL_BLENDMODE_MOD);

		// Yellow palette pane_alpha
        SDL_FillRect(argbbuffer, NULL, I_MapRGB(255, 164, 0));
        palette_yel = SDL_CreateTextureFromSurface(renderer, argbbuffer);
        SDL_SetTextureBlendMode(palette_yel, SDL_BLENDMODE_ADD);

        SDL_FillRect(argbbuffer, NULL, I_MapRGB(255, 164, 0));
        palette_yel = SDL_CreateTextureFromSurface(renderer, argbbuffer);
        SDL_SetTextureBlendMode(palette_yel, SDL_BLENDMODE_ADD);

        SDL_FillRect(argbbuffer, NULL, I_MapRGB(255, 164, 0));
        palette_yel = SDL_CreateTextureFromSurface(renderer, argbbuffer);
        SDL_SetTextureBlendMode(palette_yel, SDL_BLENDMODE_ADD);

        SDL_FillRect(argbbuffer, NULL, I_MapRGB(255, 164, 0));
        palette_yel = SDL_CreateTextureFromSurface(renderer, argbbuffer);
        SDL_SetTextureBlendMode(palette_yel, SDL_BLENDMODE_ADD);

		// Green palette
        SDL_FillRect(argbbuffer, NULL, I_MapRGB(96, 255, 64));
        palette_14 = SDL_CreateTextureFromSurface(renderer, argbbuffer);
        SDL_SetTextureBlendMode(palette_14, SDL_BLENDMODE_MOD);
#endif
        SDL_FillRect(argbbuffer, NULL, 0);
    }

    if (texture != NULL)
    {
        SDL_DestroyTexture(texture);
    }

    // Set the scaling quality for rendering the intermediate texture into
    // the upscaled texture to "nearest", which is gritty and pixelated and
    // resembles software scaling pretty well.

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

    // Create the intermediate texture that the RGBA surface gets loaded into.
    // The SDL_TEXTUREACCESS_STREAMING flag means that this texture's content
    // is going to change frequently.

    texture = SDL_CreateTexture(renderer,
                                pixel_format,
                                SDL_TEXTUREACCESS_STREAMING,
                                SCREENWIDTH, SCREENHEIGHT);

    // [JN] Workaround for SDL 2.0.14+ alt-tab bug
#if defined(_WIN32)
    SDL_SetHintWithPriority(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "1", SDL_HINT_OVERRIDE);
#endif

    // Initially create the upscaled texture for rendering to screen

    CreateUpscaledTexture(true);
}

// [crispy] re-calculate SCREENWIDTH, SCREENHEIGHT, NONWIDEWIDTH and WIDESCREENDELTA
void I_GetScreenDimensions (void)
{
	SDL_DisplayMode mode;
	int w = 16, h = 9;
	int ah;

	SCREENWIDTH = ORIGWIDTH * vid_resolution;
	SCREENHEIGHT = ORIGHEIGHT * vid_resolution;

	NONWIDEWIDTH = SCREENWIDTH;

	ah = (vid_aspect_ratio_correct == 1) ? (6 * SCREENHEIGHT / 5) : SCREENHEIGHT;

	if (SDL_GetCurrentDisplayMode(vid_video_display, &mode) == 0)
	{
		// [crispy] sanity check: really widescreen display?
		if (mode.w * ah >= mode.h * SCREENWIDTH)
		{
			w = mode.w;
			h = mode.h;
		}
	}

	// [crispy] widescreen rendering makes no sense without aspect ratio correction
	if (vid_widescreen && vid_aspect_ratio_correct == 1)
	{
		switch(vid_widescreen)
		{
			case RATIO_16_10:
				w = 16;
				h = 10;
				break;
			case RATIO_16_9:
				w = 16;
				h = 9;
				break;
			case RATIO_21_9:
				w = 21;
				h = 9;
				break;
			default:
				break;
		}

		SCREENWIDTH = w * ah / h;
		// [crispy] make sure SCREENWIDTH is an integer multiple of 4 ...
		if (vid_resolution > 1)
		{
			// [Nugget] Since we have uneven resolution multipliers, mask it twice
			SCREENWIDTH = (((SCREENWIDTH * vid_resolution) & (int)~3) / vid_resolution + 3) & (int)~3;
		}
		else
		{
			SCREENWIDTH = (SCREENWIDTH + 3) & (int)~3;
		}

		// [crispy] ... but never exceeds MAXWIDTH (array size!)
		SCREENWIDTH = MIN(SCREENWIDTH, MAXWIDTH / 3);
	}

	WIDESCREENDELTA = ((SCREENWIDTH - NONWIDEWIDTH) / vid_resolution) / 2;
}

void I_InitGraphics(void)
{
    SDL_Event dummy;
#ifndef CRISPY_TRUECOLOR
    byte *doompal;
#endif
    char *env;

    // Pass through the XSCREENSAVER_WINDOW environment variable to 
    // SDL_WINDOWID, to embed the SDL window into the Xscreensaver
    // window.

    env = getenv("XSCREENSAVER_WINDOW");

    if (env != NULL)
    {
        char winenv[30];
        unsigned int winid;

        sscanf(env, "0x%x", &winid);
        M_snprintf(winenv, sizeof(winenv), "SDL_WINDOWID=%u", winid);

        putenv(winenv);
    }

    SetSDLVideoDriver();

    // [JN] Set an event watcher for window resize to allow
    // update window contents on fly.
    SDL_AddEventWatch(HandleWindowResize, screen);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
        I_Error("Failed to initialize video: %s", SDL_GetError());
    }

    // When in screensaver mode, run full screen and auto detect
    // screen dimensions (don't change video mode)
    if (screensaver_mode)
    {
        vid_fullscreen = true;
    }

    // [crispy] run-time variable high-resolution rendering
    I_GetScreenDimensions();

#ifndef CRISPY_TRUECOLOR
    blit_rect.w = SCREENWIDTH;
    blit_rect.h = SCREENHEIGHT;
#endif

    // [crispy] (re-)initialize resolution-agnostic patch drawing
    V_Init();

    if (vid_aspect_ratio_correct == 1)
    {
        actualheight = 6 * SCREENHEIGHT / 5;
    }
    else
    {
        actualheight = SCREENHEIGHT;
    }

    // Create the game window; this may switch graphic modes depending
    // on configuration.
    AdjustWindowSize();
    SetVideoMode();

#ifndef CRISPY_TRUECOLOR
    // Start with a clear black screen
    // (screen will be flipped after we set the palette)

    SDL_FillRect(screenbuffer, NULL, 0);

    // Set the palette

    doompal = W_CacheLumpName(DEH_String("PLAYPAL"), PU_CACHE);
    I_SetPalette(doompal);
    SDL_SetPaletteColors(screenbuffer->format->palette, palette, 0, 256);
#endif

    // SDL2-TODO UpdateFocus();
    UpdateGrab();

    // On some systems, it takes a second or so for the screen to settle
    // after changing modes.  We include the option to add a delay when
    // setting the screen mode, so that the game doesn't start immediately
    // with the player unable to see anything.

    if (vid_fullscreen && !screensaver_mode)
    {
        SDL_Delay(vid_startup_delay);
    }

    // The actual 320x200 canvas that we draw to. This is the pixel buffer of
    // the 8-bit paletted screen buffer that gets blit on an intermediate
    // 32-bit RGBA screen buffer that gets loaded into a texture that gets
    // finally rendered into our window or full screen in I_FinishUpdate().

#ifndef CRISPY_TRUECOLOR
    I_VideoBuffer = screenbuffer->pixels;
#else
    I_VideoBuffer = argbbuffer->pixels;
#endif
    V_RestoreBuffer();

    // Clear the screen to black.

    memset(I_VideoBuffer, 0, SCREENWIDTH * SCREENHEIGHT * sizeof(*I_VideoBuffer));

    // clear out any events waiting at the start and center the mouse
  
    while (SDL_PollEvent(&dummy));

    initialized = true;
}

// [crispy] re-initialize only the parts of the rendering stack that are really necessary

void I_ReInitGraphics (int reinit)
{
	// [crispy] re-set rendering resolution and re-create framebuffers
	if (reinit & REINIT_FRAMEBUFFERS)
	{
		unsigned int rmask, gmask, bmask, amask;
		int unused_bpp;

		I_GetScreenDimensions();

#ifndef CRISPY_TRUECOLOR
		blit_rect.w = SCREENWIDTH;
		blit_rect.h = SCREENHEIGHT;
#endif

		// [crispy] re-initialize resolution-agnostic patch drawing
		V_Init();

#ifndef CRISPY_TRUECOLOR
		SDL_FreeSurface(screenbuffer);
		screenbuffer = SDL_CreateRGBSurface(0,
				                    SCREENWIDTH, SCREENHEIGHT, 8,
				                    0, 0, 0, 0);
#endif

		SDL_FreeSurface(argbbuffer);
		SDL_PixelFormatEnumToMasks(pixel_format, &unused_bpp,
		                           &rmask, &gmask, &bmask, &amask);
		argbbuffer = SDL_CreateRGBSurface(0,
		                                  SCREENWIDTH, SCREENHEIGHT, 32,
		                                  rmask, gmask, bmask, amask);
#ifndef CRISPY_TRUECOLOR
		// [crispy] re-set the framebuffer pointer
		I_VideoBuffer = screenbuffer->pixels;
#else
		I_VideoBuffer = argbbuffer->pixels;
#endif
		V_RestoreBuffer();

		// [crispy] it will get re-created below with the new resolution
		SDL_DestroyTexture(texture);
	}

	// [crispy] re-create renderer
	if (reinit & REINIT_RENDERER)
	{
		SDL_RendererInfo info = {0};
		int flags;

		SDL_GetRendererInfo(renderer, &info);
		flags = info.flags;

		if (vid_vsync && !(flags & SDL_RENDERER_SOFTWARE))
		{
			flags |= SDL_RENDERER_PRESENTVSYNC;
		}
		else
		{
			flags &= ~SDL_RENDERER_PRESENTVSYNC;
		}

		SDL_DestroyRenderer(renderer);
		renderer = SDL_CreateRenderer(screen, -1, flags);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

		// [crispy] the texture gets destroyed in SDL_DestroyRenderer(), force its re-creation
		texture_upscaled = NULL;
	}

	// [crispy] re-create textures
	if (reinit & REINIT_TEXTURES)
	{
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

		texture = SDL_CreateTexture(renderer,
		                            pixel_format,
		                            SDL_TEXTUREACCESS_STREAMING,
		                            SCREENWIDTH, SCREENHEIGHT);

		// [crispy] force its re-creation
		CreateUpscaledTexture(true);
	}

	// [crispy] re-set logical rendering resolution
	if (reinit & REINIT_ASPECTRATIO)
	{
		if (vid_aspect_ratio_correct == 1)
		{
			actualheight = 6 * SCREENHEIGHT / 5;
		}
		else
		{
			actualheight = SCREENHEIGHT;
		}

		if (vid_aspect_ratio_correct || vid_integer_scaling)
		{
			SDL_RenderSetLogicalSize(renderer,
			                         SCREENWIDTH,
			                         actualheight);
		}
		else
		{
			SDL_RenderSetLogicalSize(renderer, 0, 0);
		}

		#if SDL_VERSION_ATLEAST(2, 0, 5)
		SDL_RenderSetIntegerScale(renderer, vid_integer_scaling);
		#endif
	}

	// [crispy] adjust the window size and re-set the palette
	need_resize = true;
}

// -----------------------------------------------------------------------------
// I_ToggleVsync
// [JN] Uses native SDL VSync toggling function.
// -----------------------------------------------------------------------------

void I_ToggleVsync (void)
{
    SDL_RenderSetVSync(renderer, vid_vsync);
}

// Bind all variables controlling video options into the configuration
// file system.
void I_BindVideoVariables(void)
{
    M_BindIntVariable("vid_startup_delay",             &vid_startup_delay);
    M_BindIntVariable("vid_resize_delay",              &vid_resize_delay);
    M_BindIntVariable("vid_fullscreen",                &vid_fullscreen);
    M_BindIntVariable("vid_video_display",             &vid_video_display);
    M_BindIntVariable("vid_aspect_ratio_correct",      &vid_aspect_ratio_correct);
    M_BindIntVariable("vid_integer_scaling",           &vid_integer_scaling);
    M_BindIntVariable("vid_vga_porch_flash",           &vid_vga_porch_flash);
    M_BindIntVariable("vid_fullscreen_width",          &vid_fullscreen_width);
    M_BindIntVariable("vid_fullscreen_height",         &vid_fullscreen_height);
    M_BindIntVariable("vid_window_title_short",        &vid_window_title_short);
    M_BindIntVariable("vid_force_software_renderer",   &vid_force_software_renderer);
    M_BindIntVariable("vid_max_scaling_buffer_pixels", &vid_max_scaling_buffer_pixels);
    M_BindIntVariable("vid_window_width",              &vid_window_width);
    M_BindIntVariable("vid_window_height",             &vid_window_height);
    M_BindStringVariable("vid_video_driver",           &vid_video_driver);
    M_BindStringVariable("vid_screen_scale_api",       &vid_screen_scale_api);
    M_BindIntVariable("vid_window_position_x",         &vid_window_position_x);
    M_BindIntVariable("vid_window_position_y",         &vid_window_position_y);
    M_BindIntVariable("mouse_enable",                  &usemouse);
    M_BindIntVariable("mouse_grab",                    &mouse_grab);
}
#ifdef CRISPY_TRUECOLOR
const pixel_t I_BlendAdd (const pixel_t bg, const pixel_t fg)
{
	uint32_t r, g, b;

	if ((r = (fg & rmask) + (bg & rmask)) > rmask) r = rmask;
	if ((g = (fg & gmask) + (bg & gmask)) > gmask) g = gmask;
	if ((b = (fg & bmask) + (bg & bmask)) > bmask) b = bmask;

	return amask | r | g | b;
}

// [crispy] http://stereopsis.com/doubleblend.html
const pixel_t I_BlendDark (const pixel_t bg, const int d)
{
	const uint32_t ag = (bg & 0xff00ff00) >> 8;
	const uint32_t rb =  bg & 0x00ff00ff;

	uint32_t sag = d * ag;
	uint32_t srb = d * rb;

	sag = sag & 0xff00ff00;
	srb = (srb >> 8) & 0x00ff00ff;

	return amask | sag | srb;
}

const pixel_t I_BlendOver (const pixel_t bg, const pixel_t fg)
{
	const uint32_t r = ((blend_alpha * (fg & rmask) + (0xff - blend_alpha) * (bg & rmask)) >> 8) & rmask;
	const uint32_t g = ((blend_alpha * (fg & gmask) + (0xff - blend_alpha) * (bg & gmask)) >> 8) & gmask;
	const uint32_t b = ((blend_alpha * (fg & bmask) + (0xff - blend_alpha) * (bg & bmask)) >> 8) & bmask;

	return amask | r | g | b;
}

// [crispy] TINTTAB blending emulation, used for Heretic and Hexen
const pixel_t I_BlendOverTinttab (const pixel_t bg, const pixel_t fg)
{
	const uint32_t r = ((blend_alpha_tinttab * (fg & rmask) + (0xff - blend_alpha_tinttab) * (bg & rmask)) >> 8) & rmask;
	const uint32_t g = ((blend_alpha_tinttab * (fg & gmask) + (0xff - blend_alpha_tinttab) * (bg & gmask)) >> 8) & gmask;
	const uint32_t b = ((blend_alpha_tinttab * (fg & bmask) + (0xff - blend_alpha_tinttab) * (bg & bmask)) >> 8) & bmask;

	return amask | r | g | b;
}

// [crispy] More opaque ("Alt") TINTTAB blending emulation, used for Hexen's MF_ALTSHADOW drawing
const pixel_t I_BlendOverAltTinttab (const pixel_t bg, const pixel_t fg)
{
	const uint32_t r = ((blend_alpha_alttinttab * (fg & rmask) + (0xff - blend_alpha_alttinttab) * (bg & rmask)) >> 8) & rmask;
	const uint32_t g = ((blend_alpha_alttinttab * (fg & gmask) + (0xff - blend_alpha_alttinttab) * (bg & gmask)) >> 8) & gmask;
	const uint32_t b = ((blend_alpha_alttinttab * (fg & bmask) + (0xff - blend_alpha_alttinttab) * (bg & bmask)) >> 8) & bmask;

	return amask | r | g | b;
}

const pixel_t (*blendfunc) (const pixel_t fg, const pixel_t bg) = I_BlendOver;

const pixel_t I_MapRGB (const uint8_t r, const uint8_t g, const uint8_t b)
{
/*
	return amask |
	        (((r * rmask) >> 8) & rmask) |
	        (((g * gmask) >> 8) & gmask) |
	        (((b * bmask) >> 8) & bmask);
*/
	return SDL_MapRGB(argbbuffer->format, r, g, b);
}

const pixel_t I_BlendFuzz (const pixel_t bg, const pixel_t fg)
{
	const uint32_t r = ((96 * (fg & rmask) + (0xff - 96) * (bg & rmask)) >> 8) & rmask;
	const uint32_t g = ((96 * (fg & gmask) + (0xff - 96) * (bg & gmask)) >> 8) & gmask;
	const uint32_t b = ((96 * (fg & bmask) + (0xff - 96) * (bg & bmask)) >> 8) & bmask;

	return amask | r | g | b;
}

// [JN] Extra translucency blending (60% opacity).
const pixel_t I_BlendOverExtra (const pixel_t bg, const pixel_t fg)
{
	const uint32_t r = ((152 * (fg & rmask) + (0xff - 152) * (bg & rmask)) >> 8) & rmask;
	const uint32_t g = ((152 * (fg & gmask) + (0xff - 152) * (bg & gmask)) >> 8) & gmask;
	const uint32_t b = ((152 * (fg & bmask) + (0xff - 152) * (bg & bmask)) >> 8) & bmask;

	return amask | r | g | b;
}

// [JN] Shade factor used for menu and automap background shading.
const int I_ShadeFactor[] =
{
    240, 224, 208, 192, 176, 160, 144, 112, 96, 80, 64, 48, 32
};

// [JN] Saturation percent array.
// 0.66 = 0% saturation, 0.0 = 100% saturation.
const float I_SaturationPercent[100] =
{
    0.660000f, 0.653400f, 0.646800f, 0.640200f, 0.633600f,
    0.627000f, 0.620400f, 0.613800f, 0.607200f, 0.600600f,
    0.594000f, 0.587400f, 0.580800f, 0.574200f, 0.567600f,
    0.561000f, 0.554400f, 0.547800f, 0.541200f, 0.534600f,
    0.528000f, 0.521400f, 0.514800f, 0.508200f, 0.501600f,
    0.495000f, 0.488400f, 0.481800f, 0.475200f, 0.468600f,
    0.462000f, 0.455400f, 0.448800f, 0.442200f, 0.435600f,
    0.429000f, 0.422400f, 0.415800f, 0.409200f, 0.402600f,
    0.396000f, 0.389400f, 0.382800f, 0.376200f, 0.369600f,
    0.363000f, 0.356400f, 0.349800f, 0.343200f, 0.336600f,
    0.330000f, 0.323400f, 0.316800f, 0.310200f, 0.303600f,
    0.297000f, 0.290400f, 0.283800f, 0.277200f, 0.270600f,
    0.264000f, 0.257400f, 0.250800f, 0.244200f, 0.237600f,
    0.231000f, 0.224400f, 0.217800f, 0.211200f, 0.204600f,
    0.198000f, 0.191400f, 0.184800f, 0.178200f, 0.171600f,
    0.165000f, 0.158400f, 0.151800f, 0.145200f, 0.138600f,
    0.132000f, 0.125400f, 0.118800f, 0.112200f, 0.105600f,
    0.099000f, 0.092400f, 0.085800f, 0.079200f, 0.072600f,
    0.066000f, 0.059400f, 0.052800f, 0.046200f, 0.039600f,
    0.033000f, 0.026400f, 0.019800f, 0.013200f, 0
};

#endif
