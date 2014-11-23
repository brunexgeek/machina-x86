//
// sched.h
//
// Task scheduler
//
// Copyright (C) 2014 Bruno Ribeiro. All rights reserved.
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

#ifndef MACHINA_SCHED_H
#define MACHINA_SCHED_H

//#define NOPREEMPTION

#define DEFAULT_QUANTUM          36
#define QUANTUM_UNITS_PER_TICK   3

/// Amount of priority levels. Lesser values is high priority.
#define THREAD_PRIORITY_LEVELS   32

/// Number of the pages per TCB
#define PAGES_PER_TCB     2
/// Size (in bytes) of a TCB
#define TCBSIZE           (PAGES_PER_TCB * PAGESIZE)
#define TCBMASK           (~(TCBSIZE - 1))
/// Offset of the ESP stored in a TCB structure
#define TCBESP            (TCBSIZE - 4)

#define DPC_QUEUED_BIT        0
#define DPC_EXECUTING_BIT     1
#define DPC_NORAND_BIT        2

#define DPC_QUEUED            (1 << DPC_QUEUED_BIT)
#define DPC_EXECUTING         (1 << DPC_EXECUTING_BIT)
#define DPC_NORAND            (1 << DPC_NORAND_BIT)

#define TASK_QUEUE_ACTIVE              1
#define TASK_QUEUE_ACTIVE_TASK_INVALID 2

#define TASK_QUEUED       1
#define TASK_EXECUTING    2


#ifndef __ASSEMBLER__


#include <os/krnl.h>
#include <os/syspage.h>
#include <stdint.h>


/**
 * Prototype for thread functions.
 */
typedef void (*threadproc_t)(void *arg);

/**
 * Prototype for DPC functions.
 */
typedef void (*dpcproc_t)(void *arg);

/**
 * Prototype for task functions.
 */
typedef void (*taskproc_t)(void *arg);

/**
 * Thread Control Block (TCB) structure.
 */
struct tcb
{
    struct thread thread;
    uint32_t stack[(TCBSIZE - sizeof(struct thread)) / sizeof(uint32_t) - 1];
    uint32_t *esp;
};


/**
 * Delayed Procedure Call (DPC) structure.
 *
 * @remark This is based on Microsoft Windows DPC.
 */
struct dpc
{
    dpcproc_t proc;
    const char *proc_name;
    void *arg;
    struct dpc *next;
    int flags;
};


/**
 * Task structure.
 */
struct task
{
    taskproc_t proc;
    void *arg;
    struct task *next;
    int flags;
};


/**
 * Task queue structure.
 */
struct task_queue
{
    struct task *head;
    struct task *tail;
    struct thread *thread;
    int maxsize;
    int size;
    int flags;
};


struct kernel_context
{
    unsigned long esi, edi;
    unsigned long ebx, ebp;
    unsigned long eip;
    char stack[0];
};


//extern struct thread *idlethread;
//extern struct thread *threadlist;
// TODO: improve 'ktask' interface to hide this variable
extern struct task_queue sys_task_queue;

//extern struct dpc *dpc_queue_head;
//extern struct dpc *dpc_queue_tail;

extern int in_dpc;
extern int preempt;
extern unsigned long dpc_time;


//
// Thread sybsystem
//

/**
 * Returns the current running thread.
 */
struct thread *kthread_self(void) __asm__("___kthread_self");

/**
 * Mark a thread as ready to run.
 */
KERNELAPI void kthread_ready(struct thread *t, int charge, int boost);

/**
 * Block a thread until it is marked as ready to run.
 */
KERNELAPI void kthread_wait(int reason);

/**
 * Block a thread until it is marked as ready to run.
 */
KERNELAPI int kthread_alertable_wait(int reason);

/**
 * Interrupt the execution of the given thread.
 */
KERNELAPI int kthread_interrupt(struct thread *t);

/**
 * Create a new thread in kernel mode.
 */
KERNELAPI struct thread *kthread_create_kland(threadproc_t startaddr, void *arg, int priority, char *name);

/**
 * Create a new thread in user mode.
 */
int kthread_create_uland(void *entrypoint, unsigned long stacksize, char *name, struct thread **retval);

/**
 * @brief Destroy the given thread.
 *
 * The memory allocated for the thread structure will not be released.
 */
