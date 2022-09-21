/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [STORAGE subsystem] storage cached elements check and update.

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

#include "embedul.ar/source/core/manager/storage/cache.h"
#include "embedul.ar/source/core/device/board.h"
#include <stdio.h>

#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
#include "ff.h"
#endif


#define CACHE_SIGNATURE                 "EMBEDUL.AR CACHE"
#define CACHE_FWK_ELEMENTS              8
// CRC-32C (Castagnoli) polynomial, in reversed bit order.
#define CACHE_CRC32_POLY                0x82f63b78


// STORAGE private API
void STORAGE___setCachedElementsCount (const uint32_t Count);


static uint32_t crc32c (uint32_t crc, const uint8_t *buf, size_t len)
{
    crc = ~crc;

    while (len --)
    {
        crc ^= *buf++;

        crc = (crc >> 1) ^ (CACHE_CRC32_POLY & (0 - (crc & 1)));
        crc = (crc >> 1) ^ (CACHE_CRC32_POLY & (0 - (crc & 1)));
        crc = (crc >> 1) ^ (CACHE_CRC32_POLY & (0 - (crc & 1)));
        crc = (crc >> 1) ^ (CACHE_CRC32_POLY & (0 - (crc & 1)));
        crc = (crc >> 1) ^ (CACHE_CRC32_POLY & (0 - (crc & 1)));
        crc = (crc >> 1) ^ (CACHE_CRC32_POLY & (0 - (crc & 1)));
        crc = (crc >> 1) ^ (CACHE_CRC32_POLY & (0 - (crc & 1)));
        crc = (crc >> 1) ^ (CACHE_CRC32_POLY & (0 - (crc & 1)));
    }

    return ~crc;
}


static uint32_t getElementInfoSector (const uint32_t Element)
{
    // Element info sectors advance backwards from volume end. Also note that
    // LinearWrite/Read takes partition-relative sectors starting at 0 and 
    // ending at partition capacity.
    const uint32_t ElementInfoSector =
        STORAGE_VolumeInfo(STORAGE_Role_LinearCache).sectorEnd -
        STORAGE_VolumeInfo(STORAGE_Role_LinearCache).sectorBegin - Element;

    return ElementInfoSector;
}


static RAWSTOR_Status_Result
readElementInfoSector (const uint32_t Element,
                       uint8_t sectorData[static 512],
                       const uint32_t Retries)
{
    const RAWSTOR_Status_Result Rsr = 
        STORAGE_LinearRead (STORAGE_Role_LinearCache, sectorData, 
                            getElementInfoSector(Element), 1, Retries);
    return Rsr;
}


static void check (struct STORAGE_CACHE_State *const Cs,
                         const enum STORAGE_CACHE_CheckFlags Check,
                         const bool Passed)
{
    if (Passed)
    {
        Cs->checksPassed |= Check;
    }
    else
    {
        Cs->checksFailed |= Check;
    }
}


static void clearChecks (struct STORAGE_CACHE_State *const Cs)
{
    Cs->checksPassed = 0;
    Cs->checksFailed = 0;
}


static void nextTask (struct STORAGE_CACHE_State *const Cs,
                      const enum STORAGE_CACHE_Stage NextTask)
{
    Cs->lastTask = Cs->nextTask;
    Cs->nextTask = NextTask;    
}


static bool checkSectorCrc32 (uint8_t sectorData[static 512])
{
    // Stored CRC32
    const uint32_t StoredCrc32 = *((uint32_t *)&sectorData[512-4]);

    // Original CRC32 of the whole data was computed with 0 in the CRC32 field.
    *((uint32_t *)&sectorData[512-4]) = 0;

    const uint32_t ComputedCrc32 = crc32c (0, sectorData, 512);

    *((uint32_t *)&sectorData[512-4]) = StoredCrc32;

    return (ComputedCrc32 == StoredCrc32)? true : false;
}


static void start (struct STORAGE_CACHE_State *const Cs)
{
    STORAGE___setCachedElementsCount (0);

#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
    if (STORAGE_ValidVolume (STORAGE_Role_FatFsDrive0))
    {
        Cs->hasSlots = true;
    }
#endif

    if (!STORAGE_ValidVolume (STORAGE_Role_LinearCache))
    {
        // No linear cache volume.
        check       (Cs, STORAGE_CACHE_CheckFlags_Volume, false);
        nextTask    (Cs, STORAGE_CACHE_Stage_Done);
    }
    else
    {
        // Linear cache available; read Sector 0.
        check       (Cs, STORAGE_CACHE_CheckFlags_Volume, true);
        nextTask    (Cs, STORAGE_CACHE_Stage_ReadSector0);
    }
}


