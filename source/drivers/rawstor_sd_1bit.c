/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [RAWSTOR driver] generic 1-bit sd card interface.

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

#include "embedul.ar/source/drivers/rawstor_sd_1bit.h"
#include "embedul.ar/source/core/device/board.h"


//#define SUPPORTS_HOT_INSERTION

#ifndef SPI_SLOW_CLOCK
#define SPI_SLOW_CLOCK              100000
#endif

#ifndef SPI_FAST_CLOCK
#define SPI_FAST_CLOCK              14000000
#endif

#define WAIT_READY_SELECT_TIMEOUT   500
#define DATASTART_TOKEN_TIMEOUT     200
#define WAIT_READY_XMIT_TIMEOUT     500
#define INITIALIZATION_TIMEOUT      2000

// MMC/SDC commands according to SD Group "Physical Layer Simplified 
// Specification Version 6.00"
#define SD_CMD0                     0           // GO_IDLE_STATE
#define SD_CMD1                     1           // SEND_OP_COND (MMC)
#define	SD_ACMD41                   (0x80+41)   // SEND_OP_COND (SDC)
#define SD_CMD8                     8           // SEND_IF_COND
#define SD_CMD9                     9           // SEND_CSD
#define SD_CMD10                    10          // SEND_CID
#define SD_CMD12                    12          // STOP_TRANSMISSION
#define SD_ACMD13                   (0x80+13)   // SD_STATUS (SDC)
#define SD_CMD16                    16          // SET_BLOCKLEN
#define SD_CMD17                    17          // READ_SINGLE_BLOCK
#define SD_CMD18                    18          // READ_MULTIPLE_BLOCK
#define SD_CMD23                    23          // SET_BLOCK_COUNT (MMC)
#define SD_ACMD23                   (0x80+23)   // SET_WR_BLK_ERASE_COUNT (SDC)
#define SD_CMD24                    24          // WRITE_BLOCK
#define SD_CMD25                    25          // WRITE_MULTIPLE_BLOCK
#define SD_CMD32                    32          // ERASE_ER_BLK_START
#define SD_CMD33                    33          // ERASE_ER_BLK_END
#define SD_CMD38                    38          // ERASE
// 48,49 Missing
#define SD_CMD55                    55          // APP_CMD
#define SD_CMD58                    58          // READ_OCR


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


static const struct RAWSTOR_IFACE RAWSTOR_SD_1BIT_IFACE =
{
    .Description    = "generic 1-bit sdcard",
    .HardwareInit   = hardwareInit,
    .MediaInit      = mediaInit,
    .MediaRead      = mediaRead,
    .MediaWrite     = mediaWrite,
    .MediaIoctl     = mediaIoctl
};


void RAWSTOR_SD_1BIT_Init (struct RAWSTOR_SD_1BIT *const S,
                           const enum COMM_Packet Com)
{
    BOARD_AssertParams (S);

    S->tFlags   = RAWSTOR_SD_TYPE_UNKNOWN;
    S->packet   = COMM_GetPacket (Com);

    RAWSTOR_Init ((struct RAWSTOR *)S, &RAWSTOR_SD_1BIT_IFACE);
}


// Low level private functions
// "CSLow" = Enable card on the spi bus
static inline void setCSLow (struct RAWSTOR_SD_1BIT *const S)
{
    (void) S;

    OUTPUT_SET_BIT_NOW (CONTROL, StorageEnable, 1);
}


// "CSHigh" = Disable card on the spi bus
static inline void setCSHigh (struct RAWSTOR_SD_1BIT *const S)
{
    (void) S;

    OUTPUT_SET_BIT_NOW (CONTROL, StorageEnable, 0);
}


static inline void setSlowClock (struct RAWSTOR_SD_1BIT *const S)
{
    /* Set slow clock (100k-400k) */
    PACKET_Command (S->packet, DEVICE_COMMAND_SET_SPEED, 
                    &VARIANT_SpawnUint(SPI_SLOW_CLOCK));
}


