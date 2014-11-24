//
// pframe.c
//
// Page frame database functions.
// Based on Michael Ringgaard implementation (Sanos).
//
// Copyright (C) 2013-2014 Bruno Ribeiro
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


#define MAX_PFT                  (1 << 5)
#define MAX_MEMTYPES             (4)

uint32_t freeCount;              /// Number of free frames
uint32_t useableCount;           /// Number of usable frames
uint32_t frameCount;             /// Number of frames in PFDB/memory


/**
 * Pointer to frame array.
 */
uint16_t *frameArray_;


static const char *MEMTYPE_NAMES[] =
{
    "MEM",
    "RAM",
    "RESV",
    "ACPI",
    "NVS"
};


static struct
{
    const char *name;
    const char *symbol;
} PFT_NAMES[MAX_PFT] =
{
    { NULL,   NULL },
    { "FREE", "." },
    { "RAM",  "H" },
    { "RESV", "-" },
    { "MEM",  "." },
    { "NVS",  "N" },
    { "ACPI", "A" },
    { "BAD",  "*" },
    { "PTAB", "P" },
    { "DMA",  "D" },
    { "PFDB", "F" },
    { "SYS",  "Y" },
    { "TCB",  "T" },
    { "BOOT", "B" },
    { "FMAP", "M" },
    { "STCK", "S" },
    { "KMEM", "E" },
    { "KMOD", "K" },
    { "UMOD", "U" },
    { "VM",   "V" },
    { NULL,   NULL },
    { NULL,   NULL },
    { NULL,   NULL },
    { NULL,   NULL },
    { NULL,   NULL },
    { NULL,   NULL },
    { NULL,   NULL },
    { NULL,   NULL },
    { NULL,   NULL },
    { NULL,   NULL },
    { NULL,   NULL },
    { NULL,   NULL }
} ;


void panic(char *msg);


/**
 * Allocate a memory frame.
 * @return Index of the allocated frame or 0xFFFFFFFF otherwise.
 */
uint32_t kpframe_alloc(
    uint32_t count,
    uint8_t tag )
{
    register uint32_t i, j;

    if (count == 0) return INVALID_PFRAME;
    if (tag == PFT_FREE) panic("Can not allocate with tag PFT_FREE");

    // check if we have enough free memory
    if (freeCount < count) return INVALID_PFRAME;
    // find some region with available frames
    for (i = 0; i < frameCount; ++i)
    {
        if (PFRAME_GET_TAG(i) != PFT_FREE) continue;
        // check if current region has enough frames
        for (j = 0; j < count && PFRAME_GET_TAG(i+j) == PFT_FREE; ++j);
        if (j == count)
        {
            // reserve frames with given tag
            for (j = 0; j < count; ++j)
                PFRAME_SET_TAG(i+j, tag);
            // decrease the used frames counter
            freeCount -= count;
            return i;
        }
    }

    return INVALID_PFRAME;
}


void kpframe_free( uint32_t index )
{
    if (index >= frameCount) return;

    PFRAME_SET_TAG(index, PFT_FREE);
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
        PFRAME_SET_TAG(index, tag);
        ptr += PAGESIZE;
        if (tag == PFT_FREE)
            freeCount++;
        else
            freeCount--;
    }
}


uint8_t kpframe_get_tag( void *vaddr )
{
    return PFRAME_GET_TAG(virt2phys(vaddr) << PAGESHIFT);
}


const char *kpframe_tag_name( uint8_t tag )
{
    if (tag > sizeof(PFT_NAMES)) return "?";
    return PFT_NAMES[tag].name;
}


int memmap_proc(struct proc_file *pf, void *arg)
{
    struct memmap *mm = &syspage->bootparams.memmap;
    int i;
    const char *name;

    for (i = 0; i < mm->count; i++)
    {
        uint32_t type = mm->entry[i].type;

        if (type == MEMTYPE_RAM || type == MEMTYPE_RESERVED || type == MEMTYPE_ACPI || type == MEMTYPE_NVS)
            name = MEMTYPE_NAMES[type];
        else
            name = MEMTYPE_NAMES[0];

        pprintf(pf, "0x%08x-0x%08x type %-4s %8d KiB\n",
            (unsigned long) mm->entry[i].addr,
            (unsigned long) (mm->entry[i].addr + mm->entry[i].size) - 1,
            name,
            (unsigned long) mm->entry[i].size / 1024);
    }

    return 0;
}


int memusage_proc(struct proc_file *pf, void *arg)
{
    uint32_t counters[MAX_PFT];
    unsigned int n;
    const char *name;

    memset(counters, 0, sizeof(counters));

    for (n = 0; n < frameCount; n++)
    {
        uint8_t tag = PFRAME_GET_TAG(n);
        if (tag < MAX_PFT)
            counters[tag]++;
    }

    for (n = 0; n < MAX_PFT; n++)
    {
        if (counters[n] == 0) continue;
        name = PFT_NAMES[n].name;
        pprintf(pf, "%-4s    %8d KiB\n", name, counters[n] * (PAGESIZE / 1024));
    }

    return 0;
}


