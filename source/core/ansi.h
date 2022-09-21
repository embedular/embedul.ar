/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [CORE] "ansi" ecma-48 terminal control.

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

// "ANSI" escape codes
// https://en.wikipedia.org/wiki/ANSI_escape_code
// CSI stands for "Control Sequence Introducer" defined by \033[ or U+009B
#define ANSI_CSI                        "\033["     // ESC + [
#define ANSI_CSI_CLEAR_SCREEN           ANSI_CSI "2J"
#define ANSI_CSI_CURSOR_POS(row,col)    ANSI_CSI #row ";" #col "H"
#define ANSI_CSI_CURSOR_UP(n)           ANSI_CSI #n "A"
#define ANSI_CSI_CURSOR_DOWN(n)         ANSI_CSI #n "B"
#define ANSI_CSI_CURSOR_RIGHT(n)        ANSI_CSI #n "C"
#define ANSI_CSI_CURSOR_LEFT(n)         ANSI_CSI #n "D"
#define ANSI_CSI_CURSOR_SAVE            ANSI_CSI "s"
#define ANSI_CSI_CURSOR_RESTORE         ANSI_CSI "u"

#define ANSI_SGR_SET1(a)                ANSI_CSI CC_Str(a) "m"
#define ANSI_SGR_SET2(a,b)              ANSI_CSI CC_Str(a) ";" CC_Str(b) "m"
#define ANSI_SGR_SET3(a,b,c)            ANSI_CSI CC_Str(a) ";" CC_Str(b) ";" \
                                                 CC_Str(c) "m"
#define ANSI_SGR_SET4(a,b,c,d)          ANSI_CSI CC_Str(a) ";" CC_Str(b) ";" \
                                                 CC_Str(c) ";" CC_Str(d) "m"
#define ANSI_SGR_RESET                  ANSI_SGR_SET1(0)
#define ANSI_SGR_SET_FG_RGB(r,g,b)      ANSI_CSI "38;2;" CC_Str(r) ";" \
                                                         CC_Str(g) ";" \
                                                         CC_Str(b) "m"
#define ANSI_SGR_SET_BG_RGB(r,g,b)      ANSI_CSI "48;2;" CC_Str(r) ";" \
                                                         CC_Str(g) ";" \
                                                         CC_Str(b) "m"
#define ANSI_SGR_SET_FG(b,f)            ANSI_SGR_SET3(0, \
                                                      ANSI_SGR_ ## b, \
                                                      ANSI_SGR_FG_ ## f)
#define ANSI_SGR_SET_FG_BG(b,f,g)       ANSI_SGR_SET4(0, \
                                                      ANSI_SGR_ ## b, \
                                                      ANSI_SGR_FG_ ## f, \
                                                      ANSI_SGR_BG_ ## g)
#define ANSI_SGR_SET_NFG(f)             ANSI_SGR_SET_FG(NORMAL, f)
#define ANSI_SGR_SET_NFG_BG(f,g)        ANSI_SGR_SET_NFG_BG(NORMAL,f,g)
#define ANSI_SGR_SET_BFG(f)             ANSI_SGR_SET_FG(BOLD, f)
#define ANSI_SGR_SET_BFG_BG(f,g)        ANSI_SGR_SET_NFG_BG(BOLD,f,g)

// SGR Normal/Reset and Bold commands
#define ANSI_SGR_NORMAL                 0
#define ANSI_SGR_BOLD                   1

// SGR Foreground color codes 30-37, (bright, non-standard) 90-97.
#define ANSI_SGR_FG_BLACK               30
#define ANSI_SGR_FG_RED                 31
#define ANSI_SGR_FG_GREEN               32
#define ANSI_SGR_FG_BROWN               33
#define ANSI_SGR_FG_BLUE                34
#define ANSI_SGR_FG_MAGENTA             35
#define ANSI_SGR_FG_CYAN                36
#define ANSI_SGR_FG_WHITE               37  // Mostly gray
#define ANSI_SGR_FG_BRIGHT_BLACK        90  // Dark gray
#define ANSI_SGR_FG_BRIGHT_RED          91
#define ANSI_SGR_FG_BRIGHT_GREEN        92
#define ANSI_SGR_FG_BRIGHT_YELLOW       93
#define ANSI_SGR_FG_BRIGHT_BLUE         94
#define ANSI_SGR_FG_BRIGHT_PURPLE       95
#define ANSI_SGR_FG_BRIGHT_CYAN         96
#define ANSI_SGR_FG_BRIGHT_WHITE        97  // Mostly real white

// SGR Background colors codes 40-47
#define ANSI_SGR_BG_BLACK               40
#define ANSI_SGR_BG_RED                 41
#define ANSI_SGR_BG_GREEN               42
#define ANSI_SGR_BG_BROWN               43
#define ANSI_SGR_BG_BLUE                44
#define ANSI_SGR_BG_MAGENTA             45
#define ANSI_SGR_BG_CYAN                46
#define ANSI_SGR_BG_WHITE               47  // Mostly gray
