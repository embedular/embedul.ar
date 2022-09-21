/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [RAWSTOR] raw storage device driver interface, fatfs compatible.

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

#include "embedul.ar/source/core/timer.h"
#include "embedul.ar/source/core/cyclic.h"


#define RAWSTOR_SECTOR_SIZE                     512

// NOTE: this amount multiplied by the size of SDCARD_StatusLogEntry must give
//       a 2^n number required to initialize the CYCLIC structure used to store
//       the logs.
#define RAWSTOR_STATUS_LOG_ENTRIES              8

// RAWSTOR_Status, RAWSTOR_Result and IOCTRL values and parameters in
// write/read/ioctl operations are equivalents to the FatFs library own values
// for disk interface. The intention is to ensure direct interoperability
// between embedul.ar low level storage funcions and FatFs.
//
// FatFs DSTATUS = RAWSTOR_Status_Disk
// FatFs DRESULT = RAWSTOR_Status_Result
typedef uint8_t     RAWSTOR_Status_Disk;
typedef uint8_t     RAWSTOR_Status_Result;
typedef uint8_t     RAWSTOR_Status_Media;

#define RAWSTOR_Status_Disk_Ok                  0x00
#define RAWSTOR_Status_Disk_NotInitialized      0x01
#define RAWSTOR_Status_Disk_NotPresent          0x02
#define RAWSTOR_Status_Disk_WriteProtected      0x04
#define RAWSTOR_Status_Disk_START_VALUE          \
                                        (RAWSTOR_Status_Disk_NotInitialized | \
                                         RAWSTOR_Status_Disk_NotPresent)

#define RAWSTOR_Status_Result_Ok                0
#define RAWSTOR_Status_Result_ReadWriteError    1
#define RAWSTOR_Status_Result_WriteProtected    2
#define RAWSTOR_Status_Result_NotReady          3
#define RAWSTOR_Status_Result_InvalidParam      4
#define RAWSTOR_Status_Result_Timeout           5

#define RAWSTOR_Status_Media_Undefined          0
#define RAWSTOR_Status_Media_Error              1
#define RAWSTOR_Status_Media_Removed            2
#define RAWSTOR_Status_Media_Inserted           3
#define RAWSTOR_Status_Media_Initializing       4
#define RAWSTOR_Status_Media_Ready              5

// Valid for Disk, Result and Media
#define RAWSTOR_Status_KEEP_VALUE               0xFF

#define RAWSTOR_UpdateStatusDisk(s,d)           RAWSTOR_UpdateStatus(s, \
                                                    d, \
                                                    RAWSTOR_Status_KEEP_VALUE, \
                                                    RAWSTOR_Status_KEEP_VALUE, \
                                                    0, 0)

#define RAWSTOR_UpdateStatusResult(s,r)         RAWSTOR_UpdateStatus(s, \
                                                    RAWSTOR_Status_KEEP_VALUE, \
                                                    r, \
                                                    RAWSTOR_Status_KEEP_VALUE, \
                                                    0, 0)

#define RAWSTOR_UpdateStatusMedia(s,m,e1,e2)    RAWSTOR_UpdateStatus(s, \
                                                    RAWSTOR_Status_KEEP_VALUE, \
                                                    RAWSTOR_Status_KEEP_VALUE, \
                                                    m, e1, e2)


struct RAWSTOR_Status
{
    TIMER_Ticks             ticks;          // Ticks of this status report
    RAWSTOR_Status_Disk     disk;           // Same as ChaN FatFs "DSTATUS"
    RAWSTOR_Status_Result   result;         // Same as ChaN FatFs "DRESULT"
    RAWSTOR_Status_Media    media;          // Media status
    uint8_t                 mediaP1;        // Small param for media status
    uint32_t                mediaP2;        // Big param for media status
};


// Generic commands (Used by FatFs)
#define RAWSTOR_IOCTL_CMD_SYNC                  0   // Complete pending writes
#define RAWSTOR_IOCTL_CMD_GET_SECTOR_COUNT      1
#define RAWSTOR_IOCTL_CMD_GET_SECTOR_SIZE       2
#define RAWSTOR_IOCTL_CMD_GET_ERASE_BLOCK_SIZE  3   // In sectors
#define RAWSTOR_IOCTL_CMD_TRIM                  4   // Data on the block of
                                                    // sectors is no longer used

