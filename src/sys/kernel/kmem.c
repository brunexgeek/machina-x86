//
// kmem.c
//
// Kernel memory page allocator
//
// Copyright (C) 2013-2014 Bruno Ribeiro.
// Copyright (C) 2002 Michael Ringgaard.
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

#include <os/kmem.h>

#define OSVMAP_PAGES 1
#define OSVMAP_ENTRIES (OSVMAP_PAGES * PAGESIZE / sizeof(struct rmap_t))

#define KMODMAP_PAGES 1
#define KMODMAP_ENTRIES (KMODMAP_PAGES * PAGESIZE / sizeof(struct rmap_t))

struct rmap_t *osvmap = (struct rmap_t *) OSVMAP_ADDRESS;
struct rmap_t *kmodmap = (struct rmap_t *) KMODMAP_ADDRESS;


void *kmem_alloc( int pages, uint8_t tag )
{
    char *vaddr;
    int i;
    uint32_t index;

    if (tag == 0) tag = PFT_KMEM;
    vaddr = (char *) PTOB(krmap_alloc(osvmap, pages));
    for (i = 0; i < pages; i++)
    {
        index = kpframe_alloc(1, tag);
        kpage_map(vaddr + PTOB(i), index, PT_WRITABLE | PT_PRESENT);
    }

    //kprintf("alloc kmem %dK @ %p (%d KB free)\n", pages * (PAGESIZE / K), vaddr, freemem * (PAGESIZE / K));
    return vaddr;
}


void *kmem_alloc_align(int pages, int align, uint8_t tag)
{
    char *vaddr;
    int i;
    uint32_t index;

    if (tag == 0) tag = PFT_KMEM;
    vaddr = (char *) PTOB(krmap_alloc_align(osvmap, pages, align));
    for (i = 0; i < pages; i++)
    {
        index = kpframe_alloc(1, tag);
        kpage_map(vaddr + PTOB(i), index, PT_WRITABLE | PT_PRESENT);
    }

    //kprintf("alloc kmem %dK @ %p (align %dK, %d KB free)\n", pages * (PAGESIZE / K), vaddr, align * (PAGESIZE / K), freemem * (PAGESIZE / K));
    return vaddr;
}


void *kmem_alloc_linear( int pages, uint8_t tag )
{
    char *vaddr;
    int i;
    uint32_t index;

    if (tag == 0) tag = PFT_KMEM;
    index = kpframe_alloc(pages, tag);
    if (index == INVALID_PFRAME) return NULL;
    vaddr = (char *) PTOB(krmap_alloc(osvmap, pages));
    for (i = 0; i < pages; i++)
    {
        kpage_map(vaddr + PTOB(i), index, PT_WRITABLE | PT_PRESENT);
        index++;
    }

    //kprintf("alloc kmem linear %dK @ %p (%d KB free)\n", pages * (PAGESIZE / K), vaddr, freemem * (PAGESIZE / K));

    return vaddr;
}


void kmem_free( void *addr, int pages )
{
    int i;
    unsigned long pfn;

    //kprintf("free kmem %dK @ %p\n", pages * PAGESIZE / K, addr);

    for (i = 0; i < pages; i++)
    {
        pfn = BTOP(kpage_virt2phys((char *) addr + PTOB(i)));
        kpframe_free(pfn);
        kpage_unmap((char *) addr + PTOB(i));
    }

    krmap_free(osvmap, BTOP(addr), pages);
}


void *iomap(unsigned long addr, int size)
{
    char *vaddr;
    int i;
    int pages = PAGES(size);

    vaddr = (char *) PTOB(krmap_alloc(osvmap, pages));
    for (i = 0; i < pages; i++)
        kpage_map(vaddr + PTOB(i), BTOP(addr) + i, PT_WRITABLE | PT_PRESENT);

    return vaddr;
}


void iounmap(void *addr, int size) {
  int i;
  int pages = PAGES(size);

  for (i = 0; i < pages; i++) kpage_unmap((char *) addr + PTOB(i));
  krmap_free(osvmap, BTOP(addr), pages);
}


void *kmem_alloc_module(
    int pages )
{
    char *vaddr;
    int i;
    unsigned long pfn;

    vaddr = (char *) PTOB(krmap_alloc(kmodmap, pages));
    for (i = 0; i < pages; i++)
    {
        pfn = kpframe_alloc(1, PFT_KMOD);
        kpage_map(vaddr + PTOB(i), pfn, PT_WRITABLE | PT_PRESENT);
        memset(vaddr + PTOB(i), 0, PAGESIZE);
    }

    //kprintf("alloc mod mem %dK @ %p\n", pages * PAGESIZE / K, vaddr);

    return vaddr;
}


