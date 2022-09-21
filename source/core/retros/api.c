/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  RetrOS™: real-time preemtive multitasking operating system.
  user api.

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

#include "embedul.ar/source/core/retros/api.h"
#include "embedul.ar/source/core/retros/mutex.h"
#include "embedul.ar/source/core/retros/private/opaque.h"
#include "embedul.ar/source/core/retros/private/syscall.h"
#include "embedul.ar/source/core/retros/private/runtime.h"
#include "embedul.ar/source/core/retros/private/driver/storage.h"
#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/core/queue.h"

#include <stdbool.h>

// ARM Cortex only
#include "cmsis.h"


uint32_t OS_InitBufferSize (void)
{
    return sizeof (struct OS);
}


uint32_t OS_TaskBufferSize (uint32_t stack)
{
    return OS_GenericTaskBufferSize(stack? stack : OS_GenericTaskMinStackSize);
}


enum OS_Result OS_Init (void *buffer)
{
    BOARD_AssertAccess (!OS_RuntimeTask ());
    BOARD_AssertState  (!g_OS);

    if (!buffer)
    {
        return OS_Result_InvalidParams;
    }

    struct OS *os = (struct OS *) buffer;

    ZERO_MEMORY (os);

    // Highest priority for Systick, second high for SVC and lowest for PendSV.
    // Priorities for external interrupts -like those used for peripherals- must
    // be set above SVCall and below PendSV.
    NVIC_SetPriority (SysTick_IRQn  ,OS_IntPriorityTicks);
    NVIC_SetPriority (SVCall_IRQn   ,OS_IntPrioritySyscall);
    NVIC_SetPriority (PendSV_IRQn   ,OS_IntPriorityScheduler);

    // Enable MCU cycle counter
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    for (int i = OS_TaskPriority__BEGIN; i < OS_TaskPriority__COUNT; ++i)
    {
        QUEUE_Init (&os->tasksReady[i]);
        QUEUE_Init (&os->tasksWaiting[i]);
    }

    os->startedAt       = OS_TicksUndefined;
    os->terminatedAt    = OS_TicksUndefined;
    os->runMode         = OS_RunMode_Undefined;

    OS_USAGE_Init (&os->metrics);

    g_OS = os;

    return OS_Result_OK;
}


bool OS_TaskBufferSanityTest (void *taskBuffer)
{
    BOARD_AssertParams (taskBuffer);

    struct OS_TaskControl *tc = (struct OS_TaskControl *) taskBuffer;

    return (tc->uid == OS_TaskControlUID
            && tc->size
            && tc->stackTop
            && tc->stackTop <= tc->size
            && tc->priority < OS_TaskPriority__COUNT
            && tc->state < OS_TaskState__COUNT
            && tc->sp <= ((uint32_t)taskBuffer) + tc->stackTop
            && tc->sp > ((uint32_t)taskBuffer) + sizeof(struct OS_TaskControl));
}


CC_NoReturn
void OS_Forever (OS_Task bootTask, OS_TaskParam bootParam)
{    
    BOARD_AssertAccess (!OS_RuntimeTask());
    BOARD_AssertState  (g_OS);
    BOARD_AssertParams (bootTask);

    enum OS_Result r = OS_SyscallBoot (OS_RunMode_Forever, bootTask, bootParam);

    BOARD_AssertInitialized  (r == OS_Result_OK);

    // OS_Terminate() cannot be called on ON_RunMode_Forever, the scheduler
    // will run forever. The program must never get here.
    while (1)
    {
        BOARD_AssertState (false);
        __WFI ();
    }
}


enum OS_Result OS_Start (OS_Task bootTask, OS_TaskParam bootParam)
{
    BOARD_AssertAccess (!OS_RuntimeTask ());
    BOARD_AssertState  (g_OS);
    BOARD_AssertParams (bootTask);

    enum OS_Result r = OS_SyscallBoot (OS_RunMode_Finite, bootTask, bootParam);

    if (r != OS_Result_OK)
    {
        struct VARIANT v[1];
        VARIANT_SetInt (&v[0], r);
        DEBUG_MSG_ARGS ("Error booting RetrOS: %1.", &v[0], 1);
        // Unrecoverable error while trying to boot
        return r;
    }
    ////////////////////////////////////////////////////////////////////////////
    // MSP returs to this point after the OS is instructed to terminate.
    ////////////////////////////////////////////////////////////////////////////
    BOARD_AssertAccess (!OS_RuntimeTask());
    BOARD_AssertState  (g_OS && g_OS->runMode == OS_RunMode_Finite);

    return OS_SyscallShutdown ();
}


enum OS_Result OS_Terminate (void)
{
    BOARD_AssertAccess (OS_RuntimePrivilegedTask ());

    return OS_Syscall (OS_Syscall_Terminate, NULL);
}