static inline void setFastClock (struct RAWSTOR_SD_1BIT *const S)
{
    /* Set fast clock (depends on the CSD) */
    PACKET_Command (S->packet, DEVICE_COMMAND_SET_SPEED, 
                    &VARIANT_SpawnUint(SPI_FAST_CLOCK));
}


static void sendBuffer (struct RAWSTOR_SD_1BIT *const S, 
                        const uint8_t *const Data, const uint32_t Size)
{
    PACKET_Send (S->packet, Data, Size);
}


static void sendOctet (struct RAWSTOR_SD_1BIT *const S, const uint8_t Octet)
{
    sendBuffer (S, &Octet, 1);
}


static void recvBuffer (struct RAWSTOR_SD_1BIT *const S,
                        uint8_t *const Data, const uint32_t Size)
{
    PACKET_Recv (S->packet, Data, Size);
}


static uint8_t recvOctet (struct RAWSTOR_SD_1BIT *const S)
{
    uint8_t octet;
    // On SPI, it send 0xFF for each actual octet received. 
    recvBuffer (S, &octet, 1);
    return octet;
}


static bool waitForCardReady (struct RAWSTOR_SD_1BIT *const S, uint32_t timeout)
{
	uint8_t d;
    
	timeout += TICKS_Now();
	do
    {
		d = recvOctet (S);
    }
	while (d != 0xFF && TICKS_Now() < timeout);

	return (d == 0xFF)? true : false;
}


static void deselectCard (struct RAWSTOR_SD_1BIT *const S)
{
    // Deselect the card and release SPI bus by a dummy clock (force DO
    // hi-z for multiple slave SPI)
    setCSHigh   (S);
    recvOctet   (S);
}


static bool selectCard (struct RAWSTOR_SD_1BIT *const S)
{
    // Select the card then a dummy clock (force DO enabled)
	setCSLow    (S);
    recvOctet   (S);

    // Wait for card ready on a leading busy check
	if (waitForCardReady (S, WAIT_READY_SELECT_TIMEOUT))
    {
        return true;
    }

    // Timeout
	deselectCard (S);
    return false;
}


// Power Control  (Platform dependent)
// When the target system does not support socket power control, there
// is nothing to do in these functions and powerCheck() always returns true.
static void powerOn (struct RAWSTOR_SD_1BIT *const S)
{
    (void) S;

    OUTPUT_SET_BIT_NOW (CONTROL, StoragePower, 1);
}


static void powerOff (struct RAWSTOR_SD_1BIT *const S)
{
    (void) S;

    OUTPUT_SET_BIT_NOW (CONTROL, StoragePower, 0);
}


static bool powerStatus (struct RAWSTOR_SD_1BIT *const S)
{
    (void) S;

    if (!INPUT_BIT_IS_MAPPED(CONTROL, StoragePower))
    {
        return true;
    }

    const bool PowerStatus = 
                    INPUT_GET_BIT_NOW(CONTROL,StoragePower)? true : false;

    return PowerStatus;
}


static bool recvCardDatablock (struct RAWSTOR_SD_1BIT *const S, 
                               uint8_t *const Data, const uint32_t Size)
{
    const TIMER_Ticks Timeout = TICKS_Now() + DATASTART_TOKEN_TIMEOUT;
	uint8_t token;

    // Wait until DataStart token or timeout
	do
    {
		token = recvOctet (S);
	} 
    while (token == 0xFF && TICKS_Now() < Timeout);

    // Function fails if invalid DataStart token or timeout
	if (token != 0xFE)
    {
        return false;
    }

    // Store trailing data to the buffer
    recvBuffer  (S, Data, Size);
    // Discard CRC
	recvOctet   (S);
	recvOctet   (S);

	return true;
}


