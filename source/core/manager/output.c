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

    LOG_ContextBegin (O, LANG_INIT);
    {
        memset (O->bitMapping, IO_INVALID_INDEX, sizeof(O->bitMapping));
        memset (O->rangeMapping, IO_INVALID_INDEX, sizeof(O->rangeMapping));

        s_o = O;

        LOG_Items (3,
                    LANG_MAX_DEVICES,   (uint32_t)OUTPUT_MAX_DEVICES,
                    LANG_BIT_COUNT,     (uint32_t)OUTPUT_Bit__COUNT,
                    LANG_RANGE_COUNT,   (uint32_t)OUTPUT_Range__COUNT);
    }
    LOG_ContextEnd ();
}


void OUTPUT_SetDevice (struct IO *const Driver, const uint32_t DriverSource)
{
    BOARD_AssertParams (s_o->nextDeviceId < OUTPUT_MAX_DEVICES && Driver);

    s_o->device[s_o->nextDeviceId].driver       = Driver;
    s_o->device[s_o->nextDeviceId].driverSource = DriverSource;

    ++ s_o->nextDeviceId;
}


static void outputMapByDeviceId (struct OUTPUT_Mapping *const Mapping, 
                                 const uint16_t DeviceId,
                                 const uint16_t DriverIndex)
{
    BOARD_AssertParams (DeviceId < OUTPUT_MAX_DEVICES);
    BOARD_AssertState  (s_o->device[DeviceId].driver);

    Mapping->deviceId       = DeviceId;
    Mapping->driverIndex    = DriverIndex;
}


static void outputMapLast (struct OUTPUT_Mapping *const Mapping, 
                           const uint16_t DriverIndex)
{
    BOARD_AssertState (s_o->nextDeviceId);

    const uint16_t CurrentDeviceId = s_o->nextDeviceId - 1;

    outputMapByDeviceId (Mapping, CurrentDeviceId, DriverIndex);
}


void OUTPUT_MapBit (const enum OUTPUT_Bit Outb, const uint16_t DriverIndex)
{
    BOARD_AssertParams (Outb < OUTPUT_Bit__COUNT);
    outputMapLast (&s_o->bitMapping[Outb], DriverIndex);
}


void OUTPUT_MapRange (const enum OUTPUT_Range Outr, const uint16_t DriverIndex)
{
    BOARD_AssertParams (Outr < OUTPUT_Range__COUNT);
    outputMapLast (&s_o->rangeMapping[Outr], DriverIndex);   
}


static void output (const enum IO_Type IoType,
                    const struct OUTPUT_Mapping *const Mapping,
                    const uint32_t Value,
                    enum OUTPUT_UpdateValue When)
{
    const uint16_t DriverIndex = Mapping->driverIndex;

    const enum IO_UpdateValue WhenIo = (When == OUTPUT_UpdateValue_Now)?
                                                        IO_UpdateValue_Now :
                                                        IO_UpdateValue_Async;

    if (DriverIndex != IO_INVALID_INDEX)
    {
        const uint16_t DeviceId = Mapping->deviceId;

        BOARD_AssertState (DeviceId < OUTPUT_MAX_DEVICES);

        struct OUTPUT_Device * dev = &s_o->device[DeviceId];

        IO_SetOutput (dev->driver, IoType, DriverIndex,
                      s_o->device[DeviceId].driverSource, Value, WhenIo);
    }
}


void OUTPUT_Bit (const enum OUTPUT_Bit Outb, const uint32_t Value,
                 const enum OUTPUT_UpdateValue When)
{
    BOARD_AssertParams (Outb < OUTPUT_Bit__COUNT);
    output (IO_Type_Bit, &s_o->bitMapping[Outb], Value, When);
}


void OUTPUT_BitNow (const enum OUTPUT_Bit Outb, const uint32_t Value)
{
    OUTPUT_Bit (Outb, Value, OUTPUT_UpdateValue_Now);
}


void OUTPUT_BitDefer (const enum OUTPUT_Bit Outb, const uint32_t Value)
{
    OUTPUT_Bit (Outb, Value, OUTPUT_UpdateValue_Defer);
}


void OUTPUT_Range (const enum OUTPUT_Range Outr, const uint32_t Value,
                   const enum OUTPUT_UpdateValue When)
{
    BOARD_AssertParams (Outr < OUTPUT_Range__COUNT);
    output (IO_Type_Range, &s_o->rangeMapping[Outr], Value, When);
}


void OUTPUT_RangeNow (const enum OUTPUT_Range Outr, const uint32_t Value)
{
    OUTPUT_Range (Outr, Value, OUTPUT_UpdateValue_Now);
}


void OUTPUT_RangeDefer (const enum OUTPUT_Range Outr, const uint32_t Value)
{
    OUTPUT_Range (Outr, Value, OUTPUT_UpdateValue_Defer);
}


static const char * outputName (const enum IO_Type IoType,
                                const uint32_t Out)
{
    struct OUTPUT_Mapping *map = (IoType == IO_Type_Bit)?
                                            &s_o->bitMapping[Out] :
                                            &s_o->rangeMapping[Out];

    BOARD_AssertParams (map->driverIndex != IO_INVALID_INDEX);

    struct OUTPUT_Device *dev = &s_o->device[map->deviceId];

    return IO_OutputName (dev->driver, IoType, map->driverIndex);    
}


const char * OUTPUT_BitName (const enum OUTPUT_Bit Outb)
{
    BOARD_AssertParams (Outb < OUTPUT_Bit__COUNT);
    return outputName (IO_Type_Bit, Outb);
}


const char * OUTPUT_RangeName (const enum OUTPUT_Range Outr)
{
    BOARD_AssertParams (Outr < OUTPUT_Range__COUNT);
    return outputName (IO_Type_Range, Outr);
}


void OUTPUT_Update (void)
{
    for (uint32_t deviceId = 0; deviceId < OUTPUT_MAX_DEVICES; ++deviceId)
    {
        struct IO * outputDriver = s_o->device[deviceId].driver;

        if (outputDriver)
        {
            IO_Update (outputDriver);
        }
    }
}
