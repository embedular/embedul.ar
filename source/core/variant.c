/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [CORE] variant data type.

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

#include "embedul.ar/source/core/variant.h"
#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/core/utf8.h"
#include <stdint.h>
#include <stdlib.h>


#define BOOL_TRUE                               "true"
#define BOOL_FALSE                              "false"

#define REPLACEMENT_CHAR                        "�"
        
#define STRING_ARGS_MAX_CHAR_OCTETS             3
#define STRING_ARGS_PATTERN_MAX_CHARS           4
#define STRING_ARGS_PATTERN_TEMPLATES           8
#define STRING_ARGS_HINT_STRING_STYLES          2
#define STRING_ARGS_HINT_NUMBER_STYLES          1


const uint8_t s_stringArgsStringStyles[STRING_ARGS_HINT_STRING_STYLES] =
    { '\"', '\'' };


static void intToConv(struct VARIANT *const V)
{
    BOARD_AssertParams (V && V->type == VARIANT_Type_Int);

    char        * front     = V->conv;
    const int   IsNegative  = V->i < 0;
    uint64_t    n           = IsNegative? -(uint64_t)V->i : (uint64_t)V->i;

    if (V->i == 0)
    {
        *front++ = '0';
    }
    else
    {
        // Write digits in reverse order
        char *digit = front;
        while (n != 0) 
        {
            *front++ = '0' + (n % 10);
            n /= 10;
        }

        if (IsNegative)
        {
            *front++ = '-';
        }

        // Reverse the digits in place
        char *back = front - 1;
        while (digit < back)
        {
            char tmp = *digit;
            *digit = *back;
            *back = tmp;
            digit++;
            back--;
        }
    }

    *front = '\0';
}


static void uintToConv(struct VARIANT *const V)
{
    BOARD_AssertParams (V && V->type == VARIANT_Type_Uint);

    static const char   DigitsUppercase[] = "0123456789ABCDEF";
    static const char   DigitsLowercase[] = "0123456789abcdef";

    const char *const   Digits      = (V->base & VARIANT_BASE_FLAG_UPPER)?
                                        DigitsUppercase : DigitsLowercase;
    const uint32_t      Base        = V->base & VARIANT_BASE_MASK;
    const char          Suffix      = (V->base & VARIANT_BASE_FLAG_SUFFIX)?
                                        ((Base == VARIANT_BASE_HEX)? 'h' :
                                            (Base == VARIANT_BASE_OCT)? 'o'
                                            : '\0')
                                        : '\0';
    char                * front     = V->conv;

    if (Base != VARIANT_BASE_DEC &&
        Base != VARIANT_BASE_OCT &&
        Base != VARIANT_BASE_HEX)
    {
        BOARD_AssertUnexpectedValue (V, (uint32_t)Base);
    }

    if (V->u == 0)
    {
        *front++ = '0';
        if (Suffix) 
        {
            *front++ = Suffix;
        }
        *front = '\0';
    }
    else 
    {
        uint64_t n = V->u;
        uint32_t i = 0;

        // Fills from end to start leaving space for suffix
        while (n != 0)
        {
            V->conv[i++] = Digits[n % Base];
            n /= Base;
        }

        if (Suffix) {
            V->conv[i++] = Suffix;
        }

        V->conv[i] = '\0';

        // Reverse the string (in-place reversal) excluding suffix
        for (int start = 0, end = i - 2; start < end; start++, end--)
        { 
            char temp = V->conv[start];
            V->conv[start] = V->conv[end];
            V->conv[end] = temp;
        }
    }
}


static void fpToConv(struct VARIANT *const V)
{
    BOARD_AssertParams (V && V->type == VARIANT_Type_Fp);

    const double    InitialValue    = V->d;
    const int64_t   IntPart         = (int64_t)InitialValue;
    double          fractionalPart  = InitialValue - (double)IntPart;

    V->type = VARIANT_Type_Int;
    V->i    = IntPart;

    intToConv(V);

    V->type = VARIANT_Type_Fp;
    V->d    = InitialValue;

    size_t len = strnlen(V->conv, sizeof(V->conv));

    if (len >= sizeof(V->conv) - 3)
    {
        // No space left for at least the dot, one decimal digit and
        // string termination
        return;
    }

    V->conv[len++] = '.';

    const size_t MaxLen = (V->digits)? 
                            (len + V->digits < sizeof(V->conv) - 2)?
                                len + V->digits
                                : sizeof(V->conv) - 2
                            : sizeof(V->conv) - 2;

    while (len < MaxLen)
    {
        if (fractionalPart > 0.0)
        {
            fractionalPart *= 10.0;
            const uint8_t Digit = (uint8_t)fractionalPart;
            V->conv[len] = '0' + Digit;
            fractionalPart -= (double)Digit;
        }
        else if (!V->digits) {
            break;
        }
        else
        {
            V->conv[len] = '0';
        }
        ++ len;
    }

    V->conv[len] = '\0';
}


static void pointerToConv(struct VARIANT *const V)
{
    BOARD_AssertParams (V && V->type == VARIANT_Type_Pointer);

    const void *const           InitialValue    = V->p;
    const enum VARIANT_Base     InitialBase     = V->base;

    V->type = VARIANT_Type_Uint;
    V->base = VARIANT_Base_Hex_LowerSuffix;
    V->u    = (uintptr_t)V->p;

    uintToConv(V);

    V->type = VARIANT_Type_Pointer;
    V->base = InitialBase;
    V->p    = InitialValue;
}


/**
 * Returns a VARIANT of type :c:enum:`VARIANT_Type.VARIANT_Type_Uint`.
 *
 * :param Base: :c:enum:`VARIANT_Base` to format ``Uint`` when
 *              converted to a string.
 * :param Uint: Unsigned int value.
 * :return: A VARIANT.
 */
struct VARIANT VARIANT_CreateBaseUint (const enum VARIANT_Base Base,
                                       const uint64_t Uint)
{
    return (struct VARIANT)
    {
        .type = VARIANT_Type_Uint,
        .base = Base,
        .u = Uint
    };
}


/**
 * Returns a VARIANT of type :c:enum:`VARIANT_Type.VARIANT_Type_Uint`
 * and :c:enum:`VARIANT_Base.VARIANT_Base_Dec` base.
 *
 * :param Uint: Unsigned int value.
 * :return: A VARIANT.
 */
struct VARIANT VARIANT_CreateUint (const uint64_t Uint)
{
    return VARIANT_CreateBaseUint (VARIANT_Base_Dec, Uint);
}


/**
 * Returns a VARIANT of type :c:enum:`VARIANT_Type.VARIANT_Type_Int`.
 *
 * :param Int: Signed int value.
 * :return: A VARIANT.
 */
struct VARIANT VARIANT_CreateInt (const int64_t Int)
{
    return (struct VARIANT)
    {
        .type = VARIANT_Type_Int,
        .base = VARIANT_Base_Dec,
        .i = Int
    };
}


/**
 * Returns a VARIANT of type :c:enum:`VARIANT_Type.VARIANT_Type_Fp`.
 *
 * :param Fp: Floating point value.
 * :return: A VARIANT.
 */
struct VARIANT VARIANT_CreateFp (const double Fp)
{
    return (struct VARIANT)
    {
        .type = VARIANT_Type_Fp,
        .base = VARIANT_Base_Dec,
        .d = Fp
    };
}


/**
 * Returns a VARIANT of type :c:enum:`VARIANT_Type.VARIANT_Type_Pointer`.
 *
 * :param Pointer: Pointer value.
 * :return: A VARIANT.
 */
struct VARIANT VARIANT_CreatePointer (const void *const Pointer)
{
    return (struct VARIANT)
    {
        .type = VARIANT_Type_Pointer,
        .base = VARIANT_Base_Dec,
        .p = Pointer
    };
}


/**
 * Returns a VARIANT of type :c:enum:`VARIANT_Type.VARIANT_Type_String`.
 *
 * .. warning::
 *
 *    The object created will keep a pointer copy; it won't duplicate the
 *    original string contents. to avoid a dangling pointer, care must be taken
 *    to guarantee that the string pointer passed will not change its memory
 *    allocation.
 *
 * :param Base: The format of ``String`` is :c:enum:`VARIANT_Base`. Used when
 *              converting to a numeric type.
 * :param String: String pointer.
 * :return: A VARIANT.
 */
