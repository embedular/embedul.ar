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

#pragma once

#include "embedul.ar/source/core/cc.h"
#include <stdbool.h>
#include <stddef.h>


/**
 * Description
 * ===========
 *
 * A variant is a generic data type with automatic type conversion.
 * It is first assigned a value from a supported type; then queried by
 * requesting its value in the same or any other supported type.
 * Type conversion, if needed, is handled automatically. A variant
 * holds up to a 64-bit signed/unsigned integer or a double-precision
 * floating-point number. See :c:enum:`VARIANT_Type` for supported types.
 * The unsigned int type supports multiple string conversion styles by
 * choosing from different :c:enum:`VARIANT_Base` representations.
 *
 * Variants allow for arbitrary parameter count with automatic types on
 * functions through the framework. For example, see
 * :c:func:`VARIANT_ParseStringArgs` and :c:macro:`VARIANT_ParseString`.
 *
 *
 * API guide
 * =========
 *
 * Common use cases
 * ----------------
 *
 * Persistent data/named variables
 * ...............................
 *
 * A typical variable containing a value that is read or modified multiple
 * times. A :c:struct:`VARIANT` must be instantiated. Then a value can be set
 * by assignment:
 *
 * .. code-block:: c
 *
 *    struct VARIANT v = VARIANT_CreateUint (42);

 * or by taking the object instance address:
 *
 * .. code-block:: c
 *
 *    struct VARIANT v;
 *    VARIANT_SetUint (&v, 42);
 *
 * The following functions work on an existing :c:struct:`VARIANT` instance;
 * VARIANT_Create* returns an object to set by assignment, while VARIANT_Set*
 * updates the instance stored value. Both methods are equal in terms of the
 * final result.
 *
 * | :c:func:`VARIANT_CreateUint`
 * | :c:func:`VARIANT_CreateInt`
 * | :c:func:`VARIANT_CreateFp`
 * | :c:func:`VARIANT_CreatePointer`
 * | :c:func:`VARIANT_CreateBaseString`
 * | :c:func:`VARIANT_CreateString`
 * | :c:func:`VARIANT_CreateCopy`
 * | :c:func:`VARIANT_CreateBoolean`
 * | :c:func:`VARIANT_CreateBaseUint`
 * | :c:func:`VARIANT_SetUint`
 * | :c:func:`VARIANT_SetInt`
 * | :c:func:`VARIANT_SetFp`
 * | :c:func:`VARIANT_SetPointer`
 * | :c:func:`VARIANT_SetBaseString`
 * | :c:func:`VARIANT_SetString`
 * | :c:func:`VARIANT_SetCopy`
 * | :c:func:`VARIANT_SetBoolean`
 *
 * Inline/unnamed instances
 * ........................
 *
 * These :c:struct:`VARIANT` unnamed allocations are immediately destroyed
 * after use by going out-of-context.
 * Used to convenientry pass parameters to functions that take them:
 *
 * .. code-block:: c
 *
 *    :c:macro:`LOG` (NOBJ, "Hello `0", VARIANT_SpawnString("World"));
 *
 * or convert to and from supported types:
 *
 * .. code-block:: c
 *
 *    char *str = "1234";
 *    uint32_t num = VARIANT_ToUint (&VARIANT_SpawnString(str));
 *
 * The VARIANT_Spawn* macros perform inline/unnamed
 * :c:struct:`VARIANT` instantiations with explicit data type:
 *
 * | :c:macro:`VARIANT_SpawnBaseUint`
 * | :c:macro:`VARIANT_SpawnUint`
 * | :c:macro:`VARIANT_SpawnInt`
 * | :c:macro:`VARIANT_SpawnFp`
 * | :c:macro:`VARIANT_SpawnPointer`
 * | :c:macro:`VARIANT_SpawnBaseString`
 * | :c:macro:`VARIANT_SpawnString`
 * | :c:macro:`VARIANT_SpawnCopy`
 * | :c:macro:`VARIANT_SpawnBoolean`
 *
 * or by automatic parameter data type acquisition, statically detected at
 * compile-time:
 *
 * | :c:macro:`VARIANT_SpawnBaseAuto`
 * | :c:macro:`VARIANT_SpawnAuto`
 *
 * for example:
 *
 * .. code-block:: c
 *
 *    char *str = "1234";
 *    uint32_t num = VARIANT_ToUint (&VARIANT_SpawnAuto(str));
 *
 * the aforementioned "Auto" macros allow automatic instantiation of multiple
 * length, multiple type parameter lists:
 *
 * | :c:macro:`VARIANT_SpawnAutoVector`
 * | :c:macro:`VARIANT_AutoParams`
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
 * Function pointer type used in :c:func:`VARIANT_ParseStringArgs`.
 * This function pointer references a user-defined function.
 * In each call, the function should append or transmit the octets
 * provided to form a complete parsed string.
 *
 * :param Param: Parameter passed by the caller.
 * :param Data: Pointer to a const array of ``uint8_t`` elements
 *              containing available data to store.
 * :param Octets: Number of octets in the ``Data`` array.
 */
