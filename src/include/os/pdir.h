//
// pdir.h
//
// Page directory routines
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

#ifndef MACHINA_PDIR_H
#define MACHINA_PDIR_H


typedef unsigned long pte_t;

#define PAGESIZE       4096
#define PAGESHIFT      12
#define PTES_PER_PAGE  (PAGESIZE / sizeof(pte_t))

#define PT_PRESENT   0x001
#define PT_WRITABLE  0x002
#define PT_USER      0x004
#define PT_ACCESSED  0x020
#define PT_DIRTY     0x040

#define PT_GUARD     0x200
#define PT_FILE      0x400

#define PT_USER_READ    (PT_PRESENT | PT_USER)
#define PT_USER_WRITE   (PT_PRESENT | PT_USER | PT_WRITABLE)

#define PT_FLAGMASK     (PT_PRESENT | PT_WRITABLE | PT_USER | PT_ACCESSED | PT_DIRTY | PT_GUARD | PT_FILE)
#define PT_PROTECTMASK  (PT_WRITABLE | PT_USER | PT_GUARD)

#define PT_PFNMASK   0xFFFFF000
#define PT_PFNSHIFT  12

/**
 * Return the page directory entry index for given virtual address.
 *
 * The page directory can reference up to 1024 page tables. We divide the memory
 * in groups of 4MiB (each PT can map up to 1024 pages of 4KiB). This macro returns the
 * entry of the PD that point to the PT which maps the 4 MiB block address space where
 * the given address is whithin.
 */
#define PDEIDX(vaddr)  (((unsigned long) vaddr) >> 22)

/**
 * Return the page table entry index for given virtual address.
 */
#define PTEIDX(vaddr)  ((((unsigned long) vaddr) >> 12) & 0x3FF)

#define PGOFF(vaddr)   (((unsigned long) vaddr) & 0xFFF)
#define PTABIDX(vaddr) (((unsigned long) vaddr) >> 12)

/**
 * Return the number of pages it's necessary to store the given amount of bytes.
 */
#define PAGES(x) (((unsigned long)(x) + (PAGESIZE - 1)) >> PAGESHIFT)

/**
 * Return the page address which the given address is within.
 */
#define PAGEADDR(x) ((unsigned long)(x) & ~(PAGESIZE - 1))

#define PTOB(x) ((unsigned long)(x) << PAGESHIFT)
#define BTOP(x) ((unsigned long)(x) >> PAGESHIFT)

#define SET_PDE(vaddr, val) set_page_dir_entry(&pdir[PDEIDX(vaddr)], (val))
#define SET_PTE(vaddr, val) set_page_table_entry(&ptab[PTABIDX(vaddr)], (val))

/**
 * Returns the page directory entry for the given virtal address.
 */
#define GET_PDE(vaddr) (pdir[PDEIDX(vaddr)])

/**
 * Returns the page table entry for the given virtal address.
 */
#define GET_PTE(vaddr) (ptab[PTABIDX(vaddr)])

struct pdirstat
{
    int present;
    int user;
    int kernel;
    int readonly;
    int readwrite;
    int accessed;
    int dirty;
};

#include <os/krnl.h>
#include <sys/types.h>
#include <os/procfs.h>

#ifdef KERNEL

extern pte_t *pdir;
extern pte_t *ptab;

KERNELAPI void kpage_map(void *vaddr, unsigned long pfn, unsigned long flags);
KERNELAPI void kpage_unmap(void *vaddr);
KERNELAPI unsigned long virt2phys(void *vaddr);
KERNELAPI unsigned long virt2pfn(void *vaddr);
KERNELAPI pte_t kpage_get_flags(void *vaddr);
KERNELAPI void kpage_set_flags(void *vaddr, unsigned long flags);
KERNELAPI int kpage_is_mapped(void *vaddr);
KERNELAPI int kpage_is_directory_mapped(void *vaddr);
KERNELAPI void kpage_unguard(void *vaddr);
KERNELAPI void kpage_clear_dirty(void *vaddr);

KERNELAPI int mem_access(void *vaddr, int size, pte_t access);
KERNELAPI int str_access(char *s, pte_t access);

void init_pdir();
int pdir_proc(struct proc_file *pf, void *arg);
int virtmem_proc(struct proc_file *pf, void *arg);
int pdir_stat(void *addr, int len, struct pdirstat *buf);

#endif  // KERNEL

#endif  // MACHINA_PDIR_H
