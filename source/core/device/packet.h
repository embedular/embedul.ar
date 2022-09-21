/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [PACKET] packetized message device driver interface.

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

#include "embedul.ar/source/core/variant.h"
#include "embedul.ar/source/core/timer.h"


struct PACKET;


struct PACKET_BidirResult
{
    uint32_t    sentOctets;
    uint32_t    recvOctets;
};


typedef void        (* PACKET_HardwareInitFunc)(struct PACKET *const P);
typedef enum DEVICE_CommandResult
                    (* PACKET_CommandFunc)(struct PACKET *const P, 
                                           const char *const Name,
                                           struct VARIANT *const Value);
typedef uint32_t    (* PACKET_SendFunc)(struct PACKET *const P,
                                        const uint8_t *const Data,
                                        const uint32_t Octets);
typedef uint32_t    (* PACKET_RecvSizeFunc)(struct PACKET *const P,
                                        const uint32_t AvailableOctets);
typedef uint32_t    (* PACKET_RecvFunc)(struct PACKET *const P,
                                        uint8_t *const Buffer, 
                                        const uint32_t Octets);
typedef struct PACKET_BidirResult
                    (* PACKET_BidirFunc)(struct PACKET *const P,
                                         const uint8_t *const SendData,
                                         const uint32_t SendOctets,
                                         uint8_t *const RecvData,
                                         const uint32_t RecvOctets);


struct PACKET_IFACE
{
    const char                      *const Description;
    const PACKET_HardwareInitFunc   HardwareInit;
    const PACKET_CommandFunc        Command;
    const PACKET_SendFunc           Send;
    const PACKET_RecvSizeFunc       RecvSize;
    const PACKET_RecvFunc           Recv;
    const PACKET_BidirFunc          Bidir;
};


enum PACKET_Errors
{
    PACKET_Errors_None   = 0,
    PACKET_Errors_Unknown,
    PACKET_Errors_UnsupportedInterface,
    PACKET_Errors_NoPacketSize,
    PACKET_Errors_Timedout,
    PACKET_Errors_NoAck,
    PACKET_Errors_Bus,
    PACKET_Errors_TargetNoAck,
    PACKET_Errors_ArbitrationLost
};



struct PACKET
{
    const struct PACKET_IFACE       * iface;
    TIMER_Ticks                     sendTimeout;
    TIMER_Ticks                     recvTimeout;
    enum PACKET_Errors              error;
    uint32_t                        iteration;
    uint32_t                        recvSize;
    uint32_t                        sentOctets;
    uint32_t                        recvOctets;
    struct VARIANT                  sendTo;
    struct VARIANT                  recvFrom;
};


void            PACKET_Init                 (struct PACKET *const P,
                                             const struct PACKET_IFACE *const
                                             Iface);
bool            PACKET_IsValid              (struct PACKET *const P);
enum DEVICE_CommandResult   
                PACKET_Command              (struct PACKET *const P,
                                             const char *const Name,
                                             struct VARIANT *const Value);
void            PACKET_SendTo               (struct PACKET *const P,
                                             struct VARIANT *const To);
void            PACKET_SendTimeout          (struct PACKET *const P,
                                             const TIMER_Ticks Ticks);
void            PACKET_Send                 (struct PACKET *const P,
                                             const uint8_t *const Data,
                                             const uint32_t Octets);
uint32_t        PACKET_SentCount            (struct PACKET *const P);
void            PACKET_RecvTimeout          (struct PACKET *const P,
                                             const TIMER_Ticks Ticks);
void            PACKET_ClearRecvSize        (struct PACKET *const P);
uint32_t        PACKET_PeekRecvSize         (struct PACKET *const P,
                                             const uint32_t MaxOctets);
void            PACKET_Recv                 (struct PACKET *const P,
                                             uint8_t *const Buffer,
                                             const uint32_t Octets);
uint32_t        PACKET_RecvCount            (struct PACKET *const P);
void            PACKET_RecvFrom             (struct PACKET *const P,
                                             struct VARIANT *const From);
void            PACKET_Bidir                (struct PACKET *const P,
                                             const uint8_t *const SendData,
                                             const uint32_t SendOctets,
                                             uint8_t *const RecvData,
                                             const uint32_t RecvOctets);
enum PACKET_Errors
                PACKET_Error                (struct PACKET *const P);
const char *    PACKET_Description          (struct PACKET *const P);
