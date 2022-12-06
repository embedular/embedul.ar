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

/* The task to be created.  Two instances of this task are created. */
static void prvPrintTask( void *pvParameters );

/* The function that uses a mutex to control access to standard out. */
static void prvNewPrintString( const char *pcString );

/* Task stack for xTaskCreateStatic() */
static StackType_t xPrint1Stack[ 1000 ];
static StackType_t xPrint2Stack[ 1000 ];

/* Task control block for xTaskCreateStatic() */
static StaticTask_t xPrint1ControlBlock;
static StaticTask_t xPrint2ControlBlock;

/*-----------------------------------------------------------*/

/* Declare a variable of type SemaphoreHandle_t.  This is used to reference the
mutex type semaphore that is used to ensure mutual exclusive access to stdout. */
SemaphoreHandle_t xMutex;

/* Semaphore buffer for xSemaphoreCreateMutexStatic() */
StaticSemaphore_t xMutexBuffer;

/* The tasks block for a pseudo random time between 0 and xMaxBlockTime ticks. */
const TickType_t xMaxBlockTimeTicks = 0x20;

void EMBEDULAR_Main( void *param )
{
( void ) param;

    /* Before a semaphore is used it must be explicitly created.  In this example
    a mutex type semaphore is created. */
    xMutex = xSemaphoreCreateMutexStatic( &xMutexBuffer );

    /* Check the semaphore was created successfully. */
    if( xMutex == NULL )
    {
        BOARD_AssertState (false);
    }

    /* Create two instances of the tasks that attempt to write stdout.  The
    string they attempt to write is passed into the task as the task's
    parameter.  The tasks are created at different priorities so some
    pre-emption will occur. */
    xTaskCreateStatic( prvPrintTask, "Print1", 1000, "Task 1 ******************************************", 1, xPrint1Stack, &xPrint1ControlBlock );
    xTaskCreateStatic( prvPrintTask, "Print2", 1000, "Task 2 ------------------------------------------", 2, xPrint2Stack, &xPrint2ControlBlock );

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

            BOARD_Exit( 0 );
        }
    }
}
/*-----------------------------------------------------------*/

static void prvNewPrintString( const char *pcString )
{
    /* The semaphore is created before the scheduler is started so already
    exists by the time this task executes.

    Attempt to take the semaphore, blocking indefinitely if the mutex is not
    available immediately.  The call to xSemaphoreTake() will only return when
    the semaphore has been successfully obtained so there is no need to check the
    return value.  If any other delay period was used then the code must check
    that xSemaphoreTake() returns pdTRUE before accessing the resource (in this
    case standard out. */
    xSemaphoreTake( xMutex, portMAX_DELAY );
    {
        /* The following line will only execute once the semaphore has been
        successfully obtained - so standard out can be accessed freely. */
        LOG( NOBJ, pcString );
    }
    xSemaphoreGive( xMutex );
}
/*-----------------------------------------------------------*/

static void prvPrintTask( void *pvParameters )
{
char *pcStringToPrint;
const TickType_t xSlowDownDelay = pdMS_TO_TICKS( 5UL );

    /* Two instances of this task are created.  The string printed by the task
    is passed into the task using the task's parameter.  The parameter is cast
    to the required type. */
    pcStringToPrint = ( char * ) pvParameters;

    for( ;; )
    {
        /* Print out the string using the newly defined function. */
        prvNewPrintString( pcStringToPrint );

        /* Wait a pseudo random time.  Note that rand() is not necessarily
        re-entrant, but in this case it does not really matter as the code does
        not care what value is returned.  In a more secure application a version
        of rand() that is known to be re-entrant should be used - or calls to
        rand() should be protected using a critical section. */
        vTaskDelay( RANDOM_GetUint32() % xMaxBlockTimeTicks );

        /* Just to ensure the scrolling is not too fast! */
        vTaskDelay( xSlowDownDelay );
    }
}



