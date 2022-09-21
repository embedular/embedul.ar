#pragma once

#include "../../api.h"
#include "../../private/syscall.h"
#include "../../private/opaque.h"
#include "../../mutex.h"
#include "../../../base/attr.h"
#include "../../../base/queue.h"
#include "../../../base/uart.h"


enum OS_DRIVER_ComType
{
    ATTR_EnumForceUint32 (OS_DRIVER_ComType),
    OS_DRIVER_ComType_Invalid   = 0,
    OS_DRIVER_ComType_UART
};


struct OS_DRIVER_ComInitParams
{
    enum OS_DRIVER_ComType  type;
    void                    *params;
};


struct OS_DRIVER_ComUARTParams
{
    // Both sizes must be powers of 2
    void                    *handler;
    uint32_t                sendSize;
    uint32_t                recvSize;
    uint32_t                baud;
};


struct OS_DRIVER_ComUART
{
    struct UART             uart;
    struct OS_MUTEX         mutex;
};


uint32_t        OS_DRIVER_ComBufferSize     (struct OS_DRIVER_ComInitParams
                                             *initParams);
enum OS_Result  OS_DRIVER_ComInit           (struct OS_TaskControl *task,
                                             struct OS_DRIVER_ComInitParams
                                             *initParams);
enum OS_Result  OS_DRIVER_ComLock           (struct OS_TaskControl *task);
enum OS_Result  OS_DRIVER_ComUnlock         (struct OS_TaskControl *task);
enum OS_Result  OS_DRIVER_ComAccess         (struct OS_TaskControl *task,
                                             struct OS_TaskDriverComAccess *ca);
