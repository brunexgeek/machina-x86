//
// pit.c
//
// Programmable Interval Timer functions (PIT i8253)
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
#include <os/pit.h>
#include <os/cpu.h>
#include <os/procfs.h>
#include <os/trap.h>
#include <os/pic.h>


#define TMR_CTRL        0x43    // I/O for control
#define TMR_CNT0        0x40    // I/O for counter 0
#define TMR_CNT1        0x41    // I/O for counter 1
#define TMR_CNT2        0x42    // I/O for counter 2

#define TMR_SC0         0       // Select channel 0
#define TMR_SC1         0x40    // Select channel 1
#define TMR_SC2         0x80    // Select channel 2

#define TMR_LOW         0x10    // RW low byte only
#define TMR_HIGH        0x20    // RW high byte only
#define TMR_BOTH        0x30    // RW both bytes

#define TMR_MD0         0       // Mode 0
#define TMR_MD1         0x2     // Mode 1
#define TMR_MD2         0x4     // Mode 2
#define TMR_MD3         0x6     // Mode 3
#define TMR_MD4         0x8     // Mode 4
#define TMR_MD5         0xA     // Mode 5

#define TMR_BCD         1       // BCD mode

#define TMR_LATCH       0       // Latch command

#define TMR_READ        0xF0    // Read command
#define TMR_CNT         0x20    // CNT bit  (Active low, subtract it)
#define TMR_STAT        0x10    // Status bit  (Active low, subtract it)

#define TMR_CH0         0x00    // Channel 0 bit
#define TMR_CH1         0x40    // Channel 1 bit
#define TMR_CH2         0x80    // Channel 2 bit

#define PIT_CLOCK       1193180
#define TIMER_FREQ      100

#define USECS_PER_TICK  (1000000 / TIMER_FREQ)
#define MSECS_PER_TICK  (1000 / TIMER_FREQ)

#define LOADTAB_SIZE        TIMER_FREQ

#define LOADTYPE_IDLE       0
#define LOADTYPE_USER       1
#define LOADTYPE_KERNEL     2
#define LOADTYPE_DPC        3

#define PIT_CMOS_SECOND       0x00
#define PIT_CMOS_MINUTE       0x02
#define PIT_CMOS_HOUR         0x04
#define PIT_CMOS_WEEKDAY      0x06
#define PIT_CMOS_DAY          0x07
#define PIT_CMOS_MONTH        0x08
#define PIT_CMOS_YEAR         0x09
#define PIT_CMOS_CENTURY      0x32


volatile unsigned int global_ticks = 0;
volatile unsigned int global_clocks = 0;
struct timeval global_time = { 0, 0 };

static time_t upsince;
static unsigned long cycles_per_tick;
static unsigned long loops_per_tick;

static unsigned char loadtab[LOADTAB_SIZE];
static unsigned char *loadptr;
static unsigned char *loadend;

static struct interrupt timerintr;
static struct dpc timerdpc;


void timer_dpc(void *arg)
{
    run_timer_list();
}


int timer_handler(struct context *ctxt, void *arg)
{
    struct thread *t;

    // update timer clock
    global_clocks += CLOCKS_PER_TICK;
    // update tick counter
    global_ticks++;

    // update system clock
    global_time.tv_usec += USECS_PER_TICK;
    while (global_time.tv_usec >= 1000000)
    {
        global_time.tv_sec++;
        global_time.tv_usec -= 1000000;
    }

    // update thread times and load average
    t = kthread_self();
    if (in_dpc)
    {
        dpc_time++;
        *loadptr = LOADTYPE_DPC;
    }
    else
    {
        if (USERSPACE(ctxt->eip))
        {
            t->utime++;
            *loadptr = LOADTYPE_USER;
        }
        else
        {
            t->stime++;
            if (t->base_priority == PRIORITY_SYSIDLE)
            {
                *loadptr = LOADTYPE_IDLE;
            }
            else
            {
                *loadptr = LOADTYPE_KERNEL;
            }
        }
    }

    if (++loadptr == loadend) loadptr = loadtab;

    // adjust thread quantum
    t->quantum -= QUANTUM_UNITS_PER_TICK;
    if (t->quantum <= 0) preempt = 1;

    // queue timer DPC
    kdpc_queue_irq(&timerdpc, timer_dpc, "timer_dpc", NULL);

    kpic_eoi(IRQ_TMR);
    return 0;
}


