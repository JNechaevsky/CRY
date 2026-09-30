//
// Copyright(C) 2017 Christoph Oelckers
// Copyright(C) 2021 Roman Fomin
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

#ifndef G_UMAPINFO_H
#define G_UMAPINFO_H

// Per-map overrides; fields keep "not set" markers when the lump omits
// them, so callers can fall back to the builtin defaults per field.
#define UMI_TEXT_LEN 512

typedef struct
{
    char sky1[9];    // skytexture lump name, empty if not specified
    char sky2[9];    // skytexture2 lump name, empty if not specified
    int  speed;      // skyscrollspeed, -1 if not specified
    char music[9];   // music lump name, empty if not specified
    int  next;       // 0-based destination map, -1 if not specified
    int  nextsecret; // 0-based secret destination, -1 if not specified
    char intertext[UMI_TEXT_LEN]; // multi-line finale text, empty if unset
    int  endcast;    // roll call after the text: 1/0, -1 if not specified
} umapinfo_map_t;

// 1-based map number (gamemap) -> overrides, or NULL when UMAPINFO is
// absent or has no entry for that map.
const umapinfo_map_t *UMAPINFO_GetMap (int mapnum);

void UMAPINFO_Parse (void);

#endif
