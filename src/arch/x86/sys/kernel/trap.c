//
// trap.c
//
// Interrupt and trap handling for x86
//
// Copyright (C) 2013-2014 Bruno Ribeiro. All rights reserved.
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
#include <os/trap.h>
#include <os/pdir.h>
#include <os/cpu.h>
#include <os/mach.h>
#include <os/dbg.h>
#include <os/sched.h>
#include <os/vmm.h>

#define INTRS MAXIDT

struct syspage *syspage = (struct syspage *) SYSPAGE_ADDRESS;

/**
 * @brief Internal interrupt handler array.
 */
static struct interrupt *intr_handlers[INTRS];

/**
 * @brief Interrupt counter array used to keep track how many times each interrupt is handled.
 */
static int intr_counter[INTRS];

static struct interrupt divintr;
static struct interrupt brkptintr;
static struct interrupt overflowintr;
static struct interrupt boundsintr;
static struct interrupt illopintr;
static struct interrupt segintr;
static struct interrupt stackintr;
static struct interrupt genprointr;
static struct interrupt pgfintr;
static struct interrupt alignintr;
static struct interrupt sigexitintr;

char *INTR_NAMES[INTRS] =
{
    "Divide error",
    "Debug exception",
    "Non-maskable interrupt",
    "Breakpoint",
    "Overflow",
    "Bounds check",
    "Invalid opcode",
    "FPU not available",
    "Double fault",
    "FPU segment overrun",
    "Invalid task state segment",
    "Segment not present",
    "Stack fault",
    "General protection fault",
    "Page fault",
    "(reserved)",
    "FPU error",
    "Alignment check",
    "Machine check",
    "SIMD FP exception",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",
    "(reserved)",

    "IRQ0 (timer)",
    "IRQ1 (keyboard)",
    "IRQ2",
    "IRQ3 (com2)",
    "IRQ4 (com1)",
    "IRQ5",
    "IRQ6 (fdc)",
    "IRQ7 (par)",
    "IRQ8 (rtc)",
    "IRQ9",
    "IRQ10",
    "IRQ11",
    "IRQ12",
    "IRQ13",
    "IRQ14 (hdc1)",
    "IRQ15 (hdc2)",

    "System call",
    "Signal exit",
    "(unused)",
    "(unused)",
    "(unused)",
    "(unused)",
    "(unused)",
    "(unused)",
    "(unused)",
    "(unused)",
    "(unused)",
    "(unused)",
    "(unused)",
    "(unused)",
    "(unused)",
    "(unused)"
};



/**
 * @brief Tests if context is a user mode context.
 */
static inline int is_usermode(struct context *ctxt)
{
    return USERSPACE(ctxt->eip);
}


/**
 * @brief Common entry point for all interrupt service routines
 */
void trap(unsigned long args)
{
    struct context *ctxt = (struct context *) &args;
    struct thread *t = kthread_self();
    struct context *prevctxt;
    struct interrupt *intr;
    int rc;


    // Save context
    prevctxt = t->ctxt;
    t->ctxt = ctxt;

    // Statistics
    intr_counter[ctxt->traptype]++;

    // Call interrupt handlers
    intr = intr_handlers[ctxt->traptype];
    if (!intr)
    {
        dbg_enter(ctxt, NULL);
    }
    else
    {
        while (intr)
        {
            rc = intr->handler(ctxt, intr->arg);
            if (rc > 0) break;
            intr = intr->next;
        }
    }

    // If we interrupted a user mode context, dispatch DPCs,
    // check for quantum expiry, and deliver signals.
    if (is_usermode(ctxt))
    {
        kdpc_check_queue();
        ksched_check_preempt();
        if (kthread_signals_ready(t)) deliver_pending_signals(0);
    }

    // Restore context
    t->ctxt = prevctxt;
}


/**
 * @brief Generic code that setup the stack and call the commom trap handler.
 *
 * This function is called by every interrupt handler registred on IDT.
 */
void isr() __asm("___isr");


int syscall(int syscallno, char *params, struct context *ctxt);