static void readSector0 (struct STORAGE_CACHE_State *const Cs)
{
    uint8_t sectorData[512];

    // Sector 0 read
    const RAWSTOR_Status_Result Rsr = STORAGE_LinearRead (
                STORAGE_Role_LinearCache, sectorData, 0, 1, Cs->rwRetries);

    check (Cs, STORAGE_CACHE_CheckFlags_Read,
                                (Rsr == RAWSTOR_Status_Result_Ok));

    if (Rsr != RAWSTOR_Status_Result_Ok)
    {
        nextTask (Cs, STORAGE_CACHE_Stage_Done);
        return;
    }

    // Embedded sector checksum
    check (Cs, STORAGE_CACHE_CheckFlags_Checksum,
                checkSectorCrc32(sectorData));

    // NOTE: Do not trust on sector 0 data if checksum failed
    check (Cs, STORAGE_CACHE_CheckFlags_Signature,
                !memcmp((char *)&sectorData[0], CACHE_SIGNATURE, 16));

    check (Cs, STORAGE_CACHE_CheckFlags_FwkVersion,
                !strncmp((char *)&sectorData[16], CC_VcsFwkVersionStr, 64));

    check (Cs, STORAGE_CACHE_CheckFlags_AppName,
                !strncmp((char *)&sectorData[80], CC_AppNameStr, 64));

    check (Cs, STORAGE_CACHE_CheckFlags_AppVersion,
                !strncmp((char *)&sectorData[144], CC_VcsAppVersionStr, 64));

    // Sector 0 data can be trusted
    if (!(Cs->checksFailed & STORAGE_CACHE_CheckFlags_Checksum))
    {
        // Retrieve element count
        Cs->elementCount = *((uint32_t *)&sectorData[512-8]);

        // Report available element count to storage.
        STORAGE___setCachedElementsCount (Cs->elementCount);
    }

    // No filesystem slots nor cached elements, nothing to iterate
    if (!Cs->hasSlots && !Cs->elementCount)
    {
        nextTask (Cs, STORAGE_CACHE_Stage_Done);
        return;
    }

#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
    // Can check for filesystem slots. Update sector 0 if any check failed
    if (Cs->hasSlots && Cs->checksFailed)
    {
        Cs->writeSector0 = true;
    }
#endif

    nextTask (Cs, STORAGE_CACHE_Stage_BeginIteration);
}


static void beginIteration (struct STORAGE_CACHE_State *const Cs)
{
    clearChecks (Cs);

#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
    if (Cs->hasSlots)
    {
        // Read current filesystem slot to compare with the corresponding
        // cached element, if any
        nextTask (Cs, STORAGE_CACHE_Stage_ReadSlot);
        return;
    }
#endif

    nextTask (Cs, STORAGE_CACHE_Stage_CheckElementInfo);
}


