/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [IO driver] conicet-gicsafe "modulo de procesamiento" for trenes
              argentinos operaciones.

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

#include "embedul.ar/source/arch/arm-cortex/stm32/drivers/io_board_gicsafe_mp.h"
#include "embedul.ar/source/arch/arm-cortex/stm32/f4/boards/gicsafe_mp/cubemx/Core/Inc/gpio.h"
#include "embedul.ar/source/core/device/board.h"


#define GPIO_INB(_b,_name,_rev) \
    _b->inbData |= (_rev HAL_GPIO_ReadPin(_name ## _GPIO_Port, _name ## _Pin)? \
                        (1 << IO_BOARD_GICSAFE_MP_INB_ ## _name) : 0)

#define GPIO_OUTB(_b,_name,_rev) \
    HAL_GPIO_WritePin (_name ## _GPIO_Port, _name ## _Pin, \
            (_rev (_b->outbData & (1 << IO_BOARD_GICSAFE_MP_OUTB_ ## _name)))? \
                    GPIO_PIN_SET : GPIO_PIN_RESET)


static const char * s_InputBitNames[IO_BOARD_GICSAFE_MP_INB__COUNT] =
{
    [IO_BOARD_GICSAFE_MP_INB_SW1]       = "Button 1",
    [IO_BOARD_GICSAFE_MP_INB_SW2]       = "Button 2",
    [IO_BOARD_GICSAFE_MP_INB_SD_DET]    = "SD Detect"
};


static const char * s_OutputBitNames[IO_BOARD_GICSAFE_MP_OUTB__COUNT] =
{
    [IO_BOARD_GICSAFE_MP_OUTB_LED1]     = "LED 1",
    [IO_BOARD_GICSAFE_MP_OUTB_LED2]     = "LED 2",
    [IO_BOARD_GICSAFE_MP_OUTB_LED3]     = "LED 3",
    [IO_BOARD_GICSAFE_MP_OUTB_LED4]     = "LED 4",
    [IO_BOARD_GICSAFE_MP_OUTB_OUT1]     = "OUT 1",
    [IO_BOARD_GICSAFE_MP_OUTB_OUT2]     = "OUT 2",
    [IO_BOARD_GICSAFE_MP_OUTB_OUT3]     = "OUT 3",
    [IO_BOARD_GICSAFE_MP_OUTB_OUT4]     = "OUT 4",
    [IO_BOARD_GICSAFE_MP_OUTB_OUT5]     = "OUT 5",
    [IO_BOARD_GICSAFE_MP_OUTB_OUT6]     = "OUT 6",
};


static const char * s_InputRangeNames[IO_BOARD_GICSAFE_MP_INR__COUNT] =
{
    [IO_BOARD_GICSAFE_MP_INR_MAINS]         = "Mains",
    [IO_BOARD_GICSAFE_MP_INR_DC_GAUGE]      = "DC Gauge",
    [IO_BOARD_GICSAFE_MP_INR_AIN1]          = "AIn 1",
    [IO_BOARD_GICSAFE_MP_INR_AIN2]          = "AIn 2",
    [IO_BOARD_GICSAFE_MP_INR_MCU_TEMP]      = "MCU Temp",
    [IO_BOARD_GICSAFE_MP_INR_MCU_VREFINT]   = "MCU Vrefint",
    [IO_BOARD_GICSAFE_MP_INR_MCU_VBAT]      = "MCU Vbat",
    [IO_BOARD_GICSAFE_MP_INR_MOD1]          = "Mod 1",
    [IO_BOARD_GICSAFE_MP_INR_MOD2]          = "Mod 2",
    [IO_BOARD_GICSAFE_MP_INR_MOD3]          = "Mod 3",
    [IO_BOARD_GICSAFE_MP_INR_MOD4]          = "Mod 4",
    [IO_BOARD_GICSAFE_MP_INR_MOD5]          = "Mod 5",
    [IO_BOARD_GICSAFE_MP_INR_MOD6]          = "Mod 6",
    [IO_BOARD_GICSAFE_MP_INR_MOD7]          = "Mod 7",
    [IO_BOARD_GICSAFE_MP_INR_MOD8]          = "Mod 8",
    [IO_BOARD_GICSAFE_MP_INR_MOD9]          = "Mod 9",
    [IO_BOARD_GICSAFE_MP_INR_MOD10]         = "Mod 10",
    [IO_BOARD_GICSAFE_MP_INR_MOD11]         = "Mod 11",
    [IO_BOARD_GICSAFE_MP_INR_MOD12]         = "Mod 12",
    [IO_BOARD_GICSAFE_MP_INR_MOD13]         = "Mod 13",
    [IO_BOARD_GICSAFE_MP_INR_MOD14]         = "Mod 14",
    [IO_BOARD_GICSAFE_MP_INR_MOD15]         = "Mod 15",
    [IO_BOARD_GICSAFE_MP_INR_MOD16]         = "Mod 16"
};


static void         update              (struct IO *const Io);
static IO_Value     getInput            (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode,
                                         const IO_Port Port);
static void         setOutput           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode,
                                         const IO_Port Port,
                                         const IO_Value Value);
static const char * inputName           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode);
static const char * outputName          (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode);


static const struct IO_IFACE IO_BOARD_GICSAFE_MP_IFACE =
{
    IO_IFACE_DECLARE("gicsafe mp board io", BOARD_GICSAFE_MP),
    .Update         = update,
    .GetInput       = getInput,
    .SetOutput      = setOutput,
    .InputName      = inputName,
    .OutputName     = outputName
};


void IO_BOARD_GICSAFE_MP_Init (struct IO_BOARD_GICSAFE_MP *const B)
{
    BOARD_AssertParams (B);

    DEVICE_IMPLEMENTATION_Clear (B);

    IO_INIT_STATIC_PORT_INFO (B, BOARD_GICSAFE_MP);

    // Update once per frame (~60 Hz).
    IO_Init ((struct IO *)B, &IO_BOARD_GICSAFE_MP_IFACE, B->portInfo, 15);
}


void IO_BOARD_GICSAFE_MP_Attach (struct IO_BOARD_GICSAFE_MP *const B)
{
    BOARD_AssertParams (B);

    MIO_RegisterGateway (MIO_Dir_Input, (struct IO *)B, 0);

    MIO_MAP_INPUT_BIT (MAIN, A, IO_BOARD_GICSAFE_MP_INB_SW1);
    MIO_MAP_INPUT_BIT (MAIN, B, IO_BOARD_GICSAFE_MP_INB_SW2);
    MIO_MAP_INPUT_BIT (CONTROL, StorageDetect, IO_BOARD_GICSAFE_MP_INB_SD_DET);


    MIO_RegisterGateway (MIO_Dir_Output, (struct IO *)B, 0);

    MIO_MAP_OUTPUT_BIT (SIGN, Warning, IO_BOARD_GICSAFE_MP_OUTB_LED1);
    MIO_MAP_OUTPUT_BIT (SIGN, Red, IO_BOARD_GICSAFE_MP_OUTB_LED2);
    MIO_MAP_OUTPUT_BIT (SIGN, Green, IO_BOARD_GICSAFE_MP_OUTB_LED3);
    MIO_MAP_OUTPUT_BIT (SIGN, Blue, IO_BOARD_GICSAFE_MP_OUTB_LED4);
}


static void updateBitInput (struct IO_BOARD_GICSAFE_MP *const B)
{
    B->inbData = 0;

    GPIO_INB (B, SW1, !);
    GPIO_INB (B, SW2, !);
    GPIO_INB (B, SD_DET, !);
}


static void modSerialMode (void)
{
    HAL_GPIO_WritePin (IN_PS_GPIO_Port, IN_PS_Pin, 0);
}


static void modParallelMode (void)
{
    HAL_GPIO_WritePin (IN_PS_GPIO_Port, IN_PS_Pin, 1);
}


static void modClockPin (const bool Enable)
{
    HAL_GPIO_WritePin (IN_CLK_GPIO_Port, IN_CLK_Pin, Enable? 1 : 0);
}


static uint8_t modSerInPinRead ()
{
    return HAL_GPIO_ReadPin (IN_READ_GPIO_Port, IN_READ_Pin);
}


static void scanModule (struct IO_BOARD_GICSAFE_MP *const B,
                        const enum IO_BOARD_GICSAFE_MP_INR ModuleInr)
{
    const uint32_t Module = ModuleInr - IO_BOARD_GICSAFE_MP_INR_MOD1;

    modSerialMode ();

    for (int32_t b = 7; b >= 0; --b)
    {
        uint8_t *const Mdb = &B->modDb[b + Module * 8];

        *Mdb = *Mdb << 1;
        *Mdb |= modSerInPinRead ();

        modClockPin (true);
        // check max clock frequency

        if ((*Mdb & 0x7) == 0x7)
        {
            B->inrData[ModuleInr] |= 1 << b;
        }
        else if ((*Mdb & 0x7) == 0)
        {
            B->inrData[ModuleInr] &= ~(1 << b);
        }

        modClockPin (false);
    }

    modParallelMode ();
}


static void updateModules (struct IO_BOARD_GICSAFE_MP *const B)
{
    for (enum IO_BOARD_GICSAFE_MP_INR m = IO_BOARD_GICSAFE_MP_INR_MOD1;
         m <= IO_BOARD_GICSAFE_MP_INR_MOD16; ++m)
    {
        scanModule (B, m);
    }
}


static void updateRangeInput (struct IO_BOARD_GICSAFE_MP *const B)
{
    (void) B;

    // TODO:
    //
    // 1) Read ADC1_10, 11, 12, 13.
    // 2) Read MCU temp, Vrefint, Vbat.
    // 3) Adjust values accordingly.
    // 4) Store in inrData[inr].

    updateModules (B);
}


static void updateBitOutput (struct IO_BOARD_GICSAFE_MP *const B)
{
    GPIO_OUTB (B, LED1,);
    GPIO_OUTB (B, LED2,);
    GPIO_OUTB (B, LED3,);
    GPIO_OUTB (B, LED4,);
    GPIO_OUTB (B, OUT1,);
    GPIO_OUTB (B, OUT2,);
    GPIO_OUTB (B, OUT3,);
    GPIO_OUTB (B, OUT4,);
    GPIO_OUTB (B, OUT5,);
    GPIO_OUTB (B, OUT6,);
}


void update (struct IO *const Io)
{
    struct IO_BOARD_GICSAFE_MP *const B = (struct IO_BOARD_GICSAFE_MP *) Io;

    updateBitInput      (B);
    updateRangeInput    (B);
    updateBitOutput     (B);
}


IO_Value getInput (struct IO *const Io, const enum IO_Type IoType,
                   const IO_Code DriverCode, const IO_Port Port)
{
    (void) Port;

    struct IO_BOARD_GICSAFE_MP *const B = (struct IO_BOARD_GICSAFE_MP *) Io;

    if (IoType == IO_Type_Bit)
    {
        return (B->inbData & (1 << DriverCode)); 
    }

    // IoType == IO_Type_Range
    return B->inrData[DriverCode];
}


void setOutput (struct IO *const Io, const enum IO_Type IoType,
                const IO_Code DriverCode, const IO_Port Port,
                const IO_Value Value)
{
    (void) IoType;
    (void) Port;

    struct IO_BOARD_GICSAFE_MP *const B = (struct IO_BOARD_GICSAFE_MP *) Io;

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

    if (IoType == IO_Type_Bit)
    {
        return s_InputBitNames[DriverCode];
    }

    return s_InputRangeNames[DriverCode];
}


const char * outputName (struct IO *const Io, const enum IO_Type IoType,
                         const IO_Code DriverCode)
{
    (void) Io;
    (void) IoType;

    return s_OutputBitNames[DriverCode];
}