int kthread_destroy(struct thread *t);

/**
 * @brief Increase the 'suspended' counter of the given thread.
 *
 * The thread will also set to THREAD_STATE_SUSPENDED state. If the thread are running,
 * it will be preepted.
 */
int kthread_suspend(struct thread *t);

/**
 * @brief Decrease the 'suspended' counter of the given thread.
 *
 * If the 'suspended' counter reach zero, the thread is marked as ready to run again.
 */
int kthread_resume(struct thread *t);

/**
 * Terminate the current running thread.
 */
void kthread_terminate(int exitcode);

/**
 * @brief Preempt the current running thread.
 *
 * A new quantum will be assigned to the thread and the next ready thread will be marked
 * as running.
 *
 * @remark This function enable interrupts.
 */
void kthread_preempt();

int schedule_alarm(unsigned int seconds);

/**
 * Get the priority of the given thread.
 */
int kthread_get_priority( struct thread *t );

/**
 * Set the priority of the given thread.
 */
int kthread_set_priority( struct thread *t, int priority );

/**
 * @brief Makes the current thread relinquish the CPU.
 *
 * The thread will give up your quantum and the next ready thread will be marked
 * as running.
 */
KERNELAPI void kthread_yield();

/**
 * Returns the thread which have the given ID.
 */
struct thread *kthread_get(tid_t tid);

static __inline int kthread_signals_ready(struct thread *t)
{
    return t->pending_signals & ~t->blocked_signals;
}


/**
 * Retrieves the processor context for a thread
 */
int kthread_get_context(struct thread *t, struct context *ctxt);


/**
 * Sets the processor context for a thread
 */
int kthread_set_context(struct thread *t, struct context *ctxt);


//
// DPC subsystem
//

/**
 * Initialize a DPC object.
 */
KERNELAPI void kdpc_create(struct dpc *dpc);

/**
 * @brief Add the given DPC object into the scheduler queue.
 *
 * This function try to enable interrupts before exit, thus dont should be used
 * inside an interrupt handler.
 */
KERNELAPI void kdpc_queue(struct dpc *dpc, dpcproc_t proc, void *arg);

/**
 * @brief Add the given DPC object into the scheduler queue.
 *
 * This function is safe to be used inside an interrupt handler.
 */
KERNELAPI void kdpc_queue_irq(struct dpc *dpc, dpcproc_t proc, const char *proc_name, void *arg);

/**
 * @brief Start to execute all registred DPCs.
 *
 * This function will block until exist some DPC to be executed.
 */
void kdpc_dispatch_queue();

/**
 * @brief Check if have pending DPC in the scheduler queue and, if any, execute them.
 *
 * This function will block until exist some DPC to be executed.
 */
void kdpc_check_queue();


//
// Task scheduler subsystem
//

void ksched_init();
void ksched_destroy();

/**
 * @brief Start the task scheduler idle loop.
 *
 * @remark This is a endless function. From this point only threads is executed.
 */
void ksched_idle();

/**
 * Change the current running thread to the next ready one.
 */
KERNELAPI void ksched_dispatch();

/**
 * Returns a non-zero integer indicating if the system is in idle state.
 */
KERNELAPI int ksched_is_system_idle();

/**
 * @brief Add a task in the idle queue.
 *
 * All tasks in the idle queue will be executed only when the ksched_is_system_idle()
 * returns a non-zero value.
 */
KERNELAPI void ksched_add_idle_task(struct task *task, taskproc_t proc, void *arg);

static __inline void ksched_check_preempt()
{
#ifndef NOPREEMPTION
    if (preempt) kthread_preempt();
#endif
}


//
// Task subsystem
//


KERNELAPI int init_task_queue(struct task_queue *tq, int priority, int maxsize, char *name);
KERNELAPI void init_task(struct task *task);
KERNELAPI int queue_task(struct task_queue *tq, struct task *task, taskproc_t proc, void *arg);


int init_user_thread(struct thread *t, void *entrypoint);
int allocate_user_stack(struct thread *t, unsigned long stack_reserve, unsigned long stack_commit);
void kthread_mark_running();


#endif  // __ASSEMBLER__
#endif  // MACHINA_SCHED_H
