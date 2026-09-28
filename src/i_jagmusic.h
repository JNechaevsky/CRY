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

#pragma once


#include "i_sound.h"

// The music module wrapper (registered lazily; see i_sound.c routing).
extern const music_module_t music_jag_module;

// Loads the instrument bank from the WAD. Safe to call repeatedly; the
// actual load happens once. Returns true if a usable bank was found.
boolean I_JagMusic_Init(void);

// True once a valid instrument bank has been loaded.
boolean I_JagMusic_Present(void);

// Heuristic: does this data look like a valid Jaguar native music stream?
boolean I_JagMusic_IsNative(const void *data, int len);
