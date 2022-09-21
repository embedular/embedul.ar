/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  lpc43xx generic software video adapter.

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
#include <stdnoreturn.h>


// Initial status
static uint32_t s_vFramebuffer  = (uint32_t) g_framebufferA;
static uint32_t s_vFrameCount   = 0;
static uint32_t s_vShownLines   = VIDEO_ADAPTER_FB_LINE_DUP;
static uint32_t s_vFBOffset     = (uint32_t) g_framebufferA;
static uint32_t s_vLineNum      = 1;
static uint32_t s_vLineDup      = 0;
static uint32_t s_vShowAnd      = 0xFF;
static uint32_t s_vShowOr       = 0x00;
static uint32_t s_vScanAnd      = 0x24;
static uint32_t s_vScanOr       = 0x00;


void HardFault_Handler (void)
{
    while (1)
    {
        BOARD_LED_TOGGLE (M0_HARDFAULT);
        for (uint32_t i = 0; i < 9000000; ++i);
    }
}


static void initHsyncTimer (void)
{
    // -------------------------------------------------------------------------
    // Horizontal sync interrupt
    // -------------------------------------------------------------------------
    // Activate Repetitive Interrupt Timer (RIT) for periodic IRQs
    Chip_RIT_Init       (LPC_RITIMER);

    // CLK_MX_RITIMER Clock Rate = 204000000
    const uint32_t Cmp_value = (uint32_t)
            ((int32_t)(Chip_Clock_GetRate(CLK_MX_RITIMER) / MODELINE_HSYNC_HZ)
                                            + VIDEO_ADAPTER_HSYNC_RIT_TUNE);

    Chip_RIT_SetCOMPVAL (LPC_RITIMER, Cmp_value);
    Chip_RIT_EnableCTRL (LPC_RITIMER, RIT_CTRL_ENCLR);
    Chip_RIT_Enable     (LPC_RITIMER);

    // Enable IRQ for RIT
    NVIC_SetPriority    (RITIMER_IRQn, 0);
    NVIC_EnableIRQ      (RITIMER_IRQn);
}


static bool init (void)
{
    VIDEO_ADAPTER_SET_SYNC_SIGNAL (HSYNC, OFF);
    VIDEO_ADAPTER_SET_SYNC_SIGNAL (VSYNC, OFF);

    // -------------------------------------------------------------------------
    // Memory exchange area initialization
    // -------------------------------------------------------------------------
    // The M4 core will not update vdrvExchange before initialization procedures
    // and M0 bootup, so it is safe to set default driver values.
    g_videoExchange.framebuffer = s_vFramebuffer;
    g_videoExchange.frameNumber = s_vFrameCount;
    g_videoExchange.showAnd     = (uint8_t) s_vShowAnd;
    g_videoExchange.showOr      = (uint8_t) s_vShowOr;
    g_videoExchange.scanAnd     = (uint8_t) s_vScanAnd;
    g_videoExchange.scanOr      = (uint8_t) s_vScanOr;
    // Conversion from s_vShownLines "visible line count" to g_videoExchange
    // "scanline count".
    g_videoExchange.scanlines   = (uint8_t) (VIDEO_ADAPTER_FB_LINE_DUP
                                                            - s_vShownLines);

    // Error reported on adapter initialization
    g_videoExchange.errorCode   = 0;

    // Driver information
    g_videoExchange.description = VIDEO_ADAPTER_LT_STR,
    g_videoExchange.signal      = MODELINE_SIGNAL_STR,
    g_videoExchange.modeline    = LANG_MODELINE,
    g_videoExchange.build       = CC_BuildInfoStr;

    // Generic adapter Input/Output
    g_videoExchange.d           = 0;

    if (!VIDEO_ADAPTER_Init ())
    {
        return false;
    }

    // Using a mask on pixel output GPIO to only update pixel pins on a single
    // write to VIDEO_GPIO_MPIN_ADDR.
    *((uint32_t *)VIDEO_ADAPTER_GPIO_MASK_ADDR) = VIDEO_ADAPTER_GPIO_MASK;

    initHsyncTimer ();

    return true;
}


