#ifndef MACHINA_OS_MODULE_H
#define MACHINA_OS_MODULE_H


#include <stdint.h>


struct loader
{
    int flags;
    struct module *modules;
    char **modpaths;
    int nmodpaths;
    struct modalias *aliases;
};


struct module
{

    uint32_t handle;

    int count;

    int flags;

    uint32_t size;

    char *name;

    char *path;

    void *entry;

    void *symbols;

    char *image;

    struct module *next;

    struct module *prev;
};


int kmodule_get_entry_point(
    handle_t handle,
    void **entry );

int kmodule_load(
    const char *fileName,
    handle_t *handle );

void kmodule_initialize();

#endif // MACHINA_OS_MODULE_H
