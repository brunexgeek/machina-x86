//
// osloader.c
//
// Operating system loader
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

#include <os.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>

#include <os/pdir.h>
#include <os/tss.h>
#include <os/seg.h>
#include <os/syspage.h>
#include <os/timer.h>
#include <os/fpu.h>
#include <os/object.h>
#include <os/sched.h>
#include <os/dfs.h>
#include <os/dev.h>

#define VIDEO_BASE           0xB8000
#define HEAP_START           (1024 * 1024)


const char *MEM_TYPE_NAMES[] =
{
    "???",
    "RAM",
    "RESV",
    "ACPI",
    "NVS"
};


const char *MEM_UNIT_NAMES[] =
{
    "B",
    "KiB",
    "MiB",
    "GiB"
};


unsigned long mem_end = 0;      // Size of memory
char *heap;                     // Heap pointer point to next free page in memory
pte_t *pdir;                    // Page directory
struct syspage *syspage;        // System page with tss and gdt
pte_t *syspagetable;            // System page table
struct tcb *inittcb;            // Initial thread control block for kernel
unsigned long krnlentry;        // Virtual address of kernel entry point
unsigned short tssval;          // TSS selector value
unsigned short ldtnull;         // LDT null selector
struct selector gdtsel;         // GDT selector
struct selector idtsel;         // IDT selector
int bootdrive;                  // Boot drive
int bootpart;                   // Boot partition
char *krnlopts;                 // Kernel options
char *initrd;                   // Initial RAM disk location in heap
int initrd_size;                // Initial RAM disk size (in bytes)
int bootdev_cyls;               // Boot device cylinders
int bootdev_heads;              // Boot device heads
int bootdev_sects;              // Boot device sector per track

int vsprintf(char *buf, const char *fmt, va_list args);
void bios_print_string(char *str);
int bios_get_drive_params(int drive, int *cyls, int *heads, int *sects);
int bios_read_disk(int drive, int cyl, int head, int sect, int nsect, void *buffer);
int unzip(void *src, unsigned long srclen, void *dst, unsigned long dstlen, char *heap, int heapsize);
void load_kernel();


void kprintf(const char *fmt,...) {
  va_list args;
  char buffer[1024];

  va_start(args, fmt);
  vsprintf(buffer, fmt, args);
  va_end(args);

  bios_print_string(buffer);
}


void panic( char *message )
{
    kprintf("panic: %s\n", message);
    __asm__
    (
        "cli;"
        "hlt;"
    );
}


void init_biosdisk() {
  int status;

  status = bios_get_drive_params(bootdrive, &bootdev_cyls, &bootdev_heads, &bootdev_sects);
  if (status != 0) panic("Unable to initialize boot device");
}


int biosdisk_read(void *buffer, size_t count, blkno_t blkno){
  static char scratch[4096];

  char *buf = buffer;
  int sects = count / SECTORSIZE;
  int left = sects;
  while (left > 0) {
    int nsect, status;

    // Compute CHS address
    int sect = blkno % bootdev_sects + 1;
    int track = blkno / bootdev_sects;
    int head = track % bootdev_heads;
    int cyl = track / bootdev_heads;
    if (cyl >= bootdev_cyls) return -ERANGE;

    // Compute number of sectors to read
    nsect = left;
    if (nsect > sizeof(scratch) / SECTORSIZE) nsect = sizeof(scratch) / SECTORSIZE;
    if (nsect > 0x7f) nsect = 0x7f;
    if ((sect - 1) + nsect > bootdev_sects) nsect = bootdev_sects - (sect - 1);

    // Read sectors from disk into temporary buffer in low memory.
    status = bios_read_disk(bootdrive, cyl, head, sect, nsect, scratch);
    if (status != 0) {
      kprintf("biosdisk: read error %d\n", status);
      return -EIO;
    }

    // Move data to high memory
    memcpy(buf, scratch, nsect * SECTORSIZE);

    // Prepeare next read
    blkno += nsect;
    buf += nsect * SECTORSIZE;
    left -= nsect;
  }

  return count;
}

int bootrd_read(void *buffer, size_t count, blkno_t blkno) {
  memcpy(buffer, initrd + blkno * SECTORSIZE, count);
  return count;
}

int boot_read(void *buffer, size_t count, blkno_t blkno) {
  if ((bootdrive & 0xF0) == 0xF0) {
    return bootrd_read(buffer, count, blkno);
  } else {
    return biosdisk_read(buffer, count, blkno);
  }
}

char *heap_check( int numpages )
{
    // check for out of memory
    if ((unsigned long) heap + numpages * PAGESIZE >= mem_end)
        panic("out of memory");
    return heap;
}


char *heap_alloc( int numpages )
{
    char *p = heap_check(numpages);

    // initialize the pages
    memset(p, 0, numpages * PAGESIZE);
    // update heap pointer
    heap += numpages * PAGESIZE;

    return p;
}