typedef void (* VARIANT_PSA_OutProc) (void *const Param,
                                      const uint8_t *const Data,
                                      const uint32_t Octets);

/**
 * Maximum length of a string conversion associated to a :c:struct:`VARIANT`, 
 * in octets. This holds the worst case, lenghtiest conversions: an unsigned
 * integer with base :c:enum:`VARIANT_Base.VARIANT_Base_Oct` converted to a
 * string, or to a string formatted as an Octal number. The constant is
 * calculated as follows: 2^64 for a 64-bit number gives 22 Octal digits, plus
 * the "o" suffix and null-termination for a maximum of 24 octets.
 */
#define VARIANT_CONV_SIZE_MAX_OCTETS    24


/**
 * Initial value type assigned to a :c:struct:`VARIANT` instance
 * by using VARIANT_Spawn\* or VARIANT_Set\* functions.
 */
enum VARIANT_Type
{
    /** :c:type:`uint64_t` 64-bit unsigned integer. */
    VARIANT_Type_Uint = 0,
    /** :c:type:`int64_t` 64-bit signed integer. */
    VARIANT_Type_Int,
    /** ``double`` double precision floating point. */
    VARIANT_Type_Fp,
    /** ``void`` memory pointer. */
    VARIANT_Type_Pointer,
    /** ``char`` string pointer. Pointed data must be static. */
    VARIANT_Type_String,
    /** ``_Bool`` boolean. */
    VARIANT_Type_Boolean
};


#define VARIANT_BASE_MASK               0x00FF
#define VARIANT_BASE_SUFFIX_MASK        0x1000
#define VARIANT_BASE_UPPER_MASK         0x0100
#define VARIANT_BASE_OCT_VALUE          0x0008
#define VARIANT_BASE_DEC_VALUE          0x000A
#define VARIANT_BASE_HEX_VALUE          0x0010


/**
 * Variant numeric bases. Used to convert an unsigned integer value to string
 * or a string to an unsigned integer.
 */
enum VARIANT_Base
{
    /**
     * Default decimal style: no suffix.
     */
    VARIANT_Base_Dec_NoSuffix = VARIANT_BASE_DEC_VALUE,
    /**
     * Octal, "o" suffix.
     */
    VARIANT_Base_Oct_Suffix = VARIANT_BASE_OCT_VALUE |
                              VARIANT_BASE_SUFFIX_MASK,
    /**
     * Octal, no suffix.
     */
    VARIANT_Base_Oct_NoSuffix = VARIANT_BASE_OCT_VALUE,
    /**
     * Hexadecimal. Lowercase digits, "h" suffix.
     */
    VARIANT_Base_Hex_LowerSuffix = VARIANT_BASE_HEX_VALUE |
                                   VARIANT_BASE_SUFFIX_MASK,
    /**
     * Hexadecimal. Lowercase digits.
     */
    VARIANT_Base_Hex_LowerNoSuffix = VARIANT_BASE_HEX_VALUE,
    /**
     * Hexadecimal. Uppercase digits, "h" suffix.
     */
    VARIANT_Base_Hex_UpperSuffix = VARIANT_BASE_HEX_VALUE | 
                                   VARIANT_BASE_SUFFIX_MASK |
                                   VARIANT_BASE_UPPER_MASK,
    /**
     * Hexadecimal. Uppercase digits.
     */
    VARIANT_Base_Hex_UpperNoSuffix = VARIANT_BASE_HEX_VALUE | 
                                     VARIANT_BASE_UPPER_MASK,
    /**
     * Default octal style: suffix.
     */
    VARIANT_Base_Oct = VARIANT_Base_Oct_Suffix,
    /**
     * Default decimal style: no suffix.
     */
    VARIANT_Base_Dec = VARIANT_Base_Dec_NoSuffix,
    /**
     * Default hexadecimal style: uppercase digits and suffix.
     */
    VARIANT_Base_Hex = VARIANT_Base_Hex_UpperSuffix
};


