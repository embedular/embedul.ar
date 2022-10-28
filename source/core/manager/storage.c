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

#include "embedul.ar/source/core/manager/storage.h"
#include "embedul.ar/source/core/device/rawstor.h"
#include "embedul.ar/source/core/device/board.h"


#define MBR_PART_TYPE_W95_FAT32_LBA     0x0C
#define MBR_PART_TYPE_NO_FS_DATA        0xDA


static struct STORAGE * s_s = NULL;


#if defined(LIB_EMBEDULAR_HAS_FILESYSTEM) && FF_MULTI_PARTITION
PARTITION VolToPart[FF_VOLUMES] = 
{
    {0xFF, 0xFF},   // (0:) Physical drive / partition. 
    {0xFF, 0xFF},   // (1:) Physical drive / partition.
};
#endif


#if defined(LIB_EMBEDULAR_HAS_FILESYSTEM) && !FF_FS_READONLY && !FF_FS_NORTC
DWORD get_fattime (void)
{
    const struct BOARD_RealTimeClock rtc = BOARD_RealTimeClock ();

    // Pack date and time into a DWORD variable
    return (  (DWORD) (rtc.year - 1980)  << 25)
        | ((DWORD)  rtc.month         << 21)
        | ((DWORD)  rtc.dayOfMonth    << 16)
        | ((DWORD)  rtc.hour          << 11)
        | ((DWORD)  rtc.minute        << 5)
        | ((DWORD)  rtc.second        >> 1);
}
#endif


void STORAGE_Init (struct STORAGE *const S)
{
    BOARD_AssertState  (!s_s);
    BOARD_AssertParams (S);

    OBJECT_Clear (S);

    {
        LOG_AutoContext (S, LANG_INIT);

        s_s = S;

        LOG_Items (1, LANG_DEVICE_ROLES, (uint32_t)STORAGE_Role__COUNT);
    }
}


void STORAGE_SetDevice (struct RAWSTOR *const Driver)
{
    if (RAWSTOR_IsMediaReady (Driver))
    {
    #ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
        STORAGE_RegisterVolume (STORAGE_Role_FatFsDrive0, Driver, 1);
    #endif
        STORAGE_RegisterVolume (STORAGE_Role_LinearCache, Driver, 2);
    }
}


inline static void setRawVolume (const enum STORAGE_Role Role,
                                 struct STORAGE_Volume *const Vol)
{
    if (Vol->info.partitionNr)
    {
        LOG_Warn (s_s, LANG_STORAGE_MEDIA_HAS_NO_MBR);
        LOG_Items (3,
                    LANG_DRIVER,    RAWSTOR_Description(Vol->driver),
                    LANG_ROLE,      Role,
                    LANG_PARTITION, Vol->info.partitionNr);

        BOARD_AssertState (false);
    }

    // Role is not Linear. A filesystem requires a partitioning scheme.
    if (!(Role <= (int32_t)STORAGE_Role_Linear__END))
    {
        LOG_Warn (s_s, LANG_INVALID_FILESYSTEM_ROLE_NO_MBR);
        LOG_Items (2,
                    LANG_DRIVER,    RAWSTOR_Description(Vol->driver),
                    LANG_ROLE,      Role);

        BOARD_AssertState (false);
    }

    RAWSTOR_Status_Result r;
    uint32_t sectorCount = 0;

    // Linear role with no MBR and no partition requested will use the
    // entire media. 'sectorBegin' starts at zero, and 'sectorEnd' equals
    // to the entire device sectors.
    if ((r = RAWSTOR_MediaIoctl(Vol->driver, RAWSTOR_IOCTL_CMD_GET_SECTOR_COUNT,
            &sectorCount)) != RAWSTOR_Status_Result_Ok)
    {
        LOG_Warn (s_s, LANG_ERROR_GETTING_DEVICE_SECTORS);
        LOG_Items (3,
                    LANG_DRIVER,    RAWSTOR_Description(Vol->driver),
                    LANG_ROLE,      Role,
                    LANG_ERROR,     r);

        BOARD_AssertState (false);
    }

    BOARD_AssertState (sectorCount);

    // Set partition params
    Vol->info.sectorBegin   = 0;
    Vol->info.sectorEnd     = sectorCount - 1;
    Vol->info.partitionType = 0;
}