void mmap_print( struct memmap *memmap )
{
    unsigned int type;
    unsigned int i;
    unsigned int size;
    unsigned int unit;

    if (memmap == NULL) return;

    kprintf("System memory map:\n");
    for (i = 0; i < (uint8_t)memmap->count; i++)
    {
        type = memmap->entry[i].type;
        if (type >= MEMTYPE_RAM && type <= MEMTYPE_NVS) type = 0;

        size = (uint32_t)memmap->entry[i].size;
        unit = 0;
        while (size > 1024)
        {
            size /= 1024;
            unit++;
        }

        kprintf("   From 0x%08X to 0x%08X %-4s (%d %s)\n",
            (uint32_t)memmap->entry[i].addr,
            (uint32_t)memmap->entry[i].addr + (uint32_t)memmap->entry[i].size,
            MEM_TYPE_NAMES[memmap->entry[i].type],
            size,
            MEM_UNIT_NAMES[unit] );
    }
}


void mmap_setup( struct memmap *memmap )
{
    int i;

    if (memmap->count != 0)
    {
        // determine largest available RAM address from BIOS memory map
        mem_end = 0;
        for (i = 0; i < memmap->count; i++)
        {
            if (memmap->entry[i].type == MEMTYPE_RAM)
            {
                mem_end = (unsigned long) (memmap->entry[i].addr + memmap->entry[i].size);
            }
        }
    }
    else
    {
        panic("BIOS too old! This OS requires a BIOS that support INT 15h E820h.");
    }
}


void setup_descriptors()
{
    struct syspage *syspage;
    struct tss *tss;

    // Get syspage virtual address
    syspage = (struct syspage *) SYSPAGE_ADDRESS;

    // Initialize tss
    tss = &syspage->tss;
    tss->cr3 = (unsigned long) pdir;
    tss->eip = krnlentry;
    tss->cs = SEL_KTEXT;
    tss->ss0 =  SEL_KDATA;
    tss->ds =  SEL_KDATA;
    tss->es =  SEL_KDATA;
    tss->ss = SEL_KDATA;
    tss->esp = INITTCB_ADDRESS + TCBESP;
    tss->esp0 = INITTCB_ADDRESS + TCBESP;

    // Setup kernel text segment (4GB read and execute, ring 0)
    seginit(&syspage->gdt[GDT_KTEXT], 0, 0x100000, D_CODE | D_DPL0 | D_READ | D_PRESENT, D_BIG | D_BIG_LIM);

    // Setup kernel data segment (4GB read and write, ring 0)
    seginit(&syspage->gdt[GDT_KDATA], 0, 0x100000, D_DATA | D_DPL0 | D_WRITE | D_PRESENT, D_BIG | D_BIG_LIM);

    // Setup user text segment (2GB read and execute, ring 3)
    seginit(&syspage->gdt[GDT_UTEXT], 0, BTOP(OSBASE), D_CODE | D_DPL3 | D_READ | D_PRESENT, D_BIG | D_BIG_LIM);

    // Setup user data segment (2GB read and write, ring 3)
    seginit(&syspage->gdt[GDT_UDATA], 0, BTOP(OSBASE), D_DATA | D_DPL3 | D_WRITE | D_PRESENT, D_BIG | D_BIG_LIM);

    // Setup TSS segment
    seginit(&syspage->gdt[GDT_TSS], (unsigned long) &syspage->tss, sizeof(struct tss), D_TSS | D_DPL0 | D_PRESENT, 0);

    // Setup TIB segment
    seginit(&syspage->gdt[GDT_TIB], 0, PAGESIZE, D_DATA | D_DPL3 | D_WRITE | D_PRESENT, 0);

    // Set GDT to the new segment descriptors
    gdtsel.limit = (sizeof(struct segment) * MAXGDT) - 1;
    gdtsel.dt = syspage->gdt;
    __asm__
    (
        "lgdt %0"
        :
        : "m" (gdtsel)
    );

    // Make all LDT selectors invalid
    ldtnull = 0;
    __asm__
    (
        "lldt %0"
        :
        : "m" (ldtnull)
    );

    // Set IDT to IDT in syspage
    idtsel.limit = (sizeof(struct gate) * MAXIDT) - 1;
    idtsel.dt = syspage->idt;
    __asm__
    (
        "lidt %0"
        :
        : "m" (idtsel)
    );

    // Load task register with new TSS segment
    tssval = SEL_TSS;
    __asm__
    (
        "ltr %0"
        :
        : "m" (tssval)
    );
}

void copy_ramdisk(char *bootimg) {
    struct superblock *super = (struct superblock *) (bootimg + SECTORSIZE);
    int i;
    int rdpages;

    // Copy ram disk to heap
    if (!bootimg) panic("no boot image");
    if (super->signature != DFS_SIGNATURE) panic("invalid DFS signature on initial RAM disk");

    initrd_size = (1 << super->log_block_size) * super->block_count;
    rdpages = PAGES(initrd_size);
    initrd = heap_alloc(rdpages);

    if (super->compress_size != 0)
    {
        char *zheap;
        unsigned int zofs;
        unsigned int zsize;

        kprintf("[DEBUG] uncompressing boot image\n");

        // Copy uncompressed part of image
        memcpy(initrd, bootimg, super->compress_offset);

        // Uncompress compressed part of image
        zheap = heap_check(64 * 1024 / PAGESIZE);
        zofs = super->compress_offset;
        zsize = super->compress_size;
        unzip(bootimg + zofs, zsize, initrd + zofs, initrd_size - zofs, zheap, 64 * 1024);
    }
    else
    {
        memcpy(initrd, bootimg, initrd_size);
    }

    // Map initial initial ram disk into syspages
    for (i = 0; i < rdpages; i++)
        syspagetable[PTEIDX(INITRD_ADDRESS) + i] = ((unsigned long) initrd + i * PAGESIZE) | PT_PRESENT | PT_WRITABLE;

    kprintf("[DEBUG] %d KB boot image found\n", initrd_size / 1024);
}


