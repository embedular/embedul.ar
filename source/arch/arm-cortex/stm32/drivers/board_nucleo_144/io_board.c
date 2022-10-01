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

#include "embedul.ar/source/arch/arm-cortex/stm32/drivers/board_nucleo_144/io_board.h"
#include "embedul.ar/source/core/device/board.h"
#include "source/core/manager/output.h"


static const char * s_InputNames[IO_BOARD_INB__COUNT] =
{
    [IO_BOARD_INB_USER] = "user"
};


static const char * s_OutputNames[IO_BOARD_OUTB__COUNT] =
{
    [IO_BOARD_OUTB_LED_GREEN] = "green led",
    [IO_BOARD_OUTB_LED_BLUE] = "blue led",
    [IO_BOARD_OUTB_LED_RED] = "red led"
};


static void         update              (struct IO *const Io);
static IO_Count
                    availableInputs     (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Port InPort);
static IO_Count
                    availableOutputs    (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Port OutPort);
static uint32_t     getInput            (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code Code,
                                         const IO_Port InPort);
static void         setOutput           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code Code,
                                         const IO_Port OutPort,
                                         const uint32_t Value);
static const char * inputName           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code Code);
static const char * outputName          (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code Code);


static const struct IO_IFACE IO_BOARD_IFACE =
{
    .Description        = "st nucleo-144 board io",
    .Update             = update,
    .AvailableInputs    = availableInputs,
    .AvailableOutputs   = availableOutputs,
    .GetInput           = getInput,
    .SetOutput          = setOutput,
    .InputName          = inputName,
    .OutputName         = outputName
};


void IO_BOARD_Init (struct IO_BOARD *const B)
{
    BOARD_AssertParams (B);

    DEVICE_IMPLEMENTATION_Clear (B);

    BSP_PB_Init (BUTTON_USER, BUTTON_MODE_GPIO);

    BSP_LED_Init (LED_GREEN);
    BSP_LED_Init (LED_BLUE);
    BSP_LED_Init (LED_RED);

    // Update once per frame (~60 Hz).
    IO_Init ((struct IO *)B, &IO_BOARD_IFACE, 15);
}


void IO_BOARD_Attach (struct IO_BOARD *const B)
{
    BOARD_AssertParams (B);

    INPUT_SetGateway ((struct IO *)B, 0);

    INPUT_MAP_BIT (MAIN, A, IO_BOARD_INB_USER);


    OUTPUT_SetGateway ((struct IO *)B, 0);

    OUTPUT_MAP_BIT (SIGN, Warning, IO_BOARD_OUTB_LED_RED);
    OUTPUT_MAP_BIT (SIGN, Green, IO_BOARD_OUTB_LED_GREEN);
    OUTPUT_MAP_BIT (SIGN, Blue, IO_BOARD_OUTB_LED_BLUE);
}


inline static void bspButtonIn (struct IO_BOARD *const B,
                                const enum IO_BOARD_INB Inb,
                                const uint32_t Button)
{
    B->inbData |= BSP_PB_GetState(Button)? 1 << Inb : 0;
}


inline static void ledOut (struct IO_BOARD *const B,
                           const enum IO_BOARD_OUTB Outb, const uint32_t Led)
{
    (B->outbData & (1 << Outb))? BSP_LED_On(Led) : BSP_LED_Off(Led);
}


void update (struct IO *const Io)
{
    struct IO_BOARD *const B = (struct IO_BOARD *) Io;

    // Input
    B->inbData = 0;

    bspButtonIn (B, IO_BOARD_INB_USER, BUTTON_USER);

    // Output
    ledOut (B, IO_BOARD_OUTB_LED_GREEN, LED_GREEN);
    ledOut (B, IO_BOARD_OUTB_LED_BLUE, LED_BLUE);
    ledOut (B, IO_BOARD_OUTB_LED_RED, LED_RED);
}


IO_Count availableInputs (struct IO *const Io, const enum IO_Type IoType,
                          const IO_Port InPort)
{
    (void) Io;
    (void) InPort;

    // This driver handles no analog inputs
    if (IoType == IO_Type_Range)
    {
        return 0;
    }

    return IO_BOARD_INB__COUNT;
}


IO_Count availableOutputs (struct IO *const Io, const enum IO_Type IoType,
                           const IO_Port OutPort)
{
    (void) Io;
    (void) OutPort;

    // This driver handles no analog outputs
    if (IoType == IO_Type_Range)
    {
        return 0;
    }

    return IO_BOARD_OUTB__COUNT;
}


uint32_t getInput (struct IO *const Io, const enum IO_Type IoType,
                   const IO_Code Code, const IO_Port InPort)
{
    BOARD_AssertParams (IoType == IO_Type_Bit &&
                        Code < IO_BOARD_INB__COUNT);

    (void) InPort;

    struct IO_BOARD *const B = (struct IO_BOARD *) Io;

    return (B->inbData & (1 << Code));
}


void setOutput (struct IO *const Io, const enum IO_Type IoType,
                const IO_Code Code, const IO_Port OutPort,
                const uint32_t Value)
{
    BOARD_AssertParams (IoType == IO_Type_Bit &&
                        Code < IO_BOARD_OUTB__COUNT);

    (void) OutPort;

    struct IO_BOARD *const B = (struct IO_BOARD *) Io;

    if (Value)
    {
        B->outbData |= (1 << Code);
    }
    else
    {
        B->outbData &= ~(1 << Code);
    }
}


const char * inputName (struct IO *const Io, const enum IO_Type IoType,
                        const IO_Code Code)
{
    BOARD_AssertParams (IoType == IO_Type_Bit &&
                        Code < IO_BOARD_INB__COUNT);

    (void) Io;

    return s_InputNames[Code];
}


const char * outputName (struct IO *const Io, const enum IO_Type IoType,
                         const IO_Code Code)
{
    BOARD_AssertParams (IoType == IO_Type_Bit &&
                        Code < IO_BOARD_OUTB__COUNT);

    (void) Io;

    return s_OutputNames[Code];
}
