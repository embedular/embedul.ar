/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [CORE] octet array data structure.

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

#include "embedul.ar/source/core/array.h"
#include "embedul.ar/source/core/device/board.h"
#include <string.h>


/**
 * Array initialization.
 *
 * :param Buffer: Already allocated buffer.
 * :param Capacity: Buffer capacity, in :c:type:`uint8_t` elements.
 */
void ARRAY_Init (struct ARRAY *const A, uint8_t *const Buffer,
                 const uint32_t Capacity)
{
    BOARD_AssertParams (A && Buffer && Capacity);

    OBJECT_Clear (A);

    A->data     = Buffer;
    A->capacity = Capacity;
}


/**
 * Array reset. Internal index pointer is set to zero.
 *
 */
void ARRAY_Reset (struct ARRAY *const A)
{
    BOARD_AssertParams (A);
    A->index = 0;
}


/**
 * Gets the element value at the given index. The caller must check for
 * inserted elements (:c:func:`ARRAY_Count`) before specifying an invalid
 * index; the function asserts this condition.
 *
 * :param Index: Element index.
 * :return: Element value.
 */
uint8_t ARRAY_Element (struct ARRAY *const A, const uint32_t Index)
{
    BOARD_AssertParams (A && Index < A->index);
    return A->data[Index];
}


/**
 * Returns a read-only buffer pointer to the element at the given index. The
 * caller must check for inserted elements count (:c:func:`ARRAY_Count`)
 * before requesting an invalid index; the function asserts this condition.
 *
 * :param Index: Element index.
 * :return: Read-only buffer pointer.
 */
const uint8_t * ARRAY_Data (struct ARRAY *const A, const uint32_t Index)
{
    BOARD_AssertParams (A && Index < A->index);
    return &A->data[Index];
}


/**
 * Returns the number of inserted elements.
 *
 * :return: Element count.
 */
uint32_t ARRAY_Count (struct ARRAY *const A)
{
    BOARD_AssertParams (A);
    return A->index;
}


static inline uint32_t capacityLeft (struct ARRAY *const A)
{
    return (A->capacity - A->index);
}


static inline bool appendOne (struct ARRAY *const A, const uint8_t Value)
{
    if (!capacityLeft (A))
    {
        return false;
    }

    A->data[A->index ++] = Value;
    return true;
}


/**
 * Returns available space for new elements.
 *
 * :return: Capacity left. Zero if full.
 */
uint32_t ARRAY_Left (struct ARRAY *const A)
{
    BOARD_AssertParams (A);
    return capacityLeft (A);
}


/**
 * Appends one :c:type:`uint8_t` element.
 *
 * :param Value: The new element to append.
 * :return: :c:macro:`true` if the element was successfully appended.
 *          :c:macro:`false` if the array has no capacity left.
 */
bool ARRAY_AppendOne (struct ARRAY *const A, const uint8_t Value)
{
    BOARD_AssertParams (A);
    return appendOne (A, Value);
}


/**
 * Appends a null-terminated string.
 *
 * :param Str: Null-terminated string.
 * :param MaxLen: Maximum number of octets before finding a null termination in
 *                ``Str``.
 * :return: :c:macro:`true` if successfully appended,
 *          :c:macro:`false` if there is no null before MaxLen octets or no
 *          capacity left to insert the whole string.
 */
bool ARRAY_AppendString (struct ARRAY *const A, const char *const Str,
                         const size_t MaxLen)
{
    BOARD_AssertParams (A && Str);

    const size_t Len = strnlen (Str, MaxLen);
    if (Len == MaxLen)
    {
        return false;
    }

    const char *p = Str;

    while (*p)
    {
        if (!appendOne (A, (uint8_t)*p ++))
        {
            return false;
        }
    }

    return true;
}


/**
 * Append binary data.
 *
 * :param Data: Binary data.
 * :param Size: Number of octets to append.
 * :return: :c:macro:`true` if successfully appended,
 *          :c:macro:`false` if there is no capacity left to insert the data.
 */
bool ARRAY_AppendBinary (struct ARRAY *const A, const uint8_t *const Data,
                         const uint32_t Size)
{
    BOARD_AssertParams (A && Data && Size);

    for (uint32_t i = 0; i < Size; ++i)
    {
        if (!appendOne (A, Data[i]))
        {
            return false;
        }
    }

    return true;
}


/**
 * Removes elements *from the back* of the Array. Caller must check that
 * the array has at least one element; this condition is asserted.
 *
 * :return: Last element value.
 */
uint8_t ARRAY_RemoveLast (struct ARRAY *const A)
{
    BOARD_AssertParams (A && A->index);

    const uint8_t LastElement = A->data[-- A->index];
    return LastElement;
}


/**
 * Checks for content equality between two Arrays.
 *
 * :return: :c:macro:`true` if contents match, :c:macro:`false` otherwise.
 */
bool ARRAY_IsEqual (struct ARRAY *const A, struct ARRAY *const B)
{
    BOARD_AssertParams (A && B);

    // Can't be equal without same number of elements
    if (A->index != B->index)
    {
        return false;
    }

    // It is good practice for every array to manage its own buffer,
    // but that is not enforced.
    if (A->data == B->data)
    {
        return true;
    }

    return !(memcmp(A->data, B->data, A->index))? true : false;
}


/**
 * Copy contents from one Array to the other without touching
 * buffer pointers or sizes. The destination capacity must suffice; this 
 * condition is asserted.
 *
 * :param A: Source.
 * :param B: Destination.
 */
void ARRAY_Copy (struct ARRAY *const A, struct ARRAY *const B)
{
    BOARD_AssertParams (A && B && B->capacity && A->index <= B->capacity);

    memcpy (B->data, A->data, A->index);
    B->index = A->index;
}
