//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:  none
//

#include <stdio.h>
#include <stdlib.h>

#include "i_sound.h"
#include "i_system.h"

#include "d_main.h"

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

#include "id_vars.h"
#include "id_func.h"

// when to clip out sounds
// Does not fit the large outdoor areas.

#define S_CLIPPING_DIST (1200 * FRACUNIT)

// Distance tp origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).
// In the source code release: (160*FRACUNIT).  Changed back to the
// Vanilla value of 200 (why was this changed?)

#define S_CLOSE_DIST (200 * FRACUNIT)

// The range over which sound attenuates

#define S_ATTENUATOR ((S_CLIPPING_DIST - S_CLOSE_DIST) >> FRACBITS)

// Stereo separation

#define S_STEREO_SWING (96 * FRACUNIT)
static int stereo_swing;

#define NORM_PRIORITY 64
#define NORM_SEP 128

typedef struct
{
    // sound information (if null, channel avail.)
    sfxinfo_t *sfxinfo;

    // origin of sound
    mobj_t *origin;

    // handle of the sound being played
    int handle;

    int pitch;

} channel_t;

// The set of channels available

static channel_t *channels;
static degenmobj_t *sobjs;

// Maximum volume of a sound effect.
// Internal default is max out of 0-15.

int sfxVolume = 8;

// Maximum volume of music.

int musicVolume = 8;

// Internal volume level, ranging from 0-127

static int snd_SfxVolume;

// Whether songs are mus_paused

static boolean mus_paused;

// Music currently being played

static musicinfo_t *mus_playing = NULL;

// [JN] Always allocate 8 SFX channels.
// No memory reallocation will be needed upon changing of channels number.

#define MAX_SND_CHANNELS 16

// [JN] External music number, used for music playback hot-swapping.
int current_mus_num;

// [JN] jff 3/17/98 to keep track of last IDMUS specified music num
int idmusnum;

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//

void S_Init(int sfxVolume, int musicVolume)
{
    int i;

    idmusnum = -1; // [JN] jff 3/17/98 insure idmus number is blank

    I_SetOPLDriverVer(opl_doom_1_9);
    I_PrecacheSounds(S_sfx, NUMSFX);

    S_SetSfxVolume(sfxVolume);
    S_SetMusicVolume(musicVolume);

    // Allocating the internal channels for mixing
    // (the maximum numer of sounds rendered
    // simultaneously) within zone memory.
    channels = Z_Malloc(MAX_SND_CHANNELS*sizeof(channel_t), PU_STATIC, 0);
    sobjs = Z_Malloc(MAX_SND_CHANNELS*sizeof(degenmobj_t), PU_STATIC, 0);

    // Free all channels for use
    for (i=0 ; i<MAX_SND_CHANNELS ; i++)
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

    I_AtExit(S_Shutdown, true);

    // [crispy] handle stereo separation for mono-sfx and flipped levels
    S_UpdateStereoSeparation();
}

// -----------------------------------------------------------------------------
// S_ChangeSFXSystem
// [JN] Routine for sfx device hot-swapping.
// -----------------------------------------------------------------------------

void S_ChangeSFXSystem (void)
{
    int i;

    // Free all channels for use
    for (i = 0 ; i < MAX_SND_CHANNELS ; i++)
    {
        channels[i].sfxinfo = 0;
    }

    // Reinitialize sfx usefulness
    for (i = 1 ; i < NUMSFX ; i++)
    {
        S_sfx[i].lumpnum = S_sfx[i].usefulness = -1;
    }
}

// -----------------------------------------------------------------------------
// S_UpdateStereoSeparation
// [JN] Defines stereo separtion for mono sfx mode and flipped levels.
// -----------------------------------------------------------------------------

void S_UpdateStereoSeparation (void)
{
	// [crispy] play all sound effects in mono
	if (snd_monosfx)
	{
		stereo_swing = 0;
	}
	else if (gp_flip_levels)
	{
		stereo_swing = -S_STEREO_SWING;
	}
	else
	{
		stereo_swing = S_STEREO_SWING;
	}
}

void S_Shutdown(void)
{
    I_ShutdownSound();
    I_ShutdownMusic();
}

