/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [VIDEO driver] LPC4337 dualcore software video adapter.

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

#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/video_dualcore.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/video_dualcore/exchange.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/chip_18xx_43xx/chip.h"
#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/core/manager/storage/cache.h"
#include <stdio.h>


#define INVALID_ADAPTER_RESET_HANDLER_STR   "invalid video adapter reset handler"
#define NO_VIDEO_DRIVER_FOUND_STR           "no video driver found"
#define FILE_BIGGER_THAN_SRAM_STR           "video driver file does not fit in SRAM"
#define ERROR_OPENING_VIDEO_DRIVER_STR      "error opening video driver file"
#define ERROR_READING_VIDEO_DATA_STR        "error reading video driver data"
#define CACHED_BIGGER_THAN_SRAM_STR         "cached element does not fit in SRAM"
#define CACHED_LOADED_SRAM_STR              "cached element loaded to SRAM"
#define FILE_LOADED_SRAM_STR                "video driver file loaded to SRAM"
#define INVALID_DRIVER_STACK_PTR_STR        "invalid video driver stack pointer"
#define VIDEO_ADAPTER_MALFUNCTION_STR       "video adapter malfunction"
#define VIDEO_ADAPTER_MISSING_INFO_STR      "no mandatory info from video adapter"
#define LOAD_FROM_FLASH_STR                 "loading from FLASH at `0"
#define EXEC_FROM_FLASH_STR                 "executing from FLASH at `0"
#define EXEC_FROM_SRAM_STR                  "executing from SRAM at `0"
#define SRAM_SIZE_ITEM_STR                  "sram size"
#define IODATA_ITEM_STR                     "vda iodata"

#define VDA_FIRMWARE_FILENAME               "VDA_4337.FW"
#define VDA_FILEPATH                        LIB_EMBEDULAR_STORAGE_CACHE_FS_ROOT_DIR VDA_FIRMWARE_FILENAME


static volatile uint32_t s_vbi = 0;

extern uint32_t __start_core_m0app_MFlash[];
extern uint32_t __top_core_m0app_MFlash[];
extern uint32_t __start_core_m0app_Ram[];
extern uint32_t __top_core_m0app_Ram[];


void M0APP_IRQHandler (void)
{
    // Clear Interrupt
    Chip_CREG_ClearM0AppEvent ();

    // Count vertical blanking interrupt events
    ++ s_vbi;
}


void    hardwareInit        (struct VIDEO *const V);
bool    reachedVBICount     (struct VIDEO *const V);
void    waitForVBI          (struct VIDEO *const V);
void    frameTransition     (struct VIDEO *const V);


static const struct VIDEO_IFACE VIDEO_IFACE_DUALCORE =
{
    .Description        = "lpc4337 cortex-m0",
    .Width              = VIDEO_FB_WIDTH,
    .Height             = VIDEO_FB_HEIGHT,
    .HardwareInit       = hardwareInit,
    .ReachedVBICount    = reachedVBICount,
    .WaitForVBI         = waitForVBI,
    .FrameTransition    = frameTransition
};


void VIDEO_DUALCORE_Init (struct VIDEO_DUALCORE *const V)
{
    VIDEO_Init ((struct VIDEO *)V, &VIDEO_IFACE_DUALCORE,
                g_framebufferA, g_framebufferB);
}


static void videoExchangeWrite (struct VIDEO *const V)
{
    g_videoExchange.framebuffer = (uintptr_t) V->frontbuffer;
    g_videoExchange.scanlines   = V->scanlines;
    g_videoExchange.showAnd     = V->showAnd;
    g_videoExchange.showOr      = V->showOr;
    g_videoExchange.scanAnd     = V->scanAnd;
    g_videoExchange.scanOr      = V->scanOr;
}


static void videoExchangeRead (struct VIDEO *const V)
{
    V->frameNumber = g_videoExchange.frameNumber;
}


