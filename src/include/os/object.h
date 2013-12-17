//
// object.h
//
// Object manager
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

#ifndef THREAD_H
#define THREAD_H

#define MAX_WAIT_OBJECTS  16

#define WAIT_ALL 0
#define WAIT_ANY 1

#define OBJECT_ANY       -1

#define OBJECT_THREAD     0
#define OBJECT_EVENT      1
#define OBJECT_TIMER      2
#define OBJECT_MUTEX      3
#define OBJECT_SEMAPHORE  4
#define OBJECT_FILE       5
#define OBJECT_SOCKET     6
#define OBJECT_IOMUX      7
#define OBJECT_FILEMAP    8

#define OBJECT_TYPES      9

#define THREAD_STATE_INITIALIZED 0
#define THREAD_STATE_READY       1
#define THREAD_STATE_RUNNING     2
#define THREAD_STATE_WAITING     3
#define THREAD_STATE_TERMINATED  4
#define THREAD_STATE_SUSPENDED   5
#define THREAD_STATE_TRANSITION  6

#define THREAD_WAIT_OBJECT       0
#define THREAD_WAIT_BUFFER       1
#define THREAD_WAIT_TASK         2
#define THREAD_WAIT_SOCKET       3
#define THREAD_WAIT_SLEEP        4
#define THREAD_WAIT_PIPE         5
#define THREAD_WAIT_DEVIO        6

#define THREAD_FPU_USED          1
#define THREAD_FPU_ENABLED       2
#define THREAD_ALERTABLE         4
#define THREAD_INTERRUPTED       8

#define ISIOOBJECT(o) ((o)->object.type == OBJECT_SOCKET || (o)->object.type == OBJECT_FILE)

#define THREAD_NAME_LEN          16

#include <os/timer.h>
#include <os/fpu.h>

struct thread;
struct waitblock;

typedef void *object_t;

struct object {
  unsigned short type;
  short signaled;

  unsigned short handle_count;
  unsigned short lock_count;

  struct waitblock *waitlist_head;
  struct waitblock *waitlist_tail;
};

struct waitblock {
  struct thread *thread;
  struct object *object;

  unsigned short waittype;
  short waitkey;

  struct waitblock *next;

  struct waitblock *next_wait;
  struct waitblock *prev_wait;
};

struct iomux;

struct ioobject {
  struct object object;

  struct iomux *iomux;
  int context;

  struct ioobject *next;
  struct ioobject *prev;

  unsigned short events_signaled;
  unsigned short events_monitored;
};

struct iomux {
  struct object object;
  int flags;

  struct ioobject *ready_head;
  struct ioobject *ready_tail;

  struct ioobject *waiting_head;
  struct ioobject *waiting_tail;
};

struct event {
  struct object object;
  int manual_reset;
};

struct sem {
  struct object object;
  unsigned int count;
};

struct mutex {
  struct object object;
  struct thread *owner;
  int recursion;
};

struct waitable_timer {
  struct object object;
  struct timer timer;
};

struct thread {
  struct object object;

  int state;
  int quantum;
  int wait_reason;
  int flags;
  int priority;
  int base_priority;
  tid_t id;
  handle_t hndl;
  struct tib *tib;
  int suspend_count;
  void *entrypoint;
  int exitcode;
  char name[THREAD_NAME_LEN];

  uid_t ruid;
  uid_t rgid;
  uid_t euid;
  gid_t egid;
  int ngroups;
  gid_t groups[NGROUPS_MAX];
  char curdir[MAXPATH];

  sigset_t blocked_signals;
  sigset_t pending_signals;
  struct timer alarm;

  unsigned long utime;
  unsigned long stime;
  unsigned long context_switches;
  unsigned long preempts;

  struct thread *next;
  struct thread *prev;

  struct thread *next_ready;
  struct thread *prev_ready;

  struct waitblock *waitlist;
  int waitkey;

  struct thread *next_waiter;

  struct context *ctxt;

  struct fpu fpustate;
};

struct filemap {
  struct object object;

  handle_t file;
  __int64 offset;

  char *addr;
  unsigned long size;
  unsigned long protect;
  int pages;

  handle_t self;
};

#ifdef KERNEL

#define HPROTECT  1
#define HEND      0x7FFFFFFF

#define HOBJ(x)  ((struct object *) ((x) & ~HPROTECT))
#define HPROT(x) ((x) & HPROTECT)
#define HUSED(x) ((x) & OSBASE)

extern handle_t *htab;
extern int htabsize;

// object.c

void init_object(struct object *o, int type);
int close_object(struct object *o);
int destroy_object(struct object *o);

int thread_ready_to_run(struct thread *t);
void release_thread(struct thread *t);
void release_waiters(struct object *o, int waitkey);

void init_thread(struct thread *t, int priority);
void exit_thread(struct thread *t);

KERNELAPI void init_event(struct event *e, int manual_reset, int initial_state);
KERNELAPI void pulse_event(struct event *e);
KERNELAPI void set_event(struct event *e);
KERNELAPI void reset_event(struct event *e);

KERNELAPI void init_sem(struct sem *s, unsigned int initial_count);
KERNELAPI unsigned int release_sem(struct sem *s, unsigned int count);
KERNELAPI unsigned int set_sem(struct sem *s, unsigned int count);

KERNELAPI void init_mutex(struct mutex *m, int owned);
KERNELAPI int release_mutex(struct mutex *m);

int unlock_filemap(struct filemap *m);

KERNELAPI void init_waitable_timer(struct waitable_timer *t, unsigned int expires);
KERNELAPI void modify_waitable_timer(struct waitable_timer *t, unsigned int expires);
KERNELAPI void cancel_waitable_timer(struct waitable_timer *t);

KERNELAPI int wait_for_object(object_t hobj, unsigned int timeout);
KERNELAPI int wait_for_one_object(object_t hobj, unsigned int timeout, int alertable);
KERNELAPI int wait_for_all_objects(struct object **objs, int count, unsigned int timeout, int alertable);
KERNELAPI int wait_for_any_object(struct object **objs, int count, unsigned int timeout, int alertable);

// iomux.c

KERNELAPI void init_iomux(struct iomux *iomux, int flags);
int close_iomux(struct iomux *iomux);
KERNELAPI int queue_ioobject(struct iomux *iomux, object_t hobj, int events, int context);
KERNELAPI void init_ioobject(struct ioobject *iob, int type);
KERNELAPI void detach_ioobject(struct ioobject *iob);
KERNELAPI void set_io_event(struct ioobject *iob, int events);
KERNELAPI void clear_io_event(struct ioobject *iob, int events);
int dequeue_event_from_iomux(struct iomux *iomux);
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
int poll(struct pollfd fds[], unsigned int nfds, int timeout);

// hndl.c

void init_handles();

KERNELAPI struct object *hlookup(handle_t h);
KERNELAPI handle_t halloc(struct object *o);
KERNELAPI int hassign(struct object *o, handle_t h);
KERNELAPI int hfree(handle_t h);
KERNELAPI int hprotect(handle_t h);
KERNELAPI int hunprotect(handle_t h);

KERNELAPI struct object *olock(handle_t h, int type);
KERNELAPI int orel(object_t hobj);

#endif

#endif
