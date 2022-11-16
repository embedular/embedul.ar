/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  st nucleo-144 board bsp code selection by mcu family.

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

#if defined (STM32_F4)
    #include "Drivers/BSP/STM32F4xx_Nucleo_144/stm32f4xx_nucleo_144.h"
#elif defined (STM32_F7)
    #include "Drivers/BSP/STM32F7xx_Nucleo_144/stm32f7xx_nucleo_144.h"
#elif defined (STM32_H7)
    #include "Drivers/BSP/STM32H7xx_Nucleo_144/stm32h7xx_nucleo_144.h"
#else
    #error Unsupported STM32 family
#endif 