/**
 * @brief Kernel entry point for int 48 syscall
 */
void  systrap(void) __asm__("___systrap");


/**
 * @brief Kernel entry point for sysenter syscall.
 */
void sysentry(void) __asm__("___sysentry");


//
// Macros to generate interrupt service routines.
//

#define ISR(n) \
    void isr##n() __asm__("___isr" #n); \
    __asm__                \
    (                      \
        "___isr" #n ":"    \
        "push 0;"          \
        "push " #n ";"     \
        "jmp ___isr;"      \
    );                     \


#define ISRE(n) \
    void isr##n() __asm__("___isr" #n); \
    __asm__                \
    (                      \
        "___isr" #n ":"    \
        "push " #n ";"     \
        "jmp ___isr;"      \
    );                     \


ISR(0)  ISR(1)  ISR(2)   ISR(3)   ISR(4)   ISR(5)   ISR(6)   ISR(7)
ISRE(8) ISR(9)  ISRE(10) ISRE(11) ISRE(12) ISRE(13) ISRE(14) ISR(15)
ISR(16) ISR(17) ISR(18)  ISR(19)  ISR(20)  ISR(21)  ISR(22)  ISR(23)
ISR(24) ISR(25) ISR(26)  ISR(27)  ISR(28)  ISR(29)  ISR(30)  ISR(31)
ISR(32) ISR(33) ISR(34)  ISR(35)  ISR(36)  ISR(37)  ISR(38)  ISR(39)
ISR(40) ISR(41) ISR(42)  ISR(43)  ISR(44)  ISR(45)  ISR(46)  ISR(47)
        ISR(49) ISR(50)  ISR(51)  ISR(52)  ISR(53)  ISR(54)  ISR(55)
ISR(56) ISR(57) ISR(58)  ISR(59)  ISR(60)  ISR(61)  ISR(62)  ISR(63)


/**
 * @brief Setup a call frame for invoking the global signal handler.
 */
static struct siginfo *setup_signal_frame(unsigned long *esp)
{
    struct context *ctxt;
    struct siginfo *info;

    // Push context onto stack
    *esp -= sizeof(struct context);
    ctxt = (struct context *) *esp;

    // Push siginfo onto stack
    *esp -= sizeof(struct siginfo);
    info = (struct siginfo *) *esp;
    info->si_ctxt = ctxt;

    // Push pointer to signal info to stack
    *esp -= sizeof(struct siginfo *);
    *((struct siginfo **) *esp) = info;

    // Push dummy return address to stack
    *esp -= sizeof(void *);
    *((void **) *esp) = NULL;

    return info;
}

//
// send_signal
//
// Setup user stack to execute global signal handler
//

void send_signal(struct context *ctxt, int signum, void *addr) {
  unsigned long esp;
  struct siginfo *info;

  // Check for global handler
  if (!peb || !peb->globalhandler) {
    //panic("no global signal handler");
    dbg_enter(ctxt, addr);
  }

  // Get copy of user stack pointer
  esp = ctxt->esp;

  // Create a signal call frame
  info = setup_signal_frame(&esp);

  // Initialize signal info
  info->si_signo = signum;
  info->si_code = ctxt->traptype;
  memcpy(info->si_ctxt, ctxt, sizeof(struct context));
  info->si_addr = addr;

  // Set new stack pointer in user context
  ctxt->esp = esp;

  // Set instruction pointer to global signal handler routine
  ctxt->eip = (unsigned long) peb->globalhandler;
}

//
// send_user_signal
//
// Queue a signal for execution on thread. The signal will be delivered
// by different mechanisms based on the thread state:
//
// THREAD_STATE_INITIALIZED:
//   No signals can be delivered while the thread is initializing.
//
// THREAD_STATE_READY:
//   The pending signals are delivered when the thread is scheduled
//   to run again.
//
// THREAD_STATE_RUNNING:
//   The thread (self) is running. Pending signals will be handled before
//   kernel exit.
//
// THREAD_STATE_WAITING:
//   Interrupt thread if it is in an alertable wait.
//
// THREAD_STATE_TERMINATED:
//   Cannot deliver signals to a terminated thread.
//
// THREAD_STATE_SUSPENDED:
//   Signals cannot awaken suspended threads. They remain pending until the
//   thread resumes execution.
//
// THREAD_STATE_TRANSITION:
//   Signals are never sent in this intermediate state.
//

