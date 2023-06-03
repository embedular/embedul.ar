/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [IO driver] dual genesys gamepads on pca9673.

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
#include "embedul.ar/source/core/device/stream.h"
#include "embedul.ar/source/core/manager/comm.h"


#define IO_DUAL_GENESIS_PORT_COUNT      2

/*
    GENESIS (MegaDrive) Gamepad Input Layout

    Three buttons
    -------------------------------------------------------------
              UP(3)
      LEFT(1)      RIGHT(0)       (4)
            DOWN(2)              START        A(5)  B(6)  C(7)
    -------------------------------------------------------------

    Six buttons
    -------------------------------------------MODE(8)-----------
              UP(3)
      LEFT(1)      RIGHT(0)    (8)    (4)     X(9)  Y(10) Z(11)
            DOWN(2)           MODE   START    A(5)  B(6)  C(7)
    -------------------------------------------------------------
*/


enum IO_DUAL_GENESIS_INB
{
    IO_DUAL_GENESIS_INB_Right = 0,
    IO_DUAL_GENESIS_INB_Left,
    IO_DUAL_GENESIS_INB_Down,
    IO_DUAL_GENESIS_INB_Up,
    IO_DUAL_GENESIS_INB_Start,
    IO_DUAL_GENESIS_INB_A,
    IO_DUAL_GENESIS_INB_B,
    IO_DUAL_GENESIS_INB_C,
    IO_DUAL_GENESIS_INB_Mode,
    IO_DUAL_GENESIS_INB_X,
    IO_DUAL_GENESIS_INB_Y,
    IO_DUAL_GENESIS_INB_Z,
    IO_DUAL_GENESIS_INB__COUNT,
    IO_DUAL_GENESIS_INB__6Buttons_COUNT = IO_DUAL_GENESIS_INB__COUNT,
    IO_DUAL_GENESIS_INB__3Buttons_COUNT = IO_DUAL_GENESIS_INB_Mode
};


enum IO_DUAL_GENESIS_INR
{
    IO_DUAL_GENESIS_INR__COUNT
};


enum IO_DUAL_GENESIS_OUTB
{
    IO_DUAL_GENESIS_OUTB__COUNT
};


enum IO_DUAL_GENESIS_OUTR
{
    IO_DUAL_GENESIS_OUTR__COUNT
};


// This driver has no outputs
struct IO_DUAL_GENESIS_PCA9673
{
    struct IO           device;
    struct IO_PortInfo  portInfo[IO_DUAL_GENESIS_PORT_COUNT];
    uint32_t            gp1Data;
    uint32_t            gp2Data;
    struct STREAM       * stream;
    uint8_t             txData[2];
    uint8_t             rxData[2];
    #ifdef IO_DUAL_GENESIS_PCA9673_I2C_LOG_GP1
    uint8_t             log[6];
    #endif
    uint8_t             i2cAddr;
};


void IO_DUAL_GENESIS_PCA9673_Init (
                            struct IO_DUAL_GENESIS_PCA9673 *const G,
                            const enum COMM_Device ComDevice,
                            const uint8_t I2cAddr);
void IO_DUAL_GENESIS_PCA9673_Attach (
                            struct IO_DUAL_GENESIS_PCA9673 *const G);
