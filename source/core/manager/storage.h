/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [MANAGER] storage devices manager.

  Copyright 2018-2022 Santiago Germino
  <sgermino@embedul.ar> https://www.linkedin.com/in/royconejo

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#pragma once

#include "embedul.ar/source/core/device/rawstor.h"
#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
#include "ff.h"
#endif


enum STORAGE_Role
{
    STORAGE_Role_LinearCache = 0,
    STORAGE_Role_LinearPersistent,
#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
    STORAGE_Role_FatFsDrive0,   // Volume 0 (0:)
    STORAGE_Role_FatFsDrive1,   // Volume 1 (1:)
#endif
    STORAGE_Role__COUNT,
    STORAGE_Role_Linear__BEGIN  = STORAGE_Role_LinearCache,
    STORAGE_Role_Linear__END    = STORAGE_Role_LinearPersistent,
#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
    STORAGE_Role_FatFs__BEGIN   = STORAGE_Role_FatFsDrive0,
    STORAGE_Role_FatFs__END     = STORAGE_Role_FatFsDrive1
#endif
};


struct STORAGE_VolumeInfo
{
    uint32_t    sectorBegin;    // 0 = no MBR.
    uint32_t    sectorEnd;      // if no MBR, all device sectors.
    uint8_t     partitionNr;    
    uint8_t     partitionType;
};


struct STORAGE_Volume
{
    struct RAWSTOR              * driver;
    struct STORAGE_VolumeInfo   info;
};


struct STORAGE_RWSectorRequests
{
    uint32_t    read;
    uint32_t    write;
};


struct STORAGE_FailedRequests
{
    uint32_t    init;
    uint32_t    read;
    uint32_t    write;
    uint32_t    ioctl;
};


struct STORAGE
{
    struct STORAGE_Volume               volume[STORAGE_Role__COUNT];
    uint32_t                            registeredVolumes;
    struct STORAGE_RWSectorRequests     rwRequests;
    struct STORAGE_FailedRequests       failedRequests;
    uint32_t                            cachedElementsCount;
#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
    FATFS                               fatFs[STORAGE_Role_FatFs__END - 
                                                STORAGE_Role_FatFs__BEGIN + 1];
#endif
};


void        STORAGE_Init                (struct STORAGE *const S);
void        STORAGE_SetDevice           (struct RAWSTOR *const Driver);
void        STORAGE_RegisterVolume           (const enum STORAGE_Role Role,
                                         struct RAWSTOR *const Driver,
                                         const uint8_t PartitionNr);
uint32_t    STORAGE_RegisteredVolumes          (void);
bool        STORAGE_ValidVolume         (const enum STORAGE_Role Role);
struct STORAGE_VolumeInfo
            STORAGE_VolumeInfo          (const enum STORAGE_Role Role);
uint32_t    STORAGE_CachedElementsCount (void);
RAWSTOR_Status_Result
            STORAGE_LinearRead          (const enum STORAGE_Role LinearRole,
                                         uint8_t *const Data,
                                         const uint32_t LocalSector,
                                         const uint32_t Count,
                                         const uint32_t Retries);
RAWSTOR_Status_Result
            STORAGE_LinearWrite         (const enum STORAGE_Role LinearRole,
                                         const uint8_t *const Data,
                                         const uint32_t LocalSector,
                                         const uint32_t Count,
                                         const uint32_t Retries);
