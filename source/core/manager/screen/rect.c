/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [SCREEN MANAGER] clipped rectangle drawing.

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

#include "embedul.ar/source/core/manager/screen/rect.h"
#include "embedul.ar/source/core/device/board.h"


bool SCREEN_RECT_Draw (const enum SCREEN_Role Role,
                       const int32_t X, const int32_t Y, const uint16_t Width,
                       const uint16_t Height, const RGB332_Select ColorSel)
{
    const struct SCREEN_Context *const C = SCREEN_GetContext (Role);

    const int32_t Cx = SCREEN_Context__toClipX (C, X);
    const int32_t Cy = SCREEN_Context__toClipY (C, Y);

    if (SCREEN_Context__isClipRectOut (C, Cx, Cy, Width, Height))
    {
        return false;
    }

    struct SCREEN_ClippedRegion cr;
    SCREEN_Context__clip (C, Cx, Cy, Width, Height, &cr);

    const int32_t Sx = SCREEN_Context__fromClipX (C, cr.cx);
    const int32_t Sy = SCREEN_Context__fromClipY (C, cr.cy);

    uint8_t         * bb        = SCREEN_Context__backbufferXY (C, Sx, Sy);
    const uint16_t  Scanline    = C->driver->iface->Width;
    const uint8_t   Color       = RGB332_GetSelectedColor (&C->gradient,
                                                                ColorSel, 0);

    while (cr.yBottom --)
    {
        memset (bb, Color, (size_t)cr.cWidth);
        bb += Scanline;
    }

    return true;
}
