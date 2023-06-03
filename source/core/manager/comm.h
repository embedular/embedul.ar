/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [MANAGER] communication devices manager.

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

#include "embedul.ar/source/core/device/stream.h"


enum COMM_Device
{
    COMM_Device_Log = 0,
    COMM_Device_P2PSerialLocal,
    COMM_Device_P2PSerialExpansion,
    COMM_Device_SerialNetwork,
    COMM_Device_HighSpeedDeviceLocal,
    COMM_Device_HighSpeedDeviceExpansion,
    COMM_Device_LowSpeedLocalBus,
    COMM_Device_LowSpeedExpansionBus,
    COMM_Device_PeripheralBus,
    COMM_Device_IPNetwork,
    COMM_Device_IPNetworkSerialConfig,
    COMM_Device__COUNT
};


struct COMM
{
    struct STREAM   * device[COMM_Device__COUNT];
};


void            COMM_Init                   (struct COMM *const C);
bool            COMM_HasDevice              (const enum COMM_Device ComDevice);
void            COMM_SetDevice              (const enum COMM_Device ComDevice,
                                             struct STREAM *const S);
struct STREAM * COMM_GetDevice              (const enum COMM_Device ComDevice);
const char *    COMM_DeviceRoleDescription  (const enum COMM_Device ComDevice);
