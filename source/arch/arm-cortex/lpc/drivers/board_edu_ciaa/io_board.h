/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [IO driver] EDU-CIAA-NXP board.

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


#define IO_BOARD_PORT_COUNT             1

/*
    EDU-CIAA-NXP input (x) and output [x] layout
    (No additional board switches on RETRO-CIAA expansion)

         +-------------------------+
         |                         |
         |                         |
      //////////////////////////////////

      //////////////////////////////////
         |                         |
         |  RED [0]                |
         |  GREEN [1]              |
         |  BLUE [2]               |
         |   |    [3]   [4]   [5]  |
         |  RGB  LED_1 LED_2 LED_3 |
         |                         |
         |  (0)   (1)   (2)   (3)  |
         | TEC_1 TEC_2 TEC_3 TEC_4 |
         |                         |
         +-------------------------+


    RETRO-CIAA expansion output [x] layout
    (Logo backlight)

         +-------------------------+
         |                         |
         |                         |
      //////////////////////////////////


      //////////////////////////////////
         |                         |
         |       RETRO-CIAA        |
         |    [6]           [7]    |
         |   LED_Z         LED_Y   |
         |                         |
         |                         |
         |                         |
         |                         |
         |                         |
         |                         |
         +-------------------------+ 
*/

enum IO_BOARD_INB
{
    IO_BOARD_INB_TEC_1 = 0,
    IO_BOARD_INB_TEC_2,
    IO_BOARD_INB_TEC_3,
    IO_BOARD_INB_TEC_4,
#ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    IO_BOARD_INB_SD_DETECT,
    IO_BOARD_INB_WIFI_EN,
    IO_BOARD_INB_SOUND_MUTE,
#endif
    IO_BOARD_INB__COUNT
};


enum IO_BOARD_OUTB
{
    IO_BOARD_OUTB_LED_RGB_RED = 0,
    IO_BOARD_OUTB_LED_RGB_GREEN,
    IO_BOARD_OUTB_LED_RGB_BLUE,
    IO_BOARD_OUTB_LED_1,
    IO_BOARD_OUTB_LED_2,
    IO_BOARD_OUTB_LED_3,
#ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    IO_BOARD_OUTB_BOARD_BACKLIGHT,
    IO_BOARD_OUTB_SD_SELECT,
    IO_BOARD_OUTB_WIFI_EN,
    IO_BOARD_OUTB_SOUND_MUTE,
#endif
    IO_BOARD_OUTB__COUNT
};


enum IO_BOARD_INR
{
    IO_BOARD_INR__COUNT
};


enum IO_BOARD_OUTR
{
    IO_BOARD_OUTR__COUNT
};


struct IO_BOARD
{
    struct IO           device;
    struct IO_PortInfo  portInfo[IO_BOARD_PORT_COUNT];
    uint32_t            inbData;
    uint32_t            outbData;
};


void IO_BOARD_Init      (struct IO_BOARD *const B);
void IO_BOARD_Attach    (struct IO_BOARD *const B);
