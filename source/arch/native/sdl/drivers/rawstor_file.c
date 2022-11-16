/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [RAWSTOR driver] hosted environment file.

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

#include "embedul.ar/source/arch/native/sdl/drivers/rawstor_file.h"
#include "embedul.ar/source/core/device/board.h"
#include <errno.h>


// Common IO interface
static RAWSTOR_Status_Result    mediaInit       (struct RAWSTOR *const R);
static RAWSTOR_Status_Result    mediaRead       (struct RAWSTOR *const R,
                                                 uint8_t *const Data,
                                                 const uint32_t SectorBegin,
                                                 const uint32_t SectorCount);
static RAWSTOR_Status_Result    mediaWrite      (struct RAWSTOR *const R,
                                                 const uint8_t *const Data,
                                                 const uint32_t SectorBegin,
                                                 const uint32_t SectorCount);
static RAWSTOR_Status_Result    mediaIoctl      (struct RAWSTOR *const R,
                                                 const uint8_t Cmd,
                                                 void *const Data);


static const struct RAWSTOR_IFACE RAWSTOR_FILE_IFACE =
{
    .Description    = "os-hosted file",
    .MediaInit      = mediaInit,
    .MediaRead      = mediaRead,
    .MediaWrite     = mediaWrite,
    .MediaIoctl     = mediaIoctl
};


void RAWSTOR_FILE_Init (struct RAWSTOR_FILE *const F,
                        const char *const Filename)
{
    BOARD_AssertParams (F && Filename);

    DEVICE_IMPLEMENTATION_Clear (F);

    F->filename = Filename;

    RAWSTOR_Init ((struct RAWSTOR *)F, &RAWSTOR_FILE_IFACE);
}


static RAWSTOR_Status_Result mediaInit (struct RAWSTOR *const R)
{
    struct RAWSTOR_FILE *const F = (struct RAWSTOR_FILE *) R;

    RAWSTOR_UpdateStatusMedia (R, RAWSTOR_Status_Media_Initializing, 0, 0);

    F->fd = fopen (F->filename, "r+b");
    if (!F->fd)
    {
        LOG_WarnDebug (R, LANG_ERROR_OPENING_FILE);
        LOG_Items (2,
                LANG_FILENAME,          F->filename,
                LANG_ERROR,         errno);

        RAWSTOR_UpdateStatusResult (R, RAWSTOR_Status_Result_NotReady);
        return RAWSTOR_Status_Result_NotReady;
    }

    R->status.disk &= ~RAWSTOR_Status_Disk_NotPresent;
    R->status.disk &= ~RAWSTOR_Status_Disk_NotInitialized;

    RAWSTOR_UpdateStatusMedia (R, RAWSTOR_Status_Media_Ready, 0, 0);
    RAWSTOR_UpdateStatusResult (R, RAWSTOR_Status_Result_Ok);

    return RAWSTOR_Status_Result_Ok;
}


static RAWSTOR_Status_Result mediaRead (struct RAWSTOR *const R,
                                        uint8_t *const Data,
                                        const uint32_t Sector,
                                        const uint32_t Count)
{
    struct RAWSTOR_FILE *const F = (struct RAWSTOR_FILE *) R;

    // LBA (512-byte sectors) to byte addressing
    if (fseek (F->fd, Sector * RAWSTOR_SECTOR_SIZE, SEEK_SET))
    {
        return RAWSTOR_Status_Result_ReadWriteError;
    }

    const size_t read = fread (Data, RAWSTOR_SECTOR_SIZE, Count, F->fd);

	return (read != Count)? RAWSTOR_Status_Result_ReadWriteError :
                            RAWSTOR_Status_Result_Ok;
}


static RAWSTOR_Status_Result mediaWrite (struct RAWSTOR *const R,
                                         const uint8_t *const Data,
                                         const uint32_t SectorBegin,
                                         const uint32_t SectorCount)
{
    struct RAWSTOR_FILE *const F = (struct RAWSTOR_FILE *) R;

    // LBA (512-byte sectors) to byte addressing
    if (fseek (F->fd, SectorBegin * RAWSTOR_SECTOR_SIZE, SEEK_SET))
    {
        return RAWSTOR_Status_Result_ReadWriteError;
    }

    const size_t write = fwrite (Data, RAWSTOR_SECTOR_SIZE, SectorCount, F->fd);
    fflush (F->fd);

	return (write != SectorCount)? RAWSTOR_Status_Result_ReadWriteError :
                             RAWSTOR_Status_Result_Ok;
}


static RAWSTOR_Status_Result mediaIoctl (struct RAWSTOR *const R,
                                         const uint8_t Cmd,
                                         void *const Data)
{   
    struct RAWSTOR_FILE *const F = (struct RAWSTOR_FILE *) R;
        
    if (R->status.disk & RAWSTOR_Status_Disk_NotInitialized)
    {
        return RAWSTOR_Status_Result_NotReady;
    }

    switch (Cmd)
    {
        case RAWSTOR_IOCTL_CMD_SYNC:
            if (fflush (F->fd))
            {
                return RAWSTOR_Status_Result_ReadWriteError;
            }
            break;

        case RAWSTOR_IOCTL_CMD_GET_SECTOR_COUNT:
        {
            if (fseek (F->fd, 0, SEEK_END))
            {
                return RAWSTOR_Status_Result_ReadWriteError;
            }

            long size = ftell (F->fd);
            if (size < 0)
            {
                return RAWSTOR_Status_Result_ReadWriteError;
            }

            *(uint32_t *) Data = (uint32_t)size / RAWSTOR_SECTOR_SIZE;
            break;
        }

        case RAWSTOR_IOCTL_CMD_GET_SECTOR_SIZE:
            // Get R/W sector size (uint16_t)
            *(uint16_t *) Data = RAWSTOR_SECTOR_SIZE;
            break;

        // Get erase block size in unit of sector (uint32_t)           
        case RAWSTOR_IOCTL_CMD_GET_ERASE_BLOCK_SIZE:
            *(uint32_t *) Data = RAWSTOR_SECTOR_SIZE;
            break;
            
        // Erase a block of sectors (used when FF_USE_TRIM in ffconf.h is 1)
        case RAWSTOR_IOCTL_CMD_TRIM:
            // TODO
            BOARD_AssertSupported (false);
            break;

        default:
            BOARD_AssertUnexpectedValue (R, Cmd);
            break;
    }

	return RAWSTOR_Status_Result_Ok;
}
