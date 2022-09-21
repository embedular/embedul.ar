/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [CORE] compiler attributes and utilities.

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


#define CC_Paste(x,y)       x ## y
#define CC_ExpPaste(x,y)    CC_Paste(x,y)
#define CC_Str(x)           #x
#define CC_Exp(x)           x
#define CC_ExpStr(x)        CC_Str(x)
// https://renenyffenegger.ch/notes/development/languages/C-C-plus-plus/
// preprocessor/macros/__VA_ARGS__/count-arguments

#define CC_18thElement(a1, a2, a3, a4, a5, a6, a7, a8, \
                       a9, a10, a11, a12, a13, a14, a15, a16, \
                       a17, a18, ...) a18
#define CC_ArgsCount(...)   CC_18thElement(dummy __VA_OPT__(,) __VA_ARGS__, \
                                    16, 15, 14, 13, 12, 11, 10, 9, \
                                    8, 7, 6, 5, 4, 3, 2, 1, \
                                    0)

// -----------------------------------------------------------------------------
// The corresponding compiler makefile defines the following macros
// -----------------------------------------------------------------------------
#ifndef CC_NoOptimization
    #error [C compiler] Undefined no optimization directive
#endif

#ifndef CC_Fallthrough
    #error [C compiler] Undefined case fallthrough directive
#endif

#ifndef CC_Packed
    #error [C compiler] Undefined packed directive
#endif

#ifndef CC_Weak
    #error [C compiler] Undefined weak directive
#endif

#ifndef CC_Section
    #error [C compiler] Undefined section
#endif

#ifndef AS_Section
    #error [Assembly] Undefined section directive
#endif

#ifndef CC_CompilerStr
    #error Undefined compiler version
#endif

#ifndef CC_OptLevelStr
    #error Undefined compiler optimization level
#endif

#ifndef CC_DateStr
    #error Undefined date
#endif

#ifndef CC_VcsAppVersionStr
    #error Undefined application version
#endif

#ifndef CC_VcsFwkVersionStr
    #error Undefined framework version
#endif
// -----------------------------------------------------------------------------

#ifndef CC_AppNameStr
    #error Undefined application name
#endif

#ifdef DEBUG
    #define CC_BuildInfoDebugStrSpaced    " (DEBUG)"
#else
    #define CC_BuildInfoDebugStrSpaced
#endif

#define CC_BuildInfoStr \
    CC_DateStr " " CC_CompilerStr " " CC_OptLevelStr \
    CC_BuildInfoDebugStrSpaced

#define CC_EnumForce32(s)           s ## __FORCE32 = INT32_MAX


uint32_t    CC_RoundTo4     (const uint32_t Size);
uint64_t    CC_RoundTo4_64  (const uint64_t Size);
uint32_t    CC_RoundTo8     (const uint32_t Size);
uint64_t    CC_RoundTo8_64  (const uint64_t Size);
