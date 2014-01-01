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

#include <os/sched.h>
#include <os/krnl.h>
#include <os/procfs.h>
#include <os/vmm.h>
#include <os/kmem.h>
#include <os/dbg.h>
#include <os/trap.h>
#include <os/rnd.h>


#define DEFAULT_STACK_SIZE           (1 * 1024 * 1024)
#define DEFAULT_INITIAL_STACK_COMMIT (8 * 1024)

int preempt = 0;
int in_dpc = 0;
unsigned long dpc_time = 0;
unsigned long dpc_total = 0;
unsigned long dpc_lost = 0;
unsigned long thread_ready_summary = 0;

static struct thread *idle_thread = NULL;

/**
 * Head of the queue for ready thread's.
 */
struct thread *ready_queue_head[THREAD_PRIORITY_LEVELS] = { NULL };

/**
 * Tail of the queue for ready thread's.
 */
struct thread *ready_queue_tail[THREAD_PRIORITY_LEVELS] = { NULL };

/**
 * List containing all threads.
 */
struct thread *threadlist = NULL;

struct dpc *dpc_queue_head = NULL;
struct dpc *dpc_queue_tail = NULL;

struct task *idle_tasks_head = NULL;
struct task *idle_tasks_tail = NULL;

struct task_queue sys_task_queue;

static void tmr_alarm(void *arg);

void user_thread_start(void *arg);
void init_thread_stack(struct thread *t, void *startaddr, void *arg);
void switch_context(struct thread *t) __asm__("___switch_context");
int init_user_thread(struct thread *t, void *entrypoint);
int allocate_user_stack(struct thread *t, unsigned long stack_reserve, unsigned long stack_commit);
static struct dpc *kdpc_get_next();

void mark_thread_running();

/**
 * Insert 't2' before 't1'.
 */
static void insert_before( struct thread *t1, struct thread *t2 )
{
    t2->next = t1;
    t2->prev = t1->prev;
    t1->prev->next = t2;
    t1->prev = t2;
}


static void insert_after(struct thread *t1, struct thread *t2)
{
    t2->next = t1->next;
    t2->prev = t1;
    t1->next->prev = t2;
    t1->next = t2;
}


static void remove(struct thread *t)
{
    t->next->prev = t->prev;
    t->prev->next = t->next;
}

/**
 * Insert the given thread at head of the ready queue.
 */
static void insert_ready_head( struct thread *t )
{
    if (!ready_queue_head[t->priority])
    {
        t->next_ready = t->prev_ready = NULL;
        ready_queue_head[t->priority] = ready_queue_tail[t->priority] = t;
        thread_ready_summary |= (1 << t->priority);
    }
    else
    {
        t->next_ready = ready_queue_head[t->priority];
        t->prev_ready = NULL;
        t->next_ready->prev_ready = t;
        ready_queue_head[t->priority] = t;
    }
}


/**
 * Insert the given thread at tail of the ready queue.
 */
static void insert_ready_tail(struct thread *t)
{
    if (!ready_queue_tail[t->priority])
    {
        t->next_ready = t->prev_ready = NULL;
        ready_queue_head[t->priority] = ready_queue_tail[t->priority] = t;
        thread_ready_summary |= (1 << t->priority);
    }
    else
    {
        t->next_ready = NULL;
        t->prev_ready = ready_queue_tail[t->priority];
        t->prev_ready->next_ready = t;
        ready_queue_tail[t->priority] = t;
    }
}


/**
 * Remove the given thread from ready queue.
 */
static void remove_from_ready_queue( struct thread *t )
{
    if (t->next_ready) t->next_ready->prev_ready = t->prev_ready;
    if (t->prev_ready) t->prev_ready->next_ready = t->next_ready;
    if (t == ready_queue_head[t->priority]) ready_queue_head[t->priority] = t->next_ready;
    if (t == ready_queue_tail[t->priority]) ready_queue_tail[t->priority] = t->prev_ready;
    if (!ready_queue_tail[t->priority]) thread_ready_summary &= ~(1 << t->priority);
}