static bool sendCardDatablock (struct RAWSTOR_SD_1BIT *const S, 
                               const uint8_t *const Data, const uint8_t Token)
{
    // Leading busy check: Wait for card ready to accept data block
	if (!waitForCardReady (S, WAIT_READY_XMIT_TIMEOUT))
    {
        return false;
    }

	sendOctet (S, Token);
    // Do not send data if token is StopTran
    if (Token == 0xFD)
    {
        return true;
    }
    
    sendBuffer  (S, Data, 512);
    // Dummy CRC
    sendOctet   (S, 0xFF);
    sendOctet   (S, 0xFF);
    
    // Reveive data response
    const uint8_t Resp = recvOctet (S);
    
    // Data was accepted or not
    // (Busy check is done at next transmission)
    return ((Resp & 0x1F) == 0x05)? true : false;    
}


// Return value: R1 response (bit7 == 1: Failed to send)
static uint8_t sendCardCommand (struct RAWSTOR_SD_1BIT *const S, 
                                const uint8_t Command, const uint32_t Arg)
{
    uint8_t res;
  
    const uint8_t Cmd = (Command & 0x80)? Command & 0x7F : Command;

    // Send a SD_CMD55 prior to ACMD<n>
    if (Command & 0x80)
    {
		res = sendCardCommand (S, SD_CMD55, 0);
		if (res > 1) 
        {
            return res;
        }
	}

	// Select the card and wait for ready except to stop multiple block read
    if (Cmd != SD_CMD12)
    {
        deselectCard (S);
        if (!selectCard (S))
        {
            return 0xFF;
        }
    }
    
	// Send command packet
	sendOctet (S, 0x40 | Cmd);              // Start (01xxxxxx) + Command index
	sendOctet (S, (uint8_t) (Arg >> 24));	// Argument[31..24]
	sendOctet (S, (uint8_t) (Arg >> 16));	// Argument[23..16]
	sendOctet (S, (uint8_t) (Arg >>  8));   // Argument[15..8]
	sendOctet (S, (uint8_t)  Arg);			// Argument[7..0]
    
    // Valid CRC + Stop bit for SD_CMD0(0) -or-
    // Valid CRC + Stop bit for SD_CMD8(0x1AA) -or-
    // Dummy CRC + Stop bit
	uint8_t n = (Cmd == SD_CMD0)? 0x95 : (Cmd == SD_CMD8)? 0x87 : 0x01;
    
	sendOctet (S, n);

	// Receive command response
	if (Cmd == SD_CMD12)
    {
        // Discard following one byte when SD_CMD12
        recvOctet (S);
    }

    // Wait for response (10 bytes max)
	n = 10;
	do
    {
		res = recvOctet (S);
    }
	while ((res & 0x80) && --n);

    // Return received response
	return res;
}


static bool detectCard (struct RAWSTOR *const R)
{
    (void) R;

    return INPUT_GET_BIT_NOW(CONTROL,StorageDetect)? true : false;
}


