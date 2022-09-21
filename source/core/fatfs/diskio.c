#include "embedul.ar/source/core/fatfs/diskio.h"
#include "embedul.ar/source/core/manager/storage/mai.h"
#include "embedul.ar/source/core/device/board.h"


DSTATUS disk_initialize (BYTE pdrv)
{
    BOARD_AssertParams (pdrv < 2);
    return STORAGE_MAI_InitVolume (STORAGE_Role_FatFs__BEGIN + pdrv);
}


DSTATUS disk_status (BYTE pdrv)
{
    BOARD_AssertParams (pdrv < 2);
    return STORAGE_MAI_VolumeStats (STORAGE_Role_FatFs__BEGIN + pdrv).disk;
}


DRESULT disk_read (BYTE pdrv, BYTE *buff, LBA_t sector,
                                 UINT count)
{
    BOARD_AssertParams (pdrv < 2);
    return STORAGE_MAI_ReadVolume (STORAGE_Role_FatFs__BEGIN + pdrv,
                                   buff, sector, count);
}


#if !FF_FS_READONLY
DRESULT disk_write (BYTE pdrv, const BYTE *buff, LBA_t sector,
                                  UINT count)
{
    BOARD_AssertParams (pdrv < 2);
    return STORAGE_MAI_WriteVolume (STORAGE_Role_FatFs__BEGIN + pdrv,
                                    buff, sector, count);
}
#endif


DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void *buff)
{
    BOARD_AssertParams (pdrv < 2);
    return STORAGE_MAI_VolumeIoctl (STORAGE_Role_FatFs__BEGIN + pdrv, cmd,
                                    buff);
}
