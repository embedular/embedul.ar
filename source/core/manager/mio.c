/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [MANAGER] input/output devices manager.

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

#include "embedul.ar/source/core/manager/mio.h"
#include "embedul.ar/source/core/device/board.h"


static const uint16_t s_MaxGateways[MIO_Dir__COUNT] = 
{
    [MIO_Dir_Input]     = MIO_INPUT_MAX_GATEWAYS,
    [MIO_Dir_Output]    = MIO_OUTPUT_MAX_GATEWAYS
};


static const IO_PROFILE_Group s_IoProfileGroupCount[MIO_Dir__COUNT] =
{
    [MIO_Dir_Input]     = INPUT_PROFILE_Group__COUNT,
    [MIO_Dir_Output]    = OUTPUT_PROFILE_Group__COUNT
};


const char *const s_DirName[MIO_Dir__COUNT] =
{
    [MIO_Dir_Input]     = "Input",
    [MIO_Dir_Output]    = "Output"
};


static struct MIO * s_m = NULL;


static void mapByGatewayId (const enum MIO_Dir Dir,
                            struct IO_PROFILE_Map *const Map, 
                            const IO_GatewayId GatewayId,
                            const IO_Code DriverCode)
{
    BOARD_AssertState (s_m->gateways[Dir][GatewayId].driver);

    Map->gatewayId  = GatewayId;
    Map->driverCode = DriverCode;
}


static void mapCurrent (const enum MIO_Dir Dir,
                        struct IO_PROFILE_Map *const Map, 
                        const IO_Code DriverCode)
{
    BOARD_AssertState (s_m->nextGatewayId[Dir]);

    const IO_GatewayId CurrentGatewayId = s_m->nextGatewayId[Dir] - 1;

    mapByGatewayId (Dir, Map, CurrentGatewayId, DriverCode);
}


inline static struct IO_PROFILE *
getProfile (const enum MIO_Dir Dir,
            const IO_PROFILE_Group ProfileGroup)
{
    struct IO_PROFILE *const P = &s_m->profiles[Dir][ProfileGroup];
    return P;
}


// BitMap or RangeMap must exist in ProfileType
static struct IO_PROFILE_Map *
getMapping (const enum MIO_Dir Dir,
            const IO_PROFILE_Group ProfileGroup,
            const enum IO_Type IoType, const IO_Code ProfileCode)
{
    struct IO_PROFILE *const P = getProfile (Dir, ProfileGroup);

    if (ProfileCode >= P->count[IoType] || !P->map[IoType])
    {
        LOG_Warn (s_m, LANG_INVALID_IO_PROFILE);
        LOG_Items (5,   
                    "direction",            Dir,
                    "profile group id",     ProfileGroup,
                    "profile code",         ProfileCode,
                    "map type",             IoType,
                    "map address",          (void *)P->map[IoType],
                    "map element count",    P->count[IoType]);

        BOARD_AssertState (false);
    }
    return &P->map[IoType][ProfileCode];
}


static bool isMapped (const struct IO_PROFILE_Map *const Map)
{
    const IO_Code DriverCode = Map->driverCode;
    return (DriverCode != IO_INVALID_CODE)? true : false;
}


static struct IO_Gateway * getGateway (const enum MIO_Dir Dir,
                                       const struct IO_PROFILE_Map *const Map)
{
    BOARD_AssertState (Map->gatewayId < s_MaxGateways[Dir]);
    struct IO_Gateway *const G = &s_m->gateways[Dir][Map->gatewayId];
    return G;
}


static bool driversMatch (const enum MIO_Dir Dir,
                          const struct IO_PROFILE_Map *const Map,
                          const struct IO *const Driver)
{
    if (Map->driverCode == IO_INVALID_CODE)
    {
        return false;
    }

    struct IO_Gateway *const G = getGateway (Dir, Map);

    return (G->driver == Driver)? true : false;
}


