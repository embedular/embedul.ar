/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [CORE] 64-bit aligned static memory pool.

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

#include "embedul.ar/source/core/queue.h"
#include <stddef.h>


/**
 * Description
 * ===========
 *
 * Manages an already allocated memory region at a specified address by
 * assigning blocks of a requested size. Once assigned, blocks are static
 * and cannot be released or freed. This approach has the following
 * advantages:
 *
 * - The managed memory region is divided according to a fixed sequence
 *   of block requests. The resulting memory map is deterministic, and the
 *   user documentation can precisely describe its arrangement.
 * - No dynamic memory allocation implies no memory fragmentation and
 *   no need to handle the complexity of sudden out of memory conditions.
 * - There might be several memory pools residing on different
 *   memory types (a given SRAM bank, external SDRAM, etc.), each one
 *   serving a different purpose.
 *
 * The managed memory region address, each block assigned from it and their
 * sizes align to a 64-bit boundary.
 *
 *
 * Design and development status
 * =============================
 *
 * Feature-complete.
 *
 *
 * Changelog
 * =========
 *
 * ======= ========== =================== ======================================
 * Version Date*      Author              Comment
 * ======= ========== =================== ======================================
 * 1.0.0   2022.9.7   sgermino            Initial release.
 * ======= ========== =================== ======================================
 *
 * \* Date format is Year.Month.Day.
 *
 *
 * API reference
 * =============
 */


/**
 * Request a block with the remaining mempool capacity.
 */
#define MEMPOOL_BLOCKSIZE_REMAINS       ((uint32_t) -1)


/**
 * The user should treat this as an opaque structure. No member should be
 * directly accessed or modified.
 */
struct MEMPOOL
{
    uintptr_t       baseAddr;
    uint32_t        size;
    uint32_t        used;
    struct QUEUE    blocks;
};


void        MEMPOOL_Init            (struct MEMPOOL *const M,
                                     const uintptr_t BaseAddr,
                                     const uint32_t Size);
void *      MEMPOOL_Block           (struct MEMPOOL *const M,
                                     const uint32_t ReqSize,
                                     const char *const Description);
uint32_t    MEMPOOL_BlockSize       (void *const Block);
uint32_t    MEMPOOL_Available       (struct MEMPOOL *const M);
