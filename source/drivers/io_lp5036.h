/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [IO driver] lp5036 rgb led driver device controller.

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
#include "embedul.ar/source/core/manager/comm.h"


#define IO_LP5036_PORT_COUNT            1


enum IO_LP5036_INB
{
    IO_LP5036_INB__COUNT
};


enum IO_LP5036_INR
{
    IO_LP5036_INR__COUNT
};


enum IO_LP5036_OUTB
{
    IO_LP5036_OUTB__COUNT
};

/*
    36 output, 8-bit channels.
*/
enum IO_LP5036_OUTR
{
    IO_LP5036_OUTR_0 = 0,
    IO_LP5036_OUTR_1,
    IO_LP5036_OUTR_2,
    IO_LP5036_OUTR_3,
    IO_LP5036_OUTR_4,
    IO_LP5036_OUTR_5,
    IO_LP5036_OUTR_6,
    IO_LP5036_OUTR_7,
    IO_LP5036_OUTR_8,
    IO_LP5036_OUTR_9,
    IO_LP5036_OUTR_10,
    IO_LP5036_OUTR_11,
    IO_LP5036_OUTR_12,
    IO_LP5036_OUTR_13,
    IO_LP5036_OUTR_14,
    IO_LP5036_OUTR_15,
    IO_LP5036_OUTR_16,
    IO_LP5036_OUTR_17,
    IO_LP5036_OUTR_18,
    IO_LP5036_OUTR_19,
    IO_LP5036_OUTR_20,
    IO_LP5036_OUTR_21,
    IO_LP5036_OUTR_22,
    IO_LP5036_OUTR_23,
    IO_LP5036_OUTR_24,
    IO_LP5036_OUTR_25,
    IO_LP5036_OUTR_26,
    IO_LP5036_OUTR_27,
    IO_LP5036_OUTR_28,
    IO_LP5036_OUTR_29,
    IO_LP5036_OUTR_30,
    IO_LP5036_OUTR_31,
    IO_LP5036_OUTR_32,
    IO_LP5036_OUTR_33,
    IO_LP5036_OUTR_34,
    IO_LP5036_OUTR_35,
    IO_LP5036_OUTR__COUNT
};


struct IO_LP5036
{
    struct IO           device;
    struct IO_PortInfo  portInfo[IO_LP5036_PORT_COUNT];
    struct STREAM       * stream;
    // outData[0] reserved as the i2c register to write to.
    uint8_t             outData[IO_LP5036_OUTR__COUNT + 1];
    uint8_t             i2cAddr;
    uint8_t             maxIntensity;
};


void IO_LP5036_Init (struct IO_LP5036 *const L,
                     const enum COMM_Device ComDevice,
                     const uint8_t I2cAddr, const uint8_t MaxIntensity);
