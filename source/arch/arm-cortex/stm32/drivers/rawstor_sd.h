/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [RAWSTOR driver] STM32 SD.

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

#include "embedul.ar/source/core/device/rawstor.h"
#include "stm32f4xx_hal.h"


/*
    TODO: design iteration on RAWSTOR_UpdateStatusMedia. SDCARD_Status is
          shared among all sdcards.
*/

#define SDCARD_Status_Media_Initializing_P1_PowerOn     0
#define SDCARD_Status_Media_Initializing_P1_NativeMode  1
#define SDCARD_Status_Media_Initializing_P1_Idle        2


struct RAWSTOR_SD
{
    struct RAWSTOR              device;
    SD_HandleTypeDef            * hsd;
    HAL_SD_CardStatusTypeDef    cardStatus;
};


void RAWSTOR_SD_Init (struct RAWSTOR_SD *const S,
                      SD_HandleTypeDef *const Hsd,
                      const uint32_t ClockEdge,
                      const uint32_t ClockBypass,
                      const uint32_t ClockPowerSave,
                      const uint32_t BusWide,
                      const uint32_t HardwareFlowControl,
                      const uint32_t ClockDiv);
