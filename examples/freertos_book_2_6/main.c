#include "embedul.ar/source/core/main.h"
#include "source/core/device/oswrap.h"
#include "source/core/manager/input/profile.h"
#include "source/core/manager/input/profile/main.h"
#include "source/core/manager/log.h"
#include "FreeRTOS.h"
#include "task.h"


#define STR_WelcomeMessage      "freertos_book_Example2_6 is running: PO (2 de 6)"
#define STR_TaskRunningFmt      "`0 is running"
#define STR_TaskOutputStatFmt   "`0 output '`1:`2' turn `3"
#define STR_TaskBlinkStatFmt    "`0 blinking turn `1"

#define TASK_MaxInputTicks      500
#define	TASK_MaxOutputTicks     500

// Dimensions of the buffer that the task being created will use as its stack.
// NOTE: This is the number of words the stack will hold, not the number of
// bytes. For example, if each stack item is 32-bits, and this is set to 100,
// then 400 bytes (100 * 32-bits) will be allocated.
#define TASK_StackSize          1024 + 32

#define TASK_Count              3


// Buffer that the task being created will use as its stack.
// Note this is an array of StackType_t variables.
// The size of StackType_t is dependent on the RTOS port.
static StackType_t s_taskStack[TASK_Count][TASK_StackSize];

// Structure that will hold the TCB of the task being created.
static StaticTask_t s_taskControlBlock[TASK_Count];

// Tasks handle
static TaskHandle_t s_taskHandle[TASK_Count];


static const char *const s_TaskName[TASK_Count] =
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


static void vTaskFunction (void *argument)
{
    const struct TASK_IO *const TaskIo = (struct TASK_IO *) argument;

    bool        blinking    = false;
    bool        outState    = false;
    uint32_t    outTicks    = xTaskGetTickCount ();
    uint32_t    inTicks     = xTaskGetTickCount ();

    /* Print out the name of this task. */
    LOG (NOBJ, STR_TaskRunningFmt, pcTaskGetName(NULL));

    /* As per most tasks, this task is implemented in an infinite loop. */
    for( ;; )
    {
        /* Check Input State */
        const IO_Value InputEnable =
            INPUT_GetBuffered (TaskIo->InputProfile, IO_Type_Bit,
                               TaskIo->InputCode);

        if (InputEnable)
        {
            /* Delay for a period using Tick Count */
            if ((xTaskGetTickCount() - inTicks) >= TASK_MaxInputTicks)
            {
                /* Check, Update and Print Led Flag */
                blinking = blinking? false : true;

                LOG (NOBJ, STR_TaskBlinkStatFmt, pcTaskGetName(NULL),
                     blinking? "On" : "Off"); 

                /* Update and Button Tick Counter */
                inTicks = xTaskGetTickCount();

                taskYIELD ();
            }
        }

        /* Check Led Flag */
        if (blinking)
        {
            /* Delay for a period using Tick Count. */
            if ((xTaskGetTickCount() - outTicks) >= TASK_MaxOutputTicks)
            {
                /* Check, Update and Print Led State */
                outState = outState? false : true;

                LOG (NOBJ, STR_TaskOutputStatFmt, pcTaskGetName(NULL),
                     OUTPUT_PROFILE_GetTypeName(TaskIo->OutputProfile),
                     OUTPUT_MappedName(TaskIo->OutputProfile, IO_Type_Bit,
                                       TaskIo->OutputCode),
                        outState? "On" : "Off");

                /* Update Output State */
                OUTPUT_SetDefer (TaskIo->OutputProfile, IO_Type_Bit,
                                 TaskIo->OutputCode, outState);

                /* Update and Led Tick Counter */
                outTicks = xTaskGetTickCount();
            }
        }
    }
}


void EMBEDULAR_Main (void *param)
{
    (void) param;

    LOG (NOBJ, STR_WelcomeMessage);


    // Create three tasks with the same function but triggering corresponding
    // outputs from given inputs, as defined in s_TaskIO[0], [1], and [2].
    for (uint32_t task = 0; task < TASK_Count; ++task)
    {
        // Create the task without using any dynamic memory allocation.
        s_taskHandle[task] = xTaskCreateStatic (
            // Function that implements the task.
            vTaskFunction,
            // Text name for the task.
            s_TaskName[task],
            // Number of indexes in the xStack array.
            TASK_StackSize,
            // Parameter passed into the task.
            (void *)&s_TaskIO[task],
            // Priority at which the task is created.
            tskIDLE_PRIORITY + 1,
            // Array to use as the task's stack.
            s_taskStack[task],
            // Variable to hold the task's data structure.
            &s_taskControlBlock[task]);

        // Check the task was created successfully
        BOARD_AssertState (s_taskHandle[task]);
    }

    while (1)
    {
        if (INPUT_GetBuffered (INPUT_PROFILE_Type_MAIN, IO_Type_Bit,
                               INPUT_PROFILE_MAIN_Bit_B))
        {
            LOG_ContextBegin (NOBJ, "Exit requested");
            {
                for (uint32_t task = 0; task < TASK_Count; ++task)
                {
                    LOG_PendingBegin (NOBJ, "Deleting task `0",
                                      s_TaskName[task]);

                    vTaskDelete (s_taskHandle[task]);

                    LOG_PendingEndOk ();
                }

                LOG_Warn (NOBJ, "Exiting the example");
            }
            LOG_ContextEnd ();

            BOARD_Exit (0);
        }
    }
}
