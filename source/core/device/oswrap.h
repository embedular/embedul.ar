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

#pragma once

#include "embedul.ar/source/core/timer.h"
#include <stdbool.h>


typedef void (* OSWRAP_TaskFunc)(void *const Param);


struct OSWRAP;
struct STREAM;
struct BOARD;


typedef void        (* OSWRAP_GreetingsFunc)(struct OSWRAP *const O,
                                         struct STREAM *const S);

// A.1) Create a task to excute RunTask().
// A.2) Start the OS scheduler to start RunTask().
//      RunTask() performs the framework initialization, calls the main
//      application entry point -MainTask()- through OSWRAP_BootMain(),
//      performs calls to BOARD_Sync() (if a multitasking OS is available)
//      and ultimately shutdowns the framework and returns to main(). The latter
//      will only happen if the OS can stop executing its scheduler after
//      RunTask() shutdowns the framework.
// A.3) After MainTask() terminates, the return value should be available to
//      RunTask() by calling BOARD_ReturnValue(). That value will be returned
//      by main(), if closing the scheduler is an option.

typedef void        (* OSWRAP_CreateRunTaskAndStartFunc)(
                                        struct OSWRAP *const O,
                                        const OSWRAP_TaskFunc RunTask,
                                        struct BOARD *const B);

// A.2.1) RunTask() creates a task to execute MainTask().
typedef void        (* OSWRAP_CreateMainTaskFunc)(
                                        struct OSWRAP *const O,
                                        const OSWRAP_TaskFunc MainTask);

// A.2.2) When using no operating system, a call to BOARD_Sync() will do just
//        that. When running a multitasking OS, RunTask() becomes the task in
//        charge of calling BOARD_Sync(), either at fixed time intervals
//        detached from MainTask(), or at MainTask() request. 
//        Specifically:
//  
//        When autoSyncPeriod == 0
//             A call to BOARD_Sync() from an application task will unlock a
//             call to BOARD_Sync() from RunTask(), that will actually execute
//             the sync. This method is used to sync after an application task
//             finished drawing the current video frame.
//        
//        When autoSyncPeriod > 0
//             RunTask execution will be resumed at regular intervals,
//             independent from the application. This method is useful when
//             the application is not involved in video generation but still
//             requires board input and outputs.
//
//        RunTask will keep executing the sync loop that calls RunTaskSyncLock()
//        until the main task calls BOARD_Exit().
typedef void        (* OSWRAP_RunTaskSyncLockFunc)(struct OSWRAP *const O);

// As discussed in A.2.2, this function, if defined and executed by any other
// task than RunTask, it then unlocks or resumes RunTask() to perform the actual
// BOARD_Sync() and returns false. if otherwise executed by RunTask(), it
// simply returns true telling BOARD_Sync() to execute.
// If this function is undefined, it means the application runs without an OS,
// then BOARD_Sync() is executed right away; OSWRAP__syncNow() will always
// return true instead of the results of calling this interface function.
typedef bool        (* OSWRAP_SyncUnlockFunc)(struct OSWRAP *const O);

// A.2.3) MainTask() calls BOARD_Exit(ReturnValue) that in turn calls
//        OSWRAP__closeMainTask() to stop its own execution.
//        MainTask() must destroy all tasks it created before exiting.
typedef void        (* OSWRAP_CloseMainTaskFunc)(struct OSWRAP *const O);

// Closes RunTask() and optionally, the OS scheduler started in A.2.
// The scheduler must be closed for the execution to continue to the end of
// main().
typedef void        (* OSWRAP_CloseRunTaskAndEndFunc)(struct OSWRAP *const O);

typedef void        (* OSWRAP_SuspendSchedulerFunc)(struct OSWRAP *const O);
typedef void        (* OSWRAP_ResumeSchedulerFunc)(struct OSWRAP *const O);
typedef TIMER_TickHookFunc
                    (* OSWRAP_SetTickHookFunc)(struct OSWRAP *const O,
                                        const TIMER_TickHookFunc Hook);
typedef TIMER_Ticks (* OSWRAP_TicksNowFunc)(struct OSWRAP *const O);
typedef void        (* OSWRAP_DelayFunc)(struct OSWRAP *const O,
                                        const TIMER_Ticks Ticks);


struct OSWRAP_IFACE
{
    const char                          *const Description;
    const OSWRAP_GreetingsFunc          Greetings;
    const OSWRAP_CreateRunTaskAndStartFunc
                                        CreateRunTaskAndStart;
    const OSWRAP_CreateMainTaskFunc     CreateMainTask;
    const OSWRAP_RunTaskSyncLockFunc    RunTaskSyncLock;
    const OSWRAP_SyncUnlockFunc         SyncUnlock;
    const OSWRAP_CloseMainTaskFunc      CloseMainTask;
    const OSWRAP_CloseRunTaskAndEndFunc CloseRunTaskAndEnd;
    const OSWRAP_SuspendSchedulerFunc   SuspendScheduler;
    const OSWRAP_ResumeSchedulerFunc    ResumeScheduler;
    const OSWRAP_SetTickHookFunc        SetTickHook;
    const OSWRAP_TicksNowFunc           TicksNow;
    const OSWRAP_DelayFunc              Delay;
};


struct OSWRAP
{
    const struct OSWRAP_IFACE   * iface;
    uint32_t                    autoSyncPeriod_ms;
};


void            OSWRAP_Init             (struct OSWRAP *const O, const struct
                                         OSWRAP_IFACE *const Iface);
bool            OSWRAP_IsMultitasking   (void);
void            OSWRAP_EnableManualSync (void);
void            OSWRAP_EnableAutoSync   (const uint32_t Period_ms);
// Stops the multistasking OS scheduler, effectively avoiding task switches in
// the middle of a critical section.
void            OSWRAP_SuspendScheduler (void);
// Resumens the multistasking OS scheduler.
void            OSWRAP_ResumeScheduler  (void);
const char *    OSWRAP_Description      (void);