unsigned char kpit_read_cmos(int reg)
{
    unsigned char val;

    outp(0x70, reg);
    val = inp(0x71) & 0xFF;

    return val;
}


void kpit_write_cmos(int reg, unsigned char val)
{
    outp(0x70, reg);
    outp(0x71, val);
}


static int read_bcd_cmos_reg(int reg)
{
    int val;

    val = kpit_read_cmos(reg);
    return (val >> 4) * 10 + (val & 0x0F);
}


static void write_bcd_cmos_reg(int reg, unsigned char val)
{
    kpit_write_cmos(reg, (unsigned char) (((val / 10) << 4) + (val % 10)));
}


static void get_cmos_time(struct tm *tm)
{
    tm->tm_year = read_bcd_cmos_reg(0x09) + (read_bcd_cmos_reg(0x32) * 100) - 1900;
    tm->tm_mon = read_bcd_cmos_reg(0x08) - 1;
    tm->tm_mday = read_bcd_cmos_reg(0x07);
    tm->tm_hour = read_bcd_cmos_reg(0x04);
    tm->tm_min = read_bcd_cmos_reg(0x02);
    tm->tm_sec = read_bcd_cmos_reg(0x00);

    tm->tm_wday = 0;
    tm->tm_yday = 0;
    tm->tm_isdst = 0;
}


static void set_cmos_time(struct tm *tm)
{
    write_bcd_cmos_reg(PIT_CMOS_YEAR,    (unsigned char) (tm->tm_year % 100));
    write_bcd_cmos_reg(PIT_CMOS_CENTURY, (unsigned char) ((tm->tm_year + 1900) / 100));
    write_bcd_cmos_reg(PIT_CMOS_MONTH,   (unsigned char) (tm->tm_mon + 1));
    write_bcd_cmos_reg(PIT_CMOS_DAY,     (unsigned char) (tm->tm_mday));
    write_bcd_cmos_reg(PIT_CMOS_HOUR,    (unsigned char) (tm->tm_hour));
    write_bcd_cmos_reg(PIT_CMOS_MINUTE,  (unsigned char) (tm->tm_min));
    write_bcd_cmos_reg(PIT_CMOS_SECOND,  (unsigned char) (tm->tm_sec));
}


static void tsc_delay(unsigned long cycles)
{
    long end, now;

    end = (unsigned long) kmach_rdtsc() + cycles;
    do {
        __asm__("nop");
        now = (unsigned long) kmach_rdtsc();
    } while (end - now > 0);
}


static void timed_delay(unsigned long loops)
{
    __asm__
    (
        "time_delay_loop:"
        "dec %0;"
        "jns time_delay_loop;"
        :
        : "r" (loops)
    );
}


void kpit_calibrate_delay()
{
    static int cpu_speeds[] =
    {
        16, 20, 25, 33, 40, 50, 60, 66, 75, 80, 90,
        100, 110, 120, 133, 150, 166, 180, 188,
        200, 233, 250, 266,
        300, 333, 350, 366,
        400, 433, 450, 466,
        500, 533, 550, 566,
        600, 633, 650, 667,
        700, 733, 750, 766,
        800, 833, 850, 866,
        900, 933, 950, 966,
        1000, 1130, 1200, 1260
    };

    unsigned long mhz;
    unsigned long t, bit;
    int precision;

    // check support for TSC (should be supported by Pentium I or later)
    if (global_cpu.features & CPU_FEATURE_TSC)
    {

        unsigned long start;
        unsigned long end;

        t = global_ticks;
        while (t == global_ticks);
        start = (unsigned long) kmach_rdtsc();

        t = global_ticks;
        while (t == global_ticks);
        end = (unsigned long) kmach_rdtsc();

        cycles_per_tick = end - start;
    }
    else
    {
        kprintf("panic: CPU too old! This OS needs a CPU with support for RDTSC instruction.");
    }

    mhz = cycles_per_tick * TIMER_FREQ / 1000000;

    if (mhz > 1275)
    {
        mhz = ((mhz + 25) / 50) * 50;
    }
    else
    if (mhz > 14)
    {
        int *speed = cpu_speeds;
        int i;
        unsigned long bestmhz = 0;

        for (i = 0; i < sizeof(cpu_speeds) / sizeof(int); i++)
        {
            int curdiff = mhz - bestmhz;
            int newdiff = mhz - *speed;

            if (curdiff < 0) curdiff = -curdiff;
            if (newdiff < 0) newdiff = -newdiff;

            if (newdiff < curdiff) bestmhz = *speed;
            speed++;
        }

        mhz = bestmhz;
    }

    kprintf(KERN_INFO "cpu: %d cycles/tick, %d MHz processor\n", cycles_per_tick, mhz);
    global_cpu.mhz = mhz;
}


