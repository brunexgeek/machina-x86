//
// rmap.c
//
// Routines for resource mapping
//
// Copyright (C) 2024 Bruno Ribeiro.
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

//
// The first entry in a resource map is interpreted as a header.
// The r_size field tells how many slots the overall data structure
// has; r_off tells how many non-empty elements exist.
//
// The list is kept in ascending order with all non-empty elements
// first.
//

#include <stdarg.h>
#include <sys/types.h>
#include <string.h>
#include <os/rmap.h>
#include <errno.h>


#ifdef DEVELOPER
#include <stdio.h>
#include <stdlib.h>

#define kprintf  printf
#endif


#define RMAP_MAX_PAGES        (1048576)  // Number of pages for 32bits
#define RMAP_MAX_ALIGN        (256)


void panic( const char *msg )
#ifndef DEVELOPER
;
#else
{
    printf("%s\n", msg);
    exit(1);
}
#endif


#ifdef RMAP_DEBUG
static void dump( struct rmap_t *rmap )
{
    struct rmap_t *r = rmap + 1;
    uint32_t offset = 0;

    kprintf("\nrmap #%X\n", (uint32_t)rmap);

    while (1)
    {
        kprintf("rmap[%2d] .offset=0x%08x  .size=%-10d  .type='%s'\n",
            (r - rmap),
            offset,
            r->size,
            (r->used != 0)? "used" : "free");
        if (r->next == 0) break;
        offset += r->size;
        r = rmap + r->next;
    }
    kprintf("         .used=%-10d\n         .free=%d\n", RMAP_TOTAL(rmap), RMAP_MAX_PAGES - RMAP_TOTAL(rmap));
    kprintf("\n");
}
#endif


/**
 * Initialize the given resource map
 *
 * @param rmap Pointer for an array of struct rmap_t.
 * @param size Number of the elements int the @c rmap array.
 * @return Error code.
 */
int krmap_initialize(
    struct rmap_t *rmap,
    uint32_t size )
{

    if (rmap == NULL || size < 16) return -EINVAL;
    if (size > RMAP_MAX_ENTRIES) size = RMAP_MAX_ENTRIES;

    memset(rmap, 0, sizeof(struct rmap_t) * size);
    // use the first entry to hold general information
    rmap[0].next = size - 1;
    rmap[0].size = RMAP_MAX_PAGES;
    // set all pages as allocated
    rmap[1].size = RMAP_MAX_PAGES;
    rmap[1].used = 1;
    rmap[1].next = 0;

    return 0;
}


/**
 * Allocates pages.
 *
 * @return Address for the first allocated page or 0 otherwise.
 * @remarks This function can not allocate the first page in the memory (0x00000000).
 */
uint32_t krmap_alloc(struct rmap_t *rmap, uint32_t size)
{
    return krmap_alloc_align(rmap, size, 1);
}


/**
 * Returns an unused entry, if any.
 *
 * @return Pointer to the unused entry of NULL otherwise.
 */
static inline struct rmap_t *krmap_new( struct rmap_t *rmap )
{
    register struct rmap_t *current;
    struct rmap_t *limit = rmap + rmap->next;

    // get the next free entry in the resource map
    current = rmap + 1;
    while (current < limit)
    {
        if (current->size == 0) break;
        ++current;
    }
    if (current < rmap || current >= limit) return NULL;
    return current;
}


/**
 * Insert a new entry after the given entry.
 */
static inline struct rmap_t *krmap_split(
    struct rmap_t *rmap,
    struct rmap_t *p,
    struct rmap_t *current,
    uint32_t offset,
    uint32_t size )
{
    register struct rmap_t *prev = NULL, *next = NULL;
    uint32_t after;

    // check if we have enough pages too split
    if (current->size < offset + size) return NULL;

    after = current->size - offset - size;

    // insert a new entry after the current one
    if (offset > 0)
    {
        prev = current;
        // create a new entry
        current = krmap_new(rmap);
        if (current == NULL) return NULL;
        current->next = prev->next;
        prev->next = current - rmap;
        // update the new entry
        current->size = prev->size - offset;
        current->used = prev->used;
        // update the previous entry
        prev->size = offset;
    }
    // if we have remaining pages, we need a new entry
    if (after > 0)
    {
        // create a new entry
        next = krmap_new(rmap);
        if (next == NULL) return NULL;
        next->next = current->next;
        current->next = next - rmap;
        // update the new entry
        next->size = after;
        next->used = current->used;
        // update the previous entry
        current->size = current->size - after;
    }

    // adjust the sizes
    current->used = (current->used == 0);

    if (prev == NULL) prev = p;
    if (next == NULL && current->next != 0) next = rmap + current->next;

    if (prev != NULL && prev->used == current->used)
    {
        prev->size += current->size;
        prev->next = current->next;
        current->size = 0;
        current = prev;
    }
    if (next != NULL && next->used == current->used)
    {
        current->size += next->size;
        current->next = next->next;
        next->size = 0;
    }

    return current;
}


