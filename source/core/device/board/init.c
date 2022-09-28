/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [BOARD] bootup initialization sequence.

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

#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/core/manager/log.h"
#include "embedul.ar/source/core/manager/storage/cache.h"
#include <stdint.h>


#define GREET_LOGO_1        "`F36_________`L"
#define GREET_LOGO_2        "`F35/\\`P7/\\`L"
#define GREET_LOGO_3        "`F34/  \\_____/  \\`L"
#define GREET_LOGO_4        "`F33/   /\\   '\\   \\`L"
#define GREET_LOGO_5        "`F32/___/..\\_'__\\___\\`L"
#define GREET_LOGO_6        "`F32\\   \\  / '  /   /`L"
#define GREET_LOGO_7        "`F33\\   \\/___'/   /`L"
#define GREET_LOGO_8        "`F34\\  /`P5\\  /`L"
#define GREET_LOGO_9        "`F35\\/_______\\/`L2"
#define GREET_LOGO_10       "`F27|`P13|`P5|`L" 
#define GREET_LOGO_11       "`F17,---.,-.-.|---.,---.,---|.   .|`P5,---.,---.`L"
#define GREET_LOGO_12       "`F17|---'| | ||   ||---'|   ||   ||`P5,---||`L"
#define GREET_LOGO_13       "`F17``---'`` ' '``---'``---'``---'``---'``---'o" \
                            "``---^```L"

#define GREET_FMT           "`L1" \
                            "`M40`0`L1" \
                            "`M40`1`L1" \
                            "`M40`2`L1" \
                            "`M40`3`L1" \
                            "`M40`4`L1"

#define GREET_0_DESC        LOG_BASE_BOLD("embedded systems framework")
#define GREET_1_AUTH        "© 2018-2022 santiago germino"
#define GREET_2_WEB         "http://embedul.ar"
#define GREET_3_BUILD       CC_BuildInfoStr
#define GREET_4_VER         CC_VcsFwkVersionStr

#define GREET_APP_FMT       "`M40`0`L1" \
                            "`M40`1`L1" \
                            "`M40`2`L1"

#define GREET_APP_0_CAPTION LOG_BASE_BOLD("application name")
#define GREET_APP_1_NAME    CC_AppNameStr
#define GREET_APP_2_VER     CC_VcsAppVersionStr

#define GREET_APP_RIG_FMT   "`L1" \
                            "`M40`0`L1" \
                            "`M40`1`L1"

#define GREET_APP_0_RIGGED  LOG_BASE_BOLD("rigged setup")

#define GREET_BOARD_FMT     "`M40`0`L1"

#define GREET_BOARD_0       LOG_BASE_BOLD("board")


#define LOG_ITEM_OK_STR                     LOG_PENDING_OK_STR
#define LOG_ITEM_FAILED_STR                 LOG_PENDING_FAILED_STR
#define LOG_ITEM_UNTESTED_STR               " "

#define CHECK_EDATA_OVERRIDE_PROFILE        INPUT_PROFILE_Type_MAIN
#define CHECK_EDATA_OVERRIDE_INB            INPUT_PROFILE_MAIN_Bit_A
#define CHECK_EDATA_OVERRIDE_TIMEOUT        5000


#define LOG_ELEMENTS_STR(_cs) \
    ((_cs->checksPassed & STORAGE_CACHE_CheckFlags_Signature)? \
        VARIANT_ToString(&VARIANT_SpawnUint(_cs->elementCount)) : "")

#define LOG_FILE_STR(_cs) \
    ((_cs->checksPassed & STORAGE_CACHE_CheckFlags_Checksum)? \
        _cs->shortFilename : "")

