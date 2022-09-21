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

#include "embedul.ar/source/core/retros/private/metrics.h"
#include "embedul.ar/source/core/retros/private/opaque.h"
#include "embedul.ar/source/core/device/board.h"

#include <string.h>

// Cortex-M only
#include "cmsis.h"


extern uint32_t SystemCoreClock;


uint32_t OS_TaskControlSize (void)
{
    return sizeof (struct OS_TaskControl);
}


uint32_t OS_TaskStackSize (void *taskBuffer)
{
    BOARD_AssertParams (OS_TaskBufferSanityTest(taskBuffer));

    struct OS_TaskControl *tc = (struct OS_TaskControl *) taskBuffer;
    const uint32_t StackSize = tc->stackTop - sizeof (struct OS_TaskControl);

    return StackSize;
}


uint32_t OS_TaskStackUsed (void *taskBuffer)
{
    BOARD_AssertParams (OS_TaskBufferSanityTest(taskBuffer));

    struct OS_TaskControl *tc = (struct OS_TaskControl *) taskBuffer;
    const uint32_t StackUsed = (((uint32_t)taskBuffer) + tc->stackTop) - tc->sp;

    return StackUsed;
}


uint32_t OS_TaskDataSize (void *taskBuffer)
{
    BOARD_AssertParams (OS_TaskBufferSanityTest(taskBuffer));

    struct OS_TaskControl *tc = (struct OS_TaskControl *) taskBuffer;
    const uint32_t DataSize = tc->size - tc->stackTop;

    return DataSize;
}


inline static void updateCurrentCpu (struct OS_MetricsCpu *mc,
                                     const OS_Cycles Cycles)
{
    mc->curCycles += Cycles;
    ++ mc->curSwitches;
}


inline static void updateCurrentMemory (struct OS_MetricsStack *ms,
                                        const int32_t CurMem)
{
    ms->curMedian += CurMem;

    if (ms->curMin > CurMem)
    {
        ms->curMin = CurMem;
    }

    if (ms->curMax < CurMem)
    {
        ms->curMax = CurMem;
    }

    ++ ms->curMeasures;
}


inline static void updateLastCpu (struct OS_Metrics *m,
                                  struct OS_MetricsCpu *mc)
{
    mc->lastUsage       = mc->curCycles * m->cyclesPerTargetTicks;
    mc->lastCycles      = mc->curCycles;
    mc->lastSwitches    = mc->curSwitches;
    mc->curCycles       = 0;
    mc->curSwitches     = 0;
}


inline static void updateLastMemory (struct OS_MetricsStack *ms,
                                     const int32_t CurMem,
                                     const uint32_t TotalMem)
{
    if (ms->curMeasures)
    {
        ms->lastMedian  = ms->curMedian / ms->curMeasures;
        ms->lastMin     = ms->curMin;
        ms->lastMax     = ms->curMax;
    }
    else
    {
        ms->lastMedian  = CurMem;
        ms->lastMin     = CurMem;
        ms->lastMax     = CurMem;
    }

    ms->lastUsage   = (float)ms->lastMedian / (float)TotalMem;

    ms->curMedian   = 0;
    ms->curMin      = INT32_MAX;
    ms->curMax      = INT32_MIN;
    ms->curMeasures = 0;
}


enum OS_Result OS_USAGE_CpuReset (struct OS_MetricsCpu *mc)
{
    if (!mc)
    {
        return OS_Result_InvalidParams;
    }

    ZERO_MEMORY (mc);

    return OS_Result_OK;
}


enum OS_Result OS_USAGE_MemoryReset (struct OS_MetricsStack *ms)
{
    if (!ms)
    {
        return OS_Result_InvalidParams;
    }

    ZERO_MEMORY (ms);

    ms->curMin  = INT32_MAX;
    ms->curMax  = INT32_MIN;

    return OS_Result_OK;
}


enum OS_Result OS_USAGE_Init (struct OS_Metrics *m)
{
    if (!m)
    {
        return OS_Result_InvalidParams;
    }

    ZERO_MEMORY (m);

    m->targetTicksCount = OS_UsageDefaultTargetTicks;

    return OS_Result_OK;
}


bool OS_USAGE_UpdatingLastMeasures (struct OS_Metrics *m)
{
    if (!m)
    {
        return false;
    }

    return m->updateLastMeasures;
}


// Returns memory used by a task, including static amount reserved for its
// control structure.
int32_t OS_USAGE_GetUsedTaskMemory (void *taskBuffer)
{
    if (!taskBuffer)
    {
        return 0;
    }

    struct OS_TaskControl *task = (struct OS_TaskControl *) taskBuffer;

    const int32_t Used = ((int32_t)task->size)
                            - ((int32_t)task->sp - (int32_t)taskBuffer)
                            + (int32_t)sizeof(struct OS_TaskControl);

    // Must at least reflect the memory usage of the task control structure.
    BOARD_AssertState (Used >= (int32_t)sizeof(struct OS_TaskControl));

    return Used;
}


enum OS_Result OS_USAGE_SetTargetTicks (struct OS_Metrics *m,
                                        TIMER_Ticks ticks)
{
    if (!m)
    {
        return OS_Result_InvalidParams;
    }

    m->targetTicksCount = ticks;

    return OS_Result_OK;
}


enum OS_Result OS_USAGE_UpdateTarget (struct OS_Metrics *m,
                                      const TIMER_Ticks Now)
{
    if (!m)
    {
        return OS_Result_InvalidParams;
    }

    m->updateLastMeasures = false;

    // TargetTicksNext still in the future
    if (m->targetTicksNext > Now)
    {
        return OS_Result_OK;
    }

    // Update last measure only if this is not the first call to
    // UpdateTarget (no last measure to update then)
    if (m->targetTicksNext)
    {
        m->updateLastMeasures = true;
    }

    const TIMER_Ticks CountDiff = Now - m->targetTicksNext;
    m->cyclesPerTargetTicks  = 1.0f
                        / ((float) SystemCoreClock
                        / (float) (m->targetTicksCount + CountDiff)
                        * 1000.0f / BOARD_TickPeriod ());
    // Try to keep discrete-time measurements
    m->targetTicksNext = Now + m->targetTicksCount - CountDiff;

    // IMPORTANT: trace logs will need to know the exact timestamp of when
    // measurements were taken.
    m->lastMeasurementPeriod = Now;

    return OS_Result_OK;
}


// This should only be used with the currently RUNNING task (the one running
// when the scheduler took over)
enum OS_Result OS_USAGE_UpdateCurrentMeasures (struct OS_Metrics *m,
                                               struct OS_MetricsCpu *mc,
                                               struct OS_MetricsStack *ms,
                                               OS_Cycles cycles, int32_t memory)
{
    if (!m)
    {
        return OS_Result_InvalidParams;
    }

    if (mc)
    {
        updateCurrentCpu (mc, cycles);
    }

    if (ms)
    {
        updateCurrentMemory (ms, memory);
    }

    return OS_Result_OK;
}


enum OS_Result OS_USAGE_UpdateLastMeasures (struct OS_Metrics *m,
                                            struct OS_MetricsCpu *mc,
                                            struct OS_MetricsStack *ms,
                                            int32_t curMem, uint32_t totalMem)
{
    if (!m)
    {
        return OS_Result_InvalidParams;
    }

    if (!m->updateLastMeasures)
    {
        return OS_Result_InvalidOperation;
    }

    if (mc)
    {
        updateLastCpu (m, mc);
    }

    if (ms)
    {
        updateLastMemory (ms, curMem, totalMem);
    }

    return OS_Result_OK;
}
