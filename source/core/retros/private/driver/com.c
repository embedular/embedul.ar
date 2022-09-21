#include "com.h"
#include "../opaque.h"
#include "../../scheduler.h"
#include "../../../base/debug.h"
#include <string.h>


inline static struct OS_DRIVER_ComUART * getComUart (
                                            struct OS_TaskControl *task)
{
    return (-- ((struct OS_DRIVER_ComUART *) &((uint8_t *)task)[task->size]));
}


inline static enum OS_DRIVER_ComType getComType (struct OS_TaskControl *task)
{
    return *( -- ((enum OS_DRIVER_ComType *)
                                    &((uint8_t *) task)[task->stackTop]));
}


inline static uint32_t uartBufferSize (struct OS_DRIVER_ComUARTParams
                                       *uartParams)
{
    return sizeof (enum OS_DRIVER_ComType)
            + uartParams->recvSize
            + uartParams->sendSize
            + sizeof(struct OS_DRIVER_ComUART);
}


inline static enum OS_Result uartInit (struct OS_TaskControl *task,
                                       struct OS_DRIVER_ComUARTParams
                                       *uartParams, uint8_t *bStart)
{
    uint8_t *recvBuffer = &bStart[0];
    uint8_t *sendBuffer = &bStart[uartParams->recvSize];

    struct OS_DRIVER_ComUART *cu = getComUart (task);

    enum OS_Result r;
    if ((r = OS_MUTEX_Init (&cu->mutex)) != OS_Result_OK)
    {
        return r;
    }

    return UART_Init (cu->uart, uartParams->handler,
                      recvBuffer, uartParams->recvSize,
                      sendBuffer, uartParams->sendSize,
                      uartParams->baud)
            ? OS_Result_OK
            : OS_Result_Error;
}


inline static enum OS_Result uartOp (struct OS_TaskControl *task,
                                     struct OS_TaskDriverComAccess *ca)
{
    struct OS_DRIVER_ComUART *cu = getComUart (task);

    bool baseRes = false;
    switch (op)
    {
        case OS_DRIVER_ComOp_Recv:
        {
            const uint32_t Pending = CYCLIC_Pending (&cu->uart.recv);
            if (Pending)
            {
                baseRes = CYCLIC_OutToBuffer (&cu->uart.recv, ca->buf,
                                              ca->count);
                ca->processed = (Pending > ca->count)? ca->count : Pending;
            }
            break;
        }
        case OS_DRIVER_ComOp_Send:
            baseRes         = UART_PutBinary (&cu->uart, ca->buf, ca->count);
            ca->processed   = ca->count;
            break;

        default:
            return OS_Result_AssertionFailed;
    }

    return baseRes? OS_Result_OK : OS_Result_Error;
}


uint32_t OS_DRIVER_ComBufferSize (struct OS_DRIVER_ComInitParams
                                  *initParams)
{
    if (!DEBUG_Assert (initParams))
    {
        return 0;
    }

    switch (initParams->type)
    {
        case OS_DRIVER_ComType_UART:
            return uartBufferSize ((struct OS_DRIVER_ComUARTParams *)
                                    initParams);
        default:
            break;
    }

    DEBUG_Assert (false);
    return 0;
}


enum OS_Result OS_DRIVER_ComInit (struct OS_TaskControl *task,
                                  struct OS_DRIVER_ComInitParams *initParams)
{
    if (!task || !initParams || !initParams->jobs)
    {
        return OS_Result_InvalidParams;
    }

    const uint32_t  BufferSize  = OS_DRIVER_ComBufferSize (initParams);
    uint8_t         *buffer     = (uint8_t *) task;

    if (!DEBUG_Assert (BufferSize && !(BufferSize & 0b11)))
    {
        return OS_Result_InvalidBufferSize;
    }

    task->stackTop = task->size - BufferSize;

    if ((uint32_t)&buffer[task->stackTop] & 0b11)
    {
        return OS_Result_InvalidBufferAlignment;
    }

    memset (&buffer[task->stackTop], 0, BufferSize);

    *((enum OS_DRIVER_ComType *) &buffer[task->stackTop]) = initParams->type;

    uint8_t *bStart = &buffer[task->stackTop + sizeof(enum OS_DRIVER_ComType)];

    switch (initParams->type)
    {
        case OS_DRIVER_ComType_UART:
        {
            task->priority = OS_TaskPriority_DriverComUART;
            return uartInit (task, (struct OS_DRIVER_ComUARTParams *)
                             initParams, bStart);
        }

        default:
            break;
    }

    return OS_Result_Error;
}


enum OS_Result OS_DRIVER_ComLock (struct OS_TaskControl *task)
{
    if (!task)
    {
        return OS_Result_InvalidParams;
    }

    struct OS_DRIVER_ComUART *cu = getComUart (task);

    return OS_MUTEX_Lock (cu->mutex);
}


enum OS_Result OS_DRIVER_ComUnlock (struct OS_TaskControl *task)
{
    if (!task)
    {
        return OS_Result_InvalidParams;
    }

    struct OS_DRIVER_ComUART *cu = getComUart (task);

    return OS_MUTEX_Unlock (cu->mutex);
}


enum OS_Result OS_DRIVER_ComAccess (struct OS_TaskControl *task,
                                    struct OS_TaskDriverComAccess *ca)
{
    if (!task || !ca)
    {
        return OS_Result_InvalidParams;
    }

    const enum OS_DRIVER_ComType Type = getComType (task);

    switch (Type)
    {
        case OS_DRIVER_ComType_UART:
            return uartOp (task, ca);

        default:
            break;
    }

    return OS_Result_AssertionFailed;
}
