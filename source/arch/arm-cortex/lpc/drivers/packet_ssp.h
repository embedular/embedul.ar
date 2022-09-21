/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [PACKET driver] lpc43xx spi communication.

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

#include "embedul.ar/source/core/device/packet.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/chip_18xx_43xx/chip.h"


enum PACKET_SSP_FrameFmt
{
    PACKET_SSP_FrameFmt_Spi = 0,
    PACKET_SSP_FrameFmt_Ti,
    PACKET_SSP_FrameFmt_Microwire,
    PACKET_SSP_FrameFmt__COUNT
};


enum PACKET_SSP_Role
{
    PACKET_SSP_Role_Controller = 0,
    PACKET_SSP_Role_Peripheral
};


enum PACKET_SSP_ClockFmt
{
    PACKET_SSP_ClockFmt_Cpha0_Cpol0 = 0,
    PACKET_SSP_ClockFmt_Cpha0_Cpol1 = 1,
    PACKET_SSP_ClockFmt_Cpha1_Cpol0 = 2,
    PACKET_SSP_ClockFmt_Cpha1_Cpol1 = 3
};


struct PACKET_SSP
{
    struct PACKET               device;
    LPC_SSP_T                   * ssp;
    enum PACKET_SSP_Role        role;
    enum PACKET_SSP_FrameFmt    frameFmt;
    uint32_t                    speed;
    uint8_t                     bits;
    enum PACKET_SSP_ClockFmt    clockFmt;
    Chip_SSP_DATA_SETUP_T       ds;
};


void PACKET_SSP_Init (struct PACKET_SSP *const S,
                      LPC_SSP_T *const Ssp, 
                      const enum PACKET_SSP_Role Role,
                      const enum PACKET_SSP_FrameFmt FrameFmt,
                      const uint32_t Speed,
                      const uint8_t Bits,
                      const enum PACKET_SSP_ClockFmt ClockFmt);