#define LOG_CHECK_STR(_cs,_cn) \
    ((_cs->checksFailed & STORAGE_CACHE_CheckFlags_ ## _cn)? \
        LOG_ITEM_FAILED_STR : \
            (_cs->checksPassed & STORAGE_CACHE_CheckFlags_ ## _cn)? \
                LOG_ITEM_OK_STR : LOG_ITEM_UNTESTED_STR)


static void stageInit (struct BOARD *const B,
                       const enum BOARD_Stage Stage)
{
/*
    Initializes the board first, then the rig.
    The rig can set IO profiles, new IO drivers or reassign already set Comm
    drivers in the following init steps:

    BOARD_Stage_InitHardware
    BOARD_Stage_InitGreetings
    BOARD_Stage_InitIOProfiles
    BOARD_Stage_InitIOLevel1Drivers
    BOARD_Stage_InitCommDrivers
    BOARD_Stage_InitStorageDrivers
    BOARD_Stage_InitIOLevel2Drivers
    BOARD_Stage_InitReady
*/
    B->iface->StageChange (B, Stage);

    if (B->rig)
    {
        B->rig->iface->StageChange (B->rig, Stage);
    }
}


static void * stageInitObj (struct BOARD *const B,
                            const enum BOARD_Stage Stage)
{
/*
    Returns the rig object, if available, or the board object instead in
    the following init steps:

    BOARD_Stage_InitDebugStreamDriver
    BOARD_Stage_InitRandomDriver
    BOARD_Stage_InitVideoDriver
    BOARD_Stage_InitSoundDriver
*/
    void * ret = NULL;

    if (B->rig)
    {
        ret = B->rig->iface->StageChange (B->rig, Stage);
    }

    if (!ret)
    {
        ret = B->iface->StageChange (B, Stage);
    }

    return ret;
}


#define BOARD_INIT_Component(_b,_stage,_obj) \
    _b->currentStage = _stage; \
    _Generic((_obj), \
        struct NOBJ *   : stageInit(_b,_stage), \
        struct STREAM * : _obj = stageInitObj(_b,_stage), \
        struct RANDOM * : _obj = stageInitObj(_b,_stage), \
        struct VIDEO *  : _obj = stageInitObj(_b,_stage), \
        struct SOUND *  : _obj = stageInitObj(_b,_stage) \
    )


static void frameworkGreetings (struct BOARD *const B)
{
    struct STREAM *const S = B->debugStream;

    STREAM_IN_FromParsedString (S, 0, 512, GREET_LOGO_1);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_LOGO_2);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_LOGO_3);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_LOGO_4);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_LOGO_5);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_LOGO_6);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_LOGO_7);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_LOGO_8);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_LOGO_9);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_LOGO_10);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_LOGO_11);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_LOGO_12);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_LOGO_13);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_FMT,
                                           GREET_0_DESC,
                                           GREET_1_AUTH,
                                           GREET_2_WEB,
                                           GREET_3_BUILD,
                                           GREET_4_VER);
}


static void applicationGreetings (struct BOARD *const B)
{
    struct STREAM *const S = B->debugStream;

    STREAM_IN_FromParsedString (S, 0, 512, GREET_APP_FMT,
                                           GREET_APP_0_CAPTION,
                                           GREET_APP_1_NAME,
                                           GREET_APP_2_VER);

    if (B->rig)
    {
        STREAM_IN_FromParsedString (S, 0, 512, GREET_APP_RIG_FMT,
                                               GREET_APP_0_RIGGED,
                                               B->rig->iface->Description);
    }
}


static void boardGreetings (struct BOARD *const B)
{
    struct STREAM *const S = B->debugStream;

    STREAM_IN_FromParsedString (S, 0, 512, GREET_BOARD_FMT,
                                           GREET_BOARD_0);

    BOARD_INIT_Component (B, BOARD_Stage_Greetings, NOBJ);
}


static const struct LOG_Table s_IOProfilesTable =
{
    LANG_IO_PROFILES, 4,
    {
        {
            LANG_NAME, 32, 0
        },
        {
            LANG_AVAILABLE, 47, 0
        },
        {
            "bits", 62, 0
        },
        {
            "ranges", 0, 0
        }
    }
};


static const struct LOG_Table s_CommMgrTable =
{
    LANG_RESOURCES, 3,
    {
        {
            LANG_TYPE, 14, 0
        },
        {
            LANG_PURPOSE, 46, 0
        },
        {
            LANG_DRIVER_DESCRIPTION, 0, 0
        }
    }
};


static const struct LOG_Table s_IODeviceTable =
{
    LANG_DEVICES, 3,
    {
        {
            LANG_ID, 14, 0
        },
        {
            LANG_DRIVER, 46, 0
        },
        {
            LANG_SOURCE, 0, 0
        }
    }
};


static const struct LOG_Table s_InputMappingTable =
{
    LANG_MAPPINGS, 5,
    {
        {
            LANG_TYPE, 14, 0
        },
        {
            LANG_CODE, 24, 0
        },
        {
            LANG_DEVICE_ID, 35, 0
        },
        {
            LANG_DRIVER_ID, 46, 0
        },
        {
            LANG_NAME, 0, 0
        }
    }
};


static const struct LOG_Table s_OutputMappingTable =
{
    LANG_MAPPINGS, 5,
    {
        {
            LANG_TYPE, 14, 0
        },
        {
            LANG_CODE, 24, 0
        },
        {
            LANG_DEVICE, 35, 0
        },
        {
            LANG_DRIVER_ID, 46, 0
        },
        {
            LANG_NAME, 0, 0
        }
    }
};