static IO_Value getInput (const enum IO_Type IoType,
                          const struct IO_PROFILE_Map *const Map,
                          const enum MIO_InUpdateValue When)
{
    if (!isMapped (Map))
    {
        return 0;
    }

    struct IO_Gateway *const G = getGateway (MIO_Dir_Input, Map);

    const IO_Value Status =
        IO_GetInput (G->driver, IoType, Map->driverCode,
                     G->driverPort, (enum IO_UpdateValue)When);

    return Status;
}


static void setOutput (const enum IO_Type IoType,
                       const struct IO_PROFILE_Map *const Map,
                       const IO_Value Value,
                       enum MIO_OutUpdateValue When)
{
    if (!isMapped (Map))
    {
        return;
    }

    struct IO_Gateway *const G = getGateway (MIO_Dir_Output, Map);

    IO_SetOutput (G->driver, IoType, Map->driverCode,
                  G->driverPort, Value, (enum IO_UpdateValue)When);
}


static void updateInput (void)
{
    for (IO_GatewayId gId = 0; gId < MIO_INPUT_MAX_GATEWAYS; ++gId)
    {
        struct IO * inDriver = s_m->inGateway[gId].driver;

        if (inDriver)
        {
            IO_Update (inDriver);
        }
    }

#if (LIB_EMBEDULAR_CONFIG_INPUT_ACTION == 1)
    for (enum INPUT_PROFILE_Group t = 0; t < INPUT_PROFILE_Group__COUNT; ++t)
    {
        if (s_m->inBitActions[t])
        {
            struct IO_PROFILE *const P = getProfile (MIO_Dir_Input, t);

            for (uint32_t pcode = 0; pcode < P->count[IO_Type_Bit]; ++pcode)
            {
                const bool Status =
                    MIO_GetInputBuffer (t, IO_Type_Bit, pcode)? true : false;

                INPUT_ACTION_Update (&s_m->inBitActions[t][pcode], Status);
            }
        }
    }
#endif
}


static void updateOutput (void)
{
    for (uint32_t gId = 0; gId < MIO_OUTPUT_MAX_GATEWAYS; ++gId)
    {
        struct IO * outDriver = s_m->outGateway[gId].driver;

        if (outDriver)
        {
            IO_Update (outDriver);
        }
    }
}


void MIO_Init (struct MIO *const M)
{
    BOARD_AssertState   (!s_m);
    BOARD_AssertParams  (M);

    OBJECT_Clear (M);

    {
        LOG_AutoContext (M, LANG_INIT);

        s_m = M;

        s_m->profiles[MIO_Dir_Input]    = s_m->inProfiles;
        s_m->profiles[MIO_Dir_Output]   = s_m->outProfiles;

        s_m->gateways[MIO_Dir_Input]    = s_m->inGateway;
        s_m->gateways[MIO_Dir_Output]   = s_m->outGateway;

        s_m->currentMapAction[MIO_Dir_Input]    = IO_MapAction_NoRemap;
        s_m->currentMapAction[MIO_Dir_Output]   = IO_MapAction_NoRemap;

        LOG_Items (2,
                LANG_MAX_INPUT_GATEWAYS,    (uint32_t)MIO_INPUT_MAX_GATEWAYS,
                LANG_MAX_OUTPUT_GATEWAYS,   (uint32_t)MIO_OUTPUT_MAX_GATEWAYS);
    }
}


const char * MIO_DirName (const enum MIO_Dir Dir)
{
    BOARD_AssertParams (Dir < MIO_Dir__COUNT);
    return s_DirName[Dir];
}


