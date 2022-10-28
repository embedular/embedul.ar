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

#pragma once

#include <stdint.h>
#include <stdbool.h>


struct SCREEN_DOTMAP
{
    const uint8_t   * map;
    uint32_t        width;
    uint32_t        height;
    uint32_t        ppe;
    int32_t         x;
    int32_t         y;
    uint8_t         around[9];    
};


enum SCREEN_Role;


void        SCREEN_DOTMAP_Init              (struct SCREEN_DOTMAP *const M, 
                                             const uint8_t *const Map,
                                             const uint32_t W, const uint32_t H,
                                             const uint32_t Ppe);
uint8_t     SCREEN_DOTMAP_GetDot            (struct SCREEN_DOTMAP *const M,
                                             const int32_t X, const int32_t Y);
void        SCREEN_DOTMAP_Update            (struct SCREEN_DOTMAP *const M,
                                             const int32_t X, const int32_t Y);
void        SCREEN_DOTMAP_DebugDraw         (const enum SCREEN_Role Role,
                                             struct SCREEN_DOTMAP *const M,
                                             const uint32_t X, 
                                             const uint32_t Y);
void        SCREEN_DOTMAP_DebugDrawAround   (const enum SCREEN_Role Role,
                                             struct SCREEN_DOTMAP *const M, 
                                             const uint32_t X, 
                                             const uint32_t Y);