int send_user_signal(struct thread *t, int signum)
{
    // signal can only be sent to user threads
    if (!t->tib) return -EPERM;

    // add signal to the pending signal mask for thread
    t->pending_signals |= (1 << signum);

    // resume thread if signal is SIGCONT
    if (signum == SIGCONT) kthread_resume(t);

    // if not signals are ready for delivery just return
    if (!kthread_signals_ready(t)) return 0;

    // interrupt thread if it is in an alertable wait
    if (t->state == THREAD_STATE_WAITING) kthread_interrupt(t);

    return 0;
}


/**
 * @brief Examine and change blocked signals.
 */
int kthread_set_signal_mask(int how, sigset_t *set, sigset_t *oldset)
{
    struct thread *t = kthread_self();

    if (oldset) *oldset = t->blocked_signals;

    if (set)
    {
        switch (how)
        {
            case SIG_BLOCK:
                t->blocked_signals |= *set;
                break;

            case SIG_UNBLOCK:
                t->blocked_signals &= ~*set;
                break;

            case SIG_SETMASK:
                t->blocked_signals = *set;
                break;

            default:
                return -EINVAL;
        }

        if (kthread_signals_ready(t) && t->state == THREAD_STATE_WAITING) kthread_interrupt(t);
    }

    return 0;
}


/**
 * @brief Examine pending signals.
 */
int kthread_get_pending_signals( sigset_t *set )
{
    struct thread *t = kthread_self();

    if (!set) return -EINVAL;
    *set = t->pending_signals;

    return 0;
}

//
// deliver_pending_signals
//
// Deliver unblocked pending signals for current thread
//

int deliver_pending_signals(int retcode) {
  struct thread *t = kthread_self();
  struct context *ctxt;
  int sigmask;
  int signum;
  struct siginfo *info;
  unsigned long esp;

  // Find highest priority unblocked pending signal
  sigmask = kthread_signals_ready(t);
  if (sigmask == 0) return 0;
  signum = find_lowest_bit(sigmask);
  t->pending_signals &= ~(1 << signum);

  //
  // Now we must deliver the signal to the global signal handler.
  //
  // We must build a context that can be used by the sigexit handler for resuming
  // normal operation after the signal has been handled. This context is allocated
  // on the user stack for the thread together with the siginfo structure. A
  // stack frame for calling the global signal handler with the siginfo structure
  // as argument is put on the stack. The instruction pointer (eip) and stack
  // pointer (esp) are adjusted such that when the thread returns from the kernel
  // the global signal handler is called with the siginfo structure.
  //
  // When the global handler has handled the signal it calls sigexit() with the
  // siginfo structure. The sigexit handler uses the context in the siginfo
  // structure to resume normal operation.
  //

  // Get call context
  ctxt = t->ctxt;

  // Setup signal frame
  esp = ctxt->esp;
  info = setup_signal_frame(&esp);

  // Setup signal info
  info->si_signo = signum;
  info->si_code = 0;
  memcpy(info->si_ctxt, ctxt, sizeof(struct context));
  info->si_addr = NULL;

  // Setup return code in eax of context
  if (ctxt->traptype == INTR_SYSENTER || ctxt->traptype == INTR_SYSCALL) {
    if (retcode < 0) {
      info->si_ctxt->errcode = retcode;
      info->si_ctxt->eax = -1;
    } else{
      info->si_ctxt->eax = retcode;
    }
  }

  // Set new stack pointer in user context
  ctxt->esp = esp;

  // Set instruction pointer to global signal handler routine
  ctxt->eip = (unsigned long) peb->globalhandler;

  return 0;
}

//
// sigexit_handler
//
// Signal exit handler
//