struct VARIANT VARIANT_CreateBaseString (const enum VARIANT_Base Base,
                                      const char *const String)
{
    return (struct VARIANT)
    {
        .type = VARIANT_Type_String,
        .base = Base,
        .s = String
    };
}


/**
 * Returns a VARIANT of type :c:enum:`VARIANT_Type.VARIANT_Type_String`.
 *
 * .. warning::
 *
 *    The object created will keep a pointer copy; it won't duplicate the
 *    original string contents. to avoid a dangling pointer, care must be taken
 *    to guarantee that the string pointer passed will not change its memory
 *    allocation.
 *
 * :param String: String value.
 * :return: A VARIANT.
 */
struct VARIANT VARIANT_CreateString (const char *const String)
{
    return VARIANT_CreateBaseString (VARIANT_Base_Dec, String);
}


/**
 * Returns a VARIANT of type :c:enum:`VARIANT_Type.VARIANT_Type_String`.
 *
 * .. note::
 *
 *    If the original instance is of type
 *    :c:enum:`VARIANT_Type.VARIANT_Type_String` then a dangling pointer
 *    warning also applies to the second instance, as discussed in
 *    :c:func:`VARIANT_SetString`.
 *
 * :param String: String value.
 * :return: A VARIANT.
 */
struct VARIANT VARIANT_CreateCopy (const struct VARIANT *const V)
{
    struct VARIANT c;
    memcpy (&c, V, sizeof(c));
    return c;
}


/**
 * Returns a VARIANT of type :c:enum:`VARIANT_Type.VARIANT_Type_Boolean`.
 *
 * :param Boolean: Boolean value.
 * :return: A VARIANT.
 */
struct VARIANT VARIANT_CreateBoolean (const _Bool Boolean)
{
    return (struct VARIANT)
    {
        .type = VARIANT_Type_Boolean,
        .base = VARIANT_Base_Dec,
        .b = Boolean
    };
}


/**
 * Sets a :c:struct:`VARIANT` instance to hold an unsigned int value.
 * :c:func:`VARIANT_ToString` will use the :c:enum:`VARIANT_Base` specified
 * to print ``value`` as a string.
 *
 * :param Base: A base in the :c:enum:`VARIANT_Base` enum.
 * :param Value: Unsigned int value.
 *
 * Usage example:
 *
 * .. code-block:: c
 *
 *    struct VARIANT v;
 *
 *    VARIANT_SetBaseUint (&v, VARIANT_Base_Dec, 2748);
 *    const char * str = VARIANT_ToString (&v);
 *    // 'str' points to the string "2748".
 *
 *    VARIANT_SetBaseUint (&v, VARIANT_Base_Hex, 2748);
 *    const char * str = VARIANT_ToString (&v);
 *    // 'str' points to the string "ABCh".
 *
 *    VARIANT_SetBaseUint (&v, VARIANT_Base_Oct, 2748);
 *    const char * str = VARIANT_ToString (&v);
 *    // 'str' points to the string "5274o".
 */
void VARIANT_SetBaseUint (struct VARIANT *const V, const enum VARIANT_Base Base,
                          const uint64_t Value)
{
    BOARD_AssertParams (V);
    *V = VARIANT_SpawnBaseUint (Base, Value);
}


/**
 * Sets a :c:struct:`VARIANT` instance to hold a signed integer value
 * using a default base of :c:enum:`VARIANT_Base.VARIANT_Base_Dec`.
 *
 * :param Value: Unsigned integer value.
 *
 * Usage example:
 *
 * .. code-block:: c
 *
 *    struct VARIANT v;
 *
 *    VARIANT_SetUint (&v, 2748);
 *    const char * str = VARIANT_ToString (&v);
 *    // 'str' points to the string "2748".
 */
void VARIANT_SetUint (struct VARIANT *const V, const uint64_t Value)
{
    VARIANT_SetBaseUint (V, VARIANT_Base_Dec, Value);
}


/**
 * Sets a :c:struct:`VARIANT` instance to hold a signed integer value.
 *
 * :param Value: Signed integer value.
 */
void VARIANT_SetInt (struct VARIANT *const V, const int64_t Value)
{
    BOARD_AssertParams (V);
    *V = VARIANT_SpawnInt (Value);
}


/**
 * Sets a :c:struct:`VARIANT` instance to hold a double-precision
 * floating-point value.
 *
 * :param Value: Double-precision floating-point value.
 */
void VARIANT_SetFp (struct VARIANT *const V, const double Value)
{
    BOARD_AssertParams (V);
    *V = VARIANT_SpawnFp (Value);
}


/**
 * Sets a :c:struct:`VARIANT` instance to hold a pointer value.
 *
 * :param Ptr: Pointer value.
 */
void VARIANT_SetPointer (struct VARIANT *const V, const void *const Ptr)
{
    BOARD_AssertParams (V);
    *V = VARIANT_SpawnPointer (Ptr);
}


/**
 * Sets a :c:struct:`VARIANT` instance to hold a pointer to a string.
 *
 * .. warning::
 *
 *    This instance will keep a pointer copy; it won't duplicate the
 *    original string contents. Care must be taken to avoid a dangling pointer
 *    by ensuring that the string pointer passed will not change its memory
 *    allocation in the lifetime of this :c:struct:`VARIANT` object.
 *
 * :param Base: The string has a number formatted according to
 *              the :c:enum:`VARIANT_Base` specified.
 *              It is used to convert from string to a numeric data type.
 * :param Str: String pointer.
 *
 * Usage example:
 *
 * .. code-block:: c
 *
 *    struct VARIANT v;
 *
 *    VARIANT_SetBaseString (&v, VARIANT_Base_Hex, "ABC");
 *    const VARIANT_Uint v = VARIANT_ToUint (&v);
 *    // 'v' equals 2748.
 *
 *    VARIANT_SetBaseString (&v, VARIANT_Base_Oct, "777");
 *    const VARIANT_Uint v = VARIANT_ToUint (&v);
 *    // 'v' equals 511.
 *
 */
void VARIANT_SetBaseString (struct VARIANT *const V,
                            const enum VARIANT_Base Base,
                            const char *const Str)
{
    BOARD_AssertParams (V);
    *V = VARIANT_SpawnBaseString (Base, Str);
}


/**
 * Sets a :c:struct:`VARIANT` instance to hold a pointer to a string.
 * This string may or may not represent a number. Nevertheless,
 * string to integer conversion defaults to
 * :c:enum:`VARIANT_Base.VARIANT_Base_Dec`.
 *
 * .. warning::
 *
 *    This instance will keep a pointer copy; it won't duplicate the
 *    original string contents. Care must be taken to avoid a dangling pointer
 *    by ensuring that the string pointer passed will not change its memory
 *    allocation in the lifetime of this :c:struct:`VARIANT` object.
 *
 * :param Str: string pointer.
 *
 * Usage example:
 *
 * .. code-block:: c
 *
 *    struct VARIANT v;
 *
 *    VARIANT_SetString (&v, "12345");
 *    const VARIANT_Uint v = VARIANT_ToUint (&v);
 *    // 'v' equals 12345.
 */
void VARIANT_SetString (struct VARIANT *const V, const char *const Str)
{
    VARIANT_SetBaseString (V, VARIANT_Base_Dec, Str);
}


/**
 * Copies type and value of a :c:struct:`VARIANT` instance into a second
 * existing instance object. The following example code performs the same
 * operation:
 *
 * .. code-block:: c
 *
 *    struct VARIANT v;
 *    struct VARIANT a;
 *
 *    v = a;
 *
 * .. note::
 *
 *    If the original instance is of type
 *    :c:enum:`VARIANT_Type.VARIANT_Type_String` then a dangling pointer
 *    warning also applies to the second instance, as discussed in
 *    :c:func:`VARIANT_SetString`.
 */
void VARIANT_SetCopy (struct VARIANT *const V, struct VARIANT *const A)
{
    BOARD_AssertParams (V && A);
    *V = VARIANT_SpawnCopy (A);
}


/**
 * Sets a :c:struct:`VARIANT` instance to hold a boolean value.
 *
 * :param Value: Boolean value.
 */
