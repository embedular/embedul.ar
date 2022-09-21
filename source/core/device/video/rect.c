/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [VIDEO subsystem] clipped rectangle drawing.

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

#include "embedul.ar/source/core/device/video/rect.h"
#include "embedul.ar/source/core/device/board.h"


bool VIDEO_RECT_Draw (const int32_t X, const int32_t Y, const uint16_t Width,
                      const uint16_t Height, const VIDEO_RGB332_Select ColorSel)
{
    const int32_t Cx = VIDEO_ScreenToClipX (X);
    const int32_t Cy = VIDEO_ScreenToClipY (Y);

    if (!VIDEO_ClipIsInside (Cx, Cy, Width, Height))
    {
        return false;
    }

    struct VIDEO_Clip c;
    VIDEO_Clip (Cx, Cy, Width, Height, &c);

    const int32_t Sx = VIDEO_ClipToScreenX (c.cx);
    const int32_t Sy = VIDEO_ClipToScreenY (c.cy);

    uint8_t *v = VIDEO_BackbufferXY (Sx, Sy);
    const uint16_t Scanline = VIDEO_Width ();
    const uint8_t Color = VIDEO_RGB332_GetSelectedColor (
                                            VIDEO_Gradient(), ColorSel, 0);

    while (c.yBottom --)
    {
        memset (v, Color, (size_t)c.cWidth);
        v += Scanline;
    }

    return true;
}
