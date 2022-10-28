/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [SCREEN MANAGER] tilemap drawing.

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

#include "embedul.ar/source/core/manager/screen/tilemap.h"
#include "embedul.ar/source/core/manager/screen/tile.h"
#include "embedul.ar/source/core/device/board.h"


static void defaultTileProc (const enum SCREEN_Role Role,
                             const uint8_t *const *const Tiles,
                             const uint16_t Tile,
                             const int32_t X, const int32_t Y, 
                             const int32_t Tx, const int32_t Ty,
                             void *const Param)
{
    (void) Param;
    
    if (Tile)
    {
        SCREEN_TILE_Draw (Role, Tiles[Tile], X + (Tx << 3), Y + (Ty << 3));
    }
}


bool SCREEN_TILEMAP_Draw (const enum SCREEN_Role Role,
                         const struct SCREEN_TILEMAP *const Tilemap,
                         const int32_t X, const int32_t Y, 
                         const uint16_t Animation, const uint16_t Frame, 
                         SCREEN_TILEMAP_TileProc const Tileproc, 
                         SCREEN_TILEMAP_LineProc const Lineproc, 
                         void *const Param)
{
    const struct SCREEN_Context *const C = SCREEN_GetContext (Role);

    BOARD_AssertParams (Tilemap && Animation < Tilemap->animations &&
                        Tilemap->metrics &&
                        Frame < Tilemap->metrics[Animation].frames);

    const struct SCREEN_TILEMAP_Metrics *const
                        TMA = &Tilemap->metrics[Animation];

    BOARD_AssertParams (Frame < TMA->frames);
    
    // From 8x8 tiles to pixels
    const uint16_t PW = (uint16_t)(TMA->width  << 3);
    const uint16_t PH = (uint16_t)(TMA->height << 3);
    
    const int32_t Cx = SCREEN_Context__toClipX (C, X);
    const int32_t Cy = SCREEN_Context__toClipY (C, Y);

    if (SCREEN_Context__isClipRectOut (C, Cx, Cy, PW, PH))
    {
        return false;
    }
    
    struct SCREEN_ClippedRegion cr;
    SCREEN_Context__clip (C, Cx, Cy, PW, PH, &cr);

    // Pixels to 8x8 tiles. From which tile column/line it starts/ends drawing
    cr.xLeft    = cr.xLeft >> 3;
    cr.xRight   = (cr.xRight + 7) >> 3;
    cr.yTop     = cr.yTop >> 3;
    cr.yBottom  = (cr.yBottom + 7) >> 3;
    
    // Ignore adjustments in c.cx/c.cy
    const int32_t Sx = SCREEN_Context__fromClipX (C, Cx);
    const int32_t Sy = SCREEN_Context__fromClipY (C, Cy);
    
    SCREEN_TILEMAP_TileProc tileprocSel = 
                                (Tileproc)? Tileproc : defaultTileProc;

    switch (Tilemap->ptrType)
    {
        case SCREEN_TILEMAP_PtrType_8Bit:
        {
            const uint8_t *const TMAF = Tilemap->map8[Animation][Frame];
            for (int32_t ty = cr.yTop; ty < cr.yBottom; ++ty)
            {
                if (Lineproc)
                {
                    Lineproc (Sx, Sy, ty, Param);
                }
                const uint8_t *const MapLine = &TMAF[ty * TMA->width];
                for (int32_t tx = cr.xLeft; tx < cr.xRight; ++tx)
                {
                    tileprocSel (Role, Tilemap->tiles, MapLine[tx],
                                 Sx, Sy, tx, ty, Param);
                }
            }
            break;
        }

        case SCREEN_TILEMAP_PtrType_16Bit:
        {
            const uint16_t *const TMAF = Tilemap->map16[Animation][Frame];
            for (int32_t ty = cr.yTop; ty < cr.yBottom; ++ty)
            {
                if (Lineproc)
                {
                    Lineproc (Sx, Sy, ty, Param);
                }
                const uint16_t *const MapLine = &TMAF[ty * TMA->width];
                for (int32_t tx = cr.xLeft; tx < cr.xRight; ++tx)
                {
                    tileprocSel (Role, Tilemap->tiles, MapLine[tx],
                                 Sx, Sy, tx, ty, Param);
                }
            }
            break;
        }

        default:
            BOARD_AssertUnexpectedValue (NOBJ, (uint32_t)Tilemap->ptrType);
            break;
    }

    return true;
}