/**
 * The user should treat this as an opaque structure. No member should be
 * directly accessed or modified.
 */
struct VARIANT
{
    enum VARIANT_Type   type;
    // Base is ignored on integer, floating-point, boolean and pointer types.
    // On string, it is used to correctly interpret the numeric base format.
    enum VARIANT_Base   base;
    char                conv[VARIANT_CONV_SIZE_MAX_OCTETS];
    union
    {
        uint64_t        u;
        int64_t         i;
        double          d;
        const void *    p;
        const char *    s;
        _Bool           b;
    };
};


struct VARIANT_DF
{
    union
    {
        double d;
        float  f;
    } 
    df;
};


/**
 * Creates an unnamed VARIANT of type :c:enum:`VARIANT_Type.VARIANT_Type_Uint`.
 *
 * :param _b: :c:enum:`VARIANT_Base` to format the contained value when
 *            converted to string.
 * :param _u: Uint value.
 */
#define VARIANT_SpawnBaseUint(_b,_u)    (struct VARIANT[1]) { \
                                            VARIANT_CreateBaseUint(_b, _u) }[0]

/**
 * Creates an unnamed VARIANT of type :c:enum:`VARIANT_Type.VARIANT_Type_Uint`
 * with a default base :c:enum:`VARIANT_Base.VARIANT_Base_Dec`.
 *
 * :param _u: Uint value.
 */
#define VARIANT_SpawnUint(_u)           VARIANT_SpawnBaseUint( \
                                            VARIANT_Base_Dec, _u)

/**
 * Creates an unnamed VARIANT of type :c:enum:`VARIANT_Type.VARIANT_Type_Int`
 *
 * :param _i: Int value.
 */
#define VARIANT_SpawnInt(_i)            (struct VARIANT[1]) { \
                                            VARIANT_CreateInt(_i) }[0]

/**
 * Creates an unnamed VARIANT of type :c:enum:`VARIANT_Type.VARIANT_Type_Fp`
 *
 * :param _d: Floating point value.
 */
#define VARIANT_SpawnFp(_d)             (struct VARIANT[1]) { \
                                            VARIANT_CreateFp(_d) }[0]

/**
 * Creates an unnamed VARIANT of type 
 * :c:enum:`VARIANT_Type.VARIANT_Type_Pointer`
 *
 * :param _p: Pointer value.
 */
#define VARIANT_SpawnPointer(_p)        (struct VARIANT[1]) { \
                                            VARIANT_CreatePointer(_p) }[0]

/**
 * Creates an unnamed VARIANT of type
 * :c:enum:`VARIANT_Type.VARIANT_Type_String`.
 *
 * :param _b: :c:enum:`VARIANT_Base` to interpret the string when converted
 *            to a numeric type.
 * :param _s: String value.
 */                                            
#define VARIANT_SpawnBaseString(_b,_s)  (struct VARIANT[1]) { \
                                            VARIANT_CreateBaseString(_b, _s) } \
                                            [0]

/**
 * Creates an unnamed VARIANT of type
 * :c:enum:`VARIANT_Type.VARIANT_Type_String`. The string will be interpreted
 * as of base :c:enum:`VARIANT_Base.VARIANT_Base_Dec` when converted to a
 * numeric type.
 *
 * :param _s: String value.
 */                  
#define VARIANT_SpawnString(_s)         VARIANT_SpawnBaseString( \
                                            VARIANT_Base_Dec, _s)

/**
 * Creates an unnamed VARIANT of type
 * :c:enum:`VARIANT_Type.VARIANT_Type_Boolean`.
 *
 * :param _b: Boolean value.
 */     
#define VARIANT_SpawnBoolean(_b)        (struct VARIANT[1]) { \
                                            VARIANT_CreateBoolean(_b) }[0]

/**
 * Creates an unnamed exact copy of a VARIANT.
 *
 * :param _v: a valid :c:struct:`VARIANT` instance.
 */
#define VARIANT_SpawnCopy(_v)           (struct VARIANT[1]) { \
                                            VARIANT_CreateCopy(_v) }[0]


