/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [SCREEN MANAGER] sprites.

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

#include "embedul.ar/source/core/manager/screen/sprite.h"
#include "embedul.ar/source/core/misc/collide.h"
#include "embedul.ar/source/core/device/board.h"


void SCREEN_SPRITE_Init (struct SCREEN_SPRITE *const S,
                         const struct SCREEN_TILEMAP *const Tilemap)
{
    BOARD_AssertState (S && Tilemap);

    OBJECT_Clear (S);
    
    S->tilemap = Tilemap;
}


bool SCREEN_SPRITE_Collide (const struct SCREEN_SPRITE *const S1,
                            const struct SCREEN_SPRITE *const S2)
{
    BOARD_AssertState (S1 && S2);
    
    const struct SCREEN_TILEMAP_Metrics *const S1M = 
                                &S1->tilemap->metrics[S1->animation];
    const struct SCREEN_TILEMAP_Metrics *const S2M =
                                &S2->tilemap->metrics[S2->animation];
    
    return COLLIDE_AABB_Test (S1->viewX, S1->viewX + S1M->bbw,
                              S1->viewY, S1->viewY + S1M->bbh,
                              S2->viewX, S2->viewX + S2M->bbw,
                              S2->viewY, S2->viewY + S2M->bbh);
}


void SCREEN_SPRITE_SetAnimation (struct SCREEN_SPRITE *const S,
                                const uint16_t Value)
{
    BOARD_AssertParams (S && Value < S->tilemap->animations);
    
    S->animation = Value;
}


void SCREEN_SPRITE_SetAnimationFrame (struct SCREEN_SPRITE *const S,
                                      const uint32_t Value)
{
    BOARD_AssertParams (S);
    
    S->frame = Value % S->tilemap->metrics[S->animation].frames;
}


bool SCREEN_SPRITE_Draw (const enum SCREEN_Role Role,
                         struct SCREEN_SPRITE *const S)
{
    BOARD_AssertParams (Role < SCREEN_Role__COUNT && S);

    return SCREEN_TILEMAP_Draw (Role, S->tilemap, S->viewX, S->viewY,
                                S->animation, S->frame, NULL, NULL, NULL);
}
