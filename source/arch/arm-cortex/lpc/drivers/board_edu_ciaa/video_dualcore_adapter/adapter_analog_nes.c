/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  Display-dualcore specialization to analog signal output and NES gamepads
  reading.

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

#include "embedul.ar/source/arch/arm-cortex/lpc/video_dualcore_adapter/adapter.h"

/*
    JLink: Flash to appropiate lpc43xx base address with JLinkExe:
    J-Link>loadbin {signal}.bin,0x1b000000
    with {signal} being, for example, "EDID_1280x720_60Hz".
*/

// Latch pulse width >= 200 ns. 4.9 ns per instruction -> 41 instructions.
#define VIDEO_JOY_LATCH_IC              41
// Clock pulse width >= 200 ns.
#define VIDEO_JOY_CLOCK_IC              VIDEO_JOY_LATCH_IC

// usage example: SET_JOY_SIGNAL(CLOCK, HIGH)
#define VIDEO_SET_JOY_SIGNAL(x,y)       BOARD_GPIO_FAST_WRITE( \
                                            OUT_JOY_ ## x, \
                                            BOARD_GPIO_ ## y)

// usage example: var = GET_JOY_DATA(JOY1)
#define VIDEO_GET_JOY_DATA(x)           BOARD_GPIO_FAST_ACCESS( \
                                            IN_ ## x ## _DATA)


// NES Gamepad processed at VBI
static uint8_t s_vGamepad1 = 0xFF;
static uint8_t s_vGamepad2 = 0xFF;


static bool initNESGamepads (void)
{
    VIDEO_SET_JOY_SIGNAL (CLOCK, LOW);
    VIDEO_SET_JOY_SIGNAL (LATCH, LOW);

    // Gamepad status is always set by the adapter
    g_videoExchange.io.d0 = s_vGamepad1;
    g_videoExchange.io.d1 = s_vGamepad2;

    return true;
}


bool VIDEO_ADAPTER_Init (void)
{
    return initNESGamepads ();
}


void VIDEO_ADAPTER_Vbi (void)
{
    VIDEO_SET_JOY_SIGNAL (LATCH, HIGH);
    M0_InstDelay (VIDEO_JOY_LATCH_IC);
    VIDEO_SET_JOY_SIGNAL (LATCH, LOW);
    M0_InstDelay (VIDEO_JOY_LATCH_IC);

    s_vGamepad1 = VIDEO_GET_JOY_DATA (JOY1);
    s_vGamepad2 = VIDEO_GET_JOY_DATA (JOY2);

    for (uint32_t i = 0; i < 7; ++i)
    {
        VIDEO_SET_JOY_SIGNAL (CLOCK, HIGH);
        M0_InstDelay (VIDEO_JOY_CLOCK_IC);
        VIDEO_SET_JOY_SIGNAL (CLOCK, LOW);
        M0_InstDelay (VIDEO_JOY_CLOCK_IC);

        s_vGamepad1 <<= 1;
        s_vGamepad1 |= VIDEO_GET_JOY_DATA (JOY1) & 0x01;
        s_vGamepad2 <<= 1;
        s_vGamepad2 |= VIDEO_GET_JOY_DATA (JOY2) & 0x01;
    }

    g_videoExchange.io.d0 = s_vGamepad1;
    g_videoExchange.io.d1 = s_vGamepad2;
}
