/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  lpcopen board - sysinit pin config helpers.

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

#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/chip_18xx_43xx/chip.h"


#define BOARD_SCU_FAST_FUNC(x)          (SCU_MODE_INACT | \
                                         SCU_MODE_HIGHSPEEDSLEW_EN | \
                                         SCU_MODE_ZIF_DIS | \
                                         SCU_MODE_FUNC ## x)

#define BOARD_SCU_FASTIBE_FUNC(x)       (BOARD_SCU_FAST_FUNC(x) | \
                                         SCU_MODE_INBUFF_EN)

#define BOARD_SCU_INZD_FUNC(x)          (SCU_MODE_INACT | \
                                         SCU_MODE_ZIF_DIS | \
                                         SCU_MODE_FUNC ## x)

#define BOARD_SCU_INIBE_FUNC(x)         (SCU_MODE_INACT | \
                                         SCU_MODE_INBUFF_EN | \
                                         SCU_MODE_FUNC ## x)

#define BOARD_SCU_INIBEZD_FUNC(x)       (BOARD_SCU_INIBE_FUNC(x) | \
                                         SCU_MODE_ZIF_DIS)
                                         
#define BOARD_SCU_PUIBE_FUNC(x)         (SCU_MODE_PULLUP | \
                                         SCU_MODE_INBUFF_EN | \
                                         SCU_MODE_FUNC ## x)

#define BOARD_SCU_PDIBE_FUNC(x)         (SCU_MODE_PULLDOWN | \
                                         SCU_MODE_INBUFF_EN | \
                                         SCU_MODE_FUNC ## x)

// GPIO output pin initially set to disabled
#define BOARD_GPIO_OUT(grp_,num_,mode_,gpio_name_) \
            BOARD_GPIO_Out (grp_, num_, mode_, \
                            BOARD_GPIO_OUT_ ## gpio_name_ ## _PORT, \
                            BOARD_GPIO_OUT_ ## gpio_name_ ## _PIN, \
                            ! BOARD_GPIO_OUT_ ## gpio_name_ ## _EN)

#define BOARD_GPIO_IN(grp_,num_,mode_,gpio_name_) \
            BOARD_GPIO_In (grp_, num_, mode_, \
                           BOARD_GPIO_IN_ ## gpio_name_ ## _PORT, \
                           BOARD_GPIO_IN_ ## gpio_name_ ## _PIN)

// Selectable Input/Output by application, output by default
#define BOARD_GPIO_IO(grp_,num_,mode_,gpio_name_) \
            BOARD_GPIO_Out (grp_, num_, mode_, \
                            BOARD_GPIO_IO_ ## gpio_name_ ## _PORT, \
                            BOARD_GPIO_IO_ ## gpio_name_ ## _PIN, \
                            ! BOARD_GPIO_IO_ ## gpio_name_ ## _EN)


void BOARD_GPIO_Out (uint8_t group, uint8_t pin, uint16_t scu_mode,
                     uint8_t gpio_port, uint8_t gpio_pin, bool gpio_state);
void BOARD_GPIO_In  (uint8_t group, uint8_t pin, uint16_t scu_mode,
                     uint8_t gpio_port, uint8_t gpio_pin);