void VARIANT_SetBoolean (struct VARIANT *const V, const _Bool Value)
{
    BOARD_AssertParams (V);
    *V = VARIANT_SpawnBoolean (Value);
}


/**
 * Gets the original :c:struct:`VARIANT` instance hold value as an unsigned
 * integer number.
 *
 * :return: Value as an unsigned integer.
 */
uint64_t VARIANT_ToUint (struct VARIANT *const V)
{
    BOARD_AssertParams (V);

    switch (V->type)
    {
        case VARIANT_Type_Uint:
            return V->u;

        case VARIANT_Type_Int:
            return (uint64_t) V->i;

        case VARIANT_Type_Fp:
            return (uint64_t) V->d;

        case VARIANT_Type_Pointer:
            return (uint64_t) ((uintptr_t)V->p);

        case VARIANT_Type_String:
            return (uint64_t) strtoull (V->s, NULL, 
                                        (int)(V->base & VARIANT_BASE_MASK));

        case VARIANT_Type_Boolean:
            return (uint64_t) V->b? 1u : 0u; 
    }

    // All types already covered
    BOARD_AssertUnexpectedValue (V, (uint32_t)V->type);
    return 0;
}


/**
 * Gets the :c:struct:`VARIANT` instance hold value as a signed integer
 * number.
 *
 * .. note::
 *
 *    A value of type :c:enum:`VARIANT_Type.VARIANT_Type_Pointer`
 *    cannot be converted to a signed integer. Do not call this function
 *    on pointers; this condition is asserted.
 *
 * :return: Value as a signed integer.
 */
int64_t VARIANT_ToInt (struct VARIANT *const V)
{
    BOARD_AssertParams (V);
    BOARD_AssertState  (V->type != VARIANT_Type_Pointer);

    switch (V->type)
    {
        case VARIANT_Type_Uint:
            return (int64_t) V->u;

        case VARIANT_Type_Int:
            return V->i;

        case VARIANT_Type_Fp:
            return (int64_t) V->d;

        case VARIANT_Type_Pointer:
            break;

        case VARIANT_Type_String:
            return (int64_t) strtoll (V->s, NULL,
                                        (int)(V->base & VARIANT_BASE_MASK));

        case VARIANT_Type_Boolean:
            return (int64_t) V->b? 1 : 0;
    }

    return 0;
}


/**
 * Gets the :c:struct:`VARIANT` instance hold value as a double-precision
 * floating-point number.
 *
 * .. note::
 *
 *    Conversion from :c:enum:`VARIANT_Type.VARIANT_Type_Pointer`
 *    or :c:enum:`VARIANT_Type.VARIANT_Type_Boolean` to floating-point is
 *    invalid; this condition is asserted. Do not call this function on a
 *    :c:struct:`VARIANT` holding those types.
 *
 * :return: Value as a double-precision floating-point.
 */
double VARIANT_ToDouble (struct VARIANT *const V)
{
    BOARD_AssertParams (V);
    BOARD_AssertState  (V->type != VARIANT_Type_Pointer &&
                         V->type != VARIANT_Type_Boolean);

    switch (V->type)
    {
        case VARIANT_Type_Uint:
            return (double) V->u;

        case VARIANT_Type_Int:
            return (double) V->i;

        case VARIANT_Type_Fp:
            return V->d;

        case VARIANT_Type_Pointer:
            break;

        case VARIANT_Type_String:
            return strtod (V->s, NULL);

        case VARIANT_Type_Boolean:
            break;
    }

    return 0.0;
}


/**
 * Gets the :c:struct:`VARIANT` instance hold value as a string.
 *
 * :return: Pointer to the :c:struct:`VARIANT` instance temporary
 *          buffer with its value converted to a string.
 *
 *          .. warning::
 *
 *             The :c:struct:`VARIANT` instance owns the string buffer returned,
 *             with the exeption of type
 *             :c:enum:`VARIANT_Type.VARIANT_Type_Boolean` where values are
 *             statically allocated. The caller
 *             must duplicate the contents immediately to preserve them.
 */
const char* VARIANT_ToString (struct VARIANT *const V)
{
    BOARD_AssertParams (V);

    memset (V->conv, 0, sizeof(V->conv));

    switch (V->type)
    {
        case VARIANT_Type_Uint:
            uintToConv(V);
            break;

        case VARIANT_Type_Int:
            intToConv(V);
            break;

        case VARIANT_Type_Fp:
            fpToConv(V);
            break;

        case VARIANT_Type_Pointer:
            pointerToConv(V);
            break;

        case VARIANT_Type_String:
            return V->s;

        case VARIANT_Type_Boolean:
            return V->b? BOOL_TRUE : BOOL_FALSE;
    }

    return V->conv;
}


/**
 * Gets the :c:struct:`VARIANT` instance hold value converted to a string and
 * copy it to an already allocated char array buffer.
 * Buffer must hold the entire string including its trailing NULL character.
 * This condition is asserted.
 *
 * :param Buffer: An already allocated char array.
 * :param Size: Char array size, in octets.
 */
void VARIANT_ToStringBuffer (struct VARIANT *const V, char *const Buffer,
                             const uint32_t Size)
{
    BOARD_AssertParams (V && Buffer && Size);
    const uint32_t StrOctetCount = VARIANT_StringOctetCount (V);
    BOARD_AssertParams (Size >= StrOctetCount);

    const char *Str = VARIANT_ToString (V);
    memcpy (Buffer, Str, StrOctetCount);
}


/**
 * Gets the :c:struct:`VARIANT` instance hold value as a boolean; either
 * one (True) or zero (False).
 *
 * :return: Value as a boolean.
 */
_Bool VARIANT_ToBoolean (struct VARIANT *const V)
{
    BOARD_AssertParams (V);

    switch (V->type)
    {
        case VARIANT_Type_Uint:
            return (V->u)? 1u : 0u;

        case VARIANT_Type_Int:
            return (V->i)? 1u : 0u;

        case VARIANT_Type_Fp:
            return (V->d != 0.0)? 1u : 0u;

        case VARIANT_Type_Pointer:
            return (V->p)? 1u : 0u;

        case VARIANT_Type_String:
            return (V->s)? 1u : 0u;

        case VARIANT_Type_Boolean:
            return V->b;
    }

    // All types already covered
    BOARD_AssertUnexpectedValue (V, (uint32_t)V->type);
    return 0;
}


enum VARIANT_Type VARIANT_GetType (struct VARIANT *const V)
{
    BOARD_AssertParams (V);
    return V->type;
}


enum VARIANT_Base VARIANT_GetBase (struct VARIANT *const V)
{
    BOARD_AssertParams (V);
    return V->base;
}


VARIANT_Digits VARIANT_GetDigits (struct VARIANT *const V)
{
    BOARD_AssertParams (V);
    return V->digits;
}


/**
 * Changes base conversion on a :c:struct:`VARIANT` instance of type 
 * :c:enum:`VARIANT_Type.VARIANT_Type_Uint`; it does nothing otherwise.
 *
 * :param Base: New base. :c:func:`VARIANT_ToString` will format this
 *              unsigned value accordingly. If zero, nothing will be changed 
 *              and the return value will be zero.
 * :return: Previous base or zero if the :c:struct:`VARIANT` instance is not
 *          of type :c:enum:`VARIANT_Type.VARIANT_Type_Uint`
 */
enum VARIANT_Base VARIANT_ChangeBase (struct VARIANT *const V,
                                      const enum VARIANT_Base Base)
{
    BOARD_AssertParams (V);

    if (V->type != VARIANT_Type_Uint || !Base)
    {
        return 0;
    }

    const enum VARIANT_Base PrevBase = V->base;
    V->base = Base;

    return PrevBase;
}


/**
 * Changes the number of fixed decimal digits or padding zeros when converting
 * a floating-point number to string. Use with :c:struct:`VARIANT` instances of
 * type :c:enum:`VARIANT_Type.VARIANT_Type_Fp` only; that condition is asserted.
 *
 * :param Digits: Decimal digits or padding zeros to output when converting
 *                this floating-point value to string. If 'x' is zero, the
 *                string conversion will output decimals as necessary.
 *                In both cases, the upper limit will be the :c:struct:`VARIANT`
 *                internal conversion buffer capacity.
 * :return: Previous decimal digits.
 */
