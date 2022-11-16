/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [MANAGER] input devices manager.

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

#include "embedul.ar/source/core/manager/input.h"
#include "embedul.ar/source/core/device/board.h"


static struct INPUT * s_i = NULL;


static void inMapByGatewayId (struct IO_PROFILE_Map *const Map,
                              const IO_GatewayId GatewayId,
                              const IO_Code DriverCode)
{
    BOARD_AssertParams (GatewayId < INPUT_MAX_GATEWAYS);
    BOARD_AssertState  (s_i->gateways[GatewayId].driver);

    Map->gatewayId  = GatewayId;
    Map->driverCode = DriverCode;
}


static void inputMapCurrent (struct IO_PROFILE_Map *const Map,
                             const IO_Code DriverCode)
{
    BOARD_AssertState (s_i->nextGatewayId);

    const IO_GatewayId CurrentGatewayId = s_i->nextGatewayId - 1;

    inMapByGatewayId (Map, CurrentGatewayId, DriverCode);
}


inline static struct INPUT_PROFILE * 
getProfile (const enum INPUT_PROFILE_Type ProfileType)
{
    struct INPUT_PROFILE *const P = &s_i->profiles[ProfileType];
    return P;
}


// BitMap or RangeMap must exist in ProfileType
static struct IO_PROFILE_Map *
getMapping (const enum INPUT_PROFILE_Type ProfileType,
            const enum IO_Type IoType, const IO_Code ProfileCode)
{
    struct INPUT_PROFILE *const P = getProfile (ProfileType);
    if (ProfileCode >= P->count[IoType] || !P->map[IoType])
    {
        LOG_Warn (s_i, LANG_INVALID_IO_PROFILE);
        LOG_Items (4,
                    "profile code",     ProfileCode,
                    "map type",         IoType,
                    "map address",      (void *)P->map[IoType],
                    "map count",        P->count[IoType]);

        BOARD_AssertState (false);
    }
    return &P->map[IoType][ProfileCode];
}


static bool isMapped (const struct IO_PROFILE_Map *const Map)
{
    const IO_Code DriverCode = Map->driverCode;
    return (DriverCode != IO_INVALID_CODE)? true : false;
}


static struct IO_Gateway * getGateway (const struct IO_PROFILE_Map *const Map)
{
    BOARD_AssertState (Map->gatewayId < OUTPUT_MAX_GATEWAYS);
    struct IO_Gateway *const G = &s_i->gateways[Map->gatewayId];
    return G;
}


static bool driversMatch (const struct IO_PROFILE_Map *const Map,
                          const struct IO *const Driver)
{
    if (Map->driverCode == IO_INVALID_CODE)
    {
        return false;
    }

    struct IO_Gateway *const G = getGateway (Map);

    return (G->driver == Driver)? true : false;
}


static IO_Value input (const enum IO_Type IoType,
                       const struct IO_PROFILE_Map *const Map,
                       const enum INPUT_UpdateValue When)
{
    if (!isMapped (Map))
    {
        return 0;
    }

    struct IO_Gateway *const G = getGateway (Map);

    const IO_Value Status =
        IO_GetInput (G->driver, IoType, Map->driverCode,
                     G->driverPort, (enum IO_UpdateValue)When);

    return Status;
}


void INPUT_Init (struct INPUT *const I)
{
    BOARD_AssertState  (!s_i);
    BOARD_AssertParams (I);

    OBJECT_Clear (I);

    {
        LOG_AutoContext (I, LANG_INIT);

        s_i = I;

        s_i->currentMapAction = IO_MapAction_NoRemap;

        LOG_Items (1, LANG_MAX_SOURCES, (uint32_t)INPUT_MAX_GATEWAYS);
    }
}


void INPUT_RegisterGateway (struct IO *const Driver, const IO_Port DriverPort)
{
    BOARD_AssertParams  (s_i->nextGatewayId < INPUT_MAX_GATEWAYS && Driver);
    BOARD_AssertState   (BOARD_CurrentStage() < BOARD_Stage_Ready);

    s_i->gateways[s_i->nextGatewayId].driver     = Driver;
    s_i->gateways[s_i->nextGatewayId].driverPort = DriverPort;

    ++ s_i->nextGatewayId;
}


