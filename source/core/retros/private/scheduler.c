/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  RetrOS™: real-time preemtive multitasking operating system.
  scheduler functions.

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

#include "embedul.ar/source/core/retros/private/scheduler.h"
#include "embedul.ar/source/core/retros/private/opaque.h"
#include "embedul.ar/source/core/retros/private/runtime.h"
#include "embedul.ar/source/core/retros/private/metrics.h"
#include "embedul.ar/source/core/device/board.h"

// ARM Cortex only
#include "cmsis.h"


inline static void taskSigWaitEnd (struct OS_TaskControl *tc,
                                   enum OS_Result result)
{
    tc->sigWaitAction = NULL;
    tc->sigWaitObject = NULL;
    tc->sigWaitResult = result;
}


static void taskUpdateState (struct OS_TaskControl *tc, const TIMER_Ticks Now)
{
    // Task ready (not suspended).
    if (!tc->suspendedUntil)
    {
        tc->state = OS_TaskState_Ready;
        return;
    }

    // SuspendedUntil timeout still in the future.
    if (tc->suspendedUntil > Now)
    {
        // Start by assuming a waiting state
        tc->state = OS_TaskState_Waiting;

        // No signal to test, keep waiting for timeout.
        if (!tc->sigWaitAction)
        {
            return;
        }

        // On any wait action there must be a signal object
        BOARD_AssertState (tc->sigWaitObject);

        // Do the signal test
        {
            // Trick to be able to get a correct OS_TaskSelf () inside a signal
            // action run by the scheduler.
            struct OS_TaskControl *ctb = g_OS->currentTask;
            g_OS->currentTask = tc;

            // if the signal the task was waiting for can be adquired then
            // switch it to a ready state.
            if (tc->sigWaitAction (tc->sigWaitObject))
            {
                taskSigWaitEnd (tc, OS_Result_OK);
                tc->suspendedUntil  = 0;
                tc->state           = OS_TaskState_Ready;
            }

            g_OS->currentTask = ctb;
        }
    }
    // SuspendedUntil timed-out.
    else
    {
        if (!tc->sigWaitAction)
        {
            // Not waiting for a signal. Timeout is over.
            tc->lastSuspension = tc->suspendedUntil;
        }
        else
        {
            // Signal action timed out.
            taskSigWaitEnd (tc, OS_Result_Timeout);
        }

        tc->suspendedUntil  = 0;
        tc->state           = OS_TaskState_Ready;
    }
}


inline static void schedulerUpdateWaitingTasks (const TIMER_Ticks Now)
{
    for (int i = OS_TaskPriority__BEGIN; i < OS_TaskPriority__COUNT; ++i)
    {
        struct OS_TaskControl *tc;

        for (tc = (struct OS_TaskControl *)(void *)g_OS->tasksWaiting[i].head;
             tc; tc = (struct OS_TaskControl *)(void *)tc->node.next)
        {
            BOARD_AssertState (OS_TaskBufferSanityTest(tc));

            taskUpdateState (tc, Now);

            if (tc->state == OS_TaskState_Ready)
            {
                // Task state updated from Wating to Ready.
                QUEUE_DetachNode (&g_OS->tasksWaiting[i],
                                                (struct QUEUE_Node *) tc);
                QUEUE_PushNode   (&g_OS->tasksReady[i],
                                                (struct QUEUE_Node *) tc);
            }

            BOARD_AssertState (OS_TaskBufferSanityTest(tc));
        }
    }
}


