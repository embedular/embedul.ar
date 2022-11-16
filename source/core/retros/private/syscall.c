/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  RetrOS™: real-time preemtive multitasking operating system.
  system calls.

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

#include "embedul.ar/source/core/retros/private/syscall.h"
#include "embedul.ar/source/core/retros/private/runtime.h"
#include "embedul.ar/source/core/retros/private/scheduler.h"
//#include "embedul.ar/source/core/retros/private/driver/storage.h"
#include "embedul.ar/source/core/retros/mutex.h"
#include "embedul.ar/source/core/queue.h"
#include "embedul.ar/source/core/device/board.h"

// ARM Cortex only
#include "cmsis.h"

#include <string.h>
#include <stdio.h>


static OS_TaskRetVal taskIdle (OS_TaskParam arg)
{
    (void) arg;

    while (1)
    {
        __WFI ();
    }

    return 0;
}


static void taskBootReturn (OS_TaskRetVal retValue)
{
    // A retValue != 0 means an unrecoverable boot error, the OS will be closed
    // (!startedForever) or otherwise get trapped in an infinite loop.
    if (retValue)
    {
        OS_Terminate ();
        while (1)
        {
            __WFI ();
        }
    }

    OS_Syscall (OS_Syscall_TaskBootEnded, NULL);
}


static void taskCommonReturn (OS_TaskRetVal retValue)
{
    struct OS_TaskTerminate tt;
    tt.retVal   = retValue;
    tt.tc       = OS_TaskSelf ();

    OS_Syscall (OS_Syscall_TaskTerminate, &tt);
}


static void taskInitStack (struct OS_TaskControl *tc, struct OS_TaskStart *ts)
{
    uint32_t        *stackTop   = &((uint32_t *)tc) [(tc->stackTop >> 2)];
    OS_TaskReturn   taskReturn  = (tc->priority == OS_TaskPriority_Boot)
                                                ? taskBootReturn
                                                : taskCommonReturn;

    // Registers automatically stacked when entering the handler.
    // Values in this stack substitutes those.
    *(--stackTop) = 1 << 24;                    // xPSR.T = 1
    *(--stackTop) = (uint32_t) ts->func;        // xPC
    *(--stackTop) = (uint32_t) taskReturn;      // xLR
    *(--stackTop) = 0;                          // R12
    *(--stackTop) = 0;                          // R3
    *(--stackTop) = 0;                          // R2
    *(--stackTop) = 0;                          // R1
    *(--stackTop) = (uint32_t) ts->param;       // R0
    // LR pushed at interrupt handler. Here artificially set to return to
    // threaded PSP with unused FPU registers (no lazy stacking)
    *(--stackTop) = 0xFFFFFFFD;                 // LR IRQ
    // R4-R11 pushed at interrupt handler.
    *(--stackTop) = 0;                          // R11
    *(--stackTop) = 0;                          // R10
    *(--stackTop) = 0;                          // R9
    *(--stackTop) = 0;                          // R8
    *(--stackTop) = 0;                          // R7
    *(--stackTop) = 0;                          // R6
    *(--stackTop) = 0;                          // R5
    *(--stackTop) = 0;                          // R4

    tc->sp = (uint32_t) stackTop;
}


static enum OS_Result taskStart (struct OS_TaskStart *ts)
{
    if (!ts || !ts->func || !ts->description)
    {
        return OS_Result_InvalidParams;
    }

    // taskBuffer pointer must be aligned to a 8 byte boundary
    if ((uint32_t)ts->buffer & 0x07)
    {
        return OS_Result_InvalidBufferAlignment;
    }

    // taskBufferSize must be at least a multiple of 4, practical minimum size
    // for a task.
    if (ts->bufferSize < OS_GenericTaskMinBufferSize || ts->bufferSize & 0x03)
    {
        return OS_Result_InvalidBufferSize;
    }

    memset (ts->buffer, 0, ts->bufferSize);

    struct OS_TaskControl *tc = (struct OS_TaskControl *) ts->buffer;