void INPUT_CurrentMapAction (const enum IO_MapAction MapAction)
{
    s_i->currentMapAction = MapAction;
}


bool INPUT_HasProfile (const enum INPUT_PROFILE_Type ProfileType)
{
    BOARD_AssertParams (ProfileType < INPUT_PROFILE_Type__COUNT);

    struct INPUT_PROFILE *const P = getProfile (ProfileType);
    return (P->count[IO_Type_Bit] || P->count[IO_Type_Range])?
            true : false;
}


uint32_t INPUT_ProfileCodes (const enum INPUT_PROFILE_Type ProfileType,
                             const enum IO_Type IoType)
{
    BOARD_AssertParams (ProfileType < INPUT_PROFILE_Type__COUNT &&
                        IoType < IO_Type__COUNT);

    return getProfile(ProfileType)->count[IoType];
}


bool INPUT_IsMapped (const enum INPUT_PROFILE_Type ProfileType,
                     const enum IO_Type IoType, const IO_Code ProfileCode)
{
    BOARD_AssertParams (ProfileType < INPUT_PROFILE_Type__COUNT &&
                        IoType < IO_Type__COUNT);

    return isMapped (getMapping(ProfileType, IoType, ProfileCode));
}


void INPUT_Map (const enum INPUT_PROFILE_Type ProfileType,
                const enum IO_Type IoType, const IO_Code ProfileCode,
                const IO_Code DriverCode)
{
    BOARD_AssertParams (ProfileType < INPUT_PROFILE_Type__COUNT &&
                        IoType < IO_Type__COUNT);
    // Static mapping (albeith overwrittable) at initialization stage only
    BOARD_AssertState   (BOARD_CurrentStage() < BOARD_Stage_Ready);

    struct IO_PROFILE_Map *const Map =
        getMapping (ProfileType, IoType, ProfileCode);

    if (!isMapped(Map) || s_i->currentMapAction == IO_MapAction_Overwrite)
    {
        inputMapCurrent (Map, DriverCode);
    }
}


bool INPUT_MapBitByOnState (const enum INPUT_PROFILE_Type ProfileType,
                            const IO_Code ProfileCode)
{
    BOARD_AssertParams (ProfileType < INPUT_PROFILE_Type__COUNT);

    struct IO_PROFILE_Map *const Map =
        getMapping (ProfileType, IO_Type_Bit, ProfileCode);

    // Check registered drivers for one that has any input bit in 1 
    for (IO_GatewayId gId = 0; gId < INPUT_MAX_GATEWAYS; ++gId)
    {
        struct IO_Gateway *const G = &s_i->gateways[gId];
        if (G->driver)
        {
            const IO_Code DriverCode = IO_GetAnyInput (G->driver,
                                                IO_Type_Bit, G->driverPort);

            if (DriverCode != IO_INVALID_CODE)
            {
                inMapByGatewayId (Map, gId, DriverCode);
                return true;
            }
        }
    }

    return false;
}


IO_Value INPUT_Get (const enum INPUT_PROFILE_Type ProfileType,
                    const enum IO_Type IoType, const IO_Code ProfileCode,
                    const enum INPUT_UpdateValue When)
{
    BOARD_AssertParams (ProfileType < INPUT_PROFILE_Type__COUNT &&
                        IoType < IO_Type__COUNT);

    struct INPUT_PROFILE *const P = getProfile (ProfileType);

    if (!P->map[IoType])
    {
        return 0;
    }

    BOARD_AssertParams (ProfileCode < P->count[IoType]);

    return input (IoType, &P->map[IoType][ProfileCode], When);
}


IO_Value INPUT_GetNow (const enum INPUT_PROFILE_Type ProfileType,
                       const enum IO_Type IoType, const IO_Code ProfileCode)
{
    return INPUT_Get (ProfileType, IoType, ProfileCode,
                      INPUT_UpdateValue_Now);
}


IO_Value INPUT_GetBuffered (const enum INPUT_PROFILE_Type ProfileType,
                            const enum IO_Type IoType,
                            const IO_Code ProfileCode)
{
    return INPUT_Get (ProfileType, IoType, ProfileCode,
                      INPUT_UpdateValue_Buffer);
}