static const struct LOG_Table s_StorageVolumeTable =
{
    LANG_VOLUMES, 6,
    {
        {
            LANG_ROLE, 7, 0
        },
        {
            LANG_DRIVER, 29, 0
        },
        {
            LANG_DEVICE_SECTOR_BEGIN, 42, 0
        },
        {
            LANG_DEVICE_SECTOR_END, 55, 0
        },
        {
            LANG_PARTITION_NUNMBER_SHORT, 66, 0
        },
        {
            LANG_PARTITION_TYPE_SHORT, 0, VARIANT_Base_Hex
        }
    }
};


static const struct LOG_Table s_CheckSector0Table =
{
    LANG_SECTOR_0, 8,
    {
        {
            LANG_VOLUME, 11, 0
        },
        {
            LANG_DEVICE_SECTOR_READ, 23, 0
        },
        {
            LANG_CHECKSUM, 32, 0
        },
        {
            LANG_SIGNATURE, 42, 0
        },
        {
            LANG_FWK_VER_SHORT, 50, 0
        },
        {
            LANG_APP_VER_SHORT, 58, 0
        },
        {
            LANG_APP_NAME_SHORT, 67, 0
        },
        {
            LANG_ELEMENTS, 0, 0
        }
    }
};


bool BOARD_INIT_InputCountdown (const enum INPUT_PROFILE_Type ProfileType,
                                const uint32_t ProfileInb,
                                const TIMER_Ticks Countdown,
                                const char *const Msg)
{
    if (!INPUT_IsBitMapped(ProfileType, ProfileInb))
    {
        return true;
    }

    LOG (NOBJ, Msg, INPUT_MappedBitName(ProfileType, ProfileInb));

    const TIMER_Ticks   Timeout = BOARD_TicksNow () + Countdown;
    const uint32_t      LPM     = LOG_ProgressMax ();

    uint32_t lastOutColumn = LOG_ProgressBegin (LOG_ProgressType_Timeout);
    uint32_t lastBitBuffer;

    do
    {
        BOARD_Delay (100);
        BOARD_Update ();

        const TIMER_Ticks   Now = BOARD_TicksNow ();
        const int32_t       Pro = Countdown - (Timeout - Now);
        const uint32_t      Col = (Pro * LPM) / Countdown;

        if (Pro > 0)
        {
            lastOutColumn = LOG_ProgressUpdate (lastOutColumn, Col);
        }
        else 
        {
            LOG_ProgressUpdate (lastOutColumn, LPM);
        }

        lastBitBuffer = INPUT_GetBitBuffer(ProfileType, ProfileInb);
    }
    while (!lastBitBuffer && Timeout > BOARD_TicksNow());

    LOG_ProgressEnd ();

    // Timeout reached
    if (Timeout <= BOARD_TicksNow())
    {
        return true;
    }

    return lastBitBuffer? false : true;
}


static void streamOutArgs (struct BOARD *const S, const char *const Msg,
                           struct VARIANT *const ArgValues, 
                           const uint32_t ArgCount)
{
    STREAM_IN_FromParsedStringArgs (S->debugStream, 0, 512, Msg, 
                                    ArgValues, ArgCount);
}


static void boardInitSequenceBegin (struct BOARD *const B)
{
    // Debug stream available, but still no log output until LOG_Init().
    //
    // The log manager uses the stream returned by
    // BOARD_Component_DebugStreamDriver. It has no elements to set
    // or configure.
    //
    // LOG_Init() will start the board init sequence context before its own
    // init context. Call boardInitSequenceEnd() to close that log context.
    LOG_Init (&B->log, B, B->debugStream);
}


static void initManagers (struct BOARD *const B)
{
    OUTPUT_Init     (&B->output);
    INPUT_Init      (&B->input);
    COMM_Init       (&B->comm);
    STORAGE_Init    (&B->storage);
}


typedef const char * (* PROFILE_GetTypeNameFunc)(const uint32_t);


static void showProfilesSummary (const char *const ProfilesGroup,
                                 const struct INPUT_PROFILE *const Profiles,
                                 const uint32_t ProfilesCount,
                                 const PROFILE_GetTypeNameFunc GetTypeName)
{
    LOG_AutoContext (NOBJ, ProfilesGroup);

    LOG_TableBegin (&s_IOProfilesTable);

    uint32_t enabledProfilesCount = 0;

    for (enum INPUT_PROFILE_Type t = 0; t < ProfilesCount; ++t)
    {
        const struct INPUT_PROFILE *const P = &Profiles[t];
        const bool EnabledProfile = (P->bitMap || P->rangeMap);

        LOG_TableEntry (&s_IOProfilesTable, GetTypeName(t),
                        (EnabledProfile)? LOG_ITEM_OK_STR : LOG_ITEM_FAILED_STR,
                        P->bitCount, P->rangeCount);

        if (EnabledProfile)
        {
            ++ enabledProfilesCount;
        }
    }

    LOG_TableEnd (&s_IOProfilesTable);

    if (!enabledProfilesCount)
    {
        LOG_Warn (NOBJ, LANG_NO_PROFILES_SET);
    }
}


