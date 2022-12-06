/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Demo includes. */
#include "supporting_functions.h"

/* Used as a loop counter to create a very crude delay. */
#define mainDELAY_LOOP_COUNT		( 0xffffff )

/* The task function. */
void vTaskFunction( void *pvParameters );

/* Task stack for xTaskCreateStatic() */
static StackType_t xTask1Stack[ 1000 ];
static StackType_t xTask2Stack[ 1000 ];

/* Task control block for xTaskCreateStatic() */
static StaticTask_t xTask1ControlBlock;
static StaticTask_t xTask2ControlBlock;

/* Define the strings that will be passed in as the task parameters.  These are
defined const and off the stack to ensure they remain valid when the tasks are
executing. */
const char *pcTextForTask1 = "Task 1 is running";
const char *pcTextForTask2 = "Task 2 is running";

/*-----------------------------------------------------------*/

void EMBEDULAR_Main( void *param )
{
( void ) param;

    /* Create one of the two tasks. */
    xTaskCreateStatic(	vTaskFunction,			/* Pointer to the function that implements the task. */
                        "Task 1",				/* Text name for the task.  This is to facilitate debugging only. */
                        1000,			        /* Stack depth - most small microcontrollers will use much less stack than this. */
                        (void*)pcTextForTask1,	/* Pass the text to be printed in as the task parameter. */
                        1,						/* This task will run at priority 1. */
                        xTask1Stack,             /* [xTaskCreateStatic] Task stack. */
                        &xTask1ControlBlock );   /* [xTaskCreateStatic] Task control block */

    /* Create the other task in exactly the same way.  Note this time that we
    are creating the SAME task, but passing in a different parameter.  We are
    creating two instances of a single task implementation. */
    xTaskCreateStatic( vTaskFunction, "Task 2", 1000, (void*)pcTextForTask2, 1, xTask2Stack, &xTask2ControlBlock );

    /* On the embedul.ar framework, the above application entry point
       -EMBEDULAR_Main()- is executed in a task created at the end of the
       framework's initialization sequence. Thus, the framework has already
       called vTaskStartScheduler() for us. As shown in this example, the
       application task is free to create any number of additional tasks. */

    for( ;; )
    {
        /* On the embedul.ar framework, this is the main task loop. It will be
           used to check for user input through the execution of this
           example. */
        if( vExitRequested() )
        {
            /* It is good practice to destroy created tasks before exiting
               an embedul.ar framework application. */
            vTaskDelete( xTaskGetHandle( "Task 1" ) );
            vTaskDelete( xTaskGetHandle( "Task 2" ) );

            BOARD_Exit( 0 );
        }
    }
}
/*-----------------------------------------------------------*/

void vTaskFunction( void *pvParameters )
{
char *pcTaskName;
volatile uint32_t ul;

    /* The string to print out is passed in via the parameter.  Cast this to a
    character pointer. */
    pcTaskName = ( char * ) pvParameters;

    /* As per most tasks, this task is implemented in an infinite loop. */
    for( ;; )
    {
        /* Print out the name of this task. */
        vPrintString( pcTaskName );

        /* Delay for a period. */
        for( ul = 0; ul < mainDELAY_LOOP_COUNT; ul++ )
        {
            /* This loop is just a very crude delay implementation.  There is
            nothing to do in here.  Later exercises will replace this crude
            loop with a proper delay/sleep function. */
        }
    }
}


