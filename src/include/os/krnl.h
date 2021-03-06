//
// krnl.h
//
// Main kernel include file
//
// Copyright (C) 2013 Bruno Ribeiro.
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

#ifndef MACHINA_OS_KRNL_H
#define MACHINA_OS_KRNL_H


#define KERNELAPI


#define debug_info(...)   ( kprintf("[INFO ] [%s:%d] ", __FILE__, __LINE__), kprintf(__VA_ARGS__) )
#define debug_error(...)  ( kprintf("[ERROR] [%s:%d] ", __FILE__, __LINE__), kprintf(__VA_ARGS__) )


#include <os/config.h>
#include <os.h>

#include <sys/types.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <bitops.h>
#include <inifile.h>
#include <verinfo.h>
#include <os/mach.h>
#include <os/syspage.h>
#include <os/object.h>
#include <os/klog.h>
#include <os/procfs.h>


// start.c

KERNELAPI extern dev_t bootdev;
#ifndef OSLDR
KERNELAPI extern char krnlopts[KRNLOPTS_LEN];
#endif
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
