#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# 256-pixel line drawing for software video adapter.
#
# Copyright 2018-2022 Santiago Germino
# <sgermino@embedul.ar> https://www.linkedin.com/in/royconejo
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
#

.syntax unified
.thumb
.text

.global VIDEO_ADAPTER_LineOut

@ ------------------------------------------------------------------------------
@ Function parameters
@ ===================
@   r0 = Framebuffer memory address to a line of 256 RGB332 pixels.
@   r1 = AND value (fade to black)
@   r2 = OR value (fade to white)
@
@ Additional register usage
@ =========================
@   r3 = GPIO mpin address
@   r4 = Interations left
@   r5 = Output pixel (RGB332)
@   r6 = Framebuffer to GPIO bit arrangement
@
@ Purpose
@ =======
@   Output framebuffer pixels (8-bit values) to their corresponding GPIO.
@   To comply with line timing requirements, pixels are processed in groups of
@   eight pixels following a repeating sequence of 13 and 14 MCU instructions
@   for each one.
@
@   Instruction count: Setup(9) + APSetup(7) +
@                      Active pixels(32×5×14+32×3×13-APSetup(7)=3481)
@                      + Cleanup(8) = 3505.
@   Instruction period: 1 / MCUfreq(204000000 Hz) = 4.901960784 ns.
@   Function duration: 4.901960784 ns * 3505 = 17181,37 ns
@   Visible Pixels duration: 4.901960784 ns * 3481 = 17063,72 ns
@
@ Platform specific
@ =================
@   Target board hardwre has GPIO[7] unavailabe. To overcome this constrain,
@   a mask is applied to the GPIO to left GPIO[7] unmodified, and the 7th
@   framebuffer pixel bit is shifted to GPIO[8] as follows:
@
@         Pin/Bit                      8  7  6  5  4  3  2  1  0
@         Framebuffer pixel[7-0]      -- R2 R1 R0 G2 G1 G0 B1 B0
@         Pixel output GPIO[8,6-0]    R2 -- R1 R0 G2 G1 G0 B1 B0
@ ------------------------------------------------------------------------------
.thumb_func
    VIDEO_ADAPTER_LineOut:
        push {r4-r6,lr}                 @ +1+4
        ldr  r3, =VIDEO_ADAPTER_GPIO_MPIN_ADDR
                                        @ +2
        ldr  r4, =32                    @ +2        Eight pixels per iteration
    .loadPixel:
        @ Pixel 1/8
        @ ----------------------------------------------------------------------
        @ On first iteration, Active pixels Setup (APSetup) (*).
        @ ----------------------------------------------------------------------
        ldrb r5, [r0, #0]               @ +2         Pixel load
        ands r5, r5, r1                 @ +1
        orrs r5, r5, r2                 @ +1
        lsrs r6, r5, #7                 @ +1         Rxxxxxxx  ->  0000000R
        lsls r6, #8                     @ +1         0000000R00000000
        orrs r5, r6                     @ +1         0000000RxRRGGGBB
        @ ----------------------------------------------------------------------
        @ On first iteration, Horizontal Back Porch ends here (*).
        @ ----------------------------------------------------------------------
        str  r5, [r3]                   @ +2         Pixel to GPIO
        adds r0, #1                     @ +1         Buffer pointer increment
        nop                             @ +1
        nop                             @ +1
        nop                             @ +1 (13)
        @ Pixel 2/8
        ldrb r5, [r0, #0]               @ +2         Pixel load
        ands r5, r5, r1                 @ +1
        orrs r5, r5, r2                 @ +1
        lsrs r6, r5, #7                 @ +1         Rxxxxxxx  ->  0000000R
        lsls r6, #8                     @ +1         0000000R00000000
        orrs r5, r6                     @ +1         0000000RxRRGGGBB
        str  r5, [r3]                   @ +2         Pixel to GPIO
        adds r0, #1                     @ +1         Buffer pointer increment
        nop                             @ +1
        nop                             @ +1
        nop                             @ +1
        nop                             @ +1 (14)
        @ Pixel 3/8
        ldrb r5, [r0, #0]               @ +2         Pixel load
        ands r5, r5, r1                 @ +1
        orrs r5, r5, r2                 @ +1
        lsrs r6, r5, #7                 @ +1         Rxxxxxxx  ->  0000000R
        lsls r6, #8                     @ +1         0000000R00000000
        orrs r5, r6                     @ +1         0000000RxRRGGGBB
        str  r5, [r3]                   @ +2         Pixel to GPIO
        adds r0, #1                     @ +1         Buffer pointer increment
        nop                             @ +1
        nop                             @ +1
        nop                             @ +1
        nop                             @ +1 (14)
        @ Pixel 4/8
        ldrb r5, [r0, #0]               @ +2         Pixel load
        ands r5, r5, r1                 @ +1
        orrs r5, r5, r2                 @ +1
        lsrs r6, r5, #7                 @ +1         Rxxxxxxx  ->  0000000R
        lsls r6, #8                     @ +1         0000000R00000000
        orrs r5, r6                     @ +1         0000000RxRRGGGBB
        str  r5, [r3]                   @ +2         Pixel to GPIO
        adds r0, #1                     @ +1         Buffer pointer increment
        nop                             @ +1
        nop                             @ +1
        nop                             @ +1 (13)
        @ Pixel 5/8
        ldrb r5, [r0, #0]               @ +2         Pixel load
        ands r5, r5, r1                 @ +1
        orrs r5, r5, r2                 @ +1
        lsrs r6, r5, #7                 @ +1         Rxxxxxxx  ->  0000000R
        lsls r6, #8                     @ +1         0000000R00000000
        orrs r5, r6                     @ +1         0000000RxRRGGGBB
        str  r5, [r3]                   @ +2         Pixel to GPIO
        adds r0, #1                     @ +1         Buffer pointer increment
        nop                             @ +1
        nop                             @ +1
        nop                             @ +1
        nop                             @ +1 (14)
        @ Pixel 6/8
        ldrb r5, [r0, #0]               @ +2         Pixel load
        ands r5, r5, r1                 @ +1
        orrs r5, r5, r2                 @ +1
        lsrs r6, r5, #7                 @ +1         Rxxxxxxx  ->  0000000R
        lsls r6, #8                     @ +1         0000000R00000000
        orrs r5, r6                     @ +1         0000000RxRRGGGBB
        str  r5, [r3]                   @ +2         Pixel to GPIO
        adds r0, #1                     @ +1         Buffer pointer increment
        nop                             @ +1
        nop                             @ +1
        nop                             @ +1
        nop                             @ +1 (14)
        @ Pixel 7/8
        ldrb r5, [r0, #0]               @ +2         Pixel load
        ands r5, r5, r1                 @ +1
        orrs r5, r5, r2                 @ +1
        lsrs r6, r5, #7                 @ +1         Rxxxxxxx  ->  0000000R
        lsls r6, #8                     @ +1         0000000R00000000
        orrs r5, r6                     @ +1         0000000RxRRGGGBB
        str  r5, [r3]                   @ +2         Pixel to GPIO
        adds r0, #1                     @ +1         Buffer pointer increment
        nop                             @ +1
        nop                             @ +1
        nop                             @ +1 (13)
        @ Pixel 8/8
        ldrb r5, [r0, #0]               @ +2         Pixel load
        ands r5, r5, r1                 @ +1
        orrs r5, r5, r2                 @ +1
        lsrs r6, r5, #7                 @ +1         Rxxxxxxx  ->  0000000R
        lsls r6, #8                     @ +1         0000000R00000000
        orrs r5, r6                     @ +1         0000000RxRRGGGBB
        str  r5, [r3]                   @ +2         Pixel to GPIO
        adds r0, #1                     @ +1         Buffer pointer increment
        subs r4, #1                     @ +1         One less iteration
        bne  .loadPixel                 @ +3 (14)    Process new group of pixels
    .lineEnd:                           @ +1 (branch not taken)
        @ GPIO to zero; pixels inside blanking interval must be black.
        str  r4, [r3]                   @ +2
        @ ----------------------------------------------------------------------
        @ Horizontal Front Porch starts here (*).
        @ ----------------------------------------------------------------------
        pop  {r4-r6,pc}                 @ +1+4
        @ ----------------------------------------------------------------------
        @ (*): Only when DE signal is autogenerated by the TFP410 DVI
        @      transceiver. Otherwise, visible pixels starts and ends on
        @      VIDEO_DE(ON/OFF) and the entire function duration (and function
        @      call time) are part of the visible pixel period.
        @ ----------------------------------------------------------------------
