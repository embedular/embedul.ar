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

#pragma once

#include "embedul.ar/source/core/device/io.h"
#include "embedul.ar/source/core/manager/output/profile/control.h"
#include "embedul.ar/source/core/manager/output/profile/lightdev.h"
#include "embedul.ar/source/core/manager/output/profile/marquee.h"
#include "embedul.ar/source/core/manager/output/profile/sign.h"
#include <stdint.h>


#define OUTPUT_MAX_GATEWAYS      LIB_EMBEDULAR_CONFIG_OUTPUT_MAX_GATEWAYS

#define OUTPUT_MAP(_pname,_ptype,_pid,_drv_index) \
    OUTPUT_Map ## _ptype (OUTPUT_PROFILE_Type_ ## _pname, \
                    OUTPUT_PROFILE_ ## _pname ## _ ## _ptype ## _ ## _pid, \
                    _drv_index)

#define OUTPUT_MAP_BIT(_pname,_pid,_drv_index) \
    OUTPUT_MAP (_pname,Bit,_pid,_drv_index)

#define OUTPUT_MAP_RANGE(_pname,_pid,_drv_index) \
    OUTPUT_MAP (_pname,Range,_pid,_drv_index)

#define OUTPUT_BIT_IS_MAPPED(_pname,_pid) \
    OUTPUT_IS_MAPPED(_pname,Bit,_pid)

#define OUTPUT_RANGE_IS_MAPPED(_pname,_pid) \
    OUTPUT_IS_MAPPED(_pname,Range,_pid)

#define OUTPUT_SET(_pname,_ptype,_update,_pid,_value) \
    OUTPUT_Set ## _ptype ## _update (OUTPUT_PROFILE_Type_ ## _pname, \
                    OUTPUT_PROFILE_ ## _pname ## _ ## _ptype ## _ ## _pid, \
                    _value)

#define OUTPUT_SET_BIT_NOW(_pname,_pid,_value) \
    OUTPUT_SET(_pname,Bit,Now,_pid,_value)

#define OUTPUT_SET_BIT_DEFER(_pname,_pid,_value) \
    OUTPUT_SET(_pname,Bit,Defer,_pid,_value)

#define OUTPUT_SET_RANGE_NOW(_pname,_pid,_value) \
    OUTPUT_SET(_pname,Range,Now,_pid,_value)

#define OUTPUT_SET_RANGE_DEFER(_pname,_pid,_value) \
    OUTPUT_SET(_pname,Range,Defer,_pid,_value)


struct OUTPUT
{
    struct OUTPUT_PROFILE   profiles    [OUTPUT_PROFILE_Type__COUNT];
    struct IO_Gateway       gateways    [OUTPUT_MAX_GATEWAYS];
    IO_GatewayId            nextGatewayId;
};


enum OUTPUT_UpdateValue
{
    OUTPUT_UpdateValue_Now = 0,
    OUTPUT_UpdateValue_Defer
};


void            OUTPUT_Init             (struct OUTPUT *const O);
void            OUTPUT_SetGateway       (struct IO *const Driver,
                                         const IO_Port DriverPort);
bool            OUTPUT_HasProfile       (const enum OUTPUT_PROFILE_Type
                                         ProfileType);
uint32_t        OUTPUT_ProfileBits      (const enum OUTPUT_PROFILE_Type
                                         ProfileType);
uint32_t        OUTPUT_ProfileRanges    (const enum OUTPUT_PROFILE_Type
                                         ProfileType);
void            OUTPUT_MapBit           (const enum OUTPUT_PROFILE_Type
                                         ProfileType, const IO_Code ProfileCode,
                                         const IO_Code DriverCode);
void            OUTPUT_MapRange         (const enum OUTPUT_PROFILE_Type
                                         ProfileType, const IO_Code ProfileCode,
                                         const IO_Code DriverCode);
void            OUTPUT_SetBit           (const enum OUTPUT_PROFILE_Type
                                         ProfileType, const IO_Code ProfileCode,
                                         const uint32_t Value,
                                         const enum OUTPUT_UpdateValue When);
void            OUTPUT_SetBitNow        (const enum OUTPUT_PROFILE_Type
                                         ProfileType, const IO_Code ProfileCode,
                                         const uint32_t Value);
void            OUTPUT_SetBitDefer      (const enum OUTPUT_PROFILE_Type
                                         ProfileType, const IO_Code ProfileCode,
                                         const uint32_t Value);
void            OUTPUT_SetRange         (const enum OUTPUT_PROFILE_Type
                                         ProfileType, const IO_Code ProfileCode,
                                         const uint32_t Value,
                                         const enum OUTPUT_UpdateValue When);
void            OUTPUT_SetRangeNow      (const enum OUTPUT_PROFILE_Type
                                         ProfileType, const IO_Code ProfileCode,
                                         const uint32_t Value);
void            OUTPUT_SetRangeDefer    (const enum OUTPUT_PROFILE_Type
                                         ProfileType, const IO_Code ProfileCode,
                                         const uint32_t Value);
const char *    OUTPUT_MappedBitName    (const enum OUTPUT_PROFILE_Type
                                         ProfileType,
                                         const IO_Code ProfileCode);
const char *    OUTPUT_MappedRangeName  (const enum OUTPUT_PROFILE_Type
                                         ProfileType, 
                                         const IO_Code ProfileCode);
void            OUTPUT_Update           (void);