VARIANT_Digits VARIANT_ChangeDigits (struct VARIANT *const V,
                                     const VARIANT_Digits Digits)
{
    BOARD_AssertParams (V && V->type == VARIANT_Type_Fp);

    const VARIANT_Digits PrevDigits = V->digits;
    V->digits = Digits;

    return PrevDigits;
}


/**
 * Gets the :c:struct:`VARIANT` instance hold value as a string and return the
 * number of octets required to store the string, including its trailing NULL.
 *
 * :return: Octet count.
 */
uint32_t VARIANT_StringOctetCount (struct VARIANT *const V)
{
    BOARD_AssertParams (V);

    const char *const   Str         = VARIANT_ToString (V);
    const uint32_t      OctetCount  = strlen(Str) + 1;

    return OctetCount;
}


static bool compareString (const char *const S1, const char *const S2)
{
    // Same pointer means same data
    if (S1 == S2)
    {
        return true;
    }

    uint32_t i = 0;
    while (S1[i] && S2[i])
    {
        if (S1[i] != S2[i])
        {
            return false;
        }
        ++ i;
    }

    // Both ends with a NULL character
    if (!S1[i] && !S2[i])
    {
        return true;
    }

    return false;
}


/**
 * Checks types and values of two :c:struct:`VARIANT` instances for equality.
 *
 * :return: :c:macro:`true` if both types and values are equal,
 *          :c:macro:`false` otherwise.
 */
bool VARIANT_IsEqual (struct VARIANT *const V, struct VARIANT *const A)
{
    BOARD_AssertParams (V && A);

    if (V->type != A->type)
    {
        return false;
    }

    switch (V->type)
    {
        case VARIANT_Type_Uint:
            return (V->u == A->u);

        case VARIANT_Type_Int:
            return (V->i == A->i);

        case VARIANT_Type_Fp:
            return (V->d == A->d);

        case VARIANT_Type_Pointer:
            return (V->p == A->p);

        case VARIANT_Type_String:
            return compareString (V->s, A->s);

        case VARIANT_Type_Boolean:
            return (V->b == A->b);
    }

    BOARD_AssertUnexpectedValue (V, (uint32_t)V->type);
    return false;
}


/**
 * Checks values of two :c:struct:`VARIANT` instances for equality
 * by converting both values to unsigned int.
 *
 * :return: :c:macro:`true` if both unsigned int values are equal,
 *          :c:macro:`false` otherwise.
 */
bool VARIANT_IsEqualAsUint (struct VARIANT *const V, struct VARIANT *const A)
{
    BOARD_AssertParams (V && A);

    return (VARIANT_ToUint(V) == VARIANT_ToUint(A));
}


struct StringArgsPatternChar
{
    uint32_t        octets;
    uint8_t         data[STRING_ARGS_MAX_CHAR_OCTETS];
};


struct StringArgsPattern
{
    struct StringArgsPatternChar    chars[STRING_ARGS_PATTERN_MAX_CHARS];
    uint32_t                        charCount;
};


inline static void setPatternDataIndex (struct StringArgsPattern *const Pattern,
                                        const uint8_t *const Data,
                                        const uint32_t Octets,
                                        const uint32_t Index)
{
    for (uint32_t i = 0; i < Octets; ++i)
    {
        Pattern->chars[Index].data[i] = Data[i];
    }

    Pattern->chars[Index].octets = Octets;
    Pattern->charCount = Index + 1;
}


// One indexed character of one octet
inline static void resetPatternC1Index (struct StringArgsPattern *const Pattern,
                                        const uint8_t C1, const uint32_t Index)
{
    Pattern->chars[Index].data[0] = C1;
    Pattern->chars[Index].octets = 1;
}


// Pattern made of one character of one octet
inline static void resetPatternC11 (struct StringArgsPattern *const Pattern,
                                    const uint8_t C1)
{
    resetPatternC1Index (Pattern, C1, 0);
    Pattern->charCount = 1;
}

// Pattern made of two characters of one octet each
inline static void resetPatternC21 (struct StringArgsPattern *const Pattern,
                                    const uint8_t C1, const uint8_t C2)
{
    resetPatternC1Index (Pattern, C1, 0);
    resetPatternC1Index (Pattern, C2, 1);
    Pattern->charCount = 2;
}


inline static void resetPattern (struct StringArgsPattern *const Pattern,
                                 const uint32_t Template)
{
    switch (Template)
    {
        case 0:
            resetPatternC11 (Pattern, ' ');
            break;

        case 1:
            resetPatternC11 (Pattern, '-');
            break;

        case 2:
            resetPatternC11 (Pattern, '.');
            break;

        case 3:
            resetPatternC11 (Pattern, '*');
            break;

        case 4:
            resetPatternC21 (Pattern, '-', ' ');
            break;

        case 5:
            resetPatternC21 (Pattern, '.', ' ');
            break;

        case 6:
            resetPatternC21 (Pattern, '*', ' ');
            break;

        case 7:
            resetPatternC11 (Pattern, '0');
            break;

        default:
            BOARD_AssertParams (false);
            break;
    }
}


static uint32_t setPattern (struct StringArgsPattern *const Pattern,
                            const uint8_t *const Data, 
                            const uint32_t Chars)
{
    uint32_t octets = 0;
    const uint8_t *p = Data;

    for (uint32_t charIdx = 0; charIdx < Chars; ++charIdx)
    {
        if (p[0] == '\0')
        {
            break;
        }

        // three octets at most for each UTF-8 character
        struct UTF8_GetCodePointResult r = UTF8_GetCodePoint (p, 3);
        if (!r.dataLength)
        {
            break;
        }

        setPatternDataIndex (Pattern, p, r.dataLength, charIdx);

        octets  += r.dataLength;
        p       += r.dataLength;
    }

    return octets;
}


static void outPattern (VARIANT_PSA_OutProc const OutProc,
                        void *const OutProcParam,
                        struct StringArgsPattern *const Pattern,
                        const uint32_t Col,
                        const uint32_t Chars)
{
    for (uint32_t i = Col; i < Col + Chars; ++i)
    {
        const struct StringArgsPatternChar *const Pc = 
                                    &Pattern->chars[i % Pattern->charCount];

        OutProc (OutProcParam, (const uint8_t *)Pc->data, Pc->octets);
    }
}


static uint32_t outUTF8Count (VARIANT_PSA_OutProc const OutProc,
                              void *const OutProcParam,
                              const uint8_t *const Data,
                              const uint32_t Octets)
{
    OutProc (OutProcParam, Data, Octets);
    return UTF8_Count ((const uint8_t *const)Data, Octets);
}


static void outTransformAscii (VARIANT_PSA_OutProc const OutProc,
                               void *const OutProcParam,
                               const uint8_t Octet,
                               const uint8_t BeginRange,
                               const uint8_t EndRange,
                               char Delta)
{
    if (Octet < 128 && Octet >= BeginRange && Octet <= EndRange)
    {
        const uint8_t OctetOut = Octet + Delta;
        OutProc (OutProcParam, &OctetOut, 1);
    }
    else
    {
        OutProc (OutProcParam, &Octet, 1);
    }
}


static void outLowerAscii (VARIANT_PSA_OutProc const OutProc,
                           void *const OutProcParam,
                           const uint8_t *const Data,
                           const uint32_t Octets)
{
    for (uint32_t i = 0; i < Octets; ++i)
    {
        outTransformAscii (OutProc, OutProcParam, Data[i], 65, 90, 32);
    }
}


static void outUpperAscii (VARIANT_PSA_OutProc const OutProc,
                           void *const OutProcParam,
                           const uint8_t *const Data,
                           const uint32_t Octets)
{
    for (uint32_t i = 0; i < Octets; ++i)
    {
        outTransformAscii (OutProc, OutProcParam, Data[i], 97, 122, -32);
    }
}


static void outLowerCapAscii (VARIANT_PSA_OutProc const OutProc,
                              void *const OutProcParam,
                              const uint8_t *const Data,
                              const uint32_t Octets)
{
    // Capitalise first character, everything else in lowercase
    outUpperAscii (OutProc, OutProcParam, &Data[0], 1);
    outLowerAscii (OutProc, OutProcParam, &Data[1], Octets - 1);
}


