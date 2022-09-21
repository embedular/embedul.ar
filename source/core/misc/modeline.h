/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [VIDEO subsystem] display modeline macros.

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


#define MODELINE_HSYNC_OFF          (! MODELINE_HSYNC_ON)
#define MODELINE_VSYNC_OFF          (! MODELINE_VSYNC_ON)

#define MODELINE_HSYNC_HZ           ((MODELINE_PCLK_KHZ * 1000) \
                                        / MODELINE_HTOTAL)

#define MODELINE_FRONT_PORCH        (MODELINE_HSYNCSTART - MODELINE_HDISP)
#define MODELINE_HSYNC              (MODELINE_HSYNCEND - MODELINE_HSYNCSTART)
#define MODELINE_BACK_PORCH         (MODELINE_HTOTAL - MODELINE_HSYNCEND)


#define MODELINE_SYNC_1             +   // HIGH
#define MODELINE_SYNC_0             -   // LOW
#define MODELINE_SYNC_EXP(x)        CC_Paste(MODELINE_SYNC_,x)
#define MODELINE_SYNC(x)            MODELINE_SYNC_EXP(x)

#define MODELINE_STRINGIZE_H(pc,h,hss,hse,ht) \
                CC_Str(pc) "  " \
                CC_Str(h) " " \
                CC_Str(hss) " " \
                CC_Str(hse) " " \
                CC_Str(ht)

#define MODELINE_STRINGIZE_V(v,vss,vse,vt,hs,vs) \
                CC_Str(v) " " \
                CC_Str(vss) " " \
                CC_Str(vse) " " \
                CC_Str(vt) " " \
                CC_Str(hs) "hsync " \
                CC_Str(vs) "vsync"
                
#define MODELINE_STRINGIZE(pc,h,hss,hse,ht,v,vss,vse,vt,hs,vs) \
                MODELINE_STRINGIZE_H(pc,h,hss,hse,ht) "  " \
                MODELINE_STRINGIZE_V(v,vss,vse,vt,hs,vs)

#define LANG_MODELINE \
                MODELINE_STRINGIZE( \
                    MODELINE_PCLK_KHZ, \
                    MODELINE_HDISP, \
                    MODELINE_HSYNCSTART, \
                    MODELINE_HSYNCEND, \
                    MODELINE_HTOTAL, \
                    MODELINE_VDISP, \
                    MODELINE_VSYNCSTART, \
                    MODELINE_VSYNCEND, \
                    MODELINE_VTOTAL, \
                    MODELINE_SYNC(MODELINE_HSYNC_ON), \
                    MODELINE_SYNC(MODELINE_VSYNC_ON))
