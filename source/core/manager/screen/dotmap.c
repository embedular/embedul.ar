/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [SCREEN MANAGER] dotmap support code.

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

#include "embedul.ar/source/core/manager/screen/dotmap.h"
#include "embedul.ar/source/core/device/board.h"


// FIXME: ver que es ppe y si sirve para algo
void SCREEN_DOTMAP_Init (struct SCREEN_DOTMAP *const D,
                         const uint8_t *const Map, 
                         const uint32_t W, const uint32_t H,
                         const uint32_t Ppe)
{
    BOARD_AssertParams (D && Map && W && H && Ppe);
    
    OBJECT_Clear (D);
    
    D->map      = Map;
    D->width    = W;
    D->height   = H;
    D->ppe      = Ppe;
}


uint8_t SCREEN_DOTMAP_GetDot (struct SCREEN_DOTMAP *const M, const int32_t X, 
                             const int32_t Y)
{
    BOARD_AssertParams (M);
    
    const int32_t Wx = (X < 0 || X >= (int32_t)M->width)?
                                        (int32_t)((uint32_t)X % M->width)
                                            : X;

    const int32_t Wy = (Y < 0 || Y >= (int32_t)M->height)?
                                        (int32_t)((uint32_t)Y % M->height)
                                            : Y;

    return M->map[(uint32_t)Wy * M->width + (uint32_t)Wx];
}


void SCREEN_DOTMAP_Update (struct SCREEN_DOTMAP *const M, const int32_t X, 
                           const int32_t Y)
{
    BOARD_AssertParams (M);
    
    M->x = X >> 3;
    M->y = Y >> 3;

    memset (M->around, 0, sizeof(M->around));

    M->around[0] = SCREEN_DOTMAP_GetDot (M, M->x - 1, M->y - 1);
    M->around[1] = SCREEN_DOTMAP_GetDot (M, M->x    , M->y - 1);
    M->around[2] = SCREEN_DOTMAP_GetDot (M, M->x + 1, M->y - 1);
    M->around[3] = SCREEN_DOTMAP_GetDot (M, M->x - 1, M->y    );
    M->around[4] = SCREEN_DOTMAP_GetDot (M, M->x    , M->y    );
    M->around[5] = SCREEN_DOTMAP_GetDot (M, M->x + 1, M->y    );
    M->around[6] = SCREEN_DOTMAP_GetDot (M, M->x - 1, M->y + 1);
    M->around[7] = SCREEN_DOTMAP_GetDot (M, M->x    , M->y + 1);
    M->around[8] = SCREEN_DOTMAP_GetDot (M, M->x + 1, M->y + 1);
}


// Intended for debug/development only. No clipping.
void SCREEN_DOTMAP_DebugDraw (const enum SCREEN_Role Role,
                              struct SCREEN_DOTMAP *const M, 
                              const uint32_t X, const uint32_t Y)
{
    const struct SCREEN_Context *const C = SCREEN_GetContext (Role);

    BOARD_AssertParams (M);
    BOARD_AssertParams (X + M->width < C->driver->iface->Width &&
                        Y + M->height < C->driver->iface->Height);
    
    for (uint32_t j = 0; j < M->height; ++j)
    {
        for (uint32_t i = 0; i < M->width; ++i)
        {
            * SCREEN_Context__backbufferXY (C,
                (int32_t)(i + X), (int32_t)(j + Y)) =
                                        M->map[j * C->driver->iface->Width + i];
        }
    }
    
    // Current position in orange.
    * SCREEN_Context__backbufferXY (C, M->x + (int32_t)X, M->y + (int32_t)Y) =
                                        0xf0;
}

 
// Intended for debug/development only. No clipping.
void SCREEN_DOTMAP_DebugDrawAround (const enum SCREEN_Role Role,
                                    struct SCREEN_DOTMAP *const M,
                                    const uint32_t X, const uint32_t Y)
{
    const struct SCREEN_Context *const C = SCREEN_GetContext (Role);

    BOARD_AssertParams (M);
    BOARD_AssertParams (X + 3 < C->driver->iface->Width &&
                        Y + 3 < C->driver->iface->Height);

    * SCREEN_Context__backbufferXY(C, (int32_t)X  ,(int32_t)Y  ) = M->around[0];
    * SCREEN_Context__backbufferXY(C, (int32_t)X+1,(int32_t)Y  ) = M->around[1];
    * SCREEN_Context__backbufferXY(C, (int32_t)X+2,(int32_t)Y  ) = M->around[2];
    * SCREEN_Context__backbufferXY(C, (int32_t)X  ,(int32_t)Y+1) = M->around[3];
    * SCREEN_Context__backbufferXY(C, (int32_t)X+1,(int32_t)Y+1) = M->around[4];
    * SCREEN_Context__backbufferXY(C, (int32_t)X+2,(int32_t)Y+1) = M->around[5];
    * SCREEN_Context__backbufferXY(C, (int32_t)X  ,(int32_t)Y+2) = M->around[6];
    * SCREEN_Context__backbufferXY(C, (int32_t)X+1,(int32_t)Y+2) = M->around[7];
    * SCREEN_Context__backbufferXY(C, (int32_t)X+2,(int32_t)Y+2) = M->around[8];
}
