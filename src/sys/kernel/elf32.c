//
// elf32.c
//
// ELF32 binary handling
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

#include <string.h>
#include <sys/mman.h>
#include <os/kmem.h>
#include <os/kmalloc.h>
#include <os/elf32.h>
#include <os/klog.h>


static const uint8_t ELF32_MAGIC[] =
{
    0x7f, 0x45, 0x4c, 0x46,  //  0x7f, 'E', 'L', 'F'
    0x01,                    //  Only 32-bit objects.
    0x01,                    //  Only LSB data.
    0x01                     //  Only ELF version 1.
};


static const char *ELF32_RELOCS[] =
{
    "R_386_NONE",
    "R_386_32",
    "R_386_PC32",
    "R_386_GOT32",
    "R_386_PLT32",
    "R_386_COPY",
    "R_386_GLOB_DAT",
    "R_386_JMP_SLOT",
    "R_386_RELATIVE"
};

#define ELF32_RELOCS_COUNT   ( sizeof(ELF32_RELOCS) / sizeof(char*) )


static void *elf32_resolve( const char* sym )
{
    static void *handle = NULL;

    return NULL;
}


static uint32_t elf32_relocateSymbol(
    char* image,
    const elf32_symb_t *symbol,
    elf32_relc_t *reloc,
    char *stringTable,
    uint32_t bias )
{
    uint32_t address;
    uint32_t *field;
    const char* symbolName;

    switch(ELF32_R_TYPE(reloc->r_info))
    {
        case R_386_JMP_SLOT:
        case R_386_GLOB_DAT:
            // check if the current symbol is inside the binary file
            if (symbol->st_value != 0)
                address = (uint32_t)image + symbol->st_value;
            else
            {
                symbolName = stringTable + symbol->st_name;
                address = (uint32_t)elf32_resolve(symbolName);
            }
            break;
        case R_386_32:
            address = (uint32_t)image + symbol->st_value;
            break;
        case R_386_PC32:
            field = (uint32_t*)(image + reloc->r_offset);
            // check if the current symbol is inside the binary file
            if (symbol->st_value != 0)
            {
                address =  *(uint32_t*)field;
                address = (int32_t)address + ((int32_t)image + symbol->st_value);
                address = (int32_t)address - (int32_t)field;
            }
            else
            {
                symbolName = stringTable + symbol->st_name;
                address =  *(uint32_t*)field;
                address = (int32_t)address + (int32_t)elf32_resolve(symbolName);
                address = (int32_t)address - (int32_t)field;
            }
            break;
        case R_386_RELATIVE:
            field = (uint32_t*)(image + reloc->r_offset);
            address = (uint32_t)image + *field;
            break;
        default:
            kprintf("Unsupported relocation type 0x%x\n", ELF32_R_TYPE(reloc->r_info));
            address = 0;
    }

    return address;
}


void elf32_relocate(
    char *start,
    char *image,
    elf32_file_header_t *header,
    elf32_symb_t *dynTable,
    char *stringTable,
    uint32_t bias )
{
    elf32_sect_header_t *sheader;
    elf32_relc_t *rheader;
    int i, j;
    const elf32_symb_t *symbol;
    uint32_t *slot;

    // perform the relocation
    sheader = (elf32_sect_header_t*)(start + header->e_shoff);
    for(i = 0; i < header->e_shnum; ++i)
    {
        if (sheader[i].sh_type != SHT_REL) continue;

        // iterate through the relocation table
        rheader = (elf32_relc_t*)(start + sheader[i].sh_offset);
        for (j = 0; j < (sheader[i].sh_size / sizeof(elf32_relc_t)); ++j)
        {
            // get current symbol and relocation slot
            symbol = dynTable + ELF32_R_SYM(rheader[j].r_info);
            slot = (uint32_t*)(rheader[j].r_offset - bias + image);
            // relocate the symbol
            *slot = elf32_relocateSymbol(image, symbol, rheader + j, stringTable, bias);

            kprintf("Relocated   .slot=%8p   .type=%-14s   .base=%8p   .target=%08x   %s\n",
                slot,
                (ELF32_R_TYPE(rheader[j].r_info) < ELF32_RELOCS_COUNT)?
                    ELF32_RELOCS[ELF32_R_TYPE(rheader[j].r_info)] : "OTHER",
                image,
                *slot,
                stringTable + symbol->st_name );
        }
    }
}



