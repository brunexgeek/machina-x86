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
#include <os/asmutil.h>
#include <os/syspage.h>

#define DEFAULT_STACK_SIZE           (1 * 1024 * 1024)
#define DEFAULT_INITIAL_STACK_COMMIT (8 * 1024)


void init_thread_stack(struct thread *t, void *startaddr, void *arg)
{
    struct tcb *tcb = (struct tcb *) t;
    unsigned long *esp = (unsigned long *) &tcb->esp;

    *--esp = (unsigned long) arg;
    *--esp = (unsigned long) 0;
    *--esp = (unsigned long) startaddr;
    *--esp = 0; // ebp
    *--esp = 0; // ebx
    *--esp = 0; // edi
    *--esp = 0; // esi
    tcb->esp = (uint32_t*) esp;
}


void kthread_mark_running()
{
    struct thread *t;
    struct tib *tib;

    // set thread state to running
    t = kthread_self();
    t->state = THREAD_STATE_RUNNING;
    t->context_switches++;

    // set FS register to point to current TIB
    tib = t->tib;
    if (tib)
    {
        // Update TIB descriptor
        struct segment *seg;

        seg = &syspage->gdt[GDT_TIB];
        seg->base_low = (unsigned short)((unsigned long) tib & 0xFFFF);
        seg->base_med = (unsigned char)(((unsigned long) tib >> 16) & 0xFF);
        seg->base_high = (unsigned char)(((unsigned long) tib >> 24) & 0xFF);

        // Reload FS register
        __asm__
        (
            "mov ax, %0 + %1;"
            "mov fs, ax;"
            :
            : "i" (SEL_TIB), "i" (SEL_RPL3)
            : "eax"
        );
    }
}


void user_thread_start(void *arg)
{
    struct thread *t = kthread_self();
    unsigned long *stacktop;
    void *entrypoint;

    // Mark thread as running to reload fs register
    kthread_mark_running();

    // Setup arguments on user stack
    stacktop = (unsigned long *) t->tib->stacktop;
    *(--stacktop) = (unsigned long) (t->tib);
    *(--stacktop) = 0;

    // Switch to usermode and start excuting thread routine
    entrypoint = t->entrypoint;
    __asm__
    (
        "mov eax, %3;"
        "mov ebx, %4;"

        "push %0 + %2;"
        "push eax;"
        "pushfd;"
        "push %1 + %2;"
        "push ebx;"
        "mov ax, %0 + %2;"
        "mov ds, ax;"
        "mov es, ax;"
        "IRETD;"
        :
        : "i" (SEL_UDATA), "i" (SEL_UTEXT), "i" (SEL_RPL3), "m" (stacktop), "m" (entrypoint)
        : "eax"
    );
}



int kthread_get_context(struct thread *t, struct context *ctxt)
{
    memcpy(ctxt, t->ctxt, sizeof(struct context));
    return 0;
}


int kthread_set_context(struct thread *t, struct context *ctxt)
{
    t->ctxt->esp = ctxt->esp;
    t->ctxt->eip = ctxt->eip;
    t->ctxt->eflags = ctxt->eflags;
    t->ctxt->ebp = ctxt->ebp;

    t->ctxt->eax = ctxt->eax;
    t->ctxt->ebx = ctxt->ebx;
    t->ctxt->ecx = ctxt->ecx;
    t->ctxt->edx = ctxt->edx;
    t->ctxt->esi = ctxt->esi;
    t->ctxt->edi = ctxt->edi;

    return 0;
}