inline static void schedulerUpdateLastTaskMeasures ()
{
    // First iteration of a new performance measurement period?
    if (!OS_USAGE_UpdatingLastMeasures (&g_OS->metrics))
    {
        // No, keep getting performance measurements from the running task on
        // each context switch.
        return;
    }

    // Process last period accumulated performance measurements for every task
    // defined on every state (running, ready and waiting).
    struct OS_TaskControl *task = g_OS->currentTask;
    if (task)
    {
        const int32_t TaskMemory = OS_USAGE_GetUsedTaskMemory (task);
        OS_USAGE_UpdateLastMeasures (&g_OS->metrics, &task->metricsCpu,
                                        &task->metricsStack, TaskMemory,
                                        task->size);
    }

    for (int i = OS_TaskPriority__BEGIN; i < OS_TaskPriority__COUNT; ++i)
    {
        for (task = (struct OS_TaskControl *)(void *)g_OS->tasksWaiting[i].head;
             task; task = (struct OS_TaskControl *)(void *)task->node.next)
        {
            const int32_t TaskMemory = OS_USAGE_GetUsedTaskMemory (task);
            OS_USAGE_UpdateLastMeasures (&g_OS->metrics, &task->metricsCpu,
                                            &task->metricsStack, TaskMemory,
                                            task->size);
        }

        for (task = (struct OS_TaskControl *)(void *)g_OS->tasksReady[i].head;
             task; task = (struct OS_TaskControl *)(void *)task->node.next)
        {
            const int32_t TaskMemory = OS_USAGE_GetUsedTaskMemory (task);
            OS_USAGE_UpdateLastMeasures (&g_OS->metrics, &task->metricsCpu,
                                            &task->metricsStack, TaskMemory,
                                            task->size);
        }
    }
}


inline static void schedulerUpdateOwnMeasures ()
{
    // -Approximate- number of cycles used for task scheduling. It depends on
    // 1) External interrupts preemting PendSV (*).
    // 2) Code not taken into account after measurements took place
    //    (ie DWT->CYCCNT).
    // (*) Note that tasks can be preempted too.
    OS_USAGE_UpdateCurrentMeasures (&g_OS->metrics, &g_OS->metricsCpu, NULL,
                                   DWT->CYCCNT, 0);

    if (OS_USAGE_UpdatingLastMeasures (&g_OS->metrics))
    {
        OS_USAGE_UpdateLastMeasures (&g_OS->metrics, &g_OS->metricsCpu, NULL,
                                     0, 0);
    }
}


inline static void schedulerLastTaskUpdate (const uint32_t CurrentSp,
                                            const uint32_t TaskCycles,
                                            const TIMER_Ticks Now)
{
    struct OS_TaskControl *tc = g_OS->currentTask;
    if (!tc)
    {
        // if g_OS->currentTask == NULL, it is assumed that CurrentSp belongs to
        // the first context switch from MSP or an already terminated task,
        // therefore there is no task control register to update.
        return;
    }

    // Task switch from a previous running task.
    BOARD_AssertState  (OS_TaskBufferSanityTest(tc));
    BOARD_AssertState  (tc->state == OS_TaskState_Running);

    // Store task last status.
    tc->runCycles += TaskCycles;
    tc->sp         = CurrentSp;

    // Memory usage metrics always includes static use of own control structure
    // (sizeof(OS_TaskControl)).
    const int32_t CurMemory = OS_USAGE_GetUsedTaskMemory (tc);

    OS_USAGE_UpdateCurrentMeasures (&g_OS->metrics, &tc->metricsCpu,
                                    &tc->metricsStack, TaskCycles, CurMemory);

    taskUpdateState (tc, Now);

    switch (tc->state)
    {
        case OS_TaskState_Ready:
            QUEUE_PushNode (&g_OS->tasksReady[tc->priority],
                                            (struct QUEUE_Node *) tc);
            break;

        case OS_TaskState_Waiting:
            QUEUE_PushNode (&g_OS->tasksWaiting[tc->priority],
                                            (struct QUEUE_Node *) tc);
            break;

        default:
            // Invalid current task state for g_OS->currentTask.
            BOARD_AssertUnexpectedValue (NOBJ, (uint32_t)tc->state);
            break;
    }

    BOARD_AssertState (OS_TaskBufferSanityTest(tc));

    g_OS->currentTask = NULL;
}