static void initIoProfiles (struct BOARD *const B)
{
    LOG_AutoContext (NOBJ, LANG_IO_PROFILES);

    BOARD_INIT_Component (B, BOARD_Stage_InitIOProfiles, NOBJ);

    showProfilesSummary (LANG_INPUT_MGR_SUMMARY, B->input.profiles,
                         INPUT_PROFILE_Type__COUNT,
                         INPUT_PROFILE_GetTypeName);
/*
    showProfilesSummary (LANG_OUTPUT_MGR_SUMMARY, B->output.profiles,
                         OUTPUT_PROFILE_Type__COUNT, 
                         OUTPUT_PROFILE_GetTypeName);
*/
}


static void initRandomDriver (struct BOARD *const B)
{
    LOG_AutoContext (NOBJ, LANG_RNG_DRIVER);

    BOARD_INIT_Component (B, BOARD_Stage_InitRandomDriver, B->random);
}


static void initIoLevel1Drivers (struct BOARD *const B)
{
    LOG_AutoContext (NOBJ, LANG_IO_LEVEL_1_DRIVERS);

    BOARD_INIT_Component (B, BOARD_Stage_InitIOLevel1Drivers, NOBJ);
}


static void initCommDrivers (struct BOARD *const B)
{
    // The communication manager is a collection of streams used to send or
    // receive data. Each stream or packet serves a purpose specified in 
    // COMM_Stream or COMM_Packet enums.

    // COMM_Purpose_DebugMessages is automatically set with the STREAM returned
    // by BOARD_Component_DebugStreamDriver.

    LOG_AutoContext (NOBJ, LANG_COMM_DRIVERS);

    // Default LOG stream. This can be overriden by the board or rig.
    COMM_SetStream (COMM_Stream_Log, B->debugStream);

    BOARD_INIT_Component (B, BOARD_Stage_InitCommDrivers, NOBJ);
}


static void initStorageDrivers (struct BOARD *const B)
{
    LOG_AutoContext (NOBJ, LANG_STORAGE_DRIVERS);

    BOARD_INIT_Component (B, BOARD_Stage_InitStorageDrivers, NOBJ);
}


static void initIoLevel2Drivers (struct BOARD *const B)
{
    LOG_AutoContext (NOBJ, LANG_IO_LEVEL_2_DRIVERS);

    BOARD_INIT_Component (B, BOARD_Stage_InitIOLevel2Drivers, NOBJ);
}


static void showOutputManagerSummary (struct BOARD *const B)
{
    LOG_AutoContext (NOBJ, LANG_OUTPUT_MGR_SUMMARY);

    // List devices
    LOG_TableBegin (&s_IODeviceTable);
    for (uint32_t deviceId = 0; deviceId < OUTPUT_MAX_DEVICES; ++deviceId)
    {
        struct OUTPUT_Device * dev = &B->output.device[deviceId];

        if (dev->driver)
        {
            LOG_TableEntry (&s_IODeviceTable,
                                deviceId, 
                                IO_Description(dev->driver),
                                dev->driverSource);
        }
        #ifdef BOARD_INIT_SHOW_NOT_SET
        else 
        {
            LOG_TableEntry (&s_IODeviceTable,
                                deviceId, 
                                "",
                                "");
        }
        #endif
    }
    LOG_TableEnd (&s_IODeviceTable);

    LOG_Newline ();

    // List bit and range mappings
    LOG_TableBegin (&s_OutputMappingTable);
    for (uint32_t outb = 0; outb < OUTPUT_Bit__COUNT; ++outb)
    {
        struct OUTPUT_Mapping * map = &B->output.bitMapping[outb];

        if (map->driverIndex != IO_INVALID_INDEX)
        {
            LOG_TableEntry (&s_OutputMappingTable,
                                    "bit", outb,
                                    map->deviceId, map->driverIndex,
                                    OUTPUT_BitName(outb));
        }
        #ifdef BOARD_INIT_SHOW_NOT_SET
        else 
        {
            LOG_TableEntry (&s_OutputMappingTable,
                                    "bit", outb,
                                    "", "",
                                    "");
        }
        #endif
    }

    for (uint32_t outr = 0; outr < OUTPUT_Range__COUNT; ++outr)
    {
        struct OUTPUT_Mapping * map = &B->output.rangeMapping[outr];

        if (map->driverIndex != IO_INVALID_INDEX)
        {
            LOG_TableEntry (&s_OutputMappingTable,
                                    "range", outr,
                                    map->deviceId, map->driverIndex,
                                    OUTPUT_RangeName(outr));
        }
        #ifdef BOARD_INIT_SHOW_NOT_SET
        else 
        {
            LOG_TableEntry (&s_OutputMappingTable,
                                    "range", outb,
                                    "", "",
                                    "");
        }
        #endif
    }
    LOG_TableEnd (&s_OutputMappingTable);
}


