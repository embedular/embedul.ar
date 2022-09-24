/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [CORE] utf-8 character encoding.

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

/**
 * .. _utf8-description:
 *
 * Description
 * ===========
 *
 * Unicode is a standardization effort to consistently use an encoding,
 * representation, and handling of writing systems from the entire world.
 * In Unicode, code points are expressed as "U+" followed by a
 * hexadecimal number. As an example, U+2623 is the biohazard sign ☣.
 * Many, but not all, of these code points map one-to-one to single
 * characters.
 *
 * .. note:: In the software industry, there were unfortunate
 *           misconceptions about what “Unicode” meant.
 *           Those misconceptions lasted several years and confused
 *           many people into directly connecting the word “Unicode”
 *           to mean nothing else than a limited implementation of a
 *           fixed 16-bit character encoding.
 *
 * UTF-8 is part of the Unicode Standard. It is an octet-based,
 * variable-length character encoding used to encode Unicode code points.
 * UTF-8 is backwards compatible with 7-bit ASCII. The eighth, most
 * significant bit set implies that the code point data splits
 * into two to four octets depending on adjacent fixed bits, as
 * depicted in the following figure:
 *
 * .. image:: images/utf8_encoding.drawio.svg
 *
 * Embedul.ar natively supports UTF-8 character strings as
 * regular string literals stored in UTF-8 encoded files, as in the
 * following example:
 *
 * .. code-block:: c
 *
 *    // Save the source code file with UTF-8 encoding to store a proper 
 *    // multi-octet sequence.
 *    const char * utf8_string = "こんにちはせかい!";
 *
 * By design, the framework only supports the Basic Multilingual Plane or BMP
 * (code points U+0000 to U+FFFF) mainly to save memory and decrease
 * implementation complexity. UTF-8 four-octet encoded sequences (code points
 * above U+FFFF) will raise errors or assertions, depending on context.
 *
 *
 * API guide
 * =========
 *
 * Code points and UTF-8 encoded buffers
 * -------------------------------------
 *
 * UTF-8 is a variable-octet encoding. Depending on each code point represented,
 * it might need more than one octet per character. The following
 * functions deal with encoding (or decoding) a code point to (or from) a UTF-8
 * encoded buffer.
 *
 * | :c:func:`UTF8_SetCodePoint`
 * | :c:func:`UTF8_GetCodePoint`
 *
 * Handling UTF-8 encoded buffer data 
 * ----------------------------------
 *
 * The variable-octet encoding nature of UTF-8 requires specialized functions
 * to check for ill-formed data, resync the data stream to the first valid
 * character in the event of an invalid sequence, count the number of UTF-8
 * characters, and remove characters without breaking the
 * encoding.
 *
 * | :c:func:`UTF8_Check`
 * | :c:func:`UTF8_Count`
 * | :c:func:`UTF8_ReSync`
 * | :c:func:`UTF8_RemoveChars`
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
 * Information returned by :c:func:`UTF8_GetCodePoint`.
 */
struct UTF8_GetCodePointResult
{
    /**
     * Data octets used to extract
     * :c:struct:`UTF8_GetCodePointResult.codepoint`.
     */
    uint16_t    dataLength;
    /** Code point in the Basic Multilingual Plane. */
    uint16_t    codepoint;
};


/**
 * Define a code point range.
 */
struct UTF8_CodePointRange
{
    /** From which code point (inclusive). */
    uint16_t    begin;
    /** To which code point (inclusive). */
    uint16_t    end;
};


/**
 * Information returned by :c:func:`UTF8_Check`.
 */
struct UTF8_CheckResult
{
    /**
     * Number of valid Basic Multilingual Plane characters.
     */
    uint32_t    validChars;
    /**
     * Number of invalid octets in the variable-width encoding.
     */
    uint32_t    invalidOctets;
    /**
     * :c:macro:`true` if every character is inside the specified ranges,
     * :c:macro:`false` otherwise.
     */
    bool        rangePassed;
};


extern const struct UTF8_CodePointRange UTF8_LatinPrintableAlnum[7];
extern const struct UTF8_CodePointRange UTF8_Decimal[1];


uint8_t         UTF8_SetCodePoint       (uint8_t *const Data,
                                         const uint32_t Octets,
                                         const uint16_t CodePoint);
struct UTF8_GetCodePointResult
                UTF8_GetCodePoint       (const uint8_t *const Data,
                                         const uint32_t Octets);
struct UTF8_CheckResult 
                UTF8_Check              (const uint8_t *const Data,
                                         const size_t Octets,
                                         const struct UTF8_CodePointRange *const
                                         Ranges,
                                         const uint32_t RangeCount);
uint32_t        UTF8_Count              (const uint8_t *const Data,
                                         const size_t Octets);
uint32_t        UTF8_ReSync             (const uint8_t *const Data,
                                         const uint32_t Octets);
uint32_t        UTF8_RemoveChars        (struct ARRAY *const A,
                                         const uint32_t Count);
