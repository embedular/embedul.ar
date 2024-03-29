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

#include "embedul.ar/source/core/main.h"
#include "embedul.ar/source/core/manager/storage/cache.h"


#define GREET_FWK_LOGO_1    "`F36_________`L"
#define GREET_FWK_LOGO_2    "`F35/\\`P7/\\`L"
#define GREET_FWK_LOGO_3    "`F34/  \\_____/  \\`L"
#define GREET_FWK_LOGO_4    "`F33/   /\\   '\\   \\`L"
#define GREET_FWK_LOGO_5    "`F32/___/..\\_'__\\___\\`L"
#define GREET_FWK_LOGO_6    "`F32\\   \\  / '  /   /`L"
#define GREET_FWK_LOGO_7    "`F33\\   \\/___'/   /`L"
#define GREET_FWK_LOGO_8    "`F34\\  /`P5\\  /`L"
#define GREET_FWK_LOGO_9    "`F35\\/_______\\/`L2"
#define GREET_FWK_LOGO_10   "`F27|`P13|`P5|`L" 
#define GREET_FWK_LOGO_11   "`F17,---.,-.-.|---.,---.,---|.   .|`P5,---.,---.`L"
#define GREET_FWK_LOGO_12   "`F17|---'| | ||   ||---'|   ||   ||`P5,---||`L"
#define GREET_FWK_LOGO_13   "`F17``---'`` ' '``---'``---'``---'``---'``---'o" \
                            "``---^```L"

#define GREET_FWK_FMT       "`L1" \
                            "`M40`0`L1" \
                            "`M40`1`L1" \
                            "`M40`2`L1" \
                            "`M40`3`L1" \
                            "`M40`4`L1"
#define GREET_FWK_0_DESC    "embedded systems framework"
#define GREET_FWK_1_AUTH    "© 2018-2023 santiago germino"
#define GREET_FWK_2_WEB     "http://embedul.ar"
#define GREET_FWK_3_BUILD   CC_BuildInfoStr
#define GREET_FWK_4_VER     CC_VcsFwkVersionStr

#define GREET_BOARD_FMT     "`M40`0`L1"
#define GREET_BOARD_0       LOG_BASE_BOLD("board")

#define GREET_MTOS_FMT      "`M40`0`L1" \
                            "`M40`1`L1"
#define GREET_MTOS_0_TITLE  LOG_BASE_BOLD("multitasking os")

#define GREET_APP_FMT       "`M40`0`L1" \
                            "`M40`1`L1" \
                            "`M40`2`L1"
#define GREET_APP_0_TITLE LOG_BASE_BOLD("application name")
#define GREET_APP_1_NAME    CC_AppNameStr
#define GREET_APP_2_VER     CC_VcsAppVersionStr

#define GREET_RIG_FMT       "`L1" \
                            "`M40`0`L1" \
                            "`M40`1`L1"
#define GREET_RIG_0_TITLE   LOG_BASE_BOLD("rigged setup")


#define LOG_ITEM_OK_STR                     LOG_PENDING_OK_STR
#define LOG_ITEM_FAILED_STR                 LOG_PENDING_FAILED_STR
#define LOG_ITEM_UNTESTED_STR               " "

#define CHECK_EDATA_OVERRIDE_PROFILE_TYPE   INPUT_PROFILE_Group_MAIN
#define CHECK_EDATA_OVERRIDE_PROFILE_CODE   INPUT_PROFILE_MAIN_Bit_A
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
    BOARD_Stage_Greetings
    BOARD_Stage_InitIOProfiles
    BOARD_Stage_InitIOLevel1Drivers
    BOARD_Stage_InitCommDrivers
    BOARD_Stage_InitStorageDrivers
    BOARD_Stage_InitIOLevel2Drivers
    BOARD_Stage_InitScreenDrivers
    BOARD_Stage_InitIOLevel3Drivers
    BOARD_Stage_Ready
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
        struct SOUND *  : _obj = stageInitObj(_b,_stage) \
    )


