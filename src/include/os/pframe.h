//
// pframe.h
//
// Page frame database routines
//
// Copyright (C) 2013-2014 Bruno Ribeiro
// Copyright (C) 2002 Michael Ringgaard
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

#ifndef MACHINA_OS_PFRAME_H
#define MACHINA_OS_PFRAME_H


#include <os/krnl.h>
#include <os/procfs.h>


#define DMA_BUFFER_START 0x10000
#define DMA_BUFFER_PAGES 16

#define PFT_FREE              0x01 /// Available for allocation
#define PFT_HTAB              0x02
#define PFT_RESERVED          0x03 /// Reserved by system (according BIOS)
#define PFT_MEM               0x04
#define PFT_NVS               0x05 /// Non-volatile storage
#define PFT_ACPI              0x06
#define PFT_BAD               0x07
#define PFT_PTAB              0x08
#define PFT_DMA               0x09 /// DMA buffer
#define PFT_PFDB              0x0A /// Page frame database
#define PFT_SYS               0x0B
#define PFT_TCB               0x0C
#define PFT_BOOT              0x0D
#define PFT_FMAP              0x0E
#define PFT_STACK             0x0F /// Thread stack
#define PFT_KMEM              0x10 /// Kernel allocated (heap) memory
#define PFT_KMOD              0x11
#define PFT_UMOD              0x12
#define PFT_VM                0x13 /// Virtual memory
#define PFT_HEAP              0x14
#define PFT_TIB               0x15
#define PFT_PEB               0x16

#define INVALID_PFRAME        ((uint32_t)0xFFFFFFFF)


struct page_frame_t
{
    uint32_t linear : 1;   /// Indicates if frame begin a linear allocation
    uint32_t tag    : 5;
    uint32_t __resv : 2;
    uint32_t next   : 24;  /// Index for next page frame (when in free list).
};


KERNELAPI uint32_t kpframe_alloc( uint8_t tag );
KERNELAPI uint32_t kpframe_alloc_linear( uint32_t pages, uint8_t tag );
KERNELAPI void kpframe_free( uint32_t pfn );
KERNELAPI void kpframe_set_tag( void *addr, uint32_t len, uint8_t tag );
uint8_t kpframe_get_tag( void *vaddr );
const char *kpframe_tag_name( uint8_t tag );

int memmap_proc(struct proc_file *pf, void *arg);
int memusage_proc(struct proc_file *pf, void *arg);
int memstat_proc(struct proc_file *pf, void *arg);
int physmem_proc(struct proc_file *pf, void *arg);

void kpframe_init();


#endif  // MACHINA_OS_PFRAME_H
