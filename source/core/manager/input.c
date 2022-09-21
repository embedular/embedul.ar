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

    LOG_ContextBegin (I, LANG_INIT);
    {
        memset (I->bitMapping, IO_INVALID_INDEX, sizeof(I->bitMapping));
        memset (I->rangeMapping, IO_INVALID_INDEX, sizeof(I->rangeMapping));

        s_i = I;

        LOG_Items (3,
                    LANG_MAX_DEVICES,   (uint32_t)INPUT_MAX_DEVICES,
                    LANG_BIT_COUNT,     (uint32_t)INPUT_Bit__COUNT,
                    LANG_RANGE_COUNT,   (uint32_t)INPUT_Range__COUNT);
    }
    LOG_ContextEnd ();
}


void INPUT_SetDevice (struct IO *const Driver, const uint32_t DriverSource)
{
    BOARD_AssertParams (s_i->nextDeviceId < INPUT_MAX_DEVICES && Driver);

    s_i->device[s_i->nextDeviceId].driver        = Driver;
    s_i->device[s_i->nextDeviceId].driverSource  = DriverSource;

    ++ s_i->nextDeviceId;
}


static void inputMapByDeviceId (struct INPUT_Mapping *const Mapping,
                                const uint16_t DeviceId,
                                const uint16_t DriverIndex)
{
    BOARD_AssertParams (DeviceId < INPUT_MAX_DEVICES);
    BOARD_AssertState  (s_i->device[DeviceId].driver);

    Mapping->deviceId       = DeviceId;
    Mapping->driverIndex    = DriverIndex;
}


static void inputMapLast (struct INPUT_Mapping *const Mapping,
                          const uint16_t DriverIndex)
{
    BOARD_AssertState (s_i->nextDeviceId);

    const uint16_t CurrentDeviceId = s_i->nextDeviceId - 1;

    inputMapByDeviceId (Mapping, CurrentDeviceId, DriverIndex);
}


void INPUT_MapBit (const enum INPUT_Bit Inb, const uint16_t DriverIndex)
{
    BOARD_AssertParams (Inb < INPUT_Bit__COUNT);
    inputMapLast (&s_i->bitMapping[Inb], DriverIndex);
}


void INPUT_MapRange (const enum INPUT_Range Inr, const uint16_t DriverIndex)
{
    BOARD_AssertParams (Inr < INPUT_Range__COUNT);
    inputMapLast (&s_i->rangeMapping[Inr], DriverIndex);
}


bool INPUT_MapBitByOnState (const enum INPUT_Bit Inb)
{
    BOARD_AssertParams (Inb < INPUT_Bit__COUNT);

    for (uint8_t deviceId = 0; deviceId < INPUT_MAX_DEVICES; ++deviceId)
    {
        struct INPUT_Device * dev = &s_i->device[deviceId];
        if (dev->driver)
        {
            const uint16_t DriverIndex = IO_FirstOnIndex (
                                                    dev->driver,
                                                    IO_Type_Bit,
                                                    dev->driverSource);
            if (DriverIndex != IO_INVALID_INDEX)
            {
                inputMapByDeviceId (&s_i->bitMapping[Inb], deviceId, 
                                    DriverIndex);
                return true;
            }
        }
    }

    return false;
}


static bool isMapped (const struct INPUT_Mapping *const Mapping)
{
    const uint16_t DriverIndex = Mapping->driverIndex;
    return (DriverIndex != IO_INVALID_INDEX);    
}


