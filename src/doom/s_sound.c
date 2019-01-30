//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2019 Julia Nechaevskaya
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
#include <stdlib.h>

#include "i_sound.h"
#include "i_system.h"
#include "doomfeatures.h"
#include "deh_str.h"
#include "doomstat.h"
#include "doomtype.h"
#include "sounds.h"
#include "s_sound.h"
#include "m_misc.h"
#include "m_random.h"
#include "m_argv.h"
#include "p_local.h"
#include "w_wad.h"
#include "z_zone.h"
#include "jn.h"


// when to clip out sounds
// Does not fit the large outdoor areas.
#define S_CLIPPING_DIST (1200 * FRACUNIT)

// Distance tp origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).
#define S_CLOSE_DIST (200 * FRACUNIT)

// The range over which sound attenuates
#define S_ATTENUATOR ((S_CLIPPING_DIST - S_CLOSE_DIST) >> FRACBITS)

// Stereo separation
#define S_STEREO_SWING  (96 * FRACUNIT)
#define NORM_PRIORITY   64
#define NORM_SEP        128

typedef struct
{
    sfxinfo_t *sfxinfo; // sound information (if null, channel avail.)
    mobj_t    *origin;  // origin of sound
    int        handle;  // handle of the sound being played
    int        pitch;
} channel_t;


static channel_t *channels; // The set of channels available
int sfxVolume = 8;          // Maximum volume of a sound effect
int musicVolume = 8;        // Maximum volume of music
static int snd_SfxVolume;   // Internal volume level, ranging from 0-127
static boolean mus_paused;              // Whether songs are mus_paused
static musicinfo_t *mus_playing = NULL; // Music currently being played
int snd_channels = 8;       // Number of channels to use


// -----------------------------------------------------------------------------
// S_Init
//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
// -----------------------------------------------------------------------------

void S_Init (int sfxVolume, int musicVolume)
{
    int i;

    I_SetOPLDriverVer(opl_doom_1_9);
    I_PrecacheSounds(S_sfx, NUMSFX);

    S_SetSfxVolume(sfxVolume);
    S_SetMusicVolume(musicVolume);

    // Allocating the internal channels for mixing
    // (the maximum numer of sounds rendered
    // simultaneously) within zone memory.
    channels = Z_Malloc(snd_channels*sizeof(channel_t), PU_STATIC, 0);

    // Free all channels for use
    for (i=0 ; i<snd_channels ; i++)
    {
        channels[i].sfxinfo = 0;
    }

    // no sounds are playing, and they are not mus_paused
    mus_paused = 0;

    // Note that sounds have not been cached (yet).
    for (i=1 ; i<NUMSFX ; i++)
    {
        S_sfx[i].lumpnum = S_sfx[i].usefulness = -1;
    }

    // Doom defaults to pitch-shifting off.
    if (snd_pitchshift == -1)
    {
        snd_pitchshift = 0;
    }

    I_AtExit(S_Shutdown, true);
}


// -----------------------------------------------------------------------------
// S_Shutdown
// -----------------------------------------------------------------------------

void S_Shutdown(void)
{
    I_ShutdownSound();
    I_ShutdownMusic();
}


// -----------------------------------------------------------------------------
// S_StopChannel
// -----------------------------------------------------------------------------

static void S_StopChannel(int cnum)
{
    int        i;
    channel_t *c;

    c = &channels[cnum];

    if (c->sfxinfo)
    {
        // stop the sound playing

        if (I_SoundIsPlaying(c->handle))
        {
            I_StopSound(c->handle);
        }

        // check to see if other channels are playing the sound

        for (i=0; i<snd_channels; i++)
        {
            if (cnum != i && c->sfxinfo == channels[i].sfxinfo)
            {
                break;
            }
        }

        // degrade usefulness of sound data

        c->sfxinfo->usefulness--;
        c->sfxinfo = NULL;
        c->origin = NULL;
    }
}


