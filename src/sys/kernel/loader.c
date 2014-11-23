#include <os/loader.h>
#include <os/elf.h>
#include <os/kmalloc.h>
#include <os/vfs.h>
#include <os/vmm.h>
#include <os/kmem.h>
#include <kdebug.h>


struct module_t global_kmodules;

void *kmods = 0;


/**
 * Returns the amount of memory required to load the ELF32 an executable file.
 */
static uint32_t binary_memory_size(
    const elf32_file_header_t *header,
    const elf32_prog_header_t *pheader,
    uint32_t *vaddr )
{
    uint32_t size = 0, i;

    // iterate between program header entries
    for (i = 0; i < header->e_phnum; ++i)
    {
        // ignore entries with nothing to load
        if (pheader[i].p_type != PT_LOAD || pheader[i].p_filesz == 0) continue;
        // validate the segment sizes
        kassert(pheader[i].p_filesz > pheader[i].p_memsz);

        if (size == 0 && vaddr != NULL) *vaddr = pheader[i].p_vaddr;
        kprintf("elf: segment vaddr=0x%08X  size=0x%08X  align=0x%08X\n", pheader[i].p_vaddr, pheader[i].p_memsz, pheader[i].p_align);
        size += pheader[i].p_memsz + pheader[i].p_align;
    }

    return size;
}


void *kloader_load( char *filename, uint8_t isUserspace )
{
    uint8_t *buffer, *image, *start;
    struct file *file;
    elf32_file_header_t *header;
    elf32_prog_header_t *pheader;
    uint8_t *daddr;
    int i;
    uint32_t vaddr, position = 0;

    kprintf("Loading %s\n", filename);

    buffer = kmalloc(PAGESIZE);
    if (buffer == NULL) return NULL;

    // open binary file
    if (open(filename, O_RDONLY | O_BINARY, 0, &file) < 0)
    {
        kfree(buffer);
        return NULL;
    }
    // read ELF32 binary header
    if ((read(file, buffer, PAGESIZE)) < 0)
    {
        close(file);
        destroy(file);
        kfree(buffer);
        return NULL;
    }
    header = (elf32_file_header_t *) buffer;
    if (*((int*)header->e_ident) != 0x464C457F)
    {
        debug_error("invalid ELF32 binary file '%s'", filename);
        panic("run to the hills!");
    }
    pheader = (elf32_prog_header_t *) (buffer + header->e_phoff);

    // allocate memory for binary
    int size = binary_memory_size(header, pheader, NULL);
    if (size <= 0) panic("ELF32 binary has no loadable segments");
    if (isUserspace)
    {
        vaddr = 0x100000;
        image = (uint8_t*) vmalloc((void *)vaddr, size, MEM_RESERVE | MEM_COMMIT,
            PAGE_EXECUTE_READWRITE, PFT_UMOD, NULL);
        /*if (image == NULL)
            image = (uint8_t*) vmalloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE, PFT_UMOD, NULL);*/
    }
    else
    {
        image = (uint8_t*) alloc_module_mem(PAGES(size));
        if (image) memset(image, 0, PAGES(size) * PAGESIZE);
    }
    if (image == NULL)
        panic("out of memory");

    // iterate between program header entries
    pheader = (elf32_prog_header_t *)(buffer + header->e_phoff);
    daddr = image;
    for (i = 0; i < header->e_phnum; ++i)
    {
        // ignore entries with nothing to load
        if (pheader[i].p_type != PT_LOAD || pheader[i].p_filesz == 0) continue;

        //memcpy(taddr,start,phdr[i].p_filesz);
        kprintf("elf: loading from file@0x%08X to vaddr@0x%08p\n", pheader[i].p_offset, daddr);
        lseek(file, pheader[i].p_offset, SEEK_SET);
        if (read(file, daddr, pheader[i].p_memsz) < 0)
        {
            if (isUserspace)
            {
                vmfree(image, size, MEM_RELEASE);
            }
            else
            {
                free_module_mem(image, size);
            }

            close(file);
            destroy(file);
            kfree(buffer);
            return NULL;
        }

        // check if is a read only page
        if (!(pheader[i].p_flags & PF_W))
        {
            vmprotect(vaddr, pheader[i].p_memsz, PAGE_READONLY);
        }

        // check if is a executable page
        if(pheader[i].p_flags & PF_X)
        {
            vmprotect(vaddr, pheader[i].p_memsz, PAGE_EXECUTE);
        }

        daddr = image + pheader[i].p_memsz + pheader[i].p_align;
    }
}
