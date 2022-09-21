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

#include "embedul.ar/source/core/device/video/fade.h"
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


static void initFx (struct VIDEO_FADE *const F, const enum VIDEO_FADE_Fx Fx,
                    const uint8_t Mask)
{
    OBJECT_Clear (F);

    switch (Fx)
    {
        case VIDEO_FADE_Fx_Darken:
            F->table    = s_RGB332DarkenTable;
            F->max      = sizeof(s_RGB332DarkenTable) - 1;
            F->sop      = VIDEO_SOP_ShowAnd;
            break;

        case VIDEO_FADE_Fx_Brighten:
            F->table    = s_RGB332BrightenTable;
            F->max      = sizeof(s_RGB332BrightenTable) - 1;
            F->sop      = VIDEO_SOP_ShowOr;
            break;
    }

    F->fx   = Fx;
    F->mask = Mask;

    ANIM_SetValue (&F->anim, F->table[0]);
}


void VIDEO_FADE_In (struct VIDEO_FADE *const F, const enum VIDEO_FADE_Fx Fx,
                    const uint8_t Mask, const uint32_t Duration)
{
    BOARD_AssertParams (F);

    initFx (F, Fx, Mask);

    ANIM_Start (&F->anim, ANIM_Type_Blink, F->max, 0, 0, Duration, 0, 0);
}


void VIDEO_FADE_Out (struct VIDEO_FADE *const F, const enum VIDEO_FADE_Fx Fx,
                     const uint8_t Mask, const uint32_t Duration)
{
    BOARD_AssertParams (F);

    initFx (F, Fx, Mask);

    ANIM_Start (&F->anim, ANIM_Type_Blink, 0, F->max, 0, Duration, 0, 0);
}


void VIDEO_FADE_Flash (struct VIDEO_FADE *const F, const enum VIDEO_FADE_Fx Fx,
                       const uint8_t Mask, const uint8_t Intensity,
                       const uint32_t Duration, const uint32_t Repeat)
{
    BOARD_AssertParams (F);

    initFx (F, Fx, Mask);

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


void VIDEO_FADE_Update (struct VIDEO_FADE *const F)
{
    BOARD_AssertParams (F);

    if (ANIM_Pending (&F->anim))
    {
        ANIM_Update (&F->anim);
        BOARD_AssertState (F->table && F->anim.vCurrent <= F->max);
        VIDEO_SetScanlineOp (F->sop, F->table[F->anim.vCurrent] & F->mask);
    }
}


bool VIDEO_FADE_Pending (struct VIDEO_FADE *const F)
{
    BOARD_AssertParams (F);
    return ANIM_Pending (&F->anim);
}


void VIDEO_FADE_Cancel (struct VIDEO_FADE *const F)
{
    BOARD_AssertParams (F);

    if (VIDEO_FADE_Pending (F))
    {
        ANIM_SetValue (&F->anim, 0);
    }
}
