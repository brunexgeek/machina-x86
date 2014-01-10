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


#define MAX_MEMTAGS           128

unsigned long freemem;        // Number of pages free memory
unsigned long totalmem;       // Total number of pages of memory (bad pages excluded)
unsigned long maxmem;         // First unavailable memory page
struct pageframe *pfdb;       // Page frame database
struct pageframe *freelist;   // List of free pages

void panic(char *msg);


uint32_t kpframe_alloc( uint32_t tag )
{
    struct pageframe *pf;

    if (freemem == 0) panic("out of memory");

    pf = freelist;
    freelist = pf->next;
    freemem--;

    pf->tag = tag;
    pf->next = NULL;

    return pf - pfdb;
}


uint32_t kpframe_alloc_linear( uint32_t pages, uint32_t tag )
{
    struct pageframe *pf;
    struct pageframe *prevpf;

    if (pages == 0) return 0xFFFFFFFF;
    if (pages == 1) return kpframe_alloc(tag);

    // check if we have enough free memory
    if ((int) freemem < pages) return 0xFFFFFFFF;

    prevpf = NULL;
    pf = freelist;
    while (pf)
    {
        // check if it's possible allocate linear pages from the current frame
        if (pf - pfdb + pages < (int) maxmem)
        {
            int n;

            // ensure that all page frames is free and linear
            for (n = 0; n < pages; n++)
            {
                if (pf[n].tag != PFT_FREE) break;
                if (n != 0 && pf[n - 1].next != &pf[n]) break;
            }

            if (n == pages)
            {
                // fixup the 'next' pointer of the previosus frame
                if (prevpf)
                {
                    prevpf->next = pf[pages - 1].next;
                }
                else
                {
                    freelist = pf[pages - 1].next;
                }
                // fixup the tag for each frame
                for (n = 0; n < pages; n++)
                {
                    pf[n].tag = tag;
                    pf[n].next = NULL;
                }

                freemem -= pages;
                return pf - pfdb;
            }
        }

        prevpf = pf;
        pf = pf->next;
    }

    return 0xFFFFFFFF;
}


void kpframe_free( uint32_t pfn )
{
    struct pageframe *pf;

    pf = pfdb + pfn;
    pf->tag = PFT_FREE;
    pf->next = freelist;
    freelist = pf;
    freemem++;
}


void kpframe_set_tag( void *addr, uint32_t len, uint32_t tag )
{
    char *vaddr = (char *) addr;
    char *vend = vaddr + len;

    while (vaddr < vend)
    {
        unsigned long pfn = virt2phys(vaddr) >> PAGESHIFT;
        pfdb[pfn].tag = tag;
        vaddr += PAGESIZE;
    }
}


int memmap_proc(struct proc_file *pf, void *arg)
{
    struct memmap *mm = &syspage->bootparams.memmap;
    int i;
    char tagname[5];

    for (i = 0; i < mm->count; i++)
    {
        uint32_t type = mm->entry[i].type;

        if (type == PFT_RAM || type == PFT_RESERVED || type == PFT_ACPI || type == PFT_NVS)
            kpframe_tag(type, tagname);
        else
            kpframe_tag(PFT_MEM, tagname);

        pprintf(pf, "0x%08x-0x%08x type %s %8d KB\n",
            (unsigned long) mm->entry[i].addr,
            (unsigned long) (mm->entry[i].addr + mm->entry[i].size) - 1,
            type,
            (unsigned long) mm->entry[i].size / 1024);
    }

    return 0;
}


void kpframe_tag( uint32_t tag, char *str )
{
    if (tag & 0xFF000000) *str++ = (char) ((tag >> 24) & 0xFF);
    if (tag & 0x00FF0000) *str++ = (char) ((tag >> 16) & 0xFF);
    if (tag & 0x0000FF00) *str++ = (char) ((tag >> 8) & 0xFF);
    if (tag & 0x000000FF) *str++ = (char) (tag & 0xFF);
    *str++ = 0;
}

