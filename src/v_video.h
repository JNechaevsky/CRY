//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2025 Julia Nechaevskaya
// Copyright(C) 2025 Polina "Aura" N.
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
//	Gamma correction LUT.
//	Functions to draw patches (by post) directly to screen.
//	Functions to blit a block to the screen.
//


#pragma once

#include "v_patch.h"
#include "w_wad.h" // [crispy] for lumpindex_t

extern byte *dp_translation;
extern boolean V_IsPatchLump(const int lump);
extern boolean dp_translucent;
extern pixel_t *palette_pointer;

void V_MarkRect(int x, int y, int width, int height);
void V_CopyRect(int srcx, int srcy, pixel_t *source, int width, int height, int destx, int desty);
void V_DrawPatch(int x, int y, patch_t *patch);
void V_DrawShadowedPatchOptional(int x, int y, patch_t *patch);
void V_DrawPatchFullScreen(patch_t *patch, boolean flipped);
extern void V_DrawPatchFlipped(int x, int y, patch_t *patch);
void V_DrawPatchFinale(int x, int y, patch_t *patch);
void V_DrawFadePatch(int x, int y, const patch_t *restrict patch, int alpha);
void V_DrawBlock(int x, int y, int width, int height, pixel_t *src);
void V_DrawScaledBlock(int x, int y, int width, int height, byte *src);
void V_DrawFilledBox(int x, int y, int w, int h, int c);
void V_DrawHorizLine(int x, int y, int w, int c);
void V_DrawVertLine(int x, int y, int h, int c);
void V_DrawBox(int x, int y, int w, int h, int c);
void V_FillFlat(int y_start, int y_stop, int x_start, int x_stop, const byte *src, pixel_t *dest); // [crispy]
void V_Init (void);
void V_UseBuffer(pixel_t *buffer);
void V_RestoreBuffer(void);
void V_ScreenShot(const char *format);
