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


void INPUT_Init (struct INPUT *const I)
{
    BOARD_AssertState  (!s_i);
    BOARD_AssertParams (I);

    OBJECT_Clear (I);

    {
        LOG_AutoContext (I, LANG_INIT);

        s_i = I;

        LOG_Items (1, LANG_MAX_SOURCES, (uint32_t)INPUT_MAX_GATEWAYS);
    }
}


void INPUT_SetGateway (struct IO *const Driver, const IO_Port DriverPort)
{
    BOARD_AssertParams (s_i->nextGatewayId < INPUT_MAX_GATEWAYS && Driver);

    s_i->gateways[s_i->nextGatewayId].driver     = Driver;
    s_i->gateways[s_i->nextGatewayId].driverPort = DriverPort;

    ++ s_i->nextGatewayId;
}


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
    BOARD_AssertParams (ProfileType < INPUT_PROFILE_Type__COUNT);

    struct INPUT_PROFILE *const P = &s_i->profiles[ProfileType];
    return P;
}


// BitMap must exist in ProfileType
static struct IO_PROFILE_Map *
getBitMapping (const enum INPUT_PROFILE_Type ProfileType,
               const IO_Code ProfileCode)
{
    struct INPUT_PROFILE *const P = getProfile (ProfileType);
    if (ProfileCode >= P->bitCount || !P->bitMap)
    {
        LOG_Warn (s_i, LANG_INVALID_IO_PROFILE);
        LOG_Items (3,
                    "profile code",     ProfileCode,
                    "bitMap addr.",     (void *)P->bitMap,
                    "bitCount",         P->bitCount);

        BOARD_AssertState (false);
    }
    return &P->bitMap[ProfileCode];
}


// RangeMap must exist in ProfileType
static struct IO_PROFILE_Map *
getRangeMapping (const enum INPUT_PROFILE_Type ProfileType,
                 const IO_Code ProfileCode)
{
    struct INPUT_PROFILE *const P = getProfile (ProfileType);
    if (ProfileCode >= P->rangeCount || !P->rangeMap)
    {
        LOG_Warn (s_i, LANG_INVALID_IO_PROFILE);
        LOG_Items (3,
                    "profile code",     ProfileCode,
                    "rangeMap addr.",   (void *)P->rangeMap,
                    "rangeCount",       P->rangeCount);

        BOARD_AssertState (false);
    }
    return &P->rangeMap[ProfileCode];
}


bool INPUT_HasProfile (const enum INPUT_PROFILE_Type ProfileType)
{
    struct INPUT_PROFILE *const P = getProfile (ProfileType);
    return (P->bitCount || P->rangeCount)? true : false;
}


uint32_t INPUT_ProfileBits (const enum INPUT_PROFILE_Type ProfileType)
{
    return getProfile(ProfileType)->bitCount;
}


uint32_t INPUT_ProfileRanges (const enum INPUT_PROFILE_Type ProfileType)
{
    return getProfile(ProfileType)->rangeCount;
}


void INPUT_MapBit (const enum INPUT_PROFILE_Type ProfileType,
                   const IO_Code ProfileCode, const IO_Code DriverCode)
{
    inputMapCurrent (getBitMapping(ProfileType, ProfileCode), DriverCode);
}


void INPUT_MapRange (const enum INPUT_PROFILE_Type ProfileType,
                     const IO_Code ProfileCode, const IO_Code DriverCode)
{
    inputMapCurrent (getRangeMapping(ProfileType, ProfileCode), DriverCode);
}


