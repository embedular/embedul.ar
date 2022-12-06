/*
 * FreeRTOS V202112.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#pragma once

/*-----------------------------------------------------------
* Application specific definitions.
*
* These definitions should be adjusted for your particular hardware and
* application requirements.
*
* THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
* FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.  See
* https://www.FreeRTOS.org/a00110.html
*----------------------------------------------------------*/

#define configMINIMAL_STACK_SIZE                   PTHREAD_STACK_MIN /* The stack size being passed is equal to the minimum stack size needed by pthread_create(). */

#define configUSE_TRACE_FACILITY                   1

/* Run time stats gathering configuration options. */
unsigned long ulGetRunTimeCounterValue( void ); /* Prototype of function that returns run time counter. */
void vConfigureTimerForRunTimeStats( void );    /* Prototype of function that initialises the run time counter. */
#define configGENERATE_RUN_TIME_STATS               1

/* This demo can use of one or more example stats formatting functions.  These
 * format the raw data provided by the uxTaskGetSystemState() function in to human
 * readable ASCII form.  See the notes in the implementation of vTaskList() within
 * FreeRTOS/Source/tasks.c for limitations. */
#define configUSE_STATS_FORMATTING_FUNCTIONS        0

#define configINCLUDE_MESSAGE_BUFFER_AMP_DEMO       0
#if ( configINCLUDE_MESSAGE_BUFFER_AMP_DEMO == 1 )
    extern void vGenerateCoreBInterrupt( void * xUpdatedMessageBuffer );
    #define sbSEND_COMPLETED( pxStreamBuffer )    vGenerateCoreBInterrupt( pxStreamBuffer )
#endif /* configINCLUDE_MESSAGE_BUFFER_AMP_DEMO */

extern void vAssertCalled( const char * const pcFileName,
                           unsigned long ulLine );

/* projCOVERAGE_TEST should be defined on the command line so this file can be
 * used with multiple project configurations.  If it is
 */
#ifndef projCOVERAGE_TEST
    #error projCOVERAGE_TEST should be defined to 1 or 0 on the command line.
#endif

#if ( projCOVERAGE_TEST == 1 )

/* Insert NOPs in empty decision paths to ensure both true and false paths
 * are being tested. */
    #define mtCOVERAGE_TEST_MARKER()    __asm volatile ( "NOP" )

/* Ensure the tick count overflows during the coverage test. */
    #define configINITIAL_TICK_COUNT        0xffffd800UL

/* Allows tests of trying to allocate more than the heap has free. */
//    #define configUSE_MALLOC_FAILED_HOOK    0

/* To test builds that remove the static qualifier for debug builds. */
    #define portREMOVE_STATIC_QUALIFIER
#else /* if ( projCOVERAGE_TEST == 1 ) */

//    #define configUSE_MALLOC_FAILED_HOOK    1

/* Include the FreeRTOS+Trace FreeRTOS trace macro definitions. */
 //   #include "trcRecorder.h"
#endif /* if ( projCOVERAGE_TEST == 1 ) */

/* networking definitions */
#define configMAC_ISR_SIMULATOR_PRIORITY    ( configMAX_PRIORITIES - 1 )

/* Prototype for the function used to print out.  In this case it prints to the
 * console before the network is connected then a UDP port after the network has
 * connected. */
extern void vLoggingPrintf( const char * pcFormatString,
                            ... );

/* Set to 1 to print out debug messages.  If ipconfigHAS_DEBUG_PRINTF is set to
 * 1 then FreeRTOS_debug_printf should be defined to the function used to print
 * out the debugging messages. */
#define ipconfigHAS_DEBUG_PRINTF    1
#if ( ipconfigHAS_DEBUG_PRINTF == 1 )
    #define FreeRTOS_debug_printf( X )    vLoggingPrintf X
#endif

/* Set to 1 to print out non debugging messages, for example the output of the
 * FreeRTOS_netstat() command, and ping replies.  If ipconfigHAS_PRINTF is set to 1
 * then FreeRTOS_printf should be set to the function used to print out the
 * messages. */
#define ipconfigHAS_PRINTF    0
#if ( ipconfigHAS_PRINTF == 1 )
    #define FreeRTOS_printf( X )    vLoggingPrintf X
#endif
