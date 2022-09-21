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

#pragma once

#include "embedul.ar/source/core/retros/private/opaque.h"


enum OS_Syscall
{
    OS_Syscall_TaskBootEnded = 0,
    OS_Syscall_TaskStart,
    OS_Syscall_TaskYield,
    OS_Syscall_TaskWaitForSignal,
    OS_Syscall_TaskDelayFrom,
    OS_Syscall_TaskPeriodicDelay,
//    OS_Syscall_TaskDriverStorageAccess,
    OS_Syscall_TaskTerminate,
    OS_Syscall_Terminate,
    CC_EnumForce32 (OS_Syscall)
};


typedef enum OS_Result (* OS_TaskBufferInitFunc)(struct OS_TaskControl *tc,
                                                 void *params);


// SysCall params
struct OS_TaskStart
{
    void                    * buffer;
    uint32_t                bufferSize;
    OS_Task                 func;
    OS_TaskParam            param;
    enum OS_TaskPriority    priority;
    const char              * description;
    OS_TaskBufferInitFunc   bufferInitFunc;
    void                    * bufferInitParams;
};


struct OS_TaskWaitForSignal
{
    struct OS_TaskControl   * task;
    enum OS_TaskSignalType  sigType;
    void                    * sigObject;
    TIMER_Ticks             start;
    TIMER_Ticks             timeout;
};


struct OS_TaskDelayFrom
{
    TIMER_Ticks             ticks;
    TIMER_Ticks             from;
};


enum OS_TaskDriverOp
{
    OS_TaskDriverOp_Read,
    OS_TaskDriverOp_Write,
    OS_TaskDriverOP__COUNT,
    OS_TaskDriverOp_Recv    = OS_TaskDriverOp_Read,
    OS_TaskDriverOp_Send    = OS_TaskDriverOp_Write
};


struct OS_TaskDriverStorageAccess
{
    const char              * description;
    enum OS_TaskDriverOp    op;
    uint8_t                 * buf;
    uint32_t                sector;
    uint32_t                count;
    uint32_t                processed;
    TIMER_Ticks             timeout;
    struct SEMAPHORE        * sem;
    enum OS_Result          result;
};


struct OS_TaskDriverComAccess
{
    const char              * description;
    enum OS_TaskDriverOp    op;
    uint8_t                 * buf;
    uint32_t                count;
    uint32_t                processed;
    TIMER_Ticks             timeout;
    struct SEMAPHORE        * sem;
    enum OS_Result          result;
};


struct OS_TaskTerminate
{
    struct OS_TaskControl   * tc;
    OS_TaskRetVal           retVal;
};


extern enum OS_Result   OS_Syscall          (enum OS_Syscall call,
                                             void *params);
enum OS_Result          OS_SyscallBoot      (enum OS_RunMode runMode,
                                             OS_Task bootTask,
                                             OS_TaskParam bootParam);
enum OS_Result          OS_SyscallShutdown  (void);

void    OS_TaskStart_Fill       (struct OS_TaskStart *ts, void *buffer,
                                 uint32_t bufferSize, OS_Task func,
                                 void *funcParam, enum OS_TaskPriority priority,
                                 const char *description,
                                 OS_TaskBufferInitFunc bufferInitFunc,
                                 void *bufferInitParams);
