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

#pragma once

#include "embedul.ar/source/core/device/io.h"


#define IO_BOARD_GICSAFE_MP_PORT_COUNT      1
#define IO_BOARD_GICSAFE_MP_INR_MOD_COUNT   16

/*
    CONICET-GICSAFe "Modulo de Procesamiento" for TRENES ARGENTINOS OPERACIONES.
    Bit inputs (x), Bit outputs [x], and Range inputs <x> layout shown below.

    +-------------------------------------+
    |                              <3>AIn2|
    |                              <2>AIn1|
    |[3][2][1][0]                     <0>~|
    |                                   - |
    |J10<7-22>             <1>DC_In<---{ }|
    |J9[4-10]                           + |
    |          +----+                     |
    |  (0) (1) |(2) |                     |
    |   B1  B2 | SD_DET                   |
    +----------+----+---------------------+   

        [0] = LED1 (red)
        [1] = LED2 (red)
        [2] = LED3 (red)
        [3] = LED4 (red)
        [4-10] = J9 Connector: Discrete digital outputs 1-6.

        (0) = SW1/B1
        (1) = SW2/B2
        (2) = SD_DET, sd socket card detect switch.

        <0> = Mains to 6V transformer input.
        <1> = DC In power gauge, 24V nominal. 
        <2> = Analog input 1 (4-20ma or 0-10V)
        <3> = Analog input 2 (4-20ma or 0-10V)
        <4> = [MCU internal] Temperature.
        <5> = [MCU internal] VRef.
        <6> = [MCU internal] VBAT.
        <7-22> = J10 Connector: Serial input modules 1-16 (eight bits each).
*/


enum IO_BOARD_GICSAFE_MP_INB
{
    IO_BOARD_GICSAFE_MP_INB_SW1,
    IO_BOARD_GICSAFE_MP_INB_SW2,
    IO_BOARD_GICSAFE_MP_INB_SD_DET,
    IO_BOARD_GICSAFE_MP_INB__COUNT
};


enum IO_BOARD_GICSAFE_MP_OUTB
{
    IO_BOARD_GICSAFE_MP_OUTB_LED1,
    IO_BOARD_GICSAFE_MP_OUTB_LED2,
    IO_BOARD_GICSAFE_MP_OUTB_LED3,
    IO_BOARD_GICSAFE_MP_OUTB_LED4,
    IO_BOARD_GICSAFE_MP_OUTB_OUT1,
    IO_BOARD_GICSAFE_MP_OUTB_OUT2,
    IO_BOARD_GICSAFE_MP_OUTB_OUT3,
    IO_BOARD_GICSAFE_MP_OUTB_OUT4,
    IO_BOARD_GICSAFE_MP_OUTB_OUT5,
    IO_BOARD_GICSAFE_MP_OUTB_OUT6,
    IO_BOARD_GICSAFE_MP_OUTB__COUNT
};


enum IO_BOARD_GICSAFE_MP_INR
{
    IO_BOARD_GICSAFE_MP_INR_MAINS,
    IO_BOARD_GICSAFE_MP_INR_DC_GAUGE,
    IO_BOARD_GICSAFE_MP_INR_AIN1,
    IO_BOARD_GICSAFE_MP_INR_AIN2,
    IO_BOARD_GICSAFE_MP_INR_MCU_TEMP,
    IO_BOARD_GICSAFE_MP_INR_MCU_VREFINT,
    IO_BOARD_GICSAFE_MP_INR_MCU_VBAT,
    IO_BOARD_GICSAFE_MP_INR_MOD1,
    IO_BOARD_GICSAFE_MP_INR_MOD2,
    IO_BOARD_GICSAFE_MP_INR_MOD3,
    IO_BOARD_GICSAFE_MP_INR_MOD4,
    IO_BOARD_GICSAFE_MP_INR_MOD5,
    IO_BOARD_GICSAFE_MP_INR_MOD6,
    IO_BOARD_GICSAFE_MP_INR_MOD7,
    IO_BOARD_GICSAFE_MP_INR_MOD8,
    IO_BOARD_GICSAFE_MP_INR_MOD9,
    IO_BOARD_GICSAFE_MP_INR_MOD10,
    IO_BOARD_GICSAFE_MP_INR_MOD11,
    IO_BOARD_GICSAFE_MP_INR_MOD12,
    IO_BOARD_GICSAFE_MP_INR_MOD13,
    IO_BOARD_GICSAFE_MP_INR_MOD14,
    IO_BOARD_GICSAFE_MP_INR_MOD15,
    IO_BOARD_GICSAFE_MP_INR_MOD16,
    IO_BOARD_GICSAFE_MP_INR__COUNT
};


enum IO_BOARD_GICSAFE_MP_OUTR
{
    IO_BOARD_GICSAFE_MP_OUTR__COUNT
};


struct IO_BOARD_GICSAFE_MP
{
    struct IO           device;
    struct IO_PortInfo  portInfo[IO_BOARD_GICSAFE_MP_PORT_COUNT];
    uint32_t            inbData;
    uint32_t            outbData;
    IO_Value            inrData[IO_BOARD_GICSAFE_MP_INR__COUNT];
    uint8_t             modDb[IO_BOARD_GICSAFE_MP_INR_MOD_COUNT * 8];
};


void IO_BOARD_GICSAFE_MP_Init   (struct IO_BOARD_GICSAFE_MP *const B);
void IO_BOARD_GICSAFE_MP_Attach (struct IO_BOARD_GICSAFE_MP *const B);
