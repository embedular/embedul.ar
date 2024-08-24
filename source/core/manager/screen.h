/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [MANAGER] video devices manager.

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
#include "embedul.ar/source/core/manager/screen/font.h"
#include "embedul.ar/source/core/misc/rgb332.h"
#include <stdint.h>


#ifndef SCREEN_SPLASH_THEME_ROLE
    #define SCREEN_SPLASH_THEME_ROLE    SCREEN_Role_Primary
#endif


struct SCREEN_Clip
{
    uint16_t                x1;
    uint16_t                y1;
    uint16_t                x2;
    uint16_t                y2;
    uint16_t                width;
    uint16_t                height;
};


struct SCREEN_Context
{
    struct VIDEO            * driver;
    struct SCREEN_FONT      * font;
    struct RGB332_Gradient  gradient;
    struct SCREEN_Clip      clip;
};


struct SCREEN
{
    struct SCREEN_Context   context[SCREEN_Role__COUNT];
    struct SCREEN_FONT      defaultFont;
    uint32_t                registeredDevices;
};


enum SCREEN_ClippedFlags
{
    SCREEN_ClippedFlags_Left    = 0x01,
    SCREEN_ClippedFlags_Right   = 0x02,
    SCREEN_ClippedFlags_Top     = 0x04,
    SCREEN_ClippedFlags_Bottom  = 0x08
};


struct SCREEN_ClippedRegion
{
    int32_t                 cx;
    int32_t                 cy;
    int32_t                 xLeft;
    int32_t                 xRight;
    int32_t                 yTop;
    int32_t                 yBottom;
    uint16_t                cWidth;
    uint16_t                cHeight;
    enum SCREEN_ClippedFlags
                            clipped;
};


void        SCREEN_Init                 (struct SCREEN *const S);
const char *
            SCREEN_RoleName             (const enum SCREEN_Role Role);
void        SCREEN_RegisterDevice       (const enum SCREEN_Role Role,
                                         struct VIDEO *const Driver);
uint32_t    SCREEN_RegisteredDevices    (void);
bool        SCREEN_IsAvailable          (const enum SCREEN_Role Role);
const struct SCREEN_Context *
            SCREEN_GetContext           (const enum SCREEN_Role Role);
void        SCREEN_ResetContext         (const enum SCREEN_Role Role);
uint16_t    SCREEN_Width                (const enum SCREEN_Role Role);
uint16_t    SCREEN_Height               (const enum SCREEN_Role Role);
uint32_t    SCREEN_FrameNumber          (const enum SCREEN_Role Role);
void        SCREEN_ClearBack            (const enum SCREEN_Role Role,
                                         const uint8_t Color);
void        SCREEN_ClearFront           (const enum SCREEN_Role Role,
                                         const uint8_t Color);
void        SCREEN_Zap                  (const enum SCREEN_Role Role,
                                         const uint8_t Color);
void        SCREEN_BlitCachedElement    (const enum SCREEN_Role Role,
                                         const uint32_t CachedElement);
const struct SCREEN_FONT *
            SCREEN_GetFont              (const enum SCREEN_Role Role);
struct RGB332_Gradient *
            SCREEN_GetGradient          (const enum SCREEN_Role Role);

void        SCREEN_SetClippingRect      (const enum SCREEN_Role Role,
                                         const uint16_t X1, const uint16_t Y1,
                                         const uint16_t X2, const uint16_t Y2);
bool        SCREEN_Clip                 (const enum SCREEN_Role Role,
                                         const int32_t Cx, const int32_t Cy,
                                         const uint16_t Width,
                                         const uint16_t Height,
                                         struct SCREEN_ClippedRegion *const
                                         Creg);
void        SCREEN_Update               (void);
void        SCREEN_Shutdown             (void);
bool        SCREEN_Context__clip        (const struct SCREEN_Context * const C,
                                         const int32_t Cx, const int32_t Cy,
                                         const uint16_t Width,
                                         const uint16_t Height,
                                         struct SCREEN_ClippedRegion *const
                                         Creg);
bool        SCREEN_Context__isPointOut  (const struct SCREEN_Context * const C,
                                         const int32_t X, const int32_t Y);
bool        SCREEN_Context__isClipRectOut
                                        (const struct SCREEN_Context * const C,
                                         const int32_t Cx, const int32_t Cy,
                                         const uint16_t Width,
                                         const uint16_t Height);
int32_t     SCREEN_Context__toClipX     (const struct SCREEN_Context * const C,
                                         const int32_t X);
int32_t     SCREEN_Context__toClipY     (const struct SCREEN_Context * const C,
                                         const int32_t Y);
int32_t     SCREEN_Context__fromClipX   (const struct SCREEN_Context * const C,
                                         const int32_t Cx);
int32_t     SCREEN_Context__fromClipY   (const struct SCREEN_Context * const C,
                                         const int32_t Cy);
uint8_t *   SCREEN_Context__backbufferXY
                                        (const struct SCREEN_Context * const C,
                                         const int32_t X, const int32_t Y);
