//
// pdir.c
//
// Page directory management
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

#include <os/pdir.h>
#include <os/procfs.h>
#include <os/syspage.h>
#include <os/pframe.h>
#include <os/vmm.h>


/**
 * Pointer to page directory.
 */
pte_t *pdir = (pte_t *) PAGEDIR_ADDRESS;

/**
 * Pointer to pabe tables.
 */
pte_t *ptab = (pte_t *) PTBASE;

/**
 * Pointer to frame array (dynamically allocated).
 *
 * @remarks Defined at @ref pframe.c
 */
extern uint16_t *frameArray;


void kpage_initialize()
{
    uint32_t i;

    // Clear identity mapping of the first 4 MB made by the os loader
    for (i = 0; i < PTES_PER_PAGE; i++) SET_PTE(PTOB(i), 0);
}


void kpage_map(
    void *vaddress,
    uint32_t frame,
    uint32_t flags )
{
    // allocate page table if not already done
    if ((GET_PDE(vaddress) & PT_PRESENT) == 0)
    {
        //kprintf("Creating page table for 0x%08x (idx: %d)\n", vaddress, PDEIDX(vaddress));
        uint32_t pdfn;

        pdfn = kpframe_alloc(1, PFT_PTAB);
        if (USERSPACE(vaddress))
        {
            SET_PDE(vaddress, PTOB(pdfn) | PT_PRESENT | PT_WRITABLE | PT_USER);
        }
        else
        {
            SET_PDE(vaddress, PTOB(pdfn) | PT_PRESENT | PT_WRITABLE);
        }

        memset(ptab + PDEIDX(vaddress) * PTES_PER_PAGE, 0, PAGESIZE);
        kmach_register_page_table(frame);
    }

    // map page frame into address space
    SET_PTE(vaddress, PTOB(frame) | flags);
}


void kpage_unmap(
    void *vaddress )
{
    SET_PTE(vaddress, 0);
    kmach_invlpage(vaddress);
}


uint32_t kpage_virt2phys(
    void *vaddress )
{
    return ((GET_PTE(vaddress) & PT_PFNMASK) + PGOFF(vaddress));
}


uint32_t kpage_virt2frame(
    void *vaddress )
{
    return BTOP(GET_PTE(vaddress) & PT_PFNMASK);
}


pte_t kpage_get_flags(
    void *vaddress )
{
    return GET_PTE(vaddress) & PT_FLAGMASK;
}


void kpage_set_flags(
    void *vaddress,
    uint32_t flags )
{
    SET_PTE(vaddress, (GET_PTE(vaddress) & PT_PFNMASK) | flags);
    kmach_invlpage(vaddress);
}


/**
 * Return a non-zero value if the given virtual address it's mapped to a page.
 */
int kpage_is_mapped(
    void *vaddress )
{
    if ((GET_PDE(vaddress) & PT_PRESENT) == 0) return 0;
    if ((GET_PTE(vaddress) & PT_PRESENT) == 0) return 0;
    return 1;
}


/**
 * Return a non-zero value if the given virtual address it's mapped to a page table.
 */
int kpage_is_directory_mapped(
    void *vaddress )
{
    return (GET_PDE(vaddress) & PT_PRESENT) != 0;
}


void kpage_unguard(
    void *vaddress )
{
    SET_PTE(vaddress, (GET_PTE(vaddress) & ~PT_GUARD) | PT_USER);
    kmach_invlpage(vaddress);
}


void kpage_clear_dirty(
    void *vaddress )
{
    SET_PTE(vaddress, GET_PTE(vaddress) & ~PT_DIRTY);
    kmach_invlpage(vaddress);
}


int mem_access(
    void *vaddress,
    int size,
    pte_t access)
{
    uint32_t addr;
    uint32_t next;
    pte_t pte;

    addr = (uint32_t) vaddress;
    next = (addr & ~PAGESIZE) + PAGESIZE;
    while (1)
    {
        if ((GET_PDE(vaddress) & PT_PRESENT) == 0) return 0;
        pte = GET_PTE(vaddress);
        if ((pte & access) != access)
        {
            /*if (pte & PT_FILE)
            {
                if (fetch_page((void *) PAGEADDR(addr)) < 0) return 0;
                if ((GET_PTE(addr) & access) != access) return 0;
            }
            else*/
            {
                return 0;
            }
        }

        size -= next - addr;
        if (size <= 0) break;
        addr = next;
        next += PAGESIZE;
    }

    return 1;
}


int str_access(
    char *s,
    pte_t access)
{
    pte_t pte;

    while (1)
    {
        if ((GET_PDE(s) & PT_PRESENT) == 0) return 0;
        pte = GET_PTE(s);
        if ((pte & access) != access) {
            /*if (pte & PT_FILE)
            {
                if (fetch_page((void *) PAGEADDR(s)) < 0) return 0;
                if ((GET_PTE(s) & access) != access) return 0;
            }
            else*/
            {
                return 0;
            }
        }

        while (1)
        {
            if (!*s) return 1;
            s++;
            if (PGOFF(s) == 0) break;
        }
    }
}