static bool fwImageExecutesFrom (const uint32_t *const FwImage,
                                 const uint32_t *MemoryMapArea)
{
    // Check if fw reset handler address matches a given high memory map area
    return ((FwImage[1] & 0xFF000000) == (((uintptr_t)MemoryMapArea) 
                                                & 0xFF000000))? true : false;
}


static uint32_t * loadFromFlash (struct VIDEO *const V,
                                 const uint32_t FlashFwOctets,
                                 const uint32_t RamFwOctets)
{
    const void *const MFlash = (const void *)__start_core_m0app_MFlash;

    LOG (V, LOAD_FROM_FLASH_STR, MFlash);

    // Check reset handler address
    if (fwImageExecutesFrom (__start_core_m0app_MFlash,
                             __start_core_m0app_MFlash))
    {
        LOG (V, EXEC_FROM_FLASH_STR, MFlash);

        // Image executes from Flash memory
        return __start_core_m0app_MFlash;
    }
    else if (fwImageExecutesFrom (__start_core_m0app_MFlash,
                                  __start_core_m0app_Ram))
    {
        // Flash image have to be loaded to the proper Ram base address
        // FIXME: How to know the actual firmware size on FLASH? FlashFwOctets
        // must be 4K aligned and so maybe bigger than RamFwOctets, but fw 
        // image might be smaller.
        memcpy (__start_core_m0app_Ram, __start_core_m0app_MFlash,
                RamFwOctets < FlashFwOctets? RamFwOctets : FlashFwOctets);

        const void *const Ram = (const void *)__start_core_m0app_Ram;

        LOG (V, EXEC_FROM_SRAM_STR, Ram);

        // Image executes from SRAM
        return __start_core_m0app_Ram;
    }

    LOG (V, INVALID_ADAPTER_RESET_HANDLER_STR);
    LOG_Items (1, LANG_ADDRESS, __start_core_m0app_MFlash[1]);

    return NULL;
}


#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
static uint32_t * loadFromFilesystem (struct VIDEO *const V,
                                      const uint32_t FsDrive,
                                      const uint32_t RamFwOctets)
{
    BOARD_AssertParams (FsDrive < 10);

    LOG (V, LANG_LOAD_FROM_FILESYSTEM);
    LOG_Items (2,
                    LANG_VOLUME,    FsDrive,
                    LANG_PATH,      VDA_FILEPATH);

    char filepath[sizeof VDA_FILEPATH];

    memcpy (filepath, VDA_FILEPATH, sizeof VDA_FILEPATH);

    // Change drive accordingly
    filepath[0] = FsDrive + 48;

    FRESULT res;
    FILINFO fno;

    if ((res = f_stat(filepath, &fno)) != FR_OK)
    {
        LOG (V, NO_VIDEO_DRIVER_FOUND_STR);
        LOG_Items (2,
                        LANG_PATH,      filepath,
                        LANG_ERROR,     res);

        return NULL;
    }

    if (fno.fsize > RamFwOctets)
    {
        LOG (V, FILE_BIGGER_THAN_SRAM_STR);
        LOG_Items (3,
                        LANG_PATH,          filepath,
                        LANG_FILE_SIZE,         fno.fsize,
                        SRAM_SIZE_ITEM_STR,         RamFwOctets);

        return NULL;
    }

    FIL fil;

    if ((res = f_open(&fil, filepath, FA_READ)) != FR_OK)
    {
        LOG (V, ERROR_OPENING_VIDEO_DRIVER_STR);
        LOG_Items (2,
                        LANG_PATH,          filepath,
                        LANG_ERROR,             res);

        return NULL;
    }

    UINT    br = 0;
    uint8_t sectorData[512];

    for (uint32_t octets = 0; octets < fno.fsize; octets += br)
    {
        if ((res = f_read(&fil, sectorData, 512, &br)) != FR_OK)
        {
            LOG (V, ERROR_READING_VIDEO_DATA_STR);
            LOG_Items (4,
                        LANG_PATH,          filepath,
                        LANG_FILE_SIZE,     fno.fsize,
                        LANG_OCTETS,        octets,
                        LANG_ERROR,         res);

            return NULL;
        }

        uint8_t *dest = (uint8_t *)((uintptr_t)__start_core_m0app_Ram + octets);

        memcpy (dest, sectorData, 512);
    }

    LOG (V, FILE_LOADED_SRAM_STR);
    LOG_Items (2,
                    LANG_PATH,          filepath,
                    LANG_FILE_SIZE,     fno.fsize);

    // Image executes from SRAM
    return __start_core_m0app_Ram;
}
#endif


