//
// sched.h
//
// Task scheduler
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

#ifndef SCHED_H
#define SCHED_H


#include <stdint.h>


typedef void (*threadproc_t)(void *arg);
typedef void (*dpcproc_t)(void *arg);
typedef void (*taskproc_t)(void *arg);

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
 * Deferred Procedure Call (DPC) structure.
 *
 * @remark This is based on Microsoft Windows DPC.
 */
struct dpc
{
    dpcproc_t proc;
    void *arg;
    struct dpc *next;
    int flags;
};

struct task {
  taskproc_t proc;
  void *arg;
  struct task *next;
  int flags;
};

struct task_queue {
  struct task *head;
  struct task *tail;
  struct thread *thread;
  int maxsize;
  int size;
  int flags;
};

struct kernel_context {
  unsigned long esi, edi;
  unsigned long ebx, ebp;
  unsigned long eip;
  char stack[0];
};

extern struct thread *idlethread;
extern struct thread *threadlist;
extern struct task_queue sys_task_queue;

extern struct dpc *dpc_queue_head;
extern struct dpc *dpc_queue_tail;

extern int in_dpc;
extern int preempt;
extern unsigned long dpc_time;

#if 0
static __inline __declspec(naked) struct thread *self() {
  __asm {
    mov eax, esp;
    and eax, TCBMASK
    ret
  }
}
#endif

#if 0
static __inline struct thread *self() {
  unsigned long stkvar;
  return (struct thread *) (((unsigned long) &stkvar) & TCBMASK);
}
#endif

struct thread *self();

void mark_thread_running();

KERNELAPI void mark_thread_ready(struct thread *t, int charge, int boost);
KERNELAPI void enter_wait(int reason);
KERNELAPI int enter_alertable_wait(int reason);
KERNELAPI int kthread_interrupt(struct thread *t);

KERNELAPI struct thread *kthread_create_kland(threadproc_t startaddr, void *arg, int priority, char *name);

int kthread_create_uland(void *entrypoint, unsigned long stacksize, char *name, struct thread **retval);
int init_user_thread(struct thread *t, void *entrypoint);
int allocate_user_stack(struct thread *t, unsigned long stack_reserve, unsigned long stack_commit);
int kthread_destroy(struct thread *t);

struct thread *get_thread(tid_t tid);

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
 * If the 'suspended' count reach zero, the thread is marked as ready to run again.
 */
int kthread_resume(struct thread *t);

void terminate_thread(int exitcode);
void suspend_all_user_threads();
int schedule_alarm(unsigned int seconds);

int get_thread_priority(struct thread *t);
int set_thread_priority(struct thread *t, int priority);

KERNELAPI int init_task_queue(struct task_queue *tq, int priority, int maxsize, char *name);
KERNELAPI void init_task(struct task *task);
KERNELAPI int queue_task(struct task_queue *tq, struct task *task, taskproc_t proc, void *arg);

KERNELAPI void init_dpc(struct dpc *dpc);
KERNELAPI void queue_dpc(struct dpc *dpc, dpcproc_t proc, void *arg);
KERNELAPI void queue_irq_dpc(struct dpc *dpc, dpcproc_t proc, void *arg);

KERNELAPI void add_idle_task(struct task *task, taskproc_t proc, void *arg);

void idle_task();
KERNELAPI void yield();
void dispatch_dpc_queue();
void kthread_preempt();
KERNELAPI void dispatch();
KERNELAPI int system_idle();

void init_sched();

static __inline void check_dpc_queue() {
  if (dpc_queue_head) dispatch_dpc_queue();
}

static __inline void check_preempt() {
#ifndef NOPREEMPTION
  if (preempt) kthread_preempt();
#endif
}

static __inline int signals_ready(struct thread *t) {
  return t->pending_signals & ~t->blocked_signals;
}

#endif