int proc_pdir(
    struct proc_file *pf,
    void *arg )
{
    char *vaddress;
    pte_t pte;

    int ma = 0;
    int us = 0;
    int su = 0;
    int ro = 0;
    int rw = 0;
    int ac = 0;
    int dt = 0;
    int gd = 0;
    int fi = 0;

    pprintf(pf, "virtaddr physaddr flags\n");
    pprintf(pf, "-------- -------- ------\n");

    vaddress = NULL;
    while (1)
    {
        if ((GET_PDE(vaddress) & PT_PRESENT) == 0) {
            vaddress += PTES_PER_PAGE * PAGESIZE;
    }
    else
    {
        pte = GET_PTE(vaddress);
        if (pte & PT_PRESENT)
        {
            ma++;

            if (pte & PT_WRITABLE)
                rw++;
            else
                ro++;

            if (pte & PT_USER)
                us++;
            else
                su++;

            if (pte & PT_ACCESSED) ac++;
            if (pte & PT_DIRTY) dt++;
            if (pte & PT_GUARD) gd++;
            if (pte & PT_FILE) fi++;

            pprintf(pf, "%08x %08x %c%c%c%c%c%c\n",
                vaddress, PAGEADDR(pte),
                (pte & PT_WRITABLE) ? 'w' : 'r',
                (pte & PT_USER) ? 'u' : 's',
                (pte & PT_ACCESSED) ? 'a' : ' ',
                (pte & PT_DIRTY) ? 'd' : ' ',
                (pte & PT_GUARD) ? 'g' : ' ',
                (pte & PT_FILE) ? 'f' : ' ');
        }

        vaddress += PAGESIZE;
    }

    if (!vaddress) break;
    }

    pprintf(pf, "\ntotal:%d usr:%d sys:%d rw: %d ro: %d acc: %d dirty: %d guard:%d file:%d\n", ma, us, su, rw, ro, ac, dt, gd, fi);
    return 0;
}


static void kpage_print_virtmem(
    struct proc_file *output,
    char *start,
    char *end,
    uint32_t tag )
{
    const char *name;
    name = kpframe_tag_name(tag);
    pprintf(output, "%08x %08x %8dK %-4s\n", start, end - 1, (end - start) / 1024, name);
}


int proc_virtmem(
    struct proc_file *output,
    void *arg)
{
    char *vaddress;
    char *start;
    uint32_t curtag;
    int total = 0;

    pprintf(output, "start    end           size type\n");
    pprintf(output, "-------- -------- --------- ----\n");

    start = vaddress = NULL;
    curtag = 0;
    while (1)
    {
        if ((GET_PDE(vaddress) & PT_PRESENT) == 0)
        {
            if (start != NULL)
            {
                kpage_print_virtmem(output, start, vaddress, curtag);
                start = NULL;
            }

            vaddress += PTES_PER_PAGE * PAGESIZE;
        }
        else
        {
            pte_t pte = GET_PTE(vaddress);
            //uint32_t tag = pfdb[pte >> PT_PFNSHIFT].tag;
            uint32_t tag = PFRAME_GET_TAG( pte >> PT_PFNSHIFT );

            if (pte & PT_PRESENT)
            {
                if (start == NULL)
                {
                    start = vaddress;
                    curtag = tag;
                }
                else
                if (tag != curtag)
                {
                    kpage_print_virtmem(output, start, vaddress, curtag);
                    start = vaddress;
                    curtag = tag;
                }

                total += PAGESIZE;
            }
            else
            {
                if (start != NULL)
                {
                    kpage_print_virtmem(output, start, vaddress, curtag);
                    start = NULL;
                }
            }

            vaddress += PAGESIZE;
        }

        if (!vaddress) break;
    }

    if (start) kpage_print_virtmem(output, start, vaddress, curtag);
    pprintf(output, "total             %8dK\n", total / 1024);

    return 0;
}


int pdir_stat(
    void *addr,
    int len,
    struct pdirstat *buf )
{
    char *vaddress;
    char *end;
    pte_t pte;

    memset(buf, 0, sizeof(struct pdirstat));
    vaddress = (char *) addr;
    end = vaddress + len;
    while (vaddress < end) {
        if ((GET_PDE(vaddress) & PT_PRESENT) == 0)
        {
            vaddress += PTES_PER_PAGE * PAGESIZE;
            vaddress = (char *) ((uint32_t) vaddress & ~(PTES_PER_PAGE * PAGESIZE - 1));
        }
        else
        {
            pte = GET_PTE(vaddress);
            if (pte & PT_PRESENT)
            {
                buf->present++;

                if (pte & PT_WRITABLE)
                    buf->readwrite++;
                else
                    buf->readonly++;

                if (pte & PT_USER)
                    buf->user++;
                else
                    buf->kernel++;

                if (pte & PT_ACCESSED) buf->accessed++;
                if (pte & PT_DIRTY) buf->dirty++;
            }

            vaddress += PAGESIZE;
        }
    }

    return 0;
}
