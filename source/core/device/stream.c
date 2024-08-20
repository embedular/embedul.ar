/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [STREAM] input/output octet stream device driver interface.

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

#include "embedul.ar/source/core/device/stream.h"
#include "embedul.ar/source/core/device/board.h"


static bool validIface (const struct STREAM_IFACE *const Iface)
{
    return (Iface && Iface->Description && 
                        (Iface->DataIn || Iface->DataOut || Iface->DataExchange));
}


static void streamToStream (struct STREAM *const In, struct STREAM *const Out)
{
    const TIMER_Ticks   Now         = TICKS_Now ();
    const TIMER_Ticks   OutTimeout  = Now + Out->timeout;
    const TIMER_Ticks   InTimeout   = Now + In->timeout;
    uint8_t             octet;

    In->type        = STREAM_TransferType_In;
    In->status      = STREAM_TransferStatus_Ok;
    In->iteration   = 0;
    In->count       = 0;

    Out->type       = STREAM_TransferType_Out;
    Out->status     = STREAM_TransferStatus_Ok;
    Out->iteration  = 0;
    Out->count      = 0;

    do
    {
        if (OutTimeout <= TICKS_Now())
        {
            Out->status = STREAM_TransferStatus_Timedout;
            break;
        }
        
        Out->count += Out->iface->DataOut (Out, &octet, 1);

        ++ Out->iteration;

        if (Out->status != STREAM_TransferStatus_Ok)
        {
            break;
        }

        uint32_t inCount = 0;

        do 
        {
            inCount = In->iface->DataIn (In, &octet, 1);

            ++ In->iteration;

            if (InTimeout <= TICKS_Now() &&
                Out->status == STREAM_TransferStatus_Ok)
            {
                Out->status = STREAM_TransferStatus_Timedout;
            }
        }
        while (!inCount && In->status == STREAM_TransferStatus_Ok);

        In->count += inCount;
    }
    while (In->status == STREAM_TransferStatus_Ok);

    // The application programmer must call STREAM_IN_S2SRetry(In)
    // if STREAM_Count(Out) != STREAM_Count(In).
    In->s2sInRetry = octet;
}


void STREAM_Init (struct STREAM *const S, 
                  const struct STREAM_IFACE *const Iface)
{
    BOARD_AssertParams (S && Iface);

    BOARD_AssertInterface (validIface(Iface));

    OBJECT_Clear (S);

    S->iface = Iface;

    // The debug stream driver cannot log its own initialization
    if (BOARD_CurrentStage() != BOARD_Stage_InitDebugStreamDriver)
    {
        LOG_ContextBegin (S, LANG_INIT);
    }

    if (S->iface->HardwareInit)
    {
        S->iface->HardwareInit (S);
    }

    if (BOARD_CurrentStage() != BOARD_Stage_InitDebugStreamDriver)
    {    
        BOARD_AssertState (STREAM_IsValid(S));
        LOG_ContextEnd ();
    }
}


bool STREAM_IsValid (struct STREAM *const S)
{
    return (S && validIface(S->iface))? true : false;
}


bool STREAM_Connect (struct STREAM *const S)
{
    BOARD_AssertParams (STREAM_IsValid(S));

    if (S->iface->Connect)
    {
        S->iface->Connect (S);
    }

    return S->connected;
}


bool STREAM_IsConnected (struct STREAM *const S)
{
    BOARD_AssertParams (STREAM_IsValid(S));
    return S->connected;
}


enum DEVICE_CommandResult STREAM_Command (struct STREAM *const S, 
                                          const char *const Name,
                                          struct VARIANT *const Value)
{
    BOARD_AssertParams (STREAM_IsValid(S) && Name && Value);

    return DEVICE_Command (LANG_STREAM_COMMAND, &OBJECT_INFO_Spawn(S),
                           (DEVICE_CommandFunc)S->iface->Command, Name, Value);
}


