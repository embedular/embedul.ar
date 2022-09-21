/*
  Embedul.ar™ embedded systems framework - http://embedul.ar
  
  lpc4337 cortex-m0 and m4 shared memory map, including video framebuffer and
  data exchange between software video adapter and application.

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

/*
--------------------------------------------------------------------------------
Cortex-M0 Core RAM
--------------------------------------------------------------------------------
*/
__start_core_m0app_Ram  = 0x2000e000;    /* High 8K bytes of RamAHB_ETB16 */
__length_core_m0app_Ram = 0x2000;
__top_core_m0app_Ram    = __start_core_m0app_Ram + __length_core_m0app_Ram;

/*
--------------------------------------------------------------------------------
Video adapter and application data exchange area size
--------------------------------------------------------------------------------
*/
__length_video_exchange_Ram     = 0x20;

/*
--------------------------------------------------------------------------------
Framebuffer A
--------------------------------------------------------------------------------
*/
__start_RamLoc40_framebuffer    = 0x10080000;
__top_RamLoc40_framebuffer      = __start_RamLoc40_framebuffer + 0x9000;
g_framebufferA                  = __start_RamLoc40_framebuffer;

/*
--------------------------------------------------------------------------------
Framebuffer A
--------------------------------------------------------------------------------
    RamAHB32 (ORIGIN = 0x20000000, LENGTH = 0x8000) completele taken by 
    g_framebufferB.
*/
__start_RamAHB_framebuffer      = 0x20000000;
__top_RamAHB_framebuffer        = __start_RamAHB_framebuffer + 0x9000;
g_framebufferB                  = __start_RamAHB_framebuffer;

/*
--------------------------------------------------------------------------------
Data exchange between video adapter (M0) and application (M4)
--------------------------------------------------------------------------------
*/
__start_RamAHB_exchange = 0x2000c000;
__top_RamAHB_exchange   = __start_RamAHB_exchange + __length_video_exchange_Ram;
g_videoExchange         = __start_RamAHB_exchange;
