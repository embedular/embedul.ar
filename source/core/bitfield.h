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

#pragma once

#include <stdint.h>
#include <stdbool.h>


/**
 * Description
 * ===========
 *
 * Manages accesses and writes to a bitfield stored in an already allocated
 * buffer of :c:type:`uint32_t` elements. A :c:struct:`BITFIELD` instance should
 * manage its unique buffer. Each :c:type:`uint32_t` in the buffer will give 32
 * unique bits. Bits are indexed from the least significant bit starting at
 * zero. For example, the first :c:type:`uint32_t` in the buffer will hold bits
 * 0 to 31, the second :c:type:`uint32_t` will hold bits 32 to 63 and so on.
 *
 * There are two modes of operation with their own set of functions:
 *
 * Single bit
 *   Individual bits are set and get by their ``BitIndex``.
 *
 * Bit range (multibit values)
 *   An array of :c:struct:`BITFIELD_RANGE` is passed to
 *   :c:func:`BITFIELD_Init`. The :c:type:`uint32_t` buffer will hold
 *   bit ranges; bits grouped as specified by elements on a
 *   :c:struct:`BITFIELD_RANGE` array. Each range element is indexed on that
 *   array by a ``RangeIndex``.
 *
 * .. note:: It is OK to define a :c:struct:`BITFIELD_RANGE` array with ranges
 *           starting at some arbitrary bitIndex, then managing the bits below
 *           as single bits. An actual framework use case stores a gamepad's
 *           state with several digital switches as single bits and additional
 *           analog inputs like sticks and accelerometers as bit ranges
 *           according to the required resolution.
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
 * Data type that stores a ``BitIndex`` or a ``RangeIndex``. On a
 * ``BitIndex``, it allows a maximum of 65536 bit indices stored in
 * 65536/32 = 2048 :c:type:`uint32_t` buffer elements. On a ``RangeIndex``, it
 * allows a maximum indexing of 65536 ranges.
 */
typedef uint16_t    BITFIELD_Index;


/**
 * Maximum value represented in a :c:type:`BITFIELD_Index`.
 */
#define BITFIELD_INDEX_MAX          ((BITFIELD_Index) -1)


/**
 * Number of :c:type:`uint32_t` elements required to store ``x`` bits.
 */
#define BITFIELD_COUNT(x)           ((x >> 5) + 1)


/**
 * Defines a single bit range by giving a first bit index and
 * the number of additional bits in the range. For example,
 * :c:struct:`BITFIELD_RANGE.bitIndex` = 3 and
 * :c:struct:`BITFIELD_RANGE.count` = 4 will define a range of four bits from
 * bit 3 to bit 6 as follows:
 *
 * .. image:: images/bitfield_range_3to6.drawio.svg
 *
 * .. note:: Valid bit ranges do not span across :c:type:`uint32_t` buffer
 *           elements. For example, :c:struct:`BITFIELD_RANGE.bitIndex` = 30
 *           and :c:struct:`BITFIELD_RANGE.count` = 3 is an invalid bit range
 *           since bit 32 lies on the next buffer element.
 */
struct BITFIELD_RANGE
{
    /** bit index of the least significant bit in the range. */
    BITFIELD_Index  bitIndex;
    /** Number of additional bits in the range.
     *
     * .. note:: The maximum number of bits in a bit range is 32.
     *           The :c:type:`uint16_t` data type is used for struct padding.
     */
    uint16_t        count;
};



/**
 * The user should treat this as an opaque structure. No member should be
 * directly accessed or modified.
 */
struct BITFIELD
{
    uint32_t        * data;
    const struct
    BITFIELD_RANGE  * range;
    uint16_t        capacity;
    uint16_t        rangeCount;
};


void        BITFIELD_Init               (struct BITFIELD *const B,
                                         uint32_t *const Buffer,
                                         const uint16_t Capacity,
                                         const struct BITFIELD_RANGE *range,
                                         const uint16_t RangeCount);
void        BITFIELD_Reset              (struct BITFIELD *const B);
void        BITFIELD_SetBit             (struct BITFIELD *const B,
                                         const BITFIELD_Index BitIndex,
                                         const bool State);
uint32_t    BITFIELD_GetBit             (struct BITFIELD *const B,
                                         const BITFIELD_Index BitIndex);
void        BITFIELD_SetRangeValue      (struct BITFIELD *const B,
                                         const BITFIELD_Index RangeIndex,
                                         const uint32_t Value);
uint32_t    BITFIELD_GetRangeValue      (struct BITFIELD *const B,
                                         const BITFIELD_Index RangeIndex);