void STREAM_Address (struct STREAM *const S, struct VARIANT *const Address)
{
    BOARD_AssertParams (STREAM_IsValid(S));
    VARIANT_SetCopy (&S->address, Address);
}


void STREAM_Timeout (struct STREAM *const S, const TIMER_Ticks Ticks)
{
    BOARD_AssertParams (STREAM_IsValid(S));
    S->timeout = Ticks;
}


enum STREAM_TransferType STREAM_TransferType (struct STREAM *const S)
{
    BOARD_AssertParams (STREAM_IsValid(S));
    return S->type;
}


enum STREAM_TransferStatus STREAM_TransferStatus (struct STREAM *const S)
{
    BOARD_AssertParams (STREAM_IsValid(S));
    return S->status;
}


uint32_t STREAM_Count (struct STREAM *const S)
{
    BOARD_AssertParams (STREAM_IsValid(S));
    return S->count;
}


void STREAM_IN_FromBuffer (struct STREAM *const S, const uint8_t *const Data,
                           const uint32_t Octets)
{
    BOARD_AssertParams      (STREAM_IsValid(S) && Data);
    BOARD_AssertInterface   (S->iface->DataIn);

    S->type         = STREAM_TransferType_In;
    S->status       = STREAM_TransferStatus_Ok;
    S->iteration    = 0;
    S->count        = 0;

    if (!Octets)
    {
        return;
    }

    const TIMER_Ticks Timeout = TICKS_Now() + S->timeout;

    do 
    {
        S->count += S->iface->DataIn (S, &Data[S->count], Octets - S->count);

        ++ S->iteration;

        // Do not overwrite transfer status other than OK.
        if (Timeout <= TICKS_Now() && S->status == STREAM_TransferStatus_Ok)
        {
            S->status = STREAM_TransferStatus_Timedout;
        }
    }
    while (S->count < Octets && S->status == STREAM_TransferStatus_Ok);
}


void STREAM_IN_FromString (struct STREAM *const S, const char *const Str)
{
    BOARD_AssertParams (Str);

    const uint32_t Len = (uint32_t) strlen(Str);
    // The empty string ("") is a valid pointer, but has a size of
    // zero and outputs nothing.
    if (!Len)
    {
        return;
    }

    STREAM_IN_FromBuffer (S, (const uint8_t *)Str, Len);
}


struct OutProcParams
{
    struct STREAM   * stream;
    uint32_t        count;
};


static void streamOutProc (void *const Param, const uint8_t *const Data, 
                           const uint32_t Octets)
{
    struct OutProcParams *opp = (struct OutProcParams *) Param;
    STREAM_IN_FromBuffer (opp->stream, Data, Octets);
    opp->count += opp->stream->count;
}


uint32_t STREAM_IN_FromParsedStringArgs (struct STREAM *const S, 
                                         const uint32_t OutColumn, 
                                         const size_t MaxOctets,
                                         const char *const Str,
                                         struct VARIANT *const ArgValues,
                                         const uint32_t ArgCount)
{
    struct OutProcParams opp =
    {
        .stream = S,
        .count  = 0
    };

    const uint32_t LastOutColumn = 
        VARIANT_ParseStringArgs (OutColumn, MaxOctets, Str, streamOutProc, &opp,
                                 ArgValues, ArgCount);
    
    S->count = opp.count;

    return LastOutColumn;
}


void STREAM_IN_FromOctet (struct STREAM *const S, const uint8_t Octet)
{
    STREAM_IN_FromBuffer (S, &Octet, 1);
}


void STREAM_IN_FromStream (struct STREAM *const S, struct STREAM *const Out)
{
    BOARD_AssertParams      (STREAM_IsValid(S) && STREAM_IsValid(Out));
    BOARD_AssertInterface   (S->iface->DataIn && Out->iface->DataOut);

    streamToStream (S, Out);
}


