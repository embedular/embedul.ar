/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [IO driver] huewheel logic device.

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

#include "embedul.ar/source/core/device/io.h"
#include "embedul.ar/source/core/anim.h"


#define IO_HUEWHEEL_PORT_COUNT          1


struct HUEWHEEL_PolarPlacement
{
    const uint8_t       OutDriverCodeRed;
    const uint8_t       OutDriverCodeGreen;
    const uint8_t       OutDriverCodeBlue;
    const uint8_t       PolarPlacement;
};


enum HUEWHEEL_Dir
{
    HUEWHEEL_Dir_Clockwise = 0,
    HUEWHEEL_Dir_CounterCW
};


enum IO_HUEWHEEL_INB
{
    IO_HUEWHEEL_INB__COUNT
};


enum IO_HUEWHEEL_INR
{
    IO_HUEWHEEL_INR__COUNT
};


enum IO_HUEWHEEL_OUTB
{
    IO_HUEWHEEL_OUTB_Dir = 0,
    IO_HUEWHEEL_OUTB__COUNT
};


enum IO_HUEWHEEL_OUTR
{
    IO_HUEWHEEL_OUTR_Step = 0,
    IO_HUEWHEEL_OUTR_MaxLuminance,
    IO_HUEWHEEL_OUTR_Phase,
    IO_HUEWHEEL_OUTR_Duration,
    IO_HUEWHEEL_OUTR__COUNT
};


struct IO_HUEWHEEL
{
    struct IO               device;
    struct IO_PortInfo      portInfo[IO_HUEWHEEL_PORT_COUNT];
    struct IO_Gateway       outGateway;
    const struct HUEWHEEL_PolarPlacement
                            * polarPlacement;
    uint32_t                polarPlacementElements;
    // 8.8
    uint16_t                currentOffset;
    uint16_t                step;
    enum HUEWHEEL_Dir       dir;
    struct ANIM             globalSaturation;
    struct ANIM             globalLuminance;    
    uint32_t                flashPhase;
    uint8_t                 flashMaxLuminance;
};


void IO_HUEWHEEL_Init   (struct IO_HUEWHEEL *const H,
                         struct IO_Gateway OutGateway,
                         const struct HUEWHEEL_PolarPlacement
                         *const PolarPlacement,
                         const uint32_t PolarPlacementElements);
void IO_HUEWHEEL_Attach (struct IO_HUEWHEEL *const H);
