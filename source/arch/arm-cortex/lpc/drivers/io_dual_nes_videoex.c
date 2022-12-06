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


static const char * s_InputBitNames[IO_DUAL_NES_VIDEOEX_INB__COUNT] =
{
    [IO_DUAL_NES_VIDEOEX_INB_Right]     = "Right",
    [IO_DUAL_NES_VIDEOEX_INB_Left]      = "Left",
    [IO_DUAL_NES_VIDEOEX_INB_Down]      = "Down",
    [IO_DUAL_NES_VIDEOEX_INB_Up]        = "Up",
    [IO_DUAL_NES_VIDEOEX_INB_Start]     = "Start",
    [IO_DUAL_NES_VIDEOEX_INB_Select]    = "Select",
    [IO_DUAL_NES_VIDEOEX_INB_B]         = "B",
    [IO_DUAL_NES_VIDEOEX_INB_A]         = "A"
};


static void         update              (struct IO *const Io);
static uint32_t     getInput            (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode,
                                         const IO_Port Port);
static const char * inputName           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode);


static const struct IO_IFACE IO_DUAL_NES_VIDEOEX_IFACE =
{
    IO_IFACE_DECLARE("nes from video adapter", DUAL_NES_VIDEOEX),
    .Update         = update,
    .GetInput       = getInput,
    .InputName      = inputName
};


void IO_DUAL_NES_VIDEOEX_Init (struct IO_DUAL_NES_VIDEOEX *const N)
{
    BOARD_AssertParams (N);

    DEVICE_IMPLEMENTATION_Clear (N);

    IO_INIT_STATIC_PORT_INFO (N, DUAL_NES_VIDEOEX);

    // Update once per frame (~60 Hz).
    IO_Init ((struct IO *)N, &IO_DUAL_NES_VIDEOEX_IFACE, N->portInfo, 15);
}


void IO_DUAL_NES_VIDEOEX_Attach (struct IO_DUAL_NES_VIDEOEX *const N)
{
    MIO_RegisterGateway (MIO_Dir_Input, (struct IO *)N, 0);

    MIO_MAP_INPUT_BIT (GP1, Right, IO_DUAL_NES_VIDEOEX_INB_Right);
    MIO_MAP_INPUT_BIT (GP1, Left, IO_DUAL_NES_VIDEOEX_INB_Left);
    MIO_MAP_INPUT_BIT (GP1, Down, IO_DUAL_NES_VIDEOEX_INB_Down);
    MIO_MAP_INPUT_BIT (GP1, Up, IO_DUAL_NES_VIDEOEX_INB_Up);
    MIO_MAP_INPUT_BIT (GP1, Start, IO_DUAL_NES_VIDEOEX_INB_Start);
    MIO_MAP_INPUT_BIT (GP1, Select, IO_DUAL_NES_VIDEOEX_INB_Select);
    MIO_MAP_INPUT_BIT (GP1, A, IO_DUAL_NES_VIDEOEX_INB_A);
    MIO_MAP_INPUT_BIT (GP1, B, IO_DUAL_NES_VIDEOEX_INB_B);

    MIO_RegisterGateway (MIO_Dir_Input, (struct IO *)N, 1);

    MIO_MAP_INPUT_BIT (GP2, Right, IO_DUAL_NES_VIDEOEX_INB_Right);
    MIO_MAP_INPUT_BIT (GP2, Left, IO_DUAL_NES_VIDEOEX_INB_Left);
    MIO_MAP_INPUT_BIT (GP2, Down, IO_DUAL_NES_VIDEOEX_INB_Down);
    MIO_MAP_INPUT_BIT (GP2, Up, IO_DUAL_NES_VIDEOEX_INB_Up);
    MIO_MAP_INPUT_BIT (GP2, Start, IO_DUAL_NES_VIDEOEX_INB_Start);
    MIO_MAP_INPUT_BIT (GP2, Select, IO_DUAL_NES_VIDEOEX_INB_Select);
    MIO_MAP_INPUT_BIT (GP2, A, IO_DUAL_NES_VIDEOEX_INB_A);
    MIO_MAP_INPUT_BIT (GP2, B, IO_DUAL_NES_VIDEOEX_INB_B);
}


void update (struct IO *const Io)
{
    struct IO_DUAL_NES_VIDEOEX *const N = (struct IO_DUAL_NES_VIDEOEX *) Io;

    // NES hardware buttons are active-LOW
    N->gp1Data = ~g_videoExchange.io.d0;
    N->gp2Data = ~g_videoExchange.io.d1;
}


uint32_t getInput (struct IO *const Io, const enum IO_Type IoType,
                   const IO_Code DriverCode, const IO_Port Port)
{
    (void) IoType;

    struct IO_DUAL_NES_VIDEOEX *const N = (struct IO_DUAL_NES_VIDEOEX *) Io;

    return (Port == 0)? N->gp1Data & (1 << DriverCode) :
                        N->gp2Data & (1 << DriverCode);
}


const char * inputName (struct IO *const Io,
                        const enum IO_Type IoType,
                        const IO_Code DriverCode)
{
    (void) Io;
    (void) IoType;

    return s_InputBitNames[DriverCode];
}