#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
static bool getFilesystemSlotInfo (struct STORAGE_CACHE_State *const Cs)
{
    uint32_t octets = 0;

    // First CACHE_FWK_ELEMENTS reserved for internal framework use. Each
    // application defines its own higher elements.
    if (Cs->element < CACHE_FWK_ELEMENTS)
    {
        octets += strlen (LIB_EMBEDULAR_STORAGE_CACHE_FS_FRAMEWORK_DIR);
        BOARD_AssertState (sizeof(Cs->filepath) > octets);

        // Base driver+directory
        strncpy (Cs->filepath,
                 LIB_EMBEDULAR_STORAGE_CACHE_FS_FRAMEWORK_DIR,
                 sizeof(Cs->filepath));
    }
    else 
    {
        octets += strlen (LIB_EMBEDULAR_STORAGE_CACHE_FS_APPLICATION_DIR);
        BOARD_AssertState (sizeof(Cs->filepath) > octets);

        // Base driver+directory
        strncpy (Cs->filepath,
                 LIB_EMBEDULAR_STORAGE_CACHE_FS_APPLICATION_DIR,
                 sizeof(Cs->filepath));
    }

    const char *const ElementNr = 
                        VARIANT_ToString(&VARIANT_SpawnUint(Cs->element));

    // Account for a trailing '/' after ElementNr
    octets += strlen (ElementNr) + 1;
    BOARD_AssertState (sizeof(Cs->filepath) > octets);

    // Slot number and trailing '/'
    strcat (Cs->filepath, ElementNr);
    strcat (Cs->filepath, "/");

    FRESULT res;
    DIR     dir;

    if ((res = f_opendir(&dir, Cs->filepath)) != FR_OK)
    {
        // Assume the current element folder does not exist; 
        // there are no more elements to process.
        check (Cs, STORAGE_CACHE_CheckFlags_FileMetrics, false);
        return false;
    }

    // Take the first file inside "dir"
    while ((res = f_readdir(&dir, &Cs->fno)) == FR_OK && 
           Cs->fno.fname[0] &&
           Cs->fno.fattrib & AM_DIR &&
           !(Cs->fno.fattrib & AM_ARC))
    {
    }

    f_closedir (&dir);

    const bool FileFound = (res == FR_OK && Cs->fno.fname[0]);

    check (Cs, STORAGE_CACHE_CheckFlags_Filesystem, !FileFound);

    // Check source file. Error reading directory contents or no file found.
    if (!FileFound)
    {
        // Each slot should have a file
        return false;
    }

    octets += strlen (Cs->fno.fname);
    BOARD_AssertState (sizeof(Cs->filepath) > octets);

    // Add filename to filepath
    strncat (Cs->filepath, Cs->fno.fname, 13);

    return true;
}
#endif // #ifdef LIB_EMBEDULAR_HAS_FILESYSTEM


#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
static void readSlot (struct STORAGE_CACHE_State *const Cs)
{
    clearChecks (Cs);

    const bool SlotOk = getFilesystemSlotInfo (Cs);

    // There is no slot for current Cs->element
    if (!SlotOk)
    {
        // Last slot already processed

        nextTask (Cs, STORAGE_CACHE_Stage_EndIteration);
        return;
    }

    // Actual slot is already in the cache?
    // T: Check if the cached element is exactly the same file
    // F: Write filesystem slot to cached element
    nextTask (Cs, (Cs->element < Cs->elementCount)?
                        STORAGE_CACHE_Stage_CheckElementInfo :
                        STORAGE_CACHE_Stage_SlotToElementInfo);
}
#endif // #ifdef LIB_EMBEDULAR_HAS_FILESYSTEM


static void initProgressCounter (struct STORAGE_CACHE_State *const Cs)
{
    const uint32_t SectorCount = Cs->sectorEnd - Cs->sectorBegin + 1;

    Cs->progressDelta   = (Cs->progressMax << 24) / SectorCount;
    Cs->progressCurrent = 0;
}


#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
static RAWSTOR_Status_Result
writeElementInfoSector (const uint32_t Element,
                        uint8_t sectorData[static 512],
                        const uint32_t Retries)
{
    const RAWSTOR_Status_Result Rsr =
            STORAGE_LinearWrite (STORAGE_Role_LinearCache, sectorData, 
                                 getElementInfoSector(Element), 1, Retries);
    return Rsr;
}


static void slotToElementInfo (struct STORAGE_CACHE_State *const Cs)
{
    clearChecks (Cs);

    uint8_t sectorData[512];


    memset (sectorData, 0, sizeof(sectorData));

    Cs->octets          = Cs->fno.fsize;
    // Element data is stored after the last one
    Cs->sectorBegin     = Cs->sectorEnd + 1;
    Cs->sectorEnd       += (Cs->octets & 511)? 
                                (Cs->octets >> 9) + 1 :
                                (Cs->octets >> 9);
    Cs->sectorCurrent   = Cs->sectorBegin;
    Cs->checkCrc32      = 0;

    *((uint32_t *)&sectorData[0])   = Cs->fno.fdate;
    *((uint32_t *)&sectorData[4])   = Cs->fno.ftime;
    *((uint32_t *)&sectorData[8])   = Cs->octets;
    *((uint32_t *)&sectorData[12])  = Cs->sectorBegin;
    *((uint32_t *)&sectorData[16])  = Cs->sectorEnd;
    *((uint32_t *)&sectorData[20])  = 0; // storedCrc32, calculated in
                                         // slotToElementData.
    memcpy (&sectorData[24], Cs->filepath, sizeof(Cs->filepath));

    initProgressCounter (Cs);

    // Open slot file
    const FRESULT R = f_open (&Cs->file, Cs->filepath, FA_READ);

    check (Cs, STORAGE_CACHE_CheckFlags_Filesystem, (R == FR_OK));

    if (R != FR_OK)
    {
        nextTask (Cs, STORAGE_CACHE_Stage_EndIteration);
        return;
    }

    // Write element info sector
    const RAWSTOR_Status_Result Rsr = 
            writeElementInfoSector (Cs->element, sectorData, Cs->rwRetries);

    check (Cs, STORAGE_CACHE_CheckFlags_Write,
                                (Rsr == RAWSTOR_Status_Result_Ok));

    if (Rsr != RAWSTOR_Status_Result_Ok)
    {
        f_close (&Cs->file);
        nextTask (Cs, STORAGE_CACHE_Stage_EndIteration);
        return;
    }

    nextTask (Cs, STORAGE_CACHE_Stage_SlotToElementData);
}
#endif // #ifdef LIB_EMBEDULAR_HAS_FILESYSTEM


