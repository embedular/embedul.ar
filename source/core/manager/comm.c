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


static const char * s_CommStreamName[COMM_Stream__COUNT] =
{
    "log messages",
    "p2p serial local",
    "p2p serial expansion",
    "ip net. serial config"
};


static const char * s_CommPacketName[COMM_Packet__COUNT] =
{
    "serial network",
    "high speed local",
    "high speed expansion",
    "low speed local bus",
    "low speed expansion bus",
    "peripheral bus",
    "ip network"
};


void COMM_Init (struct COMM *const C)
{
    BOARD_AssertState  (!s_p);
    BOARD_AssertParams (C);

    OBJECT_Clear (C);

    LOG_ContextBegin (C, LANG_INIT);
    {
        s_p = C;

        LOG_Items (2,
                    LANG_STREAM_DEVICE_TYPES,   (uint32_t)COMM_Stream__COUNT,
                    LANG_PACKET_DEVICE_TYPES,   (uint32_t)COMM_Packet__COUNT);
    }
    LOG_ContextEnd ();
}


bool COMM_HasStream (const enum COMM_Stream Stream)
{
    return s_p->stream[Stream]? true : false;
}


bool COMM_HasPacket (const enum COMM_Packet Packet)
{
    return s_p->packet[Packet]? true : false;
}


void COMM_SetStream (const enum COMM_Stream Stream, struct STREAM *const S)
{
    BOARD_AssertParams (STREAM_IsValid(S));

    if (COMM_HasStream(Stream))
    {
        LOG_Warn (s_p, LANG_MANAGER_DRIVER_OVERWRITE);
        LOG_Items (3,
                    LANG_ROLE,  COMM_StreamRoleDescription(Stream),
                    LANG_SET,   STREAM_Description(s_p->stream[Stream]),
                    LANG_NEW,   STREAM_Description(S));
    }

    s_p->stream[Stream] = S;
}


void COMM_SetPacket (const enum COMM_Packet Packet, struct PACKET *const P)
{
    BOARD_AssertParams (PACKET_IsValid(P));

    if (COMM_HasPacket(Packet))
    {
        LOG_Warn (s_p, LANG_MANAGER_DRIVER_OVERWRITE);
        LOG_Items (3,
                    LANG_ROLE,  COMM_PacketRoleDescription(Packet),
                    LANG_SET,   PACKET_Description(s_p->packet[Packet]),
                    LANG_NEW,   PACKET_Description(P));
    }

    s_p->packet[Packet] = P;
}


struct STREAM * COMM_GetStream (const enum COMM_Stream Stream)
{
    BOARD_AssertParams (COMM_HasStream(Stream));
    return s_p->stream[Stream];
}


struct PACKET * COMM_GetPacket (const enum COMM_Packet Packet)
{
    BOARD_AssertParams (COMM_HasPacket(Packet));
    return s_p->packet[Packet];
}


const char * COMM_StreamRoleDescription (const enum COMM_Stream Stream)
{
    BOARD_AssertParams (Stream < COMM_Stream__COUNT);
    return s_CommStreamName[Stream];
}


const char * COMM_PacketRoleDescription (const enum COMM_Packet Packet)
{
    BOARD_AssertParams (Packet < COMM_Packet__COUNT);
    return s_CommPacketName[Packet];
}
