/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [IO driver] Dual NES gamepad from video adapter.

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

#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/io_dual_nes_videoex.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/video_dualcore/exchange.h"
#include "embedul.ar/source/core/device/board.h"


static const char * s_InputNames[] =
{
    [IO_DUAL_NES_VIDEOEX_INB_Right]     = "right",
    [IO_DUAL_NES_VIDEOEX_INB_Left]      = "left",
    [IO_DUAL_NES_VIDEOEX_INB_Down]      = "down",
    [IO_DUAL_NES_VIDEOEX_INB_Up]        = "up",
    [IO_DUAL_NES_VIDEOEX_INB_Start]     = "start",
    [IO_DUAL_NES_VIDEOEX_INB_Select]    = "select",
    [IO_DUAL_NES_VIDEOEX_INB_B]         = "b",
    [IO_DUAL_NES_VIDEOEX_INB_A]         = "a"
};


static void         update              (struct IO *const Io);
static IO_Count
                    availableInputs     (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Port InPort);
static uint32_t     getInput            (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code Code,
                                         const IO_Port InPort);
static const char * inputName           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code Code);


static const struct IO_IFACE IO_DUAL_NES_VIDEOEX_IFACE =
{
    .Description        = "nes from video adapter",
    .Update             = update,
    .AvailableInputs    = availableInputs,
    .AvailableOutputs   = UNSUPPORTED,
    .GetInput           = getInput,
    .SetOutput          = UNSUPPORTED,
    .InputName          = inputName,
    .OutputName         = UNSUPPORTED
};


void IO_DUAL_NES_VIDEOEX_Init (struct IO_DUAL_NES_VIDEOEX *const N)
{
    BOARD_AssertParams (N);

    DEVICE_IMPLEMENTATION_Clear (N);

    // Update once per frame (~60 Hz).
    IO_Init ((struct IO *)N, &IO_DUAL_NES_VIDEOEX_IFACE, 15);
}


void IO_DUAL_NES_VIDEOEX_Attach (struct IO_DUAL_NES_VIDEOEX *const N)
{
    INPUT_SetGateway ((struct IO *)N, 0);

    INPUT_MAP_BIT (GP1, Right, IO_DUAL_NES_VIDEOEX_INB_Right);
    INPUT_MAP_BIT (GP1, Left, IO_DUAL_NES_VIDEOEX_INB_Left);
    INPUT_MAP_BIT (GP1, Down, IO_DUAL_NES_VIDEOEX_INB_Down);
    INPUT_MAP_BIT (GP1, Up, IO_DUAL_NES_VIDEOEX_INB_Up);
    INPUT_MAP_BIT (GP1, Start, IO_DUAL_NES_VIDEOEX_INB_Start);
    INPUT_MAP_BIT (GP1, Select, IO_DUAL_NES_VIDEOEX_INB_Select);
    INPUT_MAP_BIT (GP1, A, IO_DUAL_NES_VIDEOEX_INB_A);
    INPUT_MAP_BIT (GP1, B, IO_DUAL_NES_VIDEOEX_INB_B);


    INPUT_SetGateway ((struct IO *)N, 1);

    INPUT_MAP_BIT (GP2, Right, IO_DUAL_NES_VIDEOEX_INB_Right);
    INPUT_MAP_BIT (GP2, Left, IO_DUAL_NES_VIDEOEX_INB_Left);
    INPUT_MAP_BIT (GP2, Down, IO_DUAL_NES_VIDEOEX_INB_Down);
    INPUT_MAP_BIT (GP2, Up, IO_DUAL_NES_VIDEOEX_INB_Up);
    INPUT_MAP_BIT (GP2, Start, IO_DUAL_NES_VIDEOEX_INB_Start);
    INPUT_MAP_BIT (GP2, Select, IO_DUAL_NES_VIDEOEX_INB_Select);
    INPUT_MAP_BIT (GP2, A, IO_DUAL_NES_VIDEOEX_INB_A);
    INPUT_MAP_BIT (GP2, B, IO_DUAL_NES_VIDEOEX_INB_B);
}


void update (struct IO *const Io)
{
    struct IO_DUAL_NES_VIDEOEX *const N = (struct IO_DUAL_NES_VIDEOEX *) Io;

    // NES hardware buttons are active-LOW
    N->gp1Data = ~g_videoExchange.io.d0;
    N->gp2Data = ~g_videoExchange.io.d1;
}


IO_Count availableInputs (struct IO *const Io, const enum IO_Type IoType,
                          const IO_Port InPort)
{
    (void) Io;
    (void) InPort;

    // NES controllers have no analog inputs
    if (IoType == IO_Type_Range)
    {
        return 0;
    }

    return IO_DUAL_NES_VIDEOEX_INB__COUNT;
}


uint32_t getInput (struct IO *const Io, const enum IO_Type IoType,
                   const IO_Code Code, const IO_Port InPort)
{
    BOARD_AssertParams (IoType == IO_Type_Bit &&
                        Code < IO_DUAL_NES_VIDEOEX_INB__COUNT &&
                        InPort < 2);
    
    struct IO_DUAL_NES_VIDEOEX *const N = (struct IO_DUAL_NES_VIDEOEX *) Io;

    return (InPort == 0)? N->gp1Data & (1 << Code) : N->gp2Data & (1 << Code);
}


const char * inputName (struct IO *const Io,
                        const enum IO_Type IoType,
                        const IO_Code Code)
{
    BOARD_AssertParams (IoType == IO_Type_Bit &&
                        Code < IO_DUAL_NES_VIDEOEX_INB__COUNT);

    (void) Io;

    return s_InputNames[Code];
}
