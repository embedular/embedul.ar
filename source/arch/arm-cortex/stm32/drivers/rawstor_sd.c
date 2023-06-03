/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [RAWSTOR driver] STM32 SD.

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

#include "embedul.ar/source/arch/arm-cortex/stm32/drivers/rawstor_sd.h"
#include "embedul.ar/source/core/device/board.h"


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


static const struct RAWSTOR_IFACE RAWSTOR_SD_IFACE =
{
    .Description    = "stm32 sd",
    .HardwareInit   = hardwareInit,
    .MediaInit      = mediaInit,
    .MediaRead      = mediaRead,
    .MediaWrite     = mediaWrite,
    .MediaIoctl     = mediaIoctl
};


void RAWSTOR_SD_Init (struct RAWSTOR_SD *const S,
                      SD_HandleTypeDef *const Hsd,
                      const uint32_t ClockEdge,
                      const uint32_t ClockBypass,
                      const uint32_t ClockPowerSave,
                      const uint32_t BusWide,
                      const uint32_t HardwareFlowControl,
                      const uint32_t ClockDiv)
{
    BOARD_AssertParams (S && Hsd);
    BOARD_AssertParams (IS_SDIO_CLOCK_EDGE(ClockEdge));
    BOARD_AssertParams (IS_SDIO_CLOCK_BYPASS(ClockBypass));
    BOARD_AssertParams (IS_SDIO_CLOCK_POWER_SAVE(ClockPowerSave));
    BOARD_AssertParams (IS_SDIO_BUS_WIDE(BusWide));
    BOARD_AssertParams (IS_SDIO_HARDWARE_FLOW_CONTROL(HardwareFlowControl));
    BOARD_AssertParams (IS_SDIO_CLKDIV(ClockDiv));

    Hsd->Instance                   = SDIO;
    Hsd->Init.ClockEdge             = ClockEdge;
    Hsd->Init.ClockBypass           = ClockBypass;
    Hsd->Init.ClockPowerSave        = ClockPowerSave;
    Hsd->Init.BusWide               = BusWide;
    Hsd->Init.HardwareFlowControl   = HardwareFlowControl;
    Hsd->Init.ClockDiv              = ClockDiv;

    S->hsd = Hsd;

    RAWSTOR_Init ((struct RAWSTOR *)S, &RAWSTOR_SD_IFACE);
}


static bool cardDetect (struct RAWSTOR_SD *const S)
{
    (void) S;

    return MIO_GET_INPUT_BIT_NOW(CONTROL, StorageDetect)? true : false;
}


static void hardwareInit (struct RAWSTOR *const R)
{
    struct RAWSTOR_SD *const S = (struct RAWSTOR_SD *) R;

    // Taken from stm32f4xx_hal_sd.c:352 (HAL_SD_Init())
    BOARD_AssertState (S->hsd->State == HAL_SD_STATE_RESET);

    /* Allocate lock resource and initialize it */
    S->hsd->Lock = HAL_UNLOCKED;
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
    /* Reset Callback pointers in HAL_SD_STATE_RESET only */
    hsd->TxCpltCallback    = HAL_SD_TxCpltCallback;
    hsd->RxCpltCallback    = HAL_SD_RxCpltCallback;
    hsd->ErrorCallback     = HAL_SD_ErrorCallback;
    hsd->AbortCpltCallback = HAL_SD_AbortCallback;

    if (hsd->MspInitCallback == NULL)
    {
        hsd->MspInitCallback = HAL_SD_MspInit;
    }

    /* Init the low level hardware */
    hsd->MspInitCallback (hsd);
#else
    /* Init the low level hardware : GPIO, CLOCK, CORTEX...etc */
    HAL_SD_MspInit (S->hsd);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */

    // Initial status
    if (cardDetect (S))
    {
        R->status.disk &= ~RAWSTOR_Status_Disk_NotPresent;
        RAWSTOR_UpdateStatusMedia (R, RAWSTOR_Status_Media_Inserted, 0, 0);
    }
    else
    {
        RAWSTOR_UpdateStatusMedia (R, RAWSTOR_Status_Media_Removed, 0, 0);
    }
}


static RAWSTOR_Status_Result mediaInit (struct RAWSTOR *const R)
{
    struct RAWSTOR_SD *const S = (struct RAWSTOR_SD *) R;

    if (R->status.disk & RAWSTOR_Status_Disk_NotPresent)
    {
        if (!cardDetect (S))
        {
            return RAWSTOR_Status_Result_NotReady;
        }

        R->status.disk &= ~RAWSTOR_Status_Disk_NotPresent;
        RAWSTOR_UpdateStatusMedia (R, RAWSTOR_Status_Media_Inserted, 0, 0);
    }

    RAWSTOR_UpdateStatusMedia (R, RAWSTOR_Status_Media_Initializing,
                               SDCARD_Status_Media_Initializing_P1_PowerOn,
                               0);

    // Taken from stm32f4xx_hal_sd.c:376 (HAL_SD_Init())
    S->hsd->State = HAL_SD_STATE_BUSY;

    const HAL_StatusTypeDef CardInit    = HAL_SD_InitCard (S->hsd);
    const HAL_StatusTypeDef CardStatus  = HAL_SD_GetCardStatus (S->hsd,
                                                            &S->cardStatus);

    if (CardInit == HAL_OK && CardStatus == HAL_OK)
    {
        /* Initialize the error code */
        S->hsd->ErrorCode = HAL_SD_ERROR_NONE;

        /* Initialize the SD operation */
        S->hsd->Context = SD_CONTEXT_NONE;

        /* Initialize the SD state */
        S->hsd->State = HAL_SD_STATE_READY;

        R->status.disk &= ~RAWSTOR_Status_Disk_NotInitialized;
        RAWSTOR_UpdateStatusMedia (R, RAWSTOR_Status_Media_Ready, 0, 0);
        RAWSTOR_UpdateStatusResult (R, RAWSTOR_Status_Result_Ok);
    }
    else
    {
        S->hsd->State = HAL_SD_STATE_ERROR;

        RAWSTOR_UpdateStatusMedia (R, RAWSTOR_Status_Media_Error, 0, 0);
        RAWSTOR_UpdateStatusResult (R, RAWSTOR_Status_Result_NotReady);
    }

    return R->status.result;
}