static void showInputManagerSummary (struct BOARD *const B)
{
    LOG_AutoContext (NOBJ, LANG_INPUT_MGR_SUMMARY);

    // List devices
    LOG_TableBegin (&s_IODeviceTable);
    for (uint32_t deviceId = 0; deviceId < INPUT_MAX_DEVICES; ++deviceId)
    {
        struct INPUT_Device * dev = &B->input.device[deviceId];

        if (dev->driver)
        {
            LOG_TableEntry (&s_IODeviceTable,
                                deviceId, 
                                IO_Description(dev->driver),
                                dev->driverSource);
        }
        #ifdef BOARD_INIT_SHOW_NOT_SET
        else 
        {
            LOG_TableEntry (&s_InputDeviceTable,
                                deviceId, 
                                "",
                                "");
        }
        #endif
    }
    LOG_TableEnd (&s_IODeviceTable);

    LOG_Newline ();

    for (enum INPUT_PROFILE_Type t = 0; t < INPUT_PROFILE_Type__COUNT; ++t)
    {
        {
            LOG_AutoContext (NOBJ, INPUT_PROFILE_GetTypeName(t));

            // List bit and range mappings
            LOG_TableBegin (&s_InputMappingTable);

            const struct INPUT_PROFILE *const P = &B->input.profiles[t];

            for (uint32_t inb = 0; inb < P->bitCount; ++inb)
            {
                const struct INPUT_PROFILE_Map *const Map = &P->bitMap[inb];

                if (Map->driverIndex != IO_INVALID_INDEX)
                {
                    LOG_TableEntry (&s_InputMappingTable,
                                            "bit", inb,
                                            Map->deviceId, Map->driverIndex,
                                            INPUT_MappedBitName(t, inb));
                }
                #ifdef BOARD_INIT_SHOW_NOT_SET
                else 
                {
                    LOG_TableEntry (&s_InputMappingTable,
                                            "bit", inb,
                                            "", "",
                                            "");
                }
                #endif
            }

            for (uint32_t inr = 0; inr < P->rangeCount; ++inr)
            {
                const struct INPUT_PROFILE_Map *const Map = &P->bitMap[inr];

                if (Map->driverIndex != IO_INVALID_INDEX)
                {
                    LOG_TableEntry (&s_InputMappingTable,
                                            "range", inr,
                                            Map->deviceId, Map->driverIndex,
                                            INPUT_MappedRangeName(t, inr));
                }
                #ifdef BOARD_INIT_SHOW_NOT_SET
                else 
                {
                    LOG_TableEntry (&s_InputMappingTable,
                                            "range", inr,
                                            "", "",
                                            "");
                }
                #endif
            }

            LOG_TableEnd (&s_InputMappingTable);
        }
    }
}


static void showCommManagerSummary (struct BOARD *const B)
{
    LOG_AutoContext (NOBJ, LANG_COMM_MGR_SUMMARY);

    LOG_TableBegin (&s_CommMgrTable);

    for (enum COMM_Stream i = 0; i < COMM_Stream__COUNT; ++i)
    {
        struct STREAM *const Stream = B->comm.stream[i];

        if (Stream)
        {
            LOG_TableEntry (&s_CommMgrTable,
                                "stream",
                                COMM_StreamRoleDescription(i),
                                STREAM_Description(Stream));
        }
        #ifdef BOARD_INIT_SHOW_NOT_SET
        else 
        {
            LOG_TableEntry (&s_CommMgrTable,
                                "stream",
                                COMM_StreamRoleDescription(i),
                                "");
        }
        #endif
    }

    for (enum COMM_Packet i = 0; i < COMM_Packet__COUNT; ++i)
    {
        struct PACKET *const Packet = B->comm.packet[i];

        if (Packet)
        {
            LOG_TableEntry (&s_CommMgrTable,
                                "packet",
                                COMM_PacketRoleDescription(i),
                                PACKET_Description(Packet));
        }
        #ifdef BOARD_INIT_SHOW_NOT_SET
        else 
        {
            LOG_TableEntry (&s_CommMgrTable,
                                "packet",
                                COMM_PacketRoleDescription(i),
                                "");
        }
        #endif
    }

    LOG_TableEnd (&s_CommMgrTable);
}