static void outLowerCapWordsAscii (VARIANT_PSA_OutProc const OutProc,
                                   void *const OutProcParam,
                                   const uint8_t *const Data,
                                   const uint32_t Octets)
{
    // Capitalise each word     
    bool nextUpper = true;
    for (uint32_t i = 0; i < Octets; ++i)
    {
        if (Data[i] == ' ')
        {
            OutProc (OutProcParam, &Data[i], 1);
            nextUpper = true;
        }
        else if (nextUpper)
        {
            outUpperAscii (OutProc, OutProcParam, &Data[i], 1);
            nextUpper = false;
        }
        else
        {
            outLowerAscii (OutProc, OutProcParam, &Data[i], 1);
        }
    }
}


struct OutProcHintParams
{
    VARIANT_PSA_OutProc     origOutProc;
    void                    * origOutProcParam;
    uint32_t                hintNumbers;
    enum VARIANT_Type       variantType;
    enum VARIANT_Base       variantBase;
    uint32_t                hintStrings;
    uint32_t                extraChars;
    uint8_t                 escapeAsciiChar;
    uint8_t                 escapeAsciiSymbol;
};


static void outHintString (struct OutProcHintParams *const Hp,
                           const uint8_t *const Data,
                           const uint32_t Octets)
{
    VARIANT_PSA_OutProc const 
                    OutProc             = Hp->origOutProc;
    void *const     OutProcParam        = Hp->origOutProcParam;
    const uint8_t   EscapeAsciiChar     = Hp->escapeAsciiChar;
    const uint8_t   EscapeAsciiSymbol   = Hp->escapeAsciiSymbol;

    OutProc (OutProcParam, &EscapeAsciiChar, 1);
    Hp->extraChars += 1;

    uint32_t b = 0;

    for (uint32_t i = 0; i < Octets; ++i)
    {
        if (Data[i] < 128 && Data[i] == EscapeAsciiChar)
        {
            if (b != i)
            {
                OutProc (OutProcParam, &Data[b], i - b);
                b = i + 1;
            }

            OutProc (OutProcParam, &EscapeAsciiSymbol, 1);
            Hp->extraChars += 1;
            OutProc (OutProcParam, &EscapeAsciiChar, 1);
        }
    }

    if (b != Octets)
    {
        OutProc (OutProcParam, &Data[b], Octets - b);
    }

    OutProc (OutProcParam, &EscapeAsciiChar, 1);
    Hp->extraChars += 1;
}


static void outHintNumber (struct OutProcHintParams *const Hp,
                           const uint8_t *const Data,
                           const uint32_t Octets)
{
    VARIANT_PSA_OutProc const   OutProc         = Hp->origOutProc;
    void *const                 OutProcParam    = Hp->origOutProcParam;
    const uint32_t              HintStyle       = Hp->hintNumbers;
    const enum VARIANT_Type     VariantType     = Hp->variantType;
    const enum VARIANT_Base     VariantBase     = Hp->variantBase;

    switch (HintStyle)
    {
        case 0:
        {
            BOARD_AssertUnexpectedValue (NOBJ, HintStyle);
            break;
        }

        // Base subscript
        case 1:
        {
            // Subscript zero "0". To get a subscript decimal digit, add the 
            // digit amount. For example U+2080 + 5 for subscript "5".
            // a baseUtf8[x][0] == 0 will mark an unused digit.
            uint8_t baseUtf8[2][3] = { { 0xE2, 0x82, 0x80 }, 
                                       { 0xE2, 0x82, 0x80 } };

            OutProc (OutProcParam, Data, Octets);

            switch (VariantType)
            {
                case VARIANT_Type_Pointer:
                    // Base 16 for pointers
                    baseUtf8[0][2] += 1;
                    baseUtf8[1][2] += 6;
                    break;
                
                case VARIANT_Type_Boolean:
                    // Base 2 for boolean
                    baseUtf8[0][2] += 2;
                    baseUtf8[1][0] = 0;
                    break;

                case VARIANT_Type_Fp:
                    // Floating point
                    baseUtf8[1][2] += 1;
                    break;

                case VARIANT_Type_Int:
                    // Base 10 for int
                    baseUtf8[0][2] += 1;
                    break;

                case VARIANT_Type_Uint:
                {
                    const uint32_t VariantBaseMask = VariantBase & 
                                                        VARIANT_BASE_MASK;
                    // An Uint may be converted to string using any of
                    // three bases: octal, decimal, or hexadecimal.
                    switch (VariantBaseMask)
                    {
                        case 8:
                            baseUtf8[0][2] += 8;
                            baseUtf8[1][0] = 0;
                            break;
                        
                        case 10:
                            baseUtf8[0][2] += 1;
                            break;

                        case 16:
                            baseUtf8[0][2] += 1;
                            baseUtf8[1][2] += 6;
                            break;

                        default:
                            BOARD_AssertUnexpectedValue (NOBJ, VariantBaseMask);
                            break;
                    }

                    break;
                }

                case VARIANT_Type_String:
                    BOARD_AssertUnexpectedValue (NOBJ, (uint32_t)VariantType);
                    break;
            }

            // Used octets in baseSub
            const uint32_t BaseSubOctets = baseUtf8[1][0]? 6 : 3;

            OutProc (OutProcParam, (const uint8_t *)baseUtf8, BaseSubOctets);

            Hp->extraChars += BaseSubOctets;
            break;
        }

        default:
        {
            BOARD_AssertUnexpectedValue (NOBJ, HintStyle);
            break;
        }
    }
}


static void outProcHint (void *const Param, const uint8_t *const Data,
                         const uint32_t Octets)
{
    struct OutProcHintParams *const Hp = (struct OutProcHintParams *) Param;

    if (Hp->hintStrings && Hp->variantType == VARIANT_Type_String)
    {
        outHintString (Hp, Data, Octets);
    }
    else if (Hp->hintNumbers && Hp->variantType != VARIANT_Type_String)
    {
        outHintNumber (Hp, Data, Octets);
    }
    else
    {
        Hp->origOutProc (Hp->origOutProcParam, Data, Octets);
    }
}


static void outProcHintNewArg (struct OutProcHintParams *const Hp,
                               struct VARIANT *V)
{
    Hp->variantType = VARIANT_GetType (V);
    Hp->variantBase = VARIANT_GetBase (V);
    Hp->extraChars  = 0;
}


static void outProcHintStrings (struct OutProcHintParams *const Hp,
                                const uint32_t HintStrings)
{
    Hp->hintStrings = HintStrings % STRING_ARGS_HINT_STRING_STYLES;

    switch (Hp->hintStrings)
    {
        case 1:
            Hp->escapeAsciiChar = '\"';
            break;

        case 2:
            Hp->escapeAsciiChar = '\'';
            break;
    }
}


static void outProcHintNumbers (struct OutProcHintParams *const Hp,
                                const uint32_t HintNumbers)
{
    Hp->hintNumbers = HintNumbers % STRING_ARGS_HINT_STRING_STYLES;
}


// Compensates for CSI escape sequences; these characters won't be printed.
static uint32_t countCSIAsciiChars (const uint8_t *const ArgS,
                                    const uint32_t ArgO)
{
    uint32_t charsSkipped = 0;

    for (uint32_t i = 0; i < ArgO; ++i)
    {
        if (ArgS[i] == '\033' && i + 1 < ArgO && ArgS[i+1] == '[')
        {
            i += 2;
            charsSkipped += 2;
            // Keep skipping ascii chars until finding the end of the CSI
            // sequence.
            while (i < ArgO && ArgS[i] < 0x40)
            {
                ++ i;
                ++ charsSkipped;
            }

            // Skipping cancelled by a CSI end sequence char, not by 
            // reaching Arg0.
            if (i < ArgO)
            {
                // CSI end sequence char must lie between 0x40 and 0x7E,
                // anything else is a malformed CSI sequence.
                BOARD_AssertState (ArgS[i] >= 0x40 && ArgS[i] <= 0x7E);

                ++ i;
                ++ charsSkipped;
            }
        }
    }

    return charsSkipped;
}


