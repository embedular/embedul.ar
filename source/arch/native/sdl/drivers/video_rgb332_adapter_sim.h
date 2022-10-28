/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [VIDEO driver] video adapter simulation using sdl: rgb332 256x144 framebuffer
                 integer-scaled to a 1280x720 video signal.

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

#include "embedul.ar/source/arch/native/sdl/drivers/video_rgb332.h"


struct VIDEO_RGB332_ADAPTER_SIM
{
    struct VIDEO_RGB332     device;
    uint8_t                 framebufferA[256 * 144];
    uint8_t                 framebufferB[256 * 144];
};


void VIDEO_RGB332_ADAPTER_SIM_Init (struct VIDEO_RGB332_ADAPTER_SIM *const S);
