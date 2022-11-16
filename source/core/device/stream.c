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


void STREAM_Init (struct STREAM *const S, 
                  const struct STREAM_IFACE *const Iface)
{
    BOARD_AssertParams (S && Iface);

    BOARD_AssertInterface (Iface->Description &&
                            Iface->DataIn &&
                            Iface->DataOut);

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
    return (S && S->iface && S->iface->Description && S->iface->DataIn
            && S->iface->DataOut)? true : false;
}


enum DEVICE_CommandResult STREAM_Command (struct STREAM *const S, 
                                          const char *const Name,
                                          struct VARIANT *const Value)
{
    BOARD_AssertParams (STREAM_IsValid(S) && Name && Value);

    return DEVICE_Command (LANG_STREAM_COMMAND, &OBJECT_INFO_Spawn(S),
                           (DEVICE_CommandFunc)S->iface->Command, Name, Value);
}


void STREAM_OUT_Timeout (struct STREAM *const S, const TIMER_Ticks Ticks)
{
    BOARD_AssertParams (STREAM_IsValid(S));
    S->outTimeout = Ticks;
}


void STREAM_OUT_ToBuffer (struct STREAM *const S, uint8_t *const Buffer,
                          const uint32_t Octets)
{
    BOARD_AssertParams (STREAM_IsValid(S) && Buffer);

    S->lastOut  = 0;
    S->stop     = false;

    if (!Octets)
    {
        return;
    }

    const TIMER_Ticks Timeout = TICKS_Now() + S->outTimeout;

    do
    {
        const uint32_t OutCount = S->iface->DataOut (S,
                                &Buffer[S->lastOut], Octets - S->lastOut);
        if (!OutCount)
        {
            if (Timeout <= TICKS_Now())
            {
                break;
            }
        }
        else 
        {
            S->lastOut += OutCount;
        }

        if (S->stop)
        {
            break;
        }
    }
    while (S->lastOut < Octets);
}



uint8_t STREAM_OUT_ToOctet (struct STREAM *const S)
{
    uint8_t octet;
    STREAM_OUT_ToBuffer (S, &octet, 1);
    return octet;
}


static void streamToStream (struct STREAM *const In, struct STREAM *const Out)
{
    const TIMER_Ticks   Now         = TICKS_Now ();
    const TIMER_Ticks   OutTimeout  = Now + Out->outTimeout;
    const TIMER_Ticks   InTimeout   = Now + In->inTimeout;
    bool                retryIn     = false;
    uint8_t             octet;

    Out->lastOut    = 0;
    Out->stop       = false;
    In->lastIn      = 0;

    do
    {
        if (!retryIn)
        {
            const uint32_t OutCount = Out->iface->DataOut (Out, &octet, 1);
            if (!OutCount)
            {
                if (OutTimeout <= TICKS_Now())
                {
                    break;
                }

                continue;
            }
            else
            {
                Out->lastOut += OutCount;
            }
        }

        const uint32_t InCount = In->iface->DataIn (In, &octet, 1);
        if (!InCount)
        {
            retryIn = true;

            if (InTimeout <= TICKS_Now())
            {
                break;
            }
        }
        else 
        {
            retryIn = false;
            In->lastIn += InCount;

            if (Out->stop)
            {
                break;
            }
        }
    }
    while (1);

    if (retryIn)
    {
        In->octetInRetry = octet;
    }
}


void STREAM_OUT_ToStream (struct STREAM *const S, struct STREAM *const In)
{
    BOARD_AssertParams (STREAM_IsValid(S) && STREAM_IsValid(In));
    streamToStream (In, S);
}


void STREAM_OUT_Discard (struct STREAM *const S)
{
    BOARD_AssertParams (STREAM_IsValid(S));

    uint32_t    out;
    uint8_t     octet;

    S->lastOut = 0;
    while ((out = S->iface->DataOut (S, &octet, 1)))
    {
        S->lastOut += out;
    }
}


uint32_t STREAM_OUT_Count (struct STREAM *const S)
{
    BOARD_AssertParams (STREAM_IsValid(S));
    return S->lastOut;
}


void STREAM_IN_Timeout (struct STREAM *const S, const TIMER_Ticks Ticks)
{
    BOARD_AssertParams (STREAM_IsValid(S));
    S->inTimeout = Ticks;
}


void STREAM_IN_FromBuffer (struct STREAM *const S, const uint8_t *const Data,
                           const uint32_t Octets)
{
    BOARD_AssertParams (STREAM_IsValid(S) && Data);

    S->lastIn = 0;

    if (!Octets)
    {
        return;
    }

    const TIMER_Ticks Timeout = TICKS_Now() + S->inTimeout;

    do 
    {
        const uint32_t InCount = S->iface->DataIn (S, &Data[S->lastIn],
                                                        Octets - S->lastIn);
        if (!InCount)
        {
            const TIMER_Ticks Now = TICKS_Now ();

            if (Timeout <= Now)
            {
                break;
            }
        }
        else 
        {
            S->lastIn += InCount;
        }
    }
    while (S->lastIn < Octets);
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
    uint32_t        lastIn;
};


static void streamOutProc (void *const Param, const uint8_t *const Data, 
                           const uint32_t Octets)
{
    struct OutProcParams *opp = (struct OutProcParams *) Param;
    STREAM_IN_FromBuffer (opp->stream, Data, Octets);
    opp->lastIn += opp->stream->lastIn;
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
        .lastIn = 0
    };

    const uint32_t LastOutColumn = 
        VARIANT_ParseStringArgs (OutColumn, MaxOctets, Str, streamOutProc, &opp,
                                 ArgValues, ArgCount);
    
    S->lastIn = opp.lastIn;

    return LastOutColumn;
}


void STREAM_IN_FromOctet (struct STREAM *const S, const uint8_t Octet)
{
    STREAM_IN_FromBuffer (S, &Octet, 1);
}


void STREAM_IN_FromStream (struct STREAM *const S, struct STREAM *const Out)
{
    BOARD_AssertParams (STREAM_IsValid(S) && STREAM_IsValid(Out));
    streamToStream (S, Out);
}


void STREAM_IN_OctetRetry (struct STREAM *const S)
{
    const TIMER_Ticks Timeout = TICKS_Now() + S->inTimeout;

    while (!(S->lastIn = S->iface->DataIn(S, &S->octetInRetry, 1)) && 
           Timeout > TICKS_Now())
    {
    }
}


uint32_t STREAM_IN_Count (struct STREAM *const S)
{
    BOARD_AssertParams (STREAM_IsValid(S));
    return S->lastIn;
}


const char * STREAM_Description (struct STREAM *const S)
{
    BOARD_AssertParams (STREAM_IsValid(S));
    return S->iface->Description;
}
