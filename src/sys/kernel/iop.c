//
// iop.c
//
// Port input/output
//
// Copyright (C) 2013 Bruno Ribeiro. All rights reserved.
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

#include <os/krnl.h>
#include <os/asmutil.h>

#ifndef VMACH


static void test___()
{
    return;
}

//unsigned char inb(port_t port) __asm__("___inb");
__asm__
(
    "___inb: "
    "mov      dx, [esp + 4];"
    "xor     eax, eax;"
    "in       al, dx;"
    "ret;"
);


//unsigned short inw(port_t port) __asm__("___inw");
__asm__
(
    "___inw: \n"
    "inw: "
    "mov      dx, [esp + 4];"
    "xor     eax, eax;"
    "in       ax, dx;"
    "ret;"
);



//unsigned long ind(port_t port) __asm__("___ind");
__asm__
(
    "___ind: \n"
    "ind: "
    "mov      dx, [esp + 4];"
    "in      eax, dx;"
    "ret;"
);



//void insw(port_t port, void *buf, int count) __asm__("___insw");
__asm__
(
    "___insw: "
    "mov edx, [esp + 4];"
    "mov edi, [esp + 8];"
    "mov ecx, [esp + 12];"
    "rep insw;"
);


//void insd(port_t port, void *buf, int count) __asm__("___insd");
__asm__
(
    "___insd: "
    "mov edx, [esp + 4];"
    "mov edi, [esp + 8];"
    "mov ecx, [esp + 12];"
    "rep insd;"
);


//unsigned char outb(port_t port, unsigned char val) __asm__("___outb");
__asm__
(
    "___outb: "
    "mov     dx, [esp + 4];"
    "mov     al, [esp + 8];"
    "out     dx, al;"
    "ret;"
);


//unsigned short outw(port_t port, unsigned short val) __asm__("___outw");
__asm__
(
    "___outw: "
    "mov     dx, [esp + 4];"
    "mov     ax, [esp + 8];"
    "out     dx, ax;"
    "ret;"
);


//unsigned long outd(port_t port, unsigned long val) __asm__("___outd");
__asm__
(
    "___outd: "
    "mov      dx, [esp + 4];"
    "mov     eax, [esp + 8];"
    "out      dx, eax;"
    "ret;"
);


//void outsw(port_t port, void *buf, int count) __asm__("___outsw");
__asm__
(
    "___outsw: "
    "mov  dx, [esp + 4];"  // port
    "mov esi, [esp + 8];"  // buf
    "mov ecx, [esp + 12];" // count
    "rep outsw;"
);


//void outsd(port_t port, void *buf, int count) __asm__("___outsd");
__asm__
(
    "___outsd: "
    "mov  dx, [esp + 4];"  // port
    "mov esi, [esp + 8];"  // buf
    "mov ecx, [esp + 12];" // count
    "rep outsd;"
);

#endif