/**
 * Parses a UTF-8 string to interpret and substitute special character sequences
 * with corresponding arguments in a :c:struct:`VARIANT` array, insert
 * characters to fill spaces, perform argument tabulation, or specify argument
 * formatting. The character that initiates a interpreted character sequence is
 * the grave accent (\`).
 *
 * There are four special interpreted character sequence categories, each one
 * starts with the grave accent:
 *
 * 1. Markers: \`0 to \`99.
 * 2. Marker style specifiers: \`o, \`O, \`d, \`x, \`X, \`h, \`H,
 *    \`U, \`l, \`c, and \`w.
 * 3. Commands: \`R, \`S, \`P, \`T, \`M, \`F, and \`L.
 * 4. Parser state options: \`$, \`#, \`^, and \`&.
 *
 * **Markers** follows the format "\`n", where 'n' represents a
 * decimal number from 0 to 99 inclusive, for example "\`0", "\`7", "\`25".
 * A marker indexes a zero-based array of :c:struct:`VARIANT` arguments to
 * output that indexed array value in place of the marker.
 *
 * Markers have the following advantages over classic :c:func:`printf` format
 * specifiers:
 *
 * - They represent indices to an array of :c:struct:`VARIANT`;
 *   there is no need to state argument types as any :c:struct:`VARIANT`
 *   knows the type it contains and can automatically convert it to a string.
 * - They can be placed on the string in non-sequential order ("\`6\`0\`3") or
 *   be repeated ("\`0\`0"), helping string translation and
 *   internationalization.
 * - The argument parameter list is automatically constructed from user supplied
 *   arguments. There is no risk of illegal memory accesses or memory corruption
 *   when referencing undefined arguments.
 *
 * A marker will be replaced with a Unicode replacement character (�) if there
 * is an error retrieving the :c:struct:`VARIANT` value. For example, when the
 * marker index is higher or equal than array elements available.
 *
 *
 * **Marker style specifiers** modifies the marker argument ('n') output as 
 * follows:
 *
 * \`o\ *n*
 *   Octal (base 8).
 *
 * \`O\ *n*
 *   Octal (base 8) suffixed with an "o".
 *
 * \`d\ *n*
 *   Decimal (base 10).
 *
 * \`x\ *n*
 *   Hexadecimal (base 16) with lowercase digits (a-f).
 *
 * \`X\ *n*
 *   Hexadecimal (base 16) with uppercase digits (A-F).
 *
 * \`h\ *n*
 *   Hexadecimal (base 16) with lowercase digits (a-f), suffixed with an "h".
 *
 * \`H\ *n*
 *   Hexadecimal (base 16) with uppercase digits (A-F), suffixed with an "h".
 *
 *
 * As an example, "\`o2" will format the output of ``ArgValues[2]`` to octal,
 * while "\`H1" will format ``ArgValues[1]`` as hexadecimal with uppercase
 * digits and an appended "h" suffix.
 *
 * .. note::
 *
 *    Base conversion will only work with a :c:struct:`VARIANT` whose original
 *    data type is an unsigned integer. it will do nothing on a signed integer,
 *    double, string or pointer values.
 *
 *    The default representation for all numeric types is decimal.    
 *
 * The following marker style specifiers only affect 7-bit ASCII character 
 * strings, any other character or UTF-8 sequence will be ignored:
 *
 * \`U\ *n*
 *   Uppercase.
 *
 * \`l\ *n*
 *   Lowercase.
 *
 * \`c\ *n*
 *   Lowercase with capitalisation.
 *
 * \`w\ *n*
 *   Lowercase with capitalisation on each word.
 *
 * .. note::
 *
 *    Marker style specifiers are mutually exclusive: only a single specifier
 *    modifies the foremost marker. Sequences like "\`X\`l1" are invalid.
 *
 *
 * The purpose of **Commands** is to create spacing, tabulation, and centering
 * by inserting characters based on a specific pattern. A command
 * follows the format "\`Xp", where 'X' is a single letter describing the
 * command and 'p' is the command parameter, which is a decimal number ranging
 * from 0 to 99, inclusive.
 *
 * The following commands are available:
 *
 * \`R\ *p* - Reset pattern
 *   Resets current character pattern by assigning a predefined pattern 'p' as
 *   shown in the following table:
 *
 *   +-----------+----------+-------------+
 *   |Pattern 'p'|Characters|Sample output|
 *   +===========+==========+=============+
 *   |0          |' '       |"     "      |
 *   +-----------+----------+-------------+
 *   |1          |'-'       |"\-\-\-\-\-" |
 *   +-----------+----------+-------------+
 *   |2          |'.'       |"....."      |
 *   +-----------+----------+-------------+
 *   |3          |'\*'      |"\*\*\*\*\*" |
 *   +-----------+----------+-------------+
 *   |4          |'-', ' '  |"- - -"      |
 *   +-----------+----------+-------------+
 *   |5          |'.', ' '  |". . ."      |
 *   +-----------+----------+-------------+
 *   |6          |'\*', ' ' |"\* \* \*"   |
 *   +-----------+----------+-------------+
 *   |7          |'0'       |"00000"      |
 *   +-----------+----------+-------------+
 *
 * \`S\ *p* - Set pattern
 *   Stores the next 'p' UTF-8 characters as the current character pattern to
 *   use in spacing and tabulation commands. Up to four characters of three
 *   octets each can be stored. Stored characters are considered part of the
 *   command itself and won't be printed.
 *
 * .. note::
 *
 *    Reset pattern 0 is the default. A set pattern is persistent
 *    thorough the parsed string.
 *
 * \`P\ *p* - Out pattern
 *   Inserts 'p' pattern characters.
 *
 * \`T\ *p* - Next marker argument tabulation
 *   Tabulates the next marker argument to the **left** of line column 'p' by
 *   inserting sufficient pattern characters.
 *
 * \`M\ *p* - Next marker argument centering
 *   Centers the next marker argument around column 'p' by inserting
 *   sufficient pattern characters.
 *
 * \`F\ *p* - Fill to column
 *   Inserts sufficient pattern characters to reach column 'p'.
 *
 * \`L\ *p* - Newline
 *   Inserts 'p' newline sequences ("\\r\\n"). An \`L with no numeric parameter
 *   placed right before the string end ("\`L\\0") will print a single newline.
 *
 * For example, "\`T30\`0" places ``ArgValues[0]`` to the left of column 30,
 * "\`M15\`1" centers ``ArgValues[1]`` around column 15, and "\`R7\`T8\`X0"
 * formats an eight-digit hexadecimal number with leading zeros.
 *
 * .. note::
 *
 *    Argument tabulation and centering commands insert no pattern characters
 *    when line column 'p' is lower than the argument character count.
 *
 *
 *
 * Finally, **Parser state options** changes the parser behavior on
 * *command parameters* and controls printing of hints on argument types.
 * It also determines the number of decimal digits in all floating-point to
 * string conversions, overriding individual VARIANT preferentes.
 *
 * \`# - Command parameter behaviour: immediate mode.
 *   Command parameter ('p') is passed as-is to the command.
 *   This is the default. For example, the command "\`#\`P5" will insert five
 *   pattern characters.
 *
 * \`$ - Command parameter behaviour: indirect mode.
 *   Command parameter ('p') is interpreted as a marker ('n') to index
 *   ``ArgValues``. Then, the command gets the :c:func:`VARIANT_ToUint`
 *   conversion of ``ArgValues[n]`` as its parameter. For example, the command
 *   "\`$\`P2" will insert ``VARIANT_ToUint(ArgValues[2])`` pattern characters.
 *
 * \`^\ *x* - Argument type hint for strings.
 *   Decorates arguments of type :c:enum:`VARIANT_Type.VARIANT_Type_String`
 *   as follows:
 *
 *   +-----------+-----------------------+-----------------------+
 *   |'x'        |Effect on strings      |Sample output          |
 *   +===========+=======================+=======================+
 *   |0          |Print verbatim         |This is a "test".      |
 *   +-----------+-----------------------+-----------------------+
 *   |1          |Double quotes          |"This is a \\"test\\"."|
 *   +-----------+-----------------------+-----------------------+
 *   |2          |Single quotes          |'This is a "test".'    |
 *   +-----------+-----------------------+-----------------------+
 *
 * \`&\ *x* - Argument value hint for numbers.
 *   Decorates arguments of types other than 
 *   :c:enum:`VARIANT_Type.VARIANT_Type_String` as follows:
 *
 *   +-----------+-----------------------+----------------------------+
 *   |'x'        |Effect on numbers      |Sample output = 255         |
 *   |           |                       |(base hex, upper, no suffix)|
 *   +===========+=======================+============================+
 *   |0          |Print verbatim         |FF                          |
 *   +-----------+-----------------------+----------------------------+
 *   |1          |Base as subscript      |FF₁₆                        |
 *   +-----------+-----------------------+----------------------------+
 *
 * \`.\ *x* - Global Floating-point to string decimal digits
 *   All :c:struct:`VARIANT` of type
 *   :c:enum:`VARIANT_Type.VARIANT_Type_Fp` will follow 'x' ignoring their
 *   internal setting for decimal digits. Set 'x' to zero to disable this
 *   global setting.
 *
 *   For example, let ``ArgValues[0]`` == "String value", 
 *   ``ArgValues[1]`` == false, and parser string "Sample: \`^21\`0, \`1".
 *   The resulting message will be: "Sample: 'String value', false₂".
 *
 *
 * To escape the grave accent character, use two grave acents "\`\`". Any other
 * octet after the grave accent not discussed above is invalid, for example:
 * "\`!", "\`V", or "\`Z". An interpreted character sequence or octet can
 * immediately follow a previous valid sequence. These are valid examples:
 * "\`4\`5", "\`h\`0[\`P3\`1]`L2", or "\`0\`0\`\`".
 *
 * The resulting parsed string is sequentially stored in one or more calls
 * using the user-supplied :c:type:`VARIANT_PSA_OutProc` function.
 *
 * .. note::
 *
 *    There is no way to know how many octets will require a parsed
 *    string without parsing it first. The application programmer can
 *    implement a :c:type:`VARIANT_PSA_OutProc` function to store partial
 *    results to a fixed-sized buffer but should also handle out of memory
 *    situations whilst parsing. There are convenient alternatives already
 *    designed and thoroughly used across the framework:
 *
 *    - Use the same fixed-sized buffer but managed through a
 *      :c:struct:`CYCLIC` data structure with no risk of memory overruns. See
 *      :c:func:`CYCLIC_IN_FromParsedStringArgs`.
 *    - Use a :c:struct:`STREAM` to directly output to, for example, a
 *      hardware UART implementation.
 *      See :c:func:`STREAM_IN_FromParsedStringArgs`.
 *
 *    Both implement their own :c:type:`VARIANT_PSA_OutProc` internally.
 *
 *
 * :param OutColumn: Initial output column in current line. Usually the
 *                   last output column retured from a previous call.
 * :param MaxOctets: The null termination in ``Str`` must appear before
 *                   reaching ``MaxOctets``; this condition is asserted.
 * :param Str: Pointer to a string with :c:struct:`VARIANT` markers to
 *             substitute. It may be :ref:`UTF-8 <utf8-description>`
 *             encoded.
 * :param OutProc: Function that stores resulting string octets as ``Str`` is
 *                 parsed and substituted. See :c:type:`VARIANT_PSA_OutProc`
 *                 for details.
 * :param OutProcParam: Optional parameter passed by the caller to the
 *                      ``OutProc`` function.
 * :param ArgValues: Array of :c:struct:`VARIANT`, or :c:macro:`NULL` if there
 *                   are no arguments available.
 * :param ArgCount: Number of elements in the ``ArgValues`` array or zero
 *                  if ``ArgValues`` is :c:macro:`NULL`.
 * :return: Last output column in current line. Useful to keep track of 
 *          last output column between parsing calls. Typically zero if
 *          ``Str`` ended with a newline character or newline command.
 */
