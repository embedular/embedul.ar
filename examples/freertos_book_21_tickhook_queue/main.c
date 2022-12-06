/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that has become a de facto standard.             *
     *                                                                       *
     *    Help yourself get started quickly and support the FreeRTOS         *
     *    project by purchasing a FreeRTOS tutorial book, reference          *
     *    manual, or both from: http://www.FreeRTOS.org/Documentation        *
     *                                                                       *
     *    Thank you!                                                         *
     *                                                                       *
    ***************************************************************************

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    >>! NOTE: The modification to the GPL is included to allow you to distribute
    >>! a combined work that includes FreeRTOS without being obliged to provide
    >>! the source code for proprietary components outside of the FreeRTOS
    >>! kernel.

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available from the following
    link: http://www.freertos.org/a00114.html

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?"                                     *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org - Documentation, books, training, latest versions,
    license and Real Time Engineers Ltd. contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.OpenRTOS.com - Real Time Engineers ltd license FreeRTOS to High
    Integrity Systems to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Demo includes. */
#include "supporting_functions.h"

/* The task that sends messages to the stdio gatekeeper.  Two instances of this
task are created. */
static void prvPrintTask( void *pvParameters );

/* The gatekeeper task itself. */
static void prvStdioGatekeeperTask( void *pvParameters );

/* Task stack for xTaskCreateStatic() */
static StackType_t xPrint1Stack[ 1000 ];
static StackType_t xPrint2Stack[ 1000 ];
static StackType_t xGatekeeperStack[ 1000 ];

/* Task control block for xTaskCreateStatic() */
static StaticTask_t xPrint1ControlBlock;
static StaticTask_t xPrint2ControlBlock;
static StaticTask_t xGatekeeperControlBlock;

/* Define the strings that the tasks and interrupt will print out via the
gatekeeper. */
static const char *pcStringsToPrint[] =
{
    "Task 1 ****************************************************",
    "Task 2 ----------------------------------------------------",
    "Message printed from the tick hook interrupt ##############"
};

/*-----------------------------------------------------------*/

/* Declare a variable of type QueueHandle_t.  This is used to send messages from
the print tasks to the gatekeeper task. */
static QueueHandle_t xPrintQueue;

/* The variable used to hold the queue's data structure. */
static StaticQueue_t xPrintStaticQueue;

/* The array to use as the queue's storage area.  This must be at least
uxQueueLength * uxItemSize bytes. */
uint8_t ucPrintQueueStorageArea[ 5 * sizeof( char * ) ];

/* The tasks block for a pseudo random time between 0 and xMaxBlockTime ticks. */
const TickType_t xMaxBlockTimeTicks = 0x20;

void EMBEDULAR_Main( void *param )
{
( void ) param;

    /* Before a queue is used it must be explicitly created.  The queue is created
    to hold a maximum of 5 character pointers. */
    xPrintQueue = xQueueCreateStatic( 5, sizeof( char * ), ucPrintQueueStorageArea, &xPrintStaticQueue );

    /* Check the queue was created successfully. */
    if( xPrintQueue == NULL )
    {
        BOARD_AssertState (false);
    }

    /* Create two instances of the tasks that send messages to the gatekeeper.
    The	index to the string they attempt to write is passed in as the task
    parameter (4th parameter to xTaskCreate()).  The tasks are created at
    different priorities so some pre-emption will occur. */
    xTaskCreateStatic( prvPrintTask, "Print1", 1000, ( void * ) 0, 1, xPrint1Stack, &xPrint1ControlBlock );
    xTaskCreateStatic( prvPrintTask, "Print2", 1000, ( void * ) 1, 2, xPrint2Stack, &xPrint2ControlBlock );

    /* Create the gatekeeper task.  This is the only task that is permitted
    to access standard out. */
    xTaskCreateStatic( prvStdioGatekeeperTask, "Gatekeeper", 1000, NULL, 0, xGatekeeperStack, &xGatekeeperControlBlock );

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
            vTaskDelete( xTaskGetHandle( "Print1" ) );
            vTaskDelete( xTaskGetHandle( "Print2" ) );
            vTaskDelete( xTaskGetHandle( "Gatekeeper" ) );

            BOARD_Exit( 0 );
        }
    }
}
/*-----------------------------------------------------------*/

static void prvStdioGatekeeperTask( void *pvParameters )
{
( void ) pvParameters;
char *pcMessageToPrint;

    /* This is the only task that is allowed to write to the terminal output.
    Any other task wanting to write to the output does not access the terminal
    directly, but instead sends the output to this task.  As only one task
    writes to standard out there are no mutual exclusion or serialization issues
    to consider within this task itself. */
    for( ;; )
    {
        /* Wait for a message to arrive. */
        xQueueReceive( xPrintQueue, &pcMessageToPrint, portMAX_DELAY );

        /* There is no need to check the return	value as the task will block
        indefinitely and only run again when a message has arrived.  When the
        next line is executed there will be a message to be output. */
        LOG( NOBJ, pcMessageToPrint );

        /* Now simply go back to wait for the next message. */
    }
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
static int iCount = 0;

    /* Print out a message every 200 ticks.  The message is not written out
    directly, but sent to the gatekeeper task. */
    iCount++;
    if( iCount >= 200 )
    {
        /* In this case the last parameter (xHigherPriorityTaskWoken) is not
        actually used and is set to NULL. */
        xQueueSendToFrontFromISR( xPrintQueue, &( pcStringsToPrint[ 2 ] ), NULL );

        /* Reset the count ready to print out the string again in 200 ticks
        time. */
        iCount = 0;
    }
}
/*-----------------------------------------------------------*/

static void prvPrintTask( void *pvParameters )
{
int iIndexToString;

    /* Two instances of this task are created so the index to the string the task
    will send to the gatekeeper task is passed in the task parameter.  Cast this
    to the required type. */
    iIndexToString = ( int ) ( uintptr_t) pvParameters;

    for( ;; )
    {
        /* Print out the string, not directly but by passing the string to the
        gatekeeper task on the queue.  The queue is created before the scheduler is
        started so will already exist by the time this task executes.  A block time
        is not specified as there should always be space in the queue. */
        xQueueSendToBack( xPrintQueue, &( pcStringsToPrint[ iIndexToString ] ), 0 );

        /* Wait a pseudo random time.  Note that rand() is not necessarily
        re-entrant, but in this case it does not really matter as the code does
        not care what value is returned.  In a more secure application a version
        of rand() that is known to be re-entrant should be used - or calls to
        rand() should be protected using a critical section. */
        vTaskDelay( ( RANDOM_GetUint32() % xMaxBlockTimeTicks ) );
    }
}


