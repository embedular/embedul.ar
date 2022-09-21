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

#pragma once

#include "embedul.ar/source/core/manager/storage.h"


typedef bool (* STORAGE_CACHE_ProcessBreakFunc)(void *const Param);


enum STORAGE_CACHE_CheckFlags
{
    STORAGE_CACHE_CheckFlags_Volume         = 0x0001,
    STORAGE_CACHE_CheckFlags_Read           = 0x0002,
    STORAGE_CACHE_CheckFlags_Write          = 0x0004,   
    STORAGE_CACHE_CheckFlags_Checksum       = 0x0008, 
    STORAGE_CACHE_CheckFlags_Signature      = 0x0010, 
    STORAGE_CACHE_CheckFlags_FwkVersion     = 0x0020,
    STORAGE_CACHE_CheckFlags_AppName        = 0x0040,
    STORAGE_CACHE_CheckFlags_AppVersion     = 0x0080,
    STORAGE_CACHE_CheckFlags_FileMetrics    = 0x0100,
    STORAGE_CACHE_CheckFlags_Filesystem     = 0x0200,
};


enum STORAGE_CACHE_Stage
{
    STORAGE_CACHE_Stage_Start = 0,
    STORAGE_CACHE_Stage_ReadSector0,
    STORAGE_CACHE_Stage_BeginIteration,
#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
    STORAGE_CACHE_Stage_ReadSlot,
    STORAGE_CACHE_Stage_SlotToElementInfo,
    STORAGE_CACHE_Stage_SlotToElementData,
#endif
    STORAGE_CACHE_Stage_CheckElementInfo,
    STORAGE_CACHE_Stage_CheckElementData,
    STORAGE_CACHE_Stage_NextIteration,
    STORAGE_CACHE_Stage_EndIteration,
#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
    STORAGE_CACHE_Stage_WriteSector0,
#endif
    STORAGE_CACHE_Stage_Done
};


struct STORAGE_CACHE_State
{
    bool                            skipElementDataCheck;
    bool                            hasSlots;
    uint32_t                        rwRetries;
    uint32_t                        progressMax;
    STORAGE_CACHE_ProcessBreakFunc  breakFunc;
    void                            * breakFuncParam;
    enum STORAGE_CACHE_Stage        lastTask;
    enum STORAGE_CACHE_Stage        nextTask;
    uint32_t                        elementCount;
    uint32_t                        element;
#ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
    bool                            writeSector0;
    FIL                             file;
    FILINFO                         fno;
#endif
    uint32_t                        storedFileDate;
    uint32_t                        storedFileTime;
    uint32_t                        octets;
    uint32_t                        sectorBegin;
    uint32_t                        sectorEnd;
    uint32_t                        sectorCurrent;
    uint32_t                        progressDelta;
    uint32_t                        progressCurrent;
    uint32_t                        storedCrc32;
    uint32_t                        checkCrc32;
    char                            filepath[64];
    enum STORAGE_CACHE_CheckFlags   checksFailed;
    enum STORAGE_CACHE_CheckFlags   checksPassed;
};


struct STORAGE_CACHE_ElementInfo
{
    uint32_t        octets;
    uint32_t        sectorBegin;
    uint32_t        sectorEnd;
    uint32_t        sectorCount;
    char            filepath[64];
};


void    STORAGE_CACHE_Init              (struct STORAGE_CACHE_State *const Cs,
                                         const bool SkipElementDataCheck,
                                         const uint32_t RwRetries,
                                         const uint8_t ProgressMax,
                                         const STORAGE_CACHE_ProcessBreakFunc
                                         BreakFunc, void *const BreakFuncParam);
bool    STORAGE_CACHE_ProcessNextTask   (struct STORAGE_CACHE_State *const Cs);
uint32_t
        STORAGE_CACHE_ElementProgress   (const struct STORAGE_CACHE_State
                                         *const Cs);
bool    STORAGE_CACHE_ElementFinished   (const struct STORAGE_CACHE_State
                                         *const Cs);
RAWSTOR_Status_Result
        STORAGE_CACHE_ElementInfo       (struct STORAGE_CACHE_ElementInfo *const
                                         Ei, const uint32_t Element,
                                         uint8_t SectorData[const static 512],
                                         const uint32_t Retries);
RAWSTOR_Status_Result
        STORAGE_CACHE_ElementData       (const struct 
                                         STORAGE_CACHE_ElementInfo *const Ei,
                                         const uint32_t SectorDelta,
                                         const uint32_t SectorCount,
                                         uint8_t *const ElementData,
                                         const uint32_t Retries);
