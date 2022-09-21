/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  CVT 0.92M9 1280x720 59.86 hz display modeline.

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

/*
================================================================================
Modeline determined by the cvt command
================================================================================
$ cvt -v 1280 720 60
# 1280x720 59.86 Hz (CVT 0.92M9) hsync: 44.77 kHz; pclk: 74.50 MHz
Modeline "1280x720_60.00"   74.50  1280 1344 1472 1664  720 723 728 748 -hsync 
                            +vsync
================================================================================
                       HSYNC
                    +---------+
               FP   |         |    FP    v        Visible Pixels
Line       *--------+         +----------|---------------------------------*
Pixels          64      128        192                 1280
nS             859    1718.12    2577.18              17181    (Total 22.22 uS)
================================================================================
                          VSYNC
                      +-*-*-*...*-+
                FP    |           |    BP     v     Visible Lines
           *-*-*-...*-*           *-*-*-*-*...|-*-*-*-*-*-*-*-*-*...*-*-*-*-
Lines            3          5          20               720
uS              67       111,67      446.71          16081,61  (Total 16.70 mS)
================================================================================
*/

// Active resolution/FPS/Signal standard
#define MODELINE_SIGNAL_STR     "1280x720 59.86 Hz (CVT 0.92M9)"

#define MODELINE_PCLK_KHZ       74500

#define MODELINE_HDISP          1280   // HSYNCSTART-HDISP = Front Porch
#define MODELINE_HSYNCSTART     1344   // HSYNCEND-HSYNCSTART = HSYNC pulse
#define MODELINE_HSYNCEND       1472   // HTOTAL-HSYNCEND = Back Porch
#define MODELINE_HTOTAL         1664

#define MODELINE_VDISP          720
#define MODELINE_VSYNCSTART     723
#define MODELINE_VSYNCEND       728
#define MODELINE_VTOTAL         748

#define MODELINE_HSYNC_ON       0      // -HSYNC
#define MODELINE_VSYNC_ON       1      // +VSYNC
