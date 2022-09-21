/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [STREAM driver] lpcopen usart.

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
#include "embedul.ar/source/core/device/stream.h"
#include "embedul.ar/source/core/cyclic.h"


#define STREAM_USART_DEFAULT_BAUD_RATE      115200
#define STREAM_USART_DEFAULT_LCR_FLAGS      (UART_LCR_WLEN8 | \
                                                UART_LCR_PARITY_DIS | \
                                                UART_LCR_SBS_1BIT)


struct STREAM_USART
{
    struct STREAM       device;
    struct CYCLIC       sendCb;
    struct CYCLIC       recvCb;
    LPC_USART_T         * usart;
    uint32_t            irq;
    uint32_t            baud;
    uint32_t            lcrFlags;
    bool                hwFlowCtrl;
};


void STREAM_USART0_Init (struct STREAM_USART *const U,
                         uint8_t *const InBuffer, const uint32_t InOctets,
                         uint8_t *const OutBuffer, const uint32_t OutOctets,
                         const uint32_t Baud, const uint32_t LcrFlags);
void STREAM_UART1_Init  (struct STREAM_USART *const U,
                         uint8_t *const InBuffer, const uint32_t InOctets,
                         uint8_t *const OutBuffer, const uint32_t OutOctets,
                         const uint32_t Baud, const uint32_t LcrFlags);
void STREAM_USART2_Init (struct STREAM_USART *const U,
                         uint8_t *const InBuffer, const uint32_t InOctets,
                         uint8_t *const OutBuffer, const uint32_t OutOctets,
                         const uint32_t Baud, const uint32_t LcrFlags);
void STREAM_USART3_Init (struct STREAM_USART *const U,
                         uint8_t *const InBuffer, const uint32_t InOctets,
                         uint8_t *const OutBuffer, const uint32_t OutOctets,
                         const uint32_t Baud, const uint32_t LcrFlags);
