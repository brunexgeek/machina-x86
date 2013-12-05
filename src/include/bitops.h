//
// bitops.h
//
// Bitmap manipulation routines
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

#ifndef BITOPS_H
#define BITOPS_H

#ifdef  __cplusplus
extern "C" {
#endif

static __inline void set_bit(void *bitmap, int pos) {
    /*__asm__
    (
        "movs eax, pos"
        "mov ebx, bitmap"
        "bts dword ptr [ebx], eax"
    );*/
    *((unsigned int*)bitmap) = *((unsigned int*)bitmap) | (1 << pos);
}

static __inline void clear_bit(void *bitmap, int pos)
{
    /*__asm__
    (
        "mov eax, pos"
        "mov ebx, bitmap"
        "btr dword ptr [ebx], eax"
    );*/
    *((unsigned int*)bitmap) = *((unsigned int*)bitmap) & ~(1 << pos);
}

static __inline int test_bit(void *bitmap, int pos) {
    /*
    int result;
    __asm__
    (
        "mov eax, pos"
        "mov ebx, bitmap"
        "bt dword ptr [ebx], eax"
        "sbb eax, eax"
        "mov result, eax"
    );*/
    unsigned int v;  // find the number of trailing zeros in 32-bit v
    int r;           // result goes here
    static const int table[32] =
    {
        0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
        31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
    };

    v = *((unsigned int*)bitmap);
    return table[((unsigned int)((v & -v) * 0x077CB531U)) >> 27];
}

static __inline int find_lowest_bit(unsigned mask) {
    /*int n;

    __asm__
    (
        "bsf eax, mask"
        "mov n, eax"
    );*/
    unsigned int v;  // find the number of trailing zeros in v
    static const int Mod37BitPosition[] = // map a bit value mod 37 to its position
    {
        32, 0, 1, 26, 2, 23, 27, 0, 3, 16, 24, 30, 28, 11, 0, 13, 4,
        7, 17, 0, 25, 22, 31, 15, 29, 10, 12, 6, 0, 21, 14, 9, 5,
        20, 8, 19, 18
    };
    return Mod37BitPosition[(-mask & mask) % 37];
}

#if 0
static __inline int find_highest_bit(unsigned mask)
{
    int n;

    __asm__
    (
        "bsr eax, mask"
        "mov n, eax"
    );

    return n;
}
#else
static __inline int find_highest_bit(unsigned mask) {
  int n = 31;

  while (n > 0 && !(mask & 0x80000000)) {
    mask <<= 1;
    n--;
  }

  return n;
}
#endif

static __inline void set_bits(void *bitmap, int pos, int len) {
  while (len-- > 0) set_bit(bitmap, pos++);
}

int find_first_zero_bit(void *bitmap, int len);
int find_next_zero_bit(void *bitmap, int len, int start);

#ifdef  __cplusplus
}
#endif

#endif