void MIO_RegisterGateway (const enum MIO_Dir Dir, struct IO *const Driver,
                          const IO_Port DriverPort)
{
    BOARD_AssertParams  (Dir < MIO_Dir__COUNT && Driver &&
                         s_m->nextGatewayId[Dir] < s_MaxGateways[Dir]);
    BOARD_AssertState   (BOARD_CurrentStage() < BOARD_Stage_Ready);

    s_m->gateways[Dir][s_m->nextGatewayId[Dir]].driver      = Driver;
    s_m->gateways[Dir][s_m->nextGatewayId[Dir]].driverPort  = DriverPort;

    ++ s_m->nextGatewayId[Dir];
}


void MIO_CurrentMapAction (const enum MIO_Dir Dir,
                           const enum IO_MapAction MapAction)
{
    BOARD_AssertParams (Dir < MIO_Dir__COUNT &&
                        MapAction < IO_MapAction__COUNT); 

    s_m->currentMapAction[Dir] = MapAction;
}


bool MIO_HasProfile (const enum MIO_Dir Dir,
                     const IO_PROFILE_Group ProfileGroup)
{
    BOARD_AssertParams (Dir < MIO_Dir__COUNT &&
                        ProfileGroup < s_IoProfileGroupCount[Dir]);

    struct IO_PROFILE *const P = getProfile (Dir, ProfileGroup);
    return (P->count[IO_Type_Bit] || P->count[IO_Type_Range])?
            true : false;
}


uint32_t MIO_ProfileCodes (const enum MIO_Dir Dir,
                           const IO_PROFILE_Group ProfileGroup,
                           const enum IO_Type IoType)
{
    BOARD_AssertParams (Dir < MIO_Dir__COUNT &&
                        ProfileGroup < s_IoProfileGroupCount[Dir] &&
                        IoType < IO_Type__COUNT);

    return getProfile(Dir, ProfileGroup)->count[IoType];
}


bool MIO_IsMapped (const enum MIO_Dir Dir,
                   const IO_PROFILE_Group ProfileGroup,
                   const enum IO_Type IoType, const IO_Code ProfileCode)
{
    BOARD_AssertParams (Dir < MIO_Dir__COUNT &&
                        ProfileGroup < s_IoProfileGroupCount[Dir] &&
                        IoType < IO_Type__COUNT);

    return isMapped (getMapping(Dir, ProfileGroup, IoType, ProfileCode));
}


void MIO_Map (const enum MIO_Dir Dir,
              const IO_PROFILE_Group ProfileGroup,
              const enum IO_Type IoType, const IO_Code ProfileCode,
              const IO_Code DriverCode)
{
    BOARD_AssertParams (Dir < MIO_Dir__COUNT &&
                        ProfileGroup < s_IoProfileGroupCount[Dir] &&
                        IoType < IO_Type__COUNT);

    struct IO_PROFILE_Map *const Map =
        getMapping (Dir, ProfileGroup, IoType, ProfileCode);

    if (!isMapped(Map) || s_m->currentMapAction[Dir] == IO_MapAction_Overwrite)
    {
        mapCurrent (Dir, Map, DriverCode);
    }
}


bool MIO_AutoMapInputBit (const enum INPUT_PROFILE_Group InProfileGroup,
                          const IO_Code InProfileCode)
{
    BOARD_AssertParams (InProfileGroup < INPUT_PROFILE_Group__COUNT);

    struct IO_PROFILE_Map *const Map =
        getMapping (MIO_Dir_Input, InProfileGroup, IO_Type_Bit, InProfileCode);

    // Check registered drivers for one that has any input bit in 1 
    for (IO_GatewayId gId = 0; gId < MIO_INPUT_MAX_GATEWAYS; ++gId)
    {
        struct IO_Gateway *const G = &s_m->inGateway[gId];
        if (G->driver)
        {
            const IO_Code DriverCode =
                IO_GetAnyInput (G->driver, IO_Type_Bit, G->driverPort);

            if (DriverCode != IO_INVALID_CODE)
            {
                mapByGatewayId (MIO_Dir_Input, Map, gId, DriverCode);
                return true;
            }
        }
    }

    return false;
}


