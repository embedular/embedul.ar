/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  lpcopen board - seed generated by noise from the adc.

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

#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/shared/seed.h"


uint64_t Board_GetSeed (void)
{
    ADC_CLOCK_SETUP_T cs;

    Chip_ADC_Init               (LPC_ADC0, &cs);
    // ADC maximum sampling rate: (4.5 Mhz / 11 bits) = ~400 Khz.
    Chip_ADC_SetSampleRate      (LPC_ADC0, &cs, 400000);
    Chip_ADC_SetResolution      (LPC_ADC0, &cs, ADC_10BITS);
    Chip_ADC_EnableChannel      (LPC_ADC0, ADC_CH0, DISABLE);
    Chip_SCU_ADC_Channel_Config (0, ADC_CH0);
    Chip_ADC_EnableChannel      (LPC_ADC0, ADC_CH0, ENABLE);
    Chip_ADC_SetBurstCmd        (LPC_ADC0, DISABLE);

    uint64_t seed = 0;

    // Accumulate LSB noise from the floating ADC pin
    for (uint32_t i = 0; i < 256; ++i)
    {
        Chip_ADC_SetStartMode (LPC_ADC0, ADC_START_NOW, ADC_TRIGGERMODE_RISING);

        // Wait for the A/D conversion to complete
        while (Chip_ADC_ReadStatus(LPC_ADC0, ADC_CH0, ADC_DR_DONE_STAT) != SET)
        {
        }

        uint16_t data;
        if (Chip_ADC_ReadValue(LPC_ADC0, ADC_CH0, &data) != SUCCESS)
        {
            return 0;
        }

        seed = seed ^ ((uint64_t)data << (i % 64));
    }

    Chip_ADC_EnableChannel  (LPC_ADC0, ADC_CH0, DISABLE);
    Chip_ADC_DeInit         (LPC_ADC0);

    return seed;
}
