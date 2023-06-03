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

#pragma once

#include "embedul.ar/source/core/device/io.h"


#define IO_BOARD_NUCLEO_144_PORT_COUNT  1

/*
    ST NUCLEO-144 human interface inputs (x) and outputs [x] layout.

         +-------------------+
         |     USER LEDS     |
         |     [2][1][0]     |   [0] = User led 1 (Green)
         |                   |   [1] = User led 2 (Blue)
         |                   |   [2] = User led 3 (Red)
         |                   |
         |                   |
         |                   |
         |                   |
         |                   |
         |                   |
         | USER        RESET |
         | (0)           (x) |
         +-------------------+
*/


enum IO_BOARD_NUCLEO_144_INB
{
    IO_BOARD_NUCLEO_144_INB_USER,
    IO_BOARD_NUCLEO_144_INB__COUNT
};


enum IO_BOARD_NUCLEO_144_OUTB
{
    IO_BOARD_NUCLEO_144_OUTB_LED_GREEN,
    IO_BOARD_NUCLEO_144_OUTB_LED_BLUE,
    IO_BOARD_NUCLEO_144_OUTB_LED_RED,
    IO_BOARD_NUCLEO_144_OUTB__COUNT
};


enum IO_BOARD_NUCLEO_144_INR
{
    IO_BOARD_NUCLEO_144_INR__COUNT
};


enum IO_BOARD_NUCLEO_144_OUTR
{
    IO_BOARD_NUCLEO_144_OUTR__COUNT
};


struct IO_BOARD_NUCLEO_144
{
    struct IO           device;
    struct IO_PortInfo  portInfo[IO_BOARD_NUCLEO_144_PORT_COUNT];
    uint32_t            inbData;
    uint32_t            outbData;
};


void IO_BOARD_NUCLEO_144_Init   (struct IO_BOARD_NUCLEO_144 *const B);
void IO_BOARD_NUCLEO_144_Attach (struct IO_BOARD_NUCLEO_144 *const B);