/**
 * Automatically creates an unnamed VARIANT of the specified type and base.
 *
 * :param _b: :c:enum:`VARIANT_Base` to interpret the string when converted
 *            to a numeric type, or to format the unsigned value when
 *            converted to a string.
 * :param _v: Generic unsigned or string types.
 */
#define VARIANT_SpawnBaseAuto(_b,_v) \
    _Generic((_v), \
        uint64_t        : VARIANT_SpawnBaseUint(_b, (uint64_t)(uintptr_t)_v), \
        uint32_t        : VARIANT_SpawnBaseUint(_b, (uint64_t)(uintptr_t)_v), \
        uint16_t        : VARIANT_SpawnBaseUint(_b, (uint64_t)(uintptr_t)_v), \
        uint8_t         : VARIANT_SpawnBaseUint(_b, (uint64_t)(uintptr_t)_v), \
        char *          : VARIANT_SpawnBaseString(_b, (const char *) \
                                            (uintptr_t)_v), \
        const char *    : VARIANT_SpawnBaseString(_b, (const char *) \
                                            (uintptr_t)_v), \
        const uint8_t * : VARIANT_SpawnBaseString(_b, (const char *) \
                                            (uintptr_t)_v) \
    )

/**
 * Automatically creates an unnamed VARIANT by using the specified type.
 *
 * :param _v: Any generic supported type.
 */
#define VARIANT_SpawnAuto(_v) \
    _Generic((_v), \
        uint64_t        : VARIANT_SpawnUint((uint64_t)(uintptr_t)_v), \
        uint32_t        : VARIANT_SpawnUint((uint64_t)(uintptr_t)_v), \
        uint16_t        : VARIANT_SpawnUint((uint64_t)(uintptr_t)_v), \
        uint8_t         : VARIANT_SpawnUint((uint64_t)(uintptr_t)_v), \
        int64_t         : VARIANT_SpawnInt((int64_t)(intptr_t)_v), \
        int32_t         : VARIANT_SpawnInt((int64_t)(intptr_t)_v), \
        int16_t         : VARIANT_SpawnInt((int64_t)(intptr_t)_v), \
        int8_t          : VARIANT_SpawnInt((int64_t)(intptr_t)_v), \
        double *        : VARIANT_SpawnFp(((struct VARIANT_DF *) \
                                            (uintptr_t)_v)->df.d), \
        float *         : VARIANT_SpawnFp((double)((struct VARIANT_DF *) \
                                            (uintptr_t)_v)->df.f), \
        const void *    : VARIANT_SpawnPointer((const void *)(uintptr_t)_v), \
        void *          : VARIANT_SpawnPointer((void *)(uintptr_t)_v), \
        char *          : VARIANT_SpawnString((const char *)(uintptr_t)_v), \
        const char *    : VARIANT_SpawnString((const char *)(uintptr_t)_v), \
        const uint8_t * : VARIANT_SpawnString((const char *)(uintptr_t)_v), \
        struct VARIANT *: VARIANT_SpawnCopy(((struct VARIANT *) \
                                            (uintptr_t)_v)), \
        _Bool           : VARIANT_SpawnBoolean((_Bool)(((uintptr_t)_v != \
                                            (uintptr_t)NULL)? 1u : 0u)) \
    )


#define VARIANT_ARGS_16(a,...)          VARIANT_SpawnAuto((a)), \
                                            VARIANT_ARGS_15(__VA_ARGS__)
#define VARIANT_ARGS_15(a,...)          VARIANT_SpawnAuto((a)), \
                                            VARIANT_ARGS_14(__VA_ARGS__)
#define VARIANT_ARGS_14(a,...)          VARIANT_SpawnAuto((a)), \
                                            VARIANT_ARGS_13(__VA_ARGS__)
#define VARIANT_ARGS_13(a,...)          VARIANT_SpawnAuto((a)), \
                                            VARIANT_ARGS_12(__VA_ARGS__)
#define VARIANT_ARGS_12(a,...)          VARIANT_SpawnAuto((a)), \
                                            VARIANT_ARGS_11(__VA_ARGS__)
#define VARIANT_ARGS_11(a,...)          VARIANT_SpawnAuto((a)), \
                                            VARIANT_ARGS_10(__VA_ARGS__)
