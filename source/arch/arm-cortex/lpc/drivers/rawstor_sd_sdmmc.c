/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [RAWSTOR driver] LPCOpen SD/MMC.

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

#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/rawstor_sd_sdmmc.h"
#include "embedul.ar/source/core/device/board.h"


//#define SUPPORTS_HOT_INSERTION

#ifndef SPI_SLOW_CLOCK
#define SPI_SLOW_CLOCK               100000
#endif

#ifndef SPI_FAST_CLOCK
#define SPI_FAST_CLOCK               14000000
#endif

#define WAIT_READY_SELECT_TIMEOUT    500
#define DATASTART_TOKEN_TIMEOUT      200
#define WAIT_READY_XMIT_TIMEOUT      500
#define INITIALIZATION_TIMEOUT       2000

// MMC/SDC commands according to SD Group "Physical Layer Simplified 
// Specification Version 6.00"
#define SD_CMD0                 0           // GO_IDLE_STATE
#define SD_CMD1                 1           // SEND_OP_COND (MMC)
#define	SD_ACMD41               (0x80+41)   // SEND_OP_COND (SDC)
#define SD_CMD8                 8           // SEND_IF_COND
#define SD_CMD9                 9           // SEND_CSD
#define SD_CMD10                10          // SEND_CID
#define SD_CMD12                12          // STOP_TRANSMISSION
#define SD_ACMD13               (0x80+13)	// SD_STATUS (SDC)
#define SD_CMD16                16          // SET_BLOCKLEN
#define SD_CMD17                17          // READ_SINGLE_BLOCK
#define SD_CMD18                18          // READ_MULTIPLE_BLOCK
#define SD_CMD23                23          // SET_BLOCK_COUNT (MMC)
#define SD_ACMD23               (0x80+23)   // SET_WR_BLK_ERASE_COUNT (SDC)
#define SD_CMD24                24          // WRITE_BLOCK
#define SD_CMD25                25          // WRITE_MULTIPLE_BLOCK
#define SD_CMD32                32          // ERASE_ER_BLK_START
#define SD_CMD33                33          // ERASE_ER_BLK_END
#define SD_CMD38                38          // ERASE
// Faltan 48,49
#define SD_CMD55                55          // APP_CMD
#define SD_CMD58                58          // READ_OCR


// Common IO interface
static void                     hardwareInit    (struct RAWSTOR *const R);
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


// Based on periph_sdmmc example from LPC4337 LPCOpen library
static volatile int32_t sdio_wait_exit = 0;


static const struct RAWSTOR_IFACE RAWSTOR_SD_SDMMC_IFACE =
{
    .Description    = "lpcopen sdmmc",
    .HardwareInit   = hardwareInit,
    .MediaInit      = mediaInit,
    .MediaRead      = mediaRead,
    .MediaWrite     = mediaWrite,
    .MediaIoctl     = mediaIoctl
};


void RAWSTOR_SD_SDMMC_Init (struct RAWSTOR_SD_SDMMC *const M)
{
    BOARD_AssertParams (M);

    RAWSTOR_Init ((struct RAWSTOR *)M, &RAWSTOR_SD_SDMMC_IFACE);
}


// LPCOpen code based on periph_sdmmc example from the same library.

// Delay callback for timed SDIF/SDMMC functions
static void sdmmc_waitms (uint32_t time)
{
    TIMER_Ticks timeout = BOARD_TicksNow() + time;
    while (BOARD_TicksNow() < timeout);

    return;
}


// Sets up the SD event driven wakeup
// bits: Status bits to poll for command completion
static void sdmmc_setup_wakeup (void *bits)
{
	uint32_t bit_mask = *((uint32_t *)bits);
	// Wait for IRQ - for an RTOS, you would pend on an event here with a IRQ
    // based wakeup.
	NVIC_ClearPendingIRQ (SDIO_IRQn);
	sdio_wait_exit = 0;
	Chip_SDIF_SetIntMask (LPC_SDMMC, bit_mask);
	NVIC_EnableIRQ (SDIO_IRQn);
}


