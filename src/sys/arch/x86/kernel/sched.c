//
// sched.c
//
// Task scheduler
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

#define DEFAULT_STACK_SIZE           (1 * 1024 * 1024)
#define DEFAULT_INITIAL_STACK_COMMIT (8 * 1024)


__declspec(naked) struct thread *self()
{
    __asm {
        mov eax, esp
        and eax, TCBMASK
        ret
    }
}


__declspec(naked) void switch_context(struct thread *t)
{
    __asm {
        // Save registers on current kernel stack
        push    ebp
        push    ebx
        push    edi
        push    esi

        // Store kernel stack pointer in tcb
        mov     eax, esp
        and     eax, TCBMASK
        add     eax, TCBESP
        mov     [eax], esp

        // Get stack pointer for new thread and store in esp0
        mov     eax, 20[esp]
        add     eax, TCBESP
        mov     esp, [eax]
        mov     ebp, TSS_ESP0
        mov     [ebp], eax

        // Restore registers from new kernel stack
        pop     esi
        pop     edi
        pop     ebx
        pop     ebp

        ret
    }
}


static void init_thread_stack(struct thread *t, void *startaddr, void *arg)
{
    struct tcb *tcb = (struct tcb *) t;
    unsigned long *esp = (unsigned long *) &tcb->esp;

    *--esp = (unsigned long) arg;
    *--esp = (unsigned long) 0;
    *--esp = (unsigned long) startaddr;
    *--esp = 0;
    *--esp = 0;
    *--esp = 0;
    *--esp = 0;
    tcb->esp = esp;
}


void mark_thread_running() {
    struct thread *t;
    struct tib *tib;

    // Set thread state to running
    t = self();
    t->state = THREAD_STATE_RUNNING;
    t->context_switches++;

    // Set FS register to point to current TIB
    tib = t->tib;
    if (tib)
    {
        // Update TIB descriptor
#ifdef VMACH
        set_gdt_entry(GDT_TIB, (unsigned long) tib, PAGESIZE, D_DATA | D_DPL3 | D_WRITE | D_PRESENT, 0);
#else
        struct segment *seg;

        seg = &syspage->gdt[GDT_TIB];
        seg->base_low = (unsigned short)((unsigned long) tib & 0xFFFF);
        seg->base_med = (unsigned char)(((unsigned long) tib >> 16) & 0xFF);
        seg->base_high = (unsigned char)(((unsigned long) tib >> 24) & 0xFF);
#endif

        // Reload FS register
        __asm
        {
            mov ax, SEL_TIB + SEL_RPL3
            mov fs, ax
        }
    }
}


void user_thread_start(void *arg)
{
    struct thread *t = self();
    unsigned long *stacktop;
    void *entrypoint;

    // Mark thread as running to reload fs register
    mark_thread_running();

    // Setup arguments on user stack
    stacktop = (unsigned long *) t->tib->stacktop;
    *(--stacktop) = (unsigned long) (t->tib);
    *(--stacktop) = 0;

    // Switch to usermode and start excuting thread routine
    entrypoint = t->entrypoint;
    __asm
    {
        mov eax, stacktop
        mov ebx, entrypoint

        push SEL_UDATA + SEL_RPL3
        push eax
        pushfd
        push SEL_UTEXT + SEL_RPL3
        push ebx
        mov ax, SEL_UDATA + SEL_RPL3
        mov ds, ax
        mov es, ax
        IRETD
    }
}