#define VARIANT_ARGS_10(a,...)          VARIANT_SpawnAuto((a)), \
                                            VARIANT_ARGS_9(__VA_ARGS__)
#define VARIANT_ARGS_9(a,...)           VARIANT_SpawnAuto((a)), \
                                            VARIANT_ARGS_8(__VA_ARGS__)
#define VARIANT_ARGS_8(a,...)           VARIANT_SpawnAuto((a)), \
                                            VARIANT_ARGS_7(__VA_ARGS__)
#define VARIANT_ARGS_7(a,...)           VARIANT_SpawnAuto((a)), \
                                            VARIANT_ARGS_6(__VA_ARGS__)
#define VARIANT_ARGS_6(a,...)           VARIANT_SpawnAuto((a)), \
                                            VARIANT_ARGS_5(__VA_ARGS__)
#define VARIANT_ARGS_5(a,...)           VARIANT_SpawnAuto((a)), \
                                            VARIANT_ARGS_4(__VA_ARGS__)
#define VARIANT_ARGS_4(a,...)           VARIANT_SpawnAuto((a)), \
                                            VARIANT_ARGS_3(__VA_ARGS__)
#define VARIANT_ARGS_3(a,...)           VARIANT_SpawnAuto((a)), \
                                            VARIANT_ARGS_2(__VA_ARGS__)
#define VARIANT_ARGS_2(a,...)           VARIANT_SpawnAuto((a)), \
                                            VARIANT_ARGS_1(__VA_ARGS__)
#define VARIANT_ARGS_1(a)               VARIANT_SpawnAuto((a))
#define VARIANT_ARGS_0(a)               NULL

#define VARIANT_EN_16(n)                ((struct VARIANT[]){ n })
#define VARIANT_EN_15(n)                ((struct VARIANT[]){ n })
#define VARIANT_EN_14(n)                ((struct VARIANT[]){ n })
#define VARIANT_EN_13(n)                ((struct VARIANT[]){ n })
#define VARIANT_EN_12(n)                ((struct VARIANT[]){ n })
#define VARIANT_EN_11(n)                ((struct VARIANT[]){ n })
#define VARIANT_EN_10(n)                ((struct VARIANT[]){ n })
#define VARIANT_EN_9(n)                 ((struct VARIANT[]){ n })
#define VARIANT_EN_8(n)                 ((struct VARIANT[]){ n })
#define VARIANT_EN_7(n)                 ((struct VARIANT[]){ n })
#define VARIANT_EN_6(n)                 ((struct VARIANT[]){ n })
#define VARIANT_EN_5(n)                 ((struct VARIANT[]){ n })
#define VARIANT_EN_4(n)                 ((struct VARIANT[]){ n })
#define VARIANT_EN_3(n)                 ((struct VARIANT[]){ n })
#define VARIANT_EN_2(n)                 ((struct VARIANT[]){ n })
#define VARIANT_EN_1(n)                 ((struct VARIANT[]){ n })
#define VARIANT_EN_0(n)                 n


/**
 * Automatically creates an unnamed VARIANT array consisting of automatically
 * created unnamed VARIANTs from a list of variable-length macro parameters.
 *
 * :param ...: Up to 16 parameters consisting of generic supported types.
 */
#define VARIANT_SpawnAutoVector(...) \
    CC_ExpPaste(VARIANT_EN_,CC_ArgsCount(__VA_ARGS__))( \
        CC_ExpPaste(VARIANT_ARGS_,CC_ArgsCount(__VA_ARGS__))(__VA_ARGS__))

/**
 * Takes a list of variable-length macro parameters and automatically creates
 * the following two elements, separated by a single comma:
 *
 * - An unnamed VARIANT array consisting of automatically created 
 *   unnamed VARIANTs from a list of variable-length macro parameters.
 * - Array element count, as literal.
 *
 * The resulting macro-expanded parameters are convenient to call functions 
 * (whose names usually end in "Args") that take those as a mechanism of 
 * variable-length, typed parameters in "C".
 *
 * For example, :c:macro:`VARIANT_ParseString` properly calls
 * :c:func:`VARIANT_ParseStringArgs` by constructing its required array and 
 * array length parameters from ``__VA_ARGS__``.
 *
 * :param ...: Up to 16 parameters consisting of generic supported types.
 */
#define VARIANT_AutoParams(...) \
    VARIANT_SpawnAutoVector(__VA_ARGS__), CC_ArgsCount(__VA_ARGS__)

