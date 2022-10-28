/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [SCREEN MANAGER] clipped line drawing algorithm.

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

#include "embedul.ar/source/core/manager/screen/line.h"
#include "embedul.ar/source/core/device/board.h"


inline static void swap_i32 (int32_t *const A, int32_t *const B)
{
    const int32_t Av = *A;
    *A = *B;
    *B = Av;
}


static bool clipT (int32_t num, int32_t denom,
                   float *const Te, float *const Tl)
{
    if (denom == 0)
    {
        return (num <= 0);
    }

    float t = (float)num / denom;

    if (denom > 0)
    {
        if (t > *Tl)
        {
            return false;
        }

        if (t > *Te)
        {
            *Te = t;
        }
    }
    else
    {
        if (t < *Te)
        {
            return false;
        }

        if (t < *Tl)
        {
            *Tl = t;
        }
    }

    return true;
}


// Based on https://github.com/w8r/liang-barsky and
// https://hinjang.com/articles/04.html#eight
static bool liangBarsky (const struct SCREEN_Context *const C,
                         int32_t *const X1, int32_t *const Y1,
                         int32_t *const X2, int32_t *const Y2)
{
    const int32_t Dx = *X2 - *X1;
    const int32_t Dy = *Y2 - *Y1;

    float tE = 0.0f;
    float tL = 1.0f;

    if (clipT (C->clip.x1 - (*X1),  Dx, &tE, &tL) &&
        clipT (  *X1 - C->clip.x2, -Dx, &tE, &tL) &&
        clipT (C->clip.y1 - (*Y1),  Dy, &tE, &tL) &&
        clipT (  *Y1 - C->clip.y2, -Dy, &tE, &tL))
    {
        if (tL < 1.0f)
        {
            *X2 = (int32_t) (*X1 + tL * Dx);
            *Y2 = (int32_t) (*Y1 + tL * Dy);
        }

        if (tE > 0.0f)
        {
            *X1 += (int32_t) (tE * Dx);
            *Y1 += (int32_t) (tE * Dy);
        }

        return true;
    }

    return false;
}



bool SCREEN_LINE_Draw (const enum SCREEN_Role Role,
                       int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                       const RGB332_Select ColorSel)
{
    const struct SCREEN_Context *const C = SCREEN_GetContext (Role);

    // One or both endpoints are clipped
    if (SCREEN_Context__isPointOut(C, x1, y1) ||
        SCREEN_Context__isPointOut(C, x2, y2))
    {
        // No visible line intersects the clipping rectangle if both endpoints
        // are the same point that lie outside of the clipping region.
        if (x1 == x2 && y1 == y2)
        {
            return false;
        }

        // Liang Barsky line clipping
        if (!liangBarsky (C, &x1, &y1, &x2, &y2))
        {
            return false;
        }
    }

    // Absolute value of horizontal and vertical run-lenghts.
    const uint16_t H = (uint16_t) ((x2 > x1)? x2 - x1 : x1 - x2);
    const uint16_t V = (uint16_t) ((y2 > y1)? y2 - y1 : y1 - y2);
    
    const uint16_t Width = C->driver->iface->Width;

    // 'm' is an 8.24 fixed point number
    uint32_t    m;
    uint32_t    steps;
    int32_t     step;
    int32_t     delta;
    
    // 'step' travels the greater length in an always positive step; one pixel
    // per iteration in H or one line per iteration in V. 'delta' is the line
    // break in H or pixel jump in V when successive accumulations of 'm' in
    // 'ds' come to represent an integer value. This magnitude is positive or
    // negative depending on the orientation of the line.
    if (H >= V)
    {
        // It is better to increase memory address as you proceed through the
        // longest path. If this is not accomplished by going from point 1 to 2,
        // point values are swapped.
        if (x1 > x2)
        {
            swap_i32 (&x1, &x2);
            swap_i32 (&y1, &y2);
        }

        m       = (((uint32_t)V) << 24) / H;
        steps   = H;
        step    = 1;
        delta   = (y1 > y2)? -Width : Width;
    }
    else 
    {
        if (y1 > y2)
        {
            swap_i32 (&x1, &x2);
            swap_i32 (&y1, &y2);
        }

        m       = (((uint32_t)H) << 24) / V;
        steps   = V;
        step    = Width;
        delta   = (x1 > x2)? -1 : 1;
    }

    uint8_t *bb = SCREEN_Context__backbufferXY (C, x1, y1);
    uint32_t ds = 0x007FFFFF;   // 0.5
    const int32_t Zd[2] = { 0, delta };

    // Even if 'steps' value is zero (1 and 2 are the same point) it must write
    // at least that pixel.
    
    if (RGB332_SelectIsGradientAuto (ColorSel))
    {
        uint32_t pi = 0;
        const uint8_t *const Pal = C->gradient.c;
        
        do
        {
            *bb = Pal[pi ++];
            ds += m;
            bb += step;
            bb += Zd[ds >> 24];
            ds &= 0x00FFFFFF;
            pi &= 0x7;
        }
        while (steps --);
    }
    else
    {
        const uint8_t Color =
            RGB332_GetSelectedColor (&C->gradient, ColorSel, 0);

        do
        {
            *bb = Color;
            ds += m;
            bb += step;
            bb += Zd[ds >> 24];
            ds &= 0x00FFFFFF;
        }
        while (steps --);
    }
    
    return true;
}


bool SCREEN_LINE_DrawHoriz (const enum SCREEN_Role Role, 
                            int32_t x1, int32_t y1, const int32_t W,
                            const RGB332_Select ColorSel)
{
    return SCREEN_LINE_Draw (Role, x1, y1, x1 + W, y1, ColorSel);
}


bool SCREEN_LINE_DrawVert (const enum SCREEN_Role Role, 
                           int32_t x1, int32_t y1, const int32_t H,
                           const RGB332_Select ColorSel)
{
    return SCREEN_LINE_Draw (Role, x1, y1, x1, y1 + H, ColorSel);
}
