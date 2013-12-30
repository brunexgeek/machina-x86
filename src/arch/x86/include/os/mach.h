//
// mach.h
//
// Interface to physical x86 machine
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

#if (TARGET_MACHINE==x86)

#include <os/krnl.h>
#include <os/cpu.h>
#include <os/pdir.h>
#include <os/syspage.h>


#define kmach_reboot()        { kbd_reboot(); }

static inline void kmach_sti()
{
    __asm__("sti;");
}


static inline void kmach_cli()
{
    __asm__("cli;");
}


static inline void kmach_halt()
{
    __asm__("hlt;");
}


static inline void kmach_cpuid(unsigned long reg, unsigned long values[4])
{
    __asm__
    (
        //"mov    eax, reg;"
        "cpuid;"
        "mov    esi, %1;"
        "mov    [esi], eax;"
        "mov    [esi+4], ebx;"
        "mov    [esi+8], ecx;"
        "mov    [esi+12], edx;"
        :
        : "a" (reg), "m" (values)
    );
}


static inline unsigned long kmach_get_cr0()
{
    unsigned long val;

    __asm__
    (
        "mov eax, cr0;"
        "mov %0, eax;"
        : "=r" (val)
    );

    return val;
}


static inline void kmach_set_cr0(unsigned long val)
{
    __asm__
    (
        //"mov eax, %0;"
        "mov cr0, eax;"
        :
        : "a" (val)
    );
}


static inline unsigned long kmach_get_cr2()
{
    unsigned long val;

    __asm__
    (
        "mov eax, cr2;"
        "mov %0, eax;"
        : "=r" (val)
    );

    return val;
}


static inline void kmach_wrmsr(
    unsigned long reg,
    unsigned long valuelow,
    unsigned long valuehigh )
{
    __asm__
    (
        //"mov ecx, reg;"
        //"mov eax, valuelow;"
        //"mov edx, valuehigh;"
        "wrmsr;"
        :
        : "c" (reg), "a" (valuelow), "d" (valuehigh)
    );
}


#ifndef OSLDR

static inline void kmach_set_gdt_entry(
    int entry,
    unsigned long addr,
    unsigned long size,
    int access,
    int granularity)
{
    seginit(&syspage->gdt[entry], addr, size, access, granularity);
}


static inline void kmach_set_idt_gate(int intrno, void *handler)
{
    syspage->idt[intrno].offset_low = (unsigned short) (((unsigned long) handler) & 0xFFFF);
    syspage->idt[intrno].selector = SEL_KTEXT | global_kring;
    syspage->idt[intrno].access = D_PRESENT | D_INT | D_DPL0;
    syspage->idt[intrno].offset_high = (unsigned short) (((unsigned long) handler) >> 16);
}


static inline void kmach_set_idt_trap(int intrno, void *handler)
{
    syspage->idt[intrno].offset_low = (unsigned short) (((unsigned long) handler) & 0xFFFF);
    syspage->idt[intrno].selector = SEL_KTEXT | global_kring;
    syspage->idt[intrno].access = D_PRESENT | D_TRAP | D_DPL3;
    syspage->idt[intrno].offset_high = (unsigned short) (((unsigned long) handler) >> 16);
}

#endif

static inline void kmach_switch_kernel_stack()
{
}


static inline void kmach_flushtlb()
{
    __asm__
    (
        "mov eax, cr3;"
        "mov cr3, eax;"
    );
}

static inline void kmach_invlpage(void *addr)
{
    if (global_cpu.family < CPU_FAMILY_486)
    {
        __asm__
        (
            "mov eax, cr3;"
            "mov cr3, eax;"
        );
    }
    else
    {
        __asm__
        (
            //"mov eax, addr;"
            "invlpg [eax];"
            :
            : "a" (addr)
        );
    }
}

static inline void kmach_register_page_dir(unsigned long pfn)
{
    // Do nothing
}

static inline void kmach_register_page_table(unsigned long pfn)
{
    // Do nothing
}

static inline void kmach_set_page_dir_entry(pte_t *pde, unsigned long value)
{
    *pde = value;
}

static inline void kmach_set_page_table_entry(pte_t *pte, unsigned long value)
{
    *pte = value;
}

static inline void kmach_poweroff()
{
    /*if (apm_enabled)
        apm_power_off();
    else*/
        //kprintf("kernel: power management not enabled, system stopped...\n");

    while (1)
    {
        __asm__
        (
        "cli;"
        "hlt;"
        );
    }
}


unsigned int kmach_rdtsc() __asm__("___hw_rdtsc");

KERNELAPI unsigned char inb(port_t port) __asm__("___inb");
KERNELAPI unsigned char inp(port_t port) __asm__("___inp");
KERNELAPI unsigned short inpw(port_t port) __asm__("___inpw");
KERNELAPI unsigned long inpd(port_t port) __asm__("___inpd");

KERNELAPI void insw(port_t port, void *buf, int count) __asm__("___insw");
KERNELAPI void insd(port_t port, void *buf, int count) __asm__("___insd");

KERNELAPI unsigned char outb(port_t port, unsigned char val) __asm__("___outb");
KERNELAPI unsigned char outp(port_t port, unsigned char val) __asm__("___outp");
KERNELAPI unsigned short outw(port_t port, unsigned short val) __asm__("___outw");
KERNELAPI unsigned short outpw(port_t port, unsigned short val) __asm__("___outpw");
KERNELAPI unsigned long outd(port_t port, unsigned long val) __asm__("___outd");
KERNELAPI unsigned long outpd(port_t port, unsigned long val) __asm__("___outpd");

KERNELAPI void outsw(port_t port, void *buf, int count) __asm__("___outsw");
KERNELAPI void outsd(port_t port, void *buf, int count) __asm__("___outsd");

#endif  // (TARGET_MACHINE==x86)