inline static void setMBRVolume (const enum STORAGE_Role Role,
                                 struct STORAGE_Volume *const Vol,
                                 const uint8_t *const Mbr)
{
    if (!Vol->info.partitionNr)
    {
        // ...but no partition specified. Random writes to a potentially
        // partitioned device could easily destroy its file structure.

        LOG (s_s, LANG_NO_PARTITION_SPECIFIED_MBR);
        LOG_Items (2,
                    LANG_DRIVER,    RAWSTOR_Description(Vol->driver),
                    LANG_ROLE,      Role);

        BOARD_AssertParams (false);
    }

    // Info about PartitionNr
    const uint32_t  Base    = 0x01BE + (Vol->info.partitionNr - 1) * 16; 
    const uint8_t   Type    = Mbr[Base + 0x04];
    const uint32_t  LBA1st  = *((const uint32_t *)&Mbr[Base + 0x08]);
    const uint32_t  LBACnt  = *((const uint32_t *)&Mbr[Base + 0x0C]);

    // Check matching role and PartitionNr code on MBR.
    if (Role <= STORAGE_Role_Linear__END &&
        Type != MBR_PART_TYPE_NO_FS_DATA)
    {
        LOG (s_s, LANG_INVALID_PART_LINEAR_ROLE);
        LOG_Items (5,
                    LANG_DRIVER,        RAWSTOR_Description(Vol->driver),
                    LANG_ROLE,          Role,
                    LANG_PARTITION,     Vol->info.partitionNr,
                    LANG_TYPE,          Type,
                    LANG_REQUIRED_TYPE, (uint8_t)MBR_PART_TYPE_NO_FS_DATA,
                    LOG_ItemsBases (
                        0, 0, 0, VARIANT_Base_Hex, VARIANT_Base_Hex));

        BOARD_AssertParams (false);
    }
#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
    else if (Role >= STORAGE_Role_FatFs__BEGIN && 
             Role <= STORAGE_Role_FatFs__END)
    {
        if (Type != MBR_PART_TYPE_W95_FAT32_LBA)
        {
            LOG (s_s, LANG_INVALID_PART_FILESYSTEM_ROLE);
            LOG_Items (5,
                    LANG_DRIVER,        RAWSTOR_Description(Vol->driver), 
                    LANG_ROLE,          Role,
                    LANG_PARTITION,     Vol->info.partitionNr,
                    LANG_TYPE,          Type, 
                    LANG_REQUIRED_TYPE, (uint8_t)MBR_PART_TYPE_W95_FAT32_LBA,
                    LOG_ItemsBases (
                        0, 0, 0, VARIANT_Base_Hex, VARIANT_Base_Hex));

            BOARD_AssertParams (false);
        }
    }
#endif

    // Set partition params
    Vol->info.sectorBegin   = LBA1st;
    Vol->info.sectorEnd     = LBA1st + LBACnt - 1;
    Vol->info.partitionType = Type;
}


#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
inline static void mountFilesystem (const enum STORAGE_Role Role,
                                    struct STORAGE_Volume *const Vol)
{
    // Configure FatFs to use either storage manager FatFsDrive0 or
    // FatFsDrive1 volumes with a given partition number. Each role
    // might be served by the same device driver or not. 
    const uint8_t VolumeId = Role - STORAGE_Role_FatFs__BEGIN;

    VolToPart[VolumeId].pd = VolumeId;
    VolToPart[VolumeId].pt = Vol->info.partitionNr;

    const char VolumeStr[] = { VolumeId + 48, ':', '\0' };

    // Force mount FatFs Volume "VolumeId"
    const FRESULT Fr = f_mount (&s_s->fatFs[VolumeId], VolumeStr, 1);

    if (Fr != FR_OK)
    {
        LOG (s_s, LANG_FILESYSTEM_MOUNT_ERROR);
        LOG_Items (5,
                    LANG_DRIVER,        RAWSTOR_Description(Vol->driver),
                    LANG_ROLE,          Role,
                    LANG_PARTITION,     Vol->info.partitionNr,
                    LANG_VOLUME,        VolumeStr,
                    LANG_ERROR,         Fr);

        BOARD_AssertParams (false);
    }
}
#endif


