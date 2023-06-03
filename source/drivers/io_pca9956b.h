/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [IO driver] pca9956b led lighting device driver.

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
#include "embedul.ar/source/core/manager/comm.h"



#define IO_PCA9956B_PORT_COUNT          1
#define IO_PCA9956B_CHANNEL_COUNT       24


enum IO_PCA9956B_INB
{
    IO_PCA9956B_INB_OVERTEMP,
    IO_PCA9956B_INB_CHANNEL_ERROR,
    IO_PCA9956B_INB__COUNT,
};


enum IO_PCA9956B_INR
{
    IO_PCA9956B_INR_CHANNELS_SHORTED_BITFIELD = 0,
    IO_PCA9956B_INR_CHANNELS_OPEN_BITFIELD,
    IO_PCA9956B_INR__COUNT
};


enum IO_PCA9956B_OUTB
{
    IO_PCA9956B_OUTB__COUNT
};


enum IO_PCA9956B_OUTR
{
    IO_PCA9956B_OUTR_CH0_IREF = 0,
    IO_PCA9956B_OUTR_CH1_IREF,
    IO_PCA9956B_OUTR_CH2_IREF,
    IO_PCA9956B_OUTR_CH3_IREF,
    IO_PCA9956B_OUTR_CH4_IREF,
    IO_PCA9956B_OUTR_CH5_IREF,
    IO_PCA9956B_OUTR_CH6_IREF,
    IO_PCA9956B_OUTR_CH7_IREF,
    IO_PCA9956B_OUTR_CH8_IREF,
    IO_PCA9956B_OUTR_CH9_IREF,
    IO_PCA9956B_OUTR_CH10_IREF,
    IO_PCA9956B_OUTR_CH11_IREF,
    IO_PCA9956B_OUTR_CH12_IREF,
    IO_PCA9956B_OUTR_CH13_IREF,
    IO_PCA9956B_OUTR_CH14_IREF,
    IO_PCA9956B_OUTR_CH15_IREF,
    IO_PCA9956B_OUTR_CH16_IREF,
    IO_PCA9956B_OUTR_CH17_IREF,
    IO_PCA9956B_OUTR_CH18_IREF,
    IO_PCA9956B_OUTR_CH19_IREF,
    IO_PCA9956B_OUTR_CH20_IREF,
    IO_PCA9956B_OUTR_CH21_IREF,
    IO_PCA9956B_OUTR_CH22_IREF,
    IO_PCA9956B_OUTR_CH23_IREF,
    IO_PCA9956B_OUTR_CH0_PWM,
    IO_PCA9956B_OUTR_CH1_PWM,
    IO_PCA9956B_OUTR_CH2_PWM,
    IO_PCA9956B_OUTR_CH3_PWM,
    IO_PCA9956B_OUTR_CH4_PWM,
    IO_PCA9956B_OUTR_CH5_PWM,
    IO_PCA9956B_OUTR_CH6_PWM,
    IO_PCA9956B_OUTR_CH7_PWM,
    IO_PCA9956B_OUTR_CH8_PWM,
    IO_PCA9956B_OUTR_CH9_PWM,
    IO_PCA9956B_OUTR_CH10_PWM,
    IO_PCA9956B_OUTR_CH11_PWM,
    IO_PCA9956B_OUTR_CH12_PWM,
    IO_PCA9956B_OUTR_CH13_PWM,
    IO_PCA9956B_OUTR_CH14_PWM,
    IO_PCA9956B_OUTR_CH15_PWM,
    IO_PCA9956B_OUTR_CH16_PWM,
    IO_PCA9956B_OUTR_CH17_PWM,
    IO_PCA9956B_OUTR_CH18_PWM,
    IO_PCA9956B_OUTR_CH19_PWM,
    IO_PCA9956B_OUTR_CH20_PWM,
    IO_PCA9956B_OUTR_CH21_PWM,
    IO_PCA9956B_OUTR_CH22_PWM,
    IO_PCA9956B_OUTR_CH23_PWM,
    IO_PCA9956B_OUTR__COUNT,

    IO_PCA9956B_OUTR__CH_IREF_BEGIN = IO_PCA9956B_OUTR_CH0_IREF,
    IO_PCA9956B_OUTR__CH_IREF_END   = IO_PCA9956B_OUTR_CH23_IREF,
    IO_PCA9956B_OUTR__CH_PWM_BEGIN  = IO_PCA9956B_OUTR_CH0_PWM,
    IO_PCA9956B_OUTR__CH_PWM_END    = IO_PCA9956B_OUTR_CH23_PWM
};


struct IO_PCA9956B
{
    struct IO           device;
    struct IO_PortInfo  portInfo[IO_PCA9956B_PORT_COUNT];
    struct STREAM       * stream;
    uint32_t            channelsShorted;
    uint32_t            channelsOpen;
    // ch(Iref/Pwm)[0] reserved as the i2c register to write to.
    uint8_t             chIref[IO_PCA9956B_CHANNEL_COUNT + 1];
    uint8_t             chPwm[IO_PCA9956B_CHANNEL_COUNT + 1];
    uint8_t             i2cAddr;
    uint8_t             overtemp;
    uint8_t             chError;
    uint8_t             update;
};


void IO_PCA9956B_Init       (struct IO_PCA9956B *const P,
                             const enum COMM_Device ComDevice,
                             const uint8_t I2cAddr);
void IO_PCA9956B_Attach     (struct IO_PCA9956B *const P,
                             const uint32_t ChannelOffset,
                             const uint32_t DeviceOffset);
