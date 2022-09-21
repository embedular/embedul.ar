/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  RetrOS™: real-time preemtive multitasking operating system.
  armv7-m svc, pendsv handlers for syscalls and context switching.

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

#define DWT_CYCCNT              0xE0001004
#define EXC_RETURN_FPU          0x00000010
#define EXC_RETURN_PSP          0x00000004
#define CONTROL_PSP             0x00000002
#define ERR_INVALID_CALLER      #2          @ OS_Result_InvalidCaller


.syntax unified
.thumb
.text

.extern OS_Scheduler
.extern OS_SyscallHandler

.global OS_Syscall
.global SVC_Handler
.global PendSV_Handler
.global g_OS_SchedulerPending


.thumb_func
    OS_Syscall:
            mrs      r3, control
            tst      r3, CONTROL_PSP
            beq      .syscallError
            svc      0
            ldr      r3, =g_OS_SchedulerPending
        .waitForPSCall:
            ldr      r2, [r3]
            cbz      r2, .syscallReturn
            b        .waitForPSCall
        .syscallReturn:
            bx       lr
        .syscallError:
            mov      r0, ERR_INVALID_CALLER
            bx       lr


.thumb_func
    SVC_Handler:
            tst      lr, EXC_RETURN_PSP
            bne      .fromPSP           @ Must have been called from a PSP task
            bkpt     #0
            pop      {r0}               @ Pop original R0 value on MSP
            mov      r0, ERR_INVALID_CALLER
            push     {r0}               @ Push return value to R0 on MSP
            bx       lr
        .fromPSP:
            push     {lr}
            bl       OS_SyscallHandler
            pop      {lr}
            mrs      r12, psp
            ldmfd    r12!, {r1}         @ Pop original R0 value on PSP
            stmfd    r12!, {r0}         @ Push return value to R0 on PSP
            msr      psp, r12           @ Update PSP (not really needed)
            isb
            bx       lr


.thumb_func
    PendSV_Handler:
            clrex
            mov      r2, #0
            ldr      r3, =DWT_CYCCNT
            ldr      r1, [r3]           @ Store cycle count in R1
            str      r2, [r3]           @ Zero cycle counter
            mrs      r0, psp            @ PSP in R0
            cbnz     r0, .storePSP
            @ PSP == 0, first scheduler run, store original MSP.
        .pushMSP:
            tst      lr, EXC_RETURN_FPU
            it       eq
            vpusheq  {s16-s31}
            push     {r4-r11, lr}
            b        .nextPSP
        .storePSP:
            tst      lr, EXC_RETURN_FPU
            it       eq
            vstmdbeq r0!, {s16-s31}
            stmfd    r0!, {r4-r11, lr}
        .nextPSP:
            bl       OS_Scheduler       @ R0: currentSp, R1: taskCycles
            mov      r2, #0
            ldr      r3, =DWT_CYCCNT
            str      r2, [r3]           @ Zero cycle counter
            @ psp == 0, scheduler shut down, back to msp
            cbz      r0, .recoverMSP
        .updatePSP:
            ldmfd    r0!, {r4-r11, lr}
            tst      lr, EXC_RETURN_FPU
            it       eq
            vldmiaeq r0!, {s16-s31}
            msr      psp, r0
            isb
            b        .back
        .recoverMSP:
            pop      {r4-r11, lr}
            tst      lr, EXC_RETURN_FPU
            it       eq
            vpopeq   {s16-s31}
        .back:
            bx       lr
