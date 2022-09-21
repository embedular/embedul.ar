/*
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

/*
    EDU-CIAA-NXP & RETRO-CIAA board.
    Santiago Germino, 2018.
    <royconejo@gmail.com>
*/

#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/boards/edu_ciaa/board.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/shared/sysinit_util.h"


/* The System initialization code is called prior to the application and
   initializes the board for run-time operation. Board initialization
   includes clock setup and default pin muxing configuration. */


void Board_SetupMuxing (void)
{
    // Board LEDs: RED, GREEN, BLUE, 1, 2, 3.
    BOARD_GPIO_OUT (2,  0, BOARD_SCU_PUIBE_FUNC(4), LED_RGB_RED);
    BOARD_GPIO_OUT (2,  1, BOARD_SCU_PUIBE_FUNC(4), LED_RGB_GREEN);
    BOARD_GPIO_OUT (2,  2, BOARD_SCU_PUIBE_FUNC(4), LED_RGB_BLUE);
    BOARD_GPIO_OUT (2, 10, BOARD_SCU_PUIBE_FUNC(0), LED_1);
    BOARD_GPIO_OUT (2, 11, BOARD_SCU_PUIBE_FUNC(0), LED_2);
    BOARD_GPIO_OUT (2, 12, BOARD_SCU_PUIBE_FUNC(0), LED_3);

    // Board Buttons: TEC_1/2/3/4
    BOARD_GPIO_IN (1, 0, BOARD_SCU_INIBE_FUNC(0), SWITCH_TEC_1);
    BOARD_GPIO_IN (1, 1, BOARD_SCU_INIBE_FUNC(0), SWITCH_TEC_2);
    BOARD_GPIO_IN (1, 2, BOARD_SCU_INIBE_FUNC(0), SWITCH_TEC_3);
    BOARD_GPIO_IN (1, 6, BOARD_SCU_INIBE_FUNC(0), SWITCH_TEC_4);

    // LPC_SSP1: SPI port on P2 header. PF_4: SSP1_SCK, P1_3: SSP1_MISO,
    // P1_4: SSP1_MOSI.
    Chip_SCU_PinMuxSet (0xF, 4, BOARD_SCU_FASTIBE_FUNC(0));
    Chip_SCU_PinMuxSet (0x1, 3, BOARD_SCU_INIBEZD_FUNC(5));
    Chip_SCU_PinMuxSet (0x1, 4, BOARD_SCU_INIBEZD_FUNC(5));

    // UART0: RS-485 with on-board transceiver.
    Chip_SCU_PinMuxSet (9, 5, BOARD_SCU_INIBE_FUNC(2));
    Chip_SCU_PinMuxSet (9, 6, BOARD_SCU_INIBE_FUNC(7));
    Chip_SCU_PinMuxSet (6, 2, BOARD_SCU_INIBE_FUNC(7));

    // UART2 (DEBUG UART): Wired to FT2232 IC. P7_1: U2_TXD, P7_2: U2_RXD.
    Chip_SCU_PinMuxSet (7, 1, SCU_MODE_INACT | SCU_MODE_FUNC6);
    Chip_SCU_PinMuxSet (7, 2, BOARD_SCU_INIBE_FUNC(6));

    // UART3 on expansion port: 232_TX/RX on P1 header.
    Chip_SCU_PinMuxSet (2, 3, BOARD_SCU_INIBE_FUNC(2)); // UART3 TXD
    Chip_SCU_PinMuxSet (2, 4, BOARD_SCU_INIBE_FUNC(2)); // UART3 RXD

#ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    // PCM5100A Audio Codec Output (LPC_I2S1)
    Chip_SCU_PinMuxSet (0, 0,  BOARD_SCU_FASTIBE_FUNC(7)); // I2S1_TX_WS
    Chip_SCU_PinMuxSet (0, 1,  BOARD_SCU_FASTIBE_FUNC(7)); // I2S1_TX_SDA
    Chip_SCU_PinMuxSet (1, 19, BOARD_SCU_FASTIBE_FUNC(7)); // I2S1_TX_SCK

    // ~SYNC_JOY_OE/~RGB_OE: LCD_EN on P2 header, TEC_C0 on P1 header
    // (false = enabled).
    BOARD_GPIO_OUT (4, 9, BOARD_SCU_INZD_FUNC(4), SYNC_JOY_OE);
    BOARD_GPIO_OUT (1, 5, BOARD_SCU_INZD_FUNC(0), RGB_OE);

    // ESP WROOM32/SOLO Enable.
    // P7_4 (TEC_C1 on P1 header): WIFI_EN (false = disabled).
    BOARD_GPIO_OUT (7, 4, BOARD_SCU_PDIBE_FUNC(0), WIFI_EN);

    // EDDC Enable.
    // P7_5 (TEC_C2 on P1 header): EDDC_EN (false = disabled).
    BOARD_GPIO_OUT (7, 5, BOARD_SCU_INZD_FUNC(0), EDDC_EN);

    // LED_Z/Y: GPIO3, GPIO4 on P2 header (false = on).
    BOARD_GPIO_OUT (6, 7, BOARD_SCU_FASTIBE_FUNC(4), LED_Z);
    BOARD_GPIO_OUT (6, 8, BOARD_SCU_FASTIBE_FUNC(4), LED_Y);

    // JOY_LATCH/CLK: GPIO1, GPIO6 on P2 header.
    BOARD_GPIO_OUT (6,  4, BOARD_SCU_FASTIBE_FUNC(0), JOY_LATCH);
    BOARD_GPIO_OUT (6, 10, BOARD_SCU_FASTIBE_FUNC(0), JOY_CLOCK);

    // JOY1_DATA_IN/JOY2_DATA_IN: GPIO5, GPIO7 on P2 header.
    BOARD_GPIO_IN (6,  9, SCU_MODE_INBUFF_EN | SCU_MODE_FUNC4, JOY1_DATA);
    BOARD_GPIO_IN (6, 11, SCU_MODE_INBUFF_EN | SCU_MODE_FUNC4, JOY2_DATA);

    // PCM5100A Mute.
    // P7_7 (ENET_MDC on P2 header): ~SOUND_MUTE (true = unmuted).
    BOARD_GPIO_OUT (7, 7, BOARD_SCU_FASTIBE_FUNC(0), SOUND_MUTE);

    // SPI Chip Select.
    // P1_17 (ENET_MDIO on P2 header): ~SPI_CS1 (true = disabled).
    // P1_18 (ENET_TXD0 on P2 header): ~SPI_CS2 (true = disabled).
    // P1_20 (ENET_TXD1 on P2 header): ~SPI_CS3 (true = disabled).
    // For retrociaa-poncho v1.1.1 and v1.0.2 patched with sdcard reader pcb:
    // ~SPI_CS1 is permanently connected to sdcard select.
    // ~SPI_CS2 is permanently connected to the sdcard presence switch.
    BOARD_GPIO_OUT (1, 17, BOARD_SCU_FASTIBE_FUNC(0), SD_SELECT);
    BOARD_GPIO_IN  (1, 18, BOARD_SCU_PUIBE_FUNC(0), SWITCH_SD_DETECT);
    BOARD_GPIO_OUT (1, 20, BOARD_SCU_FASTIBE_FUNC(0), SPI_CS3);

    // VSYNC/HSYNC: LCD_4, LCD_RS.
    BOARD_GPIO_OUT (4, 10, BOARD_SCU_FASTIBE_FUNC(4), VSYNC);
    BOARD_GPIO_OUT (4,  8, BOARD_SCU_FASTIBE_FUNC(4), HSYNC);

    // Pixel output (R2 R1 R0 G2 G1 G0 B1 B0)
    //          R RRGGGBB
    // GPIO2    8x6543210
    BOARD_GPIO_OUT (6, 12, BOARD_SCU_FASTIBE_FUNC(0), PIXEL_R2); // GPIO8
    BOARD_GPIO_OUT (4,  6, BOARD_SCU_FASTIBE_FUNC(0), PIXEL_R1); // LCD3
    BOARD_GPIO_OUT (4,  5, BOARD_SCU_FASTIBE_FUNC(0), PIXEL_R0); // LCD2
    BOARD_GPIO_OUT (4,  4, BOARD_SCU_FASTIBE_FUNC(0), PIXEL_G2); // LCD1
    BOARD_GPIO_OUT (4,  3, BOARD_SCU_FASTIBE_FUNC(0), PIXEL_G1); // T_FIL3
    BOARD_GPIO_OUT (4,  2, BOARD_SCU_FASTIBE_FUNC(0), PIXEL_G0); // T_FIL2
    BOARD_GPIO_OUT (4,  1, BOARD_SCU_FASTIBE_FUNC(0), PIXEL_B1); // T_FIL1
    BOARD_GPIO_OUT (4,  0, BOARD_SCU_FASTIBE_FUNC(0), PIXEL_B0); // T_FIL0
#endif // #ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
}