/**
 * Allocates aligned pages.
 *
 * @return Address for the first allocated page or 0 otherwise.
 * @remarks This function can not allocate the first page in the memory (0x00000000).
 */
uint32_t krmap_alloc_align(
    struct rmap_t *rmap,
    uint32_t size,
    uint32_t align )
{
    register struct rmap_t *current, *prev;
    register uint32_t offset;
    uint32_t before;
    #ifdef RMAP_DEBUG
    uint32_t after;
    #endif

    if (align > RMAP_MAX_ALIGN) return -1;
    if (align == 0) align = 1;

    // find first entry that fits
    current = rmap + 1;
    offset = 0;
    prev = NULL;
    while (current != rmap)
    {
        if (!current->used)
        {
            // calculates the free space before the next aligned offset
            before = offset % align;//align - (offset % align) - 1;
            // check if we have enough space in the curent entry
            if (current->size > before && current->size - before >= size)
            {
                // calculates the free space after the range
                #ifdef RMAP_DEBUG
                after = current->size - before - size;
                #endif
                break;
            }
        }
        offset += current->size;
        prev = current;
        current = rmap + current->next;
    }
    if (current == rmap)
    {
        panic("Allocation failed!");
        return 0;
    }

    #ifdef RMAP_DEBUG
    if (align > 1)
        kprintf("Allocating %d pages at 0x%08x [ .align=%d .before=%d  .after=%d ] \n", size, offset+before, align, before, after);
    else
        kprintf("Allocating %d pages at 0x%08x [ .before=%d  .after=%d ] \n", size, offset+before, before, after);
    #endif

    current = krmap_split(rmap, prev, current, before, size);

    // increments the total number of mapped pages (don't include gaps)
    RMAP_TOTAL(rmap) += size;

    #ifdef RMAP_DEBUG
    dump(rmap);
    #endif

    // returns the address for the first mapped page
    return offset + before;
}


/**
 * Free previously allocated pages.
 *
 * @return 0 on success or error code otherwise.
 */
int krmap_free(
    struct rmap_t *rmap,
    uint32_t offset,
    uint32_t size )
{
    register struct rmap_t *current, *prev;
    register uint32_t pos;
    uint32_t before;
    #ifdef RMAP_DEBUG
    uint32_t after;
    #endif

    // find first entry that fits
    current = rmap + 1;
    pos = 0;
    prev = NULL;
    while (current != rmap)
    {
        if (current->used)
        {
            // check if we have enough space in the current entry
            if (offset >= pos && offset + size <= pos + current->size)
            {
                // calculates the free space after the range
                before = offset - pos;
                #ifdef RMAP_DEBUG
                after = current->size - before - size;
                #endif
                break;
            }
        }
        pos += current->size;
        prev = current;
        current = rmap + current->next;
    }
    if (current == rmap) return -ENOMEM;

    #ifdef RMAP_DEBUG
    kprintf("Removing %d pages from 0x%08x [ .before=%d  .after=%d ] \n", size, offset, before, after);
    #endif

    current = krmap_split(rmap, prev, current, before, size);

    // decrements the total number of mapped pages (don't include gaps)
    RMAP_TOTAL(rmap) -= size;

    #ifdef RMAP_DEBUG
    dump(rmap);
    #endif

    return;
}


/**
 * Reserve the pages at requested range.
 *
 * All pages in the range should be free.
 *
 * @return 0 on success or error code otherwise.
 */
