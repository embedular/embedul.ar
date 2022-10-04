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

#pragma once

#include "embedul.ar/source/core/device/io.h"


#define IO_DUAL_NES_VIDEOEX_PORT_COUNT  2


/*
    NES Gamepad Input Layout
    ---------------------------------------------------------
              UP(3)
      LEFT(1)      RIGHT(0)     (4)     (5)      (6)    (7)
            DOWN(2)           SELECT   START      B      A
    ---------------------------------------------------------
*/


enum IO_DUAL_NES_VIDEOEX_INB
{
    IO_DUAL_NES_VIDEOEX_INB_Right = 0,
    IO_DUAL_NES_VIDEOEX_INB_Left,
    IO_DUAL_NES_VIDEOEX_INB_Down,
    IO_DUAL_NES_VIDEOEX_INB_Up,
    IO_DUAL_NES_VIDEOEX_INB_Start,
    IO_DUAL_NES_VIDEOEX_INB_Select,
    IO_DUAL_NES_VIDEOEX_INB_B,
    IO_DUAL_NES_VIDEOEX_INB_A,
    IO_DUAL_NES_VIDEOEX_INB__COUNT
};


enum IO_DUAL_NES_VIDEOEX_INR
{
    IO_DUAL_NES_VIDEOEX_INR__COUNT
};


enum IO_DUAL_NES_VIDEOEX_OUTB
{
    IO_DUAL_NES_VIDEOEX_OUTB__COUNT
};


enum IO_DUAL_NES_VIDEOEX_OUTR
{
    IO_DUAL_NES_VIDEOEX_OUTR__COUNT
};


// The driver expects periodically updated raw gamepad status data at
// g_ddaExchange.io1/.io2.
struct IO_DUAL_NES_VIDEOEX
{
    struct IO           device;
    struct IO_PortInfo  portInfo[IO_DUAL_NES_VIDEOEX_PORT_COUNT];
    uint32_t            gp1Data;
    uint32_t            gp2Data;
};


void IO_DUAL_NES_VIDEOEX_Init (struct IO_DUAL_NES_VIDEOEX *const N);
void IO_DUAL_NES_VIDEOEX_Attach (struct IO_DUAL_NES_VIDEOEX *const N);