uint32_t VARIANT_ParseStringArgs (const uint32_t OutColumn,
                                  const size_t MaxOctets, const char *const Str, 
                                  VARIANT_PSA_OutProc const OutProc,
                                  void *const OutProcParam,
                                  struct VARIANT *const ArgValues,
                                  const uint32_t ArgCount)
{
    BOARD_AssertParams (Str && OutProc);
    BOARD_AssertParams ((ArgValues && ArgCount) || (!ArgValues && !ArgCount));
    BOARD_AssertParams (strnlen(Str, MaxOctets) < MaxOctets);

    enum Command
    {
        Command_None = 0,
        Command_ResetPattern,
        Command_SetPattern,
        Command_OutPattern,
        Command_Tab,
        Command_FillToRow,
        Command_Newline
    };

    // Set to complement VARIANT_Base values
    enum StringStyle
    {
        StringStyle_Upper           = 0x00010000,
        StringStyle_Lower           = 0x00020000,
        StringStyle_Cap             = 0x00040000,
        StringStyle_CapWords        = 0x00080000,
        StringStyle__MASK           = 0xFFFF0000,
        StringStyle__VARIANT_MASK   = 0x0000FFFF
    };

    // Indirect or immediate parser modes do not take params; mode change 
    // happens immediately.
    enum Parser
    {
        Parser_None = 0,
        Parser_HintStrings,
        Parser_HintNumbers,
        Parser_GlobalDecimalDigits
    };

    enum Command    command         = Command_None;
    uint32_t        style           = 0;
    enum Parser     parser          = Parser_None;
    const uint32_t  RepCharOctets   = (uint32_t)strnlen (REPLACEMENT_CHAR, 4);
    uint32_t        i               = 0;
    uint32_t        currentCol      = OutColumn;
    uint32_t        tabCol          = 0;
    bool            tabMiddle       = false;
    bool            immediateCmd    = true;
    uint32_t        globalFpDigits  = 0;
    const uint8_t   * sp8           = (const uint8_t *)Str;

    struct OutProcHintParams outProcHintParams =
    {
            .origOutProc            = OutProc,
            .origOutProcParam       = OutProcParam,
            .hintNumbers            = 0,
            .hintStrings            = 0,
            .escapeAsciiSymbol      = '\\'
    };

    struct StringArgsPattern pattern;

    resetPattern (&pattern, 0);

    while (sp8[i])
    {
        const uint8_t Next = sp8[i + 1];
        if (!Next)
        {
            // Parsing a command or style sequence before reaching null:
            // str[i] corresponds to a character code that is part of an 
            // incomplete sequence.
            if (command || style || parser)
            {
                // Asserts that 'i' is zero since '`' should have already
                // output pending string segments.
                BOARD_AssertState (!i);
            }
            else 
            {
                // If no pending command, style or parser sequences, writes the
                // current string segment from 'sp[0]' to sp[i] 
                // (excluding null).
                currentCol += outUTF8Count (OutProc, OutProcParam, sp8, i + 1);        
            }
            // Stops sp iteration.
            break;
        }

        if (sp8[i] != '`' && !command && !style && !parser)
        {
            if (sp8[i] == '\n' || sp8[i] == '\r')
            {
                OutProc (OutProcParam, sp8, i + 1);
                sp8 = &sp8[i + 1];
                i = 0;
                // New line or carriage return, reset current column
                currentCol = 0;
            }
            else 
            {
                ++i;
            }
            continue;
        }

        // sp[i] == '`', Next == ?
        // Stores string segment from sp[0] to sp[i-1]
        if (i)
        {
            currentCol += outUTF8Count (OutProc, OutProcParam, sp8, i);
        }

        // Checks what is coming after the '`', '{command}', '{style}' or
        // '{parser}' character.
        if (Next >= '0' && Next <= '9') // [0-9]{1}
        {
            uint32_t marker = (uint32_t)(Next - '0');

            const char Next2 = sp8[i + 2];
            // Check char next to '[0-9]'
            if (Next2 >= '0' && Next2 <= '9') // [0-9]{1}
            {
                marker = marker * 10 + (uint32_t)(Next2 - '0');
                // Hack.. advance current string segment so next
                // segment will begin at the right offset.
                sp8 += 1;
            }

            // parser sequences are set as global settings, and `marker` acts
            // as their parameter; they do not modify their rightmost value or
            // generate messages as `command` or `style` do. 
            if (parser != Parser_None)
            {
                if (parser == Parser_HintStrings)
                {
                    outProcHintStrings (&outProcHintParams, marker);
                }
                else if (parser == Parser_HintNumbers)
                {
                    outProcHintNumbers (&outProcHintParams, marker);
                }
                else if (parser == Parser_GlobalDecimalDigits)
                {
                    globalFpDigits = marker;
                }

                parser = Parser_None;
            }
            else 
            {
                // Only replace marker with VARIANT_ToUint(ArgValues[marker]) 
                // if indirect mode for commands is enabled (indirect == 
                // !immediate) and there is a `command` that is not SetPattern.
                // In other words, immediate or indirect mode does not affect 
                // how SetPattern and anything else than commands get its 
                // parameters.
                if (!immediateCmd &&
                    command != Command_None &&
                    command != Command_SetPattern)
                {
                    BOARD_AssertAccess (marker < ArgCount);
                    marker = VARIANT_ToUint (&ArgValues[marker]);
                }

                switch (command)
                {
                    case Command_None:
                    {
                        const uint8_t * argS = (uint8_t *)REPLACEMENT_CHAR;
                        uint32_t        argO = RepCharOctets;
                        uint32_t        argC = 1;

                        if (marker < ArgCount)
                        {
                            struct VARIANT * const  ArgV = &ArgValues[marker];
                            const enum VARIANT_Base ArgVbase = ArgV->base;
                            const VARIANT_Digits    ArgVdigits = ArgV->digits; 

                            switch (ArgV->type)
                            {
                                case VARIANT_Type_Uint:
                                    // Change base to the one specified in
                                    // `style`
                                    VARIANT_ChangeBase (ArgV, 
                                            style & StringStyle__VARIANT_MASK);
                                    break;

                                case VARIANT_Type_Fp:
                                    if (globalFpDigits)
                                    {
                                        VARIANT_ChangeDigits(ArgV, 
                                            globalFpDigits);
                                    }
                                    break;

                                default:
                                    break;
                            }

                            argS = (uint8_t *)VARIANT_ToString (ArgV);
                            argO = (uint32_t)strlen ((char *)argS);
                            argC = UTF8_Count ((uint8_t *)argS, argO);
                            // Discard non-printing CSI characters
                            argC -= countCSIAsciiChars (argS, argO);

                            outProcHintNewArg (&outProcHintParams, ArgV);

                            ArgV->base      = ArgVbase;
                            ArgV->digits    = ArgVdigits;
                        }

                        // Tabulate argument if requested and viable
                        if (tabCol)
                        {
                            const int32_t Tcma = (int32_t)tabCol
                                            - ((tabMiddle)? argC >> 1 : argC);

                            if ((int32_t)currentCol < Tcma)
                            {
                                const uint32_t Chars = Tcma - currentCol;
                                outPattern (OutProc, OutProcParam, &pattern, 
                                            currentCol, Chars);
                                currentCol += Chars;
                            }
                        }

                        // Replace marker with proper argument value
                        switch (style & StringStyle__MASK)
                        {
                            case StringStyle_Upper:
                                outUpperAscii (outProcHint, &outProcHintParams,
                                               argS, argO);
                                break;

                            case StringStyle_Lower:
                                outLowerAscii (outProcHint, &outProcHintParams,
                                               argS, argO);
                                break;

                            case StringStyle_Cap:
                                outLowerCapAscii (outProcHint, 
                                                  &outProcHintParams, 
                                                  argS, argO);
                                break;

                            case StringStyle_CapWords:
                                outLowerCapWordsAscii (outProcHint,
                                                       &outProcHintParams,
                                                       argS, argO);
                                break;

                            default:
                                outProcHint (&outProcHintParams, argS, argO);
                                break;
                        };

                        currentCol += argC + outProcHintParams.extraChars;

                        style       = 0;
                        tabCol      = 0;
                        tabMiddle   = false;
                        break;
                    }

                    case Command_ResetPattern:
                        resetPattern (&pattern,
                                        marker % STRING_ARGS_PATTERN_TEMPLATES);
                        break;

                    case Command_SetPattern:
                        if (immediateCmd)
                        {
                            // advance `sp` to start looking for the pattern to
                            // set after the marker (`n) then increment i to 
                            // skip the pattern in `sp`. This works well with
                            // the hack for double-digit markers.
                            i += setPattern (&pattern, &sp8[2], marker);
                        }
                        else 
                        {
                            // capture the pattern stored in ArgValues[marker],
                            // four characters at most.
                            setPattern (&pattern, (uint8_t *)VARIANT_ToString(
                                                        &ArgValues[marker]), 4);
                        }
                        break;

                    case Command_OutPattern:
                        outPattern (OutProc, OutProcParam, &pattern, 
                                    currentCol, marker);
                        currentCol += marker;
                        break;

                    case Command_Tab:
                        tabCol = marker;
                        break;

                    case Command_FillToRow:
                        if (marker > currentCol)
                        {
                            // Fill up to 'marker' row with current pattern.
                            const uint32_t Chars = marker - currentCol;
                            outPattern (OutProc, OutProcParam, &pattern,
                                        currentCol, Chars);
                            currentCol += Chars;
                        }
                        break;

                    case Command_Newline:
                        if (marker)
                        {
                            for (uint32_t m = 0; m < marker; ++m)
                            {
                                OutProc (OutProcParam, (uint8_t *)"\r\n", 2);
                            }
                            currentCol = 0;
                        }
                        break;
                }

                command = Command_None;
            }
        }
        else if (command || style || parser)
        {
            // Expecting a number, but found another character instead while
            // executing a command, marker style or parser state option.
            // That invalidates the current character sequence.
            OutProc (OutProcParam, (uint8_t *)REPLACEMENT_CHAR, RepCharOctets);
            currentCol += RepCharOctets;

            // Cancel ongoing operation
            command     = Command_None;
            style       = 0;
            parser      = 0;
            tabCol      = 0;
            tabMiddle   = false;
        }
        else if (Next == '`')
        {
            // "``" is the escape sequence to get a single "`"
            OutProc (OutProcParam, &Next, 1);
            currentCol += 1;
        }
        else if (Next == 'o')
        {
            style = VARIANT_Base_Oct_NoSuffix;
        }
        else if (Next == 'd')
        {
            style = VARIANT_Base_Dec_NoSuffix;
        }
        else if (Next == 'x')
        {
            style = VARIANT_Base_Hex_LowerNoSuffix;
        }
        else if (Next == 'X')
        {
            style = VARIANT_Base_Hex_UpperNoSuffix;
        }
        else if (Next == 'h')
        {
            style = VARIANT_Base_Hex_LowerSuffix;
        }
        else if (Next == 'H')
        {
            style = VARIANT_Base_Hex_UpperSuffix;
        }
        else if (Next == 'U')
        {
            style = StringStyle_Upper;
        }
        else if (Next == 'l')
        {
            style = StringStyle_Lower;
        }
        else if (Next == 'c')
        {
            style = StringStyle_Cap;
        }
        else if (Next == 'w')
        {
            style = StringStyle_CapWords;
        }
        else if (Next == 'R')
        {
            command = Command_ResetPattern;
        }
        else if (Next == 'S')
        {
            command = Command_SetPattern;
        }
        else if (Next == 'P')
        {
            command = Command_OutPattern;
        }
        else if (Next == 'T')
        {
            command = Command_Tab;
        }
        else if (Next == 'M')
        {
            command = Command_Tab;
            tabMiddle = true;
        }
        else if (Next == 'F')
        {
            command = Command_FillToRow;
        }
        else if (Next == 'L')
        {
            command = Command_Newline;
        }
        else if (Next == '$')
        {
            immediateCmd = false;
        }
        else if (Next == '#')
        {
            immediateCmd = true;
        }
        else if (Next == '^')
        {
            parser = Parser_HintStrings;
        }
        else if (Next == '&')
        {
            parser = Parser_HintNumbers;
        }
        else if (Next == '.')
        {
            parser = Parser_GlobalDecimalDigits;
        }
        else
        {
            // Invalid char following '`'
            OutProc (OutProcParam, (uint8_t *)REPLACEMENT_CHAR, RepCharOctets);
            currentCol += RepCharOctets;
        }

        // Start a new string segment after the last sequence: "`?".
        // Note that if command or argBase is set, sp[0] will point to "?"
        // (the command, style or parser ) instead of the character
        // after it. That is so to parse numeric params or formatted markers
        // using the same code as regular markers.
        sp8 = &sp8[i + ((command || style || parser)? 1 : 2)];
        i = 0;
    }

    if (command == Command_Newline)
    {
        // There was a newline command before NULL (w/no marker): print a single 
        // newline before exiting.
        OutProc (OutProcParam, (uint8_t *)"\r\n", 2);
        currentCol = 0;
    }

    return currentCol;
}
