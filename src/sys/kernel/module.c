#include <os/object.h>
#include <sys/types.h>
#include <os/module.h>
#include <os/procfs.h>
#include <os/kmalloc.h>
#include <os/vfs.h>
#include <os/elf32.h>
#include <errno.h>


struct module *moduleList = NULL;


struct mutex moduleLock;


int kmodule_loadBinary(
    const char *fileName,
    struct module **module )
{
    struct file *binary;
    struct stat64 info;
    char *buffer;
    int result;

    result = stat((char*)fileName, &info);
    if (result < 0) return result;

    buffer = kmalloc(info.st_size);
    if (buffer == NULL) return -ENOMEM;

    result = open((char*)fileName, 0, S_IREAD, &binary);
    if (result >= 0)
    {
        result = read(binary, buffer, info.st_size);
        if (result == info.st_size)
        {
            result = elf32_load(buffer, info.st_size, module);
            kprintf("Loaded ELF32 image at 0x%8p\n", (*module)->image);
        }
        close(binary);
    }

    if (buffer != NULL)  kfree(buffer);

    return result;
}


int kmodule_load(
    const char *fileName,
    handle_t *handle )
{
    struct module *module;
    int result;

    // load the binary image
    result = kmodule_loadBinary(fileName, &module);
    if (result == 0)
    {
        module->handle = (uint32_t) module->image;
        module->count++;

        if (moduleList == NULL)
            moduleList = module;
        else
        {
            module->next = moduleList;
            moduleList->prev = module;
            moduleList = moduleList;
        }

        if (handle != NULL) *handle = module->handle;
    }

    return result;
}


int kmodule_get_entry_point(
    handle_t handle,
    void **entry )
{
    struct module *current = moduleList;

    while (current != NULL)
    {
        if (current->handle == handle)
        {
            *entry = current->entry;
            return 0;
        }
    }

    return -ENOTFOUND;
}


static int dump_mods(
    struct proc_file *output,
    struct module *modules )
{
    struct module *mod = modules;

    if (mod == NULL) return 0;

    pprintf(output, "Handle   Module           Refs Entry    Size \n");
    pprintf(output, "-------- ---------------- ---- -------- --------------\n");

    while (mod != NULL)
    {
        pprintf(output, "%08X %-16s %4d %08X %8d bytes\n",
                mod->handle,
                mod->name,
                mod->count,
                mod->entry,
                mod->size );

        mod = mod->next;
    }

    return 0;
}


static int proc_kmods(
    struct proc_file *output,
    void *arg )
{
    return dump_mods(output, moduleList);
}


static int proc_umods(
    struct proc_file *output,
    void *arg )
{
    /*if (!kpage_is_mapped((void *) PEB_ADDRESS)) return -EFAULT;
    if (!((struct peb *) PEB_ADDRESS)->usermods) return -EFAULT;

    return dump_mods(pf, ((struct peb *) PEB_ADDRESS)->usermods);*/
    return 0;
}


void kmodule_initialize()
{
    init_mutex(&moduleLock, 0);

    //init_module_database(&kmods, "krnl.dll", (hmodule_t) OSBASE, get_property(krnlcfg, "kernel", "libpath", "/boot"), find_section(krnlcfg, "modaliases"), 0);

    kmodule_load("/sys/lib3c905c.sys", NULL);

    register_proc_inode("kmods", proc_kmods, NULL);
    register_proc_inode("umods", proc_umods, NULL);

    /*krnlmod = kmods.modules;
    kpframe_set_tag(krnlmod->hmod, get_image_header(krnlmod->hmod)->optional.size_of_image, PFT_KMOD);*/
}