void free_module_mem(void *addr, int pages)
{
    int i;
    unsigned long pfn;

    //kprintf("free mod mem %dK @ %p\n", pages * PAGESIZE / K, addr);

    for (i = 0; i < pages; i++)
    {
        pfn = BTOP(kpage_virt2phys((char *) addr + PTOB(i)));
        kpframe_free(pfn);
        kpage_unmap((char *) addr + PTOB(i));
    }

    krmap_free(kmodmap, BTOP(addr), pages);
}


void kmem_initialize()
{
    int pfn;
    struct image_header *imghdr;

    // allocate page frame for kernel heap resource map and map into syspages
    pfn = kpframe_alloc(1, PFT_SYS);
    kpage_map(osvmap, pfn, PT_WRITABLE | PT_PRESENT);
    // initialize resource map for kernel heap
    if (krmap_initialize(osvmap, OSVMAP_ENTRIES) != 0)
        panic("Error initializing kernel heap map");
    // add kernel heap address space to osvmap
    krmap_free(osvmap, BTOP(KHEAPBASE), BTOP(KHEAPSIZE));

    // allocate page frame for kernel module map and map into syspages
    pfn = kpframe_alloc(1, PFT_SYS);
    kpage_map(kmodmap, pfn, PT_WRITABLE | PT_PRESENT);
    // initialize resource map for kernel module area
    if (krmap_initialize(kmodmap, KMODMAP_ENTRIES) != 0)
        panic("Error initializing kernel modules heap map");

    // Add kernel heap address space to kmodmap
    // TODO: fix this!
    //imghdr = get_image_header((hmodule_t) OSBASE);
    //krmap_free(kmodmap, BTOP(OSBASE), BTOP(KMODSIZE));
    //krmap_reserve(kmodmap, BTOP(OSBASE), BTOP(imghdr->optional.size_of_image));
}

int list_memmap(struct proc_file *pf, struct rmap_t *rmap, unsigned int startpos)
{
    struct rmap_t *current;
    unsigned int pos = 0;
    struct pdirstat stat;

    pprintf(pf, "   start      end      size committed  readonly    status\n");
    pprintf(pf, "-------- -------- --------- --------- --------- ---------\n");

    current = rmap + 1;
    while (current != rmap)
    {
        //if (pos >= startpos)
        {
            pdir_stat((void *) (pos * PAGESIZE), current->size * PAGESIZE, &stat);
            pprintf(pf, "%08X %08X %8dK %8dK %8dK      %s\n",
                pos * PAGESIZE,
                (pos + current->size) * PAGESIZE - 1,
                current->size * (PAGESIZE / 1024),
                stat.present * (PAGESIZE / 1024),
                stat.readonly * (PAGESIZE / 1024),
                (current->used) ? "used" : "free");
        }
        pos += current->size;
        current = rmap + current->next;
    }

    krmap_get_entry_count(rmap, &pos);
    pprintf(pf, "Mapping fragments: %d of %d\n", pos, rmap->next);

    /*rlim = &rmap[rmap->offset];

    for (r = rmap; r <= rlim; r++)
    {
        kprintf("entry[%d] = { offset = %d, size = %d }\n", (uint32_t)(r-rmap), r->offset, r->size);
    }


    for (r = &rmap[1]; r <= rlim; r++)
    {
        unsigned int size = r->offset - pos;

        if (size > 0)
        {
            pdir_stat((void *) (pos * PAGESIZE), size * PAGESIZE, &stat);
            pprintf(pf, "%08X %08X %8dK %8dK %8dK %8dK\n",
                pos * PAGESIZE,
                r->offset * PAGESIZE - 1,
                size * (PAGESIZE / 1024),
                stat.present * (PAGESIZE / 1024),
                stat.readonly * (PAGESIZE / 1024),
                r->size * (PAGESIZE / 1024));

            total += size;
        }
        pos = r->offset + r->size;
    }
    pprintf(pf, "Total: %dK\n", total * PAGESIZE / 1024);*/
    return 0;
}

int kmem_proc(struct proc_file *pf, void *arg)
{
    return list_memmap(pf, osvmap, BTOP(KHEAPBASE));
}

int kmodmem_proc(struct proc_file *pf, void *arg)
{
    return list_memmap(pf, kmodmap, BTOP(OSBASE));
}
