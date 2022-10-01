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


void OUTPUT_Init (struct OUTPUT *const O)
{
    BOARD_AssertState  (!s_o);
    BOARD_AssertParams (O);

    OBJECT_Clear (O);

    {
        LOG_AutoContext (O, LANG_INIT);

        s_o = O;

        LOG_Items (1, LANG_MAX_SOURCES, (uint32_t)OUTPUT_MAX_GATEWAYS);
    }
}


void OUTPUT_SetGateway (struct IO *const Driver, const IO_Port DriverPort)
{
    BOARD_AssertParams (s_o->nextGatewayId < OUTPUT_MAX_GATEWAYS && Driver);

    s_o->gateways[s_o->nextGatewayId].driver     = Driver;
    s_o->gateways[s_o->nextGatewayId].driverPort = DriverPort;

    ++ s_o->nextGatewayId;
}


static void outMapByGatewayId (struct IO_PROFILE_Map *const Map, 
                               const uint16_t GatewayId,
                               const IO_Code DriverInx)
{
    BOARD_AssertParams (GatewayId < OUTPUT_MAX_GATEWAYS);
    BOARD_AssertState  (s_o->gateways[GatewayId].driver);

    Map->gatewayId  = GatewayId;
    Map->driverCode  = DriverInx;
}


static void outputMapCurrent (struct IO_PROFILE_Map *const Map, 
                              const IO_Code DriverInx)
{
    BOARD_AssertState (s_o->nextGatewayId);

    const uint16_t CurrentGatewayId = s_o->nextGatewayId - 1;

    outMapByGatewayId (Map, CurrentGatewayId, DriverInx);
}


inline static struct OUTPUT_PROFILE * 
getProfile (const enum OUTPUT_PROFILE_Type ProfileType)
{
    BOARD_AssertParams (ProfileType < OUTPUT_PROFILE_Type__COUNT);

    struct OUTPUT_PROFILE *const P = &s_o->profiles[ProfileType];
    return P;
}


// BitMap must exist in ProfileType
static struct IO_PROFILE_Map *
getBitMapping (const enum OUTPUT_PROFILE_Type ProfileType,
               const uint32_t Inb)
{
    struct OUTPUT_PROFILE *const P = getProfile (ProfileType);
    if (Inb >= P->bitCount || !P->bitMap)
    {
        LOG_Warn (s_o, LANG_INVALID_IMPLEMENTATION);
        LOG_Items (3,
                    "inb",          Inb,
                    "bitMap",       (void *)P->bitMap,
                    "bitCount",     P->bitCount);

        BOARD_AssertState (false);
    }
    return &P->bitMap[Inb];
}


// RangeMap must exist in ProfileType
static struct IO_PROFILE_Map *
getRangeMapping (const enum OUTPUT_PROFILE_Type ProfileType,
                 const uint32_t Inr)
{
    struct OUTPUT_PROFILE *const P = getProfile (ProfileType);
    if (Inr >= P->rangeCount || !P->rangeMap)
    {
        LOG_Warn (s_o, LANG_INVALID_IMPLEMENTATION);
        LOG_Items (3,
                    "inr",          Inr,
                    "rangeMap",     (void *)P->rangeMap,
                    "rangeCount",   P->rangeCount);

        BOARD_AssertState (false);
    }
    return &P->rangeMap[Inr];
}


bool OUTPUT_HasProfile (const enum OUTPUT_PROFILE_Type ProfileType)
{
    struct OUTPUT_PROFILE *const P = getProfile (ProfileType);
    return (P->bitCount || P->rangeCount)? true : false;
}


uint32_t OUTPUT_ProfileBits (const enum OUTPUT_PROFILE_Type ProfileType)
{
    return getProfile(ProfileType)->bitCount;
}


uint32_t OUTPUT_ProfileRanges (const enum OUTPUT_PROFILE_Type ProfileType)
{
    return getProfile(ProfileType)->rangeCount;
}


void OUTPUT_MapBit (const enum OUTPUT_PROFILE_Type ProfileType,
                    const IO_Code Outb, const IO_Code DriverInx)
{
    outputMapCurrent (getBitMapping(ProfileType, Outb), DriverInx);
}


void OUTPUT_MapRange (const enum OUTPUT_PROFILE_Type ProfileType,
                      const IO_Code Outr, const IO_Code DriverInx)
{
    outputMapCurrent (getBitMapping(ProfileType, Outr), DriverInx);   
}


static bool isMapped (const struct IO_PROFILE_Map *const Map)
{
    const IO_Code DriverInx = Map->driverCode;
    return (DriverInx != IO_INVALID_CODE)? true : false;
}