static RAWSTOR_Status_Result mediaRead (struct RAWSTOR *const R,
                                        uint8_t *const Data,
                                        const uint32_t SectorBegin,
                                        const uint32_t SectorCount)
{
    struct RAWSTOR_SD *const S = (struct RAWSTOR_SD *) R;

    const HAL_StatusTypeDef Hs = HAL_SD_ReadBlocks (S->hsd, Data, SectorBegin, 
                                                    SectorCount, 500);

    return (Hs == HAL_OK)? RAWSTOR_Status_Result_Ok :
                           RAWSTOR_Status_Result_ReadWriteError;
}


static RAWSTOR_Status_Result mediaWrite (struct RAWSTOR *const R, 
                                         const uint8_t *const Data,
                                         const uint32_t SectorBegin,
                                         const uint32_t SectorCount)
{
    struct RAWSTOR_SD *const S = (struct RAWSTOR_SD *) R;

    const HAL_StatusTypeDef Hs = HAL_SD_WriteBlocks (S->hsd, (uint8_t *)Data,
                                                     SectorBegin, SectorCount,
                                                     500);

    return (Hs == HAL_OK)? RAWSTOR_Status_Result_Ok :
                           RAWSTOR_Status_Result_ReadWriteError;
}


static RAWSTOR_Status_Result mediaIoctl (struct RAWSTOR *const R,
                                         const uint8_t Cmd,
                                         void *const Data)
{
    struct RAWSTOR_SD *const S = (struct RAWSTOR_SD *) R;

	RAWSTOR_Status_Result   res;
	uint8_t                 *const Ptr = Data;

	res = RAWSTOR_Status_Result_ReadWriteError;
    
	if (Cmd == RAWSTOR_IOCTL_CMD_POWER)
    {
		switch (*Ptr) 
        {
            case RAWSTOR_IOCTL_CMD_POWER__OFF:
                res = RAWSTOR_Status_Result_InvalidParam;
                break;
                
            case RAWSTOR_IOCTL_CMD_POWER__ON:
                res = RAWSTOR_Status_Result_InvalidParam;
                break;

            case RAWSTOR_IOCTL_CMD_POWER__STATUS:
                *(Ptr+1) = 1;
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
            res = RAWSTOR_Status_Result_Ok;
            break;

        case RAWSTOR_IOCTL_CMD_GET_SECTOR_COUNT:
            // Get number of sectors on the disk (uint32_t)
            *(uint32_t *) Data = (uint32_t)(S->hsd->SdCard.BlockNbr);
            res = RAWSTOR_Status_Result_Ok;
            break;

        case RAWSTOR_IOCTL_CMD_GET_SECTOR_SIZE:
            *(uint16_t *) Data = (uint16_t)(S->hsd->SdCard.BlockSize);
            res = RAWSTOR_Status_Result_Ok;
            break;

        // Get erase block size in unit of sector (uint32_t)           
        case RAWSTOR_IOCTL_CMD_GET_ERASE_BLOCK_SIZE:
            *(uint32_t *) Data = S->cardStatus.EraseSize;
            res = RAWSTOR_Status_Result_Ok;
            break;

        // ---------------------------------------------------------------------
        // The following commands are never used by FatFs module
        // ---------------------------------------------------------------------
        case RAWSTOR_IOCTL_SDCARD_GET_TYPE:
            // Get card type flags (1 byte)
            *Ptr = (uint8_t)(S->hsd->SdCard.CardType);
            res = RAWSTOR_Status_Result_Ok;
            break;

        case RAWSTOR_IOCTL_SDCARD_GET_CSD:
            // Receive CSD as a data block (16 bytes)
            *((uint32_t *) Data + 0) = S->hsd->CSD[0];
            *((uint32_t *) Data + 1) = S->hsd->CSD[1];
            *((uint32_t *) Data + 2) = S->hsd->CSD[2];
            *((uint32_t *) Data + 3) = S->hsd->CSD[3];
            res = RAWSTOR_Status_Result_Ok;
            break;

        case RAWSTOR_IOCTL_SDCARD_GET_CID:
            // Receive CID as a data block (16 bytes)
            *((uint32_t *) Data + 0) = S->hsd->CID[0];
            *((uint32_t *) Data + 1) = S->hsd->CID[1];
            *((uint32_t *) Data + 2) = S->hsd->CID[2];
            *((uint32_t *) Data + 3) = S->hsd->CID[3];
            res = RAWSTOR_Status_Result_Ok;
            break;
/*
        case RAWSTOR_IOCTL_SDCARD_GET_SDSTAT:
            // Receive SD stats
            if ((stats = Chip_SDMMC_GetState(LPC_SDMMC)) != -1)
            {
                *((uint32_t *) Data + 0) = (uint32_t) stats;
                res = RAWSTOR_Status_Result_Ok;
            }
            break;
*/
        default:
            BOARD_AssertParams (false);
            break;
    }

	return res;
}