// A better wait callback for SDMMC driven by the IRQ flag
// return 0 on success, or failure condition (-1)
static uint32_t sdmmc_irq_driven_wait (void)
{
	uint32_t status;

	/* Wait for event, would be nice to have a timeout, but keep it  simple */
	while (sdio_wait_exit == 0) {}

	/* Get status and clear interrupts */
	status = Chip_SDIF_GetIntStatus (LPC_SDMMC);
	Chip_SDIF_ClrIntStatus (LPC_SDMMC, status);
	Chip_SDIF_SetIntMask (LPC_SDMMC, 0);

	return status;
}


// SDIO controller interrupt handler
void SDIO_IRQHandler (void)
{
	/* All SD based register handling is done in the callback
	   function. The SDIO interrupt is not enabled as part of this
	   driver and needs to be enabled/disabled in the callbacks or
	   application as needed. This is to allow flexibility with IRQ
	   handling for applicaitons and RTOSes. */
	/* Set wait exit flag to tell wait function we are ready. In an RTOS,
	   this would trigger wakeup of a thread waiting for the IRQ. */
	NVIC_DisableIRQ (SDIO_IRQn);
	sdio_wait_exit = 1;
}


// Power Control  (Platform dependent)
// When the target system does not support socket power control, there
// is nothing to do in these functions and powerCheck() always returns 1.
static void powerOn (struct RAWSTOR_SD_SDMMC *const M)
{
    (void) M;

    OUTPUT_SET_BIT_NOW (CONTROL, StoragePower, 1);

    // Wait a couple of milliseconds for power to stabilize
    BOARD_Delay (200);
}


static void powerOff (struct RAWSTOR_SD_SDMMC *const M)
{
    (void) M;

    OUTPUT_SET_BIT_NOW (CONTROL, StoragePower, 0);

    // Wait a couple of milliseconds for power to stabilize
    BOARD_Delay (200);
}


static bool powerCheck (struct RAWSTOR_SD_SDMMC *const M)
{
    (void) M;

    return INPUT_GET_BIT_NOW(CONTROL,StoragePower)? true : false;
}


static bool cardDetect (struct RAWSTOR *const R)
{
    (void) R;

    return (!LPC_SDMMC->CDETECT)? true : false;
}


static bool cardReadyWait (struct RAWSTOR *const R, uint32_t time)
{
    (void) R;

    TIMER_Ticks timeout = BOARD_TicksNow() + time;
    while (BOARD_TicksNow() < timeout
           && Chip_SDMMC_GetState(LPC_SDMMC) != -1);

    return (Chip_SDMMC_GetState(LPC_SDMMC) == -1)? false : true;
}


static void hardwareInit (struct RAWSTOR *const R)
{
    struct RAWSTOR_SD_SDMMC *const M = (struct RAWSTOR_SD_SDMMC *) R;

    powerOff (M);

    M->sdcardinfo.card_info.evsetup_cb      = sdmmc_setup_wakeup;
	M->sdcardinfo.card_info.waitfunc_cb     = sdmmc_irq_driven_wait;
	M->sdcardinfo.card_info.msdelay_func    = sdmmc_waitms;

    /* The SDIO driver needs to know the SDIO clock rate */
	Chip_SDIF_Init (LPC_SDMMC);

    // Initial status
    if (cardDetect (R))
    {
        R->status.disk &= ~RAWSTOR_Status_Disk_NotPresent;
        RAWSTOR_UpdateStatusMedia (R, RAWSTOR_Status_Media_Inserted, 0, 0);
    }
    else
    {
        RAWSTOR_UpdateStatusMedia (R, RAWSTOR_Status_Media_Removed, 0, 0);
    }

	/* Enable SD/MMC Interrupt */
	NVIC_EnableIRQ  (SDIO_IRQn);
}