    tc->size            = ts->bufferSize;
    tc->stackTop        = ts->bufferSize;
    tc->description     = ts->description;
    tc->startedAt       = OS_TicksUndefined;
    tc->terminatedAt    = OS_TicksUndefined;
    tc->priority        = ts->priority;
    tc->state           = OS_TaskState_Ready;
    tc->uid             = OS_TaskControlUID;

    SEMAPHORE_Init          (&tc->sleep, 1, 1);
    OS_USAGE_CpuReset       (&tc->metricsCpu);
    OS_USAGE_MemoryReset    (&tc->metricsStack);

    if (ts->bufferInitFunc)
    {
        enum OS_Result r;
        if ((r = ts->bufferInitFunc(tc, ts->bufferInitParams)) != OS_Result_OK)
        {
            return r;
        }
    }

    taskInitStack (tc, ts);

    QUEUE_PushNode (&g_OS->tasksReady[tc->priority], (struct QUEUE_Node *) tc);

    return OS_Result_OK;
}


static enum OS_Result taskYield ()
{
    OS_SchedulerSetPending ();

    return OS_Result_OK;
}


static enum OS_Result taskBootEnded ()
{
    BOARD_AssertState (g_OS->currentTask);

    struct OS_TaskControl *task = g_OS->currentTask;

    // No current task
    g_OS->currentTask = NULL;

    // Recycling OS owned boot task buffer for the idle task.
    struct OS_TaskStart ts;
    OS_TaskStart_Fill (&ts, (void *)task, task->size, taskIdle, NULL,
                       OS_TaskPriority_Idle, g_OS_TaskIdleDescription,
                       NULL, NULL);

    if (taskStart(&ts) != OS_Result_OK)
    {
        // Unrecoverable error
        BOARD_AssertState (false);

        while (1)
        {
            __WFI ();
        }
    }

    // Scheduler will choose the next task to run.
    OS_SchedulerSetPending ();

    return OS_Result_OK;
}


// WARNING: Anything inside each one of this functions must be able to be called
//          from the scheduler in handler mode. Not all user API functions
//          are callable from handler mode!.
static bool taskSigActionSemaphoreAcquire (void *sig)
{
    return SEMAPHORE_Acquire ((struct SEMAPHORE *) sig);
}


static bool taskSigActionSemaphoreRelease (void *sig)
{
    return SEMAPHORE_Release ((struct SEMAPHORE *) sig);
}


static bool taskSigActionMutexLock (void *sig)
{
    return (OS_MUTEX_Lock ((struct OS_MUTEX *) sig) == OS_Result_OK);
}


static bool taskSigActionMutexUnlock (void *sig)
{
    return (OS_MUTEX_Unlock ((struct OS_MUTEX *) sig) == OS_Result_OK);
}


static enum OS_Result taskWaitForSignal (struct OS_TaskWaitForSignal *wfs)
{
    if (!wfs
        || !wfs->task
        || !wfs->sigObject
        || wfs->sigType >= OS_TaskSignalType__COUNT)
    {
        return OS_Result_InvalidParams;
    }

    OS_SigAction sigAction = NULL;
    switch (wfs->sigType)
    {
        case OS_TaskSignalType_SemaphoreAcquire:
            sigAction = taskSigActionSemaphoreAcquire;
            break;
        case OS_TaskSignalType_SemaphoreRelease:
            sigAction = taskSigActionSemaphoreRelease;
            break;
        case OS_TaskSignalType_MutexLock:
            sigAction = taskSigActionMutexLock;
            break;
        case OS_TaskSignalType_MutexUnlock:
            sigAction = taskSigActionMutexUnlock;
            break;
        default:
            BOARD_AssertParams (false);
            break;
    }

    BOARD_AssertState (sigAction);

    const bool ActionResult = sigAction (wfs->sigObject);

    // Result inmediately returned if the action succeded or timeout is zero.
    if (ActionResult || !wfs->timeout)
    {
        return ActionResult? OS_Result_OK : OS_Result_Timeout;
    }

