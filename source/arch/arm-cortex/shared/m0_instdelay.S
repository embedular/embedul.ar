#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# ARM Cortex M0 delay by instructions.
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

.global M0_InstDelay

@ ------------------------------------------------------------------------------
@ Function parameters
@
@ r0 = Number of instructions to execute before return. Minimun delay is 18
@ instructions.
@ ------------------------------------------------------------------------------
@ Purpose
@
@ Perform a precise, busy delay based on instruction count running on a 
@ Cortex-M0 core.
@ ------------------------------------------------------------------------------
.thumb_func
    M0_InstDelay:
            push {lr}           @ +1+1  +2
            nop                 @ +1    +3  Accounting for subs when r0 < 4.
            nop                 @ +1    +4  Accounting for a branch not taken
                                @           when r0 < 4.
            nop                 @ +1        Accounting for the two-instructions
            movs r1, #3         @ +1    +6  pop below.
            ands r1, r0, r1     @ +1    +7
            cmp  r1, #3         @ +1    +8
            beq  .b3            @ +1    +9  +11  .b3 branch taken
            cmp  r1, #1         @ +1    +10
            beq  .b1            @ +1    +11     +13  .b1 branch taken
            cmp  r1, #0         @ +1    +12
            beq  .b0            @ +1    +13         +15  .b0 branch taken
        .b2:
            subs r0, #18        @ +1    +14
            nop                 @ +1    +15
            b    .bLoop         @ +3    +18
        .b3:
            subs r0, #15        @ +1        +12
            b    .bLoop         @ +3        +15
        .b1:
            subs r0, #17        @ +1            +14
            b    .bLoop         @ +3            +17
        .b0:
            subs r0, #16        @ +1                +16
                                @       r0 adjusted to be multiple of 4.
        .bLoop:
            subs r0, #4         @ +1    Accounting for one loop iteration.
            bge  .bLoop         @ +3    Branch taken if r0 >= 4.
        .bLoopEnd:              @ +1    Branch not taken if r0 < 4.
            pop  {pc}           @ +1+1
@ ------------------------------------------------------------------------------