enum OS_Result OS_TaskStart (void *taskBuffer, uint32_t bufferSize,
                             OS_Task func, OS_TaskParam param,
                             enum OS_TaskPriority priority,
                             const char *description)
{
    BOARD_AssertAccess (OS_RuntimePrivilegedTask ());
    BOARD_AssertParams (priority >= OS_TaskPriority_KernelHighest
                         && priority <= OS_TaskPriority_UserLowest);

    struct OS_TaskStart ts;
    OS_TaskStart_Fill (&ts, taskBuffer, bufferSize, func, param,
                       priority, description, NULL, NULL);

    return OS_Syscall (OS_Syscall_TaskStart, &ts);
}


enum OS_Result OS_TaskTerminate (void *taskBuffer, OS_TaskRetVal retVal)
{
    if (!OS_RuntimePrivilegedTask ())
    {
        return OS_Result_InvalidCaller;
    }

    struct OS_TaskTerminate tt;
    tt.retVal   = retVal;
    tt.tc       = (struct OS_TaskControl *) taskBuffer;

    return OS_Syscall (OS_Syscall_TaskTerminate, &tt);
}


void * OS_TaskSelf (void)
{
    BOARD_AssertAccess (OS_RuntimeTask ());
    BOARD_AssertState  (g_OS);

    return (void *) g_OS->currentTask;
}


enum OS_Result OS_TaskYield (void)
{
    BOARD_AssertAccess (OS_RuntimeTask ());

    return OS_Syscall (OS_Syscall_TaskYield, NULL);
}


enum OS_Result OS_TaskDelayFrom (CTICK_Qty ticks, CTICK_Qty from)
{
    BOARD_AssertAccess (OS_RuntimeTask ());

    struct OS_TaskDelayFrom df;
    df.ticks    = ticks;
    df.from     = from;

    return OS_Syscall (OS_Syscall_TaskDelayFrom, &df);
}


// When ticks == 0, lastSuspension resets with current ticks.
// An alternative to lastSuspension init might be a first call to
// OS_TaskDelay (ticks)
enum OS_Result OS_TaskPeriodicDelay (CTICK_Qty ticks)
{
    BOARD_AssertAccess (OS_RuntimeTask ());

    return OS_Syscall (OS_Syscall_TaskPeriodicDelay, &ticks);
}


enum OS_Result OS_TaskDelay (CTICK_Qty ticks)
{
    return OS_TaskDelayFrom (ticks, BOARD_TicksNow());
}


enum OS_Result OS_TaskWaitForSignal (enum OS_TaskSignalType sigType,
                                     void *sigObject, CTICK_Qty timeout)
{
    BOARD_AssertAccess (OS_RuntimeTask ());
    BOARD_AssertState  (g_OS->currentTask);

    struct OS_TaskWaitForSignal wfs;
    wfs.task        = OS_TaskSelf ();
    wfs.sigType     = sigType;
    wfs.sigObject   = sigObject;
    wfs.start       = BOARD_TicksNow ();
    wfs.timeout     = timeout;

    // NOTE: if timeout == 0 an inmediate result will be received, either
    //       OS_Result_OK or OS_Result_Timeout indicating the signal could not
    //       be taken).
    //
    //       if timeout > 0 and the inmediate result was not OK, this task will
    //       be configured to wait for a signal and a PendSV event will be
    //       set to pending to execute the scheduler. This SVC system call
    //       will return the operation last known state (OS_Result_Waiting),
    //       PendSV will be called and this task will be set to WAITING then
    //       switching back here after transitioning to READY as the result of a
    //       timeout or signal adquisition. That is why a final read to the
    //       signal result must be returned to the calling task.
    enum OS_Result r = OS_Syscall (OS_Syscall_TaskWaitForSignal, &wfs);
    if (r != OS_Result_Waiting)
    {
        // Inmediate result
        return r;
    }

    // Deferred result
    return ((struct OS_TaskControl *) OS_TaskSelf ())->sigWaitResult;
}


enum OS_Result OS_TaskSleep (void *taskBuffer)
{

}


enum OS_Result OS_TaskWakeUp (void *taskBuffer)
{

}

#if 0
// Solicitid de transferencia al driver de storage

enum OS_Result OS_TaskDriverStorage (const char* description,
                                     enum OS_TaskDriverOp op,
                                     uint8_t *buf, uint32_t sector,
                                     uint32_t count)
{
    if (!OS_RuntimeTask ())
    {
        return OS_Result_InvalidCaller;
    }

    struct OS_TaskDriverStorageAccess sa;
    sa.description  = description;
    sa.op           = op;
    sa.buf          = buf;
    sa.sector       = sector;
    sa.count        = count;

    enum OS_Result r = OS_Syscall (OS_Syscall_TaskDriverStorageAccess, &sa);
    if (r != OS_Result_Waiting)
    {
        // Inmediate result
        return r;
    }

    // Deferred result
    return sa.result;
}
#endif


enum OS_Result OS_TaskReturnValue (void *taskBuffer, uint32_t *retValue)
{
    BOARD_AssertParams (OS_TaskBufferSanityTest(taskBuffer));
    BOARD_AssertParams (retValue);

    struct OS_TaskControl *tc = (struct OS_TaskControl *) taskBuffer;

    if (tc->state != OS_TaskState_Terminated)
    {
        return OS_Result_InvalidState;
    }

    *retValue = tc->retValue;

    return OS_Result_OK;
}
