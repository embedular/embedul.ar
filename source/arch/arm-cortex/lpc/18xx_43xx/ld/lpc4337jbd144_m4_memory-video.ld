/*
  Embedul.ar™ embedded systems framework - http://embedul.ar
  
  lpc4337 cortex-m4 memory map, including video framebuffer and data exchange
  between software video adapter and application.

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

ASSERT((DEFINED(__length_video_exchange_Ram)? __length_video_exchange_Ram : 0), "Video exchange lenght is undefined or zero");
ASSERT((DEFINED(__top_RamAHB_exchange)), "Undefined __top_RamAHB_exchange");

ASSERT((DEFINED(__top_RamLoc40_framebuffer)), "Undefined __top_RamLoc40_framebuffer");
ASSERT((DEFINED(__top_RamAHB_framebuffer)), "Undefined __top_RamAHB_framebuffer");

ASSERT((DEFINED(__start_core_m0app_Ram)), "Undefined __start_core_m0app_Ram");
ASSERT((DEFINED(__top_core_m0app_Ram)), "Undefined __top_core_m0app_Ram");

MEMORY
{
    MFlashA512      (rx)    : ORIGIN = 0x1a000000, LENGTH = 0x80000
    MFlashB512      (rx)    : ORIGIN = 0x1b000000 + __length_core_m0app_MFlash,
                              LENGTH = 0x80000 - __length_core_m0app_MFlash
    RamLoc32        (rwx)   : ORIGIN = 0x10000000, LENGTH = 0x8000
    RamLoc40        (rwx)   : ORIGIN = __top_RamLoc40_framebuffer, LENGTH = 0x1000  /* Lower 36K taken by g_framebufferA */
                                                                                    /* RamAHB32 completely taken by g_framebufferB */
    RamAHB16        (rwx)   : ORIGIN = __top_RamAHB_framebuffer, LENGTH = 0x3000    /* Lower 4K taken by g_framebufferB */
    RamAHB_ETB16    (rwx)   : ORIGIN = __top_RamAHB_exchange,                       /* Lower 32B taken by vdaExchange, higher 8K taken by VDA running from SRAM */
                              LENGTH = 0x2000 - __length_video_exchange_Ram
}

__top_MFlashA512            = ORIGIN(MFlashA512) + LENGTH(MFlashA512);
__top_MFlashB512            = ORIGIN(MFlashB512) + LENGTH(MFlashB512);
__top_RamLoc32              = ORIGIN(RamLoc32) + LENGTH(RamLoc32);
__top_RamLoc40              = ORIGIN(RamLoc40) + LENGTH(RamLoc40);
__top_RamAHB16              = ORIGIN(RamAHB16) + LENGTH(RamAHB16);
__top_RamAHB_ETB16          = ORIGIN(RamAHB_ETB16) + LENGTH(RamAHB_ETB16);

__start_core_m0app_MFlash   = (__length_core_m0app_MFlash > 0)?
                                ORIGIN(MFlashB512) - __length_core_m0app_MFlash : 0;
__top_core_m0app_MFlash     = (__length_core_m0app_MFlash > 0)?
                                ORIGIN(MFlashB512) : 0;
