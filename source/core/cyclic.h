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

#pragma once

#include "embedul.ar/source/core/variant.h"


/**
 * Description
 * ===========
 *
 * Manages an already allocated buffer of :c:type:`uint8_t` elements to
 * perform a cyclic (also called circular or ring) buffer access to memory
 * as if both buffer ends were connected. Please see the `Wikipedia article on
 * circular buffers`_ for detailed information about this data structure.
 *
 * The typical use case is buffering of data streams where a consumer reads
 * data at a different pace than the producer.
 * Elements are inserted to (Data IN) or extracted from (Data OUT) the
 * :c:struct:`CYCLIC` instance.
 *
 * Input sources (producers) are single octets, buffers,
 * null-terminated strings, string with variable arguments
 * (see :c:struct:`VARIANT`) and octets provided by streams
 * (see :c:struct:`STREAM`). Output destinations (consumers) are single octets,
 * buffers or streams. Produced input data can be discarded, peeked,
 * overwritten or replaced before consumed. The figure below depicts producers,
 * consumers and operations on buffered data.
 *
 * .. image:: images/cyclic.drawio.svg
 *
 * .. note:: Inserted data may overflow the available buffer capacity,
 *           overwriting older values not yet consumed; this is especially
 *           true when reading from a :c:struct:`STREAM`. That's not a bug,
 *           but a feature™. The user must employ a buffer with an appropriate
 *           size for the intended application to avoid this data structure
 *           feature.
 *
 * There are three :c:struct:`CYCLIC` buffer usage metrics:
 *
 * :Capacity: Total amount of :c:type:`uint8_t` buffer elements.
 *            See :c:func:`CYCLIC_Capacity`.
 *
 * :Elements: Number of inserted elements waiting to be consumed.
 *            See :c:func:`CYCLIC_Elements`.
 *
 * :Available: Free space to insert further elements.
 *
 * On a given time, the actual :c:type:`uint8_t` linear memory buffer usage may
 * look as in the following figure:
 *
 * .. image:: images/cyclic_buffer.drawio.svg
 *
 * Current IN and OUT buffer pointers are managed by the :c:struct:`CYCLIC`
 * instance and are opaque to the user.
 *
 *
 * .. _`Wikipedia article on circular buffers`:
 *    https://en.wikipedia.org/wiki/Circular_buffer
 *
 *
 * API guide
 * =========
 *
 * Initialization-related functions
 * --------------------------------
 *
 * | :c:func:`CYCLIC_Init`
 * | :c:func:`CYCLIC_Reset`
 * | :c:func:`CYCLIC_Capacity`
 *
 * Status functions
 * ----------------
 *
 * | :c:func:`CYCLIC_Elements`
 * | :c:func:`CYCLIC_Available`
 * | :c:func:`CYCLIC_IN_Count`
 * | :c:func:`CYCLIC_OUT_Count`
 *
 * Data IN
 * -------
 *
 * | :c:func:`CYCLIC_IN_FromOctet`
 * | :c:func:`CYCLIC_IN_FromBuffer`
 * | :c:func:`CYCLIC_IN_FromString`
 * | :c:func:`CYCLIC_IN_FromParsedStringArgs`
 * | :c:func:`CYCLIC_IN_FromStream`
 *
 * Data OUT
 * --------
 *
 * | :c:func:`CYCLIC_OUT_ToOctet`
 * | :c:func:`CYCLIC_OUT_ToBuffer`
 * | :c:func:`CYCLIC_OUT_ToStream`
 *
 * Peek, overwrite and replace contents
 * ------------------------------------
 *
 * | :c:func:`CYCLIC_Peek`
 * | :c:func:`CYCLIC_PeekToBuffer`
 * | :c:func:`CYCLIC_PeekToCyclic`
 * | :c:func:`CYCLIC_Overwrite`
 * | :c:func:`CYCLIC_OverwriteFromBuffer`
 * | :c:func:`CYCLIC_Replace`
 * | :c:func:`CYCLIC_ReplaceAll`
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
 * Automatically instances the parameters list and parameter count required to
 * call :c:func:`CYCLIC_IN_FromParsedStringArgs`.
 *
 * :param _c: The :c:struct:`CYCLIC` instance.
 * :param _u: ``MaxOctets``, see :c:func:`CYCLIC_IN_FromParsedStringArgs`.
 * :param _s: ``Str``, see :c:func:`CYCLIC_IN_FromParsedStringArgs`.
 * :param ...: Up to 16 parameters consisting of generic supported types.
 */