static void output (const enum IO_Type IoType,
                    const struct IO_PROFILE_Map *const Map,
                    const uint32_t Value,
                    enum OUTPUT_UpdateValue When)
{
    if (!isMapped (Map))
    {
        return;
    }

    const IO_Code DriverCode = Map->driverCode;

    const enum IO_UpdateValue WhenIo = (When == OUTPUT_UpdateValue_Now)?
                                                        IO_UpdateValue_Now :
                                                        IO_UpdateValue_Async;

    if (DriverCode != IO_INVALID_CODE)
    {
        const uint16_t GatewayId = Map->gatewayId;

        BOARD_AssertState (GatewayId < OUTPUT_MAX_GATEWAYS);

        struct IO_Gateway *const G = &s_o->gateways[GatewayId];

        IO_SetOutput (G->driver, IoType, DriverCode,
                      s_o->gateways[GatewayId].driverPort, Value, WhenIo);
    }
}


void OUTPUT_SetBit (const enum OUTPUT_PROFILE_Type ProfileType,
                    const IO_Code Outb, const uint32_t Value,
                    const enum OUTPUT_UpdateValue When)
{
    struct OUTPUT_PROFILE *const P = getProfile (ProfileType);

    if (!P->bitMap)
    {
        return;
    }

    BOARD_AssertParams (Outb < P->bitCount);

    output (IO_Type_Bit, &P->bitMap[Outb], Value, When);
}


void OUTPUT_SetBitNow (const enum OUTPUT_PROFILE_Type ProfileType,
                       const IO_Code Outb, const uint32_t Value)
{
    OUTPUT_SetBit (ProfileType, Outb, Value, OUTPUT_UpdateValue_Now);
}


void OUTPUT_SetBitDefer (const enum OUTPUT_PROFILE_Type ProfileType,
                         const IO_Code Outb, const uint32_t Value)
{
    OUTPUT_SetBit (ProfileType, Outb, Value, OUTPUT_UpdateValue_Defer);
}


void OUTPUT_SetRange (const enum OUTPUT_PROFILE_Type ProfileType,
                      const IO_Code Outr, const uint32_t Value,
                      const enum OUTPUT_UpdateValue When)
{
    struct OUTPUT_PROFILE *const P = getProfile (ProfileType);

    if (!P->rangeMap)
    {
        return;
    }

    BOARD_AssertParams (Outr < P->rangeCount);

    output (IO_Type_Range, &P->rangeMap[Outr], Value, When);
}


void OUTPUT_SetRangeNow (const enum OUTPUT_PROFILE_Type ProfileType,
                         const IO_Code Outr, const uint32_t Value)
{
    OUTPUT_SetRange (ProfileType, Outr, Value, OUTPUT_UpdateValue_Now);
}


void OUTPUT_SetRangeDefer (const enum OUTPUT_PROFILE_Type ProfileType,
                           const IO_Code Outr, const uint32_t Value)
{
    OUTPUT_SetRange (ProfileType, Outr, Value, OUTPUT_UpdateValue_Defer);
}


static const char * 
mappedOutputName (const enum OUTPUT_PROFILE_Type ProfileType,
                  const enum IO_Type IoType, const IO_Code Outx)
{
    struct IO_PROFILE_Map *const Map = (IoType == IO_Type_Bit)?
                                            getBitMapping(ProfileType, Outx) :
                                            getRangeMapping(ProfileType, Outx);

    BOARD_AssertParams (Map->driverCode != IO_INVALID_CODE);

    struct IO_Gateway *const G = &s_o->gateways[Map->gatewayId];

    return IO_OutputName (G->driver, IoType, Map->driverCode);    
}


const char * OUTPUT_MappedBitName (const enum OUTPUT_PROFILE_Type ProfileType,
                                   const IO_Code Outb)
{
    return mappedOutputName (ProfileType, IO_Type_Bit, Outb);
}


const char * OUTPUT_MappedRangeName (const enum OUTPUT_PROFILE_Type ProfileType,
                                     const IO_Code Outr)
{
    return mappedOutputName (ProfileType, IO_Type_Range, Outr);
}


void OUTPUT_Update (void)
{
    for (uint32_t gatewayId = 0; gatewayId < OUTPUT_MAX_GATEWAYS; ++gatewayId)
    {
        struct IO * outDriver = s_o->gateways[gatewayId].driver;

        if (outDriver)
        {
            IO_Update (outDriver);
        }
    }
}
