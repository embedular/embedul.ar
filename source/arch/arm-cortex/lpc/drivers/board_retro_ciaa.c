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

#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/board_retro_ciaa.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/board_shared/iface_methods.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/boards/retro_ciaa/board.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/shared/seed.h"


#define BOARD_LOGO_1                "   `S1█`P6  `P7 `P8 `P6   `P6" \
                                    "         `P6 `P2  `P5   `P5`L"
#define BOARD_LOGO_2                "   ██   ██ ██`P9██    ██   ██ ██" \
                                    "    ██`P7██`P6██ ██   ██ ██   ██`L"
#define BOARD_LOGO_3                "   `S1█`P6  `P5      `P2    `P6  `P2" \
                                    "    `P2 `P5 `P2      `P2 `P7 `P7`L"
#define BOARD_LOGO_4                BOARD_LOGO_2
#define BOARD_LOGO_5                "   `S1█`P2   `P2 `P7    `P2    `P2" \
                                    "   `P2  `P6         `P6 `P2 `P2   `P2" \
                                    " `P2   `P2`L2"

#define BOARD_INFO_0_NAME           "retro-ciaa board"
#define BOARD_INFO_1_WEB            "http://www.retro-ciaa.com"
#define BOARD_INFO_2_SPECS          "nxp lpc4337fet - dualcore arm " \
                                    "cortex-m4/m0 - 204 mhz"
#define BOARD_INFO_3_SPECS          "software video adapter - 720p hdmi output"
#define BOARD_INFO_4_SPECS          "stack port expansion"

#define BOARD_INFO_FMT              "`M40`0`L1" \
                                    "`M40`1`L1" \
                                    "`M40`2`L1" \
                                    "`M40`3`L1" \
                                    "`M40`4`L1"


#ifdef BSS_SECTION_BOARD
BSS_SECTION_BOARD
#endif
static struct BOARD_RETRO_CIAA s_board_retro_ciaa;

#ifdef BSS_SECTION_IO_PROFILES
BSS_SECTION_IO_PROFILES
#endif
static struct BOARD_IO_PROFILES s_io_profiles;


static const struct HUEWHEEL_PolarPlacement s_HuewheelPolarPlacement[12] = 
{
    [0]  = {  0,  1,  2, 103 },
    [1]  = {  3,  4,  5, 114 },
    [2]  = {  6,  7,  8, 128 },
    [3]  = {  9, 10, 11, 142 },
    [4]  = { 12, 13, 14, 154 },
    [5]  = { 15, 16, 17, 170 },
    [6]  = { 18, 19, 20, 184 },
    [7]  = { 21, 22, 23, 201 },
    [8]  = { 24, 25, 26, 215 },
    [9]  = { 27, 28, 29,  17 },
    [10] = { 30, 31, 32,  26 },
    [11] = { 33, 34, 35,  78 }
};


static void * stageChange (struct BOARD *const B, const enum BOARD_Stage Stage);


static const struct BOARD_IFACE BOARD_RETROCIAA_IFACE =
{
    .Description    = "retro-ciaa",
    .StageChange    = stageChange,
    .Assert         = assertFunc,
    .SetTickFreq    = setTickFreq,
    .SetTickHook    = setTickHook,
    .TicksNow       = ticksNow,
    .Rtc            = UNSUPPORTED,
    .Delay          = delay,
    .Update         = UNSUPPORTED
};


void BOARD_Boot (struct BOARD_RIG *const R)
{
    SystemCoreClockUpdate ();

    const char *const ErrorMsg = BOARD_Init (
                                        (struct BOARD *)&s_board_retro_ciaa,
                                        &BOARD_RETROCIAA_IFACE, R);
    if (ErrorMsg)
    {
        Board_Panic (ErrorMsg);
    }
}


