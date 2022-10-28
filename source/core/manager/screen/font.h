/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [SCREEN MANAGER] font drawing for utf-8 encoded character strings.

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

#include "embedul.ar/source/core/misc/rgb332.h"
#include "embedul.ar/source/core/variant.h"


// REPLACEMENT CHARACTER
// Unicode point U+FFFD (Invalid data stream byte)
#define SCREEN_FONT_REPLACEMENT_CHAR_CODEPOINT       0xFFFD


#define SCREEN_FONT_DrawParsedString(_role,_x,_y,_col,_dp,_row,_max,_str,...) \
    SCREEN_FONT_DrawParsedStringArgs (_role,_x,_y,_col,_dp,_row,_max,_str, \
                                        VARIANT_AutoParams(__VA_ARGS__))

#define SCREEN_FONT_DrawParsedStringLite(_role,_x,_y,_col,_str,...) \
    SCREEN_FONT_DrawParsedStringArgsLite (_role,_x,_y,_col,_str, \
                                        VARIANT_AutoParams(__VA_ARGS__))


struct SCREEN_FONT_AssortedGlyph
{
    const uint16_t  code;
    const uint8_t   glyph[8];
};


enum SCREEN_FONT_DP
{
    SCREEN_FONT_DP_None         = 0x00000000,
    SCREEN_FONT_DP_Shadow       = 0x00000001,
    SCREEN_FONT_DP_AlignMask    = 0x000000F0,
    SCREEN_FONT_DP_AlignHRight  = 0x00000000,
    SCREEN_FONT_DP_AlignHLeft   = 0x00000010,
    SCREEN_FONT_DP_AlingHCenter = 0x00000020
};


// Unicode blocks reference: https://unicode-table.com/en/
// The embedul.ar framework supports the basic multilingual plane only.
struct SCREEN_FONT
{
    const char * description;
    // Basic Latin Block
    // Unicode points U+0020 - U+007F
    const uint8_t (* basicLatin)[96][8];
    // Latin 1 Supplement Block
    // Unicode points U+00A0 - U+00FF
    const uint8_t (* latin1)[96][8];
    // Greek and Coptic Characters (Block Segment)
    // Unicode points U+0390 - U+03C9
    const uint8_t (* greek)[58][8];
    // Box Drawing Block
    // Unicode points U+2500 - U+257F
    const uint8_t (* boxDrawing)[128][8];
    // Block elements Block
    // Unicode points U+2580 - U+259F
    const uint8_t (* blockElements)[32][8];
    // CJK Symbols and Punctuation Block
    // Unicode points U+3000 - U+303F
    const uint8_t (* cjkSymbols)[64][8];
    // Hiragana Block
    // Unicode points U+3040 - U+309F
    const uint8_t (* hiragana)[96][8];
    // Katakana Block
    // Unicode points U+30A0 - U+30FF
    const uint8_t (* katakana)[96][8];

    // Assorted glyphs
    const uint8_t (* (*assorted)(uint16_t codepoint))[8];
    // Default glyph to use when there is no glyph available
    const uint8_t (* missing)[8];
};


typedef void (* SCREEN_FONT_SetGlyphsFunc) (struct SCREEN_FONT *const F);


enum SCREEN_Role;


void        SCREEN_FONT_Init            (struct SCREEN_FONT *const F,
                                         SCREEN_FONT_SetGlyphsFunc const
                                         SetGlyphs);
bool        SCREEN_FONT_DrawGlyph       (const enum SCREEN_Role Role,
                                         const int32_t X, const int32_t Y,
                                         const RGB332_Select ColorSel,
                                         const uint16_t Codepoint);
uint32_t    SCREEN_FONT_DrawStringSegment
                                        (const enum SCREEN_Role Role,
                                         const int32_t X, const int32_t Y,
                                         const RGB332_Select ColorSel, 
                                         const char *const Str, 
                                         const uint32_t Octets);
uint32_t    SCREEN_FONT_DrawString      (const enum SCREEN_Role Role,
                                         const int32_t X, const int32_t Y,
                                         RGB332_Select colorSel,
                                         const enum SCREEN_FONT_DP Dp,
                                         const int8_t RowHeightAdjust,
                                         const size_t MaxOctets,
                                         const char *const Str);
uint32_t    SCREEN_FONT_AutoMaxOctets   (const enum SCREEN_Role Role);
uint32_t    SCREEN_FONT_DrawStringLite  (const enum SCREEN_Role Role,
                                         const int32_t X, const int32_t Y,
                                         const RGB332_Select ColorSel, 
                                         const char *const Str);
uint32_t    SCREEN_FONT_DrawParsedStringArgs
                                        (const enum SCREEN_Role Role, 
                                         const int32_t X, int32_t Y,
                                         const RGB332_Select ColorSel,
                                         const enum SCREEN_FONT_DP Dp,
                                         const int8_t RowHeightAdjust,
                                         const size_t MaxOctets,
                                         const char *const Str,
                                         struct VARIANT *const ArgValues,
                                         const uint32_t ArgCount);
uint32_t    SCREEN_FONT_DrawParsedStringArgsLite
                                        (const enum SCREEN_Role Role,
                                         const int32_t X, const int32_t Y,
                                         const RGB332_Select ColorSel,
                                         const char *const Str,
                                         struct VARIANT *const ArgValues,
                                         const uint32_t ArgCount);