static int sigexit_handler(struct context *ctxt, void *arg) {
  struct thread *t = kthread_self();
  struct siginfo *info;
  int debug;

  // Get parameters from sysexit() call
  debug = ctxt->eax;
  info = (struct siginfo *) ctxt->ebx;

  // Restore processor context
  //memcpy(ctxt, info->si_ctxt, sizeof(struct context));
  kthread_set_context(t, info->si_ctxt);

  // Do not allow interrupts to be disabled
  t->ctxt->eflags |= EFLAG_IF;

  // Restore errno to errcode in context
  if (ctxt->traptype == INTR_SYSENTER || ctxt->traptype == INTR_SYSCALL) {
    struct tib *tib = t->tib;
    if (tib) tib->errnum = -((int) ctxt->errcode);
  }

  if (debug) dbg_enter(ctxt, info->si_addr);

  return 1;
}

/**
 * @brief Handler for "division by zero" exception.
 */
static int div_handler(struct context *ctxt, void *arg)
{
    if (is_usermode(ctxt))
    {
        send_signal(ctxt, SIGFPE, (void *) ctxt->eip);
    }
    else
    {
        kprintf(KERN_CRIT "trap: division by zero in kernel mode\n");
        dbg_enter(ctxt, 0);
    }

    return 0;
}


/**
 * @brief Handler for "breakpoint" interrupt.
 */
static int breakpoint_handler(struct context *ctxt, void *arg)
{
    if (is_usermode(ctxt))
    {
        send_signal(ctxt, SIGTRAP, (void *) ctxt->eip);
    }
    else
    {
        kprintf(KERN_CRIT "trap: breakpoint in kernel mode\n");
        dbg_enter(ctxt, 0);
    }

    return 0;
}


/**
 * @brief Handler for "overflow" exception.
 */
static int overflow_handler(struct context *ctxt, void *arg)
{
    if (is_usermode(ctxt))
    {
        send_signal(ctxt, SIGSEGV, NULL);
    }
    else
    {
        kprintf(KERN_CRIT "trap: overflow exception in kernel mode\n");
        dbg_enter(ctxt, 0);
    }

    return 0;
}


/**
 * @brief Handler for "bounds" exception.
 */
static int bounds_handler(struct context *ctxt, void *arg)
{
    if (is_usermode(ctxt))
    {
        send_signal(ctxt, SIGSEGV, NULL);
    }
    else
    {
        kprintf(KERN_CRIT "trap: bounds exception in kernel mode\n");
        dbg_enter(ctxt, 0);
    }

    return 0;
}


/**
 * @brief Handler for "illegal instruction" exception.
 */
static int illop_handler(struct context *ctxt, void *arg)
{
    if (is_usermode(ctxt))
    {
        send_signal(ctxt, SIGILL, (void *) ctxt->eip);
    }
    else
    {
        kprintf(KERN_CRIT "trap: illegal instruction exception in kernel mode\n");
        dbg_enter(ctxt, 0);
    }

    return 0;
}


/**
 * @brief Handler for "segment not present" exception.
 */
static int seg_handler(struct context *ctxt, void *arg)
{
    if (is_usermode(ctxt))
    {
        send_signal(ctxt, SIGBUS, NULL);
    }
    else
    {
        kprintf(KERN_CRIT "trap: segment not present exception in kernel mode\n");
        dbg_enter(ctxt, 0);
    }

    return 0;
}


/**
 * @brief Handler for "stack segment" exception.
 */
static int stack_handler(struct context *ctxt, void *arg)
{
    if (is_usermode(ctxt))
    {
        send_signal(ctxt, SIGBUS, NULL);
    }
    else
    {
        kprintf(KERN_CRIT "trap: stack segment exception in kernel mode\n");
        dbg_enter(ctxt, 0);
    }

    return 0;
}


/**
 * @brief Handler for "general protection" exception.
 */
static int genpro_handler(struct context *ctxt, void *arg)
{
    if (is_usermode(ctxt))
    {
        send_signal(ctxt, SIGSEGV, NULL);
    }
    else
    {
        kprintf(KERN_CRIT "trap: general protection exception in kernel mode\n");
        dbg_enter(ctxt, 0);
    }

    return 0;
}


