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

#include "embedul.ar/source/core/device/packet.h"
#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/core/variant.h"


static bool validIface (const struct PACKET_IFACE *const Iface)
{
    return (Iface && Iface->Description && 
                        (Iface->Send || (Iface->Recv && Iface->RecvSize)));
}


void PACKET_Init (struct PACKET *const P,
                  const struct PACKET_IFACE *const Iface)
{
    BOARD_AssertParams (P && Iface);

    BOARD_AssertInterface (validIface(Iface));

    OBJECT_Clear (P);

    P->iface = Iface;

    LOG_ContextBegin (P, LANG_INIT);
    {
        if (P->iface->HardwareInit)
        {
            P->iface->HardwareInit (P);
        }

        BOARD_AssertState (PACKET_IsValid(P));
    }
    LOG_ContextEnd ();
}


bool PACKET_IsValid (struct PACKET *const P)
{
    return (P && validIface(P->iface));
}


enum DEVICE_CommandResult PACKET_Command (struct PACKET *const P, 
                                          const char *const Name,
                                          struct VARIANT *const Value)
{
    BOARD_AssertParams (PACKET_IsValid(P) && Name && Value);

    return DEVICE_Command (LANG_PACKET_COMMAND, &OBJECT_INFO_Spawn(P),
                           (DEVICE_CommandFunc)P->iface->Command, Name, Value);
}


void PACKET_SendTo (struct PACKET *const P, struct VARIANT *const To)
{
    BOARD_AssertParams (PACKET_IsValid(P) && To);
    VARIANT_SetCopy (&P->sendTo, To);
}


void PACKET_SendTimeout (struct PACKET *const P, const TIMER_Ticks Ticks)
{
    BOARD_AssertParams (PACKET_IsValid(P));
    P->sendTimeout = Ticks;
}


void PACKET_Send (struct PACKET *const P, const uint8_t *const Data,
                  const uint32_t Octets)
{
    BOARD_AssertParams (PACKET_IsValid(P) && Data);

    P->error        = PACKET_Errors_None;
    P->iteration    = 0;
    P->sentOctets   = 0;

    if (!P->iface->Send)
    {
        P->error = PACKET_Errors_UnsupportedInterface;
        return;
    }

    const TIMER_Ticks Timeout = BOARD_TicksNow() + P->sendTimeout;

    do
    {
        P->sentOctets += P->iface->Send (P, &Data[P->sentOctets], 
                                                    Octets - P->sentOctets);
        if (P->sentOctets >= Octets)
        {
            break;
        }
        else if (Timeout <= BOARD_TicksNow() && P->error == PACKET_Errors_None)
        {
            P->error = PACKET_Errors_Timedout;
        }

        ++ P->iteration;
    }
    while (P->error == PACKET_Errors_None);
}


uint32_t PACKET_SentCount (struct PACKET *const P)
{
    BOARD_AssertParams (PACKET_IsValid(P));
    return P->sentOctets;
}


void PACKET_RecvTimeout (struct PACKET *const P, const TIMER_Ticks Ticks)
{
    BOARD_AssertParams (PACKET_IsValid(P));
    P->recvTimeout = Ticks;
}


void PACKET_ClearRecvSize (struct PACKET *P)
{
    BOARD_AssertParams (PACKET_IsValid(P));

    P->recvSize = 0;
}


uint32_t PACKET_PeekRecvSize (struct PACKET *const P, const uint32_t MaxOctets)
{
    BOARD_AssertParams (PACKET_IsValid(P));

    P->error        = PACKET_Errors_None;
    P->iteration    = 0;

    if (!P->iface->RecvSize)
    {
        P->error = PACKET_Errors_UnsupportedInterface;
        return 0;
    }

    if (P->recvSize)
    {
        return P->recvSize;
    }

    const TIMER_Ticks Timeout = BOARD_TicksNow() + P->recvTimeout;

    do
    {
        const uint32_t Rs = P->iface->RecvSize (P, MaxOctets);

        if (Rs)
        {
            P->recvSize = Rs;
            break;
        }
        else if (Timeout <= BOARD_TicksNow() && P->error == PACKET_Errors_None)
        {
            P->error = PACKET_Errors_Timedout;
        }

        ++ P->iteration;
    }
    while (P->error == PACKET_Errors_None);

    return P->recvSize;
}


