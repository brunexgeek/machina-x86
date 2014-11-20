//
// trap.s
//
// Interrupt and trap handling "naked" functions for x86
//
// Copyright (C) 2013-2014 Bruno Ribeiro. All rights reserved.
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.
//

#include <os/trap.h>
#include <os/syspage.h>


    .intel_syntax noprefix
    .text
    .global ___sysentry
    .global ___systrap
    .global ___isr


___sysentry:
    mov     esp, ss:[esp];           // Get kernel stack pointer from TSS
    STI;                             // Sysenter disables interrupts, re-enable now

    push    SEL_UDATA + SEL_RPL3;    // Push ss (fixed)
    push    ebp;                     // Push esp (ebp set to esp by caller)
    pushfd;                          // Push eflg
    push    SEL_UTEXT + SEL_RPL3;    // Push cs (fixed)
    push    ecx;                     // Push eip (return address set by caller)
    push    0;                       // Push errcode
    push    INTR_SYSENTER;           // Push traptype (always sysenter)

    push    eax;                     // Push registers
    push    ecx;
    push    edx;
    push    ebx;
    push    ebp;
    push    esi;
    push    edi;

    push    SEL_UDATA + SEL_RPL3;    // Push ds (fixed)
    push    SEL_UTEXT + SEL_RPL3;    // Push es (fixed)

    mov     bx, SEL_KDATA;           // Setup kernel data segment
    mov     ds, bx;
    mov     es, bx;

    mov     ebx, esp;                // ebx = context
    push    ebx;                     // Push context
    push    edx;                     // Push params
    push    eax;                     // Push syscallno

    call    syscall;                 // Call syscall
    add     esp, 12;

    pop     es;                      // Restore registers
    pop     ds;
    pop     edi;
    pop     esi;
    pop     ebp;
    pop     ebx;

    add     esp, 20;                 // Skip edx, ecx, eax, errcode, traptype
    pop     edx;                     // Put eip into edx for sysexit
    add     esp, 4;                  // Skip cs
    popfd;                           // Restore flags
    pop     ecx;                     // Put esp into ecx for sysexit
    add     esp, 4;                  // Skip ss

    sysexit;                         // Return


___systrap:
    push    0;                       // push dummy error code
    push    INTR_SYSCALL;            // push traptype (INTR_SYSCALL)
    push    eax;                     // save registers
    push    ecx;
    push    edx;
    push    ebx;
    push    ebp;
    push    esi;
    push    edi;
    push    ds;
    push    es;
    mov     bx,  SEL_KDATA;          // setup kernel data segment (SEL_KDATA)
    mov     ds, bx;
    mov     es, bx;

    mov     ebx, esp;                // ebx = context
    push    ebx;                     // push context
    push    edx;                     // push params
    push    eax;                     // push syscallno

    call    syscall;
    add     esp, 12;
    pop     es;                      // restore registers
    pop     ds;
    pop     edi;
    pop     esi;
    pop     ebp;
    pop     ebx;
    pop     edx;
    pop     ecx;
    // skip eax, errcode, and traptype
    add     esp, 12;

    iretd;


___isr:
    cld;
    push    eax;                     // save registers
    push    ecx;
    push    edx;
    push    ebx;
    push    ebp;
    push    esi;
    push    edi;
    push    ds;
    push    es;

    mov     ax, SEL_KDATA;           // setup kernel data segment (SEL_KDATA)
    mov     ds, ax;
    mov     es, ax;
    call    trap;                    // call trap handler

    pop     es;                      // restore registers
    pop     ds;
    pop     edi;
    pop     esi;
    pop     ebp;
    pop     ebx;
    pop     edx;
    pop     ecx;
    pop     eax;
    add     esp, 8;
    iretd;
