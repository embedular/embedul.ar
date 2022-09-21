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

#pragma once

#include "embedul.ar/source/core/timer.h"
#include "embedul.ar/source/core/cc.h"

#include <stdint.h>
#include <stdbool.h>


#define OS_IntPriorityTicks         0
#define OS_IntPrioritySyscall       1
#define OS_IntPriorityScheduler     ((1 << __NVIC_PRIO_BITS) - 1)

#define OS_WaitForever              ((TIMER_Ticks) - 1)
#define OS_TicksUndefined           (OS_WaitForever - 1)

typedef uint32_t                    OS_TaskRetVal;
typedef void *                      OS_TaskParam;
typedef OS_TaskRetVal               (* OS_Task)(OS_TaskParam);


enum OS_Result
{
    OS_Result_OK                    = 0,
    OS_Result_Error                 = 1,
    OS_Result_InvalidCaller,
    OS_Result_InvalidParams,
    OS_Result_InvalidBuffer,
    OS_Result_InvalidBufferAlignment,
    OS_Result_InvalidBufferSize,
    OS_Result_InvalidState,
    OS_Result_InvalidOperation,
    OS_Result_Timeout,
    OS_Result_Waiting,
    OS_Result_Retry,
    OS_Result_BufferFull,
    OS_Result_Empty,
    OS_Result_Locked,
    OS_Result_AlreadyInitialized,
    OS_Result_NotInitialized,
    OS_Result_NoCurrentTask,
    CC_EnumForce32 (OS_Result)
};


enum OS_RunMode
{
    OS_RunMode_Undefined            = 0,
    OS_RunMode_Forever,
    OS_RunMode_Finite,
    CC_EnumForce32 (OS_RunMode)
};


enum OS_TaskPriority
{
    OS_TaskPriority_Boot            = 0,
    OS_TaskPriority_Kernel0,
    OS_TaskPriority_Kernel1,
    OS_TaskPriority_Kernel2,
    OS_TaskPriority_User0,
    OS_TaskPriority_User1,
    OS_TaskPriority_User2,
    OS_TaskPriority_Idle,
    OS_TaskPriority__COUNT,
    OS_TaskPriority__BEGIN          = OS_TaskPriority_Boot,
    OS_TaskPriority_KernelHighest   = OS_TaskPriority_Kernel0,
    OS_TaskPriority_KernelLowest    = OS_TaskPriority_Kernel2,
    OS_TaskPriority_UserHighest     = OS_TaskPriority_User0,
    OS_TaskPriority_UserLowest      = OS_TaskPriority_User2,
    CC_EnumForce32 (OS_TaskPriority)
};


enum OS_TaskSignalType
{
    OS_TaskSignalType_SemaphoreAcquire = 0,
    OS_TaskSignalType_SemaphoreRelease,
    OS_TaskSignalType_MutexLock,
    OS_TaskSignalType_MutexUnlock,
    OS_TaskSignalType__COUNT,
    CC_EnumForce32 (OS_TaskSignalType)
};


uint32_t        OS_InitBufferSize       (void);
uint32_t        OS_TaskBufferSize       (uint32_t stack);
bool            OS_TaskBufferSanityTest (void *taskBuffer);
enum OS_Result  OS_Init                 (void *buffer);
void            OS_Forever              (OS_Task bootTask,
                                         OS_TaskParam bootParam);
enum OS_Result  OS_Start                (OS_Task bootTask,
                                         OS_TaskParam bootParam);
enum OS_Result  OS_Terminate            (void);

enum OS_Result  OS_TaskStart            (void *taskBuffer, uint32_t bufferSize,
                                         OS_Task func, OS_TaskParam param,
                                         enum OS_TaskPriority priority,
                                         const char *description);
enum OS_Result  OS_TaskTerminate        (void *taskBuffer,
                                         OS_TaskRetVal retVal);

void *          OS_TaskSelf             (void);
enum OS_Result  OS_TaskYield            (void);
enum OS_Result  OS_TaskPeriodicDelay    (TIMER_Ticks ticks);
enum OS_Result  OS_TaskDelayFrom        (TIMER_Ticks ticks,
                                         TIMER_Ticks from);
enum OS_Result  OS_TaskDelay            (TIMER_Ticks ticks);
enum OS_Result  OS_TaskWaitForSignal    (enum OS_TaskSignalType sigType,
                                         void *sigObject,
                                         TIMER_Ticks timeout);
enum OS_Result  OS_TaskSleep            (void *taskBuffer);
enum OS_Result  OS_TaskWakeup           (void *taskBuffer);
enum OS_Result  OS_TaskReturnValue      (void *taskBuffer, uint32_t *retValue);
