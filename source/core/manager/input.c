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

        LOG_Items (1,
                    LANG_MAX_DEVICES,   (uint32_t)INPUT_MAX_DEVICES);
    }
}


void INPUT_SetDevice (struct IO *const Driver, const uint32_t DriverSource)
{
    BOARD_AssertParams (s_i->nextDeviceId < INPUT_MAX_DEVICES && Driver);

    s_i->device[s_i->nextDeviceId].driver        = Driver;
    s_i->device[s_i->nextDeviceId].driverSource  = DriverSource;

    ++ s_i->nextDeviceId;
}


static void inputMapByDeviceId (struct INPUT_PROFILE_Map *const Mapping,
                                const uint16_t DeviceId,
                                const uint16_t DriverIndex)
{
    BOARD_AssertParams (DeviceId < INPUT_MAX_DEVICES);
    BOARD_AssertState  (s_i->device[DeviceId].driver);

    Mapping->deviceId       = DeviceId;
    Mapping->driverIndex    = DriverIndex;
}


static void inputMapCurrent (struct INPUT_PROFILE_Map *const Mapping,
                          const uint16_t DriverIndex)
{
    BOARD_AssertState (s_i->nextDeviceId);

    const uint16_t CurrentDeviceId = s_i->nextDeviceId - 1;

    inputMapByDeviceId (Mapping, CurrentDeviceId, DriverIndex);
}


inline static struct INPUT_PROFILE * 
getProfile (const enum INPUT_PROFILE_Type ProfileType)
{
    BOARD_AssertParams (ProfileType < INPUT_PROFILE_Type__COUNT);

    struct INPUT_PROFILE *const P = &s_i->profiles[ProfileType];
    return P;
}


// BitMap must exist in ProfileType
static struct INPUT_PROFILE_Map *
getBitMapping (const enum INPUT_PROFILE_Type ProfileType,
               const uint32_t Inb)
{
    struct INPUT_PROFILE *const P = getProfile (ProfileType);
    if (Inb >= P->bitCount || !P->bitMap)
    {
        LOG_Warn (s_i, LANG_INVALID_IMPLEMENTATION);
        LOG_Items (3,
                    "inb",          Inb,
                    "bitMap",       (void *)P->bitMap,
                    "bitCount",     P->bitCount);

        BOARD_AssertState (false);
    }
    return &P->bitMap[Inb];
}


// RangeMap must exist in ProfileType
static struct INPUT_PROFILE_Map *
getRangeMapping (const enum INPUT_PROFILE_Type ProfileType,
                 const uint32_t Inr)
{
    struct INPUT_PROFILE *const P = getProfile (ProfileType);
    if (Inr >= P->rangeCount || !P->rangeMap)
    {
        LOG_Warn (s_i, LANG_INVALID_IMPLEMENTATION);
        LOG_Items (3,
                    "inr",          Inr,
                    "rangeMap",     (void *)P->rangeMap,
                    "rangeCount",   P->rangeCount);

        BOARD_AssertState (false);
    }
    return &P->rangeMap[Inr];
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
                   const uint32_t Inb, const uint16_t DriverIndex)
{
    inputMapCurrent (getBitMapping(ProfileType, Inb), DriverIndex);
}


void INPUT_MapRange (const enum INPUT_PROFILE_Type ProfileType,
                     const uint32_t Inr, const uint16_t DriverIndex)
{
    inputMapCurrent (getRangeMapping(ProfileType, Inr), DriverIndex);
}


bool INPUT_MapBitByOnState (const enum INPUT_PROFILE_Type ProfileType,
                            const uint32_t Inb)
{
    struct INPUT_PROFILE_Map *const Map = getBitMapping (ProfileType, Inb);

    // Check registered drivers for one that has any input bit in 1 
    for (uint8_t deviceId = 0; deviceId < INPUT_MAX_DEVICES; ++deviceId)
    {
        struct INPUT_Device * dev = &s_i->device[deviceId];
        if (dev->driver)
        {
            const uint16_t DriverIndex =
                            IO_FirstOnIndex (dev->driver, IO_Type_Bit,
                                             dev->driverSource);

            if (DriverIndex != IO_INVALID_INDEX)
            {
                inputMapByDeviceId (Map, deviceId, DriverIndex);
                return true;
            }
        }
    }

    return false;
}


static bool isMapped (const struct INPUT_PROFILE_Map *const Mapping)
{
    const uint16_t DriverIndex = Mapping->driverIndex;
    return (DriverIndex != IO_INVALID_INDEX)? true : false;
}


static uint32_t input (const enum IO_Type IoType,
                       const struct INPUT_PROFILE_Map *const Map,
                       const enum INPUT_UpdateValue When)
{
    if (!isMapped (Map))
    {
        return 0;
    }

    const uint16_t DriverIndex  = Map->driverIndex;
    const uint16_t DeviceId     = Map->deviceId;

    const enum IO_UpdateValue WhenIo = (When == INPUT_UpdateValue_Now)?
                                                        IO_UpdateValue_Now :
                                                        IO_UpdateValue_Async;

    BOARD_AssertState (DeviceId < INPUT_MAX_DEVICES);

    struct INPUT_Device * dev = &s_i->device[DeviceId];

    const uint32_t Status = IO_GetInput (
                                dev->driver, IoType, DriverIndex,
                                s_i->device[DeviceId].driverSource,
                                WhenIo);
    return Status;
}


