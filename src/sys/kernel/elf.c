#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include "../../include/os/elf.h"


void printk(const char* msg)
{
    fputs(msg, stderr);
}

int is_image_valid(elf32_file_header_t *hdr)
{
    return 1;
}

static void *resolve(const char* sym)
{
    static void *handle = NULL;
    /*if (handle == NULL)
    {
        handle = dlopen("c.so", RTLD_NOW);
        printf("libc.so == %p\n", handle);
        if (handle == NULL) perror("Fuuu");
    }*/
    void *result = dlsym(handle, sym);
    printf("%s = %p\n", sym, result);
    return result;
}

void relocate(elf32_sect_header_t* shdr, const elf32_symb_t* syms, const char* strings, const char* src, char* dst)
{
    elf32_relc_t* rel = (elf32_relc_t*)(src + shdr->sh_offset);
    int j;
    for(j = 0; j < shdr->sh_size / sizeof(elf32_relc_t); j += 1)
    {
        const char* sym = strings + syms[ELF32_R_SYM(rel[j].r_info)].st_name;
        switch(ELF32_R_TYPE(rel[j].r_info))
        {
            case R_386_JMP_SLOT:
            case R_386_GLOB_DAT:
                *(uint32_t*)(dst + rel[j].r_offset) = (uint32_t)resolve(sym);
                break;
        }
    }
}

void* find_symbol(
    const char* name,
    elf32_sect_header_t* shdr,
    const char* strings,
    const char* src,
    char* dst)
{
    elf32_symb_t* syms = (elf32_symb_t*)(src + shdr->sh_offset);
    int i;
    for(i = 0; i < shdr->sh_size / sizeof(elf32_symb_t); i += 1) {
        if (strcmp(name, strings + syms[i].st_name) == 0) {
            return dst + syms[i].st_value;
        }
    }
    return NULL;
}

void *elf32_load(
    char *elf_start,
    unsigned int size )
{
    elf32_file_header_t *hdr     = NULL;
    elf32_prog_header_t *phdr    = NULL;
    elf32_sect_header_t *shdr    = NULL;
    elf32_symb_t *syms    = NULL;
    char *strings = NULL;
    char *start   = NULL;
    char *taddr   = NULL;
    void *entry   = NULL;
    int i = 0;
    char *exec = NULL;

    hdr = (elf32_file_header_t *) elf_start;

    if(!is_image_valid(hdr))
    {
        printk("image_load:: invalid ELF image\n");
        return 0;
    }

    exec = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC,
                      MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

    if(!exec) {
        printk("image_load:: error allocating memory\n");
        return 0;
    }

    // Start with clean memory.
    memset(exec,0x0,size);

    phdr = (elf32_prog_header_t *)(elf_start + hdr->e_phoff);

    for(i=0; i < hdr->e_phnum; ++i) {
            if(phdr[i].p_type != PT_LOAD) {
                    continue;
            }
            if(phdr[i].p_filesz > phdr[i].p_memsz) {
                    printk("image_load:: p_filesz > p_memsz\n");
                    munmap(exec, size);
                    return 0;
            }
            if(!phdr[i].p_filesz) {
                    continue;
            }

            // p_filesz can be smaller than p_memsz,
            // the difference is zeroe'd out.
            start = elf_start + phdr[i].p_offset;
            taddr = phdr[i].p_vaddr + exec;
            memcpy(taddr,start,phdr[i].p_filesz);

            if(!(phdr[i].p_flags & PF_W)) {
                    // Read-only.
                    mprotect((unsigned char *) taddr,
                              phdr[i].p_memsz,
                              PROT_READ);
            }

            if(phdr[i].p_flags & PF_X) {
                    // Executable.
                    mprotect((unsigned char *) taddr,
                            phdr[i].p_memsz,
                            PROT_EXEC);
            }
    }

    shdr = (elf32_sect_header_t *)(elf_start + hdr->e_shoff);

    for(i=0; i < hdr->e_shnum; ++i) {
        if (shdr[i].sh_type == SHT_DYNSYM) {
            syms = (elf32_symb_t*)(elf_start + shdr[i].sh_offset);
            strings = elf_start + shdr[shdr[i].sh_link].sh_offset;
            entry = find_sym("main", shdr + i, strings, elf_start, exec);
            break;
        }
    }

    for(i=0; i < hdr->e_shnum; ++i) {
        if (shdr[i].sh_type == SHT_REL) {
            relocate(shdr + i, syms, strings, elf_start, exec);
        }
    }

   return entry;

}/* image_load */


int main(int argc, char** argv, char** envp)
{
    int (*ptr)(int, char **, char**);
    static char buf[1048576];
    FILE* elf = fopen(argv[1], "rb");
    fread(buf, sizeof buf, 1, elf);
    ptr = elf32_load(buf, sizeof buf);
    printf("Executing...");
    fflush(stdout);
    if (ptr != NULL)
        printf("Returns %d\n", ptr(argc,argv, envp) );
    return 0;
}
