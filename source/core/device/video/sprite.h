/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [VIDEO subsystem] sprites.

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

#include "embedul.ar/source/core/device/video/tilemap.h"
#include <stdint.h>


struct VIDEO_SPRITE
{
    struct VIDEO_SPRITE         * next;
    const struct VIDEO_TILEMAP  * tilemap;
    // Screen-space coordinates
    int32_t                     viewX;
    int32_t                     viewY;
    uint16_t                    animation;
    uint16_t                    frame;
};


void VIDEO_SPRITE_Init              (struct VIDEO_SPRITE *const S,
                                     const struct VIDEO_TILEMAP *const Tilemap);
bool VIDEO_SPRITE_Collide           (const struct VIDEO_SPRITE *const S1,
                                     const struct VIDEO_SPRITE *const S2);
void VIDEO_SPRITE_SetAnimation      (struct VIDEO_SPRITE *const S,
                                     const uint16_t Value);
void VIDEO_SPRITE_SetAnimationFrame (struct VIDEO_SPRITE *const S,
                                     const uint32_t Value);
bool VIDEO_SPRITE_Draw              (struct VIDEO_SPRITE *const S);
