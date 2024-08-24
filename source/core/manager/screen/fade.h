/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [SCREEN MANAGER] rgb332 screen fade effects.

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
#include "embedul.ar/source/core/manager/screen/role.h"
#include "embedul.ar/source/core/anim.h"


enum SCREEN_FADE_PassFilter
{
    SCREEN_FADE_PassFilter_None     = 0x00,
    SCREEN_FADE_PassFilter_Red      = 0xE0,
    SCREEN_FADE_PassFilter_Green    = 0x1C,
    SCREEN_FADE_PassFilter_Blue     = 0x03,
    SCREEN_FADE_PassFilter_Yellow   = (SCREEN_FADE_PassFilter_Red | \
                                       SCREEN_FADE_PassFilter_Green),
    SCREEN_FADE_PassFilter_All      = 0xFF
};


enum SCREEN_FADE_Fx
{
    SCREEN_FADE_Fx_Darken,
    SCREEN_FADE_Fx_Brighten
};


struct SCREEN_FADE
{
    struct ANIM                 anim;
    const uint8_t               * table;
    uint32_t                    max;
    enum VIDEO_SOP              sop;
    enum SCREEN_Role            role;
    enum SCREEN_FADE_Fx         fx;
    enum SCREEN_FADE_PassFilter passFilter;
};


void    SCREEN_FADE_In          (struct SCREEN_FADE *const F,
                                 const enum SCREEN_Role Role,
                                 const enum SCREEN_FADE_Fx Fx,
                                 const enum SCREEN_FADE_PassFilter PassFilter,
                                 const uint32_t Duration);
void    SCREEN_FADE_Out         (struct SCREEN_FADE *const F,
                                 const enum SCREEN_Role Role,
                                 const enum SCREEN_FADE_Fx Fx,
                                 const enum SCREEN_FADE_PassFilter PassFilter,
                                 const uint32_t Duration);
void    SCREEN_FADE_Flash       (struct SCREEN_FADE *const F,
                                 const enum SCREEN_Role Role,
                                 const enum SCREEN_FADE_Fx Fx,
                                 const enum SCREEN_FADE_PassFilter PassFilter,
                                 const uint8_t Intensity,
                                 const uint32_t Duration,
                                 const uint32_t Repeat);
void    SCREEN_FADE_Update      (struct SCREEN_FADE *const F);
bool    SCREEN_FADE_Pending     (struct SCREEN_FADE *const F);
void    SCREEN_FADE_Cancel      (struct SCREEN_FADE *const F);
