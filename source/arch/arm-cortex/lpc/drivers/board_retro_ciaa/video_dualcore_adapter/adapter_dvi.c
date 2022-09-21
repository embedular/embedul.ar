/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  Display-dualcore specialization for DVI transmitters.

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

#include "embedul.ar/source/arch/arm-cortex/lpc/video_dualcore_adapter/adapter.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/board_retro_ciaa/video_dualcore_adapter/pll0_config.h"

/*
    JLink: Flash to appropiate lpc43xx base address with JLinkExe:
    J-Link>loadbin {signal}.bin,0x1b000000
    with {signal} being, for example, "EDID_1280x720_60Hz".
*/

// I2C buffers for DVI initialization
static uint8_t  s_vTx[4];
static uint8_t  s_vRx[4];


static bool initPixelClock (void)
{
    // -------------------------------------------------------------------------
    // Reprogram the USB PLL to generate the required pixel clock frequency.
    // -------------------------------------------------------------------------
    // 1) Shut down PLL.
    //    BYPASS(1), DIRECTI(2), DIRECTO(3), CLKEN(4) = 0, FRM(6)
    //    PD(0) = 1 (PLL Power down)
    //    AUTOBLOCK(11) = 1 (Block clock automatically during frequency change)
    //    CLK_SEL(28:24) = Crystal oscillator(0x06)
    const uint32_t PLL_CTRL_INIT = (1u << 0) | (1u << 11) | (0x06 << 24);

    LPC_CGU->PLL[CGU_USB_PLL].PLL_CTRL = PLL_CTRL_INIT;

    // 2) Configure M, N y P
    LPC_CGU->PLL[CGU_USB_PLL].PLL_MDIV = PLL0_MDEC | (PLL0_SELP << 17)
                                    | (PLL0_SELI << 22) | (PLL0_SELR << 28);

    LPC_CGU->PLL[CGU_USB_PLL].PLL_NP_DIV = PLL0_PDEC | (PLL0_NDEC << 12);

    // 3) Turning on PLL, PD(0) = 1
    LPC_CGU->PLL[CGU_USB_PLL].PLL_CTRL &= ~(1u << 0);

    // 4) Wait for PLL lock monitoring LOCK BIT(0) in PLL0_STAT register.
    while (!(LPC_CGU->PLL[CGU_USB_PLL].PLL_STAT & 0x1))
    {
    }

    // 5) Turn on PLL clock output CLKEN(4) in PLL0_CTRL register.
    LPC_CGU->PLL[CGU_USB_PLL].PLL_CTRL |= (1u << 4);

    Chip_Clock_SetBaseClock (CLK_BASE_OUT, CLKIN_USBPLL, true, false);

    #ifdef DVI_ADAPTER_CLKOUT
    // Handy development code: output USB or crystal clock signal
    //Chip_SCU_ClockPinMuxSet (0, SCU_PINIO_FAST | SCU_MODE_FUNC1);

    // Chip_Clock_SetBaseClock (CLK_BASE_OUT, CLKIN_CRYSTAL, true, false);

    //            Chip_Clock_SetDivider (CLK_IDIV_E, CLKIN_MAINPLL, 3);
    //            Chip_Clock_SetBaseClock (CLK_BASE_OUT, CLKIN_IDIVE, true, false);
    #endif

    return true;
}


