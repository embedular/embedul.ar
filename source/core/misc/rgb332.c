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

#include "embedul.ar/source/core/misc/rgb332.h"
#include "embedul.ar/source/core/device/board.h"


const struct RGB332_Gradient RGB332_GradientRedYellow =
{
    .c = { 0xC0, 0xC4, 0xC8, 0xCC, 0xD0, 0xD4, 0xD8, 0xDC }
};

const struct RGB332_Gradient RGB332_GradientRainbow =
{
    .c = { 0xE0, 0xE8, 0xF0, 0xFC, 0xBC, 0x3C, 0x1A, 0x0B }
};



RGB332_Color RGB332_GetSelectedColor (
                            const struct RGB332_Gradient *const G,
                            const RGB332_Select Select,
                            const uint8_t Index)
{
    if (RGB332_SelectIsAColor (Select))
    {
        return RGB332_SelectGetColor (Select);
    }

    BOARD_AssertParams (G);

    if (RGB332_SelectIsGradientAuto (Select))
    {
        return G->c[(Index & 0x07)];
    }

    return G->c[RGB332_SelectGetGradientIndex (Select)];
}


void RGB332_GradientClear (struct RGB332_Gradient *const G)
{
    BOARD_AssertParams (G);
    OBJECT_Clear (G);
}


void RGB332_GradientCopy (struct RGB332_Gradient *const G,
                                 const struct RGB332_Gradient *const Src)
{
    BOARD_AssertParams (G);    
    memcpy (G->c, Src->c, sizeof(G->c));
}


void RGB332_GradientShift (struct RGB332_Gradient *const G,
                                  const int8_t Delta)
{
    BOARD_AssertParams (G);

    struct RGB332_Gradient pAux = { 0 };
    RGB332_GradientCopy (&pAux, G);
    
    G->c[0] = pAux.c[(Delta + 0) & 0x07];
    G->c[1] = pAux.c[(Delta + 1) & 0x07];
    G->c[2] = pAux.c[(Delta + 2) & 0x07];
    G->c[3] = pAux.c[(Delta + 3) & 0x07];
    G->c[4] = pAux.c[(Delta + 4) & 0x07];
    G->c[5] = pAux.c[(Delta + 5) & 0x07];
    G->c[6] = pAux.c[(Delta + 6) & 0x07];
    G->c[7] = pAux.c[(Delta + 7) & 0x07];
}
