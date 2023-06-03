/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [CORE] cyclic/ring buffer optimized for 2^n lenghts.

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

#include "embedul.ar/source/core/cyclic.h"
#include "embedul.ar/source/core/device/board.h"


/**
 * Initializes a :c:struct:`CYCLIC` instance.
 *
 * :param Buffer: An already allocated :c:type:`uint8_t` buffer.
 * :param Capacity: Buffer capacity, in :c:type:`uint8_t` elements.
 *
 * .. warning:: Buffer capacity must be a power of two. This condition is asserted.
 */
void CYCLIC_Init (struct CYCLIC *const C, uint8_t *const Buffer,
                  const uint32_t Capacity)
{
    BOARD_AssertParams (C && Buffer && Capacity);

    // Capacity must be a power of two.
    BOARD_AssertParams (!(Capacity & (Capacity - 1)));

    OBJECT_Clear (C);

    C->data     = Buffer;
    C->capacity = Capacity;
}


/**
 * Resets the state of a :c:struct:`CYCLIC` instance, keeping the
 * :c:type:`uint8_t` buffer pointer, its declared capacity and usage
 * statistics.
 */
void CYCLIC_Reset (struct CYCLIC *const C)
{
    BOARD_AssertParams (C && C->data && C->capacity);
    
    memset (C->data, 0, C->capacity);
    
    C->inIndex  = 0;
    C->outIndex = 0;
    C->elements = 0;
    C->lastIn   = 0;
    C->lastOut  = 0;
}


/**
 * Discard elements inserted and not yet consumed.
 */
void CYCLIC_Discard (struct CYCLIC *const C)
{
    BOARD_AssertParams (C);

    const uint32_t Elements = CYCLIC_Elements (C);
    C->outIndex = (C->outIndex + Elements) & (C->capacity - 1);
    C->elements = 0;
    C->discards += Elements;
}


/**
 * Returns buffer capacity passed when initializing this :c:struct:`CYCLIC`
 * instance.
 *
 * :return: Number of :c:type:`uint8_t` elements,
 *          as declared at initialization.
 */
uint32_t CYCLIC_Capacity (struct CYCLIC *const C)
{
    BOARD_AssertParams (C);
    return C->capacity;
}


/**
 * Return elements inserted and not yet consumed.
 *
 * :return: Number of :c:type:`uint8_t` elements available to
 *          consume.
 */
uint32_t CYCLIC_Elements (struct CYCLIC *const C)
{
    BOARD_AssertParams (C);
    return C->elements;
}


/**
 * Returns space available to insert elements.
 *
 * :return: Number of :c:type:`uint8_t` elements available.
 */
uint32_t CYCLIC_Available (struct CYCLIC *const C)
{
    BOARD_AssertParams (C);
    return (C->capacity - C->elements);
}


/**
 * **[Data IN]** Inserts a single :c:type:`uint8_t` element.
 *
 * :param Octet: Element value to insert.
 */
void CYCLIC_IN_FromOctet (struct CYCLIC *const C, const uint8_t Octet)
{
    CYCLIC_IN_FromBuffer (C, &Octet, 1);
}


static void inFrom_End (struct CYCLIC *const C, const uint32_t Count)
{
    C->writes += Count;
    C->lastIn  = Count;

    const uint32_t Left = CYCLIC_Available (C);
    if (Count > Left)
    {
        C->overflows += Count - Left;
        C->elements = C->capacity;
    }
    else
    {
        C->elements += Count;
    }
}


/**
 * **[Data IN]** Inserts an array of :c:type:`uint8_t` elements.
 *
 * :param Data: Array of :c:type:`uint8_t` elements.
 * :param Count: Number of :c:type:`uint8_t` elements to insert.
 */
void CYCLIC_IN_FromBuffer (struct CYCLIC *const C, const uint8_t *const Data,
                           const uint32_t Count)
{
    BOARD_AssertParams (C && Data && Count);

    // TODO: Optimization: identify the two data segments and do a direct
    //       memcpy to them.
    {
        const uint8_t * p = Data;
        for (uint32_t i = Count; i; --i)
        {
            C->data[C->inIndex] = *p ++;
            C->inIndex = (C->inIndex + 1) & (C->capacity - 1);
        }
    }

    inFrom_End (C, Count);
}


