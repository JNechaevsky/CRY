//
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


#include <stdlib.h>

#include "SDL_mixer.h"
#include "textscreen.h"
#include "m_config.h"
#include "m_misc.h"
#include "mode.h"
#include "sound.h"

#define WINDOW_HELP_URL "http://jnechaevsky.users.sourceforge.net/projects/rusdoom/setup/sound.html"

typedef enum
{
    OPLMODE_OPL2,
    OPLMODE_OPL3,
    NUM_OPLMODES,
} oplmode_t;


// Config file variables:

int snd_sfxdevice = SNDDEVICE_SB;
int snd_musicdevice = SNDDEVICE_SB;
int snd_samplerate = 44100;
int opl_io_port = 0x388;
int snd_cachesize = 64 * 1024 * 1024;
int snd_maxslicetime_ms = 28;
char *snd_musiccmd = "";
int snd_pitchshift = 1;
char *snd_dmxoption = "-opl3"; // [crispy] default to OPL3 emulation

static int numChannels = 8;
static int sfxVolume = 8;
static int musicVolume = 8;
static int snd_monomode = 0; // [JN] Mono SFX
static int use_libsamplerate = 0;
static float libsamplerate_scale = 0.65;

void ConfigSound(void)
{
    txt_window_t *window;

    // Build the window

    window = TXT_NewWindow("Sound configuration");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    TXT_SetColumnWidths(window, 35);
    TXT_SetWindowPosition(window, TXT_HORIZ_CENTER, TXT_VERT_TOP,
                                  TXT_SCREEN_W / 2, 4);

    TXT_AddWidgets(window,

        TXT_NewSeparator("Sound effects"),

        TXT_NewRadioButton("Disabled", &snd_sfxdevice, SNDDEVICE_NONE),
        TXT_NewRadioButton("Digital sound effects", &snd_sfxdevice, SNDDEVICE_SB),
        
        TXT_NewConditional(&snd_sfxdevice, SNDDEVICE_SB,
            TXT_NewHorizBox(TXT_NewStrut(4, 0),
            TXT_NewCheckBox("Pitch-shifted sounds", &snd_pitchshift), NULL)),

        TXT_NewConditional(&snd_sfxdevice, SNDDEVICE_SB,
            TXT_NewHorizBox(TXT_NewStrut(4, 0),
            TXT_NewCheckBox("Mono sounds", &snd_monomode), NULL)),

        TXT_NewSeparator("Music"),

        TXT_NewRadioButton("Disabled", &snd_musicdevice, SNDDEVICE_NONE),
        TXT_NewRadioButton("OPL (Adlib/Soundblaster)", &snd_musicdevice, SNDDEVICE_SB),

        NULL);


    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, TXT_NewWindowAbortAction(window));
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, TXT_NewWindowSelectAction(window));
}

void BindSoundVariables(void)
{
    M_BindIntVariable("snd_sfxdevice",            &snd_sfxdevice);
    M_BindIntVariable("snd_musicdevice",          &snd_musicdevice);
    M_BindIntVariable("snd_channels",             &numChannels);
    M_BindIntVariable("snd_samplerate",           &snd_samplerate);
    M_BindIntVariable("sfx_volume",               &sfxVolume);
    M_BindIntVariable("music_volume",             &musicVolume);
    M_BindIntVariable("snd_monomode",             &snd_monomode);

    M_BindIntVariable("use_libsamplerate",        &use_libsamplerate);
    M_BindFloatVariable("libsamplerate_scale",    &libsamplerate_scale);

    M_BindIntVariable("snd_maxslicetime_ms",      &snd_maxslicetime_ms);
    M_BindStringVariable("snd_musiccmd",          &snd_musiccmd);
    M_BindStringVariable("snd_dmxoption",         &snd_dmxoption);

    M_BindIntVariable("snd_cachesize",            &snd_cachesize);
    M_BindIntVariable("opl_io_port",              &opl_io_port);

    M_BindIntVariable("snd_pitchshift",           &snd_pitchshift);

    // Default sound volumes.
    sfxVolume = 8;  musicVolume = 8;
}