static uint32_t * loadFromCache (struct VIDEO *const V,
                                 const uint32_t RamFwOctets)
{
    LOG (V, LANG_LOAD_FROM_CACHE);
    LOG_Items (1, LANG_CACHED_ELEMENT, VDA_FIRMWARE_FILENAME);

    uint8_t sectorData[512];

    // Look for an element called VDA_FIRMWARE_FILENAME
    for (uint32_t e = 0; e < STORAGE_CachedElementsCount(); ++e)
    {
        struct STORAGE_CACHE_ElementInfo info;

        RAWSTOR_Status_Result rsr;

        rsr = STORAGE_CACHE_ElementInfo(&info, e, sectorData, 1);

        if (rsr != RAWSTOR_Status_Result_Ok)
        {
            continue;
        }

        // Remove path, if any
        const char *filename = strrchr (info.filepath, '/');
        if (!filename)
        {
            continue;
        }

        // Remove '/'
        filename += 1;

        if (!strncmp(filename, VDA_FIRMWARE_FILENAME,
                                        sizeof VDA_FIRMWARE_FILENAME))
        {
            if (info.octets > RamFwOctets)
            {
                LOG (V, CACHED_BIGGER_THAN_SRAM_STR);
                LOG_Items (4,
                        LANG_CACHED_ELEMENT,        e,
                        LANG_PATH,                  info.filepath,
                        LANG_FILE_SIZE,             info.octets,
                        SRAM_SIZE_ITEM_STR,         RamFwOctets);

                return NULL;
            }

            // Load firmware directly to SRAM
            if (STORAGE_CACHE_ElementData(&info, 0, 0,
                                    (uint8_t *)__start_core_m0app_Ram, 1)
                != RAWSTOR_Status_Result_Ok)
            {
                return NULL;
            }

            const uint32_t SramAddr = (uint32_t)(uintptr_t)
                                                    __start_core_m0app_Ram;

            LOG (V, CACHED_LOADED_SRAM_STR);
            LOG_Items (3,
                    LANG_CACHED_ELEMENT,    e,
                    LANG_PATH,              info.filepath,
                    LANG_ADDRESS,           SramAddr,
                    LOG_ItemsBases (
                        0, 0, VARIANT_Base_Hex));

            // Image executes from SRAM
            return __start_core_m0app_Ram;
        }
    }

    LOG (V, LANG_CACHED_ELEMENT_NOT_FOUND);
    LOG_Items (1, LANG_CACHED_ELEMENT, VDA_FIRMWARE_FILENAME);

    return NULL;
}


