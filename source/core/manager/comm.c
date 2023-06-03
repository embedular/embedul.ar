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

#include "embedul.ar/source/core/manager/comm.h"
#include "embedul.ar/source/core/device/board.h"


static struct COMM * s_p = NULL;


static const char * s_CommDeviceName[COMM_Device__COUNT] =
{
    [COMM_Device_Log]                       = "Log messages",
    [COMM_Device_P2PSerialLocal]            = "P2P serial local",
    [COMM_Device_P2PSerialExpansion]        = "P2P serial expansion",
    [COMM_Device_SerialNetwork]             = "Serial network",
    [COMM_Device_HighSpeedDeviceLocal]      = "High speed local",
    [COMM_Device_HighSpeedDeviceExpansion]  = "High speed expansion",
    [COMM_Device_LowSpeedLocalBus]          = "Low speed local bus",
    [COMM_Device_LowSpeedExpansionBus]      = "Low speed expansion bus",
    [COMM_Device_PeripheralBus]             = "Peripheral bus",
    [COMM_Device_IPNetwork]                 = "IP network",
    [COMM_Device_IPNetworkSerialConfig]     = "IP net. serial config"
};


void COMM_Init (struct COMM *const C)
{
    BOARD_AssertState  (!s_p);
    BOARD_AssertParams (C);

    OBJECT_Clear (C);

    {
        LOG_AutoContext (C, LANG_INIT);

        s_p = C;

        LOG_Items (1, LANG_MAX_DEVICES, (uint32_t)COMM_Device__COUNT);
    }
}


bool COMM_HasDevice (const enum COMM_Device ComDevice)
{
    return s_p->device[ComDevice]? true : false;
}


void COMM_SetDevice (const enum COMM_Device ComDevice, struct STREAM *const S)
{
    BOARD_AssertParams (STREAM_IsValid(S));

    if (COMM_HasDevice(ComDevice))
    {
        LOG_Warn (s_p, LANG_MANAGER_DRIVER_OVERWRITE);
        LOG_Items (3,
                    LANG_ROLE,  COMM_DeviceRoleDescription(ComDevice),
                    LANG_SET,   STREAM_Description(s_p->device[ComDevice]),
                    LANG_NEW,   STREAM_Description(S));
    }

    s_p->device[ComDevice] = S;
}


struct STREAM * COMM_GetDevice (const enum COMM_Device ComDevice)
{
    BOARD_AssertParams (COMM_HasDevice(ComDevice));
    return s_p->device[ComDevice];
}


const char * COMM_DeviceRoleDescription (const enum COMM_Device ComDevice)
{
    BOARD_AssertParams (ComDevice < COMM_Device__COUNT);
    return s_CommDeviceName[ComDevice];
}


const char * COMM_PacketRoleDescription (const enum COMM_Device ComDevice)
{
    BOARD_AssertParams (ComDevice < COMM_Device__COUNT);
    return s_CommDeviceName[ComDevice];
}
