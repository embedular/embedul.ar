/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [CORE] bitfield manager.

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

#include "embedul.ar/source/core/bitfield.h"
#include "embedul.ar/source/core/device/board.h"


/**
 * The maximum number of bits in a bit range.
 */
#define BITFIELD_RANGE_MAX_COUNT    32


/**
 * Initialize a :c:struct:`BITFIELD` instance.
 *
 * :param Buffer: An already allocated :c:type:`uint32_t` buffer.
 * :param Capacity: Buffer capacity, in :c:type:`uint32_t` elements.
 * :param Range: Pointer to an array of :c:struct:`BITFIELD_RANGE` defining
 *               bit ranges. The :c:struct:`BITFIELD` instance will be
 *               initialized in bit range mode. This function asserts
 *               valid ranges. To initialize the :c:struct:`BITFIELD`
 *               instance as single bit addressing mode only,
 *               set this parameter to :c:macro:`NULL`.
 * :param RangeCount: Number of elements in the array of
 *                    :c:struct:`BITFIELD_RANGE` passed as the ``range``
 *                    parameter. if ``range`` is set to :c:macro:`NULL`, then
 *                    this parameter must be zero.
 */
void BITFIELD_Init (struct BITFIELD *const B, uint32_t *const Buffer,
                    const uint16_t Capacity,
                    const struct BITFIELD_RANGE *const Range,
                    const uint16_t RangeCount)
{
    BOARD_AssertParams (B && Buffer && Capacity &&
                         ((!Range && !RangeCount) || (Range && RangeCount)));

    // Sanity check for bitfield range elements.
    for (uint32_t i = 0; i < RangeCount; ++i)
    {
        // bitIndex specified lies inside of the uint32_t buffer capacity.
        BOARD_AssertParams ((Range[i].bitIndex >> 5) < Capacity);
        // Count is a valid number of bits for a single uint32_t element.
        BOARD_AssertParams (Range[i].count <= BITFIELD_RANGE_MAX_COUNT);
        // Bit range do not span across uint32_t buffer elements.
        BOARD_AssertParams ((Range[i].bitIndex & 0x1F) + Range[i].count
                                <= BITFIELD_RANGE_MAX_COUNT);
    }

    OBJECT_Clear (B);

    B->data         = Buffer;
    B->capacity     = Capacity;
    B->range        = Range;
    B->rangeCount   = RangeCount;
}


/**
 * Set a single bit state by its ``BitIndex``.
 *
 * :param BitIndex: The bit position, from the least significant bit starting at
 *                  zero. A bit number larger than a :c:type:`uint32_t` (for
 *                  example, 32, 74, etc) will fit on the appropiate element
 *                  index in the :c:type:`uint32_t` buffer. This function
 *                  asserts the condition of sufficient buffer elements for a
 *                  given ``BitIndex``.
 * :param State: New bit state, either :c:macro:`true` or :c:macro:`false`.
 */
void BITFIELD_SetBit (struct BITFIELD *const B, const BITFIELD_Index BitIndex,
                      const bool State)
{
    BOARD_AssertParams (B && B->data && (BitIndex >> 5) < B->capacity);

    const uint32_t U32Index         = BitIndex >> 5;
    const uint16_t BitIndexMod32    = BitIndex & 0x1F;

    if (State == true)
    {
        B->data[U32Index] |= 1 << BitIndexMod32;
    }
    else
    {
        B->data[U32Index] &= ~(1 << BitIndexMod32);
    }
}


inline static uint32_t getBit (struct BITFIELD *const B,
                               const BITFIELD_Index BitIndex)
{
    return (B->data[BitIndex >> 5] & (1 << (BitIndex & 0x1F)));
}


/**
 * Get a single bit state by its ``BitIndex``.
 *
 * :param BitIndex: The bit position, from the least significant bit starting at
 *                  zero. A bit number larger than a :c:type:`uint32_t` (for
 *                  example, 32, 74, etc) will fit on the appropiate element
 *                  index in the :c:type:`uint32_t` buffer. This function
 *                  asserts the condition of sufficient buffer elements for a
 *                  given ``BitIndex``.
 * :return: Current bit state as positioned in the corresponding
 *          :c:type:`uint32_t` buffer element.
 */
uint32_t BITFIELD_GetBit (struct BITFIELD *const B,
                          const BITFIELD_Index BitIndex)
{
    BOARD_AssertParams (B && B->data && (BitIndex >> 5) < B->capacity);
    return getBit (B, BitIndex);
}


inline static uint32_t getRangeMask (struct BITFIELD *const B,
                                     const BITFIELD_Index RangeIndex)
{
    const struct BITFIELD_RANGE * BR = &B->range[RangeIndex];
    const uint32_t Mask = (0xFFFFFFFF >> (32 - BR->count)) << (BR->bitIndex
                                                                    & 0x1F);
    return Mask;
}


static void setRangeValue (struct BITFIELD *const B,
                           const BITFIELD_Index RangeIndex,
                           const uint32_t Value)
{
    const uint32_t Mask             = getRangeMask (B, RangeIndex);
    const uint16_t BitIndex         = B->range[RangeIndex].bitIndex;
    const uint16_t U32Index         = BitIndex >> 5;
    const uint16_t BitIndexMod32    = BitIndex & 0x1F;

    // Value should not be higher than the maximum number allowed by the
    // bit count in the range.
    BOARD_AssertParams (Value <= (Mask >> BitIndexMod32));

    B->data[U32Index] &= ~Mask;
    B->data[U32Index] |= (Value << BitIndexMod32) & Mask;
}


inline static uint32_t getRangeValue (struct BITFIELD *const B,
                                      const BITFIELD_Index RangeIndex)
{
    const uint32_t Mask             = getRangeMask (B, RangeIndex);
    const uint16_t BitIndex         = B->range[RangeIndex].bitIndex;
    const uint16_t U32Index         = BitIndex >> 5;
    const uint16_t BitIndexMod32    = BitIndex & 0x1F;

    return ((B->data[U32Index] & Mask) >> BitIndexMod32);
}


/**
 * Set the bit range value of a given ``RangeIndex``. Do not use this
 * function on :c:struct:`BITFIELD` instances initialized as **Single bit**
 * mode; this condition is asserted.
 *
 * :param RangeIndex: Index in the array of :c:struct:`BITFIELD_RANGE`
 *                  with the bit range value to update.
 * :param Value: New value to set in the specified bit range. Should not be
 *               higher than the maximum number allowed by the bit count in
 *               the range; this condition is asserted.
 */
void BITFIELD_SetRangeValue (struct BITFIELD *const B,
                             const BITFIELD_Index RangeIndex,
                             const uint32_t Value)
{
    BOARD_AssertParams (B && B->range && RangeIndex < B->rangeCount);
    setRangeValue (B, RangeIndex, Value);
}


/**
 * Get the bit range value of a given ``RangeIndex``. Do not use this
 * function on :c:struct:`BITFIELD` instances initialized as **Single bit**
 * mode; this condition is asserted.
 *
 * :param RangeIndex: Index in the array of :c:struct:`BITFIELD_RANGE`
 *                  with the bit range value to retrieve.
 * :return: Current value of the specified bit range.
 */
uint32_t BITFIELD_GetRangeValue (struct BITFIELD *const B,
                                 const BITFIELD_Index RangeIndex)
{
    BOARD_AssertParams (B && B->range && RangeIndex < B->rangeCount);
    return getRangeValue (B, RangeIndex);
}
