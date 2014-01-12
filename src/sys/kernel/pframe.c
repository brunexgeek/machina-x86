//
// pframe.c
//
// Page frame database routines
//
// Copyright (C) 2013-2014 Bruno Ribeiro
// Copyright (C) 2002 Michael Ringgaard
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

#include <os/pframe.h>
#include <os/pdir.h>
#include <os/object.h>
#include <os/syspage.h>


#define MAX_MEMTAGS              0x10

uint32_t freeCount;              /// Number of free frames
uint32_t usableCount;            /// Number of usable frames
uint32_t pfdbSize;               /// Sizeof PFDB (number of frames in memory)

struct page_frame_t *pfdb;       /// Base pointer for PFDB
struct page_frame_t *freeFrame;  /// Pointer to the next free frame at PFDB


static const char *FRAME_TAGS[] =
{
    "?",
    "FREE",
    "RAM",
    "RESV",
    "MEM",
    "NVS",
    "ACPI",
    "BAD",
    "PTAB",
    "DMA",
    "PFDB",
    "SYS",
    "TCB",
    "BOOT",
    "?",
    "?",
};


void panic(char *msg);


uint32_t kpframe_alloc( uint8_t tag )
{
    register struct page_frame_t *frame;

    if (freeFrame == NULL) panic("out of memory");

    frame = freeFrame;
    freeFrame = &pfdb[frame->next];
    freeCount--;

    frame->tag = tag;
    frame->next = 0;

    return frame - pfdb;
}


uint32_t kpframe_alloc_linear( uint32_t pages, uint8_t tag )
{
    register struct page_frame_t *current;
    struct page_frame_t *previous;
    uint32_t index;

    if (pages == 0) return INVALID_PFRAME;
    if (pages == 1) return kpframe_alloc(tag);

    // check if we have enough free memory
    if (freeCount < pages) return INVALID_PFRAME;

    previous = NULL;
    current = freeFrame;
    while (current)
    {
        index = current - pfdb;
        // check if it's possible allocate linear pages from the current frame
        if (current - pfdb + pages < pfdbSize)
        {
            int n;

            // ensure that all page frames is free and linear
            for (n = 0; n < pages; n++)
            {
                if (current[n].tag != PFT_FREE) break;
                if (n != 0 && current[n - 1].next != index + n) break;
            }

            if (n == pages)
            {
                // remove the pages from free list
                if (previous)
                {
                    previous->next = current[pages - 1].next;
                }
                else
                {
                    freeFrame = pfdb + current[pages - 1].next;
                }
                // update the tag for each frame
                for (n = 0; n < pages; n++)
                {
                    current[n].tag = tag;
                    current[n].next = 0;
                }

                freeCount -= pages;
                return current - pfdb;
            }
        }

        previous = current;
        current = &pfdb[current->next];
    }

    return INVALID_PFRAME;
}


void kpframe_free( uint32_t index )
{
    struct page_frame_t *frame;

    // TODO: check limits
    frame = pfdb + index;
    frame->tag = PFT_FREE;
    frame->next = freeFrame - pfdb;
    freeFrame = frame;
    freeCount++;
}


void kpframe_set_tag( void *vaddr, uint32_t len, uint8_t tag )
{
    char *ptr = (char *) vaddr;
    char *end = ptr + len;
    uint32_t index;

    while (ptr < end)
    {
        index = virt2phys(ptr) >> PAGESHIFT;
        pfdb[index].tag = tag;
        ptr += PAGESIZE;
    }
}


uint8_t kpframe_get_tag( void *vaddr )
{
    return pfdb[virt2phys(vaddr) << PAGESHIFT].tag;
}


const char *kpframe_tag_name( uint8_t tag )
{
    if (tag > sizeof(FRAME_TAGS)) return "?";
    return FRAME_TAGS[tag];
}


int memmap_proc(struct proc_file *pf, void *arg)
{
    struct memmap *mm = &syspage->bootparams.memmap;
    int i;
    const char *name;

    for (i = 0; i < mm->count; i++)
    {
        uint32_t type = mm->entry[i].type;

        if (type == PFT_RAM || type == PFT_RESERVED || type == PFT_ACPI || type == PFT_NVS)
            name = FRAME_TAGS[type];
        else
            name = FRAME_TAGS[PFT_MEM];

        pprintf(pf, "0x%08x-0x%08x type %s %8d KB\n",
            (unsigned long) mm->entry[i].addr,
            (unsigned long) (mm->entry[i].addr + mm->entry[i].size) - 1,
            name,
            (unsigned long) mm->entry[i].size / 1024);
    }

    return 0;
}


int memusage_proc(struct proc_file *pf, void *arg)
{
    unsigned int num_memtypes = 0;
    uint32_t counters[MAX_MEMTAGS];
    unsigned long tag;
    unsigned int n;
    unsigned int m;
    const char *name;

    memset(counters, 0, sizeof(counters));

    for (n = 0; n < pfdbSize; n++)
    {
        uint8_t tag = pfdb[n].tag;
        if (tag < MAX_MEMTAGS)
            counters[tag]++;
    }

    for (n = 0; n < MAX_MEMTAGS; n++)
    {
        if (counters[n] == 0) continue;
        name = FRAME_TAGS[n];
        pprintf(pf, "%-4s    %8d KB\n", name, counters[n] * (PAGESIZE / 1024));
    }

    return 0;
}