IO_Value MIO_GetInput (const enum INPUT_PROFILE_Group InProfileGroup,
                       const enum IO_Type IoType, const IO_Code InProfileCode,
                       const enum MIO_InUpdateValue When)
{
    BOARD_AssertParams (InProfileGroup < INPUT_PROFILE_Group__COUNT &&
                        IoType < IO_Type__COUNT);

    struct IO_PROFILE *const P = getProfile (MIO_Dir_Input, InProfileGroup);

    if (!P->map[IoType])
    {
        return 0;
    }

    BOARD_AssertParams (InProfileCode < P->count[IoType]);

    return getInput (IoType, &P->map[IoType][InProfileCode], When);
}


IO_Value MIO_GetInputNow (const enum INPUT_PROFILE_Group InProfileGroup,
                          const enum IO_Type IoType,
                          const IO_Code InProfileCode)
{
    return MIO_GetInput (InProfileGroup, IoType, InProfileCode,
                         MIO_InUpdateValue_Now);
}


IO_Value MIO_GetInputBuffer (const enum INPUT_PROFILE_Group InProfileType,
                             const enum IO_Type IoType,
                             const IO_Code InProfileCode)
{
    return MIO_GetInput (InProfileType, IoType, InProfileCode,
                         MIO_InUpdateValue_Buffer);
}


#if (LIB_EMBEDULAR_CONFIG_INPUT_ACTION == 1)
enum INPUT_ACTION_Type
MIO_GetInputBitAction (const enum INPUT_PROFILE_Group InProfileGroup,
                       const IO_Code InProfileCode)
{
    BOARD_AssertParams (InProfileGroup < INPUT_PROFILE_Group__COUNT);

    // Compiled with input action support, but any profile can lack input
    // action.
    if (!s_m->inBitActions[InProfileGroup])
    {
        return INPUT_ACTION_Type_None;
    }

    struct IO_PROFILE *const P = getProfile (MIO_Dir_Input, InProfileGroup);

    BOARD_AssertParams (InProfileCode < P->count[IO_Type_Bit]);

    return INPUT_ACTION_Last (&s_m->inBitActions[InProfileGroup][InProfileCode]);
}
#endif


