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
    RETRO-CIAA standalone board.
    Santiago Germino, 2020.
    <royconejo@gmail.com>
*/

#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/boards/retro_ciaa/board.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/shared/sysinit_util.h"


/* The System initialization code is called prior to the application and
   initializes the board for run-time operation. Board initialization
   includes clock setup and default pin muxing configuration. */


void Board_SetupMuxing (void)
{
    // By default, the digital function 0 with pull-up enabled is selected for
    // all pins. For pins that support a digital and analog function, the ADC
    // function select registers in the SCU enable the analog function.

    // User controlled board LED (orange, close to MCU, labeled as WARNING on
    // schematic)
    BOARD_GPIO_OUT (7, 3, BOARD_SCU_INIBE_FUNC(0), LED_WARN);

    // User controlled board Buttons: ~BOOT_ISP
    BOARD_GPIO_IN (2, 7, BOARD_SCU_INIBE_FUNC(0), SWITCH_ISP);

    // USART0: ISP/DEBUG through TTL-to-USB.
    // P2_0: U0_TXD, P2_1: U0_RXD.
    Chip_SCU_PinMuxSet (2, 0, SCU_MODE_INACT | SCU_MODE_FUNC1);
    Chip_SCU_PinMuxSet (2, 1, BOARD_SCU_INIBEZD_FUNC(1));

    // USART1: ESP32 (WiFi/BLE) module.
    // P5_2: U1_RTS, P5_4: U1_CTS, P5_6: U1_TXD, P5_7: U1_RXD.
    #if 1
    Chip_SCU_PinMuxSet (5, 2, SCU_MODE_INACT | SCU_MODE_FUNC4);
    Chip_SCU_PinMuxSet (5, 4, SCU_MODE_INACT | SCU_MODE_FUNC4);
    #else 
    // MCU: P5_2, U1_RTS, GPIO2[11] - WIFI: IO15(MTDI), CTS
    // WIFI, Low: silences boot messages printed by the ROM bootloader
    // (weak pull-up)
    BOARD_GPIO_OUT (5, 2, BOARD_SCU_FASTIBE_FUNC(0), WIFI_IO15);
    // MCU: P5_4, U1_CTS, GPIO2[13] - WIFI: IO14(MTDI), RTS
    BOARD_GPIO_OUT (5, 4, BOARD_SCU_FASTIBE_FUNC(0), WIFI_IO14);
    #endif
    Chip_SCU_PinMuxSet (5, 6, SCU_MODE_INACT | SCU_MODE_FUNC4);
    Chip_SCU_PinMuxSet (5, 7, BOARD_SCU_INIBEZD_FUNC(4));

    // I2C connected to local peripherals.
    // P2_3: I2C1_SDA, P2_4: I2C1_SCL
    Chip_SCU_PinMuxSet (2, 3, BOARD_SCU_FASTIBE_FUNC(1));
    Chip_SCU_PinMuxSet (2, 4, BOARD_SCU_FASTIBE_FUNC(1));

    // LPC_I2S0: PCM5100A Audio Codec Output.
    // P3_0: I2S0_TX_SCK, P3_1: I2S0_TX_WS, P7_2: I2S0_TX_SDA.
    Chip_SCU_PinMuxSet (3, 0, BOARD_SCU_FASTIBE_FUNC(2));
    Chip_SCU_PinMuxSet (3, 1, BOARD_SCU_FASTIBE_FUNC(0));
    Chip_SCU_PinMuxSet (7, 2, BOARD_SCU_FASTIBE_FUNC(2));

    // PCM5100A Mute.
    // P7_0: ~SOUND_MUTE.
    BOARD_GPIO_OUT (7, 0, BOARD_SCU_FASTIBE_FUNC(0), SOUND_MUTE);

    // ESP WROOM32/SOLO Enable.
    // P9_5: WIFI_EN.
    BOARD_GPIO_OUT (9, 5, BOARD_SCU_PDIBE_FUNC(4), WIFI_EN);

    // DVI Transmitter Enable.
    // P9_6: DVI_EN.
    BOARD_GPIO_OUT (9, 6, BOARD_SCU_FASTIBE_FUNC(0), DVI_EN);

    // HDMI CEC.
    // P2_5: CEC.
    Chip_SCU_PinMuxSet (2, 5, BOARD_SCU_INIBE_FUNC(4));

    // HSYNC, VSYNC, RGB_EN.
    // P2_2: HSYNC, P6_6: VSYNC, P6_7: RGB_EN.
    BOARD_GPIO_OUT (2, 2, BOARD_SCU_FASTIBE_FUNC(4), HSYNC);
    BOARD_GPIO_OUT (6, 6, BOARD_SCU_FASTIBE_FUNC(0), VSYNC);
    BOARD_GPIO_OUT (6, 7, BOARD_SCU_FASTIBE_FUNC(4), RGB_EN);

    // PCLK (CLKOUT source initialized by dvi video driver).
    // CLK2: CLKOUT.
    Chip_SCU_ClockPinMuxSet (2, BOARD_SCU_FASTIBE_FUNC(1));

    // Pixel Output (RRRGGGBB).
    // P6_11: R2, P6_10: R1, P6_9: R0, P6_5: G2, P6_4: G1, P6_3: G0.
    // P6_2: B1, P6_1: B0.
    BOARD_GPIO_OUT (6, 11, BOARD_SCU_FASTIBE_FUNC(0), PIXEL_R2);
    BOARD_GPIO_OUT (6, 10, BOARD_SCU_FASTIBE_FUNC(0), PIXEL_R1);
    BOARD_GPIO_OUT (6,  9, BOARD_SCU_FASTIBE_FUNC(0), PIXEL_R0);
    BOARD_GPIO_OUT (6,  5, BOARD_SCU_FASTIBE_FUNC(0), PIXEL_G2);
    BOARD_GPIO_OUT (6,  4, BOARD_SCU_FASTIBE_FUNC(0), PIXEL_G1);
    BOARD_GPIO_OUT (6,  3, BOARD_SCU_FASTIBE_FUNC(0), PIXEL_G0);
    BOARD_GPIO_OUT (6,  2, BOARD_SCU_FASTIBE_FUNC(0), PIXEL_B1);
    BOARD_GPIO_OUT (6,  1, BOARD_SCU_FASTIBE_FUNC(0), PIXEL_B0);

    // SD/MMC.
    // P1_6: SD_CMD. P1_9: SD_DAT0, P1_10: SD_DAT1, P1_11: SD_DAT2,
    // P1_12: SD_DAT3. P1_13: ~SD_CD, CLK0: SD_CLK.
	Chip_SCU_PinMuxSet (1, 6, BOARD_SCU_FASTIBE_FUNC(7));
	Chip_SCU_PinMuxSet (1, 9, BOARD_SCU_FASTIBE_FUNC(7));
	Chip_SCU_PinMuxSet (1, 10, BOARD_SCU_FASTIBE_FUNC(7));
	Chip_SCU_PinMuxSet (1, 11, BOARD_SCU_FASTIBE_FUNC(7));
    Chip_SCU_PinMuxSet (1, 12, BOARD_SCU_FASTIBE_FUNC(7));
	Chip_SCU_PinMuxSet (1, 13, SCU_MODE_INBUFF_EN | SCU_MODE_FUNC7);
    Chip_SCU_ClockPinMuxSet (0, BOARD_SCU_INIBE_FUNC(4));

    // P1_5: ~SD_POW.
    // Note that ~SD_POW is inverted on schematic: 0 = power on, 1 = power off.
    // See UM10503, Section 22.6.2, Table 358. So LPC_SDMMC->PWREN = 1 equals
    // OFF. To avoid confusion, this pin will be handled not as SD_POW
    // (SCU_MODE_FUNC7), but rather as a GPIO.
    BOARD_GPIO_OUT (1, 5, BOARD_SCU_INIBE_FUNC(0), SD_POW);

    // -------------------------------------------------------------------------
    // Stack Port
    // -------------------------------------------------------------------------
    // SPI/SSP0 Configured as SSP0.
    // P3_3: SSP0_SCK, P3_6: SSP0_MISO, P3_7: SSP0_MOSI, P3_8: SSP0_SSEL.
    Chip_SCU_PinMuxSet (3, 3, BOARD_SCU_FASTIBE_FUNC(2));
    Chip_SCU_PinMuxSet (3, 6, BOARD_SCU_INIBEZD_FUNC(5));
    Chip_SCU_PinMuxSet (3, 7, BOARD_SCU_INIBEZD_FUNC(5));
    Chip_SCU_PinMuxSet (3, 8, BOARD_SCU_FASTIBE_FUNC(5));

    // USART2: TTL.
    // P2_10: U2_TXD, P2_11: U2_RXD.
    Chip_SCU_PinMuxSet (2, 10, SCU_MODE_INACT | SCU_MODE_FUNC2);
    Chip_SCU_PinMuxSet (2, 11, BOARD_SCU_INIBEZD_FUNC(2));

    // USART3: EIA-485 (transceiver not included on base board).
    // P4_1: U3_TXD, P4_2: U3_RXD, P4_4: U3_DIR.
    Chip_SCU_PinMuxSet (4, 1, SCU_MODE_INACT | SCU_MODE_FUNC6);
    Chip_SCU_PinMuxSet (4, 2, BOARD_SCU_INIBEZD_FUNC(6));
    Chip_SCU_PinMuxSet (4, 4, SCU_MODE_INACT | SCU_MODE_FUNC6);

    // GPIO Ports (Input/Output selectable by application, output by default).
    // IO1: P4_8, IO2: P4_9, IO3: P4_10, IO4: P4_0, IO5: P4_3, IO6: P4_5,
    // IO7: P4_6.
    BOARD_GPIO_IO (4,  8, BOARD_SCU_INIBE_FUNC(4), STACKPORT_IO1);
    BOARD_GPIO_IO (4,  9, BOARD_SCU_INIBE_FUNC(4), STACKPORT_IO2);
    BOARD_GPIO_IO (4, 10, BOARD_SCU_INIBE_FUNC(4), STACKPORT_IO3);
    BOARD_GPIO_IO (4,  0, BOARD_SCU_INIBE_FUNC(0), STACKPORT_IO4);
    BOARD_GPIO_IO (4,  3, BOARD_SCU_INIBE_FUNC(0), STACKPORT_IO5);
    BOARD_GPIO_IO (4,  5, BOARD_SCU_INIBE_FUNC(0), STACKPORT_IO6);
    BOARD_GPIO_IO (4,  6, BOARD_SCU_INIBE_FUNC(0), STACKPORT_IO7);

    // ENET (RMII)
    // P1_15: RXD0, P0_0: RXD1, P1_18: TXD0, P1_20: TXD1, P7_7: MDC,
    // P0_1: TX_EN, P1_16: RX_DV, P1_17: MDIO, P1_19: TX_CLK.
    Chip_SCU_PinMuxSet (1, 15, BOARD_SCU_FASTIBE_FUNC(3));
	Chip_SCU_PinMuxSet (0,  0, BOARD_SCU_FASTIBE_FUNC(2));
	Chip_SCU_PinMuxSet (1, 18, BOARD_SCU_FAST_FUNC(3));
	Chip_SCU_PinMuxSet (1, 20, BOARD_SCU_FAST_FUNC(3));
    Chip_SCU_PinMuxSet (7,  7, BOARD_SCU_FAST_FUNC(7));
	Chip_SCU_PinMuxSet (0,  1, BOARD_SCU_FAST_FUNC(6));
	Chip_SCU_PinMuxSet (1, 16, BOARD_SCU_FASTIBE_FUNC(7));
    Chip_SCU_PinMuxSet (1, 17, BOARD_SCU_FASTIBE_FUNC(3));
	Chip_SCU_PinMuxSet (1, 19, BOARD_SCU_FASTIBE_FUNC(0));
}


void Board_SetupClocking(void)
{
    Chip_CREG_SetFlashAcceleration(MAX_CLOCK_FREQ);
    Chip_SetupCoreClock(CLKIN_CRYSTAL, MAX_CLOCK_FREQ, true);
    
    /* Setup system base clocks and initial states. This won't enable and
       disable individual clocks, but sets up the base clock sources for
       each individual peripheral clock. */
    #if 0
    Chip_Clock_SetBaseClock (CLK_BASE_PHY_TX, CLKIN_ENET_TX, true, false);
    Chip_Clock_SetBaseClock (CLK_BASE_PHY_RX, CLKIN_ENET_TX, true, false);
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