#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
static void slotToElementData (struct STORAGE_CACHE_State *const Cs)
{
    // Clear checks on first iteration only
    if (!Cs->sectorCurrent)
    {
        clearChecks (Cs);
    }

    uint8_t sectorData[512];

    while (Cs->sectorCurrent <= Cs->sectorEnd)
    {
        UINT br;

        FRESULT R = f_read (&Cs->file, sectorData, 512, &br);

        check (Cs, STORAGE_CACHE_CheckFlags_Filesystem, (R == FR_OK));

        if (R != FR_OK)
        {
            f_close (&Cs->file);
            nextTask (Cs, STORAGE_CACHE_Stage_EndIteration);
            return;
        }

        if (br < 512)
        {
            BOARD_AssertState (Cs->sectorCurrent == Cs->sectorEnd);
            memset (&sectorData[br], 0, 512 - br);
        }

        const RAWSTOR_Status_Result Rsr = 
                STORAGE_LinearWrite (STORAGE_Role_LinearCache, sectorData, 
                                     Cs->sectorCurrent, 1, Cs->rwRetries);

        check (Cs, STORAGE_CACHE_CheckFlags_Write,
                                (Rsr == RAWSTOR_Status_Result_Ok));

        if (Rsr != RAWSTOR_Status_Result_Ok)
        {
            f_close (&Cs->file);
            nextTask (Cs, STORAGE_CACHE_Stage_EndIteration);
            return;
        }

        Cs->checkCrc32 = crc32c (Cs->checkCrc32, sectorData, 512);

        ++ Cs->sectorCurrent;

        Cs->progressCurrent += Cs->progressDelta;

        if (Cs->breakFunc && Cs->breakFunc(Cs->breakFuncParam))
        {
            break;
        }
    }

    // Finished processing current element data
    if (Cs->sectorCurrent > Cs->sectorEnd)
    {
        f_close (&Cs->file);

        Cs->progressCurrent = Cs->progressMax << 24;

        RAWSTOR_Status_Result rsr;

        // Read ElementInfo without CRCs
        rsr = readElementInfoSector (Cs->element, sectorData, Cs->rwRetries);

        check (Cs, STORAGE_CACHE_CheckFlags_Read,
                                (rsr == RAWSTOR_Status_Result_Ok));

        if (rsr != RAWSTOR_Status_Result_Ok)
        {
            nextTask (Cs, STORAGE_CACHE_Stage_EndIteration);
            return;
        }

        // Update data and sector CRC32
        *((uint32_t *)&sectorData[20])      = Cs->checkCrc32;
        *((uint32_t *)&sectorData[512-4])   = crc32c (0, sectorData, 512);

        // Update ElementInfo with valid CRCs
        rsr = writeElementInfoSector(Cs->element, sectorData, Cs->rwRetries);

        check (Cs, STORAGE_CACHE_CheckFlags_Write,
                                (rsr == RAWSTOR_Status_Result_Ok));

        if (rsr != RAWSTOR_Status_Result_Ok)
        {
            nextTask (Cs, STORAGE_CACHE_Stage_EndIteration);
            return;
        }

        // Next slot
        nextTask (Cs, STORAGE_CACHE_Stage_NextIteration);
    }
    else
    {
        // Continue processing element data.
        nextTask (Cs, STORAGE_CACHE_Stage_SlotToElementData);
    }
}
#endif // #ifdef LIB_EMBEDULAR_HAS_FILESYSTEM


