/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [OSWRAP driver] freertos.

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

#include "embedul.ar/source/core/device/oswrap.h"
#include "embedul.ar/source/core/device/board.h"
#include "FreeRTOS.h"
#include "portmacro.h"
#include "task.h"


static TIMER_TickHookFunc s_tickHook = NULL;


// Dimensions of the buffer that the task being created will use as its stack.
// NOTE: This is the number of words the stack will hold, not the number of
// bytes. For example, if each stack item is 32-bits, and this is set to 100,
// then 400 bytes (100 * 32-bits) will be allocated.
#ifndef OSWRAP_FREERTOS_RUN_TASK_STACK_WORDS
    #define RUN_TASK_STACK_WORDS    2048
#else
    #define RUN_TASK_STACK_WORDS    OSWRAP_FREERTOS_RUN_TASK_STACK_WORDS
#endif

#ifndef OSWRAP_FREERTOS_MAIN_TASK_STACK_WORDS
    #define MAIN_TASK_STACK_WORDS   2048
#else 
    #define MAIN_TASK_STACK_WORDS   OSWRAP_FREERTOS_MAIN_TASK_STACK_WORDS
#endif

// RunTask and MainTask run at the highest priority available.
#define RUN_TASK_PRIORITY           (configMAX_PRIORITIES - 1)
#define MAIN_TASK_PRIORITY          (configMAX_PRIORITIES - 1)


struct OSWRAP_FREERTOS
{
    struct OSWRAP   device;
    TaskHandle_t    runTaskHandle;
    TaskHandle_t    mainTaskHandle;
    TickType_t      runTaskLastWakeTime;
    TickType_t      runTaskTimerSyncTicks;
};


static struct OSWRAP_FREERTOS s_oswrap_freertos;


#ifdef BSS_SECTION_OSWRAP_FREERTOS_RUN_TASK_STACK
BSS_SECTION_OSWRAP_FREERTOS_RUN_TASK_STACK
#endif
StackType_t     s_runTaskStack[RUN_TASK_STACK_WORDS];

#ifdef BSS_SECTION_OSWRAP_FREERTOS_MAIN_TASK_STACK
BSS_SECTION_OSWRAP_FREERTOS_MAIN_TASK_STACK
#endif
StackType_t     s_mainTaskStack[MAIN_TASK_STACK_WORDS];

#ifdef BSS_SECTION_OSWRAP_FREERTOS_RUN_MAIN_TCB
BSS_SECTION_OSWRAP_FREERTOS_RUN_MAIN_TCB
#endif
StaticTask_t    s_runTaskControlBlock;

#ifdef BSS_SECTION_OSWRAP_FREERTOS_RUN_MAIN_TCB
BSS_SECTION_OSWRAP_FREERTOS_RUN_MAIN_TCB
#endif
StaticTask_t    s_mainTaskControlBlock;


static void         summary                 (struct OSWRAP *const O);
static void         createRunTaskAndStart   (struct OSWRAP *const O,
                                             const OSWRAP_TaskFunc RunTask,
                                             struct BOARD *const B);
static void         createMainTask          (struct OSWRAP *const O,
                                             const OSWRAP_TaskFunc MainTask);
static void         runTaskSyncLock         (struct OSWRAP *const O);
static bool         syncUnlock              (struct OSWRAP *const O);
static void         closeMainTask           (struct OSWRAP *const O);
static void         closeRunTaskAndEnd      (struct OSWRAP *const O);
static void         suspendScheduler        (struct OSWRAP *const O);
static void         resumeScheduler         (struct OSWRAP *const O);
static TIMER_TickHookFunc
                    setTickHook             (struct OSWRAP *const O,
                                             const TIMER_TickHookFunc Hook);
static TIMER_Ticks
                    ticksNow                (struct OSWRAP *const O);
static void         delay                   (struct OSWRAP *const O,
                                             const TIMER_Ticks Ticks);
static const char * taskName                (struct OSWRAP *const O);


static const struct OSWRAP_IFACE OSWRAP_FREERTOS_IFACE =
{
    .Description            = "freertos",
    .Summary                = summary,
    .CreateRunTaskAndStart  = createRunTaskAndStart,
    .CreateMainTask         = createMainTask,
    .RunTaskSyncLock        = runTaskSyncLock,
    .SyncUnlock             = syncUnlock,
    .CloseMainTask          = closeMainTask,
    .CloseRunTaskAndEnd     = closeRunTaskAndEnd,
    .SuspendScheduler       = suspendScheduler,
    .ResumeScheduler        = resumeScheduler,
    .SetTickHook            = setTickHook,
    .TicksNow               = ticksNow,
    .Delay                  = delay,
    .TaskName               = taskName
};


void OSWRAP__boot (void)
{
    OSWRAP_Init ((struct OSWRAP *)&s_oswrap_freertos, &OSWRAP_FREERTOS_IFACE);
}


