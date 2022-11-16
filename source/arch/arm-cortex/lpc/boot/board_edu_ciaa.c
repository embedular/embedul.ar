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


#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/drivers/random_sfmt.h"
#include "embedul.ar/source/drivers/packet_esp32at_tcp_server.h"
#include "embedul.ar/source/drivers/rawstor_sd_1bit.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/io_board_edu_ciaa.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/stream_usart.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/packet_ssp.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/io_dual_nes_videoex.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/video_dualcore.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/sound_pcm5100.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/boot/shared_iface.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/boards/edu_ciaa/board.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/shared/seed.h"


#define BOARD_LOGO_1            "`F22,ad8888ba,    88`P9db  db`L"
#define BOARD_LOGO_2            "`F21d8\"'`P4``\"8b   88`P8d88b  8b`L"
#define BOARD_LOGO_3            "`F20d8'`P1388`P7d8'``8b ``8b`L"
#define BOARD_LOGO_4            "`F2088`P1488`P6d8'  ``8b ``8b`L"
#define BOARD_LOGO_5            "`F2088`P1488`P5d8YaaaaY8b  8b`L"
#define BOARD_LOGO_6            "`F20Y8,`P1388    d8\"\"\"\"\"\"\"\"8b  8b`L"
#define BOARD_LOGO_7            "`F21Y8a.`P4.a8P   88   d8'`P8``8b ``8b`L"
#define BOARD_LOGO_8            "`F22``\"Y8888Y\"'    88  d8'`P10``8b ``8b`L1"

#define BOARD_INFO_FMT          "`M40`0`L1" \
                                "`M40`1`L1" \
                                "`M40`2`L1" \
                                "`M40`3`L1" \
                                "`M40`4`L1"

#define BOARD_INFO_0_NAME_1     "proyecto ciaa"
#define BOARD_INFO_1_NAME_2     "computadora industrial abierta argentina"
#define BOARD_INFO_2_WEB        "http://www.proyecto-ciaa.com.ar"
#define BOARD_INFO_3_BOARD      "edu-ciaa-nxp board"
#define BOARD_INFO_4_SPECS      "dualcore arm cortex-m4/m0 - 204 mhz"

#define BOARD_PONCHO_FMT        "`M40`0`L1" \
                                "`M40`1`L1"

#define BOARD_PONCHO_0_NAME     "retro-ciaa poncho"
#define BOARD_PONCHO_1_SPECS    "software video adapter - 720p vga output"


#ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    #define BOARD_DESCRIPTION   "edu-ciaa w/retro poncho"
#else
    #define BOARD_DESCRIPTION   "edu-ciaa"
#endif


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
    struct IO_BOARD_EDU_CIAA                ioBoard;
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


#ifdef BSS_SECTION_BOARD
BSS_SECTION_BOARD
#endif
static struct BOARD_EDU_CIAA s_board_edu_ciaa;

#ifdef BSS_SECTION_IO_PROFILES
BSS_SECTION_IO_PROFILES
#endif
static struct BOARD_IO_PROFILES s_io_profiles;


static void * stageChange (struct BOARD *const B, const enum BOARD_Stage Stage);


static const struct BOARD_IFACE BOARD_EDU_CIAA_IFACE =
{
    .Description    = BOARD_DESCRIPTION,
    .StageChange    = stageChange,
    .Assert         = assertFunc
};


struct BOARD * BOARD__boot (const int Argc, const char **const Argv,
                            struct BOARD_RIG *const R)
{
    SystemCoreClockUpdate ();

    struct BOARD *const B = (struct BOARD *)&s_board_edu_ciaa;

    const char *const ErrorMsg = BOARD_Init (B, &BOARD_EDU_CIAA_IFACE,
                                             Argc, Argv, R);
    if (ErrorMsg)
    {
        Board_Panic (ErrorMsg);
    }

    return B;
}


static void greetings (struct STREAM *const S)
{
    STREAM_IN_FromParsedString (S, 0, 512, BOARD_LOGO_1);
    STREAM_IN_FromParsedString (S, 0, 512, BOARD_LOGO_2);
    STREAM_IN_FromParsedString (S, 0, 512, BOARD_LOGO_3);
    STREAM_IN_FromParsedString (S, 0, 512, BOARD_LOGO_4);
    STREAM_IN_FromParsedString (S, 0, 512, BOARD_LOGO_5);
    STREAM_IN_FromParsedString (S, 0, 512, BOARD_LOGO_6);
    STREAM_IN_FromParsedString (S, 0, 512, BOARD_LOGO_7);
    STREAM_IN_FromParsedString (S, 0, 512, BOARD_LOGO_8);
    STREAM_IN_FromParsedString (S, 0, 512, BOARD_INFO_FMT,
                                           BOARD_INFO_0_NAME_1,
                                           BOARD_INFO_1_NAME_2,
                                           BOARD_INFO_2_WEB,
                                           BOARD_INFO_3_BOARD,
                                           BOARD_INFO_4_SPECS);
    #ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    STREAM_IN_FromParsedString (S, 0, 512, BOARD_PONCHO_FMT,
                                           BOARD_PONCHO_0_NAME,
                                           BOARD_PONCHO_1_SPECS);
    #endif
}