void PACKET_Recv (struct PACKET *const P, uint8_t *const Buffer,
                  const uint32_t Octets)
{
    BOARD_AssertParams (PACKET_IsValid(P) && Buffer && Octets);

    P->error        = PACKET_Errors_None;
    P->iteration    = 0;
    P->recvOctets   = 0;

    if (!P->iface->Recv)
    {
        P->error = PACKET_Errors_UnsupportedInterface;
        return;
    }

    // Automatically obtains packet size if not already done so
    if (!P->recvSize && !PACKET_PeekRecvSize(P, Octets))
    {
        P->error = PACKET_Errors_NoPacketSize;
        return;
    }

    if (Octets < P->recvSize)
    {
        LOG (P, "buffer size smaller than packet");
        LOG_Items (2,   "buffer size",  Octets,
                        "packet",       P->recvSize);
        BOARD_AssertParams (false);
    }

    const TIMER_Ticks Timeout = BOARD_TicksNow() + P->recvTimeout;

    do
    {
        P->recvOctets += P->iface->Recv (P, &Buffer[P->recvOctets], 
                                                P->recvSize - P->recvOctets);
        if (P->recvOctets >= P->recvSize)
        {
            break;
        }
        else if (Timeout <= BOARD_TicksNow() && P->error == PACKET_Errors_None)
        {
            P->error = PACKET_Errors_Timedout;
        }

        ++ P->iteration;
    }
    while (P->error == PACKET_Errors_None);

    // Clear received packet size
    P->recvSize = 0;
}


uint32_t PACKET_RecvCount (struct PACKET *const P)
{
    BOARD_AssertParams (PACKET_IsValid(P));
    return P->recvOctets;
}


struct VARIANT PACKET_GetRecvFrom (struct PACKET *const P)
{
    BOARD_AssertParams (PACKET_IsValid(P));
    return P->recvFrom;
}


void PACKET_Bidir (struct PACKET *const P,
                   const uint8_t *const SendData,
                   const uint32_t SendOctets,
                   uint8_t *const RecvData,
                   const uint32_t RecvOctets)
{
    BOARD_AssertParams (PACKET_IsValid(P) && SendData && SendOctets &&
                        RecvData && RecvOctets);

    P->error        = PACKET_Errors_None;
    P->iteration    = 0;
    P->sentOctets   = 0;
    P->recvOctets   = 0;
    // Dont care for bidirectional packets
    P->recvSize     = 0;

    if (!P->iface->Bidir)
    {
        P->error = PACKET_Errors_UnsupportedInterface;
        return;
    }

    const TIMER_Ticks Timeout = BOARD_TicksNow() + P->sendTimeout;

    do
    {
        const struct PACKET_BidirResult Br = P->iface->Bidir (P,
                                                &SendData[P->sentOctets],
                                                SendOctets - P->sentOctets,
                                                &RecvData[P->recvOctets],
                                                RecvOctets - P->recvOctets);
        P->sentOctets += Br.sentOctets;
        P->recvOctets += Br.recvOctets;

        if (P->sentOctets >= SendOctets && P->recvOctets >= RecvOctets)
        {
            break;
        }
        else if (Timeout <= BOARD_TicksNow() && P->error == PACKET_Errors_None)
        {
            P->error = PACKET_Errors_Timedout;
        }

        ++ P->iteration;
    }
    while (P->error == PACKET_Errors_None);
}


enum PACKET_Errors PACKET_Error (struct PACKET *P)
{
    BOARD_AssertParams (PACKET_IsValid(P));
    return P->error;
}


const char * PACKET_Description (struct PACKET *P)
{
    BOARD_AssertParams (PACKET_IsValid(P));
    return P->iface->Description;
}