#if (LIB_EMBEDULAR_CONFIG_INPUT_ACTION == 1)
enum INPUT_ACTION_Type
INPUT_GetBitAction (const enum INPUT_PROFILE_Type ProfileType,
                    const IO_Code ProfileCode)
{
    BOARD_AssertParams (ProfileType < INPUT_PROFILE_Type__COUNT);

    struct INPUT_PROFILE *const P = getProfile (ProfileType);

    if (!P->bitAction)
    {
        return INPUT_ACTION_Type_None;
    }

    BOARD_AssertParams (ProfileCode < P->count[IO_Type_Bit]);

    return INPUT_ACTION_Last (&P->bitAction[ProfileCode]);
}
#endif


const char * INPUT_MappedName (const enum INPUT_PROFILE_Type ProfileType,
                               const enum IO_Type IoType,
                               const IO_Code ProfileCode)
{
    BOARD_AssertParams (ProfileType < INPUT_PROFILE_Type__COUNT &&
                        IoType < IO_Type__COUNT);

    struct IO_PROFILE_Map *const Map = 
        getMapping (ProfileType, IoType, ProfileCode);

    BOARD_AssertParams (Map->driverCode != IO_INVALID_CODE);

    struct IO_Gateway *const G = getGateway (Map);

    return IO_InputName (G->driver, IoType, Map->driverCode);
}


bool INPUT_AnyBit (const enum INPUT_PROFILE_SelectFlag Profiles)
{
    BOARD_AssertParams (Profiles <= INPUT_PROFILE_SelectFlag__ALL);

    if (!Profiles)
    {
        return false;
    }

    for (uint32_t i = 0; i < Profiles; ++i)
    {
        const enum INPUT_PROFILE_SelectFlag SelectFlag = (1 << i);

        if (Profiles & SelectFlag)
        {
            enum INPUT_PROFILE_Type ProfileType = (enum INPUT_PROFILE_Type) i;
            struct INPUT_PROFILE *const P = getProfile (ProfileType);
            const uint32_t Count = P->count[IO_Type_Bit];

            if (Count)
            {
                for (uint32_t pcode = 0; pcode < Count; ++pcode)
                {
                    if (input (IO_Type_Bit, &P->map[IO_Type_Bit][pcode],
                               INPUT_UpdateValue_Buffer))
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}


void INPUT_Update (void)
{
    for (IO_GatewayId gId = 0; gId < INPUT_MAX_GATEWAYS; ++gId)
    {
        struct IO * inDriver = s_i->gateways[gId].driver;

        if (inDriver)
        {
            IO_Update (inDriver);
        }
    }

#if (LIB_EMBEDULAR_CONFIG_INPUT_ACTION == 1)
    for (enum INPUT_PROFILE_Type t = 0; t < INPUT_PROFILE_Type__COUNT; ++t)
    {
        struct INPUT_PROFILE *const P = getProfile (t);
        if (P->bitAction)
        {
            for (uint32_t pcode = 0; pcode < P->count[IO_Type_Bit]; ++pcode)
            {
                const bool Status = INPUT_GetBuffered(t, IO_Type_Bit, pcode)?
                                                      true : false;
                INPUT_ACTION_Update (&P->bitAction[pcode], Status);
            }
        }
    }
#endif
}


IO_Code
INPUT__isMappedByDriver (const enum INPUT_PROFILE_Type ProfileType,
                         const enum IO_Type IoType,
                         const IO_Code ProfileCode,
                         const struct IO *const Driver)
{
    struct IO_PROFILE_Map *const Map =
        getMapping (ProfileType, IoType, ProfileCode);

    return (driversMatch(Map, Driver))? Map->driverCode : IO_INVALID_CODE;
}


struct IO_ConstGateway
INPUT__getMappedGateway (const enum INPUT_PROFILE_Type ProfileType,
                         const enum IO_Type IoType,
                         const IO_Code ProfileCode)
{
    struct IO_PROFILE_Map *const Map =
        getMapping (ProfileType, IoType, ProfileCode);

    const struct IO_Gateway *const Gateway = getGateway (Map);

    return (struct IO_ConstGateway)
    {
        .Driver     = Gateway->driver,
        .DriverPort = Gateway->driverPort 
    };
}
