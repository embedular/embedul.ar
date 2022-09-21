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
#include "embedul.ar/source/core/device/packet.h"


enum COMM_Stream
{
    COMM_Stream_Log = 0,
    COMM_Stream_P2PSerialLocal,
    COMM_Stream_P2PSerialExpansion,
    COMM_Stream_IPNetworkSerialConfig,
    COMM_Stream__COUNT
};


enum COMM_Packet
{
    COMM_Packet_SerialNetwork = 0,
    COMM_Packet_HighSpeedDeviceLocal,
    COMM_Packet_HighSpeedDeviceExpansion,
    COMM_Packet_LowSpeedLocalBus,
    COMM_Packet_LowSpeedExpansionBus,
    COMM_Packet_PeripheralBus,
    COMM_Packet_IPNetwork,
    COMM_Packet__COUNT
};


struct COMM
{
    struct STREAM       * stream[COMM_Stream__COUNT];
    struct PACKET       * packet[COMM_Packet__COUNT];
};


void            COMM_Init                   (struct COMM *const C);
bool            COMM_HasStream              (const enum COMM_Stream Stream);
bool            COMM_HasPacket              (const enum COMM_Packet Packet);
void            COMM_SetStream              (const enum COMM_Stream Stream,
                                             struct STREAM *const S);
void            COMM_SetPacket              (const enum COMM_Packet Packet,
                                             struct PACKET *const P);
struct STREAM * COMM_GetStream              (const enum COMM_Stream Stream);
struct PACKET * COMM_GetPacket              (const enum COMM_Packet Packet);
const char *    COMM_StreamRoleDescription  (const enum COMM_Stream Stream);
const char *    COMM_PacketRoleDescription  (const enum COMM_Packet Packet);
