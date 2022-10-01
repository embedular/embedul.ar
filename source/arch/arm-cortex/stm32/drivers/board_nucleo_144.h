#pragma once

#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/arch/arm-cortex/stm32/drivers/board_nucleo_144/io_board.h"
#include "embedul.ar/source/arch/arm-cortex/stm32/drivers/stream_usart.h"
#include "embedul.ar/source/arch/arm-cortex/stm32/drivers/random_rng.h"


struct BOARD_IO_PROFILES
{
    struct INPUT_PROFILE_MAIN   inMain;
    struct OUTPUT_PROFILE_SIGN  outSign;
};


struct BOARD_NUCLEO_144
{
    struct BOARD                device;
    struct IO_BOARD             ioBoard;
    struct STREAM_USART         streamDebugUsart;
    struct RANDOM_RNG           randomRng;
    uint8_t                     debugInBuffer[16];
    uint8_t                     debugOutBuffer[16];
    uint8_t                     tcpServerInBuffer[64];
    uint8_t                     tcpServerOutBuffer[1024];
};