static int uptime_proc(struct proc_file *pf, void *arg)
{
    int days, hours, mins, secs;
    int uptime = kpit_get_time() - upsince;

    days = uptime / (24 * 60 * 60);
    uptime -= days * (24 * 60 * 60);
    hours = uptime / (60 * 60);
    uptime -= hours * (60 * 60);
    mins = uptime / 60;
    secs = uptime % 60;

    pprintf(pf, "%d day%s %d hour%s %d minute%s %d second%s\n",
    days, days == 1 ? "" : "s",
    hours, hours == 1 ? "" : "s",
    mins, mins == 1 ? "" : "s",
    secs, secs == 1 ? "" : "s");

    return 0;
}


static int loadavg_proc(struct proc_file *pf, void *arg)
{
    int loadavg[4];
    unsigned char *p;

    memset(loadavg, 0, sizeof loadavg);
    p = loadtab;
    while (p < loadend) loadavg[*p++]++;

    pprintf(pf, "Load kernel: %d%% user: %d%% dpc: %d%% idle: %d%%\n",
        loadavg[LOADTYPE_KERNEL],
        loadavg[LOADTYPE_USER],
        loadavg[LOADTYPE_DPC],
        loadavg[LOADTYPE_IDLE]);

    return 0;
}


void kpit_init()
{
    struct tm tm;

    unsigned int cnt = PIT_CLOCK / TIMER_FREQ;
    outp(TMR_CTRL, TMR_CH0 + TMR_BOTH + TMR_MD3);
    outp(TMR_CNT0, (unsigned char) (cnt & 0xFF));
    outp(TMR_CNT0, (unsigned char) (cnt >> 8));

    loadptr = loadtab;
    loadend = loadtab + LOADTAB_SIZE;

    get_cmos_time(&tm);
    global_time.tv_sec = mktime(&tm);
    upsince = global_time.tv_sec;

    kdpc_create(&timerdpc);
    timerdpc.flags |= DPC_NORAND; // Timer tick is a bad source for randomness
    register_interrupt(&timerintr, INTR_TMR, timer_handler, NULL);
    kpic_enable_irq(IRQ_TMR);

    register_proc_inode("uptime", uptime_proc, NULL);
    register_proc_inode("loadavg", loadavg_proc, NULL);
}


void kpit_udelay(unsigned long us)
{
    if (global_cpu.features & CPU_FEATURE_TSC)
        tsc_delay(us * (cycles_per_tick / (1000000 / TIMER_FREQ)));
    else
        timed_delay(us * (loops_per_tick / (1000000 / TIMER_FREQ)));
}


time_t kpit_get_time()
{
    return global_time.tv_sec;
}


void kpit_set_time(struct timeval *tv)
{
    struct tm tm;

    upsince += (tv->tv_sec - global_time.tv_sec);
    global_time.tv_usec = tv->tv_usec;
    global_time.tv_sec = tv->tv_sec;
    gmtime_r(&tv->tv_sec, &tm);
    set_cmos_time(&tm);
}


int kpit_get_system_load(struct loadinfo *info)
{
    int loadavg[4];
    unsigned char *p;

    if (info == NULL) return -EINVAL;

    memset(loadavg, 0, sizeof loadavg);
    p = loadtab;
    while (p < loadend) loadavg[*p++]++;

    info->uptime = kpit_get_time() - upsince;
    info->load_user = loadavg[LOADTYPE_USER];
    info->load_system = loadavg[LOADTYPE_KERNEL];
    info->load_intr = loadavg[LOADTYPE_DPC];
    info->load_idle = loadavg[LOADTYPE_IDLE];

    return 0;
}
