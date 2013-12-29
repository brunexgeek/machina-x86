//
// cpu.c
//
// CPU information
//
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

#include <os/krnl.h>
#include <os/cpu.h>
#include <string.h>

struct cpu global_cpu;

struct cpu_model_info
{

    int vendor;
    int family;
    const char *model_names[16];
};


static struct cpu_model_info cpu_models[] =
{
    {CPU_VENDOR_INTEL,    4, {"486 DX-25/33", "486 DX-50", "486 SX", "486 DX/2", "486 SL", "486 SX/2", NULL, "486 DX/2-WB", "486 DX/4", "486 DX/4-WB", NULL, NULL, NULL, NULL, NULL, NULL}},
    {CPU_VENDOR_INTEL,    5, {"Pentium 60/66 A-step", "Pentium 60/66", "Pentium 75 - 200", "OverDrive PODP5V83", "Pentium MMX", NULL, NULL, "Mobile Pentium 75 - 200", "Mobile Pentium MMX", NULL, NULL, NULL, NULL, NULL, NULL, NULL}},
    {CPU_VENDOR_INTEL,    6, {"Pentium Pro A-step", "Pentium Pro", NULL, "Pentium II (Klamath)", NULL, "Pentium II (Deschutes)", "Mobile Pentium II", "Pentium III (Katmai)", "Pentium III (Coppermine)", NULL, "Pentium III (Cascades)", NULL, NULL, NULL, NULL}},
    {CPU_VENDOR_AMD,      4, {NULL, NULL, NULL, "486 DX/2", NULL, NULL, NULL, "486 DX/2-WB", "486 DX/4", "486 DX/4-WB", NULL, NULL, NULL, NULL, "Am5x86-WT", "Am5x86-WB"}},
    {CPU_VENDOR_AMD,      5, {"K5/SSA5", "K5", "K5", "K5", NULL, NULL, "K6", "K6", "K6-2", "K6-3", NULL, NULL, NULL, NULL, NULL, NULL}},
    {CPU_VENDOR_AMD,      6, {"Athlon", "Athlon", "Athlon", NULL, "Athlon", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}},
    {CPU_VENDOR_UMC,      4, {NULL, "U5D", "U5S", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}},
    {CPU_VENDOR_NEXGEN,   5, {"Nx586", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}},
    {CPU_VENDOR_RISE,     5, {"iDragon", NULL, "iDragon", NULL, NULL, NULL, NULL, NULL, "iDragon II", "iDragon II", NULL, NULL, NULL, NULL, NULL, NULL}},
    {CPU_VENDOR_UNKNOWN,  0, NULL }
};


static const char *table_lookup_model( const struct cpu *info )
{
    const struct cpu_model_info *current = cpu_models;
    int i;

    if (info->model >= 16) return NULL;

    for (i = 0; i < sizeof(cpu_models) / sizeof(struct cpu_model_info) != NULL; i++)
    {
        if (current->vendor == info->vendor && current->family == info->family)
            return current->model_names[info->model];
        current++;
    }

    return NULL;
}


static int eflag_supported(unsigned long flag)
{
    unsigned long f1, f2;

    __asm__
    (
        // Save eflags
        "pushfd;"

        // Get eflags into eax
        "pushfd;"
        "pop     eax;"

        // Store eflags in f1
        "mov     %1, eax;"

        // Toggle the flag we are testing
        "xor     eax, %2;"

        // Load eax into eflags
        "push    eax;"
        "popfd;"

        // Get eflags into eax
        "pushfd;"
        "pop     eax;"

        // Save in f2
        "mov     %0, eax;"

        // Restore eflags
        "popfd;"
        : "=r" (f2)
        : "m" (f1), "m" (flag)
    );

    return ((f1 ^ f2) & flag) != 0;
}


static int cpuid_supported()
{
    return eflag_supported(EFLAGS_ID);
}


