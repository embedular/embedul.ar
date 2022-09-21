/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [BOARD driver] RETRO-CIAA standalone.

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

#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/drivers/random_sfmt.h"
#include "embedul.ar/source/drivers/io_dual_genesis_pca9673.h"
#include "embedul.ar/source/drivers/io_lp5036.h"
#include "embedul.ar/source/drivers/io_huewheel.h"
#include "embedul.ar/source/drivers/io_pca9956b.h"
#include "embedul.ar/source/drivers/packet_esp32at_tcp_server.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/board_retro_ciaa/io_board.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/stream_usart.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/packet_i2c_controller.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/rawstor_sd_sdmmc.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/video_dualcore.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/sound_pcm5100.h"


struct BOARD_RETRO_CIAA
{
    struct BOARD                            device;
    struct IO_BOARD                         ioBoard;
    struct RANDOM_SFMT                      randomSfmt;
    struct STREAM_USART                     streamDebugUsart;
#ifndef BOARD_RETRO_CIAA_DISABLE_TCP_SERVER
    struct STREAM_USART                     streamEsp32atUsart;
    struct PACKET_ESP32AT_TCP_SERVER        packetEsp32Tcp;
#endif
    struct PACKET_I2C_CONTROLLER            packetI2cExpController;
    struct IO_DUAL_GENESIS_PCA9673          ioDualGenesis;
    struct IO_LP5036                        ioLp5036;
    struct IO_HUEWHEEL                      ioHuewheel;
    struct IO_PCA9956B                      ioPca9956BDeviceA;
    struct IO_PCA9956B                      ioPca9956BDeviceB;
    struct RAWSTOR_SD_SDMMC                 rawstorSdmmc;
    struct VIDEO_DUALCORE                   videoDualcore;
    struct SOUND_PCM5100                    soundPcm5100;
    uint8_t                                 debugInBuffer[16];
    uint8_t                                 debugOutBuffer[16];
#ifndef BOARD_RETRO_CIAA_DISABLE_TCP_SERVER
    uint8_t                                 tcpServerInBuffer[64];
    uint8_t                                 tcpServerOutBuffer[1024];
#endif
};