static void logSector0Status (const struct STORAGE_CACHE_State *const Cs)
{
    LOG_TableBegin (&s_CheckSector0Table);

    LOG_TableEntry (&s_CheckSector0Table,
                            LOG_CHECK_STR(Cs, Volume),
                            LOG_CHECK_STR(Cs, Read),
                            LOG_CHECK_STR(Cs, Checksum),
                            LOG_CHECK_STR(Cs, Signature),
                            LOG_CHECK_STR(Cs, FwkVersion),
                            LOG_CHECK_STR(Cs, AppVersion),
                            LOG_CHECK_STR(Cs, AppName),
                            LOG_ELEMENTS_STR(Cs));

    LOG_TableEnd (&s_CheckSector0Table);
}


static void logElementStatus (const struct STORAGE_CACHE_State *const Cs,
                              const char *const Action)
{
    LOG (NOBJ, "`0 `1(`2), `3(`4), `5(`6), `7(`8), `9(`10)",
                Action,
                LANG_READ,          LOG_CHECK_STR(Cs, Read),
                LANG_WRITE,         LOG_CHECK_STR(Cs, Write),
                LANG_CHECKSUM,      LOG_CHECK_STR(Cs, Checksum),
                LANG_FILE_METRICS,  LOG_CHECK_STR(Cs, FileMetrics),
                LANG_FILESYSTEM,    LOG_CHECK_STR(Cs, Filesystem));
}


static void logProgress (const struct STORAGE_CACHE_State *const Cs,
                         uint32_t *const LastOutColumn)
{
    // Data processing finished on next iteration, either successfully or
    // cancelled by an error.

    const bool NextTaskFinishedDataProc =
                #ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
                     Cs->nextTask != STORAGE_CACHE_Stage_SlotToElementData && 
                #endif
                     Cs->nextTask != STORAGE_CACHE_Stage_CheckElementData;

    if (NextTaskFinishedDataProc || STORAGE_CACHE_ElementFinished(Cs))
    {
        if (* LastOutColumn)
        {
            LOG_ProgressEnd ();
            * LastOutColumn = 0;
        }

        // Element data results
        logElementStatus (Cs, LANG_DATA);
    }
    else 
    {
        if (!(* LastOutColumn))
        {
            * LastOutColumn = LOG_ProgressBegin (LOG_ProgressType_Work);
        }

        * LastOutColumn = LOG_ProgressUpdate (* LastOutColumn,
                                        STORAGE_CACHE_ElementProgress(Cs));
    }
}


static bool logProcessBreakFunc (void *const Param)
{
    TIMER_Ticks *const Ticks = (TIMER_Ticks *) Param;

    if (*Ticks < BOARD_TicksNow())
    {
        *Ticks = BOARD_TicksNow() + 100;
        return true;
    }

    return false;
}


