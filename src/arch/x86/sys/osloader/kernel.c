//
// kernel.c
//
// Kernel loader
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

#include <os.h>
#include <os/pdir.h>
#include <os/tss.h>
#include <os/seg.h>
#include <os/syspage.h>
#include <os/mbr.h>
#include <os/dfs.h>
#include <os/pe.h>
#include <os/dev.h>
#include <os/elf.h>


void kprintf(const char *fmt,...);
void panic(char *msg);

extern unsigned long krnlentry;
extern int bootpart;

int boot_read(void *buffer, size_t count, blkno_t blkno);
char *heap_alloc(int numpages);

char bsect[SECTORSIZE];
char ssect[SECTORSIZE];
char gsect[SECTORSIZE];
char isect[SECTORSIZE];
blkno_t blockdir[1024];


/**
 * Read an ELF32 from the memory and expand it for execution.
 */
void kernel_load_elf32( void *image, char **start, uint32_t *size )
{
    elf32_file_header_t *header = (elf32_file_header_t*)image;
    elf32_sect_header_t *sheader;
    elf32_sect_header_t *first = NULL;
    uint32_t total = 0;
    int i;
    char *ptr, *memory;
    #ifndef NDEBUG
    const char *stringTable = NULL;
    #endif

    // check the ELF32 signature
    if (header->e_ident[0] != 0x7F || header->e_ident[1] != 0x45 ||
        header->e_ident[2] != 0x4C || header->e_ident[3] != 0x46)
        panic("Invalid ELF32 kernel!");

    sheader = (elf32_sect_header_t*) ((uint8_t*)image + header->e_shoff);

    #ifndef NDEBUG
    // look for the string table
    for (i = 0; i < header->e_shnum; ++i)
    {
        if (sheader[i].sh_type == SHT_STRTAB)
        {
            stringTable = (char*)image + sheader[i].sh_offset;
            break;
        }
    }
    kprintf("Name            Type Address    Size       Align\n");
    for (i = 0; i < header->e_shnum; ++i)
    {
        kprintf("%-15s %04d 0x%08p 0x%08x 0x%04x\n",
            stringTable + sheader[i].sh_name,
            sheader[i].sh_type,
            sheader[i].sh_addr,
            sheader[i].sh_size,
            sheader[i].sh_addr_align );
    }
    #endif

    // compute the amount of memory necessary to expand the kernel
    for (i = 0; i < header->e_shnum; ++i)
    {
        if (sheader[i].sh_type == SHT_PROGBITS)
        {
            if (total != 0)
                panic("Invalid Kernel ELF32 layout!");
            if (first == NULL) first = sheader + i;
        }
        else
        if (sheader[i].sh_type == SHT_NOBITS)
        {
            if (total != 0)
                panic("Invalid Kernel ELF32 layout!");
            total = sheader[i].sh_addr + sheader[i].sh_size;
        }
        else
            continue;
    }
    if (first == NULL) panic("Invalid Kernel ELF32 layout!");
    total -= first->sh_addr;
    *size = total;
    kprintf("This kernel requires %d bytes\n", total);

    // allocate memory to the kernel
    memory = heap_alloc(PAGES(total));

    // copy the content of each section
    #ifndef NDEBUG
    kprintf("Expanding image at [0x%p] to [0x%p]\n", image, memory);
    kprintf("Action Type     Size       From       To\n");
    kprintf("------ -------- ---------- ---------- ----------\n");
    #endif
    for (i = 0, ptr = memory; i < header->e_shnum; ++i)
    {
        if (sheader[i].sh_type == SHT_PROGBITS)
        {
            ptr =  sheader[i].sh_addr - first->sh_addr + memory;

            #ifndef NDEBUG
            kprintf("Copy   PROGBITS 0x%08x 0x%p 0x%p\n",
                sheader[i].sh_size,
                sheader[i].sh_offset,
                ptr - memory);
            #endif
            memcpy(ptr, image + sheader[i].sh_offset, sheader[i].sh_size);
        }
        else
        if (sheader[i].sh_type == SHT_NOBITS)
        {
            ptr =  sheader[i].sh_addr - first->sh_addr + memory;

            #ifndef NDEBUG
            kprintf("Clear  NOBITS   0x%08x N/A        0x%p\n",
                sheader[i].sh_size,
                sheader[i].sh_offset,
                ptr - memory);
            #endif
            memset(ptr, 0, sheader[i].sh_size);
        }
    }

    *start = memory;
    *size = total;
}