// -----------------------------------------------------------------------------
// S_Start
//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
// -----------------------------------------------------------------------------

void S_Start(void)
{
    int  cnum;
    int  mnum;

    // kill all playing sounds at start of level (trust me - a good idea)
    for (cnum=0 ; cnum<snd_channels ; cnum++)
    {
        if (channels[cnum].sfxinfo)
        {
            S_StopChannel(cnum);
        }
    }

    // start new music for the level
    mus_paused = 0;

    // [Julia] Jaguar music number
    mnum = mus_map01 + gamemap - 1;

    S_ChangeMusic(mnum, true);
}


// -----------------------------------------------------------------------------
// S_StopSound
// -----------------------------------------------------------------------------

void S_StopSound(mobj_t *origin)
{
    int cnum;

    for (cnum=0 ; cnum<snd_channels ; cnum++)
    {
        if (channels[cnum].sfxinfo && channels[cnum].origin == origin)
        {
            S_StopChannel(cnum);
            break;
        }
    }
}


// -----------------------------------------------------------------------------
// S_GetChannel :
//   If none available, return -1.  Otherwise channel #.
// -----------------------------------------------------------------------------

static int S_GetChannel(mobj_t *origin, sfxinfo_t *sfxinfo)
{
    // channel number to use
    int         cnum;
    channel_t  *c;

    // Find an open channel
    for (cnum=0 ; cnum<snd_channels ; cnum++)
    {
        if (!channels[cnum].sfxinfo)
        {
            break;
        }
        else if (origin && channels[cnum].origin == origin)
        {
            S_StopChannel(cnum);
            break;
        }
    }

    // None available
    if (cnum == snd_channels)
    {
        // Look for lower priority
        for (cnum=0 ; cnum<snd_channels ; cnum++)
        {
            if (channels[cnum].sfxinfo->priority >= sfxinfo->priority)
            {
                break;
            }
        }

        if (cnum == snd_channels)
        {
            // FUCK!  No lower priority.  Sorry, Charlie.
            return -1;
        }
        else
        {
            // Otherwise, kick out lower priority.
            S_StopChannel(cnum);
        }
    }

    c = &channels[cnum];

    // channel is decided to be cnum.
    c->sfxinfo = sfxinfo;
    c->origin = origin;

    return cnum;
}

// -----------------------------------------------------------------------------
// S_AdjustSoundParams
//
// Changes volume and stereo-separation variables
//  from the norm of a sound effect to be played.
// If the sound is not audible, returns a 0.
// Otherwise, modifies parameters and returns 1.
// -----------------------------------------------------------------------------

static int S_AdjustSoundParams(mobj_t *listener, mobj_t *source,
                               int *vol, int *sep)
{
    fixed_t  approx_dist;
    fixed_t  adx;
    fixed_t  ady;
    angle_t  angle;

    // calculate the distance to sound origin and clip it if necessary
    adx = abs(listener->x - source->x);
    ady = abs(listener->y - source->y);

    // From _GG1_ p.428. Appox. eucledian distance fast.
    approx_dist = adx + ady - ((adx < ady ? adx : ady)>>1);


    if (approx_dist > S_CLIPPING_DIST)
    {
        return 0;
    }

    // angle of source to listener
    angle = R_PointToAngle2(listener->x,
                            listener->y,
                            source->x,
                            source->y);

    if (angle > listener->angle)
    {
        angle = angle - listener->angle;
    }
    else
    {
        angle = angle + (0xffffffff - listener->angle);
    }

    angle >>= ANGLETOFINESHIFT;

    // stereo separation
    // [Julia] Support for mono sfx mode
    *sep = snd_monomode ? 128 : 128 - (FixedMul(S_STEREO_SWING, 
                                                finesine[angle]) >> FRACBITS);

    // volume calculation
    if (approx_dist < S_CLOSE_DIST)
    {
        *vol = snd_SfxVolume;
    }
    else
    {
        // distance effect
        *vol = (snd_SfxVolume 
                * ((S_CLIPPING_DIST - approx_dist)>>FRACBITS))
                / S_ATTENUATOR;
    }

    return (*vol > 0);
}