void kthread_wait(int reason)
{
    struct thread *t = kthread_self();

    t->state = THREAD_STATE_WAITING;
    t->wait_reason = reason;
    //kprintf("%s: waiting\n", t->name);
    ksched_dispatch();
    //kprintf("%s: back\n", t->name);
}


int kthread_alertable_wait(int reason)
{
    struct thread *t = kthread_self();

    if (signals_ready(t)) return -EINTR;

    t->state = THREAD_STATE_WAITING;
    t->wait_reason = reason;
    t->flags &= ~THREAD_INTERRUPTED;

    t->flags |= THREAD_ALERTABLE;
    ksched_dispatch();
    t->flags &= ~THREAD_ALERTABLE;

    if (t->flags & THREAD_INTERRUPTED)
    {
        t->flags &= ~THREAD_INTERRUPTED;
        return -EINTR;
    }

    return 0;
}

int kthread_interrupt(struct thread *t)
{
    if (t->state != THREAD_STATE_WAITING) return -EBUSY;
    if ((t->flags & THREAD_ALERTABLE) == 0) return -EPERM;
    t->flags |= THREAD_INTERRUPTED;
    kthread_ready(t, 1, 1);
    return 0;
}

void kthread_ready(struct thread *t, int charge, int boost)
{
    //int prio = t->priority;
    int newprio;

    // check for suspended thread that is now ready to run
    if (t->suspend_count > 0)
    {
        t->state = THREAD_STATE_SUSPENDED;
        return;
    }

    // if thread has been interrupted it is already ready
    if ((t->flags & THREAD_INTERRUPTED) !=0 && t->state == THREAD_STATE_READY) return;

    // Charge quantums units each time a thread is restarted
    t->quantum -= charge;

    // Add boost to threads dynamic priority
    // Boosting is applied to the threads base priority
    // Boosting must never move threads to the realtime range
    // Boosting must never decrease the dynamic priority
    newprio = t->base_priority + boost;
    if (newprio > PRIORITY_TIME_CRITICAL) newprio = PRIORITY_TIME_CRITICAL;
    if (newprio > t->priority) t->priority = newprio;

    // Set thread state to ready
    if (t->state == THREAD_STATE_READY) panic("thread already ready");
    t->state = THREAD_STATE_READY;

    // Insert thread in ready queue
    if (t->quantum > 0)
    {
        // Thread has some quantum left. Insert it at the head of the
        // ready queue for its priority.
        insert_ready_head(t);
    }
    else
    {
        // The thread has exhausted its CPU quantum. Assign a new quantum
        t->quantum = DEFAULT_QUANTUM;

        // Let priority decay towards base priority
        if (t->priority > t->base_priority) t->priority--;

        // Insert it at the end of the ready queue for its priority.
        insert_ready_tail(t);
    }

    // Signal preemption if new ready thread has priority over the running thread
    if (t->priority > kthread_self()->priority) preempt = 1;
}


void kthread_preempt()
{
    struct thread *t = kthread_self();

    // Enable interrupt in case we have been called in interupt context
    kmach_sti();

    // Count number of preempted context switches
    t->preempts++;

    // Assign a new quantum if quantum expired
    if (t->quantum <= 0)
    {
        t->quantum = DEFAULT_QUANTUM;

        // Let priority decay towards base priority
        if (t->priority > t->base_priority) t->priority--;
    }

    // Thread is ready to run
    t->state = THREAD_STATE_READY;

    // Insert thread at the end of the ready queue for its priority.
    insert_ready_tail(t);

    // Relinquish CPU
    ksched_dispatch();
}


void kernel_thread_start(void *arg)
{
    struct thread *t = kthread_self();

    // Mark thread as running
    mark_thread_running();

    // Call entry point
    ((void (*)(void *)) (t->entrypoint))(arg);
}


static struct thread *kthread_create(threadproc_t startaddr, void *arg, int priority)
{
    // Allocate a new aligned thread control block
    struct thread *t = (struct thread *) alloc_pages_align(PAGES_PER_TCB, PAGES_PER_TCB, 0x00544342 /*"TCB"*/);
    if (!t) return NULL;
    memset(t, 0, PAGES_PER_TCB * PAGESIZE);
    init_thread(t, priority);