void kernel_load( int bootdrv )
{
    struct master_boot_record *mbr;
    struct superblock *sb;
    struct groupdesc *group;
    struct inodedesc *inode;
    int blocksize;
    int blks_per_sect;
    int kernelsize;
    int kernelPages;
    char *imageAddress;
    char *addr;
    uint32_t finalSize;
    char *finalAddress;
    blkno_t blkno;
    int i;
    int j;
    pte_t *pt;
    int start;
    char *label;
    struct boot_sector *bootsect;

    // determine active boot partition if booting from harddisk
    if (bootdrv & 0x80 && (bootdrv & 0xF0) != 0xF0)
    {
        mbr = (struct master_boot_record *) bsect;
        if (boot_read(mbr, SECTORSIZE, 0) != SECTORSIZE)
        {
            panic("unable to read master boot record");
        }

        if (mbr->signature != MBR_SIGNATURE) panic("invalid boot signature");

        bootsect = (struct boot_sector *) bsect;
        label = bootsect->label;
        if (label[0] == 'S' && label[1] == 'A' && label[2] == 'N' && label[3] == 'O' && label[4] == 'S')
        {
            // disk does not have a partition table
            start = 0;
            bootpart = -1;
        }
        else
        {
            // find active partition
            bootpart = -1;
            for (i = 0; i < 4; i++)
            {
                if (mbr->parttab[i].bootid == 0x80)
                {
                    bootpart = i;
                    start = mbr->parttab[i].relsect;
                }
            }

            if (bootpart == -1) panic("no bootable partition on boot drive");
        }
    }
    else
    {
        start = 0;
        bootpart = 0;
    }

    // read super block from boot device
    sb = (struct superblock *) ssect;
    if (boot_read(sb, SECTORSIZE, 1 + start) != SECTORSIZE)
    {
        panic("unable to read super block from boot device");
    }

    // check signature and version
    if (sb->signature != DFS_SIGNATURE) panic("invalid DFS signature");
    if (sb->version != DFS_VERSION) panic("invalid DFS version");
    blocksize = 1 << sb->log_block_size;
    blks_per_sect =  blocksize / SECTORSIZE;

    // read first group descriptor
    group = (struct groupdesc *) gsect;
    if (boot_read(group, SECTORSIZE, sb->groupdesc_table_block * blks_per_sect + start) != SECTORSIZE)
    {
        panic("unable to read group descriptor from boot device");
    }

    // read inode for kernel
    inode = (struct inodedesc *) isect;
    if (boot_read(isect, SECTORSIZE, group->inode_table_block * blks_per_sect + start) != SECTORSIZE)
    {
        panic("unable to read kernel inode from boot device");
    }
    inode += DFS_INODE_KRNL;

    // calculate kernel size
    kernelsize = (int) inode->size;
    kernelPages = PAGES(kernelsize);
    kprintf("[DEBUG] loading kernel at 0x%lx (%d KiB)\n", OSBASE, kernelsize / 1024);

    // allocate page table for kernel
    if (kernelPages > PTES_PER_PAGE) panic("kernel too big");
    pt = (pte_t *) heap_alloc(1);
    pdir[PDEIDX(OSBASE)] = (unsigned long) pt | PT_PRESENT | PT_WRITABLE;

    // allocate pages for boot image
    imageAddress = heap_alloc(kernelPages);
    // TODO: release image memory

    // read boot image from boot device
    if (inode->depth == 0)
    {
        addr = imageAddress;
        for (i = 0; i < (int) inode->blocks; i++)
        {
            if (boot_read(addr, blocksize, inode->blockdir[i] * blks_per_sect + start) != blocksize)
            {
                panic("error reading kernel from boot device");
            }
            addr += blocksize;
        }
    }
    else
    if (inode->depth == 1)
    {
        addr = imageAddress;
        blkno = 0;
        for (i = 0; i < DFS_TOPBLOCKDIR_SIZE; i++)
        {
            if (boot_read(blockdir, blocksize, inode->blockdir[i] * blks_per_sect + start) != blocksize)
            {
                panic("error reading kernel inode dir from boot device");
            }

            for (j = 0; j < (int) (blocksize / sizeof(blkno_t)); j++)
            {
                if (boot_read(addr, blocksize, blockdir[j] * blks_per_sect + start) != blocksize)
                {
                    panic("error reading kernel inode dir from boot device");
                }

                addr += blocksize;

                blkno++;
                if (blkno == inode->blocks) break;
            }

            if (blkno == inode->blocks) break;
        }
    }
    else
    {
        panic("unsupported inode depth");
    }

    // process the image as ELF binary
    kernel_load_elf32(imageAddress, &finalAddress, &finalSize);
    kernelPages = PAGES(finalSize);

    // map kernel into vitual address space
    for (i = 0; i < kernelPages; i++)
        pt[i] = (unsigned long) (finalAddress + i * PAGESIZE) | PT_PRESENT | PT_WRITABLE;
}