/**
 * **[Data IN]** Inserts a null-terminated string.
 *
 * :param str: Pointer to a null-terminated string.
 * :param MaxLen: The null termination in ``str`` must be found before
 *                processing this maximum amount of ``char`` elements.
 *                This condition is asserted.
 */
void CYCLIC_IN_FromString (struct CYCLIC *const C, const char *const Str,
                           const size_t MaxLen)
{
    BOARD_AssertParams (C && Str);

    const size_t Size = strnlen (Str, MaxLen);

    BOARD_AssertParams (Size != MaxLen);

    CYCLIC_IN_FromBuffer (C, (const uint8_t *)Str, (uint32_t)Size);
}


static void cyclicOutProc (void *const Param, const uint8_t *const Buffer, 
                           const uint32_t Count)
{
    struct CYCLIC *const C = (struct CYCLIC *) Param;
    // lastIn reflects the whole operation, not a partial data insertion
    const uint32_t LastIn = C->lastIn;
    CYCLIC_IN_FromBuffer (Param, Buffer, Count);
    C->lastIn += LastIn;
}


/**
 * **[Data IN]** Inserts a null-terminated string with :c:struct:`VARIANT`
 * argument substitution.
 *
 * :param Str: Pointer to a null-terminated string with indexed argument
 *             placeholders.
 * :param MaxOctets: The null termination in ``Str`` must be found before
 *                   reaching ``MaxOctets``; this condition is asserted.
 * :param ArgValues: Array of :c:struct:`VARIANT` elements whose values will
 *                   replace the argument placeholders in ``Str``.
 * :param ArgCount: Number of indexed elements in the :c:struct:`VARIANT` array.
 */
void CYCLIC_IN_FromParsedStringArgs (struct CYCLIC *const C,
                                     const size_t MaxOctets, 
                                     const char *const Str, 
                                     struct VARIANT *const ArgValues,
                                     const uint32_t ArgCount)
{
    VARIANT_ParseStringArgs (0, MaxOctets, Str, cyclicOutProc, C, 
                             ArgValues, ArgCount);
}


/**
 * **[Data IN]** Reads a :c:struct:`STREAM` and insert each :c:type:`uint8_t`
 * element until the :c:struct:`STREAM` signals an end of file (EOF).
 *
 * :param S: :c:struct:`STREAM` to read and insert elements until EOF.
 */
void CYCLIC_IN_FromStream (struct CYCLIC *const C, struct STREAM *const S)
{
    BOARD_AssertParams (C && S);

    uint32_t count = 0;

    {
        uint8_t data;
        while ((void)(data = STREAM_OUT_ToOctet(S)), STREAM_Count(S))
        {
            C->data[C->inIndex] = data;
            C->inIndex = (C->inIndex + 1) & (C->capacity - 1);
            ++ count;
        }
    }

    inFrom_End (C, count);
}


/**
 * **[Data OUT]** Consumes an octet and returns its value. The caller must have
 * confirmed with :c:func:`CYCLIC_Elements` that there is at least one octet
 * to consume; this condition is asserted.
 *
 * :return: The octet value.
 */
uint8_t CYCLIC_OUT_ToOctet (struct CYCLIC *const C)
{
    uint8_t octet = 0x00;
    CYCLIC_OUT_ToBuffer (C, &octet, 1);
    return octet;
}


/**
 * **[Data OUT]** Consumes a given amount of octets by storing them in an
 * :c:type:`uint8_t` array already allocated by the user. The user must have
 * confirmed with :c:func:`CYCLIC_Elements` that the required amount of
 * elements is available; this condition is asserted.
 *
 * :param Data: Already allocated :c:type:`uint8_t` array.
 * :param Count: Number of octets to consume by storing them in the array.
 */