    // initialize the thread stack to start executing the thread function
    init_thread_stack(t, (void *) startaddr, arg);

    // Add thread to thread list
    insert_before(threadlist, t);

    // Signal preemption if new ready thread has priority over the running thread
    if (t->priority > kthread_self()->priority) preempt = 1;

    return t;
}


struct thread *kthread_create_kland(threadproc_t startaddr, void *arg, int priority, char *name)
{
    struct thread *t;

    // Create new thread object
    t = kthread_create(kernel_thread_start, arg, priority);
    if (!t) return NULL;
    if (name) strncpy(t->name, name, THREAD_NAME_LEN - 1);
    t->entrypoint = startaddr;

    // Mark thread as ready to run
    kthread_ready(t, 0, 0);

    // Notify debugger
    dbg_notify_create_thread(t, startaddr);

    return t;
}


int kthread_create_uland(void *entrypoint, unsigned long stacksize, char *name, struct thread **retval)
{
    struct thread *creator = kthread_self();
    struct thread *t;
    int rc;

    // determine stacksize
    if (stacksize == 0)
        stacksize = DEFAULT_STACK_SIZE;
    else
        stacksize = PAGES(stacksize) * PAGESIZE;

    // create and initialize new TCB and suspend thread
    t = kthread_create(user_thread_start, NULL, PRIORITY_NORMAL);
    if (!t) return -ENOMEM;
    if (name) strncpy(t->name, name, THREAD_NAME_LEN - 1);
    t->suspend_count++;

    // Inherit effective user and group from creator thread
    t->ruid = t->euid = creator->euid;
    t->rgid = t->egid = creator->egid;
    t->ngroups = creator->ngroups;
    memcpy(t->groups, creator->groups, creator->ngroups * sizeof(gid_t));
    strcpy(t->curdir, creator->curdir);

    // Create and initialize new TIB
    rc = init_user_thread(t, entrypoint);
    if (rc < 0) return rc;

    // Allocate user stack
    rc = allocate_user_stack(t, stacksize, DEFAULT_INITIAL_STACK_COMMIT);
    if (rc < 0) return rc;

    // Allocate self handle (it is also stored in the tib for fast access)
    t->hndl = t->tib->hndl = halloc(&t->object);

    // Protect handle from being closed
    hprotect(t->hndl);

    // Initialize signal alarm timer
    ktimer_init(&t->alarm, tmr_alarm, t);

    // Notify debugger
    dbg_notify_create_thread(t, entrypoint);

    *retval = t;
    return 0;
}


int init_user_thread(struct thread *t, void *entrypoint)
{
    struct tib *tib;

    // Allocate and initialize thread information block for thread
    tib = vmalloc(NULL, sizeof(struct tib), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE, 0x00544942 /*"TIB"*/, NULL);
    if (!tib) return -ENOMEM;

    t->entrypoint = entrypoint;
    t->tib = tib;
    tib->self = tib;
    tib->tlsbase = &tib->tls;
    tib->pid = 1;
    tib->tid = t->id;
    tib->peb = peb;
    tib->except = (struct xcptrec *) 0xFFFFFFFF;

    return 0;
}


int allocate_user_stack(struct thread *t, unsigned long stack_reserve, unsigned long stack_commit)
{
    char *stack;
    struct tib *tib;

    stack = vmalloc(NULL, stack_reserve, MEM_RESERVE, PAGE_READWRITE, 0x0053544b /*"STK"*/, NULL);
    if (!stack) return -ENOMEM;

    tib = t->tib;
    tib->stackbase = stack;
    tib->stacktop = stack + stack_reserve;
    tib->stacklimit = stack + (stack_reserve - stack_commit);

    if (!vmalloc(tib->stacklimit, stack_commit, MEM_COMMIT, PAGE_READWRITE, 0x0053544b /*"STK"*/, NULL)) return -ENOMEM;
    if (vmprotect(tib->stacklimit, PAGESIZE, PAGE_READWRITE | PAGE_GUARD) < 0) return -ENOMEM;

    return 0;
}


