/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  RetrOS™: real-time preemtive multitasking operating system.
  resource usage measurements and statistics.

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

#include "embedul.ar/source/core/retros/api.h"

#include <stdbool.h>


typedef uint64_t    OS_Cycles;

#define OS_UsageDefaultTargetTicks      1000


struct OS_Metrics
{
    TIMER_Ticks     targetTicksCount;
    TIMER_Ticks     targetTicksNext;
    float           cyclesPerTargetTicks;
    bool            updateLastMeasures;
    TIMER_Ticks     lastMeasurementPeriod;
};


struct OS_MetricsCpu
{
    OS_Cycles       curCycles;
    uint32_t        curSwitches;
    OS_Cycles       lastCycles;
    uint32_t        lastSwitches;
    float           lastUsage;
};


struct OS_MetricsStack
{
    int32_t         curMedian;
    int32_t         curMin;
    int32_t         curMax;
    int32_t         curMeasures;
    int32_t         lastMedian;
    int32_t         lastMin;
    int32_t         lastMax;
    float           lastUsage;
};


uint32_t OS_TaskControlSize (void);
uint32_t OS_TaskStackSize   (void *taskBuffer);
uint32_t OS_TaskStackUsed   (void *taskBuffer);
uint32_t OS_TaskDataSize    (void *taskBuffer);


enum OS_Result  OS_USAGE_CpuReset               (struct OS_MetricsCpu *mc);
enum OS_Result  OS_USAGE_MemoryReset            (struct OS_MetricsStack *ms);

enum OS_Result  OS_USAGE_Init                   (struct OS_Metrics *m);
bool            OS_USAGE_UpdatingLastMeasures   (struct OS_Metrics *m);
int32_t         OS_USAGE_GetUsedTaskMemory      (void *taskBuffer);
enum OS_Result  OS_USAGE_SetTargetTicks         (struct OS_Metrics *m,
                                                 TIMER_Ticks ticks);
enum OS_Result  OS_USAGE_UpdateTarget           (struct OS_Metrics *m,
                                                 const TIMER_Ticks Now);
enum OS_Result  OS_USAGE_UpdateCurrentMeasures  (struct OS_Metrics *m,
                                                 struct OS_MetricsCpu *mc,
                                                 struct OS_MetricsStack *ms,
                                                 OS_Cycles cycles,
                                                 int32_t memory);
enum OS_Result  OS_USAGE_UpdateLastMeasures     (struct OS_Metrics *m,
                                                 struct OS_MetricsCpu *mc,
                                                 struct OS_MetricsStack *ms,
                                                 int32_t curMem,
                                                 uint32_t totalMem);
