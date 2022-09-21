/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [VIDEO subsystem] rgb332 screen fade effects.

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

#include "embedul.ar/source/core/anim.h"
#include "embedul.ar/source/core/device/video.h"


#define VIDEO_FADE_MASK_PASS_ALL            0xFF
#define VIDEO_FADE_MASK_PASS_RED            0xE0
#define VIDEO_FADE_MASK_PASS_GREEN          0x1C
#define VIDEO_FADE_MASK_PASS_BLUE           0x03
#define VIDEO_FADE_MASK_PASS_YELLOW         (VIDEO_FADE_MASK_PASS_RED | \
                                            VIDEO_FADE_MASK_PASS_GREEN)


enum VIDEO_FADE_Fx
{
    VIDEO_FADE_Fx_Darken,
    VIDEO_FADE_Fx_Brighten
};


struct VIDEO_FADE
{
    struct ANIM             anim;
    const uint8_t           * table;
    uint32_t                max;
    enum VIDEO_SOP          sop;
    enum VIDEO_FADE_Fx      fx;
    uint8_t                 mask;
};


void    VIDEO_FADE_In       (struct VIDEO_FADE *const F,
                             const enum VIDEO_FADE_Fx Fx,
                             const uint8_t Mask, const uint32_t Duration);
void    VIDEO_FADE_Out      (struct VIDEO_FADE *const F,
                             const enum VIDEO_FADE_Fx Fx,
                             const uint8_t Mask, const uint32_t Duration);
void    VIDEO_FADE_Flash    (struct VIDEO_FADE *const F,
                             const enum VIDEO_FADE_Fx Fx,
                             const uint8_t Mask, const uint8_t Intensity,
                             const uint32_t Duration, const uint32_t Repeat);
void    VIDEO_FADE_Update   (struct VIDEO_FADE *const F);
bool    VIDEO_FADE_Pending  (struct VIDEO_FADE *const F);
void    VIDEO_FADE_Cancel   (struct VIDEO_FADE *const F);