/**
 * @brief Handler for "page fault" exception.
 */
static int pagefault_handler(struct context *ctxt, void *arg)
{
    void *addr;
    void *pageaddr;

    addr = (void *) kmach_get_cr2();
    pageaddr = (void *) PAGEADDR(addr);

    if (is_usermode(ctxt))
    {
        int signal = SIGSEGV;
        // check if the address is mapped on page directory
        if (kpage_is_directory_mapped(pageaddr))
        {
            pte_t flags = kpage_get_flags(pageaddr);
            // check if the page is a guard
            if (flags & PT_GUARD)
            {
                kpage_unguard(pageaddr);
                signal = SIGSTKFLT;
                if (guard_page_handler(pageaddr) == 0) signal = 0;
            }
            else
            // chech if the page is in swap file (?)
            if (flags & PT_FILE)
            {
                if ((flags & PT_PRESENT) == 0)
                {
                    kmach_sti();
                    if (fetch_page(pageaddr) == 0) signal = 0;
                }
            }
        }
        if (signal != 0) send_signal(ctxt, signal, addr);
    }
    else
    {
        kprintf(KERN_CRIT "trap: page fault in kernel mode\n");
        dbg_enter(ctxt, addr);
    }

    return 0;
}


/**
 * Handler for "alignment" exception.
 */
static int alignment_handler(struct context *ctxt, void *arg)
{
    if (is_usermode(ctxt)) {
    send_signal(ctxt, SIGBUS, (void *) kmach_get_cr2());
    } else {
    kprintf(KERN_CRIT "trap: alignment exception in kernel mode\n");
    dbg_enter(ctxt, 0);
    }

    return 0;
}

/**
 * Statistical counters for traps.
 */
static int traps_proc(struct proc_file *pf, void *arg)
{
    int i;

    pprintf(pf, "no trap                                  count\n");
    pprintf(pf, "-- -------------------------------- ----------\n");

    for (i = 0; i < INTRS; i++)
    {
        if (intr_counter[i] != 0)
        {
            pprintf(pf,"%2d %-32s %10d\n", i, INTR_NAMES[i], intr_counter[i]);
        }
    }

    return 0;
}


/**
 * Register interrupt handler for interrupt
 */
void register_interrupt( struct interrupt *intr, int intrno, intrproc_t func, void *arg )
{
    intr->handler = func;
    intr->arg = arg;
    intr->flags = 0;
    intr->next = intr_handlers[intrno];
    intr_handlers[intrno] = intr;
}


/**
 * Remove interrupt handler for interrupt.
 */
void unregister_interrupt(struct interrupt *intr, int intrno)
{
    struct interrupt *i;

    if (intr_handlers[intrno] == intr)
    {
        intr_handlers[intrno] = intr->next;
    }
    else
    {
        for (i = intr_handlers[intrno]; i != NULL; i = i->next)
        {
            if (i->next == intr)
            {
                i->next = intr->next;
                break;
            }
        }
    }
    intr->next = NULL;
}


/**
 * Initialize traps and interrupts.
 */