inline static void schedulerFindNextTask ()
{
    // There must be no task selected at this point
    BOARD_AssertState (!g_OS->currentTask);

    // Find next task according to priority and a round-robin scheme between
    // same priority tasks.
    for (int i = OS_TaskPriority__BEGIN; i < OS_TaskPriority__COUNT; ++i)
    {
        struct QUEUE *queue = &g_OS->tasksReady[i];
        if (queue->head)
        {
            g_OS->currentTask = (struct OS_TaskControl *)(void *)queue->head;
            QUEUE_DetachNode (queue, queue->head);
            break;
        }
    }
}


inline static void schedulerSetCurrentTaskReadyToRun (const TIMER_Ticks Now)
{
    // At least one task must have been selected.
    BOARD_AssertState (g_OS->currentTask);

    // Task buffer integrity test.
    BOARD_AssertState (OS_TaskBufferSanityTest(g_OS->currentTask));

    g_OS->currentTask->state = OS_TaskState_Running;

    // First time running?
    if (g_OS->currentTask->startedAt == OS_TicksUndefined)
    {
        g_OS->currentTask->startedAt = Now;
    }

    // Privileged level for Boot and Kernel priorities. Unprivileged for User.
    switch (g_OS->currentTask->priority)
    {
        case OS_TaskPriority_Boot:
        case OS_TaskPriority_Kernel0:
        case OS_TaskPriority_Kernel1:
        case OS_TaskPriority_Kernel2:
            // CONTROL[0] = 0, Privileged state in thread mode
            __set_CONTROL ((__get_CONTROL() & ~0x01u));
            __ISB ();
            break;

        default:
            // CONTROL[0] = 1, User state in thread mode
            __set_CONTROL ((__get_CONTROL() | 0x01));
            __ISB ();
            break;
    }
}


uint32_t OS_Scheduler (const uint32_t CurrentSp, const uint32_t TaskCycles)
{
    BOARD_AssertState (g_OS);

    g_OS_SchedulerPending = 0;

    // if terminatedAt has a valid amount of ticks, scheduler will return a
    // special return value (0, closing) to the PendSV handler code.
    if (g_OS->terminatedAt != OS_TicksUndefined)
    {
        return 0;
    }

    // Avoid getting different tick readings along the scheduling process
    // (May vary depending on external interrupts preempting PendSV).
    const TIMER_Ticks Now = TICKS_Now ();

    // First scheduler run
    if (!CurrentSp)
    {
        BOARD_AssertState (g_OS->startedAt == OS_TicksUndefined);
        BOARD_AssertState (!g_OS->currentTask);

        g_OS->startedAt = Now;
    }

    // Update all tasks currently in WAITING state.
    schedulerUpdateWaitingTasks (Now);

    // Update last executed task, if there is one.
    schedulerLastTaskUpdate (CurrentSp, TaskCycles, Now);

    // Find next among all READY tasks.
    schedulerFindNextTask ();

    // Set the selected task ready to be run.
    schedulerSetCurrentTaskReadyToRun (Now);

    // Check if current usage measurement period has ended.
    OS_USAGE_UpdateTarget (&g_OS->metrics, Now);

    // Update tasks and scheduler measurements accordingly.
    schedulerUpdateLastTaskMeasures ();
    schedulerUpdateOwnMeasures ();

    return g_OS->currentTask->sp;
}


enum OS_Result OS_SchedulerSetPending ()
{
    if (!OS_RuntimePrivileged ())
    {
        return OS_Result_InvalidCaller;
    }

    g_OS_SchedulerPending = 1;

    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;

    __DSB ();
    __ISB ();

    return OS_Result_OK;
}


enum OS_Result OS_SchedulerClearPending ()
{
    if (!OS_RuntimePrivileged ())
    {
        return OS_Result_InvalidCaller;
    }

    g_OS_SchedulerPending = 0;

    SCB->ICSR |= SCB_ICSR_PENDSVCLR_Msk;

    __DSB ();
    __ISB ();

    return OS_Result_OK;
}


void OS_SchedulerTickCallback (TIMER_Ticks ticks)
{
    (void) ticks;

    OS_SchedulerSetPending ();
}
