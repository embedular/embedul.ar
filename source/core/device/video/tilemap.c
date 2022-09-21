/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [VIDEO subsystem] tilemap drawing.

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

#include "embedul.ar/source/core/device/video/tilemap.h"
#include "embedul.ar/source/core/device/video/tile.h"
#include "embedul.ar/source/core/device/board.h"


static void defaultTileProc (const uint8_t *const *const Tiles,
                             const uint16_t Tile,
                             const int32_t X, const int32_t Y, 
                             const int32_t Tx, const int32_t Ty,
                             void *const Param)
{
    (void) Param;
    
    if (Tile)
    {
        VIDEO_TILE_Draw (Tiles[Tile], X + (Tx << 3), Y + (Ty << 3));
    }
}


bool VIDEO_TILEMAP_Draw (const struct VIDEO_TILEMAP *const Tilemap,
                         const int32_t X, const int32_t Y, 
                         const uint16_t Animation, const uint16_t Frame, 
                         VIDEO_TILEMAP_TileProc const Tileproc, 
                         VIDEO_TILEMAP_LineProc const Lineproc, 
                         void *const Param)
{
    BOARD_AssertParams (Tilemap && Animation < Tilemap->animations &&
                         Tilemap->metrics &&
                         Frame < Tilemap->metrics[Animation].frames);

    const struct VIDEO_TILEMAP_Metrics *const
                        TMA = &Tilemap->metrics[Animation];

    BOARD_AssertParams (Frame < TMA->frames);
    
    // From 8x8 tiles to pixels
    const uint16_t PW = (uint16_t)(TMA->width  << 3);
    const uint16_t PH = (uint16_t)(TMA->height << 3);
    
    const int32_t Cx = VIDEO_ScreenToClipX (X);
    const int32_t Cy = VIDEO_ScreenToClipY (Y);

    if (!VIDEO_ClipIsInside (Cx, Cy, PW, PH))
    {
        return false;
    }
    
    struct VIDEO_Clip c;
    VIDEO_Clip (Cx, Cy, PW, PH, &c);

    // Pixels to 8x8 tiles. From which tile column/line it starts/ends drawing
    c.xLeft     = c.xLeft >> 3;
    c.xRight    = (c.xRight + 7) >> 3;
    c.yTop      = c.yTop >> 3;
    c.yBottom   = (c.yBottom + 7) >> 3;
    
    // Ignore adjustments in c.cx/c.cy
    const int32_t Sx = VIDEO_ClipToScreenX (Cx);
    const int32_t Sy = VIDEO_ClipToScreenY (Cy);
    
    VIDEO_TILEMAP_TileProc tileprocSel = (Tileproc)? Tileproc : defaultTileProc;

    if (Tilemap->flags & VIDEO_TILEMAP_FLAG_PTR_TYPE_16_BIT)
    {
        const uint16_t *const TMAF = Tilemap->map16[Animation][Frame];
        for (int32_t ty = c.yTop; ty < c.yBottom; ++ty)
        {
            if (Lineproc)
            {
                Lineproc (Sx, Sy, ty, Param);
            }
            const uint16_t *const MapLine = &TMAF[ty * TMA->width];
            for (int32_t tx = c.xLeft; tx < c.xRight; ++tx)
            {
                tileprocSel (Tilemap->tiles, MapLine[tx], Sx, Sy, tx, ty, 
                             Param);
            }
        }
    }
    // TILEMAP_FLAG_PTR_TYPE_8_BIT
    else 
    {
        const uint8_t *const TMAF = Tilemap->map8[Animation][Frame];
        for (int32_t ty = c.yTop; ty < c.yBottom; ++ty)
        {
            if (Lineproc)
            {
                Lineproc (Sx, Sy, ty, Param);
            }
            const uint8_t *const MapLine = &TMAF[ty * TMA->width];
            for (int32_t tx = c.xLeft; tx < c.xRight; ++tx)
            {
                tileprocSel (Tilemap->tiles, MapLine[tx], Sx, Sy, tx, ty,
                             Param);
            }
        }
    }

    return true;
}


inline bool VIDEO_TILEMAP_DrawLite (const struct VIDEO_TILEMAP *const Tilemap,
                                    const int32_t X, const int32_t Y,
                                    const uint16_t Animation,
                                    const uint16_t Frame)
{
    return VIDEO_TILEMAP_Draw (Tilemap, X, Y, Animation, Frame, NULL, NULL,
                               NULL);
}


bool VIDEO_TILEMAP_DrawRepeat (const struct VIDEO_TILEMAP *const Tilemap, 
                               const int32_t X, const int32_t Y, 
                               const uint16_t Animation, const uint16_t Frame, 
                               const enum VIDEO_TILEMAP_RepeatDir Rd, 
                               VIDEO_TILEMAP_TileProc const Tileproc,
                               VIDEO_TILEMAP_LineProc const Lineproc,
                               void *const Param)
{
    if (Rd == TILEMAP_RepeatDir_None)
    {
        return VIDEO_TILEMAP_Draw (Tilemap, X, Y, Animation, Frame, Tileproc,
                                   Lineproc, Param);
    }

    BOARD_AssertParams (Tilemap && Animation < Tilemap->animations &&
                         Tilemap->metrics);

    const struct VIDEO_TILEMAP_Metrics *const 
                        TMA = &Tilemap->metrics[Animation];

    // From 8x8 tiles to pixels
    const uint16_t PW = (uint16_t)(TMA->width  << 3);
    const uint16_t PH = (uint16_t)(TMA->height << 3);

    int32_t Cx = VIDEO_ScreenToClipX (X);
    int32_t Cy = VIDEO_ScreenToClipY (Y);
    
    int32_t repeatBX = Cx;
    int32_t repeatEX = Cx + PW;
    int32_t repeatBY = Cy;
    int32_t repeatEY = Cy + PH;

    if (Rd == TILEMAP_RepeatDir_Horiz || Rd == TILEMAP_RepeatDir_All)
    {
        if (Cx > 0)
        {
            // Repeat to the left
            const int32_t RepeatLeft = (Cx / PW) + 1;
            repeatBX -= RepeatLeft * PW;
        }
        
        if (Cx > VIDEO_ClipWidth() - PW)
        {
            // Repeat to the right
            const int32_t RepeatRight =
                            ((VIDEO_ClipWidth() - (Cx + PW)) / PW) + 1;
            repeatEX += RepeatRight * PW;
        }
    }
    
    if (Rd == TILEMAP_RepeatDir_Vert || Rd == TILEMAP_RepeatDir_All)
    {
        if (Cy > 0)
        {
            // Repeat up
            const int32_t RepeatUp = (Cy / PH) + 1;
            repeatBY -= RepeatUp * PH;
        }
        
        if (Cy + PH < VIDEO_ClipHeight())
        {
            // Repeat down
            const int32_t RepeatDown =
                            ((VIDEO_ClipHeight() - (Cy + PH)) / PH) + 1;
            repeatEY += RepeatDown * PW;
        }
    }

    repeatBX = VIDEO_ClipToScreenX (repeatBX);
    repeatEX = VIDEO_ClipToScreenX (repeatEX);
    repeatBY = VIDEO_ClipToScreenY (repeatBY);
    repeatEY = VIDEO_ClipToScreenY (repeatEY);
    
    for (int32_t dy = repeatBY; dy < repeatEY; dy += PH)
    {
        for (int32_t dx = repeatBX; dx < repeatEX; dx += PW)
        {
            VIDEO_TILEMAP_Draw (Tilemap, dx, dy, Animation, Frame, Tileproc, 
                                Lineproc, Param);
        }
    }
    
    return true;
}