static uint32_t input (const enum IO_Type IoType,
                       const struct INPUT_Mapping *const Mapping,
                       const enum INPUT_UpdateValue When)
{
    if (!isMapped (Mapping))
    {
        return 0;
    }

    const uint16_t DriverIndex  = Mapping->driverIndex;
    const uint16_t DeviceId     = Mapping->deviceId;

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


uint32_t INPUT_Bit (const enum INPUT_Bit Inb, 
                    const enum INPUT_UpdateValue When)
{
    BOARD_AssertParams (Inb < INPUT_Bit__COUNT);
    return input (IO_Type_Bit, &s_i->bitMapping[Inb], When);
}


uint32_t INPUT_BitNow (const enum INPUT_Bit Inb)
{
    return INPUT_Bit (Inb, INPUT_UpdateValue_Now);
}


uint32_t INPUT_BitBuffer (const enum INPUT_Bit Inb)
{
    return INPUT_Bit (Inb, INPUT_UpdateValue_Buffer);
}


uint32_t INPUT_Range (const enum INPUT_Range Inr,
                      const enum INPUT_UpdateValue When)
{
    BOARD_AssertParams (Inr < INPUT_Range__COUNT);
    return input (IO_Type_Range, &s_i->rangeMapping[Inr], When);
}


uint32_t INPUT_RangeNow (const enum INPUT_Range Inr)
{
    return INPUT_Range (Inr, INPUT_UpdateValue_Now);
}


uint32_t INPUT_RangeBuffer (const enum INPUT_Range Inr)
{
    return INPUT_Range (Inr, INPUT_UpdateValue_Buffer);
}


static const char * inputName (const enum IO_Type IoType,
                               const uint32_t Inx)
{
    struct INPUT_Mapping *const Map = (IoType == IO_Type_Bit)?
                                            &s_i->bitMapping[Inx] :
                                            &s_i->rangeMapping[Inx];

    BOARD_AssertParams (Map->driverIndex != IO_INVALID_INDEX);

    struct INPUT_Device *dev = &s_i->device[Map->deviceId];

    return IO_InputName (dev->driver, IoType, Map->driverIndex);
}


const char * INPUT_BitName (const enum INPUT_Bit Inb)
{
    BOARD_AssertParams (Inb < INPUT_Bit__COUNT);
    return inputName (IO_Type_Bit, Inb);
}


const char * INPUT_RangeName (const enum INPUT_Range Inr)
{
    BOARD_AssertParams (Inr < INPUT_Range__COUNT);
    return inputName (IO_Type_Range, Inr);
}


bool INPUT_BitIsMapped (const enum INPUT_Bit Inb)
{
    BOARD_AssertParams (Inb < INPUT_Bit__COUNT);
    return isMapped (&s_i->bitMapping[Inb]);
}


bool INPUT_RangeIsMapped (const enum INPUT_Range Inr)
{
    BOARD_AssertParams (Inr < INPUT_Range__COUNT);
    return isMapped (&s_i->rangeMapping[Inr]);
}


static bool bitByRange (enum INPUT_Bit begin, enum INPUT_Bit end)
{
    for (uint32_t n = begin; n <= end; ++n)
    {
        if (INPUT_BitBuffer (n))
        {
            return true;
        }
    }

    return false;
}


bool INPUT_AnyBit (void)
{
    return bitByRange (0, INPUT_Bit__COUNT - 1);
}


bool INPUT_AnyBitByRange (const enum INPUT_Bit Begin,
                          const enum INPUT_Bit End)
{
    BOARD_AssertParams (Begin <= End);
    return bitByRange (Begin, End);
}


bool INPUT_AnyBitByRole (const enum INPUT_BitRole BitRole)
{
    switch (BitRole)
    {
        case INPUT_BitRole_Any:
            return INPUT_AnyBit ();
            
        case INPUT_BitRole_Board:
            return bitByRange (INPUT_Bit_Board__BEGIN, INPUT_Bit_Board__END);

        case INPUT_BitRole_P1:
            return bitByRange (INPUT_Bit_P1__BEGIN, INPUT_Bit_P1__END);

        case INPUT_BitRole_P2:
            return bitByRange (INPUT_Bit_P2__BEGIN, INPUT_Bit_P2__END);

        case INPUT_BitRole_Switch:
            return bitByRange (INPUT_Bit_Switch__BEGIN, INPUT_Bit_Switch__END);
    }

    BOARD_AssertUnexpectedValue (s_i, (uint32_t)BitRole);
    return false;
}


#if (LIB_EMBEDULAR_CONFIG_INPUT_SWITCH_ACTION == 1)
enum SWITCH_ACTION_Type INPUT_BitAsSwitchAction (const enum INPUT_Bit Inb)
{
    BOARD_AssertParams (Inb < INPUT_Bit__COUNT);

    return SWITCH_ACTION_Last (&s_i->switchAction[Inb]);
}
#endif


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

#if (LIB_EMBEDULAR_CONFIG_INPUT_SWITCH_ACTION == 1)
    for (uint32_t n = 0; n < INPUT_Bit__COUNT; ++n)
    {
        const bool Status = INPUT_BitBuffer(n)? true : false;

        SWITCH_ACTION_Update (&s_i->switchAction[n], Status);
    }
#endif
}
