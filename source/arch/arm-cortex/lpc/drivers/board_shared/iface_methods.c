/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  LPC43xx common board iface methods.

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

#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/board_shared/iface_methods.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/chip_18xx_43xx/chip.h"
#include "embedul.ar/source/arch/arm-cortex/shared/systick.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/shared/panic.h"


void assertFunc (struct BOARD *const B, const bool Condition)
{
    (void) B;

    if (!Condition)
    {
        Board_Panic (NULL);
    }
}


void setTickFreq (struct BOARD *const B, const uint32_t Hz)
{
    (void) B;
    // Cortex-M CMSIS function
    SysTick_Config (SystemCoreClock / Hz);
}


TIMER_TickHookFunc setTickHook (struct BOARD *const B,
                                TIMER_TickHookFunc const Func,
                                const enum BOARD_TickHookFuncSlot Slot)
{
    (void) B;
    return ARM_CM_SysTickSetHook (Func, Slot);
}


TIMER_Ticks ticksNow (struct BOARD *const B)
{
    (void) B;
    return ARM_CM_SysTickTime ();
}


struct BOARD_RealTimeClock rtc (struct BOARD *const B)
{
    (void) B;

    RTC_TIME_T rtcTime;
	Chip_RTC_GetFullTime (LPC_RTC, &rtcTime);

    return (struct BOARD_RealTimeClock) {
        .dayOfYear  = rtcTime.time[RTC_TIMETYPE_DAYOFYEAR],
        .year       = rtcTime.time[RTC_TIMETYPE_YEAR],
        .dayOfMonth = rtcTime.time[RTC_TIMETYPE_DAYOFMONTH],
        .month      = rtcTime.time[RTC_TIMETYPE_MONTH],
        .dayOfWeek  = rtcTime.time[RTC_TIMETYPE_DAYOFWEEK],
        .hour       = rtcTime.time[RTC_TIMETYPE_HOUR],
        .minute     = rtcTime.time[RTC_TIMETYPE_MINUTE],
        .second     = rtcTime.time[RTC_TIMETYPE_SECOND]
    };
}


void delay (struct BOARD *const B, const TIMER_Ticks Ticks)
{
    (void) B;

    const TIMER_Ticks Timeout = BOARD_TicksNow() + Ticks;

    while (Timeout > BOARD_TicksNow())
    {
        __WFI ();
    }
}