static void * stageChange (struct BOARD *const B, const enum BOARD_Stage Stage)
{
    struct BOARD_EDU_CIAA *const E = (struct BOARD_EDU_CIAA *) B;

    switch (Stage)
    {
        case BOARD_Stage_InitPreTicksHardware:
        {
            Board_Init ();
            break;
        }

        case BOARD_Stage_InitPostTicksHardware:
        {
            break;
        }

        case BOARD_Stage_InitDebugStreamDriver:
        {
            BOARD_AssertState (BOARD_DEBUG_INTERFACE == LPC_USART2);

            STREAM_USART2_Init (&E->streamDebugUsart,
                                E->debugInBuffer, sizeof(E->debugInBuffer),
                                E->debugOutBuffer, sizeof(E->debugOutBuffer),
                                BOARD_DEBUG_BAUD_RATE, BOARD_DEBUG_CONFIG);
            return &E->streamDebugUsart;
        }

        case BOARD_Stage_Greetings:
        {
            greetings ((struct STREAM *)&E->streamDebugUsart);
            break;
        }

        case BOARD_Stage_InitIOProfiles:
        {
            INPUT_PROFILE_ATTACH    (MAIN, B, s_io_profiles.inMain);
            OUTPUT_PROFILE_ATTACH   (SIGN, B, s_io_profiles.outSign);
        #ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
            INPUT_PROFILE_ATTACH    (GP1, B, s_io_profiles.inGp1);
            INPUT_PROFILE_ATTACH    (GP2, B, s_io_profiles.inGp2);
            INPUT_PROFILE_ATTACH    (CONTROL, B, s_io_profiles.inControl);
            OUTPUT_PROFILE_ATTACH   (CONTROL, B, s_io_profiles.outControl);
        #endif
            break;
        }

        case BOARD_Stage_InitRandomDriver:
        {
            RANDOM_SFMT_Init (&E->randomSfmt, Board_GetSeed());
            return &E->randomSfmt;
        }

        case BOARD_Stage_InitIOLevel1Drivers:
        {
            IO_BOARD_EDU_CIAA_Init      (&E->ioBoard);
            IO_BOARD_EDU_CIAA_Attach    (&E->ioBoard);
        #ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
            IO_DUAL_NES_VIDEOEX_Init    (&E->ioDualNes);
            IO_DUAL_NES_VIDEOEX_Attach  (&E->ioDualNes);
        #endif
            break;
        }

        case BOARD_Stage_InitCommDrivers:
        {
            PACKET_SSP_Init (&E->packetSsp, BOARD_SPI_INTERFACE,
                    PACKET_SSP_Role_Controller,
                    PACKET_SSP_FrameFmt_Spi,
                    BOARD_SPI_SPEED, BOARD_SPI_BITS, 
                    PACKET_SSP_ClockFmt_Cpha0_Cpol0);

            COMM_SetPacket (COMM_Packet_HighSpeedDeviceExpansion,
                            (struct PACKET *)&E->packetSsp);

        #ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
            BOARD_AssertState (BOARD_EXT_USART_INTERFACE == LPC_USART3);

            STREAM_USART3_Init (&E->streamExtUsart,
                        E->tcpServerInBuffer, sizeof(E->tcpServerInBuffer),
                        E->tcpServerOutBuffer, sizeof(E->tcpServerOutBuffer),
                        BOARD_EXT_USART_BAUD_RATE, BOARD_EXT_USART_CONFIG);

            COMM_SetStream (COMM_Stream_IPNetworkSerialConfig,
                            (struct STREAM *)&E->streamExtUsart);

            PACKET_ESP32AT_TCP_SERVER_Init (&E->packetEsp32Tcp,
                                    (struct STREAM *)&E->streamExtUsart);

            COMM_SetPacket (COMM_Packet_IPNetwork,
                            (struct PACKET *)&E->packetEsp32Tcp);
        #endif
            break;
        }

        case BOARD_Stage_InitStorageDrivers:
        {
        #ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
            RAWSTOR_SD_1BIT_Init (&E->rawstorSd1Bit,
                                  COMM_Packet_HighSpeedDeviceExpansion);
            STORAGE_SetDevice ((struct RAWSTOR *)&E->rawstorSd1Bit);
        #endif
            break;
        }

        case BOARD_Stage_InitIOLevel2Drivers:
        {
            break;
        }

        case BOARD_Stage_InitScreenDrivers:
        {
        #ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
            VIDEO_DUALCORE_Init (&E->videoDualcore);
            SCREEN_RegisterDevice (SCREEN_Role_Primary,
                                    (struct VIDEO *)&E->videoDualcore);    
        #endif
            break;
        }

        case BOARD_Stage_InitSoundDriver:
        {
        #ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
            SOUND_PCM5100_Init (&E->soundPcm5100,
                                BOARD_I2S_INTERFACE,
                                BOARD_I2S_GPDMA_CONN_TX);
            return &E->soundPcm5100;
        #else
            break;
        #endif
        }

        case BOARD_Stage_InitIOLevel3Drivers:
        {
            break;
        }

        case BOARD_Stage_Ready:
        {
            break;
        }

        case BOARD_Stage_ShutdownHardware:
        {
            while (1)
            {
                __WFI ();
            }
        }
    }

    return NULL;
}