static void destroy_tcb(void *arg)
{
    // Deallocate TCB
    free_pages(arg, PAGES_PER_TCB);

    // Set the TASK_QUEUE_ACTIVE_TASK_INVALID flag, to inform the sys task queue that the
    // executing task is invalid. The destroy_tcb task is placed in the TCB, and we dont want
    // the flag to be updated after this task finish executing, because we have deallocated the
    // task.
    sys_task_queue.flags |= TASK_QUEUE_ACTIVE_TASK_INVALID;
}


int kthread_destroy(struct thread *t)
{
    struct task *task;

    // We can only remove terminated threads
    if (t->state != THREAD_STATE_TERMINATED) panic("thread terminated in invalid state");

    // Deallocate user context
    if (t->tib)
    {
        // Deallocate user stack
        if (t->tib->stackbase)
        {
            vmfree(t->tib->stackbase, (char *) (t->tib->stacktop) - (char *) (t->tib->stackbase), MEM_RELEASE);
            t->tib->stackbase = NULL;
        }

        // Deallocate TIB
        vmfree(t->tib, sizeof(struct tib), MEM_RELEASE);
        t->tib = NULL;
    }

    // Notify debugger
    dbg_notify_exit_thread(t);

    // Remove thread from thread list
    remove(t);

    // Add task to delete the TCB
    task = (struct task *) (t + 1);
    init_task(task);
    queue_task(&sys_task_queue, task, destroy_tcb, t);

    return 0;
}


struct thread *kthread_get(tid_t tid)
{
    struct thread *t = threadlist;

    while (1)
    {
        if (t->id == tid) return t;
        t = t->next;
        if (t == threadlist) return NULL;
    }
}


int kthread_suspend(struct thread *t)
{
    int prevcount = t->suspend_count++;

    if (prevcount == 0)
    {
        if (t->state == THREAD_STATE_READY)
        {
            remove_from_ready_queue(t);
            t->state = THREAD_STATE_SUSPENDED;
        }
        else
        if (t->state == THREAD_STATE_RUNNING)
        {
            t->state = THREAD_STATE_SUSPENDED;
            ksched_dispatch();
        }
    }

    return prevcount;
}


int kthread_resume( struct thread *t )
{
    int prevcount = t->suspend_count;

    if (t->suspend_count > 0)
    {
        if (--(t->suspend_count) == 0)
        {
            if (t->state == THREAD_STATE_SUSPENDED || t->state == THREAD_STATE_INITIALIZED)
                kthread_ready(t, 0, 0);
        }
    }

    return prevcount;
}


/**
 * Suspend the execution of all threads other than the current one.
 */
static void ksched_suspend_all_threads()
{
    struct thread *t = threadlist;

    if (threadlist == NULL) return;

    while (1)
    {
        if (t->tib && t != kthread_self())
            kthread_suspend(t);
        t = t->next;
        if (t == threadlist) break;
    }
}


static void tmr_alarm(void *arg)
{
    struct thread *t = arg;
    send_user_signal(t, SIGALRM);
}


int schedule_alarm(unsigned int seconds)
{
    struct thread *t = kthread_self();
    int rc = 0;

    if (t->alarm.active) rc = (t->alarm.expires - ticks) / HZ;
    if (seconds == 0)
    {
        ktimer_remove(&t->alarm);
    }
    else
    {
        ktimer_modify(&t->alarm, ticks + seconds * HZ);
    }

    return rc;
}


void kthread_terminate(int exitcode)
{
    struct thread *t = kthread_self();

    t->state = THREAD_STATE_TERMINATED;
    t->exitcode = exitcode;
    ktimer_remove(&t->alarm);
    kthread_exit(t);

    hunprotect(t->hndl);
    hfree(t->hndl);

    // switch to another thread
    ksched_dispatch();
}


int kthread_get_priority(struct thread *t)
{
    return t->priority;
}


