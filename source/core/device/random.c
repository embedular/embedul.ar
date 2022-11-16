/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [RANDOM] random number generator device driver interface (singleton).

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

#include "embedul.ar/source/core/device/random.h"
#include "embedul.ar/source/core/device/board.h"
             

static struct RANDOM * s_r = NULL;


void RANDOM_Init (struct RANDOM *const R,
                  const struct RANDOM_IFACE *const Iface)
{
    BOARD_AssertState  (!s_r);
    BOARD_AssertParams (R && Iface);

    // Required interface elements
    BOARD_AssertInterface (Iface->Description &&
                           Iface->GetUint32 &&
                           Iface->GetUint64);
    OBJECT_Clear (R);

    R->iface = Iface;

    s_r = R;

    {
        LOG_AutoContext (R, LANG_INIT);

        if (R->iface->HardwareInit)
        {
            R->iface->HardwareInit (R);
        }

        LOG_Items (1, LANG_SAMPLE_VALUE, R->iface->GetUint64(R),
                   LOG_ItemsBases(VARIANT_Base_Hex_UpperSuffix));
    }
}


uint32_t RANDOM_GetUint32 (void)
{
    BOARD_AssertState (s_r);
    return s_r->iface->GetUint32(s_r);
}


uint64_t RANDOM_GetUint64 (void)
{
    BOARD_AssertState (s_r);
    return s_r->iface->GetUint64(s_r);
}


// Range is [min, max]
uint32_t RANDOM_GetUint32InRange (const uint32_t Min, const uint32_t Max)
{
    BOARD_AssertState  (s_r);
    BOARD_AssertParams (Min < Max);

    return Min + (s_r->iface->GetUint32(s_r) % ((Max - Min) + 1));
}


// Range is [min, max]
uint64_t RANDOM_GetUint64InRange (const uint64_t Min, const uint64_t Max)
{
    BOARD_AssertState  (s_r);
    BOARD_AssertParams (Min < Max);

    return Min + (s_r->iface->GetUint64(s_r) % ((Max - Min) + 1));
}


const char * RANDOM_Description (void)
{
    BOARD_AssertState (s_r);
    return s_r->iface->Description;
}
