/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [IO] input/output device driver interface.

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

#include "embedul.ar/source/core/device/io.h"
#include "embedul.ar/source/core/device/board.h"


void IO_Init (struct IO *const Io,
                     const struct IO_IFACE *iface,
                     const TIMER_Ticks DeferredUpdatePeriod)
{
    BOARD_AssertParams (Io && iface);

    // Required Interface
    BOARD_AssertInterface (
        iface->Description && iface->Update &&
        ((iface->AvailableInputs && iface->GetInput && iface->InputName) ||
        (iface->AvailableOutputs && iface->SetOutput && iface->OutputName)));

    OBJECT_Clear (Io);

    Io->iface                   = iface;
    Io->deferredUpdatePeriod    = DeferredUpdatePeriod;

    LOG_ContextBegin (Io, LANG_INIT);
    {
        if (Io->iface->HardwareInit)
        {
            Io->iface->HardwareInit (Io);
        }

        BOARD_AssertState (IO_Initialized(Io));
    }
    LOG_ContextEnd ();
}


bool IO_Initialized (struct IO *const Io)
{
    return (Io && Io->iface)? true : false;
}


void IO_Update (struct IO *const Io)
{
    BOARD_AssertParams (Io);
    BOARD_AssertState  (Io->iface && Io->iface->Update);

    if (BOARD_TicksNow() >= Io->lastDeferredUpdate + Io->deferredUpdatePeriod)
    {
        Io->iface->Update (Io);
        Io->lastDeferredUpdate = BOARD_TicksNow ();
    }
}


IO_Count IO_AvailableInputs (struct IO *const Io, const enum IO_Type IoType,
                             const uint32_t InputSource)
{
    BOARD_AssertParams (Io);
    BOARD_AssertState  (Io->iface);

    if (!Io->iface->AvailableInputs)
    {
        return 0;
    }

    return Io->iface->AvailableInputs (Io, IoType, InputSource);
}


IO_Count IO_AvailableOutputs (struct IO *const Io, const enum IO_Type IoType,
                              const uint32_t OutputSource)
{
    BOARD_AssertParams (Io);
    BOARD_AssertState  (Io->iface);

    if (!Io->iface->AvailableOutputs)
    {
        return 0;
    }

    return Io->iface->AvailableOutputs (Io, IoType, OutputSource);
}


uint32_t IO_GetInput (struct IO *const Io, const enum IO_Type IoType,
                      const uint16_t Index, const uint32_t InputSource,
                      const enum IO_UpdateValue When)
{
    BOARD_AssertParams (Io);
    BOARD_AssertState  (Io->iface && Io->iface->GetInput);

    if (When == IO_UpdateValue_Now)
    {
        Io->iface->Update (Io);        
    }

    return Io->iface->GetInput (Io, IoType, Index, InputSource);
}


void IO_SetOutput (struct IO *const Io, const enum IO_Type IoType,
                   const BITFIELD_Index Index, const uint32_t InputSource,
                   const uint32_t Value, const enum IO_UpdateValue When)
{
    BOARD_AssertParams (Io);
    BOARD_AssertState  (Io->iface && Io->iface->SetOutput &&
                        Io->iface->Update);

    Io->iface->SetOutput (Io, IoType, Index, InputSource, Value);

    if (When == IO_UpdateValue_Now)
    {
        Io->iface->Update (Io);
    }
}


uint16_t IO_FirstOnIndex (struct IO *const Io, const enum IO_Type IoType,
                          const uint32_t InputSource)
{
    BOARD_AssertParams (Io);
    BOARD_AssertState  (Io->iface && Io->iface->FirstOnIndex);

    return Io->iface->FirstOnIndex (Io, IoType, InputSource);    
}


const char * IO_Description (struct IO *const Io)
{
    BOARD_AssertParams (Io);
    BOARD_AssertState  (Io->iface && Io->iface->Description);

    return Io->iface->Description;
}


const char * IO_InputName (struct IO *const Io, const enum IO_Type IoType,
                           const uint16_t Index)
{
    BOARD_AssertParams (Io);
    BOARD_AssertState  (Io->iface && Io->iface->InputName);

    return Io->iface->InputName (Io, IoType, Index);
}


const char * IO_OutputName (struct IO *const Io, const enum IO_Type IoType,
                            const uint16_t Index)
{
    BOARD_AssertParams (Io);
    BOARD_AssertState  (Io->iface && Io->iface->OutputName);

    return Io->iface->OutputName (Io, IoType, Index);
}
