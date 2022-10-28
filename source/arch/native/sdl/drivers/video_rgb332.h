/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [VIDEO virtual driver] rgb332-based sdl framebuffer.

  Copyright 2018-2022 Santiago Germino
  <sgermino@embedul.ar> https://www.linkedin.com/in/royconejo

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#pragma once

#include "embedul.ar/source/core/device/video.h"
#include "embedul.ar/source/core/timer.h"
#include "SDL.h"


#define VIDEO_RGB332_IFACE(_desc,_w,_h) \
    .Description        = _desc, \
    .Width              = _w, \
    .Height             = _h, \
    .HardwareInit       = VIDEO_RGB332__hardwareInit, \
    .ReachedVBICount    = VIDEO_RGB332__reachedVBICount, \
    .WaitForVBI         = VIDEO_RGB332__waitForVBI, \
    .Shutdown           = VIDEO_RGB332__shutdown


struct VIDEO_RGB332_UpdateInfo
{
    const uint32_t  ShowAnd32;
    const uint32_t  ShowOr32;
    const uint32_t  ScanAnd32;
    const uint32_t  ScanOr32;
    uint8_t         *const Surface;
    const uint32_t  SurfacePitch;
};


typedef void    (* VIDEO_RGB332_UpdateSurfaceFunc)(struct VIDEO *const V,
                        const struct VIDEO_RGB332_UpdateInfo *const Ui);


struct VIDEO_RGB332
{
    struct VIDEO    device;
    SDL_Window      * displayWindow;
    SDL_Surface     * displaySurface;
    SDL_Surface     * displayBuffer;
    uint32_t        vbiStartedTicks;
    uint32_t        windowId;
    VIDEO_RGB332_UpdateSurfaceFunc
                    updateSurface;
    uint16_t        displayWidth;
    uint16_t        displayHeight;
};


void VIDEO_RGB332__hardwareInit     (struct VIDEO *const V);
bool VIDEO_RGB332__reachedVBICount  (struct VIDEO *const V);
void VIDEO_RGB332__waitForVBI       (struct VIDEO *const V);
void VIDEO_RGB332__shutdown         (struct VIDEO *const V);
