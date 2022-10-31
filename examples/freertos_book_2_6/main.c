#include "embedul.ar/source/core/device/board.h"
#include "FreeRTOS.h"
#include "task.h"


#define STR_WelcomeMessage      "freertos_book_Example2_6 is running: PO (2 de 6)"
#define STR_TaskRunningFmt      "`0 is running"
#define STR_TaskOutputStatFmt   "`0 output `1:`2 turn `3"
#define STR_TaskBlinkStatFmt    "`0 blinking turn `1"

#define TASK_MaxInputTicks	    500
#define	TASK_MaxOutputTicks     500

// Dimensions of the buffer that the task being created will use as its stack.
// NOTE: This is the number of words the stack will hold, not the number of
// bytes. For example, if each stack item is 32-bits, and this is set to 100,
// then 400 bytes (100 * 32-bits) will be allocated.
#define TASK_StackSize          128 + 32

#define TASK_Count              3


const char *const s_TaskName[TASK_Count] =
{
    [0] = "Task1",
    [1] = "Task2",
    [2] = "Task3"
};


struct TASK_IO
{
    enum INPUT_PROFILE_Type     InputProfile;
    IO_Code                     InputCode;
    enum OUTPUT_PROFILE_Type    OutputProfile;
    IO_Code                     OutputCode;
};


const struct TASK_IO s_TaskIO[TASK_Count] =
{
    [0] = {
        .InputProfile   = INPUT_PROFILE_Type_MAIN,
        .InputCode      = INPUT_PROFILE_MAIN_Bit_A,
        .OutputProfile  = OUTPUT_PROFILE_Type_SIGN,
        .OutputCode     = OUTPUT_PROFILE_SIGN_Bit_Red
    },
    [1] = { 
        .InputProfile   = INPUT_PROFILE_Type_MAIN,
        .InputCode      = INPUT_PROFILE_MAIN_Bit_A,
        .OutputProfile  = OUTPUT_PROFILE_Type_SIGN,
        .OutputCode     = OUTPUT_PROFILE_SIGN_Bit_Green
    },
    [2] = {
        .InputProfile   = INPUT_PROFILE_Type_MAIN,
        .InputCode      = INPUT_PROFILE_MAIN_Bit_A,
        .OutputProfile  = OUTPUT_PROFILE_Type_SIGN,
        .OutputCode     = OUTPUT_PROFILE_SIGN_Bit_Blue
    },
};


// simulador pc
void vAssertCalled( const char * const pcFileName,
                    unsigned long ulLine )
{
    (void) pcFileName;
    (void) ulLine;

    BOARD_AssertState (false);
}


void vApplicationMallocFailedHook( void )
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
    vAssertCalled( __FILE__, __LINE__ );
}


void vApplicationTickHook( void )
{
    /* This function will be called by each tick interrupt if
    * configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
    * added here, but the tick hook is called from an interrupt context, so
    * code must not attempt to block, and only the interrupt safe FreeRTOS API
    * functions can be used (those that end in FromISR()). */

   // BOARD_Update ();
}


void vApplicationIdleHook( void )
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



/* When configSUPPORT_STATIC_ALLOCATION is set to 1 the application writer can
 * use a callback function to optionally provide the memory required by the idle
 * and timer tasks.  This is the stack that will be used by the timer task.  It is
 * declared here, as a global, so it can be checked by a test that is implemented
 * in a different file. */
StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];