void vAssertCalled (const char * const pcFileName,
                    unsigned long ulLine)
{
    (void) pcFileName;
    (void) ulLine;

    LOG_Warn (NOBJ, "pcFilename: `0, ulLine: `1", pcFileName, (uint64_t)ulLine);

    BOARD_AssertState (false);
}


#if (configCHECK_FOR_STACK_OVERFLOW != 0)
void vApplicationStackOverflowHook (TaskHandle_t xTask, char *pcTaskName)
{
    (void) xTask;
    (void) pcTaskName;

    BOARD_AssertState (false);
}
#endif


#if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
void vApplicationMallocFailedHook (void)
{
    /* vApplicationMallocFailedHook() will only be called if
     * configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
     * function that will get called if a call to pvPortMalloc() fails.
     * pvPortMalloc() is called internally by the kernel whenever a task, queue,
     * timer or semaphore is created.  It is also called by various parts of the
     * demo application.  If heap_1.c, heap_2.c or heap_4.c is being used, then the
     * size of the    heap available to pvPortMalloc() is defined by
     * configTOTAL_HEAP_SIZE in FreeRTOSConfig.h, and the xPortGetFreeHeapSize()
     * API function can be used to query the size of free heap space that remains
     * (although it does not provide information on how the remaining heap might be
     * fragmented).  See http://www.freertos.org/a00111.html for more
     * information. */
    vAssertCalled (__FILE__, __LINE__);
}
#endif


// Overriden by a FreeRTOS application and in effect after calling
// OSWRAP_EnableAltTickHook().
// One may obfuscate the fact that this callback is called
// vApplicationTickHook_2 by using
// #define vApplicationTickHook vApplicationTickHook_2.
#if (configUSE_TICK_HOOK == 1)
CC_Weak void vApplicationTickHook_2 (void)
{
}


void vApplicationTickHook (void)
{
    /* This function will be called by each tick interrupt if
    * configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
    * added here, but the tick hook is called from an interrupt context, so
    * code must not attempt to block, and only the interrupt safe FreeRTOS API
    * functions can be used (those that end in FromISR()). */

    if (s_tickHook)
    {
        s_tickHook (xTaskGetTickCountFromISR());
    }

    if (s_oswrap_freertos.device.enableAltTickHook)
    {
        vApplicationTickHook_2 ();
    }
}
#endif


#if (configUSE_IDLE_HOOK == 1)
CC_Weak void vApplicationIdleHook (void)
{
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
     * to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
     * task.  It is essential that code added to this hook function never attempts
     * to block in any way (for example, call xQueueReceive() with a block time
     * specified, or call vTaskDelay()).  If application tasks make use of the
     * vTaskDelete() API function to delete themselves then it is also important
     * that vApplicationIdleHook() is permitted to return to its calling function,
     * because it is the responsibility of the idle task to clean up memory
     * allocated by the kernel to any task that has since deleted itself. */
}
#endif

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 * used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    uint32_t * pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
 * function then they must be declared static - otherwise they will be allocated on
 * the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
     * state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}


#if (configUSE_TIMERS == 1)
/* When configSUPPORT_STATIC_ALLOCATION is set to 1 the application writer can
 * use a callback function to optionally provide the memory required by the idle
 * and timer tasks.  This is the stack that will be used by the timer task.  It is
 * declared here, as a global, so it can be checked by a test that is implemented
 * in a different file. */
static StackType_t uxTimerTaskStack [configTIMER_TASK_STACK_DEPTH];
static StaticTask_t xTimerTaskTCB;


