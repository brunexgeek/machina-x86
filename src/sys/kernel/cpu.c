//
// cpu.c
//
// CPU detection and information.
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

#include <os/krnl.h>
#include <os/cpu.h>
#include <string.h>

/**
 * Global CPU information.
 */
struct cpu_info cpuInfo;


struct cpu_vendor
{
    const char *vendorId;

    int vendor;

    const char *name;
};


static const struct cpu_vendor CPU_VENDORS[] =
{
    { "GenuineIntel", CPU_VENDOR_INTEL,     "Intel" },
    { "AuthenticAMD", CPU_VENDOR_AMD,       "AMD" },
    { "CyrixInstead", CPU_VENDOR_CYRIX,     "Cyrix" },
    { "VIA VIA VIA ", CPU_VENDOR_VIA,       "Via" },
    { "CentaurHauls", CPU_VENDOR_CENTAUR,   "Centaur" },
    { "KVMKVMKVMKVM", CPU_VENDOR_KVM,       "KVM" },
    { "Microsoft Hv", CPU_VENDOR_MS,        "MS Hyper-V" },
    { "VMwareVMware", CPU_VENDOR_VMWARE,    "VMware" },
    { "NexGenDriven", CPU_VENDOR_NEXGEN,    "NexGen" },
    { "GenuineTMx86", CPU_VENDOR_TRANSMETA, "Transmeta" },
    { "TransmetaCPU", CPU_VENDOR_TRANSMETA, "Transmeta" },
    { NULL,           CPU_VENDOR_UNKNOWN,   "Generic" }
};


// TODO: became "naked" function
unsigned long kcpu_get_eflags()
{
    __asm__
    (
        "pushfd;"
        "pop eax;"
    );
}


void kcpu_initialize()
{
    unsigned long val[4];
    const struct cpu_vendor *current;

    // check if CPUID is not supported
    if (!cpuid_is_supported())
    {
        // Note: we can not call 'panic' here because 'stop' try to stop threads!
        kprintf("panic: CPU too old! This OS needs a CPU with support for CUPID instruction.");
        kmach_cli();
        kmach_halt();
    }

    // get vendor ID
    kmach_cpuid(0x00000000, val);
    cpuInfo.level = val[0];
    memcpy(cpuInfo.vendorId + 0, val + 1, 4);
    memcpy(cpuInfo.vendorId + 4, val + 3, 4);
    memcpy(cpuInfo.vendorId + 8, val + 2, 4);
    // retrieve the vendor name and code
    current = CPU_VENDORS;
    while (1)
    {
        if (current->vendorId != NULL || strcmp(cpuInfo.vendorId, current->vendorId) == 0)
        {
            cpuInfo.vendor = current->vendor;
            memset(cpuInfo.vendorName, 0, CPU_VENDOR_NAME_SIZE);
            strncpy(cpuInfo.vendorName, current->name, CPU_VENDOR_NAME_SIZE-1);
        }
        if (current->vendorId != NULL) break;
        current = current + 1;
    }

    // get family, model, stepping and features
    if (cpuInfo.level >= 0x00000001)
    {
        kmach_cpuid(0x00000001, val);
        cpuInfo.family = (val[0] >> 8) & 0x0F;
        cpuInfo.model = (val[0] >> 4) & 0x0F;
        cpuInfo.stepping = val[0] & 0x0F;
        cpuInfo.features = val[3];
    }

    // get brand string
    kmach_cpuid(0x80000000, val);
    if (val[0] >= 0x80000004)
    {
        char model[64];
        char *p, *q;
        int space;

        memset(model, 0, 64);
        kmach_cpuid(0x80000002, (unsigned long *) model);
        kmach_cpuid(0x80000003, (unsigned long *) (model + 16));
        kmach_cpuid(0x80000004, (unsigned long *) (model + 32));

        // trim brand string
        p = model;
        q = cpuInfo.modelId;
        space = 0;
        while (*p == ' ') p++;
        while (*p)
        {
            if (*p == ' ')
            {
                space = 1;
            }
            else
            {
                if (space) *q++ = ' ';
                space = 0;
                *q++ = *p;
            }
            p++;
        }
        *q = 0;
    }
    else
    {
        // Note: we can not call 'panic' here because 'stop' try to stop threads!
        kprintf("panic: CPU too old! This OS needs CPUID instruction with support for processor brand.");
        kmach_cli();
        kmach_halt();
    }

    kprintf(KERN_INFO "cpu: %s family %d model %d stepping %d\n", cpuInfo.modelId, cpuInfo.family, cpuInfo.model, cpuInfo.stepping);
}


int proc_cpuinfo(
    struct proc_file *output,
    void *arg )
{
    pprintf(output, "%s family %d model %d stepping %d\n", cpuInfo.modelId, cpuInfo.family, cpuInfo.model, cpuInfo.stepping);
    return 0;
}


int kcpu_get_info(struct cpu_info *info)
{
    info->vendor = cpuInfo.vendor;
    info->family = cpuInfo.family;
    info->model = cpuInfo.model;
    info->stepping = cpuInfo.stepping;
    info->mhz = cpuInfo.mhz;
    info->features = cpuInfo.features;
    info->pageSize = PAGESIZE;
    strcpy(info->vendorId, cpuInfo.vendorId);
    strcpy(info->modelId, cpuInfo.modelId);

    return 0;
}