#define CYCLIC_IN_FromParsedString(_c,_m,_s,...) \
    CYCLIC_IN_FromParsedStringArgs (_c,_m,_s, \
                                VARIANT_AutoParams(__VA_ARGS__))


struct STREAM;


/**
 * The user should treat this as an opaque structure. No member should be
 * directly accessed or modified.
 */
struct CYCLIC
{
    uint8_t     *data;
    uint32_t    capacity;
    uint32_t    elements;
    // in: producer / data input
    uint32_t    inIndex;
    // out: consumer / data output
    uint32_t    outIndex;
    //
    uint32_t    lastIn;
    uint32_t    lastOut;
    // Statistics
    uint32_t    reads;
    uint32_t    writes;
    uint32_t    overflows;
    uint32_t    peeks;
    uint32_t    discards;
};


void            CYCLIC_Init                     (struct CYCLIC *const C,
                                                 uint8_t *const Buffer,
                                                 const uint32_t Capacity);
void            CYCLIC_Reset                    (struct CYCLIC *const C);
void            CYCLIC_Discard                  (struct CYCLIC *const C);
uint32_t        CYCLIC_Capacity                 (struct CYCLIC *const C);
uint32_t        CYCLIC_Elements                 (struct CYCLIC *const C);
uint32_t        CYCLIC_Available                (struct CYCLIC *const C);
void            CYCLIC_IN_FromOctet             (struct CYCLIC *const C,
                                                 const uint8_t Octet);
void            CYCLIC_IN_FromBuffer            (struct CYCLIC *const C,
                                                 const uint8_t *const Data,
                                                 const uint32_t Count);
void            CYCLIC_IN_FromString            (struct CYCLIC *const C,
                                                 const char *const Str,
                                                 const size_t MaxLen);
void            CYCLIC_IN_FromParsedStringArgs  (struct CYCLIC *const C,
                                                 const size_t MaxOctets,
                                                 const char *const Str,
                                                 struct VARIANT *const 
                                                 ArgValues,
                                                 const uint32_t ArgCount);
void            CYCLIC_IN_FromStream            (struct CYCLIC *const C,
                                                 struct STREAM *const S);
uint8_t         CYCLIC_OUT_ToOctet              (struct CYCLIC *const C);
void            CYCLIC_OUT_ToBuffer             (struct CYCLIC *const C,
                                                 uint8_t *const Data,
                                                 const uint32_t Count);
void            CYCLIC_OUT_ToStream             (struct CYCLIC *const C,
                                                 struct STREAM *const S);
uint8_t         CYCLIC_Peek                     (struct CYCLIC *const C,
                                                 const uint32_t Index);
void            CYCLIC_PeekToBuffer             (struct CYCLIC *const C,
                                                 const uint32_t Index,
                                                 uint8_t *const Data,
                                                 const uint32_t Count);
void            CYCLIC_PeekToCyclic             (struct CYCLIC *const C,
                                                 const uint32_t Index,
                                                 const uint32_t Count,
                                                 struct CYCLIC *const D);
void            CYCLIC_Overwrite                (struct CYCLIC *const C,
                                                 const uint32_t Index,
                                                 const uint8_t Octet);
void            CYCLIC_OverwriteFromBuffer      (struct CYCLIC *const C,
                                                 const uint32_t Index,
                                                 const uint8_t *const Data,
                                                 const uint32_t Count);
void            CYCLIC_Replace                  (struct CYCLIC *const C,
                                                 const uint32_t Index,
                                                 const uint32_t Count,
                                                 const uint8_t Match,
                                                 const uint8_t Replace);
void            CYCLIC_ReplaceAll               (struct CYCLIC *const C,
                                                 const uint8_t Match,
                                                 const uint8_t Replace);
uint32_t        CYCLIC_IN_Count                 (struct CYCLIC *const C);
uint32_t        CYCLIC_OUT_Count                (struct CYCLIC *const C);
    