int memstat_proc(struct proc_file *pf, void *arg)
{
    pprintf(pf, "Total     %8d MiB\nUsed      %8d KiB\nFree      %8d KiB\nReserved  %8d KiB\n",
        frameCount * PAGESIZE / (1024 * 1024),
        (useableCount - freeCount) * PAGESIZE / 1024,
        freeCount * PAGESIZE / 1024, (frameCount - useableCount) * PAGESIZE / 1024);

    return 0;
}


int physmem_proc(struct proc_file *pf, void *arg)
{
    uint32_t n;
    uint8_t tag, keep = 1;

    for (n = 0, tag = 0; n < MAX_PFT; ++n)
    {
        if (PFT_NAMES[n].name == NULL) continue;
        if (tag != 0 && (tag % 6) == 0) pprintf(pf, "\n");
        pprintf(pf, "%s = %-4s   ", PFT_NAMES[n].symbol, PFT_NAMES[n].name);
        ++tag;
    }

    pprintf(pf, "\n\n");

    for (n = 0; n < frameCount; n++)
    {
        if (n % 64 == 0)
        {
            if (n > 0)
            {
                pprintf(pf, "\n");
                if (!keep) break;
                keep = 0;
            }
            pprintf(pf, "%08X ", PTOB(n));
        }

        tag = PFRAME_GET_TAG(n);
        if (tag != PFT_FREE) keep = 1;
        if (tag > MAX_PFT)
        {
            kprintf("tag = %d\n", tag);
            panic("Fuuu");
        }
        if (PFT_NAMES[tag].symbol == NULL)
            pprintf(pf, "?");
        else
            pprintf(pf, "%s", PFT_NAMES[tag].symbol);
    }

    pprintf(pf, "\n");
    return 0;
}


void kpframe_initialize()
{
    unsigned long heap;
    unsigned long pfdbpages;
    unsigned long i, j;
    unsigned long memend;
    pte_t *pt;
    struct memmap *memmap;

    // register page directory
    kmach_register_page_dir(virt2pfn(pdir));

    // calculates number of pages needed for page frame database
    memend = syspage->ldrparams.memend;
    heap = syspage->ldrparams.heapend;
    pfdbpages = PAGES((memend / PAGESIZE) * sizeof(uint16_t));
    if ((pfdbpages + 2) * PAGESIZE + heap >= memend) panic("not enough memory for page table database");
    kprintf("[DEBUG] PFDB requires %d pages to map %d KiB\n", pfdbpages, memend / 1024);
    // intialize page tables for mapping the largest possible PFDB into kernel space
    // (for a machine with 4GB of physical RAM we need 524 pages for PFDB)
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
    frameCount = syspage->ldrparams.memend / PAGESIZE;
    useableCount = 0;
    freeCount = 0;
    frameArray_ = (uint16_t *) PFDBBASE;
    memset(frameArray_, 0, pfdbpages * PAGESIZE);
    for (i = 0; i < frameCount; i++) PFRAME_SET_TAG(i, PFT_BAD);

    // add all memory from memory map to PFDB
    memmap = &syspage->bootparams.memmap;
    for (i = 0; i < (unsigned long) memmap->count; i++)
    {
        uint32_t first = (uint32_t) memmap->entry[i].addr / PAGESIZE;
        uint32_t last = first + (uint32_t) memmap->entry[i].size / PAGESIZE;

        if (first >= frameCount) continue;
        if (last > frameCount) last = frameCount;

        if (memmap->entry[i].type == MEMTYPE_RAM)
        {
            for (j = first; j < last; j++) PFRAME_SET_TAG(j, PFT_FREE);
            useableCount += (last - first);
        }
        else
        if (memmap->entry[i].type == MEMTYPE_RESERVED)
        {
            for (j = first; j < last; j++) PFRAME_SET_TAG(j, PFT_RESERVED);
        }
    }
    // reserve physical page 0 for BIOS
    PFRAME_SET_TAG(0, PFT_RESERVED);
    useableCount--;
    // add interval [heapstart:heap] to PFDB as page table pages
    for (i = syspage->ldrparams.heapstart / PAGESIZE; i < heap / PAGESIZE; i++) PFRAME_SET_TAG(i, PFT_PTAB);
    // reserve DMA buffers at 0x10000 (used by floppy driver)
    for (i = DMA_BUFFER_START / PAGESIZE; i < DMA_BUFFER_START / PAGESIZE + DMA_BUFFER_PAGES; i++) PFRAME_SET_TAG(i, PFT_DMA);
    // fixup tags for PFDB, syspage and intial TCB
    kpframe_set_tag(frameArray_, pfdbpages * PAGESIZE, PFT_PFDB);
    kpframe_set_tag(syspage, PAGESIZE, PFT_SYS);
    kpframe_set_tag(kthread_self(), TCBSIZE, PFT_TCB);
    kpframe_set_tag((void *) INITRD_ADDRESS, syspage->ldrparams.initrd_size, PFT_BOOT);
    // update the amount of free frames
    for (i = 0; i < frameCount; i++)
        if (PFRAME_GET_TAG(i) == PFT_FREE) freeCount++;
}