/**
 * Automatically instances the parameters list and parameter count required to
 * call :c:func:`VARIANT_ParseStringArgs`.
 *
 * :param _oc: ``OutColumn``, see :c:func:`VARIANT_ParseStringArgs`.
 * :param _mx: ``MaxOctets``, see :c:func:`VARIANT_ParseStringArgs`.
 * :param _s: ``Str``, see :c:func:`VARIANT_ParseStringArgs`.
 * :param _op: ``OutProc``, see :c:func:`VARIANT_ParseStringArgs`.
 * :param _opp: ``OutProcParam``, see :c:func:`VARIANT_ParseStringArgs`.
 * :param ...: Up to 16 parameters consisting of generic supported types.
 * :return: see :c:func:`VARIANT_ParseStringArgs`.
 */
#define VARIANT_ParseString(_oc,_mx,_s,_op,_opp,...) \
    VARIANT_ParseStringArgs (_oc,_mx,_s,_op,_opp, \
                             VARIANT_AutoParams(__VA_ARGS__))


struct VARIANT  VARIANT_CreateUint          (const uint64_t Uint);
struct VARIANT  VARIANT_CreateInt           (const int64_t Int);
struct VARIANT  VARIANT_CreateFp            (const double Fp);
struct VARIANT  VARIANT_CreatePointer       (const void *const Pointer);
struct VARIANT  VARIANT_CreateBaseString    (const enum VARIANT_Base Base,
                                             const char *const String);
struct VARIANT  VARIANT_CreateString        (const char *const String);
struct VARIANT  VARIANT_CreateCopy          (const struct VARIANT *const V);
struct VARIANT  VARIANT_CreateBoolean       (const _Bool Boolean);
struct VARIANT  VARIANT_CreateBaseUint      (const enum VARIANT_Base Base,
                                             const uint64_t Uint);
void            VARIANT_SetBaseUint         (struct VARIANT *const V,
                                             const enum VARIANT_Base base,
                                             const uint64_t Value);
void            VARIANT_SetUint             (struct VARIANT *const V,
                                             const uint64_t Value);
void            VARIANT_SetInt              (struct VARIANT *const V,
                                             const int64_t Value);
void            VARIANT_SetFp               (struct VARIANT *const V,
                                             const double Value);
void            VARIANT_SetPointer          (struct VARIANT *const V, 
                                             const void *const Ptr);
void            VARIANT_SetBaseString       (struct VARIANT *const V,
                                             const enum VARIANT_Base Base,
                                             const char *const Str);
void            VARIANT_SetString           (struct VARIANT *const V,
                                             const char *const Str);
void            VARIANT_SetCopy             (struct VARIANT *const V,
                                             struct VARIANT *const A);
void            VARIANT_SetBoolean          (struct VARIANT *const V,
                                             const _Bool Value);
uint64_t        VARIANT_ToUint              (struct VARIANT *const V);
int64_t         VARIANT_ToInt               (struct VARIANT *const V);
double          VARIANT_ToDouble            (struct VARIANT *const V);
const char *    VARIANT_ToString            (struct VARIANT *const V);
void            VARIANT_ToStringBuffer      (struct VARIANT *const V,
                                             char *const Buffer,
                                             const uint32_t Size);
_Bool           VARIANT_ToBoolean           (struct VARIANT *const V);
enum VARIANT_Type
                VARIANT_ActualType          (struct VARIANT *const V);
enum VARIANT_Base
                VARIANT_CurrentBase         (struct VARIANT *const V);
enum VARIANT_Base   
                VARIANT_ChangeBaseUint      (struct VARIANT *const V,
                                             const enum VARIANT_Base Base);
uint32_t        VARIANT_StringOctetCount    (struct VARIANT *const V);
bool            VARIANT_IsEqual             (struct VARIANT *const V,
                                             struct VARIANT *const A);
bool            VARIANT_IsEqualAsUint       (struct VARIANT *const V,
                                             struct VARIANT *const A);
uint32_t        VARIANT_ParseStringArgs     (const uint32_t OutColumn,
                                             const size_t MaxOctets,
                                             const char *const Str,
                                             VARIANT_PSA_OutProc const OutProc,
                                             void *const OutProcParam,
                                             struct VARIANT *const ArgValues,
                                             const uint32_t ArgCount);