bool MIO_GetAnyInputBit (const enum INPUT_PROFILE_SelectFlag InProfiles)
{
    BOARD_AssertParams (InProfiles <= INPUT_PROFILE_SelectFlag__ALL);

    if (!InProfiles)
    {
        return false;
    }

    for (uint32_t i = 0; i < InProfiles; ++i)
    {
        const enum INPUT_PROFILE_SelectFlag SelectFlag = (1 << i);

        if (InProfiles & SelectFlag)
        {
            enum INPUT_PROFILE_Group ProfileType = (enum INPUT_PROFILE_Group) i;
            struct IO_PROFILE *const P = getProfile (MIO_Dir_Input,
                                                     ProfileType);
            const uint32_t Count = P->count[IO_Type_Bit];

            if (Count)
            {
                for (uint32_t pcode = 0; pcode < Count; ++pcode)
                {
                    if (getInput (IO_Type_Bit, &P->map[IO_Type_Bit][pcode],
                               MIO_InUpdateValue_Buffer))
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}


void MIO_SetOutput (const enum OUTPUT_PROFILE_Group OutProfileGroup,
                    const enum IO_Type IoType, const IO_Code OutProfileCode,
                    const enum MIO_OutUpdateValue When,
                    const IO_Value Value)
{
    BOARD_AssertParams (OutProfileGroup < OUTPUT_PROFILE_Group__COUNT &&
                        IoType < IO_Type__COUNT);

    struct IO_PROFILE *const P = getProfile (MIO_Dir_Output, OutProfileGroup);

    if (!P->map[IoType])
    {
        return;
    }

    BOARD_AssertParams (OutProfileCode < P->count[IoType]);

    setOutput (IoType, &P->map[IoType][OutProfileCode], Value, When);
}


void MIO_SetOutputNow (const enum OUTPUT_PROFILE_Group OutProfileGroup,
                       const enum IO_Type IoType, const IO_Code OutProfileCode,
                       const IO_Value Value)
{
    MIO_SetOutput (OutProfileGroup, IoType, OutProfileCode,
                   MIO_OutUpdateValue_Now, Value);
}


void MIO_SetOutputDeferred (const enum OUTPUT_PROFILE_Group OutProfileGroup,
                            const enum IO_Type IoType,
                            const IO_Code OutProfileCode,
                            const IO_Value Value)
{
    MIO_SetOutput (OutProfileGroup, IoType, OutProfileCode,
                   MIO_OutUpdateValue_Defer, Value);
}


const char * MIO_MappedName (const enum MIO_Dir Dir,
                             const IO_PROFILE_Group ProfileGroup,
                             const enum IO_Type IoType,
                             const IO_Code ProfileCode)
{
    BOARD_AssertParams (Dir < MIO_Dir__COUNT &&
                        ProfileGroup < s_IoProfileGroupCount[Dir] &&
                        IoType < IO_Type__COUNT);

    struct IO_PROFILE_Map *const Map =
        getMapping (Dir, ProfileGroup, IoType, ProfileCode);

    BOARD_AssertParams (Map->driverCode != IO_INVALID_CODE);

    struct IO_Gateway *const G = getGateway (Dir, Map);

    if (Dir == MIO_Dir_Output)
    {
        return IO_OutputName (G->driver, IoType, Map->driverCode);
    }

    // MIO_Dir_Input
    return IO_InputName (G->driver, IoType, Map->driverCode);
}


void MIO_Update (void)
{
    updateInput ();
    updateOutput ();
}


IO_PROFILE_Group MIO__profileGroupCount (const enum MIO_Dir Dir)
{
    BOARD_AssertParams (Dir < MIO_Dir__COUNT);
    return (Dir == MIO_Dir_Input)? INPUT_PROFILE_Group__COUNT :
                                   OUTPUT_PROFILE_Group__COUNT;
}


const char * MIO__profileGroupName (const enum MIO_Dir Dir,
                                   const IO_PROFILE_Group ProfileGroup)
{
    BOARD_AssertParams (Dir < MIO_Dir__COUNT &&
                        ProfileGroup < s_IoProfileGroupCount[Dir]);

    if (Dir == MIO_Dir_Input)
    {
        return INPUT_PROFILE_GetGroupName (ProfileGroup);
    }

    return OUTPUT_PROFILE_GetGroupName (ProfileGroup);
}


uint32_t MIO__maxGateways (const enum MIO_Dir Dir)
{
    BOARD_AssertParams (Dir < MIO_Dir__COUNT);
    return s_MaxGateways[Dir];
}


IO_Code 
MIO__isMappedByDriver (const enum MIO_Dir Dir,
                       const IO_PROFILE_Group ProfileGroup,
                       const enum IO_Type IoType,
                       const IO_Code ProfileCode,
                       const struct IO *const Driver)
{
    struct IO_PROFILE_Map *const Map =
        getMapping (Dir, ProfileGroup, IoType, ProfileCode);

    return (driversMatch(Dir, Map, Driver))? Map->driverCode : IO_INVALID_CODE;
}


struct IO_ConstGateway
MIO__getMappedGateway (const enum MIO_Dir Dir,
                       const IO_PROFILE_Group ProfileGroup,
                       const enum IO_Type IoType,
                       const IO_Code ProfileCode)
{
    struct IO_PROFILE_Map *const Map =
        getMapping (Dir, ProfileGroup, IoType, ProfileCode);

    const struct IO_Gateway *const Gateway = getGateway (Dir, Map);

    return (struct IO_ConstGateway)
    {
        .Driver     = Gateway->driver,
        .DriverPort = Gateway->driverPort 
    };
}
