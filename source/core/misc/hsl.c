/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  hue-saturation-lightness color space conversions.

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

#include "embedul.ar/source/core/misc/hsl.h"
#include "embedul.ar/source/core/device/board.h"
#include <math.h>


struct RGB888 HSL_ToRgb (const uint8_t Hue, const uint8_t Saturation,
                         const uint8_t Lightness)
{
    const float s = (float)Saturation / 255.0f;
    const float l = (float)Lightness / 255.0f;
    const float c = (1.0f - fabsf(2.0f * l - 1.0f)) * s;
    const float x = c * (1.0f - fabsf(fmodf(Hue / 42.0f, 2.0f) - 1.0f));
    const float m = l - c / 2.0f;

    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;

    if (Hue <= 42)
    {
        r = c;
        g = x;
    }
    else if (Hue <= 85)
    {
        r = x;
        g = c;
    }
    else if (Hue <= 128)
    {
        g = c;
        b = x;
    }
    else if (Hue <= 170)
    {
        g = x;
        b = c;
    }
    else if (Hue <= 212)
    {
        r = x;
        b = c;
    }
    else
    {
        r = c;
        b = x;
    }

    return (struct RGB888) {
        .r = (uint8_t) (roundf ((r + m) * 255.0f)),
        .g = (uint8_t) (roundf ((g + m) * 255.0f)),
        .b = (uint8_t) (roundf ((b + m) * 255.0f))
    };
}
