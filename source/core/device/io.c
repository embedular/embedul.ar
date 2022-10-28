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
              const struct IO_IFACE *const Iface,
              const struct IO_PortInfo *const PortInfo,
              const TIMER_Ticks DeferredUpdatePeriod)
{
    BOARD_AssertParams (Io && Iface && PortInfo);

    // Required Interface
    BOARD_AssertInterface (
        Iface->Description && Iface->PortCount > 0 && Iface->Update &&
        ((Iface->GetInput && Iface->InputName) ||
         (Iface->SetOutput && Iface->OutputName)));

    OBJECT_Clear (Io);

    Io->iface                   = Iface;
    Io->portInfo                = PortInfo;
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


IO_Port IO_PortCount (struct IO *const Io)
{
    BOARD_AssertParams (Io);
    BOARD_AssertState  (Io->iface);

    return Io->iface->PortCount;
}


IO_Count IO_InputCount (struct IO *const Io, const enum IO_Type IoType)
{
    BOARD_AssertParams (Io && IoType < IO_Type__COUNT);
    BOARD_AssertState  (Io->iface);

    return Io->iface->InCount[IoType];
}


IO_Count IO_OutputCount (struct IO *const Io, const enum IO_Type IoType)
{
    BOARD_AssertParams (Io && IoType < IO_Type__COUNT);
    BOARD_AssertState  (Io->iface);

    return Io->iface->OutCount[IoType];
}


IO_Count IO_AvailableInputs (struct IO *const Io, const enum IO_Type IoType,
                             const IO_Port Port)
{
    BOARD_AssertParams (Io && IoType < IO_Type__COUNT);
    BOARD_AssertState  (Io->iface && Io->portInfo);
    BOARD_AssertParams (Port < Io->iface->PortCount);

    return Io->portInfo[Port].inAvailable[IoType];
}


IO_Count IO_AvailableOutputs (struct IO *const Io, const enum IO_Type IoType,
                              const IO_Port OutPort)
{
    BOARD_AssertParams (Io && IoType < IO_Type__COUNT);
    BOARD_AssertState  (Io->iface && Io->portInfo);
    BOARD_AssertParams (OutPort < Io->iface->PortCount);

    return Io->portInfo[OutPort].outAvailable[IoType];
}


IO_Value IO_GetInput (struct IO *const Io, const enum IO_Type IoType,
                      const IO_Code DriverCode, const IO_Port Port,
                      const enum IO_UpdateValue When)
{
    BOARD_AssertParams (Io && IoType < IO_Type__COUNT);
    BOARD_AssertState  (Io->iface && Io->iface->GetInput && Io->iface->Update);
    BOARD_AssertParams (DriverCode < Io->iface->InCount[IoType] &&
                        Port < Io->iface->PortCount);

    if (When == IO_UpdateValue_Now)
    {
        Io->iface->Update (Io);        
    }

    return Io->iface->GetInput (Io, IoType, DriverCode, Port);
}


void IO_SetOutput (struct IO *const Io, const enum IO_Type IoType,
                   const IO_Code DriverCode, const IO_Port Port,
                   const IO_Value Value, const enum IO_UpdateValue When)
{
    BOARD_AssertParams (Io && IoType < IO_Type__COUNT);
    BOARD_AssertState  (Io->iface && Io->iface->SetOutput && Io->iface->Update);
    BOARD_AssertParams (DriverCode < Io->iface->OutCount[IoType] &&
                        Port < Io->iface->PortCount);

    Io->iface->SetOutput (Io, IoType, DriverCode, Port, Value);

    if (When == IO_UpdateValue_Now)
    {
        Io->iface->Update (Io);
    }
}


IO_Code IO_GetAnyInput (struct IO *const Io, const enum IO_Type IoType,
                        const IO_Port Port)
{
    BOARD_AssertParams (Io && IoType < IO_Type__COUNT);
    BOARD_AssertState  (Io->iface && Io->iface->GetAnyInput);
    BOARD_AssertParams (Port < Io->iface->PortCount);

    return Io->iface->GetAnyInput (Io, IoType, Port);    
}


const char * IO_Description (struct IO *const Io)
{
    BOARD_AssertParams (Io);
    BOARD_AssertState  (Io->iface && Io->iface->Description);

    return Io->iface->Description;
}


const char * IO_InputName (struct IO *const Io, const enum IO_Type IoType,
                           const IO_Code DriverCode)
{
    BOARD_AssertParams (Io && IoType < IO_Type__COUNT);
    BOARD_AssertState  (Io->iface && Io->iface->InputName);
    BOARD_AssertParams (DriverCode < Io->iface->InCount[IoType]);

    return Io->iface->InputName (Io, IoType, DriverCode);
}


const char * IO_OutputName (struct IO *const Io, const enum IO_Type IoType,
                            const IO_Code DriverCode)
{
    BOARD_AssertParams (Io && IoType < IO_Type__COUNT);
    BOARD_AssertState  (Io->iface && Io->iface->OutputName);
    BOARD_AssertParams (DriverCode < Io->iface->OutCount[IoType]);

    return Io->iface->OutputName (Io, IoType, DriverCode);
}
