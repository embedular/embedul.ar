/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [TICKS] device interface (singleton).

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

#include "embedul.ar/source/core/timer.h"


struct TICKS;


typedef void                (* TICKS_HardwareInitFunc)(struct TICKS *const T);
typedef TIMER_TickHookFunc  (* TICKS_SetHookFunc)(struct TICKS *const T,
                                            TIMER_TickHookFunc const Hook);
typedef TIMER_Ticks         (* TICKS_NowFunc)(struct TICKS *const T);
typedef void                (* TICKS_DelayFunc)(struct TICKS *const T,
                                            const TIMER_Ticks Ticks);


struct TICKS_IFACE
{
    const char                      *const Description;
    const TICKS_HardwareInitFunc    HardwareInit;
    const TICKS_SetHookFunc         SetHook;
    const TICKS_NowFunc             Now;
    const TICKS_DelayFunc           Delay;
};


struct TICKS
{
    const struct TICKS_IFACE        * iface;
};


void                TICKS_Init          (struct TICKS *const T,
                                         const struct TICKS_IFACE *const Iface);
TIMER_Ticks         TICKS_Now           (void);
TIMER_TickHookFunc  TICKS_SetHook       (const TIMER_TickHookFunc Hook);
void                TICKS_Delay         (const TIMER_Ticks Ticks);
const char *        TICKS_Description   (void);
