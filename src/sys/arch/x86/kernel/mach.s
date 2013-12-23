//
// mach.s
//
// Machine "naked" functions for x86
//
// Copyright (C) 2013 Bruno Ribeiro. All rights reserved.
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


    .intel_syntax noprefix
    .text

    .global ___inb
    .global ___inp
    .global ___inw
    .global ___inpw
    .global ___ind
    .global ___inpd
    .global ___insw
    .global ___insd
    .global ___outb
    .global ___outp
    .global ___outw
    .global ___outpw
    .global ___outd
    .global ___outpd
    .global ___outsw
    .global ___outsd
    .global ___hw_rdtsc

___inb:
___inp:
    mov  dx, [esp + 0x04]  // port
    xor  eax, eax
    in   al, dx
    ret


___inw:
___inpw:
    mov  dx, [esp + 0x04]  // port
    xor  eax, eax
    in   ax, dx
    ret


___ind:
___inpd:
    ind:
    mov  dx, [esp + 0x04]  // port
    in   eax, dx
    ret


___insw:
    mov edx, [esp + 0x04]   // port
    mov edi, [esp + 0x08]   // buf
    mov ecx, [esp + 0x0C]   // count
    rep insw
    ret


___insd:
    mov dx,  [esp + 0x04]  // port
    mov edi, [esp + 0x08]  // buf
    mov ecx, [esp + 0x0C]  // count
    rep insd
    ret


___outb:
___outp:
    mov     dx, [esp + 0x04]
    mov     al, [esp + 0x08]
    out     dx, al
    ret


___outw:
___outpw:
    mov     dx, [esp + 0x04]
    mov     ax, [esp + 0x08]
    out     dx, ax
    ret


___outd:
___outpd:
    mov      dx, [esp + 0x04]
    mov     eax, [esp + 0x08]
    out      dx, eax
    ret


___outsw:
    mov edx, [esp + 0x04]  // port
    mov esi, [esp + 0x08]  // buf
    mov ecx, [esp + 0x0c]  // count
    rep outsw
    ret


___outsd:
    mov  dx, [esp + 0x04]  // port
    mov esi, [esp + 0x08]  // buf
    mov ecx, [esp + 0x0c]  // count
    rep outsd
    ret


___hw_rdtsc:
    rdtsc
    ret