// -----------------------------------------------------------------------------
// S_StartSound
// -----------------------------------------------------------------------------

void S_StartSound(void *origin_p, int sfx_id)
{
    sfxinfo_t  *sfx;
    mobj_t     *origin;
    int         rc;
    int         sep;
    int         pitch;
    int         cnum;
    int         volume;

    origin = (mobj_t *) origin_p;
    volume = snd_SfxVolume;

    // [crispy] make non-fatal, consider zero volume
    if (sfx_id == sfx_None || !snd_SfxVolume)
    {
        return;
    }

    // check for bogus sound #
    if (sfx_id < 1 || sfx_id > NUMSFX)
    {
        I_Error("Bad sfx #: %d", sfx_id);
    }

    sfx = &S_sfx[sfx_id];

    // Initialize sound parameters
    pitch = NORM_PITCH;
    if (sfx->link)
    {
        volume += sfx->volume;
        pitch = sfx->pitch;

        if (volume < 1)
        {
            return;
        }

        if (volume > snd_SfxVolume)
        {
            volume = snd_SfxVolume;
        }
    }

    // Check to see if it is audible,
    //  and if not, modify the params
    if (origin && origin != players[consoleplayer].mo)
    {
        rc = S_AdjustSoundParams(players[consoleplayer].mo,
                                 origin,
                                 &volume,
                                 &sep);

        if (origin->x == players[consoleplayer].mo->x
         && origin->y == players[consoleplayer].mo->y)
        {
            sep = NORM_SEP;
        }

        if (!rc)
        {
            return;
        }
    }
    else
    {
        sep = NORM_SEP;
    }

    // [Julia] Jaguar: down-pitched sounds, they are not variative.
    pitch = 122;

    // kill old sound
    S_StopSound(origin);

    // try to find a channel
    cnum = S_GetChannel(origin, sfx);

    if (cnum < 0)
    {
        return;
    }

    // increase the usefulness
    if (sfx->usefulness++ < 0)
    {
        sfx->usefulness = 1;
    }

    if (sfx->lumpnum < 0)
    {
        sfx->lumpnum = I_GetSfxLumpNum(sfx);
    }

    channels[cnum].pitch = pitch;
    channels[cnum].handle = I_StartSound(sfx, cnum, volume, sep, channels[cnum].pitch);
}


// -----------------------------------------------------------------------------
// S_StartSoundOnce
// -----------------------------------------------------------------------------

void S_StartSoundOnce (void *origin_p, int sfx_id)
{
    int cnum;
    const sfxinfo_t *const sfx = &S_sfx[sfx_id];

    for (cnum = 0; cnum < snd_channels; cnum++)
    {
        if (channels[cnum].sfxinfo == sfx &&
            channels[cnum].origin == origin_p)
        {
            return;
        }
    }

    S_StartSound(origin_p, sfx_id);
}


// -----------------------------------------------------------------------------
// S_PauseSound
//
// Stop and resume music, during game PAUSE.
// -----------------------------------------------------------------------------

void S_PauseSound(void)
{
    if (mus_playing && !mus_paused)
    {
        I_PauseSong();
        mus_paused = true;
    }
}


// -----------------------------------------------------------------------------
// S_ResumeSound
// -----------------------------------------------------------------------------

void S_ResumeSound(void)
{
    if (mus_playing && mus_paused)
    {
        I_ResumeSong();
        mus_paused = false;
    }
}


// -----------------------------------------------------------------------------
// S_UpdateSounds
//
// Updates music & sounds
// -----------------------------------------------------------------------------

