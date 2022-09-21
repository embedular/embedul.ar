/*
  Embedul.ar™ embedded systems framework - http://embedul.ar
  
  lpc4337 cortex-m0 memory map.

  Copyright 2018-2022 Santiago Germino
  <sgermino@embedul.ar>

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

ASSERT((DEFINED(__length_core_m0app_MFlash)), "Undefined __length_core_m0app_MFlash");
ASSERT((DEFINED(__start_core_m0app_Ram)), "Undefined __start_core_m0app_Ram");
ASSERT((DEFINED(__length_core_m0app_Ram)? __length_core_m0app_Ram : 1), "M0 ram size must not be zero");
ASSERT((DEFINED(__top_core_m0app_Ram)), "Undefined __top_core_m0app_Ram");

MEMORY
{
    /* (4K Min Erase/Write Page Size) */
    MFlashB512      (rx)    : ORIGIN = 0x1b000000, LENGTH = __length_core_m0app_MFlash
    Ram_m0app       (rwx)   : ORIGIN = __start_core_m0app_Ram, LENGTH = __length_core_m0app_Ram
}

__top_MFlashB512    = ORIGIN(MFlashB512) + LENGTH(MFlashB512);
__start_Ram_m0app   = ORIGIN(Ram_m0app);
__top_Ram_m0app     = ORIGIN(Ram_m0app) + LENGTH(Ram_m0app);
