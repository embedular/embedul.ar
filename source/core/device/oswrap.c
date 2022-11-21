/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [OSWRAP] OS wrapper device driver interface (singleton).

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

#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/core/device/oswrap.h"


static struct OSWRAP * s_o = NULL;


static bool validOSInterface (const struct OSWRAP_IFACE *const Iface)
{
    // Either all present or all absent
    return ((Iface->CreateRunTaskAndStart &&
             Iface->CreateMainTask &&
             Iface->RunTaskSyncLock &&
             Iface->SyncUnlock &&
             Iface->CloseMainTask &&
             Iface->CloseRunTaskAndEnd &&
             Iface->SuspendScheduler &&
             Iface->ResumeScheduler) ||
            (!Iface->CreateRunTaskAndStart &&
             !Iface->CreateMainTask &&
             !Iface->RunTaskSyncLock &&
             !Iface->SyncUnlock &&
             !Iface->CloseMainTask &&
             !Iface->CloseRunTaskAndEnd &&
             !Iface->SuspendScheduler &&
             !Iface->ResumeScheduler))?
             true : false;
}


void OSWRAP_Init (struct OSWRAP *const O,
                  const struct OSWRAP_IFACE *const Iface)
{
    BOARD_AssertState   (!s_o);
    BOARD_AssertParams  (O && Iface);

    BOARD_AssertInterface (Iface->Description && validOSInterface(Iface));

    OBJECT_Clear (O);

    O->iface = Iface;

    // Note that autosync is disabled (manual sync mode). That is required
    // for the init splash screens to work correctly. The actual autosync
    // period (LIB_EMBEDULAR_CONFIG_OS_AUTOSYNC_PERIOD) will be enabled after
    // the init spash screens.
    O->autoSyncPeriod_ms = 0;

    s_o = O;
}


bool OSWRAP_IsMultitasking (void)
{
    return (s_o->iface->CreateRunTaskAndStart)?
            true : false;
}


void OSWRAP_EnableManualSync (void)
{
    OSWRAP_EnableAutoSync (0);
}


void OSWRAP_EnableAutoSync (const uint32_t Period_ms)
{
    s_o->autoSyncPeriod_ms = Period_ms;
}


void OSWRAP_SuspendScheduler (void)
{
    if (s_o->iface->SuspendScheduler)
    {
        s_o->iface->SuspendScheduler (s_o);
    }
}


void OSWRAP_ResumeScheduler (void)
{
    if (s_o->iface->ResumeScheduler)
    {
        s_o->iface->ResumeScheduler (s_o);
    }
}


const char * OSWRAP_Description (void)
{
    return s_o->iface->Description;
}


void OSWRAP__summary (void)
{
    if (s_o->iface->Summary)
    {
        s_o->iface->Summary (s_o);
    }
}


int OSWRAP__createRunTaskAndStart (struct BOARD *const B,
                                   const OSWRAP_TaskFunc RunTask)
{
    if (!s_o->iface->CreateRunTaskAndStart)
    {
        RunTask (B);
    }
    else
    {
        s_o->iface->CreateRunTaskAndStart (s_o, RunTask, B);
    }

    return BOARD_ReturnValue ();
}


void OSWRAP__createMainTaskAndSync (struct BOARD *const B,
                                    const OSWRAP_TaskFunc MainTask)
{
    if (!s_o->iface->CreateMainTask)
    {
        // No OS: Simply call MainTask from the only thread available.
        MainTask (NULL);
    }
    else
    {
        // Spawn the application main task
        s_o->iface->CreateMainTask (s_o, MainTask);

        while (!B->exit)
        {
            // Sync using one of two methods depending on
            // timerSyncPeriod_ms.
            s_o->iface->RunTaskSyncLock (s_o);

            BOARD_Sync ();
        }
    }
}


bool OSWRAP__syncNow (void)
{
    if (!s_o->iface->SyncUnlock)
    {
        return true;
    }

    return s_o->iface->SyncUnlock (s_o);
}


void OSWRAP__closeMainTask (void)
{
    if (s_o->iface->CloseMainTask)
    {
        s_o->iface->CloseMainTask (s_o);
    }
}


void OSWRAP__closeRunTaskAndEnd (void)
{
    if (s_o->iface->CloseRunTaskAndEnd)
    {
        s_o->iface->CloseRunTaskAndEnd (s_o);
    }
}


TIMER_TickHookFunc OSWRAP__setTickHook (const TIMER_TickHookFunc Hook)
{
    BOARD_AssertState (s_o->iface->SetTickHook);

    return s_o->iface->SetTickHook (s_o, Hook);
}


// Used by TICKS_OSWRAP
// Note that TICKS_OSWRAP should only be used when
// OSWRAP_IsMultitasking() == true
TIMER_Ticks OSWRAP__ticksNow (void)
{
    BOARD_AssertState (s_o->iface->TicksNow);

    return s_o->iface->TicksNow (s_o);
}


void OSWRAP__delay (const TIMER_Ticks Ticks)
{
    BOARD_AssertState (s_o->iface->Delay);

    return s_o->iface->Delay (s_o, Ticks);
}
