/*
  Embedul.ar™ embedded systems framework - http://embedul.ar
  
  lpc4337 cortex-m4 memory map.

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
ASSERT((DEFINED(__top_core_m0app_Ram)), "Undefined __top_core_m0app_Ram");

MEMORY
{
    MFlashA512      (rx)    : ORIGIN = 0x1a000000, LENGTH = 0x80000
    MFlashB512      (rx)    : ORIGIN = 0x1b000000 + __length_core_m0app_MFlash,
                              LENGTH = 0x80000 - __length_core_m0app_MFlash
    RamLoc32        (rwx)   : ORIGIN = 0x10000000, LENGTH = 0x8000
    RamLoc40        (rwx)   : ORIGIN = 0x10080000, LENGTH = 0xa000
    RamAHB32        (rwx)   : ORIGIN = 0x20000000, LENGTH = 0x8000
    RamAHB16        (rwx)   : ORIGIN = 0x20008000, LENGTH = 0x4000
    /* RamAHB_ETB16 reserved to the M0 Core */
    RamAHB_ETB16    (rwx)   : ORIGIN = 0x2000c000, LENGTH = 0x4000
}

__top_MFlashA512            = ORIGIN(MFlashA512) + LENGTH(MFlashA512);
__top_MFlashB512            = ORIGIN(MFlashB512) + LENGTH(MFlashB512);
__top_RamLoc32              = ORIGIN(RamLoc32) + LENGTH(RamLoc32);
__top_RamLoc40              = ORIGIN(RamLoc40) + LENGTH(RamLoc40);
__top_RamAHB32              = ORIGIN(RamAHB32) + LENGTH(RamAHB32);
__top_RamAHB16              = ORIGIN(RamAHB16) + LENGTH(RamAHB16);
__top_RamAHB_ETB16          = ORIGIN(RamAHB_ETB16) + LENGTH(RamAHB_ETB16);

__start_core_m0app_MFlash   = (__length_core_m0app_MFlash > 0)?
                                ORIGIN(MFlashB512) - __length_core_m0app_MFlash : 0;
__top_core_m0app_MFlash     = (__length_core_m0app_MFlash > 0)?
                                ORIGIN(MFlashB512) : 0;