bool INPUT_MapBitByOnState (const enum INPUT_PROFILE_Type ProfileType,
                            const IO_Code ProfileCode)
{
    struct IO_PROFILE_Map *const Map = getBitMapping (ProfileType, ProfileCode);

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


static bool isMapped (const struct IO_PROFILE_Map *const Map)
{
    const IO_Code DriverCode = Map->driverCode;
    return (DriverCode != IO_INVALID_CODE)? true : false;
}


static uint32_t input (const enum IO_Type IoType,
                       const struct IO_PROFILE_Map *const Map,
                       const enum INPUT_UpdateValue When)
{
    if (!isMapped (Map))
    {
        return 0;
    }

    const IO_GatewayId  GatewayId   = Map->gatewayId;
    const IO_Code       DriverCode  = Map->driverCode;

    const enum IO_UpdateValue WhenIo = (When == INPUT_UpdateValue_Now)?
                                                        IO_UpdateValue_Now :
                                                        IO_UpdateValue_Async;

    BOARD_AssertState (GatewayId < INPUT_MAX_GATEWAYS);

    struct IO_Gateway *const G = &s_i->gateways[GatewayId];

    const uint32_t Status = IO_GetInput (
                                G->driver, IoType, DriverCode,
                                s_i->gateways[GatewayId].driverPort,
                                WhenIo);
    return Status;
}


uint32_t INPUT_GetBit (const enum INPUT_PROFILE_Type ProfileType,
                       const IO_Code ProfileCode,
                       const enum INPUT_UpdateValue When)
{
    struct INPUT_PROFILE *const P = getProfile (ProfileType);

    if (!P->bitMap)
    {
        return 0;
    }

    BOARD_AssertParams (ProfileCode < P->bitCount);

    return input (IO_Type_Bit, &P->bitMap[ProfileCode], When);
}


uint32_t INPUT_GetBitNow (const enum INPUT_PROFILE_Type ProfileType,
                          const IO_Code ProfileCode)
{
    return INPUT_GetBit (ProfileType, ProfileCode, INPUT_UpdateValue_Now);
}


uint32_t INPUT_GetBitBuffer (const enum INPUT_PROFILE_Type ProfileType,
                             const IO_Code ProfileCode)
{
    return INPUT_GetBit (ProfileType, ProfileCode, INPUT_UpdateValue_Buffer);
}


uint32_t INPUT_GetRange (const enum INPUT_PROFILE_Type ProfileType,
                         const IO_Code ProfileCode,
                         const enum INPUT_UpdateValue When)
{
    struct INPUT_PROFILE *const P = getProfile (ProfileType);

    if (!P->rangeMap)
    {
        return 0;
    }

    BOARD_AssertParams (ProfileCode < P->rangeCount);

    return input (IO_Type_Range, &P->rangeMap[ProfileCode], When);
}


#if (LIB_EMBEDULAR_CONFIG_INPUT_ACTION == 1)
enum INPUT_ACTION_Type
INPUT_GetBitAction (const enum INPUT_PROFILE_Type ProfileType,
                    const IO_Code ProfileCode)
{
    struct INPUT_PROFILE *const P = getProfile (ProfileType);

    if (!P->bitAction)
    {
        return INPUT_ACTION_Type_None;
    }

    BOARD_AssertParams (ProfileCode < P->bitCount);

    return INPUT_ACTION_Last (&P->bitAction[ProfileCode]);
}
#endif


uint32_t INPUT_GetRangeNow (const enum INPUT_PROFILE_Type ProfileType,
                            const IO_Code ProfileCode)
{
    return INPUT_GetRange (ProfileType, ProfileCode, INPUT_UpdateValue_Now);
}


uint32_t INPUT_GetRangeBuffer (const enum INPUT_PROFILE_Type ProfileType,
                               const IO_Code ProfileCode)
{
    return INPUT_GetRange (ProfileType, ProfileCode, INPUT_UpdateValue_Buffer);
}


static const char *
mappedInputName (const enum INPUT_PROFILE_Type ProfileType,
                 const enum IO_Type IoType, const IO_Code ProfileCode)
{
    struct IO_PROFILE_Map *const Map = (IoType == IO_Type_Bit)?
                                    getBitMapping(ProfileType, ProfileCode) :
                                    getRangeMapping(ProfileType, ProfileCode);

    BOARD_AssertParams (Map->driverCode != IO_INVALID_CODE);

    struct IO_Gateway *const G = &s_i->gateways[Map->gatewayId];

    return IO_InputName (G->driver, IoType, Map->driverCode);
}


const char * INPUT_MappedBitName (const enum INPUT_PROFILE_Type ProfileType,
                                  const IO_Code ProfileCode)
{
    return mappedInputName (ProfileType, IO_Type_Bit, ProfileCode);
}


const char * INPUT_MappedRangeName (const enum INPUT_PROFILE_Type ProfileType,
                                    const IO_Code ProfileCode)
{
    return mappedInputName (ProfileType, IO_Type_Range, ProfileCode);
}


bool INPUT_IsBitMapped (const enum INPUT_PROFILE_Type ProfileType,
                        const IO_Code ProfileCode)
{
    return isMapped (getBitMapping(ProfileType, ProfileCode));
}


bool INPUT_IsRangeMapped (const enum INPUT_PROFILE_Type ProfileType,
                          const IO_Code ProfileCode)
{
    return isMapped (getRangeMapping(ProfileType, ProfileCode));
}


bool INPUT_AnyBit (const enum INPUT_PROFILE_SelectFlag Profiles)
{
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

            if (P->bitCount)
            {
                for (uint32_t pcode = 0; pcode < P->bitCount; ++pcode)
                {
                    if (input (IO_Type_Bit, &P->bitMap[pcode],
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
            for (uint32_t pcode = 0; pcode < P->bitCount; ++pcode)
            {
                const bool Status = INPUT_GetBitBuffer(t, pcode)? true : false;
                INPUT_ACTION_Update (&P->bitAction[pcode], Status);
            }
        }
    }
#endif
}