void* elf32_findSymbol(
    char *start,
    char *image,
    const char* name  )
{
    elf32_file_header_t *header;
    elf32_sect_header_t *sheader;
    elf32_symb_t *symbol;
    char *stringTable;
    uint32_t bias = 0xFFFFFFFF;
    int i, j;

    header = (elf32_file_header_t *) start;

    // look for tables
    sheader = (elf32_sect_header_t *)(start + header->e_shoff);
    for (i = 0; i < header->e_shnum; ++i)
    {
        // compute the bias (will be greater than zero only for ET_EXEC)
        if (sheader[i].sh_type == SHT_PROGBITS && bias == 0xFFFFFFFF)
            bias = sheader[i].sh_addr - sheader[i].sh_offset;

        if (sheader[i].sh_type == SHT_STRTAB)
            stringTable = start + sheader[i].sh_offset;
    }
    if (stringTable == NULL) return NULL;

    // look for the symbol
    for (i = 0; i < header->e_shnum; ++i)
    {
        if (sheader[i].sh_type != SHT_SYMTAB/* && sheader[i].sh_type != SHT_DYNSYM*/) continue;

        /*if (sheader[i].sh_type == SHT_SYMTAB)
            printf("Looking at SYMTAB\n");
        else
            printf("Looking at DYNSYM\n");*/

        symbol = (elf32_symb_t*)(start + sheader[i].sh_offset);
        for (j = 0; j < sheader[i].sh_size / sizeof(elf32_symb_t); ++j)
        {
            //printf("%s == %s?\n", name, stringTable + symbol[j].st_name);
            if (strcmp(name, stringTable + symbol[j].st_name) == 0)
            {
                //printf("bias = 0x%08X\n", bias);
                return image - bias + symbol[j].st_value;
            }
        }
    }

    return NULL;
}


uint32_t elf32_getImageSize( elf32_prog_header_t *pheader, uint16_t count )
{
    uint32_t size, addr, i;
    uint32_t bias = 0xFFFFFFFF;

    for(i = 0, size = 0; i < count; ++i)
    {
        if(pheader[i].p_type != PT_LOAD) continue;
        if (bias == 0xFFFFFFFF) bias = pheader[i].p_paddr;
        size = pheader[i].p_memsz;
        addr = pheader[i].p_paddr;
    }

    return addr - bias + size;
}


int elf32_load(
    char *start,
    uint32_t size,
    struct module **module )
{
    elf32_file_header_t *header;
    elf32_prog_header_t *pheader;
    elf32_sect_header_t *sheader;
    elf32_symb_t *dynTable = NULL;
    char *stringTable = NULL;
    char *source;
    char *destination;
    char *image;
    int i = 0;
    size_t bias = 0xFFFFFFFF;
    uint32_t imageSize;

    header = (elf32_file_header_t *) start;

    // check ELF header
    if(memcmp(header->e_ident, ELF32_MAGIC, sizeof(ELF32_MAGIC)) != 0)
    {
        kprintf("image_load:: invalid ELF image\n");
        return -EINVAL;
    }
    // check if supported ELF type (until we have processes, only share-library is supported)
    if (/*header->e_type != ET_EXEC &&*/ header->e_type != ET_DYN)
    {
        kprintf("Unsupported ELF type\n");
        return -EINVAL;;
    }
    // check if supported machine
    if (header->e_machine != EM_386)
    {
        kprintf("Unsupported machine\n");
        return -EINVAL;
    }