void RIT_IRQHandler (void)
{
//    __DSB ();
//    __ISB ();

    // if (Chip_RIT_GetIntStatus(LPC_RITIMER) == SET)
//    if (!(LPC_RITIMER->CTRL & RIT_CTRL_INT))
//    {
 //       return;
 //   }

    switch (s_vLineNum)
    {
        case (MODELINE_VSYNCSTART + 1):
            VIDEO_ADAPTER_SET_SYNC_SIGNAL (VSYNC, ON);
            break;

        case (MODELINE_VSYNCEND + 1):
            VIDEO_ADAPTER_SET_SYNC_SIGNAL (VSYNC, OFF);
            break;
    }

/*
    const uint32_t R1 = s_vLineNum > MODELINE_VSYNCSTART;
    const uint32_t R2 = s_vLineNum < MODELINE_VSYNCEND + 1;
    const uint32_t R3 = R1 && R2;

    if (R3)
    {
        VIDEO_SET_SYNC_SIGNAL (VSYNC, ON);
    }
    else
    {
        VIDEO_SET_SYNC_SIGNAL (VSYNC, OFF);
    }
*/
    VIDEO_ADAPTER_SET_SYNC_SIGNAL (HSYNC, ON);
    M0_InstDelay (VIDEO_ADAPTER_HSYNC_IC);
    VIDEO_ADAPTER_SET_SYNC_SIGNAL (HSYNC, OFF);

    Chip_RIT_ClearInt       (LPC_RITIMER);
    NVIC_ClearPendingIRQ    (RITIMER_IRQn);
}


noreturn
static void signalLoop ()
{
    while (1)
	{
        // Visible Pixels
        if (s_vLineNum < MODELINE_VDISP + 1)
        {
            // __DSB ();
            // __ISB ();

            __WFI ();
            M0_InstDelay (VIDEO_ADAPTER_BACK_PORCH_IC);

//            VIDEO_RGB (ON);
            VIDEO_ADAPTER_LineOut (s_vFBOffset,
                        (s_vLineDup < s_vShownLines)? s_vShowAnd : s_vScanAnd,
                        (s_vLineDup < s_vShownLines)? s_vShowOr  : s_vScanOr);
//            VIDEO_RGB (OFF);
        }
        else
        // Vertical Blanking Interval (VFP + VSYNC + VBP)
        {
            // First VBI line
            if (s_vLineNum == MODELINE_VDISP + 1)
            {
                __WFI ();
                // Target work duration must not exceed single line time
                VIDEO_ADAPTER_Vbi ();
            }
            else
            // Second or higher VBI line
            {
                __WFI ();
                // Si se procesaron todas las lineas visibles y la primer linea
                // del vertical blanking, termina toda operacion pendiente de
                // acceso a memoria y genera una interrupcion al core M4.
                if (s_vLineNum == MODELINE_VDISP + 2)
                {
                    g_videoExchange.frameNumber = ++ s_vFrameCount;
                    //  __DSB ();
                    __SEV ();
                }
            }
        }

        // Avanza a la proxima linea
        if (++ s_vLineNum > MODELINE_VTOTAL)
        {
            // Si se procesaron todas inicia un nuevo frame tomando los datos
            // de exchange
            s_vFramebuffer  = g_videoExchange.framebuffer;
            s_vShownLines   = VIDEO_ADAPTER_FB_LINE_DUP
                                    - g_videoExchange.scanlines;
            s_vShowAnd      = g_videoExchange.showAnd;
            s_vShowOr       = g_videoExchange.showOr;
            s_vScanAnd      = g_videoExchange.scanAnd;
            s_vScanOr       = g_videoExchange.scanOr;
            s_vLineNum      = 1;
            s_vFBOffset     = s_vFramebuffer;
            s_vLineDup      = 0;
        }
        // Se fija si debe procesar la proxima linea del framebuffer o se sigue
        // duplicando la actual
        //else if (++ s_vLineDup > 4)
        else if (++ s_vLineDup >= VIDEO_ADAPTER_FB_LINE_DUP)
        {
            s_vLineDup = 0;
            s_vFBOffset += VIDEO_FB_WIDTH;
        }
	}
}


int main ()
{
	if (init ())
    {
        signalLoop ();
    }

    return 0;
}