void STORAGE_RegisterVolume (const enum STORAGE_Role Role,
                             struct RAWSTOR *const Driver,
                             const uint8_t PartitionNr)
{
    BOARD_AssertParams (Driver && 
                        PartitionNr < 5 && 
                        Role < STORAGE_Role__COUNT);
    BOARD_AssertState  (!s_s->volume[Role].driver);

    uint8_t mbr[512];
    RAWSTOR_Status_Result r;

    s_s->volume[Role].driver            = Driver;
    s_s->volume[Role].info.partitionNr  = PartitionNr;

    // Read device MBR, if any. Any device/media must already be usable to be
    // assigned as a volume. The read is a good test of the former since it
    // will cause the device media to be ready and will test an actual
    // operation on it. 
    if ((r = RAWSTOR_MediaRead(Driver, mbr, 0, 1)) != RAWSTOR_Status_Result_Ok)
    {
        LOG_Warn (s_s, LANG_MEDIA_READ_ERROR);
        LOG_Items (3,
                    LANG_DRIVER,        RAWSTOR_Description(Driver),
                    LANG_ROLE,          Role,
                    LANG_ERROR,         r);

        BOARD_AssertState (false);
    }

    // No MBR
    if (mbr[0x01FE] != 0x55 || mbr[0x01FF] != 0xAA)
    {
        setRawVolume (Role, &s_s->volume[Role]);
    }
    // Assume there is a MBR
    else 
    {
        setMBRVolume (Role, &s_s->volume[Role], mbr);
    }

#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
    if (Role >= STORAGE_Role_FatFs__BEGIN && Role <= STORAGE_Role_FatFs__END)
    {
        mountFilesystem (Role, &s_s->volume[Role]);
    }
#endif

    ++ s_s->registeredVolumes;
}


uint32_t STORAGE_RegisteredVolumes (void)
{
    return s_s->registeredVolumes;
}


bool STORAGE_ValidVolume (const enum STORAGE_Role Role)
{
    return s_s->volume[Role].driver? true : false;
}


struct STORAGE_VolumeInfo STORAGE_VolumeInfo (const enum STORAGE_Role Role)
{
    return s_s->volume[Role].info;
}


static RAWSTOR_Status_Result drvRead (const enum STORAGE_Role Role,
                                      uint8_t *data, uint32_t sector,
                                      uint32_t count)
{
    const struct STORAGE_Volume * vol = &s_s->volume[Role];

    s_s->rwRequests.read += count;

    const RAWSTOR_Status_Result R = 
        RAWSTOR_MediaRead (vol->driver, data, sector, count);

    if (R != RAWSTOR_Status_Result_Ok)
    {
        ++ s_s->failedRequests.read;
    }

    return R;
}


static RAWSTOR_Status_Result drvWrite (const enum STORAGE_Role Role,
                                       const uint8_t *data, 
                                       uint32_t sector, uint32_t count)
{
    const struct STORAGE_Volume * vol = &s_s->volume[Role];

    s_s->rwRequests.write += count;

    const RAWSTOR_Status_Result R = 
        RAWSTOR_MediaWrite (vol->driver, data, sector, count);

    if (R != RAWSTOR_Status_Result_Ok)
    {
        ++ s_s->failedRequests.write;
    }

    return R;
}


void STORAGE___setCachedElementsCount (const uint32_t Count)
{
    s_s->cachedElementsCount = Count;
}


uint32_t STORAGE_CachedElementsCount (void)
{
    return s_s->cachedElementsCount;
}


static uint32_t checkAccess (const enum STORAGE_Role LinearRole,
                             const struct STORAGE_Volume *const Vol,
                             const uint32_t LocalSector, const uint32_t Count,
                             const char *const Message)
{
    // Partition-local to device sectors
    const uint32_t DeviceSector = Vol->info.sectorBegin + LocalSector;

    // Check out-of-bounds access. "sector end" inclusive, hence the - 1.
    if (DeviceSector + Count - 1 > Vol->info.sectorEnd)
    {
        LOG (s_s, Message);
        LOG_Items (6,
                LANG_DRIVER,                RAWSTOR_Description(Vol->driver),
                LANG_ROLE,                  LinearRole,
                LANG_PARTITION,             Vol->info.partitionNr,
                LANG_DEVICE_SECTOR_BEGIN,   DeviceSector,
                LANG_DEVICE_SECTOR_COUNT,   Count,
                LANG_DEVICE_SECTOR_END,     Vol->info.sectorEnd);

        BOARD_AssertParams (false);
    }

    return DeviceSector;
}


