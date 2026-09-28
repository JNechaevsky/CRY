//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2026 Julia Nechaevskaya
// Copyright(C) 2026 Polina "Aura" N.
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
//  Jaguar Doom native music ("DMX-Jag") software synthesizer for CRY.
//  Based on Calico Doom's Jaguar music player:
//  https://github.com/team-eternity/calico-doom
//   src/s_sound.c and src/sdl/sdl_sound.cpp
//
//  Copyright (c) 2016 James Haley, the MIT License (MIT).


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "SDL.h"
#include "SDL_mixer.h"

#include "doomtype.h"
#include "z_zone.h"
#include "w_wad.h"
#include "i_sound.h"
#include "i_jagmusic.h"

// ---------------------------------------------------------------------------
// Format constants
// ---------------------------------------------------------------------------

#define JAG_INST_RATE   11025     // instrument samples are stored at 11025 Hz
#define JAG_SFX_HDR     28        // sfx_t header size (BE u32 x3 + padding)
#define JAG_MAX_INST    256
#define JAG_NUM_CH      16        // sequencer channels (nibble 0..15)

// Command opcodes (upper nibble of the command byte).
enum
{
    JAGC_NOP    = 0x00,   // no operation
    JAGC_NOTEON = 0x10,   // note on:  [inst][vol][unused][step:24b BE]
    JAGC_NOTEOFF= 0x20,   // note off
    JAGC_TEMPO  = 0x30,   // set tempo:[sppc:32b BE]
    JAGC_VOL    = 0x40,   // set vol:  [vol][unused]
    JAGC_STEP   = 0x50    // set step: [step:24b BE]
};

// ---------------------------------------------------------------------------
// Instrument bank
// ---------------------------------------------------------------------------

typedef struct
{
    float *data;          // converted [-1,1] mono PCM
    int    len;           // sample count
    int    loopstart;     // native sample index, or -1 if no loop
} jag_inst_t;

static jag_inst_t jag_inst[JAG_MAX_INST];
static boolean    jag_bank_loaded = false;
static boolean    jag_bank_attempted = false;
static int        jag_bank_count = 0;

// ---------------------------------------------------------------------------
// Sequencer channels
// ---------------------------------------------------------------------------

typedef struct
{
    int    active;
    int    inst;
    double pos;           // fractional index into instrument data
    double inc;           // native samples consumed per output sample
    double vol;           // 0..1 (per-channel)
    int    loopstart;
    int    len;
    const float *data;
} jag_ch_t;

static jag_ch_t jag_ch[JAG_NUM_CH];

// ---------------------------------------------------------------------------
// Song state (guarded by a simple flag; audio thread reads, main writes)
// ---------------------------------------------------------------------------

typedef enum { JAG_READ_DELAY, JAG_READ_CMD } jag_state_t;

static const unsigned char *jag_music = NULL;
static const unsigned char *jag_start = NULL;
static const unsigned char *jag_end   = NULL;
static int    jag_looping = 0;
static int    jag_playing = 0;
static int    jag_paused  = 0;
static double jag_cur_sample = 0.0;      // absolute output-sample clock
static double jag_next_event = 0.0;      // output-sample time of next event
static double jag_inc_per_delay = 0.0;   // sppc * mixrate / 20500
static jag_state_t jag_state = JAG_READ_DELAY;
static int    jag_mixrate = 44100;
static double jag_master = 1.0;          // 0..1 master music volume