/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 * application must provide an implementation of vApplicationGetTimerTaskMemory()
 * to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory (StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     uint32_t * pulTimerTaskStackSize)
{
    /* Pass out a pointer to the StaticTask_t structure in which the Timer
     * task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
#endif


static void summaryTaskStack (struct OSWRAP *const O,
                         const char *const TaskName,
                         const uint32_t StackWords)
{
    LOG_AutoContext (O, "task: `0", TaskName);

    LOG_Items (2, 
                    "words",    StackWords,
                    "octets",   StackWords * sizeof(StackType_t));
}


static void summary (struct OSWRAP *const O)
{
    {
        LOG_AutoContext (O, "stack");

        LOG_Items (1, "word size (in octets)", (uint32_t)sizeof(StackType_t));

        summaryTaskStack (O, "run", RUN_TASK_STACK_WORDS);
        summaryTaskStack (O, "main", MAIN_TASK_STACK_WORDS);
    }
}


static void createRunTaskAndStart (struct OSWRAP *const O,
                                   const OSWRAP_TaskFunc RunTask,
                                   struct BOARD *const B)
{
    struct OSWRAP_FREERTOS *const F = (struct OSWRAP_FREERTOS *) O;

    F->runTaskHandle = xTaskCreateStatic (RunTask,
                                          "RunTask",
                                          RUN_TASK_STACK_WORDS, B,
                                          RUN_TASK_PRIORITY,
                                          s_runTaskStack,
                                          &s_runTaskControlBlock);

    BOARD_AssertState (F->runTaskHandle);

    F->runTaskLastWakeTime   = xTaskGetTickCount ();
    F->runTaskTimerSyncTicks = pdMS_TO_TICKS (O->autoSyncPeriod_ms);

    vTaskStartScheduler ();
}


static void createMainTask (struct OSWRAP *const O,
                            const OSWRAP_TaskFunc MainTask)
{
    struct OSWRAP_FREERTOS *const F = (struct OSWRAP_FREERTOS *) O;

    F->mainTaskHandle = xTaskCreateStatic (MainTask,
                                          "MainTask",
                                          MAIN_TASK_STACK_WORDS, NULL,
                                          MAIN_TASK_PRIORITY,
                                          s_mainTaskStack,
                                          &s_mainTaskControlBlock);

    BOARD_AssertState (F->mainTaskHandle);
}


static void runTaskSyncLock (struct OSWRAP *const O)
{
    struct OSWRAP_FREERTOS *const F = (struct OSWRAP_FREERTOS *) O;

    // TODO: Avoid being delayed or forever suspended if the MainTask was
    // closed in between the evaluation of B->exit and here.
    if (O->autoSyncPeriod_ms)
    {
        F->runTaskTimerSyncTicks = pdMS_TO_TICKS (O->autoSyncPeriod_ms);
        xTaskDelayUntil (&F->runTaskLastWakeTime, F->runTaskTimerSyncTicks);
    }
    else 
    {
        vTaskSuspend (F->runTaskHandle);
    }
}


static bool syncUnlock (struct OSWRAP *const O)
{
    struct OSWRAP_FREERTOS *const F = (struct OSWRAP_FREERTOS *) O;

    if (xTaskGetCurrentTaskHandle() == F->runTaskHandle)
    {
        // Always execute BOARD_Sync() when called from runTask.
        return true;
    }
    else 
    {
        // Not called from runTask. BOARD_Sync() can only be called
        // from runTask. Since its execution is locked (either suspended or
        // waiting for a timer) it is now unlocked at the caller's request
        // to perform an inmediate single call to BOARD_Sync().
        //
        // Note that BOARD_Sync() runs inside a critical section: the
        // scheduler is suspended thourough the entire execution of
        // BOARD_Sync(). That means no other task could be called while
        // sync is in progress.
        if (O->autoSyncPeriod_ms)
        {
            xTaskAbortDelay (F->runTaskHandle);
        }
        else
        {
            vTaskResume (F->runTaskHandle);
        }

        return false;
    }
}


static void closeMainTask (struct OSWRAP *const O)
{
    struct OSWRAP_FREERTOS *const F = (struct OSWRAP_FREERTOS *) O;

    vTaskDelete (F->mainTaskHandle);

    F->mainTaskHandle = NULL;
}


static void closeRunTaskAndEnd (struct OSWRAP *const O)
{
#if 0
    // On platforms where vTaskEndScheduler is unsupported, we should delete the
    // current task (the one executing this code) without further action.
    // However, on an embedded platform, boardShutdownSequence() might have shut
    // off the hardware, so this might never get executed.
    
    struct OSWRAP_FREERTOS *const F = (struct OSWRAP_FREERTOS *) O;

    vTaskDelete (F->runTaskHandle);

    F->runTaskHandle = NULL;
#else
    (void) O;
    // This function should delete all pending tasks (likely the one executing
    // this code, RunTask) and close the FreeRTOS scheduler to keep executing
    // from after the scheduler was started. Works perfect when the target is a
    // posix (linux) simulator.
    vTaskEndScheduler ();
#endif
}


static void suspendScheduler (struct OSWRAP *const O)
{
    (void) O;

    vTaskSuspendAll ();
}


static void resumeScheduler (struct OSWRAP *const O)
{
    (void) O;

    xTaskResumeAll ();
}


static TIMER_TickHookFunc setTickHook (struct OSWRAP *const O,
                                       const TIMER_TickHookFunc Hook)
{
    (void) O;

    const TIMER_TickHookFunc LastHook = s_tickHook;

    s_tickHook = Hook;

    return LastHook;
}


static TIMER_Ticks ticksNow (struct OSWRAP *const O)
{
    (void) O;

    return xTaskGetTickCount ();
}


static void delay (struct OSWRAP *const O,
                   const TIMER_Ticks Ticks)
{
    (void) O;

    vTaskDelay ((TickType_t)Ticks);
}


static const char * taskName (struct OSWRAP *const O)
{
    (void) O;

    return pcTaskGetName (NULL);
}
