/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [MANAGER] input/output devices manager.

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
#include "embedul.ar/source/core/misc/input/profile/control.h"
#include "embedul.ar/source/core/misc/input/profile/gp1.h"
#include "embedul.ar/source/core/misc/input/profile/gp2.h"
#include "embedul.ar/source/core/misc/input/profile/lightdev.h"
#include "embedul.ar/source/core/misc/input/profile/main.h"
#include "embedul.ar/source/core/misc/output/profile/control.h"
#include "embedul.ar/source/core/misc/output/profile/lightdev.h"
#include "embedul.ar/source/core/misc/output/profile/marquee.h"
#include "embedul.ar/source/core/misc/output/profile/sign.h"
#include <stdint.h>


#define MIO_INPUT_MAX_GATEWAYS      LIB_EMBEDULAR_CONFIG_INPUT_MAX_GATEWAYS
#define MIO_OUTPUT_MAX_GATEWAYS     LIB_EMBEDULAR_CONFIG_OUTPUT_MAX_GATEWAYS

/*
    _io: OUTPUT, INPUT
    _pname: profile group name
    _ptype: Bit, Range.
    _pcode: profile group code
*/
#define MIO_PROFILE_PARAMS(_io,_pname,_ptype,_pcode) \
    _io ## _PROFILE_Group_ ## _pname, \
    IO_Type_ ## _ptype, \
    _io ## _PROFILE_ ## _pname ## _ ## _ptype ## _ ## _pcode


#define MIO_MAP_INPUT(_pname,_ptype,_pcode,_drv_index) \
    MIO_Map (MIO_Dir_Input, \
             MIO_PROFILE_PARAMS(INPUT,_pname,_ptype,_pcode), _drv_index)

#define MIO_MAP_INPUT_BIT(_pname,_pcode,_drv_index) \
    MIO_MAP_INPUT(_pname,Bit,_pcode,_drv_index)

#define MIO_MAP_INPUT_RANGE(_pname,_pcode,_drv_index) \
    MIO_MAP_INPUT(_pname,Range,_pcode,_drv_index)


#define MIO_MAP_OUTPUT(_pname,_ptype,_pcode,_drv_index) \
    MIO_Map (MIO_Dir_Output, \
             MIO_PROFILE_PARAMS(OUTPUT,_pname,_ptype,_pcode), _drv_index)

#define MIO_MAP_OUTPUT_BIT(_pname,_pcode,_drv_index) \
    MIO_MAP_OUTPUT(_pname,Bit,_pcode,_drv_index)

#define MIO_MAP_OUTPUT_RANGE(_pname,_pcode,_drv_index) \
    MIO_MAP_OUTPUT(_pname,Range,_pcode,_drv_index)


#define MIO_IS_INPUT_MAPPED(_pname,_ptype,_pcode) \
    MIO_IsMapped (MIO_Dir_Input, \
                  MIO_PROFILE_PARAMS(INPUT,_pname,_ptype,_pcode))

#define MIO_IS_INPUT_BIT_MAPPED(_pname,_pcode) \
    MIO_IS_INPUT_MAPPED(_pname,Bit,_pcode)

#define MIO_IS_INPUT_RANGE_MAPPED(_pname,_pcode) \
    MIO_IS_INPUT_MAPPED(_pname,Range,_pcode)


#define MIO_IS_OUTPUT_MAPPED(_pname,_ptype,_pcode) \
    MIO_IsMapped (MIO_Dir_Output, \
                  MIO_PROFILE_PARAMS(OUTPUT,_pname,_ptype,_pcode))

#define MIO_IS_OUTPUT_BIT_MAPPED(_pname,_pcode) \
    MIO_IS_OUTPUT_MAPPED(_pname,Bit,_pcode)

#define MIO_IS_OUTPUT_RANGE_MAPPED(_pname,_pcode) \
    MIO_IS_OUTPUT_MAPPED(_pname,Range,_pcode)