RAWSTOR_Status_Result STORAGE_LinearRead (const enum STORAGE_Role LinearRole,
                                          uint8_t *const Data,
                                          const uint32_t LocalSector,
                                          const uint32_t Count,
                                          const uint32_t Retries)
{
    BOARD_AssertParams (LinearRole <= STORAGE_Role_Linear__END);

    const struct STORAGE_Volume *const Vol = &s_s->volume[LinearRole];

    // Linear accesses allowed in 0xDA partitions only.
    BOARD_AssertState  (Vol->driver &&
                        Vol->info.partitionType == MBR_PART_TYPE_NO_FS_DATA);

    // Partition-local to device sectors
    const uint32_t DeviceSector = checkAccess (LinearRole, Vol,
                                               LocalSector, Count, 
                                               LANG_OUT_OF_BOUNDS_READ_ACCESS);

    uint32_t currentRetries = Retries;
    static RAWSTOR_Status_Result r;
    do 
    {
        drvRead (LinearRole, Data, DeviceSector, Count);
    }
    while (r != RAWSTOR_Status_Result_Ok && currentRetries --);

    return r;
}


RAWSTOR_Status_Result STORAGE_LinearWrite (const enum STORAGE_Role LinearRole,
                                           const uint8_t *const Data,
                                           const uint32_t LocalSector,
                                           const uint32_t Count,
                                           const uint32_t Retries)
{
    BOARD_AssertParams (LinearRole <= STORAGE_Role_Linear__END);

    const struct STORAGE_Volume *const Vol = &s_s->volume[LinearRole];

    // Linear accesses allowed in 0xDA partitions only.
    BOARD_AssertState  (Vol->driver &&
                        Vol->info.partitionType == MBR_PART_TYPE_NO_FS_DATA);

    // Partition-local to device sectors
    const uint32_t DeviceSector = checkAccess (LinearRole, Vol,
                                               LocalSector, Count, 
                                               LANG_OUT_OF_BOUND_WRITE_ACCESS);

    uint32_t currentRetries = Retries;
    static RAWSTOR_Status_Result r;
    do 
    {
        r = drvWrite (LinearRole, Data, DeviceSector, Count);
    }
    while (r != RAWSTOR_Status_Result_Ok && currentRetries --);

    return r;
}


#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
RAWSTOR_Status_Disk STORAGE_MAI_InitVolume (const enum STORAGE_Role Role)
{
    BOARD_AssertState (s_s->volume[Role].driver);

    const RAWSTOR_Status_Result R = 
        RAWSTOR_MediaInit (s_s->volume[Role].driver);

    if (R != RAWSTOR_Status_Result_Ok)
    {
        ++ s_s->failedRequests.init;
    }

    return RAWSTOR_Status(s_s->volume[Role].driver).disk;
}


struct RAWSTOR_Status STORAGE_MAI_VolumeStats (const enum STORAGE_Role Role)
{
    BOARD_AssertState (s_s->volume[Role].driver);
    return RAWSTOR_Status (s_s->volume[Role].driver);
}


RAWSTOR_Status_Result STORAGE_MAI_ReadVolume (const enum STORAGE_Role Role,
                                              uint8_t *data, uint32_t sector,
                                              uint32_t count)
{
    BOARD_AssertState (s_s->volume[Role].driver);
    return drvRead (Role, data, sector, count);
}


RAWSTOR_Status_Result STORAGE_MAI_WriteVolume (const enum STORAGE_Role Role,
                                               const uint8_t *data,
                                               uint32_t sector, uint32_t count)
{
    BOARD_AssertState (s_s->volume[Role].driver);
    return drvWrite (Role, data, sector, count);
}


RAWSTOR_Status_Result STORAGE_MAI_VolumeIoctl (const enum STORAGE_Role Role,
                                               uint8_t cmd, void *data)
{
    BOARD_AssertState (s_s->volume[Role].driver);

    const RAWSTOR_Status_Result R =
        RAWSTOR_MediaIoctl (s_s->volume[Role].driver, cmd, data);

    if (R != RAWSTOR_Status_Result_Ok)
    {
        ++ s_s->failedRequests.ioctl;
    }

    return R;
}
#endif  // #ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