// Generic commands (Not used by FatFs)
#define RAWSTOR_IOCTL_CMD_POWER                 5   // Get/Set power status
#define RAWSTOR_IOCTL_CMD_LOCK                  6   // Lock/Unlock media removal
#define RAWSTOR_IOCTL_CMD_EJECT                 7   // Eject media
#define RAWSTOR_IOCTL_CMD_FORMAT                8   // Create physical format on
                                                    // the media

// RAWSTOR_IOCTL_CMD_POWER sub command (passed as *data)
#define RAWSTOR_IOCTL_CMD_POWER_OFF             0
#define RAWSTOR_IOCTL_CMD_POWER_ON              1
#define RAWSTOR_IOCTL_CMD_POWER_STATUS          2


struct RAWSTOR;


typedef void (* RAWSTOR_HardwareInitFunc)(struct RAWSTOR *const R);
typedef RAWSTOR_Status_Result (* RAWSTOR_MediaInitFunc)(
                                        struct RAWSTOR *const R);
typedef RAWSTOR_Status_Result (* RAWSTOR_MediaReadFunc)(
                                        struct RAWSTOR *const R,
                                        uint8_t *const Data,
                                        const uint32_t SectorBegin,
                                        const uint32_t SectorCount);
typedef RAWSTOR_Status_Result (* RAWSTOR_MediaWriteFunc)(
                                        struct RAWSTOR *const R,
                                        const uint8_t *const Data,
                                        const uint32_t SectorBegin, 
                                        const uint32_t SectorCount);
typedef RAWSTOR_Status_Result (* RAWSTOR_MediaIoctlFunc)(
                                        struct RAWSTOR *const R,
                                        const uint8_t Cmd, void *const Data);


struct RAWSTOR_IFACE
{
    const char                      * const Description;
    const RAWSTOR_HardwareInitFunc  HardwareInit;
    const RAWSTOR_MediaInitFunc     MediaInit;
    const RAWSTOR_MediaReadFunc     MediaRead;
    const RAWSTOR_MediaWriteFunc    MediaWrite;
    const RAWSTOR_MediaIoctlFunc    MediaIoctl;
};


struct RAWSTOR
{
    const struct RAWSTOR_IFACE      * iface;
    struct RAWSTOR_Status           status;
    struct CYCLIC                   statusLog;
    uint8_t                         statusLogBuffer[RAWSTOR_STATUS_LOG_ENTRIES *
                                                sizeof(struct RAWSTOR_Status)];
    uint32_t                        sectorOffset;
};


void            RAWSTOR_Init                (struct RAWSTOR *const R,
                                             const struct RAWSTOR_IFACE *iface);
struct RAWSTOR_Status
                RAWSTOR_Status              (struct RAWSTOR *const R);
bool            RAWSTOR_PeekStatusLog       (struct RAWSTOR *const R,
                                             uint32_t entry,
                                             struct RAWSTOR_Status *status);
void            RAWSTOR_UpdateStatus        (struct RAWSTOR *const R,
                                             const RAWSTOR_Status_Disk Disk,
                                             const RAWSTOR_Status_Result Result,
                                             const RAWSTOR_Status_Media Media,
                                             const uint8_t MediaP1,
                                             const uint32_t MediaP2);
const char *    RAWSTOR_MediaStatusString   (const RAWSTOR_Status_Media Sm);
bool            RAWSTOR_IsMediaReady        (struct RAWSTOR *const R);
void            RAWSTOR_HardwareInit        (struct RAWSTOR *const R);
RAWSTOR_Status_Result
                RAWSTOR_MediaInit           (struct RAWSTOR *const R);
RAWSTOR_Status_Result
                RAWSTOR_MediaRead           (struct RAWSTOR *const R,
                                             uint8_t *const Data,
                                             const uint32_t SectorBegin,
                                             const uint32_t SectorCount);
RAWSTOR_Status_Result
                RAWSTOR_MediaWrite          (struct RAWSTOR *const R,
                                             const uint8_t *const Data,
                                             const uint32_t SectorBegin,
                                             const uint32_t SectorCount);
RAWSTOR_Status_Result
                RAWSTOR_MediaIoctl          (struct RAWSTOR *const R,
                                             const uint8_t Cmd,
                                             void *const Data);
const char *    RAWSTOR_Description         (struct RAWSTOR *const R);