int memusage_proc(struct proc_file *pf, void *arg)
{
    unsigned int num_memtypes = 0;
    struct { unsigned long tag; int pages; } memtype[MAX_MEMTAGS];
    unsigned long tag;
    unsigned int n;
    unsigned int m;

    for (n = 0; n < maxmem; n++)
    {
        tag = pfdb[n].tag;

        m = 0;
        while (m < num_memtypes && tag != memtype[m].tag) m++;

        if (m < num_memtypes)
        {
            memtype[m].pages++;
        }
        else
        if (m < MAX_MEMTAGS)
        {
            memtype[m].tag = tag;
            memtype[m].pages = 1;
            num_memtypes++;
        }
    }

    for (n = 0; n < num_memtypes; n++)
    {
        char tagname[5];
        kpframe_tag(memtype[n].tag, tagname);
        pprintf(pf, "%-4s    %8d KB\n", tagname, memtype[n].pages * (PAGESIZE / 1024));
    }

    return 0;
}


int memstat_proc(struct proc_file *pf, void *arg)
{
    pprintf(pf, "Memory %dMB total, %dKB used, %dKB free, %dKB reserved\n",
        maxmem * PAGESIZE / (1024 * 1024),
        (totalmem - freemem) * PAGESIZE / 1024,
        freemem * PAGESIZE / 1024, (maxmem - totalmem) * PAGESIZE / 1024);

    return 0;
}


int physmem_proc(struct proc_file *pf, void *arg)
{
    unsigned int n;
    char tagname[5];

    for (n = 0; n < maxmem; n++)
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
                kpframe_tag(pfdb[n].tag, tagname);
                pprintf(pf, "%c", *tagname);
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
    struct pageframe *pf;
    struct memmap *memmap;

    // register page directory
    kmach_register_page_dir(virt2pfn(pdir));

    // calculates number of pages needed for page frame database
    memend = syspage->ldrparams.memend;
    heap = syspage->ldrparams.heapend;
    pfdbpages = PAGES((memend / PAGESIZE) * sizeof(struct pageframe));
    if ((pfdbpages + 2) * PAGESIZE + heap >= memend) panic("not enough memory for page table database");
    kprintf("[DEBUG] PFDB requires %d pages\n", pfdbpages);
    // intialize page tables for mapping the largest possible PFDB into kernel space
    // (for a machine with 4GB of physical RAM we need 2048 pages for PFDB)
    kmach_set_page_dir_entry(&pdir[PDEIDX(PFDBBASE)], heap | PT_PRESENT | PT_WRITABLE);
    kmach_set_page_dir_entry(&pdir[PDEIDX(PFDBBASE) + 1], (heap + PAGESIZE) | PT_PRESENT | PT_WRITABLE);
    pt = (pte_t *) heap;
    heap += 2 * PAGESIZE;
    memset(pt, 0, 2 * PAGESIZE);
    kmach_register_page_table(BTOP(pt));
    kmach_register_page_table(BTOP(pt) + 1);
    // allocate and map pages for page frame database
    for (i = 0; i < pfdbpages; i++)
    {
        kmach_set_page_table_entry(&pt[i], heap | PT_PRESENT | PT_WRITABLE);
        heap += PAGESIZE;
    }

    // initialize page frame database
    maxmem = syspage->ldrparams.memend / PAGESIZE;
    totalmem = 0;
    freemem = 0;
    pfdb = (struct pageframe *) PFDBBASE;
    memset(pfdb, 0, pfdbpages * PAGESIZE);
    for (i = 0; i < maxmem; i++) pfdb[i].tag = PFT_BAD;
    // add all memory from memory map to PFDB
    memmap = &syspage->bootparams.memmap;
    for (i = 0; i < (unsigned long) memmap->count; i++)
    {
        unsigned long first = (unsigned long) memmap->entry[i].addr / PAGESIZE;
        unsigned long last = first + (unsigned long) memmap->entry[i].size / PAGESIZE;

        if (first >= maxmem) continue;
        if (last >= maxmem) last = maxmem;

        if (memmap->entry[i].type == MEMTYPE_RAM)
        {
            for (j = first; j < last; j++) pfdb[j].tag = PFT_FREE;
            totalmem += (last - first);
        }
        else
        if (memmap->entry[i].type == MEMTYPE_RESERVED)
        {
            for (j = first; j < last; j++) pfdb[j].tag = PFT_RESERVED;
        }
    }
    // reserve physical page 0 for BIOS
    pfdb[0].tag = PFT_RESERVED;
    totalmem--;
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
    pf = pfdb + maxmem;
    do {
        pf--;

        if (pf->tag == PFT_FREE)
        {
            pf->next = freelist;
            freelist = pf;
            freemem++;
        }
    } while (pf > pfdb);
}
