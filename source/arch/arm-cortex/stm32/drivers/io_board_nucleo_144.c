/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [IO driver] generic st nucleo-144 board.

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

#include "embedul.ar/source/arch/arm-cortex/stm32/drivers/io_board_nucleo_144.h"
#include "embedul.ar/source/arch/arm-cortex/stm32/nucleo_144_bsp.h"
#include "embedul.ar/source/core/device/board.h"


static const char * s_InputBitNames[IO_BOARD_NUCLEO_144_INB__COUNT] =
{
    [IO_BOARD_NUCLEO_144_INB_USER] = "User"
};


static const char * s_OutputBitNames[IO_BOARD_NUCLEO_144_OUTB__COUNT] =
{
    [IO_BOARD_NUCLEO_144_OUTB_LED_GREEN]    = "Green LED",
    [IO_BOARD_NUCLEO_144_OUTB_LED_BLUE]     = "Blue LED",
    [IO_BOARD_NUCLEO_144_OUTB_LED_RED]      = "Red LED"
};


static void         update              (struct IO *const Io);
static uint32_t     getInput            (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode,
                                         const IO_Port Port);
static void         setOutput           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode,
                                         const IO_Port Port,
                                         const uint32_t Value);
static const char * inputName           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode);
static const char * outputName          (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode);


static const struct IO_IFACE IO_BOARD_NUCLEO_144_IFACE =
{
    IO_IFACE_DECLARE("st nucleo-144 board io", BOARD_NUCLEO_144),
    .Update         = update,
    .GetInput       = getInput,
    .SetOutput      = setOutput,
    .InputName      = inputName,
    .OutputName     = outputName
};


void IO_BOARD_NUCLEO_144_Init (struct IO_BOARD_NUCLEO_144 *const B)
{
    BOARD_AssertParams (B);

    DEVICE_IMPLEMENTATION_Clear (B);

    IO_INIT_STATIC_PORT_INFO (B, BOARD_NUCLEO_144);

    BSP_PB_Init (BUTTON_USER, BUTTON_MODE_GPIO);

    BSP_LED_Init (LED_GREEN);
    BSP_LED_Init (LED_BLUE);
    BSP_LED_Init (LED_RED);

    // Update once per frame (~60 Hz).
    IO_Init ((struct IO *)B, &IO_BOARD_NUCLEO_144_IFACE, B->portInfo, 15);
}


void IO_BOARD_NUCLEO_144_Attach (struct IO_BOARD_NUCLEO_144 *const B)
{
    BOARD_AssertParams (B);

    MIO_RegisterGateway (MIO_Dir_Input, (struct IO *)B, 0);

    MIO_MAP_INPUT_BIT (MAIN, A, IO_BOARD_NUCLEO_144_INB_USER);


    MIO_RegisterGateway (MIO_Dir_Output, (struct IO *)B, 0);

    MIO_MAP_OUTPUT_BIT (SIGN, Warning, IO_BOARD_NUCLEO_144_OUTB_LED_RED);
    MIO_MAP_OUTPUT_BIT (SIGN, Red, IO_BOARD_NUCLEO_144_OUTB_LED_RED);
    MIO_MAP_OUTPUT_BIT (SIGN, Green, IO_BOARD_NUCLEO_144_OUTB_LED_GREEN);
    MIO_MAP_OUTPUT_BIT (SIGN, Blue, IO_BOARD_NUCLEO_144_OUTB_LED_BLUE);
}


inline static void bspButtonIn (struct IO_BOARD_NUCLEO_144 *const B,
                                const enum IO_BOARD_NUCLEO_144_INB Inb,
                                const uint32_t Button)
{
    B->inbData |= BSP_PB_GetState(Button)? 1 << Inb : 0;
}


inline static void ledOut (struct IO_BOARD_NUCLEO_144 *const B,
                           const enum IO_BOARD_NUCLEO_144_OUTB Outb,
                           const uint32_t Led)
{
    (B->outbData & (1 << Outb))? BSP_LED_On(Led) : BSP_LED_Off(Led);
}


void update (struct IO *const Io)
{
    struct IO_BOARD_NUCLEO_144 *const B = (struct IO_BOARD_NUCLEO_144 *) Io;

    // Input
    B->inbData = 0;

    bspButtonIn (B, IO_BOARD_NUCLEO_144_INB_USER, BUTTON_USER);

    // Output
    ledOut (B, IO_BOARD_NUCLEO_144_OUTB_LED_GREEN, LED_GREEN);
    ledOut (B, IO_BOARD_NUCLEO_144_OUTB_LED_BLUE, LED_BLUE);
    ledOut (B, IO_BOARD_NUCLEO_144_OUTB_LED_RED, LED_RED);
}


uint32_t getInput (struct IO *const Io, const enum IO_Type IoType,
                   const IO_Code DriverCode, const IO_Port Port)
{
    (void) IoType;
    (void) Port;

    struct IO_BOARD_NUCLEO_144 *const B = (struct IO_BOARD_NUCLEO_144 *) Io;

    return (B->inbData & (1 << DriverCode));
}


void setOutput (struct IO *const Io, const enum IO_Type IoType,
                const IO_Code DriverCode, const IO_Port Port,
                const uint32_t Value)
{
    (void) IoType;
    (void) Port;

    struct IO_BOARD_NUCLEO_144 *const B = (struct IO_BOARD_NUCLEO_144 *) Io;

    if (Value)
    {
        B->outbData |= (1 << DriverCode);
    }
    else
    {
        B->outbData &= ~(1 << DriverCode);
    }
}


const char * inputName (struct IO *const Io, const enum IO_Type IoType,
                        const IO_Code DriverCode)
{
    (void) Io;
    (void) IoType;

    return s_InputBitNames[DriverCode];
}


const char * outputName (struct IO *const Io, const enum IO_Type IoType,
                         const IO_Code DriverCode)
{
    (void) Io;
    (void) IoType;

    return s_OutputBitNames[DriverCode];
}
