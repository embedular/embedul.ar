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

#include "embedul.ar/source/drivers/random_sfmt.h"
#include "embedul.ar/source/core/device/board.h"


static void         hardwareInit    (struct RANDOM *const R);
static uint32_t     getUint32       (struct RANDOM *const R);
static uint64_t     getUint64       (struct RANDOM *const R);


static const struct RANDOM_IFACE RANDOM_SFMT_IFACE =
{
    .Description    = "fast mersenne twister",
    .HardwareInit   = hardwareInit,
    .GetUint32      = getUint32,
    .GetUint64      = getUint64
};


void RANDOM_SFMT_Init (struct RANDOM_SFMT *const S, const uint64_t Seed)
{
    BOARD_AssertParams (S);

    DEVICE_IMPLEMENTATION_Clear (S);

    S->seed = Seed;

    RANDOM_Init ((struct RANDOM *)S, &RANDOM_SFMT_IFACE);
}


static void hardwareInit (struct RANDOM *const R)
{
    struct RANDOM_SFMT *const S = (struct RANDOM_SFMT *) R;

    sfmt_init_by_array (&S->sfmt, (uint32_t *)&S->seed, 2);

    LOG_Items (1, LANG_PERIOD, (uint32_t)SFMT_MEXP);
}


static uint32_t getUint32 (struct RANDOM *const R)
{
    struct RANDOM_SFMT *const S = (struct RANDOM_SFMT *) R;

    return sfmt_genrand_uint32 (&S->sfmt);
}


static uint64_t getUint64 (struct RANDOM *const R)
{
    struct RANDOM_SFMT *const S = (struct RANDOM_SFMT *) R;

    return sfmt_genrand_uint64 (&S->sfmt);
}
