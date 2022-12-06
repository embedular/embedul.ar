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

/* The two task functions. */
void vTask1( void *pvParameters );
void vTask2( void *pvParameters );

/* Task stack for xTaskCreateStatic() */
static StackType_t xTask1Stack[ 1000 ];
static StackType_t xTask2Stack[ 1000 ];

/* Task control block for xTaskCreateStatic() */
static StaticTask_t xTask1ControlBlock;
static StaticTask_t xTask2ControlBlock;

/* Used to hold the handle of Task2. */
TaskHandle_t xTask2Handle;

/*-----------------------------------------------------------*/

void EMBEDULAR_Main( void *param )
{
( void ) param;

	/* Create the first task at priority 1.  This time the task parameter is
	not used and is set to NULL. */
	xTaskCreateStatic( vTask1, "Task 1", 1000, NULL, 1, xTask1Stack, &xTask1ControlBlock );
                /* The task is created at priority 1 ^. */

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
            /* Task 2 deletes itself. */

            BOARD_Exit( 0 );
        }
    }
}
/*-----------------------------------------------------------*/

void vTask1( void *pvParameters )
{
( void ) pvParameters;
const TickType_t xDelay100ms = pdMS_TO_TICKS( 100UL );

	for( ;; )
	{
		/* Print out the name of this task. */
		vPrintString( "Task1 is running" );

		/* Create task 2 at a higher priority.  Again the task parameter is not 
		used so is set to NULL. */
		xTask2Handle = xTaskCreateStatic( vTask2, "Task 2", 1000, NULL, 2, xTask2Stack, &xTask2ControlBlock );
		/* The task handle is returned by xTaskCreateStatic() */

		/* Task2 has/had the higher priority, so for Task1 to reach here Task2
		must have already executed and deleted itself.  Delay for 100 
		milliseconds. */
		vTaskDelay( xDelay100ms );
	}
}

/*-----------------------------------------------------------*/

void vTask2( void *pvParameters )
{
( void ) pvParameters;

	/* Task2 does nothing but delete itself.  To do this it could call vTaskDelete()
	using a NULL parameter, but instead and purely for demonstration purposes it
	instead calls vTaskDelete() with its own task handle. */
	vPrintString( "Task2 is running and about to delete itself" );
	vTaskDelete( xTask2Handle );
}
/*-----------------------------------------------------------*/