static RAWSTOR_Status_Result mediaInit (struct RAWSTOR *const R)
{
    struct RAWSTOR_SD_SDMMC *m = (struct RAWSTOR_SD_SDMMC *) R;

    if (R->status.disk & RAWSTOR_Status_Disk_NotPresent)
    {
        if (!cardDetect (R))
        {
            return RAWSTOR_Status_Result_NotReady;
        }

        R->status.disk &= ~RAWSTOR_Status_Disk_NotPresent;
        RAWSTOR_UpdateStatusMedia (R, RAWSTOR_Status_Media_Inserted, 0, 0);
    }

    RAWSTOR_UpdateStatusMedia (R, RAWSTOR_Status_Media_Initializing,
                               SDCARD_Status_Media_Initializing_P1_PowerOn,
                               0);

    // Not really in native mode, but there is no way to update
    // SDCARD_Status_Media_Initializing until Chip_SDMMC_Aquire() either 
    // succeeds or fails.
    RAWSTOR_UpdateStatusMedia (R, RAWSTOR_Status_Media_Initializing,
                               SDCARD_Status_Media_Initializing_P1_NativeMode,
                               0);

    // Socket power on
    powerOn (m);

    // Enumerate SD card
    if (!Chip_SDMMC_Acquire (LPC_SDMMC, &m->sdcardinfo))
    {
        // Failed
        powerOff (m);

        RAWSTOR_UpdateStatusMedia (R, RAWSTOR_Status_Media_Error, 0, 0);
        RAWSTOR_UpdateStatusResult (R, RAWSTOR_Status_Result_NotReady);
    }
    else
    {
        R->status.disk &= ~RAWSTOR_Status_Disk_NotInitialized;
        RAWSTOR_UpdateStatusMedia (R, RAWSTOR_Status_Media_Ready, 0, 0);
        RAWSTOR_UpdateStatusResult (R, RAWSTOR_Status_Result_Ok);
    }

    return R->status.result;
}


static RAWSTOR_Status_Result mediaRead (struct RAWSTOR *const R,
                                        uint8_t *const Data,
                                        const uint32_t SectorBegin,
                                        const uint32_t SectorCount)
{
    (void) R;

    const uint32_t r = Chip_SDMMC_ReadBlocks (LPC_SDMMC, Data, SectorBegin,
                                              SectorCount);

    return r? RAWSTOR_Status_Result_Ok :
              RAWSTOR_Status_Result_ReadWriteError;
}


static RAWSTOR_Status_Result mediaWrite (struct RAWSTOR *const R, 
                                         const uint8_t *const Data,
                                         const uint32_t SectorBegin,
                                         const uint32_t SectorCount)
{
    (void) R;

    const uint32_t r = Chip_SDMMC_WriteBlocks (LPC_SDMMC, Data, SectorBegin,
                                               SectorCount);

    return r? RAWSTOR_Status_Result_Ok :
              RAWSTOR_Status_Result_ReadWriteError;
}


