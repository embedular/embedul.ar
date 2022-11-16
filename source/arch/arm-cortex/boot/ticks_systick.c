/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [TICKS driver] arm cortex systick.

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

#include "embedul.ar/source/core/device/ticks.h"
#include "embedul.ar/source/arch/arm-cortex/cmsis.h"


struct TICKS_SYSTICK
{
    struct TICKS    device;
};


static struct TICKS_SYSTICK s_ticks_systick;


// Note that this driver supports a single instance of itself
static volatile TIMER_Ticks         s_ticks = 0;
static volatile TIMER_TickHookFunc  s_hook  = NULL;


// Cortex-M specific interrupt handler
void SysTick_Handler (void)
{
    ++ s_ticks;

    if (s_hook)
    {
        s_hook (s_ticks);
    }
}


static void                 hardwareInit    (struct TICKS *const T);
static TIMER_TickHookFunc   setHook         (struct TICKS *const T,
                                             TIMER_TickHookFunc const Hook);
static TIMER_Ticks          now             (struct TICKS *const T);
static void                 delay           (struct TICKS *const T,
                                             const TIMER_Ticks Ticks);


static const struct TICKS_IFACE TICKS_SYSTICK_IFACE =
{
    .Description    = "arm cortex systick",
    .HardwareInit   = hardwareInit,
    .SetHook        = setHook,
    .Now            = now,
    .Delay          = delay
};


void TICKS__boot (void)
{
    TICKS_Init ((struct TICKS *)&s_ticks_systick, &TICKS_SYSTICK_IFACE);
}


static void hardwareInit (struct TICKS *const T)
{
    (void) T;

    const uint32_t Frequency_hz = 1000;

    SysTick_Config (SystemCoreClock / Frequency_hz);
}


static TIMER_TickHookFunc setHook (struct TICKS *const T,
                                   TIMER_TickHookFunc const Hook)
{
    (void) T;

    const TIMER_TickHookFunc LastHook = s_hook;
    s_hook = Hook;

    return LastHook;
}


static TIMER_Ticks now (struct TICKS *const T)
{
    (void) T;

    return s_ticks;
}


static void delay (struct TICKS *const T, const TIMER_Ticks Ticks)
{
    (void) T;

    const TIMER_Ticks Timeout = s_ticks + Ticks;

    while (Timeout > s_ticks)
    {
        __WFI ();
    }
}