int krmap_reserve(
    struct rmap_t *rmap,
    uint32_t offset,
    uint32_t size)
{
    register struct rmap_t *current, *prev;
    register uint32_t pos;
    uint32_t before;
    #ifdef RMAP_DEBUG
    uint32_t after;
    #endif

    if (rmap == NULL || size == 0) return -EINVAL;

    // find first entry that fits
    current = rmap + 1;
    pos = 0;
    prev = NULL;
    while (current != rmap)
    {
        if (!current->used)
        {
            // check if we have enough space in the curent entry
            if (offset >= pos && offset + size <= pos + current->size)
            {
                // calculates the free space after the range
                before = offset - pos;
                #ifdef RMAP_DEBUG
                after = current->size - before - size;
                #endif
                break;
            }
        }
        pos += current->size;
        prev = current;
        current = rmap + current->next;
    }
    if (current == rmap) return -ENOMEM;

    #ifdef RMAP_DEBUG
    kprintf("Reserving %d pages from 0x%08x [ .before=%d  .after=%d ] \n", size, offset, before, after);
    #endif

    if ( krmap_split(rmap, prev, current, before, size) == NULL) return -ENOMEM;

    // increments the total number of mapped pages (don't include gaps)
    RMAP_TOTAL(rmap) += size;

    #ifdef RMAP_DEBUG
    dump(rmap);
    #endif

    return 0;
}


/**
 * Checks allocation status for a page range.
 *
 * @returns 0 if free, 1 if allocated or -1 if partially allocated.
 */
int krmap_get_status(
    struct rmap_t *rmap,
    uint32_t offset,
    uint32_t size )
{
    register struct rmap_t *current;
    uint32_t pos;

    current = rmap + 1;
    pos = 0;
    while (1)
    {
        if (pos <= offset && pos + current->size > offset)
        {
            // check if the region is inside of the current entry
            if (offset >= pos && offset + size <= pos + current->size)
                return current->used;
            else
                return -1;
        }

        pos += current->size;
        current = rmap + current->next;
    }

    // increments the total number of mapped pages (don't include gaps)
    RMAP_TOTAL(rmap) += size;

    #ifdef RMAP_DEBUG
    dump(rmap);
    #endif

    return 0;
}


/**
 * Returns the amount of used entry in the map.
 *
 * @return Number of entry or an error code otherwise.
 */
int krmap_get_entry_count(
    struct rmap_t *rmap,
    uint32_t *count )
{
    uint16_t c = 0;
    struct rmap_t *current = rmap + 1;

    if (rmap == NULL || count == NULL) return -EINVAL;

    while (1)
    {
        ++c;
        if (current->next == 0) break;
        current = rmap + current->next;
    }
    *count = c;

    return 0;
}


#ifdef DEVELOPER
int main( int argc, char **argv )
{
    uint8_t memory[128];
    struct rmap_t *rmap = (struct rmap_t*) memory;


    kprintf("struct rmap_t == %d\n", sizeof(struct rmap_t));
    kprintf("RMAP_MAX_PAGES == %d\n", RMAP_MAX_PAGES);

    krmap_init(rmap, 20);
    dump(rmap);
    krmap_free(rmap, 0x00092000, 450560);
    krmap_alloc(rmap, 2);
    krmap_alloc(rmap, 1);
    krmap_alloc_align(rmap, 2, 2);
    krmap_alloc(rmap, 2);
    krmap_alloc(rmap, 1);
    krmap_alloc(rmap, 1);

    /*dump(rmap);
    krmap_free(rmap, 0, 100);
    krmap_reserve(rmap, 50, 10);
    kprintf( "Mapped to #%d\n\n", krmap_alloc(rmap, 1) );
    kprintf( "Mapped to #%d\n\n", krmap_alloc(rmap, 1) );
    kprintf( "Mapped to #%d\n\n", krmap_alloc_align(rmap, 2, 4) );
    kprintf( "Mapped to #%d\n\n", krmap_alloc(rmap, 3) );
    krmap_free(rmap, 3, 4);
    kprintf( "Mapped to #%d\n\n", krmap_alloc_align(rmap, 2, 5) );
    kprintf( "Mapped to #%d\n\n", krmap_alloc(rmap, 1) );
    krmap_free(rmap, 0, RMAP_MAX_PAGES);*/

    return 0;
}
#endif
