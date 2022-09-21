/*
  Embedul.ar™ embedded systems framework - http://embedul.ar
  
  lpc4337 cortex-m0 and m4 shared memory map.

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
__start_core_m0app_Ram  = 0x2000c000;    /* Full 16K bytes of RamAHB_ETB16 */
__length_core_m0app_Ram = 0x4000;
__top_core_m0app_Ram    = __start_core_m0app_Ram + __length_core_m0app_Ram;
