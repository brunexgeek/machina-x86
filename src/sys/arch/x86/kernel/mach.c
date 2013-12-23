//
// mach.c
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

#include <os/krnl.h>
#include <os/mach.h>


void printnum( unsigned int value )
{
    kprintf("printnum: 0x%x\n", value);
}

static void hw_sti()
{
    __asm__("sti;");
}


static void hw_cli()
{
    __asm__("cli;");
}


static void hw_hlt()
{
      __asm__("hlt;");
}


static void hw_iretd()
{
    __asm__
    (
        "add esp,4;"
        "iretd;"
    );
}

static void hw_sysret()
{
    __asm__
    (
        "add esp,4;"
        "sysexit;"
    );
}


static void hw_cpuid(unsigned long reg, unsigned long values[4]) {
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

static unsigned long hw_get_cr0() {
  unsigned long val;

    __asm__
    (
        "mov eax, cr0;"
        "mov %0, eax;"
        : "=r" (val)
    );

  return val;
}

static void hw_set_cr0(unsigned long val) {
    __asm__
    (
        //"mov eax, %0;"
        "mov cr0, eax;"
        :
        : "a" (val)
    );
}

static unsigned long hw_get_cr2()
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


static void hw_wrmsr(
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

static void hw_set_gdt_entry(int entry, unsigned long addr, unsigned long size, int access, int granularity) {
  seginit(&syspage->gdt[entry], addr, size, access, granularity);
}

static void hw_set_idt_gate(int intrno, void *handler) {
  syspage->idt[intrno].offset_low = (unsigned short) (((unsigned long) handler) & 0xFFFF);
  syspage->idt[intrno].selector = SEL_KTEXT | mach.kring;
  syspage->idt[intrno].access = D_PRESENT | D_INT | D_DPL0;
  syspage->idt[intrno].offset_high = (unsigned short) (((unsigned long) handler) >> 16);
}

static void hw_set_idt_trap(int intrno, void *handler) {
  syspage->idt[intrno].offset_low = (unsigned short) (((unsigned long) handler) & 0xFFFF);
  syspage->idt[intrno].selector = SEL_KTEXT | mach.kring;
  syspage->idt[intrno].access = D_PRESENT | D_TRAP | D_DPL3;
  syspage->idt[intrno].offset_high = (unsigned short) (((unsigned long) handler) >> 16);
}

static void hw_switch_kernel_stack() {
}

static void hw_flushtlb()
{
    __asm__
    (
        "mov eax, cr3;"
        "mov cr3, eax;"
    );
}

static void hw_invlpage(void *addr) {
    if (cpu.family < CPU_FAMILY_486)
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

static void hw_register_page_dir(unsigned long pfn)
{
    // Do nothing
}

static void hw_register_page_table(unsigned long pfn)
{
    // Do nothing
}

static void hw_set_page_dir_entry(pte_t *pde, unsigned long value)
{
    *pde = value;
}

static void hw_set_page_table_entry(pte_t *pte, unsigned long value)
{
    *pte = value;
}

static void hw_poweroff()
{
    /*if (apm_enabled)
        apm_power_off();
    else*/
        kprintf("kernel: power management not enabled, system stopped...\n");

    while (1)
    {
        __asm__
        (
        "cli;"
        "hlt;"
        );
    }
}


static void hw_reboot()
{
    kbd_reboot();
}


unsigned int __inline hw_rdtsc() __asm__("___hw_rdtsc");


struct mach mach =
{
  0, // kernel ring
  hw_sti,
  hw_cli,
  hw_hlt,
  hw_iretd,
  hw_sysret,
  NULL,//hw_in,
  NULL,//hw_inw,
  NULL,//hw_ind,
  NULL,//hw_insw,
  NULL,//hw_insd,
  NULL,//hw_out,
  NULL,//hw_outw,
  NULL,//hw_outd,
  NULL,//hw_outsw,
  NULL,//hw_outsd,
  hw_cpuid,
  hw_get_cr0,
  hw_set_cr0,
  hw_get_cr2,
  NULL,//hw_rdtsc
  hw_wrmsr,
  hw_set_gdt_entry,
  hw_set_idt_gate,
  hw_set_idt_trap,
  hw_switch_kernel_stack,
  hw_flushtlb,
  hw_invlpage,
  hw_register_page_dir,
  hw_register_page_table,
  hw_set_page_dir_entry,
  hw_set_page_table_entry,
  hw_poweroff,
  hw_reboot
};