/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 * application must provide an implementation of vApplicationGetTimerTaskMemory()
 * to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     uint32_t * pulTimerTaskStackSize )
{
/* If the buffers to be provided to the Timer task are declared inside this
 * function then they must be declared static - otherwise they will be allocated on
 * the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;

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


static void boardUpdateFunction (void *argument)
{
    (void) argument;

    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS (10);

    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount ();
    for( ;; )
    {
        BOARD_Update ();

        // Wait for the next cycle.
        BaseType_t xWasDelayed = xTaskDelayUntil( &xLastWakeTime, xFrequency );

        // Perform action here. xWasDelayed value can be used to determine
        // whether a deadline was missed if the code here took too long.
    }    
}


StackType_t boardUpdateStack[TASK_StackSize];
StaticTask_t boardUpdateControlBlock;


static void vTaskFunction (void *argument)
{
    const struct TASK_IO *const TaskIo = (struct TASK_IO *) argument;

    bool        blinking    = false;
    bool        outState    = false;
	uint32_t    outTicks    = xTaskGetTickCount ();
	uint32_t    inTicks     = xTaskGetTickCount ();

	/* Print out the name of this task. */
	taskENTER_CRITICAL();
	{
        LOG (NOBJ, STR_TaskRunningFmt, pcTaskGetName(NULL));
    }
	taskEXIT_CRITICAL();

	/* As per most tasks, this task is implemented in an infinite loop. */
	for( ;; )
	{
		/* Check Input State */
        const IO_Value InputOn =
            INPUT_GetBuffered (TaskIo->InputProfile, IO_Type_Bit,
                               TaskIo->InputCode);

        if (InputOn)
        {
			/* Delay for a period using Tick Count */
			if ((xTaskGetTickCount() - inTicks) >= TASK_MaxInputTicks)
			{
                /* Check, Update and Print Led Flag */
                blinking = blinking? false : true;

                taskENTER_CRITICAL();
                {
                    LOG (NOBJ, STR_TaskBlinkStatFmt, pcTaskGetName(NULL),
                        blinking? "On" : "Off");
                }
                taskEXIT_CRITICAL();    

				/* Update and Button Tick Counter */
        		inTicks = xTaskGetTickCount();
			}
		}

		/* Check Led Flag */
		if (blinking)
		{
			/* Delay for a period using Tick Count. */
			if( (xTaskGetTickCount() - outTicks) >= TASK_MaxOutputTicks )
			{
				/* Check, Update and Print Led State */
                outState = outState? false : true;

                taskENTER_CRITICAL();
                {
                    LOG (NOBJ, STR_TaskOutputStatFmt, pcTaskGetName(NULL),
                         OUTPUT_PROFILE_GetTypeName(TaskIo->OutputProfile),
                         OUTPUT_MappedName(TaskIo->OutputProfile, IO_Type_Bit,
                                           TaskIo->OutputCode),
                         blinking? "On" : "Off");
                }
                taskEXIT_CRITICAL();

                /* Update Output State */
                OUTPUT_SetDefer (TaskIo->OutputProfile, IO_Type_Bit,
                                 TaskIo->OutputCode, outState);

				/* Update and Led Tick Counter */
				outTicks = xTaskGetTickCount();
			}
		}
	}
}


int EMBEDULAR_Main (const int Argc, const char *const Argv[])
{
    (void) Argc;
    (void) Argv;


        xTaskCreateStatic (
            // Function that implements the task.
            boardUpdateFunction,
            // Text name for the task.
            "BoardUpdate",
            // Number of indexes in the xStack array.
            TASK_StackSize,
            // Parameter passed into the task.
            NULL,
            // Priority at which the task is created.
            tskIDLE_PRIORITY + 1,
            // Array to use as the task's stack.
            boardUpdateStack,
            // Variable to hold the task's data structure.
            &boardUpdateControlBlock);


    // Buffer that the task being created will use as its stack.
    // Note this is an array of StackType_t variables.
    // The size of StackType_t is dependent on the RTOS port.
    StackType_t taskStack[TASK_Count][TASK_StackSize];

    // Structure that will hold the TCB of the task being created.
    StaticTask_t taskControlBlock[TASK_Count];

    // Tasks handle
    TaskHandle_t taskHandle[TASK_Count];


    LOG (NOBJ, STR_WelcomeMessage);

    for (uint32_t task = 0; task < TASK_Count; ++task)
    {
        // Create the task without using any dynamic memory allocation.
        taskHandle[task] = xTaskCreateStatic (
            // Function that implements the task.
            vTaskFunction,
            // Text name for the task.
            s_TaskName[task],
            // Number of indexes in the xStack array.
            TASK_StackSize,
            // Parameter passed into the task.
            (void *)&s_TaskIO[task],
            // Priority at which the task is created.
            tskIDLE_PRIORITY,
            // Array to use as the task's stack.
            taskStack[task],
            // Variable to hold the task's data structure.
            &taskControlBlock[task]);

        // Check the task was created successfully
        BOARD_AssertState (taskHandle[task]);
    }

    vTaskStartScheduler ();

    return 0;
}