static bool loadVideoAdapter (struct VIDEO *const V)
{
    uint32_t * fwAddr = 0;

    const uint32_t FlashFwOctets = 
                (__top_core_m0app_MFlash - __start_core_m0app_MFlash) << 2;
    const uint32_t RamFwOctets =
                (__top_core_m0app_Ram - __start_core_m0app_Ram) << 2;


    // Load or execute from flash if the ld script reserves flash storage
    // for this purpose.
    if (FlashFwOctets)
    {
        fwAddr = loadFromFlash (V, FlashFwOctets, RamFwOctets);
    }

#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
    // Load from filesystem drives, if available.
    if (!fwAddr && STORAGE_ValidVolume(STORAGE_Role_FatFsDrive0))
    {
        fwAddr = loadFromFilesystem (V, 0, RamFwOctets);
    }

    if (!fwAddr && STORAGE_ValidVolume(STORAGE_Role_FatFsDrive1))
    {
        fwAddr = loadFromFilesystem (V, 1, RamFwOctets);
    }
#endif

    // Load from linear cache, if available.
    if (!fwAddr && STORAGE_ValidVolume(STORAGE_Role_LinearCache) &&
        STORAGE_CachedElementsCount())
    {
        fwAddr = loadFromCache (V, RamFwOctets);
    }

    if (!fwAddr)
    {
        LOG_Warn (V, NO_VIDEO_DRIVER_FOUND_STR);
        return false;
    }

    // Check stack top at the start of binary image
    if (fwAddr[0] != (uintptr_t)__top_core_m0app_Ram)
    {
        LOG_Warn (V, INVALID_DRIVER_STACK_PTR_STR);
        LOG_Items (1, LANG_ADDRESS, fwAddr[0],
                          LOG_ItemsBases (VARIANT_Base_Hex));

        BOARD_AssertInitialized (false);
    }

    s_vbi = 0;

    // Start video driver execution
    Chip_RGU_TriggerReset       (RGU_M0APP_RST);
	Chip_Clock_Enable           (CLK_M4_M0APP);
	Chip_CREG_SetM0AppMemMap    ((uintptr_t) fwAddr);
	Chip_RGU_ClearReset         (RGU_M0APP_RST);

    // Wait for 5 vbi cycles in less than 200 ms to confirm adapter execution
    // Timeout must account for time spent at adapter initialization.
    const TIMER_Ticks Timeout = TICKS_Now() + 200;

    do
    {
        __WFI ();

        if (TICKS_Now() >= Timeout)
        {
            LOG_Warn (V, VIDEO_ADAPTER_MALFUNCTION_STR);

            if (g_videoExchange.errorCode)
            {
                LOG_Items (2,
                            LANG_ERROR,         g_videoExchange.errorCode,
                            IODATA_ITEM_STR,    g_videoExchange.d);
            }

            // -----------------------------------------------------------------
            // The M0 core must remain active (not in reset state) to start a
            // M0 debug session to replay and inspect this error.
            // -----------------------------------------------------------------
            // Chip_RGU_TriggerReset   (RGU_M0APP_RST);
            // Chip_Clock_Disable      (CLK_M4_M0APP);

            BOARD_AssertInitialized (false);
        }
    }
    while (s_vbi < 5);

    return true;
}


void hardwareInit (struct VIDEO *const V)
{
    memset (&g_videoExchange, 0, sizeof(struct VIDEO_DUALCORE_Exchange));

    // Vertical Blanking Interrupt from M0 video adapter
    NVIC_DisableIRQ     (M0APP_IRQn);
    NVIC_SetPriority    (M0APP_IRQn, 5);
    NVIC_EnableIRQ      (M0APP_IRQn);

    BOARD_AssertState (loadVideoAdapter (V));

    // This fields are set once after initialization
    V->adapterDescription   = g_videoExchange.description;
    V->adapterSignal        = g_videoExchange.signal;
    V->adapterModeline      = g_videoExchange.modeline;
    V->adapterBuild         = g_videoExchange.build;
}


bool reachedVBICount (struct VIDEO *const V)
{
    return (s_vbi >= V->waitVbiCount)? true : false;
}


void waitForVBI (struct VIDEO *const V)
{
    if (V->waitVbiCount)
    {
        // Missed a complete count of Vertical Blanking Interrupts
        if (s_vbi > V->waitVbiCount)
        {
            V->vbiCountMisses += s_vbi - V->waitVbiCount;
            // Wait just one more VBI to synchronize
            s_vbi = V->waitVbiCount - 1;
        }

        // Wait for the required number of vertical blanking intervals
        while (s_vbi <= V->waitVbiCount)
        {
            __WFE ();
        }
    }
    s_vbi = 0;
}


void frameTransition (struct VIDEO *const V)
{
    // Exchange data from/to video driver
    videoExchangeWrite (V);
    videoExchangeRead (V);
}
