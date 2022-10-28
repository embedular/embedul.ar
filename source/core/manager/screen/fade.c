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

#include "embedul.ar/source/core/manager/screen/fade.h"
#include "embedul.ar/source/core/device/board.h"


static const uint8_t s_RGB332BrightenTable[] =
{                       // RRRGGGBB
    0x00,               // 00000000
    0x24,               // 00100100
    0x49,               // 01001001
    0x92,               // 10010010
    0xB6,               // 10110110
    0xDB,               // 11011011
    0xFF,               // 11111111
};


static const uint8_t s_RGB332DarkenTable[] =
{                       // RRRGGGBB
    0xFF,               // 11111111
    0xDB,               // 11011011
    0xB6,               // 10110110
    0x6D,               // 01101101
    0x49,               // 01001001
    0x24,               // 00100100
    0x00,               // 00000000
};


static void initFx (struct SCREEN_FADE *const F,
                    const enum SCREEN_Role Role,
                    const enum SCREEN_FADE_Fx Fx,
                    const enum SCREEN_FADE_PassFilter PassFilter)
{
    OBJECT_Clear (F);

    switch (Fx)
    {
        case SCREEN_FADE_Fx_Darken:
            F->table    = s_RGB332DarkenTable;
            F->max      = sizeof(s_RGB332DarkenTable) - 1;
            F->sop      = VIDEO_SOP_ShowAnd;
            break;

        case SCREEN_FADE_Fx_Brighten:
            F->table    = s_RGB332BrightenTable;
            F->max      = sizeof(s_RGB332BrightenTable) - 1;
            F->sop      = VIDEO_SOP_ShowOr;
            break;
    }

    F->role         = Role;
    F->fx           = Fx;
    F->passFilter   = PassFilter;

    ANIM_SetValue (&F->anim, F->table[0]);
}


void SCREEN_FADE_In (struct SCREEN_FADE *const F,
                     const enum SCREEN_Role Role,
                     const enum SCREEN_FADE_Fx Fx,
                     const enum SCREEN_FADE_PassFilter PassFilter,
                     const uint32_t Duration)
{
    BOARD_AssertParams (F);

    initFx (F, Role, Fx, PassFilter);

    ANIM_Start (&F->anim, ANIM_Type_Blink, F->max, 0, 0, Duration, 0, 0);
}


void SCREEN_FADE_Out (struct SCREEN_FADE *const F,
                      const enum SCREEN_Role Role,
                      const enum SCREEN_FADE_Fx Fx,
                      const enum SCREEN_FADE_PassFilter PassFilter,
                      const uint32_t Duration)
{
    BOARD_AssertParams (F);

    initFx (F, Role, Fx, PassFilter);

    ANIM_Start (&F->anim, ANIM_Type_Blink, 0, F->max, 0, Duration, 0, 0);
}


void SCREEN_FADE_Flash (struct SCREEN_FADE *const F,
                        const enum SCREEN_Role Role,
                        const enum SCREEN_FADE_Fx Fx,
                        const enum SCREEN_FADE_PassFilter PassFilter,
                        const uint8_t Intensity,
                        const uint32_t Duration, const uint32_t Repeat)
{
    BOARD_AssertParams (F);

    initFx (F, Role, Fx, PassFilter);

    const uint32_t vScaled = ((Intensity + 1) * F->max) >> 8;

    if (vScaled < 1)
    {
        ANIM_SetValue (&F->anim, F->anim.vBegin);
    }
    else
    {
        ANIM_Start (&F->anim, ANIM_Type_PingPong, 0, vScaled,
                    0, Duration >> 1, Duration, Repeat);
    }
}


void SCREEN_FADE_Update (struct SCREEN_FADE *const F)
{
    BOARD_AssertParams (F);

    if (ANIM_Pending (&F->anim))
    {
        ANIM_Update (&F->anim);

        BOARD_AssertState (F->table && F->anim.vCurrent <= F->max);

        const struct SCREEN_Context *const C = SCREEN_GetContext (F->role);

        VIDEO_SetScanlineOp (C->driver, F->sop, 
                             F->table[F->anim.vCurrent] & F->passFilter);
    }
}


bool SCREEN_FADE_Pending (struct SCREEN_FADE *const F)
{
    BOARD_AssertParams (F);
    return ANIM_Pending (&F->anim);
}


void SCREEN_FADE_Cancel (struct SCREEN_FADE *const F)
{
    BOARD_AssertParams (F);

    if (SCREEN_FADE_Pending (F))
    {
        ANIM_SetValue (&F->anim, 0);
    }
}