int kthread_set_priority(struct thread *t, int priority)
{
    if (priority < 0 || priority >= THREAD_PRIORITY_LEVELS) return -EINVAL;
    if (t->base_priority == priority) return 0;

    // check if the thread is tying to change it own priority
    if (t == kthread_self())
    {
        t->base_priority = t->priority = priority;
        // we need to reschedule if new priority is lower
        if (priority < t->priority)
        {
            kthread_ready(t, 0, 0);
            ksched_dispatch();
        }
    }
    else
    {
        // Note: if thread is ready to run, we remove it from the current ready queue
        //       and insert int the ready queue for the new priority. Otherwise, this
        //       will be done automacally when the thread runs again.
        if (t->state == THREAD_STATE_READY)
        {
            remove_from_ready_queue(t);
            t->base_priority = t->priority = priority;
            t->state = THREAD_STATE_TRANSITION;
            kthread_ready(t, 0, 0);
        }
        else
        {
            t->base_priority = t->priority = priority;
        }
    }

    return 0;
}


static void task_queue_task(void *tqarg)
{
    struct task_queue *tq = tqarg;
    struct task *task;
    taskproc_t proc;
    void *arg;

    while (1)
    {
        // Wait until tasks arrive on the task queue
        while (tq->head == NULL)
        {
            kthread_wait(THREAD_WAIT_TASK);
        }

        // Get next task from task queue
        task = tq->head;
        tq->head = task->next;
        if (tq->tail == task) tq->tail = NULL;
        tq->size--;

        // Execute task
        task->flags &= ~TASK_QUEUED;
        if ((task->flags & TASK_EXECUTING) == 0)
        {
            task->flags |= TASK_EXECUTING;
            proc = task->proc;
            arg = task->arg;
            tq->flags |= TASK_QUEUE_ACTIVE;

            proc(arg);

            if (!(tq->flags & TASK_QUEUE_ACTIVE_TASK_INVALID)) task->flags &= ~TASK_EXECUTING;
            tq->flags &= ~(TASK_QUEUE_ACTIVE | TASK_QUEUE_ACTIVE_TASK_INVALID);
        }
    }
}


int init_task_queue(struct task_queue *tq, int priority, int maxsize, char *name)
{
    memset(tq, 0, sizeof(struct task_queue));
    tq->maxsize = maxsize;
    tq->thread = kthread_create_kland(task_queue_task, tq, priority, name);

    return 0;
}

void init_task(struct task *task)
{
    task->proc = NULL;
    task->arg = NULL;
    task->next = NULL;
    task->flags = 0;
}


int queue_task(struct task_queue *tq, struct task *task, taskproc_t proc, void *arg)
{
    if (!tq) tq = &sys_task_queue;
    if (task->flags & TASK_QUEUED) return -EBUSY;
    if (tq->maxsize != INFINITE && tq->size >= tq->maxsize) return -EAGAIN;

    task->proc = proc;
    task->arg = arg;
    task->next = NULL;
    task->flags |= TASK_QUEUED;

    if (tq->tail)
    {
        tq->tail->next = task;
        tq->tail = task;
    }
    else
    {
        tq->head = tq->tail = task;
    }

    tq->size++;

    if ((tq->flags & TASK_QUEUE_ACTIVE) == 0 && tq->thread->state == THREAD_STATE_WAITING)
    {
        kthread_ready(tq->thread, 0, 0);
    }

    return 0;
}


void kdpc_create( struct dpc *dpc )
{
    dpc->proc = NULL;
    dpc->arg = NULL;
    dpc->next = NULL;
    dpc->flags = 0;
}


void kdpc_queue_irq( struct dpc *dpc, dpcproc_t proc,  const char *proc_name, void *arg)
{
    if (dpc->flags & DPC_QUEUED)
    {
        dpc_lost++;
        return;
    }

    dpc->proc = proc;
    dpc->proc_name = proc_name;
    dpc->arg = arg;
    dpc->next = NULL;
    if (dpc_queue_tail) dpc_queue_tail->next = dpc;
    dpc_queue_tail = dpc;
    if (!dpc_queue_head) dpc_queue_head = dpc;
    set_bit(&dpc->flags, DPC_QUEUED_BIT);
    //kprintf("[DEBUG] installed DPC for '%s'\n", (proc_name == NULL) ? "<unnamed_proc>" : proc_name);
}


