/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  low-level interface between an arm cortex systick and embedul.ar

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

#include "embedul.ar/source/arch/arm-cortex/shared/systick.h"


static volatile TIMER_Ticks         s_Ticks = 0;
static volatile TIMER_TickHookFunc  s_TickHook[BOARD_TickHookFuncSlot__COUNT] = 
                                    { 
                                        (void *)0, (void *)0, (void *)0 
                                    };


// Cortex-M specific interrupt handler
void SysTick_Handler (void)
{
    ++ s_Ticks;

    for (uint32_t i = 0; i < BOARD_TickHookFuncSlot__COUNT; ++i)
    {
        if (s_TickHook[i]) 
        {
            s_TickHook[i] (s_Ticks);
        }       
    }
}


TIMER_TickHookFunc ARM_CM_SysTickSetHook (
                                    TIMER_TickHookFunc const Func,
                                    const enum BOARD_TickHookFuncSlot Slot)
{   
    if (Slot >= BOARD_TickHookFuncSlot__COUNT)
    {
        return (void *)0;
    }

    const TIMER_TickHookFunc Lth = s_TickHook[Slot];
    s_TickHook[Slot] = Func;

    return Lth;
}


TIMER_Ticks ARM_CM_SysTickTime (void)
{
    return s_Ticks;
}
