/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [VIDEO driver] video adapter simulation using sdl: rgb332 256x144 framebuffer
                 integer-scaled to a 1280x720 video signal.

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

#include "embedul.ar/source/arch/native/sdl/drivers/video_rgb332_adapter_sim.h"
#include "embedul.ar/source/core/device/board.h"


static void updateSurface (struct VIDEO *const V,
                           const struct VIDEO_RGB332_UpdateInfo *const Ui);


static const struct VIDEO_IFACE VIDEO_IFACE_ADAPTER_SIM =
{
    VIDEO_RGB332_IFACE("sdl video adapter simulator", 256, 144)
};


void VIDEO_RGB332_ADAPTER_SIM_Init (struct VIDEO_RGB332_ADAPTER_SIM *const S)
{
    S->device.displayWidth  = 1280;
    S->device.displayHeight = 720;
    S->device.updateSurface = updateSurface;

    VIDEO_Init ((struct VIDEO *)S, &VIDEO_IFACE_ADAPTER_SIM,
                S->framebufferA, S->framebufferB);
}


static void updateSurface (struct VIDEO *const V,
                           const struct VIDEO_RGB332_UpdateInfo *const Ui)
{
    uint8_t *b = V->frontbuffer;
    uint8_t *s = Ui->Surface;

    // 256x144 framebuffer integer-scaled to a 1280x720 signal
    for (int h = 0; h < 144; ++h)
    {
        uint8_t line[1280];

        for (uint32_t whd = 0; whd < 1280; whd += 5)
        {
            line[whd+0] = *b;
            line[whd+1] = *b;
            line[whd+2] = *b;
            line[whd+3] = *b;
            line[whd+4] = *b;
            b ++;
        }

        // Visible scanlines, or direct-color operations
        if (Ui->ShowAnd32 != 0xFF || Ui->ShowOr32 != 0x00 || V->scanlines)
        {
            // 4 pixes per iteration (320 x 4 = 1280)
            uint32_t lineShow[320];
            uint32_t lineScan[320];

            memcpy (lineShow, line, 1280);
            memcpy (lineScan, line, 1280);

            for (uint32_t i = 0; i < 320; ++i)
            {
                lineShow[i] &= Ui->ShowAnd32;
                lineShow[i] |= Ui->ShowOr32;
                lineScan[i] &= Ui->ScanAnd32;
                lineScan[i] |= Ui->ScanOr32;
            }

            memcpy (s, (5 > V->scanlines)? lineShow : lineScan, 1280);
            s += Ui->SurfacePitch;
            memcpy (s, (4 > V->scanlines)? lineShow : lineScan, 1280);
            s += Ui->SurfacePitch;
            memcpy (s, (3 > V->scanlines)? lineShow : lineScan, 1280);
            s += Ui->SurfacePitch;
            memcpy (s, (2 > V->scanlines)? lineShow : lineScan, 1280);
            s += Ui->SurfacePitch;
            memcpy (s, (1 > V->scanlines)? lineShow : lineScan, 1280);
            s += Ui->SurfacePitch;
        }
        else 
        {
            memcpy (s, line, 1280); s += Ui->SurfacePitch;
            memcpy (s, line, 1280); s += Ui->SurfacePitch;
            memcpy (s, line, 1280); s += Ui->SurfacePitch;
            memcpy (s, line, 1280); s += Ui->SurfacePitch;
            memcpy (s, line, 1280); s += Ui->SurfacePitch;
        }
    }
}