    // Configuring task to be suspended until timeout. In the meantime, the
    // scheduler will be retrying this signal action.
    wfs->task->sigWaitAction    = sigAction;
    wfs->task->sigWaitObject    = wfs->sigObject;
    wfs->task->sigWaitResult    = OS_Result_Waiting;
    wfs->task->suspendedUntil   = (wfs->timeout == OS_WaitForever)
                                        ? OS_WaitForever
                                        : TICKS_Now() + wfs->timeout;
    OS_SchedulerSetPending ();

    return OS_Result_Waiting;
}


#warning agregar puntero task a struct como en taskWaitForSignal()
static enum OS_Result taskDelayFrom (struct OS_TaskDelayFrom *df)
{
    if (!g_OS->currentTask)
    {
        return OS_Result_NoCurrentTask;
    }

    if (!df)
    {
        return OS_Result_InvalidParams;
    }

    g_OS->currentTask->suspendedUntil = df->from + df->ticks;

    OS_SchedulerSetPending ();

    return OS_Result_OK;
}


static enum OS_Result taskPeriodicDelay (TIMER_Ticks *ticks)
{
    if (!g_OS->currentTask)
    {
        return OS_Result_NoCurrentTask;
    }

    if (!ticks)
    {
        return OS_Result_InvalidParams;
    }

    // Zero ticks causes a lastSuspension reset with current ticks.
    if (*ticks == 0)
    {
        g_OS->currentTask->lastSuspension = TICKS_Now ();
        return OS_Result_OK;
    }

    g_OS->currentTask->suspendedUntil = g_OS->currentTask->lastSuspension
                                            + *ticks;
    OS_SchedulerSetPending ();

    return OS_Result_OK;
}


struct OS_TaskControl * taskFind (enum OS_TaskPriority priority,
                                  const char* description)
{
    struct OS_TaskControl   *task;
    struct QUEUE            *queue;

    queue = &g_OS->tasksWaiting[priority];
    for (task = (struct OS_TaskControl *) queue->head; task;
         task = (struct OS_TaskControl *) queue->head->next)
    {
        // Description matched by pointer address, not pointer contents.
        if (task->description == description)
        {
            return task;
        }
    }

    queue = &g_OS->tasksReady[priority];
    for (task = (struct OS_TaskControl *) queue->head; task;
         task = (struct OS_TaskControl *) queue->head->next)
    {
        if (task->description == description)
        {
            return task;
        }
    }

    task = g_OS->currentTask;
    if (task && task->priority == priority && task->description == description)
    {
        return task;
    }

    return NULL;
}


enum OS_Result taskSleep (struct OS_TaskControl *task)
{
    if (!task)
    {
        return OS_Result_InvalidParams;
    }

    if (!SEMAPHORE_Available (&task->sleep))
    {
        // Already sleeping
        return OS_Result_OK;
    }

    if (SEMAPHORE_Acquire (&task->sleep))
    {
        // This new signal adquisition request will keep the task sleeping
        // until the semaphore is released.
        struct OS_TaskWaitForSignal wfs;
        wfs.task        = task;
        wfs.sigType     = OS_TaskSignalType_SemaphoreAcquire;
        wfs.sigObject   = &task->sleep;
        wfs.start       = TICKS_Now ();
        wfs.timeout     = OS_WaitForever;

        return taskWaitForSignal (&wfs);
    }

    BOARD_AssertState (false);

    return OS_Result_Error;
}


enum OS_Result taskWakeup (struct OS_TaskControl *task)
{
    if (!task)
    {
        return OS_Result_InvalidParams;
    }

    // A sleeping task cannot wakeup itself. This condition is likely a bug.
    BOARD_AssertState (task != g_OS->currentTask);

    // If not awake, then release "sleep" semaphore. On the next scheduling,
    // pending signal adquisition will succeed and this tasks state will change
    // from WAITING to READY. Note that calling this function on an already
    // awaken task does not return an error but won't invoke the scheduler
    // either.
    if (!SEMAPHORE_Available (&task->sleep))
    {
        BOARD_AssertState (SEMAPHORE_Release(&task->sleep));

        OS_SchedulerSetPending ();
    }

    return OS_Result_OK;
}


#if 0
// Llamado desde OS_SyscallHandler, case OS_Syscall_TaskDriverStorageAccess