static void hardwareInit (struct RAWSTOR *const R)
{
    struct RAWSTOR_SD_1BIT *const P = (struct RAWSTOR_SD_1BIT *) R;

    /*
        http://elm-chan.org/docs/mmc/mmc_e.html#spimode
        Citing ELM-ChaN:
            "the data is transferred in byte oriented serial communication.
            The SPI mode 0 is defined for SDC. For the MMC, it is not the SPI
            spec, both latch and shift operations are defined with rising edge
            of the SCLK, but it seems to work at mode 0 at the SPI mode. Thus
            the SPI mode 0 (CPHA=0, CPOL=0) is the proper setting to control
            MMC/SDC."
    */

    PACKET_Command (P->packet, DEVICE_COMMAND_SET_FRAME_BITS,
                    &VARIANT_SpawnUint(8));
    PACKET_Command (P->packet, DEVICE_COMMAND_SET_SPEED,
                    &VARIANT_SpawnUint(SPI_SLOW_CLOCK));

    deselectCard (P);

    // Initial status
    if (detectCard (R))
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
    struct RAWSTOR_SD_1BIT *const P = (struct RAWSTOR_SD_1BIT *) R;

    if (R->status.disk & RAWSTOR_Status_Disk_NotPresent)
    {
        if (!detectCard (R))
        {
            return RAWSTOR_Status_Result_NotReady;
        }

        R->status.disk &= ~RAWSTOR_Status_Disk_NotPresent;
        RAWSTOR_UpdateStatusMedia (R, RAWSTOR_Status_Media_Inserted, 0, 0);
    }
    
    RAWSTOR_UpdateStatusMedia (R, RAWSTOR_Status_Media_Initializing,
                        RAWSTOR_SD_STATUS_MEDIA_INITIALIZING__P1_POWER_ON,
                        0);

    // According to http://elm-chan.org/docs/mmc/mmc_e.html
    // -------------------------------------------------------------------------
    // Socket power on
	powerOn (P);
    // After supply voltage reached 2.2 volts, wait for one millisecond at 
    // least.
    TIMER_Ticks timeout = TICKS_Now() + 20;
    while (TICKS_Now() < timeout)
    {
        __WFI ();
    }
    // Set SPI clock rate between 100 kHz and 400 kHz. 
    setSlowClock (P);
    // Set DI and CS high (recvByte sends 0xFF)
    recvOctet (P);
    setCSHigh (P);
    // and apply 74 or more clock pulses to SCLK. The card will enter its native
    // operating mode and go ready to accept native command.
  	for (uint32_t n = 10; n; n--) 
    {
        // clock pulses (n * 8)
        recvOctet (P);
    }
    
    // Now in native operating mode accepting native commands
    RAWSTOR_UpdateStatusMedia (R, RAWSTOR_Status_Media_Initializing,
                        RAWSTOR_SD_STATUS_MEDIA_INITIALIZING__P1_NATIVE_MODE,
                        0);

    // The TRAN_SPEED field in the CSD register indicates the maximum clock rate
    // of the card. The maximum clock rate is 20MHz for MMC, 25MHz for SDC in 
    // most cases.
    
	uint8_t ty = 0;
    // Card reset
	if (sendCardCommand (P, SD_CMD0, 0) == 1)
    {
        // Now on IDLE state
        RAWSTOR_UpdateStatusMedia (R, RAWSTOR_Status_Media_Initializing,
                            RAWSTOR_SD_STATUS_MEDIA_INITIALIZING__P1_IDLE,
                            0);

        timeout = TICKS_Now() + INITIALIZATION_TIMEOUT;
        
        // SD_CMD8 "Send Interface Condition Command".
        // 0x0AA = Check pattern. 0x100 = 2.7-3.6V
        // Is the card SDv2? / SDHC
        if (sendCardCommand (P, SD_CMD8, 0x1AA) == 1)
        {          
            uint8_t ocr[4];
            // Get 32 bit return value of R7 resp
            recvBuffer (P, ocr, 4);
            // Voltage & check pattern should match.
            // Does the card support 2.7-3.6V?
			if (ocr[2] == 0x01 && ocr[3] == 0xAA)
            {
                // SD_ACMD41 "Initialization Command"
                // bit 30 HCS: "Host Capacity Support" 1b: SDHC or SDXC 
                // Supported. Wait for leaving idle state (SD_ACMD41 with
                // HCS bit)
				while (TICKS_Now() < timeout &&
                       sendCardCommand (P, SD_ACMD41, 1UL << 30))
                {
                }

                if (TICKS_Now() < timeout &&
                        sendCardCommand (P, SD_CMD58, 0) == 0)
                {	
                    // Check CCS bit in the OCR
                    recvBuffer (P, ocr, 4);
                    // Check if the card is SDv2
					ty = (ocr[0] & 0x40)? RAWSTOR_SD_TYPE_SD_V2 | 
                                            RAWSTOR_SD_TYPE_BLOCK_ADDRESSING :
                                                RAWSTOR_SD_TYPE_SD_V2;
				}
			}
        }
        // Not an SDv2 card / SDSC or MMC
        else
        {
            uint8_t cmd;
            // SDv1
            if (sendCardCommand (P, SD_ACMD41, 0) <= 1)
            {
                ty  = RAWSTOR_SD_TYPE_SD_V1;
                cmd = SD_ACMD41;
            }
            // MMCv3
            else
            {
                ty  = RAWSTOR_SD_TYPE_MMC_V3;
                cmd = SD_CMD1;
            }
            
            // Wait for leaving idle state
			while (TICKS_Now() < timeout &&
                   sendCardCommand (P, cmd, 0));
            
            // Set R/W block length to 512
			if (TICKS_Now() >= timeout ||
                    sendCardCommand (P, SD_CMD16, 512) != 0)
            {
				ty = 0;
            }
		}
	}
    
	P->tFlags = ty;

	deselectCard (P);

    // Initialization succeded
    if (ty)
    {
        setFastClock (P);

		R->status.disk &= ~RAWSTOR_Status_Disk_NotInitialized;
        RAWSTOR_UpdateStatusMedia (R, RAWSTOR_Status_Media_Ready, 0, 0);
        RAWSTOR_UpdateStatusResult (R, RAWSTOR_Status_Result_Ok);
	}
    // Initialization failed
    else 
    {
		powerOff (P);

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
    struct RAWSTOR_SD_1BIT *const P = (struct RAWSTOR_SD_1BIT *) R;

    // LBA to BA conversion (byte addressing cards)
    const uint32_t RealSector = 
                            (!(P->tFlags & RAWSTOR_SD_TYPE_BLOCK_ADDRESSING))?
                                SectorBegin << 9 : SectorBegin;

    // READ_MULTIPLE_BLOCK : READ_SINGLE_BLOCK
    const uint8_t Cmd = (SectorCount > 1)? SD_CMD18 : SD_CMD17;
    
    uint32_t sectorsRead = 0;
    
    // Send first sector
    if (sendCardCommand (P, Cmd, RealSector) == 0)
    {
        // Read `Count` sectors 
        do
        {
			if (!recvCardDatablock (P, &Data[sectorsRead << 9], 512))
            {
                break;
            }
		}
        while (++ sectorsRead < SectorCount);
        
		if (Cmd == SD_CMD18)
        {
            // STOP_TRANSMISSION
            sendCardCommand (P, SD_CMD12, 0);
        }
	}
    
	deselectCard (P);

	return (sectorsRead < SectorCount)?
                        RAWSTOR_Status_Result_ReadWriteError :
                        RAWSTOR_Status_Result_Ok;
}


static RAWSTOR_Status_Result mediaWrite (struct RAWSTOR *R, 
                                         const uint8_t *const Data,
                                         const uint32_t SectorBegin, 
                                         const uint32_t SectorCount)
{
    struct RAWSTOR_SD_1BIT *const P = (struct RAWSTOR_SD_1BIT *) R;

    // LBA to BA conversion (byte addressing cards)
    const uint32_t RealSector = 
                            (!(P->tFlags & RAWSTOR_SD_TYPE_BLOCK_ADDRESSING))?
                                SectorBegin << 9 : SectorBegin;

    uint32_t sectorsWritten = 0;

    // Single sector write
	if (SectorCount == 1) 
    {
        // WRITE_BLOCK
		if ((sendCardCommand (P, SD_CMD24, RealSector) == 0) &&
                sendCardDatablock (P, Data, 0xFE))
        {
            sectorsWritten = 1;
        }
	}
    // Multiple sector write
	else 
    {
		if (P->tFlags & RAWSTOR_SD_TYPE_SD_ANY)
        {
            // Predefine number of sectors
            sendCardCommand (P, SD_ACMD23, SectorCount);
        }
        
        // WRITE_MULTIPLE_BLOCK
		if (sendCardCommand (P, SD_CMD25, RealSector) == 0)
        {
			do 
            {
				if (!sendCardDatablock (P, &Data[sectorsWritten << 9], 0xFC))
                {
                    break;
                }
			}
            while (++ sectorsWritten < SectorCount);

            // STOP_TRAN token
			if (!sendCardDatablock (P, 0, 0xFD))
            {
                // TODO: do proper error handling
				sectorsWritten = 1;
            }
		}
	}
	deselectCard (P);

	return (sectorsWritten < SectorCount)? 
                        RAWSTOR_Status_Result_ReadWriteError :
                        RAWSTOR_Status_Result_Ok;
}


static RAWSTOR_Status_Result mediaIoctl (struct RAWSTOR *const R,
                                         const uint8_t Cmd,
                                         void *const Data)
{   
    struct RAWSTOR_SD_1BIT *p = (struct RAWSTOR_SD_1BIT *) R;

	RAWSTOR_Status_Result   res;
	uint8_t                 csd[16], n, *ptr = Data;
	uint16_t                *dp, st, ed, csize;

	res = RAWSTOR_Status_Result_ReadWriteError;
    
	if (Cmd == RAWSTOR_IOCTL_CMD_POWER)
    {
		switch (*ptr) 
        {
            case RAWSTOR_IOCTL_CMD_POWER_OFF:
                if (powerStatus (p))
                {
                    powerOff (p);
                }
                res = RAWSTOR_Status_Result_Ok;
                break;
                
            case RAWSTOR_IOCTL_CMD_POWER_ON:
                powerOn (p);
                res = RAWSTOR_Status_Result_Ok;
                break;
                
            case RAWSTOR_IOCTL_CMD_POWER_STATUS:
                *(ptr+1) = powerStatus(p)? 1 : 0;
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
            if (selectCard (p))
            {
                res = RAWSTOR_Status_Result_Ok;
                // Deselecting at the end
                // deselectCard ();
            }
            break;

        case RAWSTOR_IOCTL_CMD_GET_SECTOR_COUNT:
            // Get number of sectors on the disk (uint32_t)
            if ((sendCardCommand (p, SD_CMD9, 0) == 0) &&
                    recvCardDatablock (p, csd, 16))
            {
                // SDC ver 2.00
                if ((csd[0] >> 6) == 1)
                {                           
                    csize = csd[9] + ((uint16_t)csd[8] << 8) + 
                                            ((uint32_t)(csd[7] & 63) << 16) + 1;
                    *(uint32_t *) Data = (uint32_t)csize << 10;
                } 
                // SDC ver 1.XX or MMC
                else
                {
                    n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + 
                                            ((csd[9] & 3) << 1) + 2;
                    csize = (csd[8] >> 6) + ((uint16_t)csd[7] << 2) + 
                                            ((uint16_t)(csd[6] & 3) << 10) + 1;
                    *(uint32_t *) Data = (uint32_t)csize << (n - 9);
                }
                res = RAWSTOR_Status_Result_Ok;
            }
            break;

        case RAWSTOR_IOCTL_CMD_GET_SECTOR_SIZE:
            // Get R/W sector size (uint16_t)
            *(uint16_t *) Data = RAWSTOR_SECTOR_SIZE;
            res = RAWSTOR_Status_Result_Ok;
            break;

        // Get erase block size in unit of sector (uint32_t)           
        case RAWSTOR_IOCTL_CMD_GET_ERASE_BLOCK_SIZE:
            // SDC ver 2.0
            if (p->tFlags & RAWSTOR_SD_TYPE_SD_V2)
            {
                // Read SD status
                if (sendCardCommand (p, SD_ACMD13, 0) == 0)
                {                 
                    recvOctet (p);
                    // Read partial block
                    if (recvCardDatablock (p, csd, 16))
                    {				
                        for (n = 64 - 16; n; n--) 
                        {
                            // Purge trailing data
                            recvOctet (p);
                        }
                        *(uint32_t *) Data = 16UL << (csd[10] >> 4);
                        res = RAWSTOR_Status_Result_Ok;
                    }
                }
            } 
            // SDC ver 1.XX or MMC
            else 
            {
                // Read CSD
                if ((sendCardCommand (p, SD_CMD9, 0) == 0) &&
                        recvCardDatablock (p, csd, 16))
                {
                    if (p->tFlags & RAWSTOR_SD_TYPE_SD_V1)
                    {
                        *(uint32_t *) Data = (((csd[10] & 63) << 1) + 
                                ((uint16_t)(csd[11] & 128) >> 7) + 1) << 
                                    ((csd[13] >> 6) - 1);
                    } 
                    // MMC
                    else
                    {
                        *(uint32_t *) Data = 
                                ((uint16_t)((csd[10] & 124) >> 2) + 1) * 
                                    (((csd[11] & 3) << 3) + 
                                        ((csd[11] & 224) >> 5) + 1);
                    }
                    res = RAWSTOR_Status_Result_Ok;
                }
            }
            break;
            
        // Erase a block of sectors (used when FF_USE_TRIM in ffconf.h is 1)
        case RAWSTOR_IOCTL_CMD_TRIM:
            if (!(p->tFlags & RAWSTOR_SD_TYPE_SD_ANY))
            {
                break;
            }
            
            if (mediaIoctl (R, RAWSTOR_SD_IOCTL_GET_CSD, csd))
            {
                break;
            }
            
            // Check if sector erase can be applied to the card
            if (!(csd[0] >> 6) && !(csd[10] & 0x40))
            {
                break;
            }
            
            // Load sector block
            dp = Data;
            st = dp[0];
            ed = dp[1];
            
            if (!(p->tFlags & RAWSTOR_SD_TYPE_BLOCK_ADDRESSING))
            {
                st *= 512;
                ed *= 512;
            }
            
            // Erase sector block
            if (sendCardCommand (p, SD_CMD32, st) == 0 &&
                sendCardCommand (p, SD_CMD33, ed) == 0 &&
                sendCardCommand (p, SD_CMD38,  0) == 0 &&
                waitForCardReady (p, 30000))
            {
                // FatFs does not check result of this command
                res = RAWSTOR_Status_Result_Ok;
            }
            break;
        // ---------------------------------------------------------------------
        // The following commands are never used by FatFs module
        // ---------------------------------------------------------------------
        case RAWSTOR_SD_IOCTL_GET_TYPE:
            // Get card type flags (1 byte)
            *ptr = p->tFlags;
            res = RAWSTOR_Status_Result_Ok;
            break;

        case RAWSTOR_SD_IOCTL_GET_CSD:
            // Receive CSD as a data block (16 bytes)
            if (sendCardCommand (p, SD_CMD9, 0) == 0 &&
                    recvCardDatablock (p, ptr, 16))
            {
                res = RAWSTOR_Status_Result_Ok;
            }
            break;

        case RAWSTOR_SD_IOCTL_GET_CID:
            // Receive CID as a data block (16 bytes)
            if (sendCardCommand (p, SD_CMD10, 0) == 0 &&
                    recvCardDatablock (p, ptr, 16))
            {
                res = RAWSTOR_Status_Result_Ok;
            }
            break;

        case RAWSTOR_SD_IOCTL_GET_OCR:
            // Receive OCR as an R3 resp (4 bytes)
            if (sendCardCommand (p, SD_CMD58, 0) == 0)
            {
                for (n = 4; n; n--) 
                {
                    *ptr++ = recvOctet (p);
                }
                res = RAWSTOR_Status_Result_Ok;
            }
            break;

        case RAWSTOR_SD_IOCTL_GET_SDSTAT:
            // Receive SD status as a data block (64 bytes)
            if (sendCardCommand(p, SD_ACMD13, 0) == 0)
            {
                recvOctet (p);
                if (recvCardDatablock (p, ptr, 64))
                {
                    res = RAWSTOR_Status_Result_Ok;
                }
            }
            break;

        default:
            BOARD_AssertParams (false);
            break;
    }

    deselectCard (p);

	return res;
}
