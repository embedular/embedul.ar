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

#include "embedul.ar/source/core/utf8.h"
#include "embedul.ar/source/core/device/board.h"


#define BEGIN_2B_MARK           0xC0            // 0b11000000
#define BEGIN_2B_MASK           0xE0            // 0b11100000
#define BEGIN_2B_DATA_MASK      0x1F            // 0b00011111
#define BEGIN_3B_MARK           0xE0            // 0b11100000
#define BEGIN_3B_MASK           0xF0            // 0b11110000
#define BEGIN_3B_DATA_MASK      0x0F            // 0b00001111
#define PAYLOAD_MARK            0x80            // 0b10000000
#define PAYLOAD_MASK            0xC0            // 0b11000000
#define PAYLOAD_DATA_MASK       0x3F            // 0b00111111


const struct UTF8_CodePointRange UTF8_LatinPrintableAlnum[7] =
{
    // Basic Latin
    { .begin = 0x0030, .end = 0x0039 },
    { .begin = 0x0041, .end = 0x005A },
    { .begin = 0x0061, .end = 0x007A },
    // Latin 1 Supplement
    { .begin = 0x00C0, .end = 0x00D6 },
    { .begin = 0x00D8, .end = 0x00F6 },
    { .begin = 0x00F8, .end = 0x00FF },
    // Latin Extended-A
    { .begin = 0x0100, .end = 0x017F }
};


const struct UTF8_CodePointRange UTF8_Decimal[1] =
{
    // Decimal numbers in basic Latin block
    { .begin = 0x0030, .end = 0x0039 },
};


/**
 * Encodes a UTF-8 character. This function
 * will only encode a variable-width character up to three octets in
 * length, that is, a code point from the Unicode basic multilingual
 * plane (U+0000 through U+FFFF).
 *
 * :param Data: An array of :c:type:`uint8_t` containing enough space to
 *              store the encoded code point (as much as three octets).
 * :param Octets: Element count in ``data``.
 * :param CodePoint: Unicode code point from the basic multilingual plane.
 */
uint8_t UTF8_SetCodePoint (uint8_t *const Data, const uint32_t Octets,
                           const uint16_t CodePoint)
{
    BOARD_AssertParams (Data && Octets);

    // UTF-8 one octet, same as 7-bit ASCII.
    if (CodePoint < 128)
    {
        Data[0] = (uint8_t) CodePoint;

        return 1;
    }

    // UTF-8 two octets, 11 bit payload.
    if (CodePoint <= 0x7FF)
    {
        BOARD_AssertParams (Octets >= 2);

        Data[0] = BEGIN_2B_MARK | ((CodePoint >> 6) & BEGIN_2B_DATA_MASK);
        Data[1] = PAYLOAD_MARK | (CodePoint & PAYLOAD_DATA_MASK);

        return 2;
    }

    // UTF-8 three octets, 16 bit payload.
    // codepoint is <= 0xFFFF, value constrained by data type.
    BOARD_AssertParams (Octets >= 3);

    Data[0] = BEGIN_3B_MARK | ((CodePoint >> 12) & BEGIN_3B_DATA_MASK);
    Data[1] = PAYLOAD_MARK | ((CodePoint >> 6) & PAYLOAD_DATA_MASK);
    Data[2] = PAYLOAD_MARK | (CodePoint & PAYLOAD_DATA_MASK);

    return 3;
}


/**
 * Decodes a UTF-8 character. This function
 * will only assemble variable-width characters up to three octets in
 * length, enough to decode all code points from the Unicode basic
 * multilingual plane (U+0000 through U+FFFF).
 *
 * :param Data: An array of :c:type:`uint8_t` containing UTF-8 code points.
 * :param Octets: Element count in ``data``.
 */
struct UTF8_GetCodePointResult UTF8_GetCodePoint (const uint8_t *const Data,
                                                  const uint32_t Octets)
{
    BOARD_AssertParams (Data && Octets);

    // RETRO-CIAA supports the basic multilingual plane only.
    // (0x0000 - 0xFFFF)

    struct UTF8_GetCodePointResult result;

    // UTF-8 one octet, same as 7-bit ASCII.
    if (Data[0] < 128)
    {
        result.dataLength = 1;
        result.codepoint  = Data[0];
    }
    // UTF-8 two octets, 11 bit payload.
    else if (Octets > 1
             && ((Data[0] & BEGIN_2B_MASK) == BEGIN_2B_MARK)
             && ((Data[1] & PAYLOAD_MASK) == PAYLOAD_MARK))
    {
        result.dataLength = 2;
        result.codepoint  = (uint16_t)((Data[0] & BEGIN_2B_DATA_MASK) << 6 |
                                       (Data[1] & PAYLOAD_DATA_MASK));
    }
    // UTF-8 three octets, 16 bit payload.
    else if (Octets > 2
             && ((Data[0] & BEGIN_3B_MASK) == BEGIN_3B_MARK)
             && ((Data[1] & PAYLOAD_MASK) == PAYLOAD_MARK)
             && ((Data[2] & PAYLOAD_MASK) == PAYLOAD_MARK))
    {
        result.dataLength = 3;
        result.codepoint  = (uint16_t)((Data[0] & BEGIN_3B_DATA_MASK) << 12 |
                                       (Data[1] & PAYLOAD_DATA_MASK) << 6  |
                                       (Data[2] & PAYLOAD_DATA_MASK));
    }
    else
    {
        // codepoint higher than 0xFFFF or invalid encoding.
        result.dataLength = 0;
    }

    return result;
}