int memstat_proc(struct proc_file *pf, void *arg)
{
    pprintf(pf, "Memory %dMB total, %dKB used, %dKB free, %dKB reserved\n",
        pfdbSize * PAGESIZE / (1024 * 1024),
        (usableCount - freeCount) * PAGESIZE / 1024,
        freeCount * PAGESIZE / 1024, (pfdbSize - usableCount) * PAGESIZE / 1024);

    return 0;
}


int physmem_proc(struct proc_file *pf, void *arg)
{
    unsigned int n;
    const char *name;

    for (n = 0; n < pfdbSize; n++)
    {
        if (n % 64 == 0)
        {
            if (n > 0) pprintf(pf, "\n");
            pprintf(pf, "%08X ", PTOB(n));
        }

        switch (pfdb[n].tag)
        {
            case PFT_FREE:
                pprintf(pf, ".");
                break;
            case PFT_RESERVED:
                pprintf(pf, "-");
                break;
            case 0:
                pprintf(pf, "?");
                break;
            default:
                name = FRAME_TAGS[pfdb[n].tag];
                pprintf(pf, "%c", *name);
        }
    }

    pprintf(pf, "\n");
    return 0;
}


void kpframe_init()
{
    unsigned long heap;
    unsigned long pfdbpages;
    unsigned long i, j;
    unsigned long memend;
    pte_t *pt;
    struct page_frame_t *pf;
    struct memmap *memmap;

    // register page directory
    kmach_register_page_dir(virt2pfn(pdir));

    // calculates number of pages needed for page frame database
    memend = syspage->ldrparams.memend;
    heap = syspage->ldrparams.heapend;
    pfdbpages = PAGES((memend / PAGESIZE) * sizeof(struct page_frame_t));
    if ((pfdbpages + 2) * PAGESIZE + heap >= memend) panic("not enough memory for page table database");
    kprintf("[DEBUG] PFDB requires %d pages\n", pfdbpages);
    // intialize page tables for mapping the largest possible PFDB into kernel space
    // (for a machine with 4GB of physical RAM we need 1048 pages for PFDB)
    kmach_set_page_dir_entry(&pdir[PDEIDX(PFDBBASE)], heap | PT_PRESENT | PT_WRITABLE);
    pt = (pte_t *) heap;
    heap += PAGESIZE;
    memset(pt, 0, PAGESIZE);
    kmach_register_page_table(BTOP(pt));
    // allocate and map pages for page frame database
    for (i = 0; i < pfdbpages; i++)
    {
        kmach_set_page_table_entry(&pt[i], heap | PT_PRESENT | PT_WRITABLE);
        heap += PAGESIZE;
    }

    // initialize page frame database
    pfdbSize = syspage->ldrparams.memend / PAGESIZE;
    usableCount = 0;
    freeCount = 0;
    pfdb = (struct page_frame_t *) PFDBBASE;
    memset(pfdb, 0, pfdbpages * PAGESIZE);
    for (i = 0; i < pfdbSize; i++) pfdb[i].tag = PFT_BAD;
    // add all memory from memory map to PFDB
    memmap = &syspage->bootparams.memmap;
    for (i = 0; i < (unsigned long) memmap->count; i++)
    {
        uint32_t first = (uint32_t) memmap->entry[i].addr / PAGESIZE;
        uint32_t last = first + (uint32_t) memmap->entry[i].size / PAGESIZE;

        if (first >= pfdbSize) continue;
        if (last >= pfdbSize) last = pfdbSize;

        if (memmap->entry[i].type == MEMTYPE_RAM)
        {
            for (j = first; j < last; j++) pfdb[j].tag = PFT_FREE;
            usableCount += (last - first);
        }
        else
        if (memmap->entry[i].type == MEMTYPE_RESERVED)
        {
            for (j = first; j < last; j++) pfdb[j].tag = PFT_RESERVED;
        }
    }
    // reserve physical page 0 for BIOS
    pfdb[0].tag = PFT_RESERVED;
    usableCount--;
    // add interval [heapstart:heap] to PFDB as page table pages
    for (i = syspage->ldrparams.heapstart / PAGESIZE; i < heap / PAGESIZE; i++) pfdb[i].tag = PFT_PTAB;
    // reserve DMA buffers at 0x10000 (used by floppy driver)
    for (i = DMA_BUFFER_START / PAGESIZE; i < DMA_BUFFER_START / PAGESIZE + DMA_BUFFER_PAGES; i++) pfdb[i].tag = PFT_DMA;
    // fixup tags for PFDB, syspage and intial TCB
    kpframe_set_tag(pfdb, pfdbpages * PAGESIZE, PFT_PFDB);
    kpframe_set_tag(syspage, PAGESIZE, PFT_SYS);
    kpframe_set_tag(kthread_self(), TCBSIZE, PFT_TCB);
    kpframe_set_tag((void *) INITRD_ADDRESS, syspage->ldrparams.initrd_size, PFT_BOOT);
    // insert all free pages into free list
    pf = pfdb + pfdbSize;
    do {
        pf--;

        if (pf->tag == PFT_FREE)
        {
            pf->next = (uint32_t)(freeFrame - pfdb);
            freeFrame = pf;
            freeCount++;
        }
    } while (pf > pfdb);
}
