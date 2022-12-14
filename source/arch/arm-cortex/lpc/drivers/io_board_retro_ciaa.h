/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [IO driver] RETRO-CIAA standalone board.

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
#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/boards/retro_ciaa/board.h"


#define IO_BOARD_RETRO_CIAA_PORT_COUNT             1

/*
    RETRO-CIAA Standalone human interface input (x) and output [x] layout.

         +------------------------------+
         |                              |
         |                              |
         |                              |
         |  [0]                         |
         |  WARN                        |
         |  (1)                         |
         |  WAKEUP                      |
         |  (x)    (0)                  |
         |  RESET  ISP                  |
         +------------------------------+
*/


enum IO_BOARD_RETRO_CIAA_INB
{
    IO_BOARD_RETRO_CIAA_INB_ISP = 0,
    IO_BOARD_RETRO_CIAA_INB_WAKEUP,
    IO_BOARD_RETRO_CIAA_INB_BOARD_BACKLIGHT,
    IO_BOARD_RETRO_CIAA_INB_SD_POW,
    IO_BOARD_RETRO_CIAA_INB_WIFI_EN,
    IO_BOARD_RETRO_CIAA_INB_SOUND_MUTE,
    IO_BOARD_RETRO_CIAA_INB__COUNT
};


enum IO_BOARD_RETRO_CIAA_OUTB
{
    IO_BOARD_RETRO_CIAA_OUTB_LED_WARN = 0,
    IO_BOARD_RETRO_CIAA_OUTB_BOARD_BACKLIGHT,
    IO_BOARD_RETRO_CIAA_OUTB_SD_POW,
    IO_BOARD_RETRO_CIAA_OUTB_WIFI_EN,
    IO_BOARD_RETRO_CIAA_OUTB_SOUND_MUTE,
    IO_BOARD_RETRO_CIAA_OUTB__COUNT
};


enum IO_BOARD_RETRO_CIAA_INR
{
    IO_BOARD_RETRO_CIAA_INR__COUNT
};


enum IO_BOARD_RETRO_CIAA_OUTR
{
    IO_BOARD_RETRO_CIAA_OUTR__COUNT
};


struct IO_BOARD_RETRO_CIAA
{
    struct IO           device;
    struct IO_PortInfo  portInfo[IO_BOARD_RETRO_CIAA_PORT_COUNT];
    uint32_t            inbData;
    uint32_t            outbData;
    volatile enum CHIP_EVRT_SRC_ACTIVE
                        wakeupActive;
};



void IO_BOARD_RETRO_CIAA_Init       (struct IO_BOARD_RETRO_CIAA *const B);
void IO_BOARD_RETRO_CIAA_Attach     (struct IO_BOARD_RETRO_CIAA *const B);