void ktrap_init()
{
    int i;

    // Initialize interrupt dispatch table
    for (i = 0; i < INTRS; i++)
    {
        intr_handlers[i] = NULL;
        intr_counter[i] = 0;
    }

    // setup the x86 interrupt table
    kmach_set_idt_gate(0, isr0);
    kmach_set_idt_gate(1, isr1);
    kmach_set_idt_gate(2, isr2);
    kmach_set_idt_trap(3, isr3);
    kmach_set_idt_gate(4, isr4);
    kmach_set_idt_gate(5, isr5);
    kmach_set_idt_gate(6, isr6);
    kmach_set_idt_gate(7, isr7);
    kmach_set_idt_gate(8, isr8);
    kmach_set_idt_gate(9, isr9);
    kmach_set_idt_gate(10, isr10);
    kmach_set_idt_gate(11, isr11);
    kmach_set_idt_gate(12, isr12);
    kmach_set_idt_gate(13, isr13);
    kmach_set_idt_gate(14, isr14);
    kmach_set_idt_gate(15, isr15);
    kmach_set_idt_gate(16, isr16);
    kmach_set_idt_gate(17, isr17);
    kmach_set_idt_gate(18, isr18);
    kmach_set_idt_gate(19, isr19);
    kmach_set_idt_gate(20, isr20);
    kmach_set_idt_gate(21, isr21);
    kmach_set_idt_gate(22, isr22);
    kmach_set_idt_gate(23, isr23);
    kmach_set_idt_gate(24, isr24);
    kmach_set_idt_gate(25, isr25);
    kmach_set_idt_gate(26, isr26);
    kmach_set_idt_gate(27, isr27);
    kmach_set_idt_gate(28, isr28);
    kmach_set_idt_gate(29, isr29);
    kmach_set_idt_gate(30, isr30);
    kmach_set_idt_gate(31, isr31);
    kmach_set_idt_gate(32, isr32);
    kmach_set_idt_gate(33, isr33);
    kmach_set_idt_gate(34, isr34);
    kmach_set_idt_gate(35, isr35);
    kmach_set_idt_gate(36, isr36);
    kmach_set_idt_gate(37, isr37);
    kmach_set_idt_gate(38, isr38);
    kmach_set_idt_gate(39, isr39);
    kmach_set_idt_gate(40, isr40);
    kmach_set_idt_gate(41, isr41);
    kmach_set_idt_gate(42, isr42);
    kmach_set_idt_gate(43, isr43);
    kmach_set_idt_gate(44, isr44);
    kmach_set_idt_gate(45, isr45);
    kmach_set_idt_gate(46, isr46);
    kmach_set_idt_gate(47, isr47);
    kmach_set_idt_trap(48, systrap);
    kmach_set_idt_trap(49, isr49);
    kmach_set_idt_gate(50, isr50);
    kmach_set_idt_gate(51, isr51);
    kmach_set_idt_gate(52, isr52);
    kmach_set_idt_gate(53, isr53);
    kmach_set_idt_gate(54, isr54);
    kmach_set_idt_gate(55, isr55);
    kmach_set_idt_gate(56, isr56);
    kmach_set_idt_gate(57, isr57);
    kmach_set_idt_gate(58, isr58);
    kmach_set_idt_gate(59, isr59);
    kmach_set_idt_gate(60, isr60);
    kmach_set_idt_gate(61, isr61);
    kmach_set_idt_gate(62, isr62);
    kmach_set_idt_gate(63, isr63);

    // define OS interrupt handlers
    register_interrupt(&divintr, INTR_DIV, div_handler, NULL);
    register_interrupt(&brkptintr, INTR_BPT, breakpoint_handler, NULL);
    register_interrupt(&overflowintr, INTR_OVFL, overflow_handler, NULL);
    register_interrupt(&boundsintr, INTR_BOUND, bounds_handler, NULL);
    register_interrupt(&illopintr, INTR_INSTR, illop_handler, NULL);
    register_interrupt(&segintr, INTR_SEG, seg_handler, NULL);
    register_interrupt(&stackintr, INTR_STACK, stack_handler, NULL);
    register_interrupt(&genprointr, INTR_GENPRO, genpro_handler, NULL);
    register_interrupt(&pgfintr, INTR_PGFLT, pagefault_handler, NULL);
    register_interrupt(&alignintr, INTR_ALIGN, alignment_handler, NULL);
    register_interrupt(&sigexitintr, INTR_SIGEXIT, sigexit_handler, NULL);

    // Initialize fast syscall
    if (global_cpu.features & CPU_FEATURE_SEP)
    {
        kmach_wrmsr(MSR_SYSENTER_CS, SEL_KTEXT | global_kring, 0);
        kmach_wrmsr(MSR_SYSENTER_ESP, TSS_ESP0, 0);
        kmach_wrmsr(MSR_SYSENTER_EIP, (unsigned long) sysentry, 0);
    }

    // Register /proc/traps
    register_proc_inode("traps", traps_proc, NULL);
}
