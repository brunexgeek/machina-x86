//
// mach.c
//
// Interface to physical/virtual machine
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

#if (TARGET_MACHINE==x86)
#include "../arch/x86/kernel/mach.c"
#endif


int probe_vmi(); // from vmi.c
int init_vmi(); // from vmi.c


struct mach mach =
{
  0, // kernel ring

  hw_sti,
  hw_cli,
  hw_hlt,
  hw_iretd,
  hw_sysret,
  hw_in,
  hw_inw,
  hw_ind,
  hw_insw,
  hw_insd,
  hw_out,
  hw_outw,
  hw_outd,
  hw_outsw,
  hw_outsd,
  hw_cpuid,
  hw_get_cr0,
  hw_set_cr0,
  hw_get_cr2,
  hw_rdtsc,
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

#ifdef VMACH

void init_mach() {
  kprintf("mach: running in machine virtualization mode\n");
  if (probe_vmi()) init_vmi();
}

#else

void init_mach() {
}

#endif
