/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [VIDEO subsystem] rgb332 color selection and gradients.

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

#include <stdint.h>


typedef uint8_t     VIDEO_RGB332_Color;
typedef uint16_t    VIDEO_RGB332_Select;


struct VIDEO_RGB332_Gradient
{
    VIDEO_RGB332_Color c[8];
};


extern const struct VIDEO_RGB332_Gradient   VIDEO_RGB332_GradientRedYellow;
extern const struct VIDEO_RGB332_Gradient   VIDEO_RGB332_GradientRainbow;

#define VIDEO_RGB332_GRADIENT_DEFAULT       VIDEO_RGB332_GradientRainbow


#define VIDEO_RGB332_SGA_VALUE              0x0100
#define VIDEO_RGB332_SGI_FLAG               0x0200
#define VIDEO_RGB332_SC_MAX                 0x00FF


// Passing of color parameter by VIDEO_RGB332_Select
#define VIDEO_RGB332_SelectColor(x)             (x & VIDEO_RGB332_SC_MAX)
#define VIDEO_RGB332_SelectGradientAuto          VIDEO_RGB332_SGA_VALUE
#define VIDEO_RGB332_SelectGradientIndex(x)     (VIDEO_RGB332_SGI_FLAG | \
                                                                    (x & 0x07))

// Query value type on a given VIDEO_RGB332_Select
#define VIDEO_RGB332_SelectIsAColor(x)          (x <= VIDEO_RGB332_SC_MAX)
#define VIDEO_RGB332_SelectIsGradientAuto(x)    (x == VIDEO_RGB332_SGA_VALUE)
#define VIDEO_RGB332_SelectIsGradientIndex(x)   (x & VIDEO_RGB332_SGI_FLAG)

// Value extraction from VIDEO_RGB332_Select
#define VIDEO_RGB332_SelectGetColor(x)          (uint8_t) \
                                                    (x & VIDEO_RGB332_SC_MAX)
#define VIDEO_RGB332_SelectGetGradientIndex(x)  (x & 0x0007)


VIDEO_RGB332_Color
        VIDEO_RGB332_GetSelectedColor   (const struct VIDEO_RGB332_Gradient
                                         *const G,
                                         const VIDEO_RGB332_Select Select,
                                         const uint8_t Index);
void    VIDEO_RGB332_GradientClear      (struct VIDEO_RGB332_Gradient *const G);
void    VIDEO_RGB332_GradientCopy       (struct VIDEO_RGB332_Gradient *const G,
                                         const struct VIDEO_RGB332_Gradient
                                         *const Src);
void    VIDEO_RGB332_GradientShift      (struct VIDEO_RGB332_Gradient *const G,
                                         const int8_t Delta);