#define MIO_GET_INPUT(_pname,_ptype,_pcode,_when) \
    MIO_GetInput (MIO_PROFILE_PARAMS(OUTPUT,_pname,_ptype,_pcode), \
                  MIO_OutUpdateValue_ ## _when)

#define MIO_GET_INPUT_BIT_NOW(_pname,_pcode) \
    MIO_GET_INPUT(_pname,Bit,_pcode,Now)

#define MIO_GET_INPUT_BIT_BUFFER(_pname,_pcode) \
    MIO_GET_INPUT(_pname,Bit,_pcode,Buffer)

#define MIO_GET_INPUT_RANGE_NOW(_pname,_pcode) \
    MIO_GET_INPUT(_pname,Range,_pcode,Now)

#define MIO_GET_INPUT_RANGE_BUFFER(_pname,_pcode) \
    MIO_GET_INPUT(_pname,Range,_pcode,Buffer)

#if (LIB_EMBEDULAR_CONFIG_INPUT_ACTION == 1)
    #define MIO_GET_INPUT_BIT_ACTION(_pname,_pcode) \
        MIO_GetInputBitAction (INPUT_PROFILE_Group ## _pname, \
                               INPUT_PROFILE_ ## _pname ## _Bit_ ## _pcode)

    #define MIO_CHECK_INPUT_BIT_ACTION(_pname,_pcode,_action) \
        ((MIO_GET_INPUT_BIT_ACTION(_pname,_pcode) == \
            INPUT_ACTION_Type_ ## _action)? true : false)
#endif


#define MIO_SET_OUTPUT(_pname,_ptype,_pcode,_when,_value) \
    MIO_SetOutput (MIO_PROFILE_PARAMS(OUTPUT,_pname,_ptype,_pcode), \
                   MIO_OutUpdateValue_ ## _when,_value)

#define MIO_SET_OUTPUT_BIT_NOW(_pname,_pcode,_value) \
    MIO_SET_OUTPUT(_pname,Bit,_pcode,Now,_value)

#define MIO_SET_OUTPUT_BIT_DEFER(_pname,_pcode,_value) \
    MIO_SET_OUTPUT(_pname,Bit,_pcode,Defer,_value)

#define MIO_SET_OUTPUT_RANGE_NOW(_pname,_pcode,_value) \
    MIO_SET_OUTPUT(_pname,Range,_pcode,Now,_value)

#define MIO_SET_OUTPUT_RANGE_DEFER(_pname,_pcode,_value) \
    MIO_SET_OUTPUT(_pname,Range,_pcode,Defer,_value)


enum MIO_Dir
{
    MIO_Dir_Input,
    MIO_Dir_Output,
    MIO_Dir__COUNT
};


enum MIO_InUpdateValue
{
    MIO_InUpdateValue_Now = IO_UpdateValue_Now,
    MIO_InUpdateValue_Buffer = IO_UpdateValue_Async
};


enum MIO_OutUpdateValue
{
    MIO_OutUpdateValue_Now = IO_UpdateValue_Now,
    MIO_OutUpdateValue_Defer = IO_UpdateValue_Async
};


struct MIO
{
    struct IO_PROFILE       inProfiles          [INPUT_PROFILE_Group__COUNT];
    // inBitActions pointers exists but NULL when
    // LIB_EMBEDULAR_CONFIG_INPUT_ACTION == 1.
    struct INPUT_ACTION     * inBitActions      [INPUT_PROFILE_Group__COUNT];
    struct IO_PROFILE       outProfiles         [OUTPUT_PROFILE_Group__COUNT];
    struct IO_PROFILE       * profiles          [MIO_Dir__COUNT];
    struct IO_Gateway       inGateway           [MIO_INPUT_MAX_GATEWAYS];
    struct IO_Gateway       outGateway          [MIO_OUTPUT_MAX_GATEWAYS];
    struct IO_Gateway       * gateways          [MIO_Dir__COUNT];
    IO_GatewayId            nextGatewayId       [MIO_Dir__COUNT];
    enum IO_MapAction       currentMapAction    [MIO_Dir__COUNT];
};


void        MIO_Init                (struct MIO *const M);
const char *
            MIO_DirName             (const enum MIO_Dir Dir); 
void        MIO_RegisterGateway     (const enum MIO_Dir Dir,
                                     struct IO *const Driver,
                                     const IO_Port DriverPort);
void        MIO_CurrentMapAction    (const enum MIO_Dir Dir,
                                     const enum IO_MapAction MapAction);
bool        MIO_HasProfile          (const enum MIO_Dir Dir,
                                     const IO_PROFILE_Group ProfileGroup);
uint32_t    MIO_ProfileCodes        (const enum MIO_Dir Dir,
                                     const IO_PROFILE_Group ProfileGroup,
                                     const enum IO_Type IoType);
bool        MIO_IsMapped            (const enum MIO_Dir Dir,
                                     const IO_PROFILE_Group ProfileGroup,
                                     const enum IO_Type IoType,
                                     const IO_Code ProfileCode);
void        MIO_Map                 (const enum MIO_Dir Dir,
                                     const IO_PROFILE_Group ProfileGroup,
                                     const enum IO_Type IoType,
                                     const IO_Code ProfileCode,
                                     const IO_Code DriverCode);
bool        MIO_AutoMapInputBit     (const enum INPUT_PROFILE_Group
                                     InProfileGroup,
                                     const IO_Code InProfileCode);
IO_Value    MIO_GetInput            (const enum INPUT_PROFILE_Group
                                     InProfileGroup, const enum IO_Type IoType,
                                     const IO_Code InProfileCode,
                                     const enum MIO_InUpdateValue When);
IO_Value    MIO_GetInputNow         (const enum INPUT_PROFILE_Group
                                     InProfileGroup, const enum IO_Type IoType,
                                     const IO_Code InProfileCode);
IO_Value    MIO_GetInputBuffer      (const enum INPUT_PROFILE_Group
                                     InProfileType, const enum IO_Type IoType,
                                     const IO_Code InProfileCode);
#if (LIB_EMBEDULAR_CONFIG_INPUT_ACTION == 1)
enum INPUT_ACTION_Type

            MIO_GetInputBitAction   (const enum INPUT_PROFILE_Group
                                     InProfileGroup,
                                     const IO_Code InProfileCode);
#endif
bool        MIO_GetAnyInputBit      (const enum INPUT_PROFILE_SelectFlag
                                     InProfiles);
void        MIO_SetOutput           (const enum OUTPUT_PROFILE_Group
                                     OutProfileGroup, const enum IO_Type IoType,
                                     const IO_Code OutProfileCode,
                                     const enum MIO_OutUpdateValue When,
                                     const IO_Value Value);
void        MIO_SetOutputNow        (const enum OUTPUT_PROFILE_Group
                                     OutProfileGroup, const enum IO_Type IoType,
                                     const IO_Code OutProfileCode,
                                     const IO_Value Value);
void        MIO_SetOutputDeferred   (const enum OUTPUT_PROFILE_Group
                                     OutProfileGroup, const enum IO_Type IoType,
                                     const IO_Code OutProfileCode,
                                     const IO_Value Value);
const char *
            MIO_MappedName          (const enum MIO_Dir Dir,
                                     const IO_PROFILE_Group ProfileGroup,
                                     const enum IO_Type IoType,
                                     const IO_Code ProfileCode);
void        MIO_Update              (void);

IO_PROFILE_Group
            MIO__profileGroupCount  (const enum MIO_Dir Dir);
const char *
            MIO__profileGroupName   (const enum MIO_Dir Dir,
                                     const IO_PROFILE_Group ProfileGroup);
uint32_t    MIO__maxGateways        (const enum MIO_Dir Dir);
IO_Code     MIO__isMappedByDriver   (const enum MIO_Dir Dir,
                                     const IO_PROFILE_Group ProfileGroup,
                                     const enum IO_Type IoType,
                                     const IO_Code ProfileCode,
                                     const struct IO *const Driver);
struct IO_ConstGateway
            MIO__getMappedGateway   (const enum MIO_Dir Dir,
                                     const IO_PROFILE_Group ProfileGroup,
                                     const enum IO_Type IoType,
                                     const IO_Code ProfileCode);
