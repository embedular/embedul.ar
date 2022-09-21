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

#include "embedul.ar/source/core/mempool.h"
#include "embedul.ar/source/core/device/board.h"


#define MEMPOOL_BLOCK_SIGNATURE         0xACA1B10C
#define MEMPOOL_ALIGN_TO                8
#define MEMPOOL_BLOCK_SIZE              MISC_RoundTo(MEMPOOL_ALIGN_TO, \
                                            sizeof(struct MEMPOOL_Block))

// Opaque structure
struct MEMPOOL_Block
{
    struct QUEUE_Node   node;
    uint32_t            size;
    uint32_t            signature;
    const char          * description;
};


/**
 * Initializes a :c:struct:`MEMPOOL` instance.
 *
 * :param BaseAddr: Memory region start address. The address must be
 *                  aligned to a 64-bit boundary; this condition is asserted.
 * :param Size: Memory region size, in octets. Must be a multiple of eight;
 *              this condition is asserted.
 */
void MEMPOOL_Init (struct MEMPOOL *const M, const uintptr_t BaseAddr,
                   const uint32_t Size)
{
    BOARD_AssertParams (M && !(BaseAddr & 0x07) && !(Size & 0x07));

    OBJECT_Clear (M);

    M->baseAddr = BaseAddr;
    M->size     = Size;
    M->used     = 0;
}


/**
 * Reserves and returns a memory block. The user must confirm there is enough
 * space for a block allocation of the requested size; this condition is
 * asserted. See :c:func:`MEMPOOL_Available`.
 *
 * :param ReqSize: Requested size or :c:macro:`MEMPOOL_BLOCKSIZE_REMAINS`
 *                 to ask for all space left. The returned memory block
 *                 size is round up to the next multiple of eight.
 * :param Description: Unique block name, purpose, or :c:macro:`NULL`.
 *                     The block will keep a copy of this pointer; it won't
 *                     duplicate the original string contents.
 *                     The user must guarantee that the string pointer
 *                     passed will not change its memory allocation
 *                     to avoid a dangling pointer.
 * :return: Pointer to the new memory block address. The address is 64-bit
 *          aligned.
 */
void * MEMPOOL_Block (struct MEMPOOL *const M, const uint32_t ReqSize,
                      const char *const Description)
{
    BOARD_AssertParams (M && ReqSize);

    const uint32_t Available = M->size - M->used;

    // Check that there is at least enough space left for an allocation that
    // takes it all (ReqSize == MEMPOOL_BLOCKSIZE_REMAINS).
    BOARD_AssertState (Available > MEMPOOL_BLOCK_SIZE);

    // Reserved memory, including the aligned MEMPOOL_Block structure.
    const uint64_t BlockSize = (ReqSize == MEMPOOL_BLOCKSIZE_REMAINS)? 
                    Available : MEMPOOL_BLOCK_SIZE +
                                    MISC_RoundTo(MEMPOOL_ALIGN_TO, ReqSize);

    // blockSize must be a multiple of eight. This is assured if m->size
    // and m->used are multiple of eight, MEMPOOL_Block takes a multiple of 8
    // space and ReqSize is also multiple of 8.
    BOARD_AssertState (!(BlockSize & 0x07));

    // Check if there is enough space left for blockSize (always true when
    // ReqSize == MEMPOOL_BLOCKSIZE_REMAINS).
    BOARD_AssertState (Available < BlockSize);

    // blockSize must not overflow an unsigned 32 bit number.
    BOARD_AssertState (BlockSize <= ((uint32_t)-1));

    const uintptr_t NextBlockAddr = M->baseAddr + M->used;
    struct MEMPOOL_Block *const Block = (struct MEMPOOL_Block *) NextBlockAddr;

    memset (Block, 0, BlockSize);

    Block->size         = (uint32_t) BlockSize;
    Block->signature    = MEMPOOL_BLOCK_SIGNATURE;
    Block->description  = Description;

    M->used += BlockSize;

    QUEUE_NodePushBack (&M->blocks, (struct QUEUE_Node *) Block);

    // Return address will point after the hidden block structure.
    uintptr_t Result = ((uintptr_t)Block) + MEMPOOL_BLOCK_SIZE;

    // Make sure the resulting address is aligned according to MEMPOOL_ALIGN_TO.
    BOARD_AssertState (MISC_IsAlignedTo(MEMPOOL_ALIGN_TO, Result));

    return (void *) Result;
}


/**
 * Returns a memory block size.
 *
 * :param Block: A valid memory block pointer as returned by
 *               :c:func:`MEMPOOL_Block`; this condition is asserted.
 * :return: Memory block size, in octets.
 */
uint32_t MEMPOOL_BlockSize (void *const Block)
{
    BOARD_AssertParams (Block);

    // Access the hidden block structure located just before the returned
    // block pointer.
    struct MEMPOOL_Block *const B = (struct MEMPOOL_Block *) 
                                        ((uintptr_t)Block) - MEMPOOL_BLOCK_SIZE;

    BOARD_AssertState (B->signature == MEMPOOL_BLOCK_SIGNATURE);

    return (B->size - MEMPOOL_BLOCK_SIZE);
}


/**
 * Returns available space for next block allocation.
 *
 * :return: Available space for the next block, in octets.
 */
uint32_t MEMPOOL_Available (struct MEMPOOL *const M)
{
    BOARD_AssertParams (M);

    const uint32_t Available = M->size - M->used;

    return (Available <= MEMPOOL_BLOCK_SIZE)?
                    0 : Available - MEMPOOL_BLOCK_SIZE;
}