void CYCLIC_OUT_ToBuffer (struct CYCLIC *const C, uint8_t *const Data,
                          const uint32_t Count)
{
    BOARD_AssertParams (C && Data);
    const uint32_t Elements = CYCLIC_Elements (C);
    BOARD_AssertParams (Count <= Elements);

    C->reads    += Count;
    C->elements -= Count;

    {
        uint8_t *p = Data;
        for (C->lastOut = 0; C->lastOut < Count; ++ C->lastOut)
        {
            *p ++ = C->data[C->outIndex];
            C->outIndex = (C->outIndex + 1) & (C->capacity - 1);
        }
    }
}


/**
 * **[Data OUT]** Tries to consume all elements transferring them to a
 * :c:struct:`STREAM`. The user must have confirmed that there is at least one
 * element to consume; this condition is asserted. After the call, the user
 * must not assume all elements were effectively consumed. The operation may
 * stop prematurely if the stream is unable to receive more items. Depending on
 * the actual :c:struct:`STREAM` implementation, the user may need to wait
 * some time and repeatedly call this function until all elements are consumed.
 * See :c:func:`CYCLIC_Elements` and :c:func:`STREAM_IN_Count`.
 *
 * Below is an usage example:
 *
 * .. literalinclude:: code-samples/cyclic-out-tostream.c
 *    :language: c
 *
 * :param S: :c:struct:`STREAM` instance to transfer consumed elements.
 *           The STREAM may abort the operation at any time.
 */
void CYCLIC_OUT_ToStream (struct CYCLIC *const C, struct STREAM *const S)
{
    BOARD_AssertParams (C && S);

    C->lastOut = 0;

    {
        uint32_t elements = CYCLIC_Elements (C);
        BOARD_AssertState (elements);

        while (elements --)
        {
            STREAM_IN_FromOctet (S, C->data[C->outIndex]);
            if (!STREAM_Count (S))
            {
                break;
            }

            C->outIndex = (C->outIndex + 1) & (C->capacity - 1);
            ++ C->lastOut;
        }
    }

    C->reads    += C->lastOut;
    C->elements -= C->lastOut;
}


/**
 * Peeks an inserted element before consuming it. The user must check that
 * there are enough inserted elements for the requested ``Index``; otherwise,
 * the return value will be invalid.
 *
 * :param Index: A Zero-based ``Index`` selects which element to peek;
 *               zero is the first element consumed.
 * :return: A copy of the element.
 */
uint8_t CYCLIC_Peek (struct CYCLIC *const C, const uint32_t Index)
{
    BOARD_AssertParams (C);

    ++ C->peeks;
    return C->data[(C->outIndex + Index) & (C->capacity - 1)];
}


/**
 * Peeks inserted elements before consuming them by copying their values to an
 * already allocated array of :c:type:`uint8_t`. The user must check that
 * there are enough inserted elements for the requested ``Index`` plus
 * ``Count``; otherwise, the return value will be invalid.
 *
 * :param Index: A Zero-based ``Index`` selects from which element it starts to
 *               peek; zero is the first element consumed.
 * :param Data: :c:type:`uint8_t` array to copy element values.
 * :param Count: Number of elements to peek, starting from ``Index``.
 */
void CYCLIC_PeekToBuffer (struct CYCLIC *const C, const uint32_t Index,
                          uint8_t *const Data, const uint32_t Count)
{
    BOARD_AssertParams (C && Data && Count);
    
    {
        uint8_t *p = Data;
        for (uint32_t i = 0; i < Count; ++i)
        {
            *p ++ = C->data[(C->outIndex + Index + i) & (C->capacity - 1)];
        }
    }
    
    C->peeks += Count;
}


/**
 * Peek inserted elements before consuming them by inserting a copy of their
 * values to another c:struct:`CYCLIC` instance. The user must check that
 * there are enough inserted elements for the requested ``Index`` plus
 * ``Count``; otherwise, the return value will be invalid.
 *
 * :param Index: A Zero-based ``Index`` selects from which element it starts to
 *               peek; zero is the first element consumed.
 * :param Count: Number of elements to peek, starting from ``Index``.
 * :param D: c:struct:`CYCLIC` instance to store a copy of the elements.
 */