void init_cpu()
{
    unsigned long val[4];
    char *vendorname;

    // check if CPUID is not supported
    if (!cpuid_supported())
    {
        // it must be either an 386 or 486 processor
        if (eflag_supported(EFLAGS_AC))
            global_cpu.family = CPU_FAMILY_486;
        else
            global_cpu.family = CPU_FAMILY_386;

        global_cpu.vendor = CPU_VENDOR_UNKNOWN;
        strcpy(global_cpu.vendorid, "IntelCompatible");
        vendorname = "Intel Compatible";
        sprintf(global_cpu.modelid, "%s %d86", vendorname, global_cpu.family);

        // Note: we will refuse older CPU's
        panic("unsupported CPU (too old)");
    }
    else
    {
        // get vendor ID
        kmach_cpuid(0x00000000, val);
        global_cpu.cpuid_level = val[0];
        memcpy(global_cpu.vendorid + 0, val + 1, 4);
        memcpy(global_cpu.vendorid + 4, val + 3, 4);
        memcpy(global_cpu.vendorid + 8, val + 2, 4);

        if (strcmp(global_cpu.vendorid, "GenuineIntel") == 0)
        {
            global_cpu.vendor = CPU_VENDOR_INTEL;
            vendorname = "Intel";
        }
        else if (strcmp(global_cpu.vendorid, "AuthenticAMD") == 0)
        {
            global_cpu.vendor = CPU_VENDOR_AMD;
            vendorname = "AMD";
        }
        else
        if (strcmp(global_cpu.vendorid, "CyrixInstead") == 0)
        {
            global_cpu.vendor = CPU_VENDOR_CYRIX;
            vendorname = "Cyrix";
        }
        else
        if (strcmp(global_cpu.vendorid, "UMC UMC UMC ") == 0)
        {
            global_cpu.vendor = CPU_VENDOR_UMC;
            vendorname = "UMC";
        }
        else
        if (strcmp(global_cpu.vendorid, "CentaurHauls") == 0)
        {
            global_cpu.vendor = CPU_VENDOR_CENTAUR;
            vendorname = "Centaur";
        }
        else
        if (strcmp(global_cpu.vendorid, "NexGenDriven") == 0)
        {
            global_cpu.vendor = CPU_VENDOR_NEXGEN;
            vendorname = "NexGen";
        }
        else
        if (strcmp(global_cpu.vendorid, "GenuineTMx86") == 0 || strcmp(global_cpu.vendorid, "TransmetaCPU") == 0)
        {
            global_cpu.vendor = CPU_VENDOR_TRANSMETA;
            vendorname = "Transmeta";
        }
        else
        {
            global_cpu.vendor = CPU_VENDOR_UNKNOWN;
            vendorname = global_cpu.vendorid;
        }

        // get all other informations
        if (global_cpu.cpuid_level >= 0x00000001)
        {
            kmach_cpuid(0x00000001, val);
            global_cpu.family = (val[0] >> 8) & 0x0F;
            global_cpu.model = (val[0] >> 4) & 0x0F;
            global_cpu.stepping = val[0] & 0x0F;
            global_cpu.features = val[3];
        }

        // SEP CPUID bug: Pentium Pro reports SEP but doesn't have it until model 3 stepping 3
        if (global_cpu.family == 6 && global_cpu.model < 3 && global_cpu.stepping < 3)
            global_cpu.features &= ~CPU_FEATURE_SEP;

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
            q = global_cpu.modelid;
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
            const char *modelid = table_lookup_model(&global_cpu);
            if (modelid)
                sprintf(global_cpu.modelid, "%s %s", vendorname, modelid);
            else
                sprintf(global_cpu.modelid, "%s %d86", vendorname, global_cpu.family);
        }
    }

    kprintf(KERN_INFO "cpu: %s family %d model %d stepping %d\n", global_cpu.modelid, global_cpu.family, global_cpu.model, global_cpu.stepping);
}


int cpu_proc(struct proc_file *pf, void *arg)
{
    pprintf(pf, "%s family %d model %d stepping %d\n", global_cpu.modelid, global_cpu.family, global_cpu.model, global_cpu.stepping);
    return 0;
}


int cpu_sysinfo(struct cpuinfo *info)
{
    info->cpu_vendor = global_cpu.vendor;
    info->cpu_family = global_cpu.family;
    info->cpu_model = global_cpu.model;
    info->cpu_stepping = global_cpu.stepping;
    info->cpu_mhz = global_cpu.mhz;
    info->cpu_features = global_cpu.features;
    info->pagesize = PAGESIZE;
    strcpy(info->vendorid, global_cpu.vendorid);
    strcpy(info->modelid, global_cpu.modelid);

    return 0;
}


// TODO: became "naked" function
unsigned long eflags()
{
    __asm__
    (
        "pushfd;"
        "pop eax;"
    );
}