static void S_StopChannel(int cnum)
{
    int i;
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

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//

void S_Start(void)
{
    int cnum;
    int mnum;

    // kill all playing sounds at start of level
    //  (trust me - a good idea)
    for (cnum=0 ; cnum<snd_channels ; cnum++)
    {
        if (channels[cnum].sfxinfo)
        {
            S_StopChannel(cnum);
        }
    }

    // start new music for the level
    if (musicVolume) // [crispy] do not reset pause state at zero music volume
    mus_paused = 0;

    if (idmusnum != -1)
    {
        mnum = idmusnum; //jff 3/17/98 reload IDMUS music if not -1
    }
    else
    {
        mnum = mus_map01 + gamemap - 1;
    }

    S_ChangeMusic(mnum, true);
}

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

// [crispy] removed map objects may finish their sounds
// When map objects are removed from the map by P_RemoveMobj(), instead of
// stopping their sounds, their coordinates are transfered to "sound objects"
// so stereo positioning and distance calculations continue to work even after
// the corresponding map object has already disappeared.
// Thanks to jeff-d and kb1 for discussing this feature and the former for the
// original implementation idea: https://www.doomworld.com/vb/post/1585325
void S_UnlinkSound(mobj_t *origin)
{
    int cnum;

    if (origin)
    {
        for (cnum=0 ; cnum<snd_channels ; cnum++)
        {
            if (channels[cnum].sfxinfo && channels[cnum].origin == origin)
            {
                degenmobj_t *const sobj = &sobjs[cnum];
                sobj->x = origin->x;
                sobj->y = origin->y;
                sobj->z = origin->z;
                channels[cnum].origin = (mobj_t *) sobj;
                break;
            }
        }
    }
}

//
// S_GetChannel :
//   If none available, return -1.  Otherwise channel #.
//

static int S_GetChannel(mobj_t *origin, sfxinfo_t *sfxinfo)
{
    // channel number to use
    int                cnum;

    channel_t*        c;

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
// P_ApproxDistanceZ
// [JN] Gives an estimation of distance using three axises.
// Adapted from EDGE, converted to fixed point math.
// [PN] Optimized with bitwise shifts (>> 1) for performance.
// Precise results can be obtained with sqrt, but it's computationally expensive:
//   return sqrt(dx * dx + dy * dy + dz * dz);
// -----------------------------------------------------------------------------

static int64_t S_ApproxDistanceZ (int64_t dx, int64_t dy, int64_t dz)
{
	dx = llabs(dx);
	dy = llabs(dy);
	dz = llabs(dz);

	int64_t dxy = (dy > dx) ? dy + (dx >> 1) : dx + (dy >> 1);

	return (dz > dxy) ? dz + (dxy >> 1) : dxy + (dz >> 1);
}

//
// Changes volume and stereo-separation variables
//  from the norm of a sound effect to be played.
// If the sound is not audible, returns a 0.
// Otherwise, modifies parameters and returns 1.
//

static int S_AdjustSoundParams(mobj_t *listener, mobj_t *source,
                               int *vol, int *sep)
{
    int64_t        approx_dist;
    int64_t        adx;
    int64_t        ady;
    int64_t        adz; // [JN] Z-axis sfx distance
    angle_t        angle;

    // calculate the distance to sound origin
    //  and clip it if necessary
    adx = abs(listener->x - source->x);
    ady = abs(listener->y - source->y);
    adz = abs(listener->z - source->z);

    // [JN] Always use XYZ sound attenuation.
    approx_dist = S_ApproxDistanceZ(adx, ady, adz);

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
    *sep = 128 - (FixedMul(stereo_swing, finesine[angle]) >> FRACBITS);

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

void S_StartSound(void *origin_p, int sfx_id)
{
    sfxinfo_t *sfx;
    mobj_t *origin;
    int rc;
    int sep;
    int pitch;
    int cnum;
    int volume;

    if (!snd_SfxVolume)
    {
        return;
    }

    origin = (mobj_t *) origin_p;
    volume = snd_SfxVolume;

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
    if (origin && origin != players[displayplayer].mo && origin != players[displayplayer].so) // [crispy] weapon sound source
    {
        rc = S_AdjustSoundParams(players[displayplayer].mo,
                                 origin,
                                 &volume,
                                 &sep);

        if (origin->x == players[displayplayer].mo->x
         && origin->y == players[displayplayer].mo->y)
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

    // [JN] Jaguar: optionally emulate real Jaguar hardware lower pitch.
    pitch = 120;

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

//
// Stop and resume music, during game PAUSE.
//

void S_PauseSound(void)
{
    if (mus_playing && !mus_paused)
    {
        I_PauseSong();
        mus_paused = true;
    }
}

void S_ResumeSound(void)
{
    if (mus_playing && mus_paused)
    {
        I_ResumeSong();
        mus_paused = false;
    }
}

//
// Updates music & sounds
//

void S_UpdateSounds(mobj_t *listener)
{
    int                audible;
    int                cnum;
    int                volume;
    int                sep;
    sfxinfo_t*        sfx;
    channel_t*        c;

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
                if (c->origin && listener != c->origin && c->origin != players[displayplayer].so) // [crispy] weapon sound source
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

void S_SetMusicVolume(int volume)
{
    if (volume < 0 || volume > 127)
    {
        I_Error("Attempt to set music volume at %d",
                volume);
    }

    // [crispy] [JN] Fixed bug when music was hearable with zero volume
    if (!musicVolume)
    {
        S_PauseSound();
    }
    else
    if (!paused)
    {
        S_ResumeSound();
    }

    I_SetMusicVolume(volume);
}

void S_SetSfxVolume(int volume)
{
    if (volume < 0 || volume > 127)
    {
        I_Error("Attempt to set sfx volume at %d", volume);
    }

    snd_SfxVolume = volume;
}

//
// Starts some music with the music id found in sounds.h.
//

void S_ChangeMusic(int musicnum, int looping)
{
    musicinfo_t *music = NULL;
    char namebuf[9];
    void *handle;

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

    // [JN] After inner muscial number has been set, sync it with
    // external number, used in M_ID_MusicSystem.
    current_mus_num = musicnum;

    // shutdown old music
    S_StopMusic();

    // [JN] CRY: different music handling, depending on game state
    // to emulate both PC and Jaguar music arrangement.
    switch (gamestate) 
    {
        case GS_DEMOSCREEN:
            M_snprintf(namebuf, sizeof(namebuf), "m_intro");
            music->lumpnum = W_GetNumForName(namebuf);
        break;

        case GS_LEVEL:
            if (emu_jaguar_music)
            {
                // [JN] No music on game levels, 
                // so don't try to play empty music!
                return;
            }
            else
            {
                M_snprintf(namebuf, sizeof(namebuf), "m_%s", music->name);
                music->lumpnum = W_GetNumForName(namebuf);
            }
        break;

        case GS_INTERMISSION:
            if (emu_jaguar_music)
            {
                const int jag_intermusic[] =
                {
                    1, 2, 4, 6, 9, 10, 11, 14, 16, 17,
                    1, 2, 4, 6, 9, 10, 11, 14, 16, 17,
                    1, 2, 4, 6, 6
                };
                M_snprintf(namebuf, sizeof(namebuf), "m_map%02d", jag_intermusic[gamemap]);
            }
            else
            {
                M_snprintf(namebuf, sizeof(namebuf), "m_inter");
            }
            music->lumpnum = W_GetNumForName(namebuf);
        break;

        case GS_FINALE:
            M_snprintf(namebuf, sizeof(namebuf), "m_map02");
            music->lumpnum = W_GetNumForName(namebuf);
        break;
    }

    music->data = W_CacheLumpNum(music->lumpnum, PU_STATIC);

    handle = I_RegisterSong(music->data, W_LumpLength(music->lumpnum));
    music->handle = handle;
    I_PlaySong(handle, looping);

    mus_playing = music;
}

boolean S_MusicPlaying(void)
{
    return I_MusicIsPlaying();
}

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

// -----------------------------------------------------------------------------
// S_MuteUnmuteSound
// [JN] Sets sfx and music volume to 0 when window loses 
//      it's focus and restores back when focus is regained.
// -----------------------------------------------------------------------------

void S_MuteUnmuteSound (boolean mute)
{
    if (mute)
    {
        // Stop all sounds and clear sfx channels.
        for (int i = 0 ; i < snd_channels ; i++)
        {
            S_StopChannel(i);
        }

        // Set volume variables to zero.
        S_SetMusicVolume(0);
        S_SetSfxVolume(0);
    }
    else
    {
        S_SetMusicVolume(musicVolume * 8);
        S_SetSfxVolume(sfxVolume * 8);
    }

    // All done, no need to invoke function until next 
    // minimizing/restoring of game window is happened.
    volume_needs_update = false;
}
