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

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


/**
 * Description
 * ===========
 *
 * Manages accesses and assignments on an already allocated buffer of
 * :c:type:`uint8_t` elements, keeping track of its usage. A common use case is
 * buffer management of UTF-8 strings, or operations on binary data, characters
 * and strings over a fixed :c:type:`uint8_t` buffer in general. Each
 * :c:struct:`ARRAY` instance should handle a unique buffer. The Array instance
 * does not own the buffer passed at initialization and will never try to free
 * or reallocate it.
 *
 *
 * Design and development status
 * =============================
 *
 * This is not a full-featured Array implementation. Only the bare
 * minimum functionality required for known use cases has been implemented.
 * String and binary stores are not optimized for speed; these are executed one
 * element at a time by internal calls to :c:func:`ARRAY_AppendOne`.
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
 * The user should treat this as an opaque structure. No member should be
 * directly accessed or modified.
 */
struct ARRAY
{
    uint8_t     * data;
    uint32_t    index;
    uint32_t    capacity;
};


void                ARRAY_Init              (struct ARRAY *const A,
                                             uint8_t *const Buffer,
                                             const uint32_t Capacity);
void                ARRAY_Reset             (struct ARRAY *const A);
uint8_t             ARRAY_Element           (struct ARRAY *const A,
                                             const uint32_t Index);
const uint8_t *     ARRAY_Data              (struct ARRAY *const A,
                                             const uint32_t Index);
uint32_t            ARRAY_Count             (struct ARRAY *const A);
uint32_t            ARRAY_Left              (struct ARRAY *const A);
bool                ARRAY_AppendOne         (struct ARRAY *const A,
                                             const uint8_t Value);
bool                ARRAY_AppendString      (struct ARRAY *const A, 
                                             const char *const Str,
                                             const size_t MaxLen);
bool                ARRAY_AppendBinary      (struct ARRAY *const A,
                                             const uint8_t *const Data,
                                             const uint32_t Size);
uint8_t             ARRAY_RemoveLast        (struct ARRAY *const A);
bool                ARRAY_IsEqual           (struct ARRAY *const A,
                                             struct ARRAY *const B);
void                ARRAY_Copy              (struct ARRAY *const A,
                                             struct ARRAY *const B);
