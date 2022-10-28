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

#define OUTPUT_PARAMS(_pname,_ptype,_pid) \
    OUTPUT_PROFILE_Type_ ## _pname, \
    IO_Type_ ## _ptype, \
    OUTPUT_PROFILE_ ## _pname ## _ ## _ptype ## _ ## _pid

#define OUTPUT_MAP(_pname,_ptype,_pid,_drv_index) \
    OUTPUT_Map (OUTPUT_PARAMS(_pname,_ptype,_pid), _drv_index)

#define OUTPUT_MAP_BIT(_pname,_pid,_drv_index) \
    OUTPUT_MAP (_pname,Bit,_pid,_drv_index)

#define OUTPUT_MAP_RANGE(_pname,_pid,_drv_index) \
    OUTPUT_MAP (_pname,Range,_pid,_drv_index)

#define OUTPUT_IS_MAPPED(_pname,_ptype,_pid) \
    OUTPUT_IsMapped (OUTPUT_PARAMS(_pname,_ptype,_pid))

#define OUTPUT_BIT_IS_MAPPED(_pname,_pid) \
    OUTPUT_IS_MAPPED(_pname,Bit,_pid)

#define OUTPUT_RANGE_IS_MAPPED(_pname,_pid) \
    OUTPUT_IS_MAPPED(_pname,Range,_pid)

#define OUTPUT_SET(_pname,_ptype,_when,_pid,_value) \
    OUTPUT_Set ## _when (OUTPUT_PARAMS(_pname,_ptype,_pid),_value)

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
    enum IO_MapAction       currentMapAction;
};


enum OUTPUT_UpdateValue
{
    OUTPUT_UpdateValue_Now = IO_UpdateValue_Now,
    OUTPUT_UpdateValue_Defer = IO_UpdateValue_Async
};


void            OUTPUT_Init             (struct OUTPUT *const O);
void            OUTPUT_RegisterGateway  (struct IO *const Driver,
                                         const IO_Port DriverPort);
void            OUTPUT_CurrentMapAction (const enum IO_MapAction MapAction);
bool            OUTPUT_HasProfile       (const enum OUTPUT_PROFILE_Type
                                         ProfileType);
uint32_t        OUTPUT_ProfileCodes     (const enum OUTPUT_PROFILE_Type
                                         ProfileType,
                                         const enum IO_Type IoType);
bool            OUTPUT_IsMapped         (const enum OUTPUT_PROFILE_Type
                                         ProfileType, const enum IO_Type IoType,
                                         const IO_Code ProfileCode);
void            OUTPUT_Map              (const enum OUTPUT_PROFILE_Type
                                         ProfileType, const enum IO_Type IoType,
                                         const IO_Code ProfileCode,
                                         const IO_Code DriverCode);
void            OUTPUT_Set              (const enum OUTPUT_PROFILE_Type
                                         ProfileType, const enum IO_Type IoType,
                                         const IO_Code ProfileCode,
                                         const IO_Value Value,
                                         const enum OUTPUT_UpdateValue When);
void            OUTPUT_SetNow           (const enum OUTPUT_PROFILE_Type
                                         ProfileType, const enum IO_Type IoType,
                                         const IO_Code ProfileCode,
                                         const IO_Value Value);
void            OUTPUT_SetDefer         (const enum OUTPUT_PROFILE_Type
                                         ProfileType, const enum IO_Type IoType,
                                         const IO_Code ProfileCode,
                                         const IO_Value Value);
const char *    OUTPUT_MappedName       (const enum OUTPUT_PROFILE_Type
                                         ProfileType, const enum IO_Type IoType,
                                         const IO_Code ProfileCode);
void            OUTPUT_Update           (void);
IO_Code         OUTPUT__isMappedByDriver
                                        (const enum OUTPUT_PROFILE_Type
                                         ProfileType, const enum IO_Type IoType,
                                         const IO_Code ProfileCode,
                                         const struct IO *const Driver);
struct IO_ConstGateway
                OUTPUT__getMappedGateway
                                        (const enum OUTPUT_PROFILE_Type
                                         ProfileType, const enum IO_Type IoType,
                                         const IO_Code ProfileCode);
