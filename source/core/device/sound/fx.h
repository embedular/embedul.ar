/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [SOUND subsystem] sound effects.

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

#include <stdint.h>


#define SOUND_FX_FORMAT(rate,bits,ch)       ((((ch) & 0xFF) << 24) | \
                                            (((bits) & 0xFF) << 16) | \
                                            ((rate) & 0xFFFF))
#define SOUND_FX_FORMAT_TO_RATE(fmt)        ((fmt) & 0xFFFF)
#define SOUND_FX_FORMAT_TO_CHANNELS(fmt)    (((fmt) >> 24) & 0xFF)
#define SOUND_FX_FORMAT_TO_BITS(fmt)        (((fmt) >> 16) & 0xFF)


#define SOUND_FX_FORMAT_MIXER               SOUND_FX_FORMAT( \
                                                SOUND_MIXER_SAMPLE_RATE, \
                                                SOUND_MIXER_BITS, \
                                                SOUND_MIXER_CHANNELS)

#define SOUND_FX_FORMAT_16BIT_STEREO        SOUND_FX_FORMAT( \
                                                SOUND_MIXER_SAMPLE_RATE, \
                                                16, \
                                                2)

#define SOUND_FX_FORMAT_16BIT_MONO          SOUND_FX_FORMAT( \
                                                SOUND_MIXER_SAMPLE_RATE, \
                                                16, \
                                                1)

#define SOUND_FX_FORMAT_16BIT_STEREO_HSR    SOUND_FX_FORMAT( \
                                                SOUND_MIXER_SAMPLE_RATE >> 1, \
                                                16, \
                                                2)

#define SOUND_FX_FORMAT_16BIT_MONO_HSR      SOUND_FX_FORMAT( \
                                                SOUND_MIXER_SAMPLE_RATE >> 1, \
                                                16, \
                                                1)

#define SOUND_FX_FORMAT_16BIT_STEREO_QSR    SOUND_FX_FORMAT( \
                                                SOUND_MIXER_SAMPLE_RATE >> 2, \
                                                16, \
                                                2)

#define SOUND_FX_FORMAT_16BIT_MONO_QSR      SOUND_FX_FORMAT( \
                                                SOUND_MIXER_SAMPLE_RATE >> 2, \
                                                16, \
                                                1)

#define SOUND_FX_FORMAT_8BIT_MONO           SOUND_FX_FORMAT( \
                                                SOUND_MIXER_SAMPLE_RATE, \
                                                8, \
                                                1)

#define SOUND_FX_FORMAT_8BIT_STEREO_HSR     SOUND_FX_FORMAT( \
                                                SOUND_MIXER_SAMPLE_RATE >> 1, \
                                                8, \
                                                2)

#define SOUND_FX_FORMAT_8BIT_MONO_HSR       SOUND_FX_FORMAT( \
                                                SOUND_MIXER_SAMPLE_RATE >> 1, \
                                                8, \
                                                1)

#define SOUND_FX_FORMAT_8BIT_STEREO_QSR     SOUND_FX_FORMAT( \
                                                SOUND_MIXER_SAMPLE_RATE >> 2, \
                                                8, \
                                                2)

#define SOUND_FX_FORMAT_8BIT_MONO_QSR       SOUND_FX_FORMAT( \
                                                SOUND_MIXER_SAMPLE_RATE >> 2, \
                                                8, \
                                                1)


#define SOUND_FX_SlotIndexNone              ((uint16_t) -1)
#define SOUND_FX_SlotIndexAny               ((uint16_t) -2)
#define SOUND_FX_SlotIndexNoOverlap         ((uint16_t) -3)


typedef uint16_t SOUND_FX_SlotIndex;


struct SOUND_FX
{
    union
    {
        const void      * data;
        const int8_t    * ds8;
        const uint8_t   * du8;
        const int16_t   * ds16;
        const uint16_t  * du16;
    };
    uint32_t            size;
    uint32_t            format;
};


enum SOUND_FX_SlotStatus
{
    SOUND_FX_SlotStatus_Idle = 0,
    SOUND_FX_SlotStatus_Playing,
};


#define SFX_SlotFlag_Reserved           0x0001

typedef uint16_t    SOUND_FX_SlotFlag;


struct SOUND_FX_Slot
{
    const struct SOUND_FX       * sfx;
    uint32_t                    offset;
    enum SOUND_FX_SlotStatus    status;
    SOUND_FX_SlotFlag           flags;
    uint8_t                     volLeft;
    uint8_t                     volRight;
};