static void jag_read_be24(const unsigned char *p, int *out)
{
    *out = (p[0] << 16) | (p[1] << 8) | p[2];
}
static int jag_read_be32(const unsigned char *p)
{
    return (int)(((unsigned)p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
}

// Recompute the delay->samples multiplier from the current sppc.
static void jag_recompute_tempo(int sppc)
{
    // Calico: ms_per_clock = sppc / 20.5 ; samples = ms * rate / 1000
    jag_inc_per_delay = (double)sppc * (double)jag_mixrate / 20500.0;
}

// Convert a 24-bit step value to a native-sample increment.
static double jag_step_to_inc(int step)
{
    // Calico plays (mixrate-resampled) data with increment (step<<1)/65536.
    // We keep native 11025 data, so scale by 11025/mixrate.
    return (double)(step << 1) / 65536.0 * (double)JAG_INST_RATE / (double)jag_mixrate;
}

static void jag_stop_channel(int ch)
{
    if (ch >= 0 && ch < JAG_NUM_CH)
        jag_ch[ch].active = 0;
}

// Process one command byte (Calico M_GetEvent). Returns 1 to stop the song.
static int jag_do_event(void)
{
    int cmd = *jag_music++;
    int ch  = cmd & 15;
    int hi  = cmd & 240;

    switch (hi)
    {
        case JAGC_TEMPO:
        {
            int sppc = jag_read_be32(jag_music);
            jag_music += 4;
            jag_recompute_tempo(sppc);
            break;
        }
        case JAGC_NOTEON:
        {
            int inst  = *jag_music++;
            int vol   = *jag_music++;
            jag_music++;                       // unused byte
            int step; jag_read_be24(jag_music, &step); jag_music += 3;

            if (ch < JAG_NUM_CH && inst < JAG_MAX_INST && jag_inst[inst].data)
            {
                jag_ch_t *c = &jag_ch[ch];
                c->active    = 1;
                c->inst      = inst;
                c->data      = jag_inst[inst].data;
                c->len       = jag_inst[inst].len;
                c->loopstart = jag_inst[inst].loopstart;
                c->pos       = 0.0;
                c->inc       = jag_step_to_inc(step);
                c->vol       = (vol / 191.0);
                if (c->vol > 1.0) c->vol = 1.0;
            }
            break;
        }
        case JAGC_NOTEOFF:
            jag_stop_channel(ch);
            break;
        case JAGC_VOL:
        {
            int vol = *jag_music++;
            jag_music++;                       // unused byte
            if (ch < JAG_NUM_CH && jag_ch[ch].active)
            {
                double v = vol / 191.0;
                jag_ch[ch].vol = v > 1.0 ? 1.0 : v;
            }
            break;
        }
        case JAGC_STEP:
        {
            int step; jag_read_be24(jag_music, &step); jag_music += 3;
            if (ch < JAG_NUM_CH && jag_ch[ch].active)
                jag_ch[ch].inc = jag_step_to_inc(step);
            break;
        }
        case JAGC_NOP:
            break;
        default:
            return 1;                          // unknown opcode -> stop
    }
    return 0;
}

// Mix n output frames into the (already-populated) int16 stereo buffer.
static void jag_mix(int16_t *stream, int frames)
{
    for (int i = 0; i < frames; i++)
    {
        double acc = 0.0;

        for (int c = 0; c < JAG_NUM_CH; c++)
        {
            jag_ch_t *ch = &jag_ch[c];
            if (!ch->active)
                continue;

            int idx = (int)ch->pos;
            if (idx >= ch->len - 1)
            {
                if (ch->loopstart >= 0)
                {
                    ch->pos = (double)ch->loopstart;
                    idx = ch->loopstart;
                    if (idx >= ch->len - 1) { ch->active = 0; continue; }
                }
                else
                {
                    ch->active = 0;
                    continue;
                }
            }

            double frac = ch->pos - idx;
            double s = ch->data[idx] * (1.0 - frac) + ch->data[idx + 1] * frac;
            acc += s * ch->vol;
            ch->pos += ch->inc;
        }

        if (acc != 0.0)
        {
            acc *= jag_master;
            int16_t *l = &stream[i * 2];
            int16_t *r = l + 1;
            double fl = (double)*l + acc * 32767.0;
            double fr = (double)*r + acc * 32767.0;
            if (fl >  32767.0) fl =  32767.0;
            if (fl < -32768.0) fl = -32768.0;
            if (fr >  32767.0) fr =  32767.0;
            if (fr < -32768.0) fr = -32768.0;
            *l = (int16_t)fl;
            *r = (int16_t)fr;
        }
    }
}

// SDL2_mixer post-mix callback: advance the sequencer and add our audio.
static void jag_postmix(void *udata, Uint8 *bytes, int len)
{
    int16_t *stream = (int16_t *)bytes;
    int frames = len / 4;                     // S16 stereo = 4 bytes/frame

    if (!jag_playing || jag_paused || !jag_music)
        return;

    int done = 0;
    while (done < frames)
    {
        // Fire every event scheduled up to the current sample.
        while (jag_cur_sample >= jag_next_event)
        {
            // wrap / end handling before touching the stream
            if (jag_music >= jag_end)
            {
                if (!jag_looping)
                {
                    jag_playing = 0;
                    return;
                }
                jag_music = jag_start;
                jag_state = JAG_READ_DELAY;
            }

            if (jag_state == JAG_READ_DELAY)
            {
                int d = *jag_music++;
                jag_next_event += (double)d * jag_inc_per_delay;
                jag_state = JAG_READ_CMD;
                if (jag_cur_sample < jag_next_event)
                    break;                    // cmd fires later
            }
            else /* JAG_READ_CMD */
            {
                if (jag_do_event())
                {
                    jag_playing = 0;
                    return;
                }
                jag_state = JAG_READ_DELAY;
            }
        }

        int seg = (int)(jag_next_event - jag_cur_sample);
        if (seg <= 0) seg = 1;
        if (seg > frames - done) seg = frames - done;

        jag_mix(stream + done * 2, seg);
        jag_cur_sample += seg;
        done += seg;
    }
}

// ---------------------------------------------------------------------------
// Instrument bank loading
// ---------------------------------------------------------------------------

static int jag_parse_inst_num(const char *name)
{
    // "I001" -> 1 ; "P036" -> 36 + 128
    if (name[0] != 'I' && name[0] != 'P' && name[0] != 'i' && name[0] != 'p')
        return -1;
    if (!(name[1] >= '0' && name[1] <= '9')) return -1;
    if (!(name[2] >= '0' && name[2] <= '9')) return -1;
    if (!(name[3] >= '0' && name[3] <= '9')) return -1;
    int num = (name[1]-'0')*100 + (name[2]-'0')*10 + (name[3]-'0');
    if (name[0] == 'P' || name[0] == 'p')
        num += 128;
    return (num >= 0 && num < JAG_MAX_INST) ? num : -1;
}

static void jag_load_one(int lumpnum)
{
    lumpinfo_t *li = lumpinfo[lumpnum];
    int idx = jag_parse_inst_num(li->name);
    if (idx < 0)
        return;

    int size = W_LumpLength(lumpnum);
    if (size <= JAG_SFX_HDR)
        return;

    const unsigned char *d = W_CacheLumpNum(lumpnum, PU_CACHE);

    int samples   = jag_read_be32(d + 0);
    int loopstart = jag_read_be32(d + 4);
    int loopend   = jag_read_be32(d + 8);

    if (samples != size - JAG_SFX_HDR || samples < 4)
    {
        W_ReleaseLumpNum(lumpnum);
        return;
    }

    // Percussion patches use loopstart == 0xffffffff (no loop); a normal
    // loop needs loopstart < samples and loopend within the sample.
    int has_loop = !(loopstart == (int)0xffffffffu)
                && loopstart >= 0 && loopstart < samples
                && loopend <= samples && loopstart <= loopend;

    float *f = Z_Malloc(sizeof(float) * samples, PU_STATIC, NULL);
    const unsigned char *pcm = d + JAG_SFX_HDR;
    for (int i = 0; i < samples; i++)
    {
        f[i] = (float)pcm[i] * 2.0f / 255.0f - 1.0f;   // match Calico
    }

    W_ReleaseLumpNum(lumpnum);

    if (jag_inst[idx].data)
    {
        Z_Free(jag_inst[idx].data);
    }
    jag_inst[idx].data      = f;
    jag_inst[idx].len       = samples;
    jag_inst[idx].loopstart = has_loop ? loopstart : -1;
    jag_bank_count++;
}

boolean I_JagMusic_Init(void)
{
    if (jag_bank_attempted)
        return jag_bank_loaded;

    jag_bank_attempted = true;

    lumpindex_t start = W_CheckNumForName("INSTSTRT");
    lumpindex_t end   = W_CheckNumForName("INSTEND");

    if (start < 0 || end < 0 || end <= start + 1)
    {
        return (jag_bank_loaded = false);
    }

    for (lumpindex_t l = start + 1; l < end; l++)
    {
        jag_load_one(l);
    }

    jag_bank_loaded = (jag_bank_count > 0);
    return jag_bank_loaded;
}

boolean I_JagMusic_Present(void)
{
    return jag_bank_loaded;
}

// ---------------------------------------------------------------------------
// Native-format validator (walks the event stream like the player would)
// ---------------------------------------------------------------------------

boolean I_JagMusic_IsNative(const void *data, int len)
{
    if (!jag_bank_loaded || len < 16)
        return false;

    const unsigned char *p = (const unsigned char *)data;
    const unsigned char *end = p + len;
    int events = 0, saw_tempo = 0;

    while (p < end)
    {
        p++;                                  // delay byte
        if (p >= end) break;
        int hi = *p++ & 240;

        switch (hi)
        {
            case JAGC_TEMPO:  p += 4; saw_tempo = 1; break;
            case JAGC_NOTEON: p += 6; break;
            case JAGC_NOTEOFF: break;
            case JAGC_VOL:    p += 2; break;
            case JAGC_STEP:   p += 3; break;
            case JAGC_NOP:    break;
            default:
                return false;                 // unknown opcode?
        }
        events++;
    }

    // A real track consumes (almost) the whole lump and has a tempo marker.
    return saw_tempo && events > 8 && p >= end - 2;
}

// ---------------------------------------------------------------------------
// Song handle / module plumbing
// ---------------------------------------------------------------------------

typedef struct
{
    const unsigned char *data;
    int len;
} jag_song_t;

static jag_song_t jag_current = { NULL, 0 };

static boolean I_Jag_Init(void)
{
    return I_JagMusic_Init();
}

static void I_Jag_Shutdown(void)
{
    Mix_SetPostMix(NULL, NULL);
}

static void I_Jag_SetVolume(int volume)
{
    jag_master = (volume <= 0) ? 0.0 : (volume / 127.0);
}

static void I_Jag_Pause(void)
{
    jag_paused = 1;
}

static void I_Jag_Resume(void)
{
    jag_paused = 0;
}

static void *I_Jag_RegisterSong(void *data, int len)
{
    if (!I_JagMusic_Init())
        return NULL;

    jag_current.data = (const unsigned char *)data;
    jag_current.len  = len;
    return &jag_current;
}

static void I_Jag_UnRegisterSong(void *handle)
{
    (void)handle;
    jag_current.data = NULL;
    jag_current.len  = 0;
}

static void I_Jag_PlaySong(void *handle, boolean looping)
{
    (void)handle;
    if (!jag_current.data || !jag_current.len)
    {
        return;
    }

    int channels = 2;
    Uint16 fmt;

    if (Mix_QuerySpec(&jag_mixrate, &fmt, &channels))
    {
        if (jag_mixrate <= 0)
            jag_mixrate = 44100;
    }
    else
    {
        jag_mixrate = 44100;
    }

    for (int c = 0; c < JAG_NUM_CH; c++)
    {
        jag_ch[c].active = 0;
    }

    jag_music = jag_start = jag_current.data;
    jag_end   = jag_current.data + jag_current.len;
    jag_looping = looping;
    jag_cur_sample = 0.0;
    jag_next_event = 0.0;
    jag_state = JAG_READ_DELAY;
    jag_inc_per_delay = 0.0;                  // until first tempo event
    jag_playing = 1;
    jag_paused = 0;

    Mix_SetPostMix(jag_postmix, NULL);
}

static void I_Jag_StopSong(void)
{
    jag_playing = 0;
    Mix_SetPostMix(NULL, NULL);

    for (int c = 0; c < JAG_NUM_CH; c++)
    {
        jag_ch[c].active = 0;
    }
}

static boolean I_Jag_IsPlaying(void)
{
    return jag_playing && !jag_paused;
}

static void I_Jag_Poll(void)
{
    // No-op!
}

const music_module_t music_jag_module =
{
    NULL, 0,                 // no snddevice auto-selection
    I_Jag_Init,
    I_Jag_Shutdown,
    I_Jag_SetVolume,
    I_Jag_Pause,
    I_Jag_Resume,
    I_Jag_RegisterSong,
    I_Jag_UnRegisterSong,
    I_Jag_PlaySong,
    I_Jag_StopSong,
    I_Jag_IsPlaying,
    I_Jag_Poll
};
