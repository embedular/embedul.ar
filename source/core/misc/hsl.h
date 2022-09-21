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

#pragma once

#include "embedul.ar/source/core/misc/rgb888.h"


#define HSL_HUE_RED                 0
#define HSL_HUE_YELLOW              42
#define HSL_HUE_GREEN               85
#define HSL_HUE_CYAN                128
#define HSL_HUE_BLUE                170
#define HSL_HUE_MAGENTA             212

#define HSL_SATURATION_GRAY         0
#define HSL_SATURATION_FULL         255

#define HSL_LUMINANCE_DARKEST       0
#define HSL_LUMINANCE_FULLSAT       128
#define HSL_LUMINANCE_BRIGHTEST     255


struct RGB888 HSL_ToRgb (const uint8_t Hue, const uint8_t Saturation,
                         const uint8_t Lightness);
