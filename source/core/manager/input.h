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

#pragma once

#include "embedul.ar/source/core/device/io.h"
#include "embedul.ar/source/core/manager/input/profile/control.h"
#include "embedul.ar/source/core/manager/input/profile/gp1.h"
#include "embedul.ar/source/core/manager/input/profile/gp2.h"
#include "embedul.ar/source/core/manager/input/profile/lightdev.h"
#include "embedul.ar/source/core/manager/input/profile/main.h"
#include <stdint.h>


#define INPUT_MAX_GATEWAYS       LIB_EMBEDULAR_CONFIG_INPUT_MAX_GATEWAYS

#if (LIB_EMBEDULAR_CONFIG_INPUT_ACTION == 1)
    #define INPUT_ACTION(_inbn,_swa) \
            ((INPUT_BitAsSwitchAction(INPUT_Bit_ ## _inbn) == \
                        INPUT_ACTION_Type_ ## _swa)? true : false)
#endif

#define INPUT_PROFILE_TYPE(_pname) \
    INPUT_PROFILE_Type_ ## _pname

#define INPUT_PROFILE_CODE(_pname,_ptype,_pid) \
    INPUT_PROFILE_ ## _pname ## _ ## _ptype ## _ ## _pid

#define INPUT_PARAMS(_pname,_ptype,_pid) \
    INPUT_PROFILE_TYPE(_pname), \
    IO_Type_ ## _ptype, \
    INPUT_PROFILE_CODE(_pname,_ptype,_pid)

#define INPUT_MAP(_pname,_ptype,_pid,_drv_index) \
    INPUT_Map (INPUT_PARAMS(_pname,_ptype,_pid), _drv_index)

#define INPUT_MAP_BIT(_pname,_pid,_drv_index) \
    INPUT_MAP (_pname,Bit,_pid,_drv_index)

#define INPUT_MAP_RANGE(_pname,_pid,_drv_index) \
    INPUT_MAP (_pname,Range,_pid,_drv_index)

#define INPUT_IS_MAPPED(_pname,_ptype,_pid) \
    INPUT_IsMapped (INPUT_PARAMS(_pname,_ptype,_pid))

#define INPUT_BIT_IS_MAPPED(_pname,_pid) \
    INPUT_IS_MAPPED(_pname,Bit,_pid)

#define INPUT_RANGE_IS_MAPPED(_pname,_pid) \
    INPUT_IS_MAPPED(_pname,Range,_pid)

#define INPUT_GET(_pname,_ptype,_when,_pid) \
    INPUT_Get ## _when (INPUT_PARAMS(_pname,_ptype,_pid))

#define INPUT_GET_BIT_NOW(_pname,_pid) \
    INPUT_GET(_pname,Bit,Now,_pid)

#define INPUT_GET_BIT_BUFFERED(_pname,_pid) \
    INPUT_GET(_pname,Bit,Buffer,_pid)

#define INPUT_GET_BIT_ACTION(_pname,_pid) \
    INPUT_GetBitAction (INPUT_PROFILE_TYPE(_pname), \
                        INPUT_PROFILE_CODE(_pname,Bit,_pid))

#define INPUT_CHECK_BIT_ACTION(_pname,_pid,_action) \
    ((INPUT_GET_BIT_ACTION(_pname,_pid) == INPUT_ACTION_Type_ ## _action)? \
        true : false)

#define INPUT_GET_RANGE_NOW(_pname,_pid) \
    INPUT_GET(_pname,Range,Now,_pid)

#define INPUT_GET_RANGE_BUFFER(_pname,_pid) \
    INPUT_GET(_pname,Range,Buffer,_pid)


struct INPUT
{
    struct INPUT_PROFILE    profiles    [INPUT_PROFILE_Type__COUNT];
    struct IO_Gateway       gateways    [INPUT_MAX_GATEWAYS];
    IO_GatewayId            nextGatewayId;
    enum IO_MapAction       currentMapAction;
};


enum INPUT_UpdateValue
{
    INPUT_UpdateValue_Now = IO_UpdateValue_Now,
    INPUT_UpdateValue_Buffer = IO_UpdateValue_Async
};


void            INPUT_Init              (struct INPUT *const I);
void            INPUT_RegisterGateway   (struct IO *const Driver,
                                         const IO_Port DriverPort);
void            INPUT_CurrentMapAction  (const enum IO_MapAction MapAction);
bool            INPUT_HasProfile        (const enum INPUT_PROFILE_Type
                                         ProfileType);
uint32_t        INPUT_ProfileCodes      (const enum INPUT_PROFILE_Type
                                         ProfileType,
                                         const enum IO_Type IoType);
bool            INPUT_IsMapped          (const enum INPUT_PROFILE_Type
                                         ProfileType, const enum IO_Type IoType,
                                         const IO_Code ProfileCode);
void            INPUT_Map               (const enum INPUT_PROFILE_Type
                                         ProfileType, const enum IO_Type IoType,
                                         const IO_Code ProfileCode,
                                         const IO_Code DriverCode);
bool            INPUT_MapBitByOnState   (const enum INPUT_PROFILE_Type
                                         ProfileType,
                                         const IO_Code ProfileCode);
IO_Value        INPUT_Get               (const enum INPUT_PROFILE_Type
                                         ProfileType, const enum IO_Type IoType,
                                         const IO_Code ProfileCode,
                                         const enum INPUT_UpdateValue When);
IO_Value        INPUT_GetNow            (const enum INPUT_PROFILE_Type
                                         ProfileType, const enum IO_Type IoType, 
                                         const IO_Code ProfileCode);
IO_Value        INPUT_GetBuffered       (const enum INPUT_PROFILE_Type
                                         ProfileType, const enum IO_Type IoType,
                                         const IO_Code ProfileCode);
#if (LIB_EMBEDULAR_CONFIG_INPUT_ACTION == 1)
enum INPUT_ACTION_Type
                INPUT_GetBitAction      (const enum INPUT_PROFILE_Type
                                         ProfileType,
                                         const IO_Code ProfileCode);
#endif
const char *    INPUT_MappedName        (const enum INPUT_PROFILE_Type
                                         ProfileType, const enum IO_Type IoType,
                                         const IO_Code ProfileCode);
bool            INPUT_AnyBit            (const enum INPUT_PROFILE_SelectFlag
                                         Profiles);
void            INPUT_Update            (void);
IO_Code         INPUT__isMappedByDriver (const enum INPUT_PROFILE_Type
                                         ProfileType, const enum IO_Type IoType,
                                         const IO_Code ProfileCode,
                                         const struct IO *const Driver);
struct IO_ConstGateway
                INPUT__getMappedGateway (const enum INPUT_PROFILE_Type
                                         ProfileType, const enum IO_Type IoType,
                                         const IO_Code ProfileCode);