void kdpc_queue( struct dpc *dpc, dpcproc_t proc, void *arg)
{
    kmach_cli();
    kdpc_queue_irq(dpc, proc, NULL, arg);
    kmach_sti();
}


static struct dpc *kdpc_get_next()
{
    struct dpc *dpc;

    kmach_cli();

    if (dpc_queue_head)
    {
        dpc = dpc_queue_head;
        dpc_queue_head = dpc->next;
        if (dpc_queue_tail == dpc) dpc_queue_tail = NULL;
    }
    else
    {
        dpc = NULL;
    }

    kmach_sti();

    return dpc;
}

void kdpc_check_queue()
{
    if (dpc_queue_head) kdpc_dispatch_queue();
}


void kdpc_dispatch_queue()
{
    struct dpc *dpc;
    dpcproc_t proc;
    void *arg;

    if (in_dpc) panic("sched: nested execution of dpc queue");
    in_dpc = 1;

    while (1) {
        // Get next deferred procedure call
        // As a side effect this will enable interrupts
        dpc = kdpc_get_next();
        if (!dpc) break;

        // Execute DPC
        proc = dpc->proc;
        arg = dpc->arg;
        clear_bit(&dpc->flags, DPC_QUEUED_BIT);

        if ((dpc->flags & DPC_EXECUTING) == 0)
        {
            set_bit(&dpc->flags, DPC_EXECUTING_BIT);
            proc(arg);
            clear_bit(&dpc->flags, DPC_EXECUTING_BIT);
            dpc_total++;
        }

        #ifdef RANDOMDEV
        if ((dpc->flags & DPC_NORAND) == 0) add_dpc_randomness(dpc);
        #endif
    }

    in_dpc = 0;
}


static struct thread *find_ready_thread()
{
    int prio;
    struct thread *t;

    // Find highest priority non-empty ready queue
    if (thread_ready_summary == 0) return NULL;
    prio = find_highest_bit(thread_ready_summary);

    // Remove thread from ready queue
    t = ready_queue_head[prio];
    if (!t->next_ready)
    {
        ready_queue_head[prio] = ready_queue_tail[prio] = NULL;
        thread_ready_summary &= ~(1 << prio);
    }
    else
    {
        t->next_ready->prev_ready = NULL;
        ready_queue_head[prio] = t->next_ready;
    }

    t->next_ready = NULL;
    t->prev_ready = NULL;

    return t;
}


void ksched_dispatch()
{
    struct thread *curthread = kthread_self();
    struct thread *t;

    // Clear preemption flag
    preempt = 0;

    // Execute all queued DPCs
    if (dpc_queue_head) kdpc_dispatch_queue();

    // Find next thread to run
    t = find_ready_thread();
    if (!t) panic("No thread ready to run");

    // If current thread has been selected to run again then just return
    if (t == curthread)
    {
        t->state = THREAD_STATE_RUNNING;
        return;
    }

    // Save fpu state if fpu has been used
    if (curthread->flags & THREAD_FPU_ENABLED)
    {
        fpu_disable(&curthread->fpustate);
        t->flags &= ~THREAD_FPU_ENABLED;
    }

    // Switch to new thread
    //kprintf("[TRACE] switch to '%s'\n", t->name);
    switch_context(t);
    //kprintf("[TRACE] back to '%s'\n", curthread->name);
    #ifdef VMACH
    switch_kernel_stack();
    #endif

    // Mark new thread as running
    mark_thread_running();
}

void kthread_yield()
{
    struct thread *t = kthread_self();

    // Give up remaining quantum
    t->quantum = 0;

    // Mark thread as ready to run
    kthread_ready(t, 0, 0);

    // Dispatch next thread
    ksched_dispatch();
}

int ksched_is_system_idle()
{
    if (thread_ready_summary != 0) return 0;
    if (dpc_queue_head != NULL) return 0;
    return 1;
}

void ksched_add_idle_task(struct task *task, taskproc_t proc, void *arg)
{
    task->proc = proc;
    task->arg = arg;
    task->next = NULL;
    task->flags |= TASK_QUEUED;

    if (idle_tasks_tail)
    {
        idle_tasks_tail->next = task;
        idle_tasks_tail = task;
    }
    else
    {
        idle_tasks_head = idle_tasks_tail = task;
    }
}