struct CLK_BASE_STATES
{
   CHIP_CGU_BASE_CLK_T clk;     /* Base clock */
   CHIP_CGU_CLKIN_T clkin;      /* Base clock source, see UM for allowable souorces per base clock */
   bool autoblock_enab;         /* Set to true to enable autoblocking on frequency change */
   bool powerdn;                /* Set to true if the base clock is initially powered down */
};


/* Initial base clock states are mostly on */
#if 0
static const struct CLK_BASE_STATES InitClkStates[] =
{
   /* Ethernet Clock base */
//   {CLK_BASE_PHY_TX, CLKIN_ENET_TX, true, false},
//   {CLK_BASE_PHY_RX, CLKIN_ENET_TX, true, false},
   /* Clocks derived from dividers */
   //{CLK_BASE_USB0, CLKIN_IDIVD, true, true}
};
#endif

void Board_SetupClocking(void)
{
    Chip_CREG_SetFlashAcceleration(MAX_CLOCK_FREQ);
    Chip_SetupCoreClock(CLKIN_CRYSTAL, MAX_CLOCK_FREQ, true);
    
    /* Setup system base clocks and initial states. This won't enable and
       disable individual clocks, but sets up the base clock sources for
       each individual peripheral clock. */
    #if 0
    for (uint32_t i = 0; i < (sizeof(InitClkStates) / sizeof(InitClkStates[0])); ++i)
    {
        const struct CLK_BASE_STATES *c = &InitClkStates[i];
        Chip_Clock_SetBaseClock (c->clk, c->clkin, c->autoblock_enab,
                                c->powerdn);
    }
    #endif

    /* Reset and enable 32Khz oscillator */
    LPC_CREG->CREG0 &= ~((1u << 3) | (1u << 2));
    LPC_CREG->CREG0 |= (1u << 1) | (1u << 0);
}


/* Set up and initialize hardware prior to call to main */
void Board_SystemInit(void)
{
    /* Setup system clocking and memory. This is done early to allow the
       application and tools to clear memory and use scatter loading to
       external memory. */
    Board_SetupMuxing();
    Board_SetupClocking();
}