static void checkElementInfo (struct STORAGE_CACHE_State *const Cs)
{
    clearChecks (Cs);

    uint8_t sectorData[512];

    // Read element info sector
    const RAWSTOR_Status_Result Rsr = 
            readElementInfoSector (Cs->element, sectorData, Cs->rwRetries);

    check (Cs, STORAGE_CACHE_CheckFlags_Read, 
                                (Rsr == RAWSTOR_Status_Result_Ok));

    if (Rsr != RAWSTOR_Status_Result_Ok)
    {
        nextTask (Cs, STORAGE_CACHE_Stage_EndIteration);
        return;
    }

    // Check sector CRC32
    check (Cs, STORAGE_CACHE_CheckFlags_Checksum,
                                checkSectorCrc32(sectorData));

    if (Cs->checksFailed & STORAGE_CACHE_CheckFlags_Checksum)
    {
    #ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
        nextTask (Cs, STORAGE_CACHE_Stage_SlotToElementInfo);
    #else
        nextTask (Cs, STORAGE_CACHE_Stage_EndIteration);
    #endif
        return;
    }

#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
    // Preserve original sectorEnd.
    const uint32_t LastSectorEnd = Cs->sectorEnd;
#endif

    // Sector is ok. Retrieve element info.
    Cs->storedFileDate  = *((uint32_t *)&sectorData[0]);
    Cs->storedFileTime  = *((uint32_t *)&sectorData[4]);
    Cs->octets          = *((uint32_t *)&sectorData[8]);
    Cs->sectorBegin     = *((uint32_t *)&sectorData[12]);
    Cs->sectorEnd       = *((uint32_t *)&sectorData[16]);
    Cs->sectorCurrent   = Cs->sectorBegin;
    Cs->storedCrc32     = *((uint32_t *)&sectorData[20]);
    Cs->checkCrc32      = 0;

    initProgressCounter (Cs);

#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
    if (Cs->hasSlots)
    {
        const char *const StoredFilepath
                            = (const char *)&sectorData[24];

        const bool FileMetricsMatch =
                            Cs->fno.fdate == Cs->storedFileDate &&
                            Cs->fno.ftime == Cs->storedFileTime &&
                            Cs->fno.fsize == Cs->octets &&
                            Cs->fno.fattrib & AM_ARC &&
                            !strncmp(Cs->filepath, StoredFilepath,
                                                sizeof(Cs->filepath));

        check (Cs, STORAGE_CACHE_CheckFlags_FileMetrics, FileMetricsMatch);

        // Check element info parameters against actual file
        if (!FileMetricsMatch)
        {
            // The current cached element requires a re-write.
            // Revert sectorEnd to the last value so that SlotToElementInfo can 
            // calculate the proper sectorBegin for the updated element.
            Cs->sectorEnd = LastSectorEnd;

            // Rebuild Sector0 after processing all slots
            Cs->writeSector0 = true;

            nextTask (Cs, STORAGE_CACHE_Stage_SlotToElementInfo);
            return;
        }
    }
#endif // LIB_EMBEDULAR_HAS_FILESYSTEM

    nextTask (Cs, (Cs->skipElementDataCheck)?
                        STORAGE_CACHE_Stage_NextIteration :
                        STORAGE_CACHE_Stage_CheckElementData);
}