uint32_t INPUT_GetBit (const enum INPUT_PROFILE_Type ProfileType,
                       const uint32_t Inb, const enum INPUT_UpdateValue When)
{
    struct INPUT_PROFILE *const P = getProfile (ProfileType);

    if (!P->bitMap)
    {
        return 0;
    }

    BOARD_AssertParams (Inb < P->bitCount);

    return input (IO_Type_Bit, &P->bitMap[Inb], When);
}


uint32_t INPUT_GetBitNow (const enum INPUT_PROFILE_Type ProfileType,
                          const uint32_t Inb)
{
    return INPUT_GetBit (ProfileType, Inb, INPUT_UpdateValue_Now);
}


uint32_t INPUT_GetBitBuffer (const enum INPUT_PROFILE_Type ProfileType,
                             const uint32_t Inb)
{
    return INPUT_GetBit (ProfileType, Inb, INPUT_UpdateValue_Buffer);
}


uint32_t INPUT_GetRange (const enum INPUT_PROFILE_Type ProfileType,
                         const uint32_t Inr, const enum INPUT_UpdateValue When)
{
    struct INPUT_PROFILE *const P = getProfile (ProfileType);

    if (!P->rangeMap)
    {
        return 0;
    }

    BOARD_AssertParams (Inr < P->rangeCount);

    return input (IO_Type_Range, &P->rangeMap[Inr], When);
}


#if (LIB_EMBEDULAR_CONFIG_INPUT_ACTION == 1)
enum INPUT_ACTION_Type
INPUT_GetBitAction (const enum INPUT_PROFILE_Type ProfileType,
                    const uint32_t Inb)
{
    struct INPUT_PROFILE *const P = getProfile (ProfileType);

    if (!P->bitAction)
    {
        return INPUT_ACTION_Type_None;
    }

    BOARD_AssertParams (Inb < P->bitCount);

    return INPUT_ACTION_Last (&P->bitAction[Inb]);
}
#endif


uint32_t INPUT_GetRangeNow (const enum INPUT_PROFILE_Type ProfileType,
                            const uint32_t Inr)
{
    return INPUT_GetRange (ProfileType, Inr, INPUT_UpdateValue_Now);
}


uint32_t INPUT_GetRangeBuffer (const enum INPUT_PROFILE_Type ProfileType,
                               const uint32_t Inr)
{
    return INPUT_GetRange (ProfileType, Inr, INPUT_UpdateValue_Buffer);
}


static const char * mappedInputName (const enum INPUT_PROFILE_Type ProfileType,
                                     const enum IO_Type IoType,
                                     const uint32_t Inx)
{
    struct INPUT_PROFILE_Map *const Map = (IoType == IO_Type_Bit)?
                                            getBitMapping(ProfileType, Inx) :
                                            getRangeMapping(ProfileType, Inx);

    BOARD_AssertParams (Map->driverIndex != IO_INVALID_INDEX);

    struct INPUT_Device *const Dev = &s_i->device[Map->deviceId];

    return IO_InputName (Dev->driver, IoType, Map->driverIndex);
}


const char * INPUT_MappedBitName (const enum INPUT_PROFILE_Type ProfileType,
                                  const uint32_t Inb)
{
    return mappedInputName (ProfileType, IO_Type_Bit, Inb);
}


const char * INPUT_MappedRangeName (const enum INPUT_PROFILE_Type ProfileType,
                                    const uint32_t Inr)
{
    return mappedInputName (ProfileType, IO_Type_Range, Inr);
}


bool INPUT_IsBitMapped (const enum INPUT_PROFILE_Type ProfileType,
                        const uint32_t Inb)
{
    return isMapped (getBitMapping(ProfileType, Inb));
}


bool INPUT_IsRangeMapped (const enum INPUT_PROFILE_Type ProfileType,
                          const uint32_t Inr)
{
    return isMapped (getRangeMapping(ProfileType, Inr));
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
                for (uint32_t inb = 0; inb < P->bitCount; ++inb)
                {
                    if (input (IO_Type_Bit, &P->bitMap[inb],
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
    for (uint32_t deviceId = 0; deviceId < INPUT_MAX_DEVICES; ++deviceId)
    {
        struct IO * inputDriver = s_i->device[deviceId].driver;

        if (inputDriver)
        {
            IO_Update (inputDriver);
        }
    }

#if (LIB_EMBEDULAR_CONFIG_INPUT_ACTION == 1)
    for (enum INPUT_PROFILE_Type t = 0; t < INPUT_PROFILE_Type__COUNT; ++t)
    {
        struct INPUT_PROFILE *const P = getProfile (t);
        if (P->bitAction)
        {
            for (uint32_t inb = 0; inb < P->bitCount; ++inb)
            {
                const bool Status = INPUT_GetBitBuffer(t, inb)? true : false;
                INPUT_ACTION_Update (&P->bitAction[inb], Status);
            }
        }
    }
#endif
}