static void checkCache (const uint32_t ReadWriteRetries)
{
    bool skipElementDataCheck = false;

    if (!BOARD_INIT_InputCountdown (CHECK_EDATA_OVERRIDE_PROFILE,
                                    CHECK_EDATA_OVERRIDE_INB,
                                    CHECK_EDATA_OVERRIDE_TIMEOUT,
                                    LANG_SKIP_CACHE_ELEMENT_DATA_FMT))
    {
        skipElementDataCheck = true;
    }

    LOG_Newline ();

    struct STORAGE_CACHE_State  cs;
    TIMER_Ticks                 nextBreak = 0;

    STORAGE_CACHE_Init (&cs, skipElementDataCheck, ReadWriteRetries,
                        LOG_ProgressMax(), logProcessBreakFunc,
                        (void *)&nextBreak);

    {
        LOG_AutoContext (NOBJ, LANG_LINEAR_CACHE_CHECK);

        uint32_t    lastOutColumn           = 0;
        bool        elementContextOpened    = false;

        while (STORAGE_CACHE_ProcessNextTask (&cs))
        {
            switch (cs.lastTask)
            {
                case STORAGE_CACHE_Stage_Start:
                    break;

                case STORAGE_CACHE_Stage_ReadSector0:
                    logSector0Status (&cs);
                    break;

                case STORAGE_CACHE_Stage_BeginIteration:
                    BOARD_AssertState (!elementContextOpened);
                    LOG_ContextBegin (NOBJ, LANG_CACHED_ELEMENT);
                    elementContextOpened = true;
                    LOG_Items (1, LANG_ID, cs.element);
                    break;

            #ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
                case STORAGE_CACHE_Stage_ReadSlot:
                    break;

                case STORAGE_CACHE_Stage_SlotToElementInfo:
                    LOG_Items (2,
                                LANG_FILENAME,  cs.filepath,
                                LANG_OCTETS,    cs.fno.fsize);
                    LOG (NOBJ, LANG_UPDATING);
                    logElementStatus (&cs, LANG_INFO);
                    break;

                case STORAGE_CACHE_Stage_SlotToElementData:
                    logProgress (&cs, &lastOutColumn);
                    break;
            #endif // LIB_EMBEDULAR_HAS_FILESYSTEM

                case STORAGE_CACHE_Stage_CheckElementInfo:
                    LOG_Items (1,
                                LANG_FILENAME,  cs.filepath);
                    LOG (NOBJ, LANG_CHECKING);
                    logElementStatus (&cs, LANG_INFO);
                    break;

                case STORAGE_CACHE_Stage_CheckElementData:
                    logProgress (&cs, &lastOutColumn);
                    break;

                case STORAGE_CACHE_Stage_NextIteration:
                    if (elementContextOpened)
                    {
                        LOG_ContextEnd ();
                        elementContextOpened = false;
                    }
                    break;

                case STORAGE_CACHE_Stage_EndIteration:
                    // Tried to open a inexistent slot
                    if (cs.checksFailed & STORAGE_CACHE_CheckFlags_FileMetrics)
                    {
                        LOG (NOBJ, LANG_CACHE_FS_SLOT_NOT_FOUND);
                    }
                    if (elementContextOpened)
                    {
                        LOG_ContextEnd ();
                        elementContextOpened = false;
                    }
                    break;

            #ifdef LIB_EMBEDULAR_HAS_FILESYSTEM
                case STORAGE_CACHE_Stage_WriteSector0:
                    LOG (NOBJ, LANG_SECTOR_0_UPDATE);
                    break;
            #endif

                case STORAGE_CACHE_Stage_Done:
                    break;
            };
        }
    }
}


static void showStorageManagerSummary (struct BOARD *const B)
{
    LOG_AutoContext (NOBJ, LANG_STORAGE_MGR_SUMMARY);

#if LIB_EMBEDULAR_HAS_FILESYSTEM
    LOG (NOBJ, LANG_FILESYSTEM_SUPPORT_ENABLED);
    {
        LOG_AutoContext (NOBJ, LANG_CACHE_FS_SLOT_PATHS);

        LOG_Items (1, 
            LANG_FRAMEWORK, LIB_EMBEDULAR_STORAGE_CACHE_FS_FRAMEWORK_DIR);
        LOG_Items (1, 
            LANG_APPLICATION, LIB_EMBEDULAR_STORAGE_CACHE_FS_APPLICATION_DIR);
    }
#else
    LOG_Warn (NOBJ, LANG_NO_FILESYSTEM_SUPPORT);
#endif

    if (!STORAGE_VolumesSet ())
    {
        LOG_Warn (NOBJ, LANG_NO_STORAGE_VOLUMES_SET);
        LOG_Warn (NOBJ, LANG_NO_FILESYSTEM_NOR_CACHE);
    }
    else
    {
        LOG_TableBegin (&s_StorageVolumeTable);
        for (uint32_t role = 0; role < STORAGE_Role__COUNT; ++role)
        {
            struct STORAGE_Volume *vol = &B->storage.volume[role];

            if (vol->driver)
            {
                LOG_TableEntry (&s_StorageVolumeTable,
                                    role,
                                    RAWSTOR_Description(vol->driver),
                                    vol->info.sectorBegin,
                                    vol->info.sectorEnd,
                                    vol->info.partitionNr, 
                                    vol->info.partitionType);
            }
            #ifdef BOARD_INIT_SHOW_NOT_SET
            else
            {
                LOG_TableEntry (&s_StorageVolumeTable,
                                    role,
                                    "",
                                    "",
                                    "",
                                    "", 
                                    "");
            }
            #endif
        }
        LOG_TableEnd (&s_StorageVolumeTable);

        checkCache (3);
    }
}


static void showManagersSummary (struct BOARD *const B)
{
    showOutputManagerSummary    (B);
    showInputManagerSummary     (B);
    showCommManagerSummary      (B);
    showStorageManagerSummary   (B);
}