void ksched_idle()
{
    struct thread *t = kthread_self();

    while (1)
    {
        struct task *task = idle_tasks_head;
        if (task)
        {
            while (task)
            {
                if (!ksched_is_system_idle()) break;
                task->flags |= TASK_EXECUTING;
                task->proc(task->arg);
                task->flags &= ~TASK_EXECUTING;
                task = task->next;
            }
        }
        else
        {
            if (ksched_is_system_idle())
            {
                kmach_halt();
            }
        }

        kthread_ready(t, 0, 0);
        ksched_dispatch();
        //if ((eflags() & EFLAG_IF) == 0) panic("sched: interrupts disabled in idle loop");
    }
}

static int threads_proc(struct proc_file *pf, void *arg) {
    static char *threadstatename[] = {"init", "ready", "run", "wait", "term", "susp", "trans"};
    static char *waitreasonname[] = {"wait", "fileio", "taskq", "sockio", "sleep", "pipe", "devio"};
    struct thread *t = threadlist;
    char *state;
    unsigned long stksiz;

    pprintf(pf, "tid tcb      hndl state  prio s #h   user kernel ctxtsw stksiz name\n");
    pprintf(pf, "--- -------- ---- ------ ---- - -- ------ ------ ------ ------ --------------\n");
    while (1)
    {
        if (t->state == THREAD_STATE_WAITING)
        {
            state = waitreasonname[t->wait_reason];
        }
        else
        {
            state = threadstatename[t->state];
        }

        if (t->tib)
        {
            stksiz = (char *) (t->tib->stacktop) - (char *) (t->tib->stacklimit);
        } else {
            stksiz = 0;
        }

        pprintf(pf,"%3d %p %4d %-6s %2d%+2d %1d %2d%7d%7d%7d%6dK %s\n",
        t->id, t, t->hndl, state, t->base_priority, t->priority - t->base_priority,
        t->suspend_count, t->object.handle_count,
        t->utime, t->stime, t->context_switches,
        stksiz / 1024,
        t->name);

        t = t->next;
        if (t == threadlist) break;
    }

    return 0;
}

static int dpcs_proc(struct proc_file *pf, void *arg)
{
    pprintf(pf, "dpc time   : %8d\n", dpc_time);
    pprintf(pf, "total dpcs : %8d\n", dpc_total);
    pprintf(pf, "lost dpcs  : %8d\n", dpc_lost);

    return 0;
}


void ksched_init()
{
    // initialize scheduler
    dpc_queue_head = dpc_queue_tail = NULL;
    memset(ready_queue_head, 0, sizeof(ready_queue_head));
    memset(ready_queue_tail, 0, sizeof(ready_queue_tail));

    // the initial kernel thread will later become the idle thread
    idle_thread = kthread_self();
    threadlist = idle_thread;

    // the idle thread is always ready to run
    memset(idle_thread, 0, sizeof(struct thread));
    idle_thread->object.type = OBJECT_THREAD;
    idle_thread->priority = PRIORITY_SYSIDLE;
    idle_thread->state = THREAD_STATE_RUNNING;
    idle_thread->next = idle_thread;
    idle_thread->prev = idle_thread;
    strcpy(idle_thread->name, "idle");
    thread_ready_summary = (1 << PRIORITY_SYSIDLE);

    // Initialize system task queue
    init_task_queue(&sys_task_queue, PRIORITY_NORMAL /*PRIORITY_SYSTEM*/, INFINITE, "systask");

    // Register /proc/threads and /proc/dpcs
    register_proc_inode("threads", threads_proc, NULL);
    register_proc_inode("dpcs", dpcs_proc, NULL);
}


void ksched_destroy()
{
    ksched_suspend_all_threads();
    threadlist = NULL;
}


__asm__
(
    ".global ___kthread_self;"
    "___kthread_self: "
    "mov eax, esp;"
    "and eax, " TO_STRING(TCBMASK) ";"
    "ret;"
);
