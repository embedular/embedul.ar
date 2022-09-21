/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [RANDOM] random number generator device driver interface.

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


struct RANDOM;


typedef void        (* RANDOM_HardwareInitFunc)(struct RANDOM *const R);
typedef uint32_t    (* RANDOM_GetUint32Func)(struct RANDOM *const R);
typedef uint64_t    (* RANDOM_GetUint64Func)(struct RANDOM *const R);


struct RANDOM_IFACE
{
    const char                      *const Description;
    const RANDOM_HardwareInitFunc   HardwareInit;
    const RANDOM_GetUint32Func      GetUint32;
    const RANDOM_GetUint64Func      GetUint64;
};


struct RANDOM
{
    const struct RANDOM_IFACE       * iface;
};


void            RANDOM_Init                 (struct RANDOM *const R, const
                                             struct RANDOM_IFACE *const Iface);
uint32_t        RANDOM_GetUint32            (void);
uint64_t        RANDOM_GetUint64            (void);
uint32_t        RANDOM_GetUint32InRange     (const uint32_t Min,
                                             const uint32_t Max);
uint64_t        RANDOM_GetUint64InRange     (const uint64_t Min,
                                             const uint64_t Max);
const char *    RANDOM_Description          (void);