inline bool SCREEN_TILEMAP_DrawLite (const enum SCREEN_Role Role,
                                    const struct SCREEN_TILEMAP *const Tilemap,
                                    const int32_t X, const int32_t Y,
                                    const uint16_t Animation,
                                    const uint16_t Frame)
{
    return SCREEN_TILEMAP_Draw (Role, Tilemap, X, Y, Animation, Frame,
                                NULL, NULL, NULL);
}


bool SCREEN_TILEMAP_DrawRepeat (const enum SCREEN_Role Role,
                               const struct SCREEN_TILEMAP *const Tilemap, 
                               const int32_t X, const int32_t Y, 
                               const uint16_t Animation, const uint16_t Frame, 
                               const enum SCREEN_TILEMAP_RepeatDir Rd, 
                               SCREEN_TILEMAP_TileProc const Tileproc,
                               SCREEN_TILEMAP_LineProc const Lineproc,
                               void *const Param)
{
    if (Rd == SCREEN_TILEMAP_RepeatDir_None)
    {
        return SCREEN_TILEMAP_Draw (Role, Tilemap, X, Y, Animation, Frame,
                                    Tileproc, Lineproc, Param);
    }

    const struct SCREEN_Context *const C = SCREEN_GetContext (Role);

    BOARD_AssertParams (Tilemap && Animation < Tilemap->animations &&
                        Tilemap->metrics);

    const struct SCREEN_TILEMAP_Metrics *const 
                        TMA = &Tilemap->metrics[Animation];

    // From 8x8 tiles to pixels
    const uint16_t PW = (uint16_t)(TMA->width  << 3);
    const uint16_t PH = (uint16_t)(TMA->height << 3);

    const int32_t Cx = SCREEN_Context__toClipX (C, X);
    const int32_t Cy = SCREEN_Context__toClipY (C, Y);
    
    int32_t repeatBX = Cx;
    int32_t repeatEX = Cx + PW;
    int32_t repeatBY = Cy;
    int32_t repeatEY = Cy + PH;

    if (Rd == SCREEN_TILEMAP_RepeatDir_Horiz || 
        Rd == SCREEN_TILEMAP_RepeatDir_All)
    {
        if (Cx > 0)
        {
            // Repeat to the left
            const int32_t RepeatLeft = (Cx / PW) + 1;
            repeatBX -= RepeatLeft * PW;
        }

        if (Cx > C->clip.width - PW)
        {
            // Repeat to the right
            const int32_t RepeatRight =
                            ((C->clip.width - (Cx + PW)) / PW) + 1;
            repeatEX += RepeatRight * PW;
        }
    }
    
    if (Rd == SCREEN_TILEMAP_RepeatDir_Vert ||
        Rd == SCREEN_TILEMAP_RepeatDir_All)
    {
        if (Cy > 0)
        {
            // Repeat up
            const int32_t RepeatUp = (Cy / PH) + 1;
            repeatBY -= RepeatUp * PH;
        }
        
        if (Cy + PH < C->clip.height)
        {
            // Repeat down
            const int32_t RepeatDown =
                            ((C->clip.height - (Cy + PH)) / PH) + 1;
            repeatEY += RepeatDown * PW;
        }
    }

    repeatBX = SCREEN_Context__fromClipX (C, repeatBX);
    repeatEX = SCREEN_Context__fromClipX (C, repeatEX);
    repeatBY = SCREEN_Context__fromClipY (C, repeatBY);
    repeatEY = SCREEN_Context__fromClipY (C, repeatEY);
    
    for (int32_t dy = repeatBY; dy < repeatEY; dy += PH)
    {
        for (int32_t dx = repeatBX; dx < repeatEX; dx += PW)
        {
            SCREEN_TILEMAP_Draw (Role, Tilemap, dx, dy, Animation, Frame,
                                 Tileproc, Lineproc, Param);
        }
    }
    
    return true;
}
