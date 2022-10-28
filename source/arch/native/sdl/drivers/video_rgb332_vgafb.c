/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [VIDEO driver] rgb322 640x480 framebuffer using sdl.

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

#include "embedul.ar/source/arch/native/sdl/drivers/video_rgb332_vgafb.h"
#include "embedul.ar/source/core/device/board.h"


static void updateSurface (struct VIDEO *const V,
                           const struct VIDEO_RGB332_UpdateInfo *const Ui);


static const struct VIDEO_IFACE VIDEO_IFACE_VGAFB =
{
    VIDEO_RGB332_IFACE("sdl vga framebuffer", 640, 480)
};


void VIDEO_RGB332_VGAFB_Init (struct VIDEO_RGB332_VGAFB *const S)
{
    S->device.displayWidth  = 640;
    S->device.displayHeight = 480;
    S->device.updateSurface = updateSurface;

    VIDEO_Init ((struct VIDEO *)S, &VIDEO_IFACE_VGAFB, S->framebuffer, NULL);
}


static void updateSurface (struct VIDEO *const V,
                           const struct VIDEO_RGB332_UpdateInfo *const Ui)
{
    uint8_t *b = V->frontbuffer;
    uint8_t *s = Ui->Surface;

    if (Ui->SurfacePitch == 640)
    {
        memcpy (s, b, 640 * 480);
    }
    else 
    {
        for (uint32_t h = 0; h < 480; ++h)
        {
            memcpy (s, b, 640);
            s += Ui->SurfacePitch;
            b += 640;
        }
    }

    // Direct-color operations (scanlines unsupported)
    if (Ui->ShowAnd32 != 0xFF || Ui->ShowOr32 != 0x00)
    {
        for (uint32_t h = 0; h < 480; ++h)
        {
            uint32_t *s32 = (uint32_t *)&Ui->Surface[h * Ui->SurfacePitch];

            for (uint32_t w = 0; w < 160; ++w)
            {
                s32[w] &= Ui->ShowAnd32;
                s32[w] |= Ui->ShowOr32;
            }
        }
    }
}