static RAWSTOR_Status_Result mediaIoctl (struct RAWSTOR *const R,
                                         const uint8_t Cmd,
                                         void *const Data)
{
    struct RAWSTOR_SD_SDMMC *m = (struct RAWSTOR_SD_SDMMC *) R;

	RAWSTOR_Status_Result   res;
    int32_t                 stats;
	uint8_t                 *ptr = Data;

	res = RAWSTOR_Status_Result_ReadWriteError;
    
	if (Cmd == RAWSTOR_IOCTL_CMD_POWER)
    {
		switch (*ptr) 
        {
            case RAWSTOR_IOCTL_CMD_POWER_OFF:
                if (powerCheck (m))
                {
                    powerOff (m);
                }
                res = RAWSTOR_Status_Result_Ok;
                break;
                
            case RAWSTOR_IOCTL_CMD_POWER_ON:
                powerOn (m);
                res = RAWSTOR_Status_Result_Ok;
                break;
                
            case RAWSTOR_IOCTL_CMD_POWER_STATUS:
                *(ptr+1) = powerCheck (m)? 1 : 0;
                res = RAWSTOR_Status_Result_Ok;
                break;
                
            default:
                BOARD_AssertParams (false);
                break;
		}
        
        return res;
    }
        
    if (R->status.disk & RAWSTOR_Status_Disk_NotInitialized)
    {
        return RAWSTOR_Status_Result_NotReady;
    }

    switch (Cmd) 
    {
        case RAWSTOR_IOCTL_CMD_SYNC:
            /* Wait for end of internal write process of the drive */
            if (cardReadyWait (R, 50))
            {
                res = RAWSTOR_Status_Result_Ok;
            }
            break;

        case RAWSTOR_IOCTL_CMD_GET_SECTOR_COUNT:
            // Get number of sectors on the disk (uint32_t)
            *(uint32_t *) Data = m->sdcardinfo.card_info.blocknr;
            res = RAWSTOR_Status_Result_Ok;
            break;

        case RAWSTOR_IOCTL_CMD_GET_SECTOR_SIZE:
            // Get R/W sector size (uint16_t)
            // NOTE:
            // It's strange how _MMC_Acquire() stores the actual SD card block
            // size (block_len) but then adjusts the card total block capacity
            // (blocknr) to 512-byte blocks. FatFS and retrociaa assume 512
            // byte blocks and perform reads and writes on them. For example,
            // Chip_SDMMC_Write/ReadBlocks() will take a 512 byte start sector
            // and total number of equally sized sectors. This is why the code
            // returns a sector size of 512 and not m->sdcardinfo.card_info.
            // block_len.
            *(uint16_t *) Data = RAWSTOR_SECTOR_SIZE;
            res = RAWSTOR_Status_Result_Ok;
            break;

        // Get erase block size in unit of sector (uint32_t)           
        case RAWSTOR_IOCTL_CMD_GET_ERASE_BLOCK_SIZE:
            // Taken from fsmci_cfg.h. I Have no idea why they're using a fixed
            // size or its correctness.
            // ---------------------------------------------------------
            // Get the size of one erase block in the card (Fixed to 4K)
            *(uint32_t *) Data = (4UL * 1024);
            res = RAWSTOR_Status_Result_Ok;
            break;
            
        // ---------------------------------------------------------------------
        // The following commands are never used by FatFs module
        // ---------------------------------------------------------------------
        case SDCARD_IOCTL_GET_TYPE:
            // Get card type flags (1 byte)
            *ptr = (uint8_t) m->sdcardinfo.card_info.card_type;
            res = RAWSTOR_Status_Result_Ok;
            break;

        case SDCARD_IOCTL_GET_CSD:
            // Receive CSD as a data block (16 bytes)
            *((uint32_t *) Data + 0) = m->sdcardinfo.card_info.csd[0];
            *((uint32_t *) Data + 1) = m->sdcardinfo.card_info.csd[1];
            *((uint32_t *) Data + 2) = m->sdcardinfo.card_info.csd[2];
            *((uint32_t *) Data + 3) = m->sdcardinfo.card_info.csd[3];
            res = RAWSTOR_Status_Result_Ok;
            break;

        case SDCARD_IOCTL_GET_CID:
            // Receive CID as a data block (16 bytes)
            *((uint32_t *) Data + 0) = m->sdcardinfo.card_info.cid[0];
            *((uint32_t *) Data + 1) = m->sdcardinfo.card_info.cid[1];
            *((uint32_t *) Data + 2) = m->sdcardinfo.card_info.cid[2];
            *((uint32_t *) Data + 3) = m->sdcardinfo.card_info.cid[3];
            res = RAWSTOR_Status_Result_Ok;
            break;

        case SDCARD_IOCTL_GET_SDSTAT:
            // Receive SD stats
            if ((stats = Chip_SDMMC_GetState(LPC_SDMMC)) != -1)
            {
                *((uint32_t *) Data + 0) = (uint32_t) stats;
                res = RAWSTOR_Status_Result_Ok;
            }
            break;

        default:
            BOARD_AssertParams (false);
            break;
    }

	return res;
}
