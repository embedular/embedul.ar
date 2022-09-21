/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [RANDOM driver] fast mersenne twister.

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

#include "embedul.ar/source/arch/arm-cortex/stm32/drivers/random_rng.h"
#include "embedul.ar/source/core/device/board.h"


static uint32_t     getUint32       (struct RANDOM *const R);
static uint64_t     getUint64       (struct RANDOM *const R);


static const struct RANDOM_IFACE RANDOM_RNG_IFACE =
{
    .Description    = "stm32 rng",
    .HardwareInit   = UNSUPPORTED,
    .GetUint32      = getUint32,
    .GetUint64      = getUint64
};


void RANDOM_RNG_Init (struct RANDOM_RNG *const N,
                      RNG_HandleTypeDef *const Hrng)
{
    BOARD_AssertParams (N && Hrng);

    DEVICE_IMPLEMENTATION_Clear (N);

    N->hrng = Hrng;

    RANDOM_Init ((struct RANDOM *)N, &RANDOM_RNG_IFACE);
}


static uint32_t get32 (struct RANDOM_RNG *const N)
{
    uint32_t value;

    const int Ret = HAL_RNG_GenerateRandomNumber (N->hrng, &value);

    if (Ret == HAL_OK)
    {
        return value;
    }

    LOG_Warn (&N->device, LANG_DEVICE_MALFUNCTION);
    LOG_Items (1, LANG_ERROR, (int32_t)Ret);

    return 0;
}


static uint32_t getUint32 (struct RANDOM *const R)
{
    struct RANDOM_RNG *const N = (struct RANDOM_RNG *) R;

    return get32 (N);
}


static uint64_t getUint64 (struct RANDOM *const R)
{
    struct RANDOM_RNG *const N = (struct RANDOM_RNG *) R;

    const uint64_t Value = (uint64_t)get32(N) << 32 | get32(N);

    return Value;
}
