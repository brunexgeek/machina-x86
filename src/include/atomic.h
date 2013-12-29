//
// atomic.h
//
// Atomic operations
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


#ifndef MACHINA_ATOMIC_H
#define MACHINA_ATOMIC_H


// On uniprocessors, the 'lock' prefixes are not necessary (and expensive).
// Since Machina does not (yet) support SMP the 'lock' prefix is disabled for now.

static __inline int atomic_add(int *dest, int value)
{
    __asm__
    (
        //"mov edx, dest;"
        //"mov eax, value;"
        "mov ecx, eax;"
        /*lock*/ "xadd dword ptr [edx], eax;"
        "add eax, ecx;"
        :
        : "d" (dest), "a" (value)
    );
}


static __inline int atomic_increment(int *dest)
{
    __asm__
    (
        //"mov edx, dest;"
        "mov eax, 1;"
        /*lock*/ "xadd dword ptr [edx], eax;"
        "inc eax;"
        :
        : "d" (dest)
    );
}

static __inline int atomic_decrement(int *dest)
{
    __asm__
    (
        //"mov edx, dest;"
        "mov eax, -1;"
        /*lock*/ "xadd dword ptr [edx], eax;"
        "dec eax;"
        :
        : "d" (dest)
    );
}

static __inline int atomic_exchange(int *dest, int value)
{
    __asm__
    (
        //"mov eax, value;"
        //"mov ecx, dest;"
        "xchg eax, dword ptr [ecx];"
        :
        : "c" (dest), "a" (value)
    );
}

static __inline int atomic_compare_and_exchange(int *dest, int exchange, int comperand)
{
    __asm__
    (
        //"mov edx, dest;"
        //"mov ecx, exchange;"
        //"mov eax, comperand;"
        /*lock*/ "cmpxchg dword ptr [edx], ecx;"
        :
        : "d" (dest), "c" (exchange), "a" (comperand)
    );
}


#endif