static void checkElementData (struct STORAGE_CACHE_State *const Cs)
{
    // Clear checks on first iteration only
    if (!Cs->sectorCurrent)
    {
        clearChecks (Cs);
    }

    uint8_t sectorData[512];

    while (Cs->sectorCurrent <= Cs->sectorEnd)
    {
        // read nth element data sector
        RAWSTOR_Status_Result r = STORAGE_LinearRead (
                                        STORAGE_Role_LinearCache, sectorData, 
                                        Cs->sectorCurrent, 1, Cs->rwRetries);

        if (r != RAWSTOR_Status_Result_Ok)
        {
            check (Cs, STORAGE_CACHE_CheckFlags_Read, false);
            nextTask (Cs, STORAGE_CACHE_Stage_EndIteration);
            return;
        }

        Cs->checkCrc32 = crc32c (Cs->checkCrc32, sectorData, 512);

        ++ Cs->sectorCurrent;

        Cs->progressCurrent += Cs->progressDelta;

        if (Cs->breakFunc && Cs->breakFunc(Cs->breakFuncParam))
        {
            break;
        }
    }

    // Finished processing current element data
    if (Cs->sectorCurrent > Cs->sectorEnd)
    {
        // All data reads must succeed in order to enable this flag.
        check (Cs, STORAGE_CACHE_CheckFlags_Read, true);

        Cs->progressCurrent = Cs->progressMax << 24;

        // Check element data CRC32
        check (Cs, STORAGE_CACHE_CheckFlags_Checksum,
                            (Cs->checkCrc32 == Cs->storedCrc32));

        if (Cs->checksFailed & STORAGE_CACHE_CheckFlags_Checksum)
        {
            nextTask (Cs, STORAGE_CACHE_Stage_EndIteration);
            return;
        }

        nextTask (Cs, STORAGE_CACHE_Stage_NextIteration);
    }
    else 
    {
        // Keep processing current data
        nextTask (Cs, STORAGE_CACHE_Stage_CheckElementData);
    }
}


static void nextIteration (struct STORAGE_CACHE_State *const Cs)
{
    clearChecks (Cs);

    ++ Cs->element;

    if (Cs->hasSlots || Cs->element + 1 < Cs->elementCount)
    {
        // Process next filesystem slot or element info
        nextTask (Cs, STORAGE_CACHE_Stage_BeginIteration);
        return;
    }

    // Last cached element already processed
    nextTask (Cs, STORAGE_CACHE_Stage_EndIteration);
}


static void endIteration (struct STORAGE_CACHE_State *const Cs)
{
    // endIteration will carry on the checks performed in the last task.

#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
    const enum STORAGE_CACHE_Stage NextStage = (Cs->writeSector0)?
                                        STORAGE_CACHE_Stage_WriteSector0 :
                                        STORAGE_CACHE_Stage_Done;
#else 
    const enum STORAGE_CACHE_Stage NextStage = STORAGE_CACHE_Stage_Done;
#endif

    nextTask (Cs, NextStage);
}


#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
static void writeSector0 (struct STORAGE_CACHE_State *const Cs)
{
    clearChecks (Cs);

    uint8_t sectorData[512];

    memset (sectorData, 0x00, sizeof(sectorData));

    memcpy  ((char *)&sectorData[0], CACHE_SIGNATURE, 16);
    strncpy ((char *)&sectorData[16], CC_VcsFwkVersionStr, 64);
    strncpy ((char *)&sectorData[80], CC_AppNameStr, 64);
    strncpy ((char *)&sectorData[144], CC_VcsAppVersionStr, 64);

    *((uint32_t *)&sectorData[512-8]) = Cs->element;
    *((uint32_t *)&sectorData[512-4]) = crc32c (0, sectorData, 512);

    const RAWSTOR_Status_Result Rsr = STORAGE_LinearWrite (
                STORAGE_Role_LinearCache, sectorData, 0, 1, Cs->rwRetries);

    check (Cs, STORAGE_CACHE_CheckFlags_Write,
                                (Rsr == RAWSTOR_Status_Result_Ok));

    STORAGE___setCachedElementsCount (Cs->element);

    nextTask (Cs, STORAGE_CACHE_Stage_Done);
}
#endif  // #ifdef LIB_EMBEDULAR_HAS_FILESYSTEM


void STORAGE_CACHE_Init (struct STORAGE_CACHE_State *const Cs,
                         const bool SkipElementDataCheck,
                         const uint32_t RwRetries,
                         const uint8_t ProgressMax,
                         const STORAGE_CACHE_ProcessBreakFunc BreakFunc,
                         void *const BreakFuncParam)
{
    BOARD_AssertParams (ProgressMax);

    memset (Cs, 0, sizeof(struct STORAGE_CACHE_State));

    Cs->skipElementDataCheck    = SkipElementDataCheck;
    Cs->rwRetries               = RwRetries;
    Cs->progressMax             = ProgressMax;
    Cs->breakFunc               = BreakFunc;
    Cs->breakFuncParam          = BreakFuncParam;
    Cs->nextTask                = STORAGE_CACHE_Stage_Start;
}


