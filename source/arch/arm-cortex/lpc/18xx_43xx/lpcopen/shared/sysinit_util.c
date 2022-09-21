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

#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/shared/sysinit_util.h"


void BOARD_GPIO_Out (uint8_t group, uint8_t pin, uint16_t scu_mode,
                     uint8_t gpio_port, uint8_t gpio_pin, bool gpio_state)
{
    Chip_SCU_PinMuxSet          (group, pin, scu_mode);
    Chip_GPIO_SetPinDIROutput   (LPC_GPIO_PORT, gpio_port, gpio_pin);
    Chip_GPIO_SetPinState       (LPC_GPIO_PORT, gpio_port, gpio_pin, 
                                 gpio_state);
}


void BOARD_GPIO_In (uint8_t group, uint8_t pin, uint16_t scu_mode,
                    uint8_t gpio_port, uint8_t gpio_pin)
{
    Chip_SCU_PinMuxSet          (group, pin, scu_mode);
    Chip_GPIO_SetPinDIRInput    (LPC_GPIO_PORT, gpio_port, gpio_pin);
}
