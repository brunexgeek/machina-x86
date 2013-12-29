//
// krnl.h
//
// Main kernel include file
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

#ifndef MACHINA_OS_KRNL_H
#define MACHINA_OS_KRNL_H


#define KERNELAPI


#include <os/config.h>
#include <os.h>

#include <sys/types.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <bitops.h>
/*

#include <rmap.h>
*/
#include <inifile.h>
/*
#include <moddb.h>*/
#include <verinfo.h>
/*
#include <os/tss.h>
#include <os/seg.h>
#include <os/fpu.h>
#include <os/cpu.h>

#include <os/pdir.h>
#include <os/pframe.h>

*/
#include <os/mach.h>/*
#include <os/kmem.h>
#include <os/kmalloc.h>
#include <os/vmm.h>
*/
#include <os/syspage.h>
/*
#include <os/pe.h>

#include <os/buf.h>

#include <os/timer.h>
#include <os/user.h>
*/
#include <os/object.h>
/*
#include <os/queue.h>
#include <os/sched.h>
#include <os/trap.h>
#include <os/dbg.h>*/
#include <os/klog.h>
/*
#include <os/pic.h>
#include <os/pit.h>

#include <os/dev.h>
#include <os/pci.h>
#include <os/pnpbios.h>
#include <os/virtio.h>

#include <os/video.h>
#include <os/kbd.h>
#include <os/rnd.h>

#include <os/iovec.h>
#include <os/vfs.h>
#include <os/dfs.h>
#include <os/devfs.h>*/
#include <os/procfs.h>
/*
#include <os/mbr.h>

#include <os/pe.h>
#include <os/ldr.h>

#include <os/syscall.h>

#include <net/net.h>
*/

// start.c

KERNELAPI extern dev_t bootdev;
KERNELAPI extern char krnlopts[KRNLOPTS_LEN];
KERNELAPI extern struct section *krnlcfg;
KERNELAPI extern struct peb *peb;

KERNELAPI void panic(char *msg);
KERNELAPI int license();

KERNELAPI void stop(int mode);



// opts.c

char *get_option(char *opts, char *name, char *buffer, int size, char *defval);
int get_num_option(char *opts, char *name, int defval);

// strtol.c

unsigned long strtoul(const char *nptr, char **endptr, int ibase);

// vsprintf.c

int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);

#endif  // MACHINA_OS_KRNL_H
