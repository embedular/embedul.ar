/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  RetrOS™: real-time preemtive multitasking operating system.
  mutual exclusion access object.

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

#include "embedul.ar/source/core/retros/mutex.h"
#include "embedul.ar/source/core/device/board.h"
#include <string.h>


enum OS_Result OS_MUTEX_Init (struct OS_MUTEX *m)
{
    BOARD_AssertParams (m);

    ZERO_MEMORY (m);

    SEMAPHORE_Init (&m->sem, 1, 1);

    m->owner = OS_TaskSelf ();

    BOARD_AssertState (m->owner);

    return OS_Result_OK;
}


enum OS_Result OS_MUTEX_Lock (struct OS_MUTEX *m)
{
    BOARD_AssertParams (m);

    // Mutex already locked when asking to lock from the same task
    if (!SEMAPHORE_Available (&m->sem) && m->owner == OS_TaskSelf ())
    {
        return OS_Result_OK;
    }

    if (SEMAPHORE_Acquire (&m->sem))
    {
        m->owner = OS_TaskSelf ();
        return OS_Result_OK;
    }

    return OS_Result_Retry;
}


enum OS_Result OS_MUTEX_Unlock (struct OS_MUTEX *m)
{
    BOARD_AssertParams (m && m->owner);

    // Only the owner can unlock a locked mutex
    if (m->owner != OS_TaskSelf ())
    {
        return OS_Result_InvalidCaller;
    }

    // Already unlocked
    if (SEMAPHORE_Available (&m->sem))
    {
        return OS_Result_OK;
    }

    // Unlock
    if (SEMAPHORE_Release (&m->sem))
    {
        m->owner = NULL;
        return OS_Result_OK;
    }

    return OS_Result_Retry;
}
