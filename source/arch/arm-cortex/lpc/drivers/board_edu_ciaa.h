/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [BOARD driver] EDU-CIAA-NXP w/support for RETRO-CIAA poncho.

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
#include "embedul.ar/source/drivers/packet_esp32at_tcp_server.h"
#include "embedul.ar/source/drivers/rawstor_sd_1bit.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/board_edu_ciaa/io_board.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/stream_usart.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/packet_ssp.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/io_dual_nes_videoex.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/video_dualcore.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/sound_pcm5100.h"


struct BOARD_IO_PROFILES
{
    struct INPUT_PROFILE_MAIN               inMain;
    struct OUTPUT_PROFILE_SIGN              outSign;
#ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    struct INPUT_PROFILE_GP1                inGp1;
    struct INPUT_PROFILE_GP2                inGp2;
    struct INPUT_PROFILE_CONTROL            inControl;
    struct OUTPUT_PROFILE_CONTROL           outControl;
#endif
};


struct BOARD_EDU_CIAA
{
    struct BOARD                            device;
    struct IO_BOARD                         ioBoard;
    struct RANDOM_SFMT                      randomSfmt;
    struct STREAM_USART                     streamDebugUsart;
    struct STREAM_USART                     streamExtUsart;
    struct PACKET_SSP                       packetSsp;
#ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    struct IO_DUAL_NES_VIDEOEX              ioDualNes;
    struct PACKET_ESP32AT_TCP_SERVER        packetEsp32Tcp;
    struct RAWSTOR_SD_1BIT                  rawstorSd1Bit;
    struct VIDEO_DUALCORE                   videoDualcore;
    struct SOUND_PCM5100                    soundPcm5100;
    uint8_t                                 tcpServerInBuffer[64];
    uint8_t                                 tcpServerOutBuffer[1024];
#endif
    uint8_t                                 debugInBuffer[16];
    uint8_t                                 debugOutBuffer[16];
};
