#pragma once

#include "embedul.ar/source/core/retros/api.h"
#include "embedul.ar/source/core/retros/private/syscall.h"
#include "embedul.ar/source/core/retros/private/opaque.h"
#include "embedul.ar/source/core/cc.h"
#include "embedul.ar/source/core/queue.h"


struct OS_DRIVER_StorageInitParams
{
    uint32_t                            stack;
    uint32_t                            jobs;
};


struct OS_DRIVER_StorageJob
{
    struct QUEUE_Node                   node;
    struct OS_TaskDriverStorageAccess   * access;
}
CC_Aligned(4);


struct OS_DRIVER_StorageData
{
    struct QUEUE                        queue;
    uint32_t                            maxJobs;
    struct OS_DRIVER_StorageJob         * nextFreeJob;
    struct OS_TaskDriverStorageAccess   * pending;
    uint32_t                            jobsSucceeded;
    uint32_t                            jobsFailed;
    uint32_t                            sectorsRead;
    uint32_t                            sectorsWritten;
}
CC_Aligned(4);


uint32_t        OS_DRIVER_StorageBufferSize (struct OS_DRIVER_StorageInitParams
                                             *initParams);
enum OS_Result  OS_DRIVER_StorageInit       (struct OS_TaskControl *tc,
                                             struct OS_DRIVER_StorageInitParams
                                             *initParams);
enum OS_Result  OS_DRIVER_StorageJobAdd     (struct OS_TaskControl *tc,
                                             struct OS_TaskDriverStorageAccess
                                             *sa);
enum OS_Result  OS_DRIVER_StorageJobTake    (struct OS_TaskControl *tc,
                                             struct OS_TaskDriverStorageAccess
                                             **sa);
enum OS_Result  OS_DRIVER_StorageJobDone    (struct OS_TaskControl *tc,
                                             struct OS_TaskDriverStorageAccess
                                             *sa, enum OS_Result result);
