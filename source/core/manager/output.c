/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [MANAGER] output devices manager.

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

#include "embedul.ar/source/core/manager/output.h"
#include "embedul.ar/source/core/device/board.h"


static struct OUTPUT * s_o = NULL;


static void outMapByGatewayId (struct IO_PROFILE_Map *const Map, 
                               const IO_GatewayId GatewayId,
                               const IO_Code DriverCode)
{
    BOARD_AssertParams (GatewayId < OUTPUT_MAX_GATEWAYS);
    BOARD_AssertState  (s_o->gateways[GatewayId].driver);

    Map->gatewayId  = GatewayId;
    Map->driverCode = DriverCode;
}


static void outputMapCurrent (struct IO_PROFILE_Map *const Map, 
                              const IO_Code DriverCode)
{
    BOARD_AssertState (s_o->nextGatewayId);

    const IO_GatewayId CurrentGatewayId = s_o->nextGatewayId - 1;

    outMapByGatewayId (Map, CurrentGatewayId, DriverCode);
}


inline static struct OUTPUT_PROFILE * 
getProfile (const enum OUTPUT_PROFILE_Type ProfileType)
{
    struct OUTPUT_PROFILE *const P = &s_o->profiles[ProfileType];
    return P;
}


// BitMap or RangeMap must exist in ProfileType
static struct IO_PROFILE_Map *
getMapping (const enum OUTPUT_PROFILE_Type ProfileType,
            const enum IO_Type IoType, const IO_Code ProfileCode)
{
    struct OUTPUT_PROFILE *const P = getProfile (ProfileType);
    if (ProfileCode >= P->count[IoType] || !P->map[IoType])
    {
        LOG_Warn (s_o, LANG_INVALID_IO_PROFILE);
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
    struct IO_Gateway *const G = &s_o->gateways[Map->gatewayId];
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


static void output (const enum IO_Type IoType,
                    const struct IO_PROFILE_Map *const Map,
                    const IO_Value Value,
                    enum OUTPUT_UpdateValue When)
{
    if (!isMapped (Map))
    {
        return;
    }

    struct IO_Gateway *const G = getGateway (Map);

    IO_SetOutput (G->driver, IoType, Map->driverCode,
                  G->driverPort, Value, (enum IO_UpdateValue)When);
}


void OUTPUT_Init (struct OUTPUT *const O)
{
    BOARD_AssertState  (!s_o);
    BOARD_AssertParams (O);

    OBJECT_Clear (O);

    {
        LOG_AutoContext (O, LANG_INIT);

        s_o = O;

        s_o->currentMapAction = IO_MapAction_NoRemap;

        LOG_Items (1, LANG_MAX_SOURCES, (uint32_t)OUTPUT_MAX_GATEWAYS);
    }
}


void OUTPUT_RegisterGateway (struct IO *const Driver, const IO_Port DriverPort)
{
    BOARD_AssertParams (s_o->nextGatewayId < OUTPUT_MAX_GATEWAYS && Driver);

    s_o->gateways[s_o->nextGatewayId].driver     = Driver;
    s_o->gateways[s_o->nextGatewayId].driverPort = DriverPort;

    ++ s_o->nextGatewayId;
}


void OUTPUT_CurrentMapAction (const enum IO_MapAction MapAction)
{
    s_o->currentMapAction = MapAction;
}


bool OUTPUT_HasProfile (const enum OUTPUT_PROFILE_Type ProfileType)
{
    BOARD_AssertParams (ProfileType < OUTPUT_PROFILE_Type__COUNT);

    struct OUTPUT_PROFILE *const P = getProfile (ProfileType);
    return (P->count[IO_Type_Bit] || P->count[IO_Type_Range])?
            true : false;
}


uint32_t OUTPUT_ProfileCodes (const enum OUTPUT_PROFILE_Type ProfileType,
                              const enum IO_Type IoType)
{
    BOARD_AssertParams (ProfileType < OUTPUT_PROFILE_Type__COUNT &&
                        IoType < IO_Type__COUNT);

    return getProfile(ProfileType)->count[IoType];
}


bool OUTPUT_IsMapped (const enum OUTPUT_PROFILE_Type ProfileType,
                      const enum IO_Type IoType, const IO_Code ProfileCode)
{
    BOARD_AssertParams (ProfileType < OUTPUT_PROFILE_Type__COUNT &&
                        IoType < IO_Type__COUNT);

    return isMapped (getMapping(ProfileType, IoType, ProfileCode));
}


void OUTPUT_Map (const enum OUTPUT_PROFILE_Type ProfileType,
                 const enum IO_Type IoType, const IO_Code ProfileCode,
                 const IO_Code DriverCode)
{
    BOARD_AssertParams (ProfileType < OUTPUT_PROFILE_Type__COUNT &&
                        IoType < IO_Type__COUNT);

    struct IO_PROFILE_Map *const Map =
        getMapping (ProfileType, IoType, ProfileCode);

    if (!isMapped(Map) || s_o->currentMapAction == IO_MapAction_Overwrite)
    {
        outputMapCurrent (Map, DriverCode);
    }
}


void OUTPUT_Set (const enum OUTPUT_PROFILE_Type ProfileType,
                 const enum IO_Type IoType, const IO_Code ProfileCode,
                 const IO_Value Value, const enum OUTPUT_UpdateValue When)
{
    BOARD_AssertParams (ProfileType < OUTPUT_PROFILE_Type__COUNT &&
                        IoType < IO_Type__COUNT);

    struct OUTPUT_PROFILE *const P = getProfile (ProfileType);

    if (!P->map[IoType])
    {
        return;
    }

    BOARD_AssertParams (ProfileCode < P->count[IoType]);

    output (IoType, &P->map[IoType][ProfileCode], Value, When);
}


void OUTPUT_SetNow (const enum OUTPUT_PROFILE_Type ProfileType,
                    const enum IO_Type IoType, const IO_Code ProfileCode,
                    const IO_Value Value)
{
    OUTPUT_Set (ProfileType, IoType, ProfileCode, Value,
                OUTPUT_UpdateValue_Now);
}


void OUTPUT_SetDefer (const enum OUTPUT_PROFILE_Type ProfileType,
                      const enum IO_Type IoType, const IO_Code ProfileCode,
                      const IO_Value Value)
{
    OUTPUT_Set (ProfileType, IoType, ProfileCode, Value,
                OUTPUT_UpdateValue_Defer);
}


const char * OUTPUT_MappedName (const enum OUTPUT_PROFILE_Type ProfileType,
                                const enum IO_Type IoType,
                                const IO_Code ProfileCode)
{
    BOARD_AssertParams (ProfileType < OUTPUT_PROFILE_Type__COUNT &&
                        IoType < IO_Type__COUNT);

    struct IO_PROFILE_Map *const Map =
        getMapping (ProfileType, IoType, ProfileCode);

    BOARD_AssertParams (Map->driverCode != IO_INVALID_CODE);

    struct IO_Gateway *const G = getGateway (Map);

    return IO_OutputName (G->driver, IoType, Map->driverCode);    
}


void OUTPUT_Update (void)
{
    for (uint32_t gId = 0; gId < OUTPUT_MAX_GATEWAYS; ++gId)
    {
        struct IO * outDriver = s_o->gateways[gId].driver;

        if (outDriver)
        {
            IO_Update (outDriver);
        }
    }
}


IO_Code 
OUTPUT__isMappedByDriver (const enum OUTPUT_PROFILE_Type ProfileType,
                          const enum IO_Type IoType,
                          const IO_Code ProfileCode,
                          const struct IO *const Driver)
{
    struct IO_PROFILE_Map *const Map =
        getMapping (ProfileType, IoType, ProfileCode);

    return (driversMatch(Map, Driver))? Map->driverCode : IO_INVALID_CODE;
}


struct IO_ConstGateway
OUTPUT__getMappedGateway (const enum OUTPUT_PROFILE_Type ProfileType,
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