    // allocate memory for ELF32 image
    pheader = (elf32_prog_header_t *)(start + header->e_phoff);
    imageSize = elf32_getImageSize(pheader, header->e_phnum);
    image = kmem_alloc(PAGES(imageSize), PFT_KMOD);
    if (image == NULL)
    {
        kprintf("Error allocating memory\n");
        return -ENOMEM;
    }
    memset(image, 0, imageSize);
    kprintf("ELF32 image requires %d bytes\n", imageSize);

    // copy every ELF segments
    for(i = 0; i < header->e_phnum; ++i)
    {
        // validate segment information
        if (pheader[i].p_type != PT_LOAD || pheader[i].p_filesz == 0) continue;
        if (pheader[i].p_filesz > pheader[i].p_memsz)
        {
            kprintf("ELF32 segment file size can not be greater than in memory\n");
            goto ESCAPE;
        }
        // compute the bias (will be greater than zero only for ET_EXEC)
        if (bias == 0xFFFFFFFF) bias = pheader[i].p_vaddr;
        // copy data from program section to ELF32 image
        source = start + pheader[i].p_offset;
        destination = image + pheader[i].p_vaddr - bias;
        if (destination + pheader[i].p_filesz > image + imageSize)
        {
            kprintf("Not enough space for program segment\n");
            goto ESCAPE;
        }
        kprintf("Loading segment with %d bytes to 0x%p\n",  pheader[i].p_memsz, destination);
        memcpy(destination, source, pheader[i].p_filesz);
    }

    // look for symbol table and string table
    sheader = (elf32_sect_header_t *)(start + header->e_shoff);
    for (i = 0; i < header->e_shnum; ++i)
    {
        // check if current entry is dynamic symbol table
        if (sheader[i].sh_type == SHT_DYNSYM)
        {
            dynTable = (elf32_symb_t*)(start + sheader[i].sh_offset);
            stringTable = start + sheader[sheader[i].sh_link].sh_offset;
            kprintf("String table at 0x%p\n", stringTable);
            break;
        }
    }
    if (stringTable == NULL || dynTable == NULL) return -EINVAL;

    // relocate internal and external symbols
    elf32_relocate(start, image, header, dynTable, stringTable, bias);

    // protect program segments
    /*for (i = 0; i < header->e_phnum; ++i)
    {
        if (pheader[i].p_type != PT_LOAD || pheader[i].p_filesz == 0) continue;

        source = pheader[i].p_vaddr - bias + image;
        // check if read-only segment
        if ((pheader[i].p_flags & PF_W) == 0)
            vmprotect((unsigned char *) source, pheader[i].p_memsz, PROT_READ);
        // check if executable segment
        if (pheader[i].p_flags & PF_X)
            vmprotect((unsigned char *) source, pheader[i].p_memsz, PROT_EXEC);
    }*/

    *module = kmalloc(sizeof(struct module));
    memset(*module, 0, sizeof(struct module));
    if (*module == NULL) goto ESCAPE;
    (*module)->image = image;
    (*module)->size = imageSize;
    (*module)->symbols = (char*)dynTable - (char*)start + (char*)image;

    return 0;
ESCAPE:
    kmem_free(image, PAGES(imageSize));
    return -EINVAL;
}

/*
int main(int argc, char** argv, char** envp)
{
    int (*ptr)(int,int);
    static char buf[1048576];
    size_t readed;
    char *image;

    FILE* elf = fopen(argv[1], "rb");
    readed = fread(buf, 1, sizeof(buf), elf);
    printf("Read %d bytes from '%s'\n", readed, argv[1]);

    image = elf32_load(buf, readed);

    ptr = elf32_findSymbol(buf, image, "entry");
    printf("Executing entry point function at %p...\n", ptr);
    fflush(stdout);
    if (ptr != NULL)
    {
        printf("0x%08X at 0x%8p\n", *(uint32_t*)ptr, ptr);
        int result = ptr(5, 3);
        printf("Returned %d\n", result);
    }
    return 0;
}*/