bool STORAGE_CACHE_ProcessNextTask (struct STORAGE_CACHE_State *const Cs)
{
    BOARD_AssertParams (Cs);

    switch (Cs->nextTask)
    {
        case STORAGE_CACHE_Stage_Start:
            start (Cs);
            break;

        case STORAGE_CACHE_Stage_ReadSector0:
            readSector0 (Cs);
            break;

        case STORAGE_CACHE_Stage_BeginIteration:
            beginIteration (Cs);
            break;

    #ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
        case STORAGE_CACHE_Stage_ReadSlot:
            readSlot (Cs);
            break;

        case STORAGE_CACHE_Stage_SlotToElementInfo:
            slotToElementInfo (Cs);
            break;

        case STORAGE_CACHE_Stage_SlotToElementData:
            slotToElementData (Cs);
            break;
    #endif // LIB_EMBEDULAR_HAS_FILESYSTEM

        case STORAGE_CACHE_Stage_CheckElementInfo:
            checkElementInfo (Cs);
            break;

        case STORAGE_CACHE_Stage_CheckElementData:
            checkElementData (Cs);
            break;

        case STORAGE_CACHE_Stage_NextIteration:
            nextIteration (Cs);
            break;

        case STORAGE_CACHE_Stage_EndIteration:
            endIteration (Cs);
            break;

    #ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
        case STORAGE_CACHE_Stage_WriteSector0:
            writeSector0 (Cs);
            break;
    #endif

        case STORAGE_CACHE_Stage_Done:
            // Stop the process
            return false;
    }

    // Continue processing
    return true;
}


uint32_t
STORAGE_CACHE_ElementProgress (const struct STORAGE_CACHE_State *const Cs)
{
    BOARD_AssertParams (Cs);

    return Cs->progressCurrent >> 24;
}


bool STORAGE_CACHE_ElementFinished (const struct STORAGE_CACHE_State *const Cs)
{
    BOARD_AssertParams (Cs);

    return (Cs->progressCurrent == Cs->progressMax << 24)? true : false;
}

/*
    Call to STORAGE_CachedElementsCount() to check if Element is available
    first!
*/
RAWSTOR_Status_Result STORAGE_CACHE_ElementInfo (
                            struct STORAGE_CACHE_ElementInfo *const Ei,
                            const uint32_t Element,
                            uint8_t SectorData[const static 512],
                            const uint32_t Retries)
{
    BOARD_AssertParams (Ei &&
                        Element < STORAGE_CachedElementsCount() && 
                        SectorData);

    const RAWSTOR_Status_Result Rsr =
                readElementInfoSector (Element, SectorData, Retries);

    if (Rsr == RAWSTOR_Status_Result_Ok)
    {
        Ei->octets      = *((uint32_t *)&SectorData[8]);
        Ei->sectorBegin = *((uint32_t *)&SectorData[12]);
        Ei->sectorEnd   = *((uint32_t *)&SectorData[16]);
        Ei->sectorCount = Ei->sectorEnd - Ei->sectorBegin + 1;
        memcpy (Ei->filepath, &SectorData[24], sizeof(Ei->filepath));
    }
    else
    {
        memset (Ei, 0, sizeof(*Ei));

        LOG_WarnDebug (NOBJ, LANG_CACHED_ELEMENT_READ_FAILED);
        LOG_Items (2, 
                        LANG_CACHED_ELEMENT,    Element,
                        LANG_ERROR,             Rsr);
    }

    return Rsr;
}


RAWSTOR_Status_Result STORAGE_CACHE_ElementData (
                        const struct STORAGE_CACHE_ElementInfo *const Ei,
                        const uint32_t SectorDelta, const uint32_t SectorCount,
                        uint8_t *const ElementData, const uint32_t Retries)
{
    BOARD_AssertParams (Ei && ElementData &&
                        SectorDelta + SectorCount <= Ei->sectorCount);

    // user-defined SectorCount must account for SectorDelta.
    const uint32_t Sectors = SectorCount? SectorCount :
                                          Ei->sectorCount - SectorDelta;

    const RAWSTOR_Status_Result Rsr = 
                STORAGE_LinearRead (STORAGE_Role_LinearCache, ElementData,
                                    Ei->sectorBegin + SectorDelta, Sectors,
                                    Retries);

    if (Rsr != RAWSTOR_Status_Result_Ok)
    {
        LOG (NOBJ, LANG_CACHED_ELEMENT_READ_FAILED);
        LOG_Items (3,
                        LANG_PATH,      Ei->filepath,
                        LANG_ERROR,     Rsr);
    }

    return Rsr;
}
