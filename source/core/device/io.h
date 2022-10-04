/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [IO] input/output device driver interface.

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

#pragma once

#include "embedul.ar/source/core/bitfield.h"
#include "embedul.ar/source/core/timer.h"


#define IO_INVALID_CODE             ((IO_Code) -1)

#define IO_IFACE_DECLARE(_desc,_name) \
    .Description                = _desc, \
    .PortCount                  = IO_ ## _name ## _PORT_COUNT, \
    .InCount[IO_Type_Bit]       = IO_ ## _name ## _INB__COUNT, \
    .InCount[IO_Type_Range]     = IO_ ## _name ## _INR__COUNT, \
    .OutCount[IO_Type_Bit]      = IO_ ## _name ## _OUTB__COUNT, \
    .OutCount[IO_Type_Range]    = IO_ ## _name ## _OUTR__COUNT

#define IO_INIT_STATIC_PORT_INFO(_impl,_name) \
    for (unsigned int i = 0; i < IO_ ## _name ## _PORT_COUNT; ++i) { \
    _impl->portInfo[i].inAvailable [IO_Type_Bit]    = \
        IO_ ## _name ## _INB__COUNT; \
    _impl->portInfo[i].inAvailable [IO_Type_Range]  = \
        IO_ ## _name ## _INR__COUNT; \
    _impl->portInfo[i].outAvailable[IO_Type_Bit]    = \
        IO_ ## _name ## _OUTB__COUNT; \
    _impl->portInfo[i].outAvailable[IO_Type_Range]  = \
        IO_ ## _name ## _OUTR__COUNT; }


typedef uint16_t    IO_Count;
typedef uint16_t    IO_Code;
typedef uint16_t    IO_GatewayId;
typedef uint32_t    IO_Port;


enum IO_Type
{
    IO_Type_Bit = 0,
    IO_Type_Range,
    IO_Type__COUNT
};


enum IO_UpdateValue
{
    IO_UpdateValue_Now = 0,
    // Output: deferred update, Input: buffered value
    IO_UpdateValue_Async,
};


struct IO;

typedef void            (* IO_HardwareInitFunc)(struct IO *const Io);
typedef void            (* IO_UpdateFunc)(struct IO *const Io);
typedef uint32_t        (* IO_GetInputFunc)(struct IO *const Io,
                         const enum IO_Type IoType, const IO_Code DriverCode,
                         const IO_Port Port);
typedef void            (* IO_SetOutputFunc)(struct IO *const Io,
                         const enum IO_Type IoType, const IO_Code DriverCode,
                         const IO_Port OutPort, const uint32_t Value);
typedef IO_Code         (* IO_GetAnyInputFunc)(struct IO *const Io,
                         const enum IO_Type IoType, const IO_Port Port);
typedef const char *    (* IO_InputNameFunc)(struct IO *const Io,
                         const enum IO_Type IoType, const IO_Code DriverCode);
typedef const char *    (* IO_OutputNameFunc)(struct IO *const Io,
                         const enum IO_Type IoType, const IO_Code DriverCode);


struct IO_IFACE
{
    const char                      * const Description;
    const IO_Port                   PortCount;
    const IO_Count                  InCount[IO_Type__COUNT];
    const IO_Count                  OutCount[IO_Type__COUNT];
    const IO_HardwareInitFunc       HardwareInit;
    const IO_UpdateFunc             Update;
    const IO_GetInputFunc           GetInput;
    const IO_GetAnyInputFunc        GetAnyInput;
    const IO_SetOutputFunc          SetOutput;
    const IO_InputNameFunc          InputName;
    const IO_OutputNameFunc         OutputName;
};


struct IO_PortInfo
{
    IO_Count                inAvailable[IO_Type__COUNT];
    IO_Count                outAvailable[IO_Type__COUNT];
};


struct IO
{
    const struct IO_IFACE       * iface;
    const struct IO_PortInfo    * portInfo;
    TIMER_Ticks                 deferredUpdatePeriod;
    TIMER_Ticks                 lastDeferredUpdate;
};


struct IO_Gateway
{
    struct IO               * driver;
    IO_Port                 driverPort;
};


void            IO_Init                 (struct IO *const Io,
                                         const struct IO_IFACE *const Iface,
                                         const struct IO_PortInfo *const
                                         PortInfo, const TIMER_Ticks
                                         DeferredUpdatePeriod);
bool            IO_Initialized          (struct IO *const Io);
void            IO_Update               (struct IO *const Io);
IO_Port         IO_PortCount            (struct IO *const Io);
IO_Count        IO_InputCount           (struct IO *const Io,
                                         const enum IO_Type IoType);
IO_Count        IO_OutputCount          (struct IO *const Io,
                                         const enum IO_Type IoType);
IO_Count        IO_AvailableInputs      (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Port Port);
IO_Count        IO_AvailableOutputs     (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Port OutPort);
uint32_t        IO_GetInput             (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode,
                                         const IO_Port Port,
                                         const enum IO_UpdateValue When);
void            IO_SetOutput            (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode,
                                         const IO_Port OutPort,
                                         const uint32_t Value,
                                         const enum IO_UpdateValue When);
IO_Code         IO_GetAnyInput          (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Port Port);
const char *    IO_Description          (struct IO *const Io);
const char *    IO_InputName            (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode);
const char *    IO_OutputName           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode);