void CYCLIC_PeekToCyclic (struct CYCLIC *const C, const uint32_t Index,
                          const uint32_t Count, struct CYCLIC *const D)
{
    BOARD_AssertParams (C && D);

    for (uint32_t i = 0; i < Count; ++i)
    {
        CYCLIC_IN_FromOctet (D,
                    C->data[(C->outIndex + Index + i) & (C->capacity - 1)]);
    }

    C->peeks += Count;
}


/**
 * Overwrites an inserted element. The user must check that
 * there are enough inserted elements for the requested ``Index``; this
 * condition is asserted.
 *
 * :param Index: A Zero-based ``Index`` selects which element to overwrite;
 *               zero is the first element consumed.
 * :param Octet: Overwrite the inserted element with this value.
 */
void CYCLIC_Overwrite (struct CYCLIC *const C, const uint32_t Index,
                       const uint8_t Octet)
{
    BOARD_AssertParams (C);
    BOARD_AssertParams (Index < C->elements);

    C->data[(C->outIndex + Index) & (C->capacity - 1)] = Octet;
    ++ C->writes;
}


/**
 * Overwrites inserted elements by replacing their values with the ones in an
 * array of :c:type:`uint8_t`. The user must check that there are enough
 * inserted elements for the requested ``Index`` plus ``Count``; this
 * condition is asserted.
 *
 * :param Index: A Zero-based ``Index`` selects from which element it starts to
 *               overwrite; zero is the first element consumed.
 * :param Data: Overwrite the inserted elements with the ones from this
 *              :c:type:`uint8_t` array.
 * :param Count: Number of elements to overwrite starting from ``Index``.
 */
void CYCLIC_OverwriteFromBuffer (struct CYCLIC *const C, const uint32_t Index,
                                 const uint8_t *const Data, 
                                 const uint32_t Count)
{
    BOARD_AssertParams (C && Data && Count);
    BOARD_AssertParams (Index + Count <= C->elements);

    {
        const uint8_t *p = Data;
        for (uint32_t i = 0; i < Count; ++i)
        {
            C->data[(C->outIndex + Index + i) & (C->capacity - 1)] = *p ++;
        }
    }

    C->writes += Count;
}


/**
 * Replaces an inserted element that is equal to a given value. The user must
 * check that there are enough inserted elements for the requested ``Index``
 * plus ``Count``; this condition is asserted.
 *
 * :param Index: A Zero-based ``Index`` selects from which element it starts to
 *               check; zero is the first element consumed.
 * :param Count: Number of elements to check, starting from ``Index``.
 * :param Match: Value to check for equality.
 * :param Replace: If equal, replace with this value.
 */
void CYCLIC_Replace (struct CYCLIC *const C, const uint32_t Index,
                     const uint32_t Count, const uint8_t Match,
                     const uint8_t Replace)
{
    BOARD_AssertParams (C && Count);
    BOARD_AssertParams (Index + Count <= C->elements);

    for (uint32_t i = 0; i < Count; ++i)
    {
        uint8_t *p = &C->data[(C->outIndex + Index + i) & (C->capacity - 1)];
        if (*p == Match)
        {
            *p = Replace;
        }
    }
}


/**
 * Replaces all inserted elements that are equal to a given value. The user must
 * check that there is at least one element; this condition is asserted.
 *
 * :param Match: Value to check for equality.
 * :param Replace: If equal, replace with this value.
 */
void CYCLIC_ReplaceAll (struct CYCLIC *const C, const uint8_t Match,
                        const uint8_t Replace)
{
    CYCLIC_Replace (C, 0, CYCLIC_Elements(C), Match, Replace);
}


/**
 * Returns number of insertions from the last **[Data IN]** operation.
 *
 * :return: Number of insertions.
 */
uint32_t CYCLIC_IN_Count (struct CYCLIC *const C)
{
    BOARD_AssertParams (C);
    return C->lastIn;
}


/**
 * Returns number of consumptions from the last **[Data OUT]** operation.
 *
 * :return: Number of consumptions.
 */
uint32_t CYCLIC_OUT_Count (struct CYCLIC *const C)
{
    BOARD_AssertParams (C);
    return C->lastOut;
}