/**
 * Checks that UTF-8 encoded characters matches all code point ranges
 * and returns the number of valid **Basic Multilingual Plane** characters.
 * This function detects and assembles a well-formed UTF-8 character by using
 * :c:func:`UTF8_GetCodePoint`.
 *
 * :param Data: An array of :c:type:`uint8_t` elements containing UTF-8
 *              encoded characters.
 * :param Octets: Element count in ``data``.
 * :param Ranges: An array of :c:struct:`UTF8_CodePointRange` with code point
 *                ranges. An out-of-range character will fail the range test.
 *                This parameter may be :c:macro:`NULL`, in which case there
 *                will be no range checking.
 * :param RangeCount: Number of :c:struct:`UTF8_CodePointRange` ranges in the
 *                    ``ranges`` array or zero if ``ranges`` is :c:macro:`NULL`.
 * :return: :c:struct:`UTF8_CheckResult` with results.
 */
struct UTF8_CheckResult UTF8_Check (const uint8_t *const Data, 
                                    const size_t Octets,
                                    const struct UTF8_CodePointRange *const
                                    Ranges, const uint32_t RangeCount)
{
    BOARD_AssertParams (Data);
    BOARD_AssertParams ((Ranges && RangeCount) || (!Ranges && !RangeCount));

    struct UTF8_CheckResult res;
    res.validChars      = 0;
    res.invalidOctets   = 0;
    res.rangePassed     = true;

    uint32_t i = 0;

    while (i < Octets)
    {
        const uint8_t * d       = &Data[i];
        const uint32_t  Size    = Octets - i;

        const struct UTF8_GetCodePointResult CPR = UTF8_GetCodePoint (d, Size);
        // r.dataLength of zero means an invalid codepoint (higher than 0xFFFF)
        // or malformed UTF-8 stream.
        if (!CPR.dataLength)
        {
            const uint32_t SyncOffset = UTF8_ReSync (d, Size);
            res.invalidOctets += SyncOffset;

            BOARD_AssertState (res.invalidOctets <= Octets);

            // End of data
            if (SyncOffset == Size)
            {
                return res;
            }

            i += SyncOffset;
        }
        else
        {
            if (Ranges && res.rangePassed)
            {
                for (uint32_t r = 0; r < RangeCount; ++r)
                {
                    if (CPR.codepoint >= Ranges[r].begin &&
                        CPR.codepoint <= Ranges[r].end)
                    {
                        continue;
                    }
                    else
                    {
                        res.rangePassed = false;
                        break;
                    }
                }
            }

            ++ res.validChars;
            i += CPR.dataLength;
        }
    }

    return res;
}


/**
 * Checks a UTF-8 encoded stream and returns the number of valid
 * **Basic Multilingual Plane** characters plus invalid octets.
 * This function is a shortcut to calling :c:func:`UTF8_Check` with no range
 * checking.
 *
 * :param Data: An array of :c:type:`uint8_t` elements containing UTF-8
 *              encoded characters.
 * :param Octets: Element count in ``data``.
 * :return: Number of valid characters plus invalid octets.
 */
uint32_t UTF8_Count (const uint8_t *const Data, const size_t Octets)
{
    struct UTF8_CheckResult r = UTF8_Check (Data, Octets, NULL, 0);
    return r.validChars + r.invalidOctets;
}


/**
 * Resynchronizes an ill-formed multi-octet character sequence to the
 * next single-octet or first multi-octet character.
 *
 * :param Data: An array of :c:type:`uint8_t` elements.
 * :param Octets: Element count in ``data``.
 * :return: Offset to the next single-octet, first multi-octet character
 *          sequence or ``Octets`` when there is none.
 */
uint32_t UTF8_ReSync (const uint8_t *const Data, const uint32_t Octets)
{
    BOARD_AssertParams (Data && Octets);

    uint32_t offset = 0;

    do
    {
        const uint8_t Octet = Data[offset];
        // Stops at the start of a new single-octet or multi-octet sequence.
        if (Octet <= 127 || Octet >= 192)
        {
            return offset;
        }
    }
    while (++ offset < Octets);

    return offset;
}


/**
 * Removes a number of UTF-8 characters starting from the end of an
 * :c:struct:`ARRAY`.
 *
 * :param A: The :c:struct:`ARRAY` instance containing UTF-8 characters.
 * :param Count: Number of UTF-8 characters to remove from the end.
 * :return: Number of characters effectively removed.
 */
uint32_t UTF8_RemoveChars (struct ARRAY *const A, const uint32_t Count)
{
    BOARD_AssertParams (A);

    uint32_t charsRemoved = 0;

    while (ARRAY_Count (A))
    {
        if (charsRemoved >= Count)
        {
            break;
        }

        const uint8_t Octet = ARRAY_RemoveLast (A);
        // Count single ASCII or the beginning of a UTF-8 multi-octet character.
        if (Octet <= 127 || Octet >= 192)
        {
            ++ charsRemoved;
        }
    }

    return charsRemoved;
}
