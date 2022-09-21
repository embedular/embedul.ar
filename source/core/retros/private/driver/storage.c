#include "embedul.ar/source/core/retros/private/driver/storage.h"
#include "embedul.ar/source/core/retros/private/opaque.h"
#include "embedul.ar/source/core/retros/private/scheduler.h"
#include "embedul.ar/source/core/device/board.h"

#include <string.h>


inline static struct OS_DRIVER_StorageData * getStorageData (
                                                struct OS_TaskControl *tc)
{
    return (struct OS_DRIVER_StorageData *) &((uint8_t *) tc)
                    [tc->size - sizeof(struct OS_DRIVER_StorageData)];
}


uint32_t OS_DRIVER_StorageBufferSize (struct OS_DRIVER_StorageInitParams
                                      *initParams)
{
    BOARD_AssertParams (initParams && initParams->stack && initParams->jobs);

    return OS_GenericTaskBufferSize(initParams->stack)
                + sizeof(struct OS_DRIVER_StorageJob) * initParams->jobs
                + sizeof(struct OS_DRIVER_StorageData);
}


enum OS_Result OS_DRIVER_StorageInit (struct OS_TaskControl *tc,
                                      struct OS_DRIVER_StorageInitParams
                                      *initParams)
{
    if (!tc || !initParams || !initParams->jobs)
    {
        return OS_Result_InvalidParams;
    }

    tc->priority = OS_TaskPriority_DriverStorage;

    const uint32_t  DrvBufSize  = OS_DRIVER_StorageBufferSize (initParams);
    uint8_t         *buffer     = (uint8_t *) tc;

    if (!DrvBufSize || (DrvBufSize & 0x03))
    {
        return OS_Result_InvalidBufferSize;
    }

    tc->stackTop -= DrvBufSize;

    if ((uint32_t)&buffer[tc->stackTop] & 0x03)
    {
        return OS_Result_InvalidBufferAlignment;
    }

    memset (&buffer[tc->stackTop], 0, DrvBufSize);

    struct OS_DRIVER_StorageData *data = getStorageData (tc);

    data->maxJobs     = initParams->jobs;
    data->nextFreeJob = (struct OS_DRIVER_StorageJob *) &buffer[tc->stackTop];

    return OS_Result_OK;
}


// WARNING: Whoever call this function must assure the integrity of *sa contents
//          in memory across all operations involving that data. This means to
//          avoid destruction of the local struct after exiting the OS API call
//          that produced the syscall. This is currently managed by putting the
//          caller task to sleep until the storage operation completes, but can
//          be made asynchronic by performing a deep copy of the data.
enum OS_Result OS_DRIVER_StorageJobAdd (struct OS_TaskControl *tc,
                                        struct OS_TaskDriverStorageAccess *sa)
{
    if (!tc || !sa)
    {
        return OS_Result_InvalidParams;
    }

    struct OS_DRIVER_StorageData *data = getStorageData (tc);

    if (data->queue.elements >= data->maxJobs)
    {
        return OS_Result_BufferFull;
    }

    struct OS_DRIVER_StorageJob *job = data->nextFreeJob;
    // This should never happen given the number of elements in the queue is
    // less than data->maxJobs.
    BOARD_AssertState (!job->access);

    // Next free job. Jobs are consumed in a FIFO scheme.
    if ((void *)(++ data->nextFreeJob) == (void *)data)
    {
        data->nextFreeJob = (struct OS_DRIVER_StorageJob *)
                                        &((uint8_t *)tc)[tc->stackTop];
    }

    job->access = sa;

    if (!QUEUE_PushNode (&data->queue, (struct QUEUE_Node *) job))
    {
        return OS_Result_Error;
    }

    return OS_Result_OK;
}


enum OS_Result OS_DRIVER_StorageJobTake (struct OS_TaskControl *tc,
                                         struct OS_TaskDriverStorageAccess **sa)
{
    if (!tc || !sa)
    {
        return OS_Result_InvalidParams;
    }

    struct OS_DRIVER_StorageData *data = getStorageData (tc);

    if (!data->queue.head)
    {
        BOARD_AssertState (!data->queue.elements);

        return OS_Result_Empty;
    }

    struct OS_DRIVER_StorageJob *job = (struct OS_DRIVER_StorageJob *)
                                                        data->queue.head;

    if (!QUEUE_DetachNode (&data->queue, data->queue.head))
    {
        return OS_Result_Error;
    }

    data->pending   = job->access;
    *sa             = job->access;

    // Job slot marked as free.
    job->access = NULL;

    return OS_Result_OK;
}


enum OS_Result OS_DRIVER_StorageJobDone (struct OS_TaskControl *tc,
                                         struct OS_TaskDriverStorageAccess *sa,
                                         enum OS_Result result)
{
    if (!tc || !sa)
    {
        return OS_Result_InvalidParams;
    }

    sa->result = result;

    struct OS_DRIVER_StorageData *data = getStorageData (tc);

    if (result == OS_Result_OK)
    {
        ++ data->jobsSucceeded;
    }
    else
    {
        ++ data->jobsFailed;
    }

    if (sa->op == OS_TaskDriverOp_Read)
    {
        data->sectorsRead += sa->count;
    }
    else
    {
        data->sectorsWritten += sa->count;
    }

    // Release associate caller semaphore and set scheduler interrupt to pending
    // to resume caller execution.
    const bool SemReleaseResult = SEMAPHORE_Release (sa->sem);

    BOARD_AssertState (SemReleaseResult);

    OS_SchedulerSetPending ();

    return OS_Result_OK;
}