static void initVideoDriver (struct BOARD *const B)
{
    LOG_AutoContext (NOBJ, LANG_VIDEO_DRIVER);

    BOARD_INIT_Component (B, BOARD_Stage_InitVideoDriver, B->video);

    if (!B->video)
    {
        #if (LIB_EMBEDULAR_CONFIG_NEED_VIDEO == 1)
            BOARD_AssertInitialized (LANG_DEVICE_INIT_FAILED);
        #else
            LOG_Warn (B, LANG_DEVICE_INIT_FAILED);
        #endif
    }
}


static void initSoundDriver (struct BOARD *const B)
{
    LOG_AutoContext (NOBJ, LANG_SOUND_DRIVER);

    BOARD_INIT_Component (B, BOARD_Stage_InitSoundDriver, B->sound);

    if (!B->sound)
    {
        #if (LIB_EMBEDULAR_CONFIG_NEED_SOUND == 1)
            BOARD_AssertInitialized (LANG_DEVICE_INIT_FAILED);
        #else
            LOG_Warn (B, LANG_DEVICE_INIT_FAILED);
        #endif
    }
}


static void boardInitSequenceEnd (void)
{
    // Close the board init context opened in LOG_Init()
    LOG_ContextEnd ();
}


#if (LIB_EMBEDULAR_CONFIG_SPLASH_SCREENS == 1)
void CC_ExpPaste(SPLASH_THEME_,LIB_EMBEDULAR_CONFIG_SPLASH_THEME_L1)(void);
void CC_ExpPaste(SPLASH_THEME_,LIB_EMBEDULAR_CONFIG_SPLASH_THEME_L2)(void);
void CC_ExpPaste(SPLASH_THEME_,LIB_EMBEDULAR_CONFIG_SPLASH_THEME_L3)(void);
#endif


void BOARD_INIT_Sequence (struct BOARD *const B)
{
    // -------------------------------------------------------------------------
    // Assert checks available, but still NO assert output or log messages
    // -------------------------------------------------------------------------
    BOARD_AssertParams      (B);
    // -------------------------------------------------------------------------
    // No assert messages output. If board initialization fails, the board
    // interface implementation itself should inform the user what went wrong
    // by using an OS function, LED or other ad-hoc method.
    BOARD_INIT_Component    (B, BOARD_Stage_InitHardware, NOBJ);
    // -------------------------------------------------------------------------
    // System debug stream required for assert messages.
    // -------------------------------------------------------------------------
    BOARD_INIT_Component    (B, BOARD_Stage_InitDebugStreamDriver,
                             B->debugStream);
    BOARD_AssertInitialized (B->debugStream);
    // -------------------------------------------------------------------------
    // Assert messages working. Still no log messages.
    // -------------------------------------------------------------------------
    streamOutArgs           (B, LOG_BASE_COLOR "`L", NULL, 0);
    frameworkGreetings      (B);
    streamOutArgs           (B, LOG_BASE_COLOR "`L", NULL, 0);
    boardGreetings          (B);
    streamOutArgs           (B, LOG_BASE_COLOR "`L", NULL, 0);
    applicationGreetings    (B);
    streamOutArgs           (B, LOG_BASE_COLOR "`L", NULL, 0);
    // -------------------------------------------------------------------------
    // Starts the log manager
    // -------------------------------------------------------------------------
    boardInitSequenceBegin  (B);
    // -------------------------------------------------------------------------
    // Log messages available.
    // -------------------------------------------------------------------------
    initManagers            (B);
    initIoProfiles          (B);
    initRandomDriver        (B);
    initIoLevel1Drivers     (B);
    initCommDrivers         (B);
    initStorageDrivers      (B);
    initIoLevel2Drivers     (B);
    showManagersSummary     (B);
    initVideoDriver         (B);
    initSoundDriver         (B);
    boardInitSequenceEnd    ();
    // -------------------------------------------------------------------------
    // Board ready.
    // -------------------------------------------------------------------------
    BOARD_INIT_Component (B, BOARD_Stage_Ready, NOBJ);

    #if (LIB_EMBEDULAR_CONFIG_SPLASH_SCREENS == 1)
    BOARD_AssertState (BOARD_VideoDeviceOk());

    {
        LOG_AutoContext (B->video, LANG_SPLASH_SCREENS);

        CC_ExpPaste(SPLASH_THEME_,LIB_EMBEDULAR_CONFIG_SPLASH_THEME_L1)();
        CC_ExpPaste(SPLASH_THEME_,LIB_EMBEDULAR_CONFIG_SPLASH_THEME_L2)();
        CC_ExpPaste(SPLASH_THEME_,LIB_EMBEDULAR_CONFIG_SPLASH_THEME_L3)();
    }
    #endif
}