void S_UpdateSounds(mobj_t *listener)
{
    int         audible;
    int         cnum;
    int         volume;
    int         sep;
    sfxinfo_t  *sfx;
    channel_t  *c;

    I_UpdateSound();

    for (cnum=0; cnum<snd_channels; cnum++)
    {
        c = &channels[cnum];
        sfx = c->sfxinfo;

        if (c->sfxinfo)
        {
            if (I_SoundIsPlaying(c->handle))
            {
                // initialize parameters
                volume = snd_SfxVolume;
                sep = NORM_SEP;

                if (sfx->link)
                {
                    volume += sfx->volume;
                    if (volume < 1)
                    {
                        S_StopChannel(cnum);
                        continue;
                    }
                    else if (volume > snd_SfxVolume)
                    {
                        volume = snd_SfxVolume;
                    }
                }

                // check non-local sounds for distance clipping
                //  or modify their params
                if (c->origin && listener != c->origin)
                {
                    audible = S_AdjustSoundParams(listener,
                                                  c->origin,
                                                  &volume,
                                                  &sep);

                    if (!audible)
                    {
                        S_StopChannel(cnum);
                    }
                    else
                    {
                        I_UpdateSoundParams(c->handle, volume, sep);
                    }
                }
            }
            else
            {
                // if channel is allocated but sound has stopped,
                //  free it
                S_StopChannel(cnum);
            }
        }
    }
}


// -----------------------------------------------------------------------------
// S_SetMusicVolume
// -----------------------------------------------------------------------------

void S_SetMusicVolume(int volume)
{
    if (volume < 0 || volume > 127)
    {
        I_Error("Attempt to set music volume at %d", volume);
    }

    // [crispy] & [Julia] Fixed bug when music was hearable with zero volume
    if (musicVolume == 0)
    {
        S_PauseSound();
    }
    else if (!paused)
    {
        S_ResumeSound();
    }

    I_SetMusicVolume(volume);
}


// -----------------------------------------------------------------------------
// S_SetSfxVolume
// -----------------------------------------------------------------------------

void S_SetSfxVolume(int volume)
{
    if (volume < 0 || volume > 127)
    {
        I_Error("Attempt to set sfx volume at %d", volume);
    }

    snd_SfxVolume = volume;
}


// -----------------------------------------------------------------------------
// S_StartMusic
//
// Starts some music with the music id found in sounds.h.
// -----------------------------------------------------------------------------

void S_StartMusic(int m_id)
{
    S_ChangeMusic(m_id, false);
}


// -----------------------------------------------------------------------------
// S_ChangeMusic
// -----------------------------------------------------------------------------

void S_ChangeMusic(int musicnum, int looping)
{
    musicinfo_t  *music = NULL;
    char          namebuf[9];
    void         *handle;

    if (musicnum <= mus_None || musicnum >= NUMMUSIC)
    {
        I_Error("Bad music number %d", musicnum);
    }
    else
    {
        music = &S_music[musicnum];
    }

    if (mus_playing == music)
    {
        return;
    }

    // shutdown old music
    S_StopMusic();

    // get lumpnum if neccessary
    if (!music->lumpnum)
    {
        M_snprintf(namebuf, sizeof(namebuf), "d_%s", DEH_String(music->name));
        music->lumpnum = W_GetNumForName(namebuf);
    }

    music->data = W_CacheLumpNum(music->lumpnum, PU_STATIC);

    handle = I_RegisterSong(music->data, W_LumpLength(music->lumpnum));
    music->handle = handle;
    I_PlaySong(handle, looping);

    mus_playing = music;
}


// -----------------------------------------------------------------------------
// S_MusicPlaying
// -----------------------------------------------------------------------------

boolean S_MusicPlaying(void)
{
    return I_MusicIsPlaying();
}


// -----------------------------------------------------------------------------
// S_StopMusic
// -----------------------------------------------------------------------------

void S_StopMusic(void)
{
    if (mus_playing)
    {
        if (mus_paused)
        {
            I_ResumeSong();
        }

        I_StopSong();
        I_UnRegisterSong(mus_playing->handle);
        W_ReleaseLumpNum(mus_playing->lumpnum);
        mus_playing->data = NULL;
        mus_playing = NULL;
    }
}

