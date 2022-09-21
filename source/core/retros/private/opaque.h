/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  RetrOS™: real-time preemtive multitasking operating system.
  private (opaque to the user) definitions.

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

#include "embedul.ar/source/core/retros/private/metrics.h"
#include "embedul.ar/source/core/retros/semaphore.h"
#include "embedul.ar/source/core/retros/api.h"
#include "embedul.ar/source/core/cc.h"
#include "embedul.ar/source/core/queue.h"
#include <stdalign.h>


#define OS_TaskControlUID                   0x7A5C057C
#define OS_IntegerRegisters                 17
#define OS_FPointRegisters                  (16 + 1 + 16)
#define OS_ContextRegisters                 (OS_FPointRegisters \
                                                + OS_IntegerRegisters)
#define OS_GenericTaskMinStackSize          128
#define OS_GenericTaskOverhead              (sizeof(struct OS_TaskControl) \
                                                + (OS_ContextRegisters * 4))
#define OS_GenericTaskBufferSize(stack)     (OS_GenericTaskOverhead + (stack))
#define OS_GenericTaskMinBufferSize         OS_GenericTaskBufferSize( \
                                                OS_GenericTaskMinStackSize)


typedef bool                (* OS_SigAction) (void *sig);
typedef void                (* OS_TaskReturn) (OS_TaskRetVal retVal);


extern struct OS            * g_OS;
extern volatile uint32_t    g_OS_SchedulerPending;
extern volatile uint32_t    g_OS_SchedulerTickBarrier;
extern volatile uint32_t    g_OS_SchedulerTicksMissed;
extern const char           * g_OS_TaskBootDescription;
extern const char           * g_OS_TaskIdleDescription;
extern TIMER_TickHookFunc   g_OS_PrevTickHookFunc;


enum OS_TaskState
{
    OS_TaskState_Terminated = 0,
    OS_TaskState_Waiting,
    OS_TaskState_Ready,
    OS_TaskState_Running,
    OS_TaskState__COUNT,
    CC_EnumForce32 (OS_TaskState)
};


#warning use alignas on variables. check alignment with alignof
alignas(8) struct OS_TaskControl
{
    struct QUEUE_Node       node;
    uint32_t                size;
    uint32_t                stackTop;
    const char              * description;
    uint32_t                retValue;
    TIMER_Ticks             startedAt;
    TIMER_Ticks             terminatedAt;
    TIMER_Ticks             suspendedUntil;
    TIMER_Ticks             lastSuspension;
    OS_SigAction            sigWaitAction;
    void                    * sigWaitObject;
    enum OS_Result          sigWaitResult;
    struct SEMAPHORE        sleep;
    enum OS_TaskPriority    priority;
    enum OS_TaskState       state;
    OS_Cycles               runCycles;
    struct OS_MetricsCpu    metricsCpu;
    struct OS_MetricsStack  metricsStack;
    uint32_t                sp;
    uint32_t                uid;
};


struct OS
{
    TIMER_Ticks             startedAt;
    TIMER_Ticks             terminatedAt;
    enum OS_RunMode         runMode;
    OS_Cycles               runCycles;
    struct OS_Metrics       metrics;
    struct OS_MetricsCpu    metricsCpu;
    // The one and only RUNNING task.
    struct OS_TaskControl   * currentTask;
    // Tasks in READY state.
    struct QUEUE            tasksReady          [OS_TaskPriority__COUNT];
    // Tasks in WAITING state.
    struct QUEUE            tasksWaiting        [OS_TaskPriority__COUNT];
    // OS managed task buffer. Used for boot task at init, then for idle task.
    alignas(8) uint8_t      bootIdleTaskBuffer  [OS_GenericTaskMinBufferSize];
}
CC_Aligned(8);