__attribute__((section("entryp"))) void __attribute__((stdcall)) start(void *hmod, struct bootparams *bootparams, int reserved)
{
    pte_t *pt;
    int i;
    char *bootimg = bootparams->bootimg;

    kprintf("\n");

    // create the memory mapping
    mmap_setup(&bootparams->memmap);
    mmap_print(&bootparams->memmap);

    // page allocation starts at 1MB
    heap = (char *) HEAP_START;
    // allocate page for page directory
    pdir = (pte_t *) heap_alloc(1);
    // make recursive entry for access to page tables
    pdir[PDEIDX(PTBASE)] = (unsigned long) pdir | PT_PRESENT | PT_WRITABLE;

    // allocate system page
    syspage = (struct syspage *) heap_alloc(1);
    // allocate initial thread control block
    inittcb = (struct tcb *) heap_alloc(PAGES_PER_TCB);
    // allocate a page for system page table
    syspagetable = (pte_t *) heap_alloc(1);

    // map system page, system page table and video buffer
    pdir[PDEIDX(SYSBASE)] = (unsigned long) syspagetable | PT_PRESENT | PT_WRITABLE;
    syspagetable[PTEIDX(SYSPAGE_ADDRESS)] = (unsigned long) syspage | PT_PRESENT | PT_WRITABLE;
    syspagetable[PTEIDX(PAGEDIR_ADDRESS)] = (unsigned long) pdir | PT_PRESENT | PT_WRITABLE;
    syspagetable[PTEIDX(VIDBASE_ADDRESS)] = VIDEO_BASE | PT_PRESENT | PT_WRITABLE;
    // map initial TCB
    for (i = 0; i < PAGES_PER_TCB; i++)
    {
        syspagetable[PTEIDX(INITTCB_ADDRESS) + i] = ((unsigned long) inittcb + i * PAGESIZE) | PT_PRESENT | PT_WRITABLE;
    }

    // Map first 4MB to physical memory
    pt = (pte_t *) heap_alloc(1);
    pdir[0] = (unsigned long) pt | PT_PRESENT | PT_WRITABLE | PT_USER;
    for (i = 0; i < PTES_PER_PAGE; i++) pt[i] = (i * PAGESIZE) | PT_PRESENT | PT_WRITABLE;

    // Copy kernel from boot device
    bootdrive = bootparams->bootdrv;
    if ((bootdrive & 0xF0) == 0xF0)
    {
        copy_ramdisk(bootimg);
        load_kernel(bootdrive);
    }
    else
    {
        init_biosdisk();
        load_kernel(bootdrive);
    }

    // Set page directory (CR3) and enable paging (PG bit in CR0)
    __asm__
    (
        "mov eax, dword ptr %0;"
        "mov cr3, eax;"
        "mov eax, cr0;"
        "or eax, 0x80000000;"
        "mov cr0, eax;"
        :
        : "m" (pdir)
    );

    // setup new descriptors in syspage
    setup_descriptors();

    // setup boot parameters
    memcpy(&syspage->bootparams, bootparams, sizeof(struct bootparams));
    syspage->ldrparams.heapstart = HEAP_START;
    syspage->ldrparams.heapend = (unsigned long) heap;
    syspage->ldrparams.memend = mem_end;
    syspage->ldrparams.bootdrv = bootdrive;
    syspage->ldrparams.bootpart = bootpart;
    syspage->ldrparams.initrd_size = initrd_size;
    memcpy(syspage->biosdata, (void *) 0x0400, 256);

    // kernel options are located in the boot parameter block
    krnlopts = syspage->bootparams.krnlopts;

    // Reload segment registers
    __asm__
    (
        "mov ax, %1;"
        "mov ds, ax;"
        "mov es, ax;"
        "mov fs, ax;"
        "mov gs, ax;"
        "mov ss, ax;"
        "push %0;"
        "push offset startpg;"
        "retf;"
        "startpg:"
        :
        : "i" (SEL_KTEXT), "i" (SEL_KDATA)
    );

    // Switch to inital kernel stack and jump to kernel
    __asm__
    (
        "mov esp, %0 + %1;"
        "push 0;"
        "push dword ptr [krnlopts];"
        "push %2;"
        //"call dword ptr [krnlentry];"
        //"mov eax, %2;"
        "call eax;"
        "cli;"
        "hlt;"
        :
        : "i" (INITTCB_ADDRESS), "i" (TCBESP), "a" (OSBASE)
    );
}