static bool initDVI (void)
{
    // -------------------------------------------------------------------------
    // DVI output initialization
    // -------------------------------------------------------------------------
    // s_vTx[0] == Register Sub-Address
    do
    {
        // ---------------------------------------------------------------------
        // NOTE: SIL164 supports up to 100 Khz I2C. Local I2C interface must
        //       have been configured to 100 Khz at this stage.
        // ---------------------------------------------------------------------
        // Read VEN_ID[7:0], [15:8]
        // SIL164, Silicon Image (Lattice): 0x0001
        // TFP410, Texas Instruments: 0x014C
        s_vTx[0] = 0x00;
        if ((g_videoExchange.errorCode = Board_I2CMTransfer(BOARD_I2C_INTERFACE,
                BOARD_I2C_SLAVE_DVI_TRANSMITTER, s_vTx, 1, s_vRx, 2))
            != I2CM_STATUS_OK)
        {
            break;
        }
        g_videoExchange.io.d0 = s_vRx[1];
        g_videoExchange.io.d1 = s_vRx[0];

        // Read DEV_ID[7:0], [15:8]
        s_vTx[0] = 0x02;
        if ((g_videoExchange.errorCode = Board_I2CMTransfer(BOARD_I2C_INTERFACE,
                BOARD_I2C_SLAVE_DVI_TRANSMITTER, s_vTx, 1, s_vRx, 2))
            != I2CM_STATUS_OK)
        {
            break;
        }
        g_videoExchange.io.d2 = s_vRx[1];
        g_videoExchange.io.d3 = s_vRx[0];

        // Write CTL_1_MODE (default after reset: 0xFE)
        s_vTx[0] = 0x08;
        // Bit 6 to 0: TMDS circuitry enable state determined by ~PD.
        // Bit 1 to 1: Power down (~PD) to normal operation.
        s_vTx[1] = 0xBF;
        if ((g_videoExchange.errorCode = Board_I2CMTransfer(BOARD_I2C_INTERFACE,
                BOARD_I2C_SLAVE_DVI_TRANSMITTER, s_vTx, 2, NULL, 0))
            != I2CM_STATUS_OK)
        {
            break;
        }

        // TFP410 ONLY

        // Write DE_TOP (default after reset: 0x00)
        s_vTx[0] = 0x34;
        // Bit [7:0]: DE_TOP[7:0]
        s_vTx[1] = (MODELINE_VTOTAL - MODELINE_VSYNCSTART) & 0xFF;
        if ((g_videoExchange.errorCode = Board_I2CMTransfer(BOARD_I2C_INTERFACE,
                BOARD_I2C_SLAVE_DVI_TRANSMITTER, s_vTx, 2, NULL, 0))
            != I2CM_STATUS_OK)
        {
            break;
        }

        // Write DE_CNT (default after reset: 0x0000)
        s_vTx[0] = 0x36;
        // Bit [7:0]: DE_CNT[7:0]
        s_vTx[1] = MODELINE_HDISP & 0xFF;
        // Bit [2:0]: DE_CNT[10:8]
        s_vTx[2] = MODELINE_HDISP >> 8;
        if ((g_videoExchange.errorCode = Board_I2CMTransfer(BOARD_I2C_INTERFACE,
                BOARD_I2C_SLAVE_DVI_TRANSMITTER, s_vTx, 3, NULL, 0))
            != I2CM_STATUS_OK)
        {
            break;
        }

        // Write DE_LIN (default after reset: 0x0000)
        s_vTx[0] = 0x38;
        // Bit [7:0]: DE_CNT[7:0]
        s_vTx[1] = MODELINE_VDISP & 0xFF;
        // Bit [2:0]: DE_CNT[10:8]
        s_vTx[2] = MODELINE_VDISP >> 8;
        if ((g_videoExchange.errorCode = Board_I2CMTransfer(BOARD_I2C_INTERFACE,
                BOARD_I2C_SLAVE_DVI_TRANSMITTER, s_vTx, 3, NULL, 0))
            != I2CM_STATUS_OK)
        {
            break;
        }

        // Write DE_DLY (default after reset: 0x00)
        s_vTx[0] = 0x32;
        // Bit [7:0]: DE_DLY[7:0]
        s_vTx[1] = (MODELINE_HTOTAL - MODELINE_HSYNCSTART) & 0xFF;
        if ((g_videoExchange.errorCode = Board_I2CMTransfer(BOARD_I2C_INTERFACE,
                BOARD_I2C_SLAVE_DVI_TRANSMITTER, s_vTx, 2, NULL, 0))
            != I2CM_STATUS_OK)
        {
            break;
        }

        // Write DE_CTL (default after reset: 0x00)
        s_vTx[0] = 0x33;
        // Bit 6 to 1: Enable DE generator.
        // Bit 5: VSYNC polarity, 0 active low, 1 active high.
        // Bit 4: HSYNC polarity, 0 active low, 1 active high.
        // Bit 0: DE_DLY[8]
        s_vTx[1] = 0x40 | (MODELINE_VSYNC_ON << 5) | (MODELINE_HSYNC_ON << 4) |
                   ((MODELINE_HTOTAL - MODELINE_HSYNCSTART) >> 8);
        if ((g_videoExchange.errorCode = Board_I2CMTransfer(BOARD_I2C_INTERFACE,
                BOARD_I2C_SLAVE_DVI_TRANSMITTER, s_vTx, 2, NULL, 0))
            != I2CM_STATUS_OK)
        {
            break;
        }
    }
    while (0);

    if (g_videoExchange.errorCode != I2CM_STATUS_OK)
    {
        g_videoExchange.io.d7 = s_vTx[0];
        return false;
    }

    // Enable video interface link
    BOARD_GPIO_SET_STATE (OUT_DVI_EN, ENABLED);

    return true;
}


bool VIDEO_ADAPTER_Init (void)
{
    if (!initPixelClock ())
    {
        return false;
    }

    if (!initDVI ())
    {
        return false;
    }

    return true;
}


void VIDEO_ADAPTER_Vbi (void)
{

}
