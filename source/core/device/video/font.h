/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [VIDEO subsystem] font drawing for utf-8 encoded character strings.

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

#include "embedul.ar/source/core/device/video/rgb332.h"
#include "embedul.ar/source/core/variant.h"


// REPLACEMENT CHARACTER
// Unicode point U+FFFD (Invalid data stream byte)
#define VIDEO_FONT_REPLACEMENT_CHAR_CODEPOINT       0xFFFD


#define VIDEO_FONT_DrawParsedString(_f,_x,_y,_c,_d,_r,_m,_s,...) \
    VIDEO_FONT_DrawParsedStringArgs (_f,_x,_y,_c,_d,_r,_m,_s, \
                                        VARIANT_AutoParams(__VA_ARGS__))

#define VIDEO_FONT_DrawParsedStringLite(_x,_y,_c,_s,...) \
    VIDEO_FONT_DrawParsedStringArgsLite (_x,_y,_c,_s, \
                                            VARIANT_AutoParams(__VA_ARGS__))


struct VIDEO_FONT_AssortedGlyph
{
    const uint16_t  code;
    const uint8_t   glyph[8];
};


enum VIDEO_FONT_DP
{
    VIDEO_FONT_DP_None          = 0x00000000,
    VIDEO_FONT_DP_Shadow        = 0x00000001,
    VIDEO_FONT_DP_AlignMask     = 0x000000F0,
    VIDEO_FONT_DP_AlignHRight   = 0x00000000,
    VIDEO_FONT_DP_AlignHLeft    = 0x00000010,
    VIDEO_FONT_DP_AlingHCenter  = 0x00000020
};


// Unicode blocks reference: https://unicode-table.com/en/
// RETRO-CIAA supports the basic multilingual plane only.
struct VIDEO_FONT
{
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


typedef void (* VIDEO_FONT_SetGlyphsFunc) (struct VIDEO_FONT *const F);


void            VIDEO_FONT_Init             (struct VIDEO_FONT *const F,
                                             VIDEO_FONT_SetGlyphsFunc const 
                                             SetGlyphs);
uint32_t        VIDEO_FONT_CountGlyphsMax   (const char *const Str, 
                                             const size_t MaxOctets);
uint32_t        VIDEO_FONT_CountGlyphs      (const char *const Str, 
                                             const uint32_t Octets);
const uint8_t * VIDEO_FONT_SelectGlyph      (const struct VIDEO_FONT *const F, 
                                             const uint16_t Codepoint);
bool            VIDEO_FONT_DrawGlyph        (const struct VIDEO_FONT *const F,
                                             const int32_t X, const int32_t Y,
                                             const VIDEO_RGB332_Select ColorSel,
                                             const uint16_t Codepoint);
bool            VIDEO_FONT_DrawGlyphLite    (const int32_t X, const int32_t Y,
                                             const VIDEO_RGB332_Select ColorSel,
                                             const uint16_t Codepoint);
uint32_t        VIDEO_FONT_DrawStringSegment
                                            (const struct VIDEO_FONT *const F, 
                                             const int32_t X, const int32_t Y,
                                             const VIDEO_RGB332_Select ColorSel, 
                                             const char *const Str, 
                                             const uint32_t Octets);
uint32_t        VIDEO_FONT_DrawString       (const struct VIDEO_FONT *const F,
                                             const int32_t X, const int32_t Y,
                                             VIDEO_RGB332_Select colorSel,
                                             const enum VIDEO_FONT_DP Dp,
                                             int8_t rowHeightAdjust,
                                             const size_t MaxOctets,
                                             const char *const Str);
uint32_t        VIDEO_FONT_AutoMaxOctets    (void);
uint32_t        VIDEO_FONT_DrawStringLite   (const int32_t X, const int32_t Y,
                                             const VIDEO_RGB332_Select ColorSel, 
                                             const char *const Str);
uint32_t        VIDEO_FONT_DrawParsedStringArgs
                                            (const struct VIDEO_FONT *const F, 
                                             const int32_t X, int32_t Y,
                                             const VIDEO_RGB332_Select ColorSel,
                                             const enum VIDEO_FONT_DP Dp,
                                             const int8_t RowHeightAdjust,
                                             const size_t MaxOctets,
                                             const char *const Str,
                                             struct VARIANT *const ArgValues,
                                             const uint32_t ArgCount);
uint32_t        VIDEO_FONT_DrawParsedStringArgsLite
                                            (const int32_t X, const int32_t Y,
                                             const VIDEO_RGB332_Select ColorSel,
                                             const char *const Str,
                                             struct VARIANT *const ArgValues,
                                             const uint32_t ArgCount);