enum OS_Result taskDriverStorageAccess (struct OS_TaskDriverStorageAccess *sa)
{
    if (!g_OS->currentTask)
    {
        return OS_Result_NoCurrentTask;
    }

    if (!sa)
    {
        return OS_Result_InvalidParams;
    }

    if (!sa->description || !sa->buf || !sa->count)
    {
        return OS_Result_InvalidParams;
    }

    struct OS_TaskControl *task = taskFind (OS_TaskPriority_DriverStorage,
                                            sa->description);
    if (!task)
    {
        // Requested driver does not exist.
        return OS_Result_NotInitialized;
    }

    sa->result  = OS_Result_NotInitialized;
    sa->sem     = &g_OS->currentTask->sleep;

    enum OS_Result r;

    // Put the caller to sleep on the next scheduler execution.
    // The caller will be waken up by the storage driver when the job is
    // finished -or- the operation timeout has been reached.
    if ((r = taskSleep(g_OS->currentTask)) != OS_Result_OK)
    {
        return r;
    }

    // Add a new job to the driver.
    if ((r = OS_DRIVER_StorageJobAdd(task, sa)) != OS_Result_OK)
    {
        return r;
    }

    // If not already awake, wake up the driver to start the new job.
    if ((r = taskWakeup(task)) != OS_Result_OK)
    {
        return r;
    }

    // Waiting for the new job to be picked up by the driver and be done.
    return OS_Result_Waiting;
}
#endif


// Tasks can be abruptly terminated by calling OS_TaskTerminate(TaskBuffer)
// from another task.
static enum OS_Result taskTerminate (struct OS_TaskTerminate *tt)
{
    // This operation must be initiated by the current task either to terminate
    // itself or terminate another task.
    if (!g_OS->currentTask)
    {
        return OS_Result_NoCurrentTask;
    }

    if (!tt)
    {
        return OS_Result_InvalidParams;
    }

    if (tt->tc)
    {
        BOARD_AssertState (OS_TaskBufferSanityTest(tt->tc));
    }

    // tt->tc == NULL implies that the current task called
    // OS_TaskTerminate(NULL, x) to terminate itself.
    if (!tt->tc)
    {
        BOARD_AssertState (g_OS->currentTask->state
                                        == OS_TaskState_Running);
        tt->tc = g_OS->currentTask;
    }
    else if (tt->tc->state == OS_TaskState_Terminated)
    {
        // Trying to terminate an already terminated task, do nothing.
        return OS_Result_InvalidState;
    }

    switch (tt->tc->state)
    {
        case OS_TaskState_Running:
            // Only the currently running task can terminate the running task,
            // that is, itself.
            BOARD_AssertState (g_OS->currentTask == tt->tc);
            g_OS->currentTask = NULL;
            break;

        case OS_TaskState_Ready:
            QUEUE_DetachNode (&g_OS->tasksReady[tt->tc->priority],
                                            (struct QUEUE_Node *) tt->tc);
            break;

        case OS_TaskState_Waiting:
            QUEUE_DetachNode (&g_OS->tasksWaiting[tt->tc->priority],
                                            (struct QUEUE_Node *) tt->tc);
            break;

        default:
            // Invalid task state
            BOARD_AssertUnexpectedValue (NOBJ, (uint32_t)tt->tc->state);
            return OS_Result_InvalidState;
    }

    tt->tc->retValue        = tt->retVal;
    tt->tc->state           = OS_TaskState_Terminated;
    tt->tc->terminatedAt    = TICKS_Now ();

    // If there is no current task, it is assumed that the current running task
    // has been terminated itself (see above), so the scheduler needs to be
    // waken up to assign a new current task. The terminated task stack pointer
    // that called this syscall will no longer be used; the terminated task will
    // never receive and process the return value that follows.
    if (!g_OS->currentTask)
    {
        OS_SchedulerSetPending ();
        return OS_Result_OK;
    }
    else
    {
        // Check that there is a current task terminating a different task.
        BOARD_AssertState (g_OS->currentTask != tt->tc);
    }

