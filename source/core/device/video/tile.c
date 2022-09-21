/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [VIDEO subsystem] tile drawing.

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

#include "embedul.ar/source/core/device/video/tile.h"
#include "embedul.ar/source/core/device/board.h"


bool VIDEO_TILE_Draw (const uint8_t *const TileData, const int32_t X, 
                      const int32_t Y)
{
    BOARD_AssertParams (TileData);
    
    const int32_t Cx = VIDEO_ScreenToClipX (X);
    const int32_t Cy = VIDEO_ScreenToClipY (Y);
    
    if (!VIDEO_ClipIsInside (Cx, Cy, 8, 8))
    {
        return false;
    }

    struct VIDEO_Clip c;
    VIDEO_Clip (Cx, Cy, 8, 8, &c);

    const int32_t Sx = VIDEO_ClipToScreenX (c.cx);
    const int32_t Sy = VIDEO_ClipToScreenY (c.cy);

    const uint8_t *mask = NULL;

    if (TileData[0])
    {
        // Mask data is located after tile header and pixels
        mask = TileData + 4 + (8 * 8);
    }

    // Skip tile header
    const uint8_t *td = TileData + 4;

    // Tile clipping
    td += 8 * c.yTop;
    if (mask)
    {
        mask += c.yTop;
    }

    uint8_t *v = VIDEO_BackbufferXY (Sx, Sy);
    const uint16_t Scanline = VIDEO_Width ();

    if (!mask)
    {    
        if (c.xLeft == 0 && c.xRight == 8)
        {    
            for (int32_t i = c.yTop; i < c.yBottom; ++i)
            {
                v[0] = td[0];
                v[1] = td[1];
                v[2] = td[2];
                v[3] = td[3];
                v[4] = td[4];
                v[5] = td[5];
                v[6] = td[6];
                v[7] = td[7];
                v += Scanline;
                td += 8;
            }
        }
        else 
        {
            for (int32_t i = c.yTop; i < c.yBottom; ++i)
            {
                uint32_t k = 0;
                for (int32_t j = c.xLeft; j < c.xRight; ++j)
                {
                    v[k] = td[j];
                    ++ k;
                }
                v += Scanline;
                td += 8;
            }        
        }
    }
    else
    {
        if (c.xLeft == 0 && c.xRight == 8)
        {    
            for (int32_t i = c.yTop; i < c.yBottom; ++i)
            {
                if (*mask & 0x01) v[0] = td[0];
                if (*mask & 0x02) v[1] = td[1];
                if (*mask & 0x04) v[2] = td[2];
                if (*mask & 0x08) v[3] = td[3];
                if (*mask & 0x10) v[4] = td[4];
                if (*mask & 0x20) v[5] = td[5];
                if (*mask & 0x40) v[6] = td[6];
                if (*mask & 0x80) v[7] = td[7];
                v += Scanline;
                td += 8;
                mask += 1;
            }
        }
        else
        {
            for (int32_t i = c.yTop; i < c.yBottom; ++i)
            {
                uint32_t k = 0;
                for (int32_t j = c.xLeft; j < c.xRight; ++j)
                {
                    if (*mask & (1 << j)) v[k] = td[j];
                    ++ k;
                }
                v += Scanline;
                td += 8;
                mask += 1;
            }        
        }
    }
    
    return true;
}