static void greetings (struct STREAM *const B)
{
    STREAM_IN_FromParsedString (B, 0, 512, BOARD_LOGO_1);
    STREAM_IN_FromParsedString (B, 0, 512, BOARD_LOGO_2);
    STREAM_IN_FromParsedString (B, 0, 512, BOARD_LOGO_3);
    STREAM_IN_FromParsedString (B, 0, 512, BOARD_LOGO_4);
    STREAM_IN_FromParsedString (B, 0, 512, BOARD_LOGO_5);
    STREAM_IN_FromParsedString (B, 0, 512, BOARD_INFO_FMT,
                                           BOARD_INFO_0_NAME,
                                           BOARD_INFO_1_WEB,
                                           BOARD_INFO_2_SPECS,
                                           BOARD_INFO_3_SPECS,
                                           BOARD_INFO_4_SPECS);
}


static void * stageChange (struct BOARD *const B, const enum BOARD_Stage Stage)
{
    struct BOARD_RETRO_CIAA *const R = (struct BOARD_RETRO_CIAA *) B;

    switch (Stage)
    {
        case BOARD_Stage_InitHardware:
        {
            Board_Init ();
            BOARD_SetTickFreq (1000);
            break;
        }

        case BOARD_Stage_InitDebugStreamDriver:
        {
            BOARD_AssertState (BOARD_DEBUG_INTERFACE == LPC_USART0);

            STREAM_USART0_Init (&R->streamDebugUsart,
                                R->debugInBuffer, sizeof(R->debugInBuffer),
                                R->debugOutBuffer, sizeof(R->debugOutBuffer),
                                BOARD_DEBUG_BAUD_RATE, BOARD_DEBUG_CONFIG);
            return &R->streamDebugUsart;
        }

        case BOARD_Stage_Greetings:
        {
            greetings ((struct STREAM *)&R->streamDebugUsart);
            break;
        }

        case BOARD_Stage_InitIOProfiles:
        {
            INPUT_PROFILE_ATTACH    (CONTROL, B, s_io_profiles.inControl);
            INPUT_PROFILE_ATTACH    (GP1, B, s_io_profiles.inGp1);
            INPUT_PROFILE_ATTACH    (GP2, B, s_io_profiles.inGp2);
            INPUT_PROFILE_ATTACH    (LIGHTDEV, B, s_io_profiles.inLightdev);
            INPUT_PROFILE_ATTACH    (MAIN, B, s_io_profiles.inMain);
            OUTPUT_PROFILE_ATTACH   (CONTROL, B, s_io_profiles.outControl);
            OUTPUT_PROFILE_ATTACH   (LIGHTDEV, B, s_io_profiles.outLightdev);
            OUTPUT_PROFILE_ATTACH   (MARQUEE, B, s_io_profiles.outMarquee);
            OUTPUT_PROFILE_ATTACH   (SIGN, B, s_io_profiles.outSign);
            break;
        }

        case BOARD_Stage_InitRandomDriver:
        {
            RANDOM_SFMT_Init (&R->randomSfmt, Board_GetSeed());
            return &R->randomSfmt;
        }

        case BOARD_Stage_InitIOLevel1Drivers:
        {
            IO_BOARD_Init   (&R->ioBoard);
            IO_BOARD_Attach (&R->ioBoard);
            break;
        }

        case BOARD_Stage_InitCommDrivers:
        {
            BOARD_AssertState (BOARD_ESP_INTERFACE == LPC_UART1);

            // Stack Port I2C interface
            PACKET_I2C_CONTROLLER_Init (&R->packetI2cExpController,
                                        BOARD_STACKPORT_I2C_INTERFACE,
                                        BOARD_STACKPORT_I2C_SPEED);

            COMM_SetPacket (COMM_Packet_LowSpeedExpansionBus,
                            (struct PACKET *)&R->packetI2cExpController);

        #ifndef BOARD_RETRO_CIAA_DISABLE_TCP_SERVER
            STREAM_UART1_Init (&R->streamEsp32atUsart,
                        R->tcpServerInBuffer, sizeof(R->tcpServerInBuffer),
                        R->tcpServerOutBuffer, sizeof(R->tcpServerOutBuffer),
                        BOARD_ESP_BAUD_RATE, BOARD_ESP_CONFIG);

            COMM_SetStream (COMM_Stream_IPNetworkSerialConfig,
                            (struct STREAM *)&R->streamEsp32atUsart);

            PACKET_ESP32AT_TCP_SERVER_Init (&R->packetEsp32Tcp,
                                    (struct STREAM *)&R->streamEsp32atUsart);

            COMM_SetPacket (COMM_Packet_IPNetwork,
                            (struct PACKET *)&R->packetEsp32Tcp);
        #endif
            break;
        }

        case BOARD_Stage_InitStorageDrivers:
        {
            RAWSTOR_SD_SDMMC_Init (&R->rawstorSdmmc);
            STORAGE_SetDevice ((struct RAWSTOR *)&R->rawstorSdmmc);
            break;
        }

        case BOARD_Stage_InitIOLevel2Drivers:
        {
            {
                LOG_AutoContext (B, "available expansion modules");

                LOG_PendingBegin (B, "genesis module");
                if (Board_DetectedStackPortModules (BOARD_SP_GENESIS))
                {
                    LOG_PendingEndOk ();

                    // RGB led driver for marquee and marquee driver.
                    // Set RGB pixels at half intensity to avoid consuming over 
                    // 500 mA.
                    IO_LP5036_Init (&R->ioLp5036,
                                    COMM_Packet_LowSpeedExpansionBus,
                                    BOARD_SP_GENESIS_I2C_ADDR_LP5036,
                                    0x80);

                    IO_HUEWHEEL_Init    (&R->ioHuewheel,
                                         (struct IO_Gateway) {
                                            .driver = (struct IO *)&R->ioLp5036,
                                            .driverPort = 0
                                         }, 
                                         s_HuewheelPolarPlacement, 12);
                    IO_HUEWHEEL_Attach  (&R->ioHuewheel);

                    OUTPUT_SET_RANGE_DEFER (MARQUEE, Step, 520);

                    IO_DUAL_GENESIS_PCA9673_Init    (
                                            &R->ioDualGenesis, 
                                            COMM_Packet_LowSpeedExpansionBus,
                                            BOARD_SP_GENESIS_I2C_ADDR_PCA9673);
                    IO_DUAL_GENESIS_PCA9673_Attach  (&R->ioDualGenesis);
                }
                else
                {
                    LOG_PendingEndFail ();
                }

                LOG_PendingBegin (B, "ssl module");
                if (Board_DetectedStackPortModules (BOARD_SP_SSL))
                {
                    LOG_PendingEndOk ();

                    IO_PCA9956B_Init    (&R->ioPca9956BDeviceA,
                                         COMM_Packet_LowSpeedExpansionBus,
                                         BOARD_SP_SSL_I2C_ADDR_PCA9956B_DA);
                    IO_PCA9956B_Attach  (&R->ioPca9956BDeviceA, 0, 0);
                    IO_PCA9956B_Init    (&R->ioPca9956BDeviceB,
                                         COMM_Packet_LowSpeedExpansionBus,
                                         BOARD_SP_SSL_I2C_ADDR_PCA9956B_DB);
                    IO_PCA9956B_Attach  (&R->ioPca9956BDeviceB, 24, 1);
                }
                else
                {
                    LOG_PendingEndFail ();
                }
            }
            break;
        }

        case BOARD_Stage_InitScreenDrivers:
        {
            VIDEO_DUALCORE_Init (&R->videoDualcore);
            SCREEN_RegisterDevice (SCREEN_Role_Primary,
                                    (struct VIDEO *)&R->videoDualcore);
            break;
        }

        case BOARD_Stage_InitSoundDriver:
        {
            SOUND_PCM5100_Init (&R->soundPcm5100,
                                BOARD_I2S_INTERFACE,
                                BOARD_I2S_GPDMA_CONN_TX);
            return &R->soundPcm5100;
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
            }
        }
    }

    return NULL;
}