static void frameworkGreetings (struct BOARD *const B)
{
    struct STREAM *const S = B->debugStream;

    STREAM_IN_FromParsedString (S, 0, 512, GREET_FWK_LOGO_1);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_FWK_LOGO_2);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_FWK_LOGO_3);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_FWK_LOGO_4);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_FWK_LOGO_5);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_FWK_LOGO_6);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_FWK_LOGO_7);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_FWK_LOGO_8);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_FWK_LOGO_9);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_FWK_LOGO_10);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_FWK_LOGO_11);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_FWK_LOGO_12);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_FWK_LOGO_13);
    STREAM_IN_FromParsedString (S, 0, 512, GREET_FWK_FMT,
                                           GREET_FWK_0_DESC,
                                           GREET_FWK_1_AUTH,
                                           GREET_FWK_2_WEB,
                                           GREET_FWK_3_BUILD,
                                           GREET_FWK_4_VER);
}


static void mtosGreetings (struct BOARD *const B)
{
    struct STREAM *const S = B->debugStream;

    STREAM_IN_FromParsedString (S, 0, 512, GREET_MTOS_FMT,
                                           GREET_MTOS_0_TITLE,
                                           OSWRAP_Description());
}


static void applicationGreetings (struct BOARD *const B)
{
    struct STREAM *const S = B->debugStream;

    STREAM_IN_FromParsedString (S, 0, 512, GREET_APP_FMT,
                                           GREET_APP_0_TITLE,
                                           GREET_APP_1_NAME,
                                           GREET_APP_2_VER);

    if (B->rig)
    {
        STREAM_IN_FromParsedString (S, 0, 512, GREET_RIG_FMT,
                                               GREET_RIG_0_TITLE,
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


static const struct LOG_Table s_IOGatewayTable =
{
    LANG_GATEWAYS, 3,
    {
        {
            LANG_ID, 14, 0
        },
        {
            LANG_DRIVER, 46, 0
        },
        {
            LANG_PORT, 0, 0
        }
    }
};


static const struct LOG_Table s_ProfileMappingTable =
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
            LANG_GATEWAY, 35, 0
        },
        {
            LANG_DRIVER_CODE, 46, 0
        },
        {
            LANG_DRIVER_CODE_DESC, 0, 0
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


#ifdef LIB_EMBEDULAR_HAS_VIDEO
static const struct LOG_Table s_ScreenMgrTable =
{
    LANG_SCREENS, 5,
    {
        {
            LANG_ROLE, 14, 0
        },
        {
            LANG_DRIVER, 44, 0
        },
        {
            LANG_WIDTH, 53, 0
        },
        {
            LANG_HEIGHT, 62, 0
        },
        {
            LANG_FRAMEBUFFERS, 0, 0
        }
    }
};
#endif


bool BOARD_INIT_InputCountdown (const enum INPUT_PROFILE_Group ProfileType,
                                const IO_Code ProfileCode,
                                const TIMER_Ticks Countdown,
                                const char *const Msg)
{
    if (!MIO_IsMapped (MIO_Dir_Input, ProfileType, IO_Type_Bit, ProfileCode))
    {
        return true;
    }

    LOG (NOBJ, Msg, MIO_MappedName(MIO_Dir_Input, ProfileType, IO_Type_Bit,
                                   ProfileCode));

    const TIMER_Ticks   Timeout = TICKS_Now () + Countdown;
    const uint32_t      LPM     = LOG_ProgressMax ();

    uint32_t lastOutColumn = LOG_ProgressBegin (LOG_ProgressType_Timeout);
    uint32_t lastBitBuffer;

    do
    {
        TICKS_Delay (100);
        BOARD_Sync ();

        const TIMER_Ticks Now = TICKS_Now ();

        if (Timeout <= Now)
        {
            LOG_ProgressUpdate (lastOutColumn, LPM);
        }
        else
        {
            const int32_t   Pro = Countdown - (Timeout - Now);
            const uint32_t  Col = (Pro * LPM) / Countdown;

            lastOutColumn = LOG_ProgressUpdate (lastOutColumn, Col);
        }

        lastBitBuffer = MIO_GetInputBuffer (ProfileType, IO_Type_Bit, 
                                            ProfileCode);
    }
    while (!lastBitBuffer && Timeout > TICKS_Now());

    LOG_ProgressEnd ();

    // Timeout reached
    if (Timeout <= TICKS_Now())
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
    MIO_Init        (&B->mio);
    COMM_Init       (&B->comm);
    STORAGE_Init    (&B->storage);
    SCREEN_Init     (&B->screen);
}


void OSWRAP__summary (void);


static void showOswrapSummary ()
{
    LOG_AutoContext (NOBJ, "oswrap summary");

    OSWRAP__summary ();
}


static void showIoProfilesSummary (struct BOARD *const B, 
                                   const enum MIO_Dir Dir, 
                                   const char *const Title)
{
    LOG_AutoContext (&B->mio, Title);

    LOG_TableBegin (&s_IOProfilesTable);

    uint32_t enabledProfilesCount = 0;

    const IO_PROFILE_Group ProfileGroupCount = MIO__profileGroupCount (Dir);

    for (IO_PROFILE_Group g = 0; g < ProfileGroupCount; ++g)
    {
        const bool EnabledProfile = MIO_HasProfile (Dir, g);

        const struct IO_PROFILE *const P = &B->mio.profiles[Dir][g];

        LOG_TableEntry (&s_IOProfilesTable, MIO__profileGroupName(Dir, g),
                        (EnabledProfile)? LOG_ITEM_OK_STR : LOG_ITEM_FAILED_STR,
                        P->count[IO_Type_Bit], P->count[IO_Type_Range]);

        if (EnabledProfile)
        {
            ++ enabledProfilesCount;
        }
    }

    LOG_TableEnd (&s_IOProfilesTable);

    if (!enabledProfilesCount)
    {
        LOG_Warn (&B->mio, LANG_NO_PROFILES_SET);
    }
}


static void initIoProfiles (struct BOARD *const B)
{
    LOG_AutoContext (NOBJ, LANG_IO_PROFILES);

    BOARD_INIT_Component (B, BOARD_Stage_InitIOProfiles, NOBJ);

    showIoProfilesSummary (B, MIO_Dir_Input, LANG_INPUT_SUMMARY);
    showIoProfilesSummary (B, MIO_Dir_Output, LANG_OUTPUT_SUMMARY);
}


static void initRandomDriver (struct BOARD *const B)
{
    LOG_AutoContext (NOBJ, LANG_RNG_DRIVER);

    BOARD_INIT_Component (B, BOARD_Stage_InitRandomDriver, B->random);
}


static void initIoLevel1Drivers (struct BOARD *const B)
{
    LOG_AutoContext (NOBJ, LANG_IO_LEVEL_1_DRIVERS);

    MIO_CurrentMapAction (MIO_Dir_Input, IO_MapAction_Overwrite);
    MIO_CurrentMapAction (MIO_Dir_Output, IO_MapAction_Overwrite);

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
    COMM_SetDevice (COMM_Device_Log, B->debugStream);

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

    MIO_CurrentMapAction (MIO_Dir_Input, IO_MapAction_Overwrite);
    MIO_CurrentMapAction (MIO_Dir_Output, IO_MapAction_Overwrite);

    BOARD_INIT_Component (B, BOARD_Stage_InitIOLevel2Drivers, NOBJ);
}


static void initScreenDrivers (struct BOARD *const B)
{
    LOG_AutoContext (NOBJ, LANG_SCREEN_DRIVERS);

    BOARD_INIT_Component (B, BOARD_Stage_InitScreenDrivers, NOBJ);

    if (!SCREEN_RegisteredDevices ())
    {
        #if (LIB_EMBEDULAR_CONFIG_NEED_VIDEO == 1)
            BOARD_AssertInitialized (LANG_DEVICE_INIT_FAILED);
        #else
            LOG_Warn (B, LANG_DEVICE_INIT_FAILED);
        #endif
    }
}


static void initIoLevel3Drivers (struct BOARD *const B)
{
    LOG_AutoContext (NOBJ, LANG_IO_LEVEL_3_DRIVERS);

    MIO_CurrentMapAction (MIO_Dir_Input, IO_MapAction_Overwrite);
    MIO_CurrentMapAction (MIO_Dir_Output, IO_MapAction_Overwrite);

    BOARD_INIT_Component (B, BOARD_Stage_InitIOLevel3Drivers, NOBJ);
}


static void showIoManagerTypeMapping (const enum MIO_Dir Dir,
                                      const struct IO_PROFILE *const P,
                                      const IO_PROFILE_Group ProfileGroup,
                                      const enum IO_Type IoType)
{
    for (uint32_t pcode = 0; pcode < P->count[IoType]; ++pcode)
    {
        struct IO_PROFILE_Map *const Map = &P->map[IoType][pcode];

        if (Map->driverCode != IO_INVALID_CODE)
        {
            LOG_TableEntry (&s_ProfileMappingTable,
                            IO_TypeName(IoType), pcode,
                            Map->gatewayId, Map->driverCode,
                            MIO_MappedName(Dir, ProfileGroup, IoType, pcode));
        }
        #ifdef BOARD_INIT_SHOW_NOT_SET
        else 
        {
            LOG_TableEntry (&s_ProfileMappingTable,
                            IO_TypeName(IoType), pcode,
                            "", "",
                            "");
        }
        #endif
    }
}


static void showIoManagerSummary (struct BOARD *const B,
                                  const enum MIO_Dir Dir,
                                  const char *const Title)
{
    LOG_AutoContext (&B->mio, Title);

    // List devices
    LOG_TableBegin (&s_IOGatewayTable);
    for (uint32_t gwId = 0; gwId < MIO__maxGateways(Dir); ++gwId)
    {
        struct IO_Gateway *const G = &B->mio.gateways[Dir][gwId];

        if (G->driver)
        {
            LOG_TableEntry (&s_IOGatewayTable,
                            gwId, 
                            IO_Description(G->driver),
                            G->driverPort);
        }
        #ifdef BOARD_INIT_SHOW_NOT_SET
        else 
        {
            LOG_TableEntry (&s_IODeviceTable,
                            gwId, 
                            "",
                            "");
        }
        #endif
    }
    LOG_TableEnd (&s_IOGatewayTable);

    LOG_Newline ();

    for (IO_PROFILE_Group pg = 0; pg < MIO__profileGroupCount(Dir); ++pg)
    {
        // Profile is available
        if (MIO_HasProfile (Dir, pg))
        {
            LOG_AutoContext (NOBJ, "`0:`1 profile",
                             MIO_DirName (Dir),
                             MIO__profileGroupName(Dir, pg));


            const struct IO_PROFILE *const P = &B->mio.profiles[Dir][pg];

            // List bit and range mappings
            LOG_TableBegin (&s_ProfileMappingTable);

            showIoManagerTypeMapping (Dir, P, pg, IO_Type_Bit);
            showIoManagerTypeMapping (Dir, P, pg, IO_Type_Range);

            LOG_TableEnd (&s_ProfileMappingTable);
        }
    }
}


static void showCommManagerSummary (struct BOARD *const B)
{
    LOG_AutoContext (NOBJ, LANG_COMM_MGR_SUMMARY);

    LOG_TableBegin (&s_CommMgrTable);

    for (enum COMM_Device dev = 0; dev < COMM_Device__COUNT; ++dev)
    {
        struct STREAM *const S = B->comm.device[dev];

        if (S)
        {
            LOG_TableEntry (&s_CommMgrTable,
                            "stream",
                            COMM_DeviceRoleDescription(dev),
                            STREAM_Description(S));
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

    if (*Ticks < TICKS_Now())
    {
        *Ticks = TICKS_Now() + 100;
        return true;
    }

    return false;
}


static void checkCache (const uint32_t ReadWriteRetries)
{
    bool skipElementDataCheck = false;

    if (!BOARD_INIT_InputCountdown (CHECK_EDATA_OVERRIDE_PROFILE_TYPE,
                                    CHECK_EDATA_OVERRIDE_PROFILE_CODE,
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

    if (!STORAGE_RegisteredVolumes ())
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


static void showScreenManagerSummary (struct BOARD *const B)
{
    LOG_AutoContext (NOBJ, LANG_SCREEN_MGR_SUMMARY);

#ifdef LIB_EMBEDULAR_HAS_VIDEO
    if (!SCREEN_RegisteredDevices())
    {
        LOG_Warn (NOBJ, LANG_NO_SCREEN_DEVICES_SET);
    }
    else
    {
        // List devices
        LOG_TableBegin (&s_ScreenMgrTable);

        for (enum SCREEN_Role r = 0; r < SCREEN_Role__COUNT; ++r)
        {
            if (SCREEN_IsAvailable (r))
            {
                struct VIDEO *const V = B->screen.context[r].driver;

                LOG_TableEntry (&s_ScreenMgrTable,
                                SCREEN_RoleName(r),
                                VIDEO_Description(V),
                                V->iface->Width,
                                V->iface->Height,
                                (uint32_t)((V->frontbuffer != V->backbuffer)?
                                    2 : 1));
            }
        }

        LOG_TableEnd (&s_ScreenMgrTable);
    }
#else
    (void) B;

    LOG_Warn (NOBJ, LANG_NO_VIDEO_SUPPORT);
#endif
}


static void showManagersSummary (struct BOARD *const B)
{
    showIoManagerSummary        (B, MIO_Dir_Input, LANG_INPUT_SUMMARY);
    showIoManagerSummary        (B, MIO_Dir_Output, LANG_OUTPUT_SUMMARY);
    showCommManagerSummary      (B);
    showStorageManagerSummary   (B);
    showScreenManagerSummary    (B);
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


// Provided by build-system selected driver
void TICKS__boot (void);


static void boardInitSequence (struct BOARD *const B)
{
    // -------------------------------------------------------------------------
    // Assert checks available, but still NO assert output or log messages
    // -------------------------------------------------------------------------
    BOARD_AssertParams      (B);
    // -------------------------------------------------------------------------
    // No assert messages output. If board initialization fails, the board
    // interface implementation itself should inform the user what went wrong
    // by using an OS function, LED or other ad-hoc method.
    // -------------------------------------------------------------------------
    // Initialization of base hardware required to boot the build-system
    // selected ticks driver. For example, in this stage the board initializes
    // its main peripherals or multimedia library.
    BOARD_INIT_Component    (B, BOARD_Stage_InitPreTicksHardware, NOBJ);
    // Ticks driver boot
    TICKS__boot ();
    // Hardware and/or BSP initialization that depends on an initialized ticks
    // driver.
    BOARD_INIT_Component    (B, BOARD_Stage_InitPostTicksHardware, NOBJ);
    // -------------------------------------------------------------------------
    // System debug stream required for assert messages.
    // -------------------------------------------------------------------------
    BOARD_INIT_Component    (B, BOARD_Stage_InitDebugStreamDriver,
                             B->debugStream);
    BOARD_AssertInitialized (B->debugStream);
    // Default debug stream output timeout (data input to the stream) for
    // greeting messages prior to log manager init.
    STREAM_Timeout (B->debugStream, LOG_DEBUG_STREAM_TIMEOUT);
    // -------------------------------------------------------------------------
    // Assert messages working. Still no log messages.
    // -------------------------------------------------------------------------
    streamOutArgs           (B, LOG_BASE_COLOR "`L", NULL, 0);
    frameworkGreetings      (B);
    streamOutArgs           (B, LOG_BASE_COLOR "`L", NULL, 0);
    boardGreetings          (B);
    streamOutArgs           (B, LOG_BASE_COLOR "`L", NULL, 0);    
    mtosGreetings           (B);
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
    showOswrapSummary       ();
    initIoProfiles          (B);
    initRandomDriver        (B);
    initIoLevel1Drivers     (B);
    initCommDrivers         (B);
    initStorageDrivers      (B);
    initIoLevel2Drivers     (B);
    initScreenDrivers       (B);
    initSoundDriver         (B);
    initIoLevel3Drivers     (B);
    showManagersSummary     (B);
    // -------------------------------------------------------------------------
    boardInitSequenceEnd    ();
    // -------------------------------------------------------------------------
    // Board ready.
    // -------------------------------------------------------------------------
    BOARD_INIT_Component (B, BOARD_Stage_Ready, NOBJ);

#if (LIB_EMBEDULAR_CONFIG_SPLASH_SCREENS == 1)
    if (SCREEN_RegisteredDevices ())
    {
        LOG_AutoContext (NOBJ, LANG_SPLASH_SCREENS);

        CC_ExpPaste(SPLASH_THEME_,LIB_EMBEDULAR_CONFIG_SPLASH_THEME_L1)();
        CC_ExpPaste(SPLASH_THEME_,LIB_EMBEDULAR_CONFIG_SPLASH_THEME_L2)();
        CC_ExpPaste(SPLASH_THEME_,LIB_EMBEDULAR_CONFIG_SPLASH_THEME_L3)();
    }
#endif

    OSWRAP_EnableAutoSync (LIB_EMBEDULAR_CONFIG_OS_AUTOSYNC_PERIOD);
}


static void boardShutdownSequence (struct BOARD *const B)
{
    LOG_Warn (B, LANG_SHUTTING_DOWN);

    // Managers shutdown (will shut down their registered drivers)
    SCREEN_Shutdown ();

    // BOARD_Stage_InitHardware counterpart
    B->iface->StageChange (B, BOARD_Stage_ShutdownHardware);
}


void OSWRAP__greetings              (struct STREAM *const S);
int  OSWRAP__createRunTaskAndStart  (struct BOARD *const B,
                                     const OSWRAP_TaskFunc RunTask);
void OSWRAP__createMainTaskAndSync  (struct BOARD *const B,
                                     const OSWRAP_TaskFunc MainTask);
void OSWRAP__closeRunTaskAndEnd     (void);


static void runTask (void *param)
{
    // -------------------------------------------------------------------------
    // Assert checks available, but still NO assert output or log messages.
    // -------------------------------------------------------------------------
    BOARD_AssertParams (param);

    struct BOARD *const B = (struct BOARD *) param;

    boardInitSequence (B);
    // -------------------------------------------------------------------------
    // Board ready.
    // -------------------------------------------------------------------------
    BOARD_AssertState (BOARD_CurrentStage() == BOARD_Stage_Ready);

    LOG_ContextBegin (NOBJ, LANG_APPLICATION);
    {
        LOG_Items (1, LANG_NAME, CC_AppNameStr);
        LOG_Items (1, LANG_VERSION, CC_VcsAppVersionStr);

        LOG_ContextBegin (NOBJ, LANG_ENTERING_APP_MAIN);
        {
            OSWRAP__createMainTaskAndSync (B, EMBEDULAR_Main);

            LOG (NOBJ, LANG_RETURNED_FROM_APP_MAIN);
            LOG_Items (1, LANG_RETURN_VALUE, (int64_t)BOARD_ReturnValue());
        }
        LOG_ContextEnd ();
    }
    LOG_ContextEnd ();

    boardShutdownSequence (B);

    // The below OSWRAP function will never be executed if an embedded system
    // actually shut down the hardware. On targets running over an OS,
    // boardShutdownSequence() will close the multimedia libraries, then the
    // above function will close the application and then return a value to the
    // OS through main().
    OSWRAP__closeRunTaskAndEnd ();
}


int BOARD__run (struct BOARD *const B)
{
    // -------------------------------------------------------------------------
    // Assert checks available, but still NO assert output or log messages.
    // -------------------------------------------------------------------------
    const int RetVal = OSWRAP__createRunTaskAndStart (B, runTask);

    return RetVal;
}
