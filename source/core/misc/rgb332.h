/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  rgb332 8-bit color selection and gradients.

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


typedef uint8_t     RGB332_Color;
typedef uint16_t    RGB332_Select;


struct RGB332_Gradient
{
    RGB332_Color c[8];
};


extern const struct RGB332_Gradient      RGB332_GradientRedYellow;
extern const struct RGB332_Gradient      RGB332_GradientRainbow;

#define RGB332_GRADIENT_DEFAULT          RGB332_GradientRainbow


#define RGB332_SGA_VALUE                 0x0100
#define RGB332_SGI_FLAG                  0x0200
#define RGB332_SC_MAX                    0x00FF


// Passing of color parameter by RGB332_Select
#define RGB332_SelectColor(x)            (x & RGB332_SC_MAX)
#define RGB332_SelectGradientAuto         RGB332_SGA_VALUE
#define RGB332_SelectGradientIndex(x)    (RGB332_SGI_FLAG | \
                                                                    (x & 0x07))

// Query value type on a given RGB332_Select
#define RGB332_SelectIsAColor(x)         (x <= RGB332_SC_MAX)
#define RGB332_SelectIsGradientAuto(x)   (x == RGB332_SGA_VALUE)
#define RGB332_SelectIsGradientIndex(x)  (x & RGB332_SGI_FLAG)

// Value extraction from RGB332_Select
#define RGB332_SelectGetColor(x)         (uint8_t) \
                                                    (x & RGB332_SC_MAX)
#define RGB332_SelectGetGradientIndex(x) (x & 0x0007)


RGB332_Color
        RGB332_GetSelectedColor  (const struct RGB332_Gradient
                                         *const G, const RGB332_Select
                                         Select, const uint8_t Index);
void    RGB332_GradientClear     (struct RGB332_Gradient
                                         *const G);
void    RGB332_GradientCopy      (struct RGB332_Gradient *const G,
                                         const struct RGB332_Gradient
                                         *const Src);
void    RGB332_GradientShift     (struct RGB332_Gradient *const G,
                                         const int8_t Delta);
