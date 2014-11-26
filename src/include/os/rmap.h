//
// rmap.h
//
// Routines for resource mapping
//
// Copyright (C) 2014 Bruno Ribeiro.
// All rights reserved.
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

#ifndef MACHINA_OS_RMAP_H
#define MACHINA_OS_RMAP_H


#define RMAP_TOTAL(rmptr)     ( (rmap)->size )
#define RMAP_MAX_ENTRIES      (0xFFFF)

#include <stdint.h>

void kprintf(const char *fmt, ...);


//#define RMAP_DEBUG

/**
 * Resource mapping entry.
 */
struct rmap_t
{
    /**
     * Number of pages (used or free) in the chunk.
     */
    uint32_t size;

    /**
     * Index of the next chunk. If the last chunk, this value should be zero.
     */
    uint16_t next;

    /**
     * Indicate if the chunk information is about allocated pages.
     */
    uint16_t used;
};


#ifdef  __cplusplus
extern "C" {
#endif

int krmap_initialize(
    struct rmap_t *r,
    uint32_t size);

uint32_t krmap_alloc(
    struct rmap_t *rmap,
    uint32_t size);

uint32_t krmap_alloc_align(
    struct rmap_t *rmap,
    uint32_t size,
    uint32_t align );

int krmap_free(
    struct rmap_t *rmap,
    uint32_t offset,
    uint32_t size );

int krmap_reserve(
    struct rmap_t *rmap,
    uint32_t offset,
    uint32_t size );

int krmap_get_status(
    struct rmap_t *rmap,
    uint32_t offset,
    uint32_t size);

int krmap_get_entry_count(
    struct rmap_t *rmap,
    uint32_t *count );

#ifdef  __cplusplus
}
#endif


#endif  // MACHINA_OS_RMAP_H