    return OS_Result_OK;
}


enum OS_Result osTerminate ()
{
    if (g_OS->runMode != OS_RunMode_Finite)
    {
        // OS cannot be terminated
        return OS_Result_InvalidOperation;
    }

    g_OS->terminatedAt = TICKS_Now ();

    OS_SchedulerSetPending ();

    // When terminatedAt is given a valid number of ticks, the scheduler will
    // perform a last context switch to recover execution from MSP right after
    // the first context switch at OS_Start(). Then scheduler execution will be
    // terminated along with any defined task, even the caller of this function.
    return OS_Result_OK;
}


enum OS_Result OS_SyscallHandler (enum OS_Syscall call, void *params)
{
    BOARD_AssertState (g_OS);

    switch (call)
    {
        case OS_Syscall_TaskBootEnded:
            return taskBootEnded ();

        case OS_Syscall_TaskStart:
            return taskStart ((struct OS_TaskStart *) params);

        case OS_Syscall_TaskYield:
            return taskYield ();

        case OS_Syscall_TaskWaitForSignal:
            return taskWaitForSignal ((struct OS_TaskWaitForSignal *) params);

        case OS_Syscall_TaskDelayFrom:
            return taskDelayFrom ((struct OS_TaskDelayFrom *) params);

        case OS_Syscall_TaskPeriodicDelay:
            return taskPeriodicDelay ((TIMER_Ticks *) params);

//        case OS_Syscall_TaskDriverStorageAccess:
//            return taskDriverStorageAccess (
//                                (struct OS_TaskDriverStorageAccess *) params);

        case OS_Syscall_TaskTerminate:
            return taskTerminate ((struct OS_TaskTerminate *) params);

        case OS_Syscall_Terminate:
            return osTerminate ();

        default:
            break;
    }

    return OS_Result_InvalidParams;
}


// This function can only be called from MSP / Privileged mode.
enum OS_Result OS_SyscallBoot (enum OS_RunMode runMode, OS_Task bootTask,
                               OS_TaskParam bootParam)
{
    BOARD_AssertAccess (OS_RuntimePrivileged ());
    BOARD_AssertState  (g_OS);
    BOARD_AssertState  (g_OS->runMode == OS_RunMode_Undefined);

    g_OS->runMode = runMode;

    struct OS_TaskStart ts;
    OS_TaskStart_Fill (&ts, g_OS->bootIdleTaskBuffer,
                       sizeof (g_OS->bootIdleTaskBuffer),
                       bootTask, bootParam, OS_TaskPriority_Boot,
                       g_OS_TaskBootDescription, NULL, NULL);

    enum OS_Result r = taskStart (&ts);
    if (r == OS_Result_OK)
    {
        // PSP at zero marks the first switch from MSP to PSP after
        // initialization.
        __set_PSP (0);

        BOARD_SetTickHook (OS_SchedulerTickCallback, &g_OS_PrevTickHookFunc);

        OS_SchedulerSetPending ();
    }

    return r;
}


// This function can only be called from MSP / Privileged mode.
enum OS_Result OS_SyscallShutdown (void)
{
    BOARD_AssertAccess (OS_RuntimePrivileged ());
    BOARD_AssertState  (g_OS);
    BOARD_AssertState  (g_OS->runMode == OS_RunMode_Finite);

    OS_SchedulerClearPending ();

    BOARD_SetTickHook (g_OS_PrevTickHookFunc, NULL);

    g_OS = NULL;

    return OS_Result_OK;
}


void OS_TaskStart_Fill (struct OS_TaskStart *ts, void *buffer,
                        uint32_t bufferSize, OS_Task func, OS_TaskParam param,
                        enum OS_TaskPriority priority, const char *description,
                        OS_TaskBufferInitFunc bufferInitFunc,
                        void *bufferInitParams)
{
    BOARD_AssertParams  ts->bufferSize          = bufferSize;
    ts->func                = func;
    ts->param               = param;
    ts->priority            = priority;
    ts->description         = description;
    ts->bufferInitFunc      = bufferInitFunc;
    ts->bufferInitParams    = bufferInitParams;
}
