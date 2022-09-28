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


#define INPUT_MAX_DEVICES       LIB_EMBEDULAR_CONFIG_INPUT_MAX_DEVICES

#if (LIB_EMBEDULAR_CONFIG_INPUT_ACTION == 1)
#define INPUT_ACTION(_inbn,_swa) \
            ((INPUT_BitAsSwitchAction(INPUT_Bit_ ## _inbn) == \
                        INPUT_ACTION_Type_ ## _swa)? true : false)
#endif

#define INPUT_MAP(_pname,_ptype,_pid,_drv_index) \
    INPUT_Map ## _ptype (INPUT_PROFILE_Type_ ## _pname, \
                    INPUT_PROFILE_ ## _pname ## _ ## _ptype ## _ ## _pid, \
                    _drv_index)

#define INPUT_MAP_BIT(_pname,_pid,_drv_index) \
    INPUT_MAP (_pname,Bit,_pid,_drv_index)

#define INPUT_MAP_RANGE(_pname,_pid,_drv_index) \
    INPUT_MAP (_pname,Range,_pid,_drv_index)

#define INPUT_IS_MAPPED(_pname,_ptype,_pid) \
    INPUT_ ## _ptype ## IsMapped (INPUT_PROFILE_Type_ ## _pname, \
                    INPUT_PROFILE_ ## _pname ## _Bit_ ## _pid)

#define INPUT_BIT_IS_MAPPED(_pname,_pid) \
    INPUT_IS_MAPPED(_pname,Bit,_pid)

#define INPUT_RANGE_IS_MAPPED(_pname,_pid) \
    INPUT_IS_MAPPED(_pname,Range,_pid)

#define INPUT_GET(_pname,_ptype,_update,_pid) \
    INPUT_Get ## _ptype ## _update (INPUT_PROFILE_Type_ ## _pname, \
                    INPUT_PROFILE_ ## _pname ## _Bit_ ## _pid)

#define INPUT_GET_BIT_NOW(_pname,_pid) \
    INPUT_GET(_pname,Bit,Now,_pid)

#define INPUT_GET_BIT_BUFFER(_pname,_pid) \
    INPUT_GET(_pname,Bit,Buffer,_pid)

#define INPUT_GET_BIT_ACTION(_pname,_pid) \
    INPUT_GET(_pname,Bit,Action,_pid)

#define INPUT_CHECK_BIT_ACTION(_pname,_pid,_action) \
    ((INPUT_GET_BIT_ACTION(_pname,_pid) == _action)? true : false)

#define INPUT_GET_RANGE_NOW(_pname,_pid) \
    INPUT_GET(_pname,Range,Now,_pid)

#define INPUT_GET_RANGE_BUFFER(_pname,_pid) \
    INPUT_GET(_pname,Range,Buffer,_pid)


// Device = Driver + Driver Source
struct INPUT_Device
{
    struct IO               * driver;
    uint32_t                driverSource;
};


struct INPUT
{
    struct INPUT_Device     device      [INPUT_MAX_DEVICES];
    struct INPUT_PROFILE    profiles    [INPUT_PROFILE_Type__COUNT];
    uint16_t                nextDeviceId;
};


enum INPUT_UpdateValue
{
    INPUT_UpdateValue_Now = 0,
    INPUT_UpdateValue_Buffer
};


void            INPUT_Init                  (struct INPUT *const I);
void            INPUT_SetDevice             (struct IO *const Driver,
                                             const uint32_t DriverSource);
bool            INPUT_HasProfile            (const enum INPUT_PROFILE_Type
                                             ProfileType);
uint32_t        INPUT_ProfileBits           (const enum INPUT_PROFILE_Type
                                             ProfileType);
uint32_t        INPUT_ProfileRanges         (const enum INPUT_PROFILE_Type
                                             ProfileType);
void            INPUT_MapBit                (const enum INPUT_PROFILE_Type
                                             ProfileType, const uint32_t Inb,
                                             const uint16_t DriverIndex);
void            INPUT_MapRange              (const enum INPUT_PROFILE_Type
                                             ProfileType, const uint32_t Inr,
                                             const uint16_t DriverIndex);
bool            INPUT_MapBitByOnState       (const enum INPUT_PROFILE_Type
                                             ProfileType, const uint32_t Inb);
uint32_t        INPUT_GetBit                (const enum INPUT_PROFILE_Type
                                             ProfileType, const uint32_t Inb,
                                             const enum INPUT_UpdateValue When);
uint32_t        INPUT_GetBitNow             (const enum INPUT_PROFILE_Type
                                             ProfileType, const uint32_t Inb);
uint32_t        INPUT_GetBitBuffer          (const enum INPUT_PROFILE_Type
                                             ProfileType, const uint32_t Inb);
#if (LIB_EMBEDULAR_CONFIG_INPUT_ACTION == 1)
enum INPUT_ACTION_Type
                INPUT_GetBitAction          (const enum INPUT_PROFILE_Type
                                             ProfileType, const uint32_t Inb);
#endif
uint32_t        INPUT_GetRange              (const enum INPUT_PROFILE_Type
                                             ProfileType, const uint32_t Inr,
                                             const enum INPUT_UpdateValue When);
uint32_t        INPUT_GetRangeNow           (const enum INPUT_PROFILE_Type
                                             ProfileType, const uint32_t Inr);
uint32_t        INPUT_GetRangeBuffer        (const enum INPUT_PROFILE_Type
                                             ProfileType, const uint32_t Inr);
const char *    INPUT_MappedBitName         (const enum INPUT_PROFILE_Type
                                             ProfileType, const uint32_t Inb);
const char *    INPUT_MappedRangeName       (const enum INPUT_PROFILE_Type
                                             ProfileType, const uint32_t Inr);
bool            INPUT_IsBitMapped           (const enum INPUT_PROFILE_Type
                                             ProfileType, const uint32_t Inb);
bool            INPUT_IsRangeMapped         (const enum INPUT_PROFILE_Type
                                             ProfileType, const uint32_t Inr);
bool            INPUT_AnyBit                (const enum INPUT_PROFILE_SelectFlag
                                             Profiles);
void            INPUT_Update                (void);