void STREAM_IN_S2SRetry (struct STREAM *const S)
{
    BOARD_AssertParams      (STREAM_IsValid(S) &&
                             S->type == STREAM_TransferType_In);
    BOARD_AssertInterface   (S->iface->DataIn);

    const TIMER_Ticks Timeout = TICKS_Now() + S->timeout;

    while (!(S->count = S->iface->DataIn(S, &S->s2sInRetry, 1)) && 
           Timeout > TICKS_Now() && S->status == STREAM_TransferStatus_Ok)
    {
    }
}


void STREAM_OUT_ToBuffer (struct STREAM *const S, uint8_t *const Buffer,
                          const uint32_t Octets)
{
    BOARD_AssertParams      (STREAM_IsValid(S) && Buffer);
    BOARD_AssertInterface   (S->iface->DataOut);

    S->type         = STREAM_TransferType_Out;
    S->status       = STREAM_TransferStatus_Ok;
    S->iteration    = 0;
    S->count        = 0;

    if (!Octets)
    {
        return;
    }

    const TIMER_Ticks Timeout = TICKS_Now() + S->timeout;

    do
    {
        S->count += S->iface->DataOut (S, &Buffer[S->count], Octets - S->count);

        ++ S->iteration;

        // Do not overwrite transfer status other than OK.
        if (Timeout <= TICKS_Now() && S->status == STREAM_TransferStatus_Ok)
        {
            S->status = STREAM_TransferStatus_Timedout;
        }
    }
    while (S->count < Octets && S->status == STREAM_TransferStatus_Ok);
}


uint8_t STREAM_OUT_ToOctet (struct STREAM *const S)
{
    uint8_t octet;
    STREAM_OUT_ToBuffer (S, &octet, 1);
    return octet;
}


void STREAM_OUT_ToStream (struct STREAM *const S, struct STREAM *const In)
{
    BOARD_AssertParams      (STREAM_IsValid(S) && STREAM_IsValid(In));
    BOARD_AssertInterface   (S->iface->DataOut && In->iface->DataIn);

    streamToStream (In, S);
}


void STREAM_OUT_Discard (struct STREAM *const S)
{
    BOARD_AssertParams      (STREAM_IsValid(S));
    BOARD_AssertInterface   (S->iface->DataOut);

    uint32_t    out;
    uint8_t     octet;

    S->count = 0;
    while ((out = S->iface->DataOut (S, &octet, 1)))
    {
        S->count += out;
    }
}


void STREAM_EXCHANGE_Buffers (struct STREAM *const S,
                                const uint8_t *const InData,
                                const uint32_t InOctets,
                                uint8_t *const OutBuffer,
                                const uint32_t OutOctets)
{
    BOARD_AssertParams      (STREAM_IsValid(S) && InData && OutBuffer);
    BOARD_AssertInterface   (S->iface->DataExchange);

    const TIMER_Ticks   Timeout     = TICKS_Now() + S->timeout;
    const uint32_t      TotalOctets = InOctets + OutOctets;
    uint32_t            inCount     = 0;
    uint32_t            outCount    = 0;

    S->type         = STREAM_TransferType_Composite;
    S->status       = STREAM_TransferStatus_Ok;
    S->iteration    = 0;
    S->count        = 0;

    if (!InOctets && !OutOctets)
    {
        return;
    }

    do
    {
        const struct STREAM_DataExchangeResult Cr = S->iface->DataExchange (S, 
                                &InData[inCount], InOctets - inCount,
                                &OutBuffer[outCount], OutOctets - outCount);

        inCount     += Cr.inCount;
        outCount    += Cr.outCount;

        ++ S->iteration;

        S->count = inCount + outCount;

        // Do not overwrite transfer status other than OK.
        if (Timeout <= TICKS_Now() && S->iteration == STREAM_TransferStatus_Ok)
        {
            S->status = STREAM_TransferStatus_Timedout;
        }
    }
    while (S->count < TotalOctets && S->status == STREAM_TransferStatus_Ok);
}


const char * STREAM_Description (struct STREAM *const S)
{
    BOARD_AssertParams (STREAM_IsValid(S));
    return S->iface->Description;
}
