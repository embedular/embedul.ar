/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  RetrOS™: real-time preemtive multitasking operating system.
  armv7-m semaphore synchronization object.

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

#include "embedul.ar/source/core/retros/semaphore.h"
#include "embedul.ar/source/core/cc.h"
#include "embedul.ar/source/core/device/board.h"

// ARM Cortex only
#include "cmsis.h"


void SEMAPHORE_Init (struct SEMAPHORE *s, uint32_t resources,
                     uint32_t available)
{
    BOARD_AssertParams (s && resources && available <= resources);

    ZERO_MEMORY (s);

    s->resources = resources;
    s->available = available;
}


CC_NoOptimization
bool SEMAPHORE_Acquire (struct SEMAPHORE *s)
{
    BOARD_AssertParams (s);

    // http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dai0321a/BIHEJCHB.html
    __DMB ();

    uint32_t value = __LDREXW (&s->available);

    if (value == 0)
    {
        return false;
    }

    -- value;

    if (__STREXW (value, &s->available))
    {
        return false;
    }

    return true;
}


CC_NoOptimization
bool SEMAPHORE_Release (struct SEMAPHORE *s)
{
    BOARD_AssertParams (s);

    __DMB ();

    uint32_t value = __LDREXW (&s->available);

    if (value + 1 > s->resources)
    {
        return false;
    }

    ++ value;

    if (__STREXW (value, &s->available))
    {
        return false;
    }

    return true;
}


CC_NoOptimization
uint32_t SEMAPHORE_Available (struct SEMAPHORE *s)
{
    BOARD_AssertParams (s);

    return __LDREXW (&s->available);
}
