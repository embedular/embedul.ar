/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  lpc43xx generic software video adapter.

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

// Includes corresponding LPCOpen board depending on selected target path
#include "board.h"
#include "embedul.ar/source/core/cc.h"
// Modeline selected by the project makefile
#include CC_ExpStr(VIDEO_ADAPTER_MODELINE)
#include "embedul.ar/source/arch/arm-cortex/shared/m0_instdelay.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/video_dualcore/exchange.h"
#include "embedul.ar/source/core/misc/modeline.h"
#include <stdbool.h>


#ifndef VIDEO_ADAPTER_GPIO_MASK
    #error Undefined Pixel GPIO Mask (VIDEO_ADAPTER_GPIO_MASK)
#endif

#ifndef VIDEO_ADAPTER_GPIO_MASK_ADDR
    #error Undefined Pixel GPIO Mask Address (VIDEO_ADAPTER_GPIO_MASK_ADDR)
#endif

#ifndef VIDEO_ADAPTER_GPIO_MPIN_ADDR
    #error Undefined Pixel GPIO Masked Pin Address \
            (VIDEO_ADAPTER_GPIO_MPIN_ADDR)
#endif

// Framebuffer line repetitions needed to fill the vertical signal resolution.
// WARNING: MODELINE_VDISP must be multiple of VIDEO_FB_WIDTH
#define VIDEO_ADAPTER_FB_LINE_DUP       (MODELINE_VDISP / VIDEO_FB_HEIGHT)

// IPP = MCU instructions per video signal pixel.
#define VIDEO_ADAPTER_IPP               (204000.0f / (float)MODELINE_PCLK_KHZ)

#ifndef VIDEO_ADAPTER_HSYNC_RIT_TUNE
    #define VIDEO_ADAPTER_HSYNC_RIT_TUNE        0
#endif

#ifndef VIDEO_ADAPTER_HSYNC_IC_TUNE
    #define VIDEO_ADAPTER_HSYNC_IC_TUNE         0
#endif

#ifndef VIDEO_ADAPTER_BACK_PORCH_IC_TUNE
    #define VIDEO_ADAPTER_BACK_PORCH_IC_TUNE    0
#endif

#define VIDEO_ADAPTER_HSYNC_IC          ((uint32_t)(MODELINE_HSYNC * \
                                        VIDEO_ADAPTER_IPP) + \
                                            VIDEO_ADAPTER_HSYNC_IC_TUNE)
#define VIDEO_ADAPTER_BACK_PORCH_IC     ((uint32_t)(MODELINE_BACK_PORCH * \
                                        VIDEO_ADAPTER_IPP) + \
                                            VIDEO_ADAPTER_BACK_PORCH_IC_TUNE)

// usage example: VIDEO_SET_SYNC_SIGNAL(HSYNC, ON)
#define VIDEO_ADAPTER_SET_SYNC_SIGNAL(x,y) \
    BOARD_GPIO_FAST_WRITE (OUT_ ## x, MODELINE_ ## x ## _ ## y)

#ifdef BOARD_ADAPTER_GPIO_OUT_RGB_EN_EN
    #define VIDEO_ADAPTER_RGB_ON        BOARD_GPIO_OUT_RGB_EN_EN
    #define VIDEO_ADAPTER_RGB_OFF       (! BOARD_GPIO_OUT_RGB_EN_EN)
    #define VIDEO_ADAPTER_RGB(x)        BOARD_GPIO_FAST_WRITE( \
                                            OUT_RGB_EN, VIDEO_RGB_ ## x)
#else
    #define VIDEO_ADAPTER_RGB(x)
#endif

#define VIDEO_ADAPTER_LT_STR            \
                            VIDEO_ADAPTER_STR " " VIDEO_ADAPTER_LINEOUT_STR \
                            " " CC_ExpStr(VIDEO_ADAPTER_HSYNC_RIT_TUNE) \
                            " " CC_ExpStr(VIDEO_ADAPTER_HSYNC_IC_TUNE) \
                            " " CC_ExpStr(VIDEO_ADAPTER_BACK_PORCH_IC_TUNE)


extern void     VIDEO_ADAPTER_LineOut       (const uint32_t FbLine,
                                             const uint32_t AndValue,
                                             const uint32_t OrValue);
bool            VIDEO_ADAPTER_Init          (void);
void            VIDEO_ADAPTER_Vbi           (void);
