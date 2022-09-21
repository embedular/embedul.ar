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

#pragma once

#include <stdint.h>
#include <stdbool.h>


#define VIDEO_TILEMAP_FLAGMASK_PTR_TYPE         0x0001
#define VIDEO_TILEMAP_FLAG_PTR_TYPE_8_BIT       0x0000
#define VIDEO_TILEMAP_FLAG_PTR_TYPE_16_BIT      0x0001


struct VIDEO_TILEMAP_Metrics
{
    uint16_t            frames;    
    uint16_t            width;
    uint16_t            height;
    uint8_t             bbw;
    uint8_t             bbh;
};


struct VIDEO_TILEMAP
{
    uint16_t            flags;
    uint16_t            animations;
    union
    {
        const void      *const *const * map;
        const uint8_t   *const *const * map8;
        const uint16_t  *const *const * map16;
    };
    const uint8_t       *const * tiles;
    const struct VIDEO_TILEMAP_Metrics
                        * metrics;
};


enum VIDEO_TILEMAP_RepeatDir
{
    TILEMAP_RepeatDir_None = 0,
    TILEMAP_RepeatDir_Horiz,
    TILEMAP_RepeatDir_Vert,
    TILEMAP_RepeatDir_All
};


typedef void (* VIDEO_TILEMAP_TileProc) (const uint8_t *const *const Tiles,
                                         const uint16_t Tile,
                                         const int32_t X, const int32_t Y, 
                                         const int32_t Tx, const int32_t Ty,
                                         void *const Param);

typedef void (* VIDEO_TILEMAP_LineProc) (int32_t x, int32_t y, int32_t ty,
                                         void *param);


bool VIDEO_TILEMAP_Draw         (const struct VIDEO_TILEMAP *const Tilemap,
                                 const int32_t X, const int32_t Y, 
                                 const uint16_t Animation, const uint16_t Frame, 
                                 VIDEO_TILEMAP_TileProc const Tileproc, 
                                 VIDEO_TILEMAP_LineProc const Lineproc, 
                                 void *const Param);
bool VIDEO_TILEMAP_DrawLite     (const struct VIDEO_TILEMAP *const Tilemap,
                                 const int32_t X, const int32_t Y,
                                 const uint16_t Animation,
                                 const uint16_t Frame);
bool VIDEO_TILEMAP_DrawRepeat   (const struct VIDEO_TILEMAP *const Tilemap, 
                                 const int32_t X, const int32_t Y, 
                                 const uint16_t Animation, const uint16_t Frame, 
                                 const enum VIDEO_TILEMAP_RepeatDir Rd, 
                                 VIDEO_TILEMAP_TileProc const Tileproc,
                                 VIDEO_TILEMAP_LineProc const Lineproc,
                                 void *const Param);
