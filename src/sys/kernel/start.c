//
// start.c
//
// Kernel initialization
//
// Copyright (C) 2013-2014 Bruno Ribeiro.
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

#include <os/krnl.h>
#include <os/cpu.h>
#include <os/vmm.h>
#include <os/pdir.h>
#include <os/pframe.h>
#include <os/kmem.h>
#include <os/mach.h>
#include <os/dev.h>
#include <os/kbd.h>
#include <os/dfs.h>
#include <os/devfs.h>
#include <os/pic.h>
#include <os/trap.h>
#include <os/pit.h>
#include <net/stats.h>
#include <net/arp.h>
#include <net/tcp.h>
#include <net/ip.h>
#include <net/udp.h>
#include <net/raw.h>
#include <net/dhcp.h>
#include <net/socket.h>
#include <net/net.h>
#include <os/sched.h>
#include <os/module.h>


#ifdef BSD
char *copyright =
"Redistribution and use in source and binary forms, with or without\n"
"modification, are permitted provided that the following conditions\n"
"are met:\n"
"\n"
"1. Redistributions of source code must retain the above copyright\n"
"   notice, this list of conditions and the following disclaimer.\n"
"2. Redistributions in binary form must reproduce the above copyright\n"
"   notice, this list of conditions and the following disclaimer in the\n"
"   documentation and/or other materials provided with the distribution.\n"
"3. Neither the name of the project nor the names of its contributors\n"
"   may be used to endorse or promote products derived from this software\n"
"   without specific prior written permission.\n"
"\n"
"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND\n"
"ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n"
"WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\n"
"DISCLAIMED IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR\n"
"ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES\n"
"(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\n"
"LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON\n"
"ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n"
"(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n"
"SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n";
#endif


#define ONPANIC_HALT      EXITOS_HALT
#define ONPANIC_REBOOT    EXITOS_REBOOT
#define ONPANIC_DEBUG     EXITOS_DEBUG
#define ONPANIC_POWEROFF  EXITOS_POWEROFF



// syscall.c

void init_syscall();

// cpu.c

//int cpu_proc(struct proc_file *pf, void *arg);

// smbfs.c

void init_smbfs();

// pipefs.c

void init_pipefs();
int pipe(struct file **readpipe, struct file **writepipe);

// cdfs.c

void init_cdfs();

// cons.c

extern int serial_console;
void init_console();
int console(struct unit *unit, char *opts);
void console_print(char *buffer, int size);

// serial.c

void init_serial();

// ramdisk.c

int create_initrd();

// hd.c

void init_hd();

// fd.c

void init_fd();

// virtioblk.c

void init_vblk();

// apm.c

void apm_power_off();
extern int apm_enabled;



struct thread *mainthread;
struct section *krnlcfg;
int onpanic = ONPANIC_HALT;
struct peb *peb;
char krnlopts[KRNLOPTS_LEN];

void main(void *arg);

int license()
{
    return LICENSE;
}

void stop(int mode)
{
    ksched_destroy();

    umount_all();
    tcp_shutdown();
    msleep(200);

    switch (mode)
    {
        case EXITOS_HALT:
            kprintf("kernel: system stopped\n");
            break;

        case EXITOS_POWEROFF:
            kprintf("kernel: power down...\n");
            kmach_poweroff();
            break;

        case EXITOS_REBOOT:
            kprintf("kernel: rebooting...\n");
            kmach_reboot();
            break;

        case EXITOS_DEBUG:
            dbg_break();
            break;
    }

    while (1)
    {
        kmach_cli();
        kmach_halt();
    }
}

void panic(char *msg)
{
    static int inpanic = 0;

    if (inpanic)
    {
        kprintf(KERN_EMERG "double panic: %s, halting\n", msg);
        kmach_cli();
        kmach_halt();
    }

    inpanic = 1;
    kprintf(KERN_EMERG "panic: %s\n", msg);

    if (onpanic == ONPANIC_DEBUG)
    {

        //if (debugging) dbg_output(msg);
        dbg_break();
    }
    else
    {
        stop(onpanic);
    }
}


static int load_kernel_config()
{
    struct file *f;
    int size;
    int rc;
    struct stat64 buffer;
    char config[MAXPATH];
    char *props;

    get_option(krnlopts, "config", config, sizeof(config), "/boot/krnl.ini");

    rc = open(config, O_RDONLY | O_BINARY, 0, &f);
    if (rc < 0) return rc;

    fstat(f, &buffer);
    size = (int) buffer.st_size;

    props = (char *) kmalloc(size + 1);
    if (!props)
    {
        close(f);
        destroy(f);
        return -ENOMEM;
        }

    rc = read(f, props, size);
    if (rc < 0)
    {
        free(props);
        close(f);
        destroy(f);
        return rc;
    }

    close(f);
    destroy(f);

    props[size] = 0;

    krnlcfg = parse_properties(props);
    free(props);

    return 0;
}

static void init_filesystem()
{
    int rc;
    char bootdev[8];
    char rootdev[128];
    char rootfs[32];
    char *rootfsopts;
    char fsoptbuf[128];

    // Initialize built-in file systems
    init_vfs();
    init_dfs();
    init_devfs();
    init_procfs();
    init_pipefs();
    init_smbfs();
    init_cdfs();

    // Determine boot device
    if ((syspage->ldrparams.bootdrv & 0xF0) == 0xF0)
    {
        create_initrd();
        strcpy(bootdev, "initrd");
    }
    else
    if (syspage->ldrparams.bootdrv & 0x80)
    {
        if (syspage->ldrparams.bootpart == -1)
        {
            sprintf(bootdev, "hd%c", '0' + (syspage->ldrparams.bootdrv & 0x7F));
        }
        else
        {
            sprintf(bootdev, "hd%c%c", '0' + (syspage->ldrparams.bootdrv & 0x7F), 'a' + syspage->ldrparams.bootpart);
        }
    }
    else
    {
        sprintf(bootdev, "fd%c", '0' + (syspage->ldrparams.bootdrv & 0x7F));
    }

    // If default boot device is not found try a virtual device.
    if (kdev_get_number(bootdev) == NODEV && kdev_get_number("vd0") != NODEV) strcpy(bootdev, "vd0");

    // Determine root file system
    get_option(krnlopts, "rootdev", rootdev, sizeof(rootdev), bootdev);
    get_option(krnlopts, "rootfs", rootfs, sizeof(rootfs), "dfs");
    rootfsopts = get_option(krnlopts, "rootopts", fsoptbuf, sizeof(fsoptbuf), NULL);

    kprintf(KERN_INFO "mount: root on %s\n", rootdev);

    // Mount file systems
    rc = mount(rootfs, "/", rootdev, rootfsopts, NULL);
    if (rc < 0) panic("error mounting root filesystem");

    rc = mount("devfs", "/dev", NULL, NULL, NULL);
    if (rc < 0) panic("error mounting dev filesystem");

    rc = mount("procfs", "/proc", NULL, NULL, NULL);
    if (rc < 0) panic("error mounting proc filesystem");
}

/*static int version_proc(struct proc_file *pf, void *arg) {
  hmodule_t krnl = (hmodule_t) OSBASE;
  struct verinfo *ver;
  time_t ostimestamp;
  char osname[32];
  struct tm tm;

  ostimestamp = get_image_header(krnl)->header.timestamp;
  gmtime_r(&ostimestamp, &tm);

  ver = get_version_info(krnl);
  if (ver)  {
    if (get_version_value(krnl, "ProductName", osname, sizeof(osname)) < 0) strcpy(osname, "Sanos");
    pprintf(pf, "%s version %d.%d.%d.%d", osname, ver->file_major_version, ver->file_minor_version, ver->file_release_number, ver->file_build_number);

    if (ver->file_flags & VER_FLAG_PRERELEASE) pprintf(pf, " prerelease");
    if (ver->file_flags & VER_FLAG_PATCHED) pprintf(pf, " patch");
    if (ver->file_flags & VER_FLAG_PRIVATEBUILD) pprintf(pf, " private");
    if (ver->file_flags & VER_FLAG_DEBUG) pprintf(pf, " debug");
  } else {
    pprintf(pf, "%s version %d.%d.%d.%d", OS_NAME, OS_VERSION_MAJOR, OS_VERSION_MINOR, OS_RELEASE, OS_BUILD);
  }

  pprintf(pf, " (Built %04d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
#ifdef _MSC_VER
  pprintf(pf, " with MSVC %d.%02d on WIN32", _MSC_VER / 100, _MSC_VER % 100);
#endif
#ifdef _TCC_VER
  pprintf(pf, " with TCC %s on %s", _TCC_VER, _TCC_PLATFORM);
#endif
  pprintf(pf, ")\n");

  return 0;
}*/


static int copyright_proc(struct proc_file *pf, void *arg)
{
    hmodule_t krnl = (hmodule_t) OSBASE;
    char copy[128];
    char legal[128];

    if (get_version_value(krnl, "LegalCopyright", copy, sizeof(copy)) < 0) strcpy(copy, OS_COPYRIGHT);
    if (get_version_value(krnl, "LegalTrademarks", legal, sizeof(legal)) < 0) strcpy(legal, OS_LEGAL);

    //version_proc(pf, arg);
    pprintf(pf, "%s %s\n\n", copy, legal);
    proc_write(pf, copyright, strlen(copyright));
    return 0;
}


void dummy_func( void *arg)
{
    while (1)
    {
        struct thread *t = kthread_self();
        kprintf("Thread %s\n", t->name);
        kthread_wait(1);
    }
}


__attribute__((section("entryp"))) void __attribute__((stdcall)) start(
    void *hmod,
    char *opts,
    int reserved2 )
{
    // copy kernel options
    strcpy(krnlopts, opts);
    //if (get_option(opts, "silent", NULL, 0, NULL) != NULL) kprint_enabled = 0;
    //if (get_option(opts, "serialconsole", NULL, 0, NULL) != NULL) serial_console = 1;
    serial_console = 1;

    // Initialize console
    init_console();

    kprintf(KERN_INFO "Welcome to Machina operating system!\n");
    // Display banner
    if (*krnlopts) kprintf(KERN_INFO "options: %s\n", krnlopts);

    // initialize machine
    init_mach();
    // initialize CPU
    kcpu_initialize();
    // initialize page frame database
    kpframe_initialize();
    // initialize page directory
    kpage_initialize();
    // initialize kernel heap
    kmem_initialize();
    // initialize kernel allocator
    init_malloc();
    // initialize virtual memory manager
    kvmm_initialize();
    // flush tlb
    kmach_flushtlb();

    // Register memory management procs
    register_proc_inode("memmap", proc_memmap, NULL);
    register_proc_inode("memusage", proc_memusage, NULL);
    register_proc_inode("memstat", proc_memstat, NULL);
    register_proc_inode("physmem", proc_physmem, NULL);
    register_proc_inode("pdir", proc_pdir, NULL);
    register_proc_inode("virtmem", proc_virtmem, NULL);
    register_proc_inode("kmem", proc_kmem, NULL);
    register_proc_inode("kmodmem", kmodmem_proc, NULL);
    register_proc_inode("kheap", kheapstat_proc, NULL);
    register_proc_inode("vmem", vmem_proc, NULL);
    register_proc_inode("cpu", proc_cpuinfo, NULL);

    // Initialize interrupts, floating-point support, and real-time clock
    kpic_init();
    ktrap_init();
    init_fpu();
    kpit_init();

    // Initialize timers, scheduler, and handle manager
    init_timers();
    ksched_init();
    init_handles();
    init_syscall();

    // enable interrupts and calibrate delay
    kmach_sti();
    kpit_calibrate_delay();

    // Start main task and dispatch to idle task
    mainthread = kthread_create_kland(main, 0, PRIORITY_NORMAL, "init");
    kthread_create_kland(dummy_func, 0, PRIORITY_NORMAL, "dummy");
    kthread_create_kland(dummy_func, 0, PRIORITY_NORMAL, "dumbass");
    ksched_idle();
}


void init_net()
{
    stats_init();
    netif_init();
    ether_init();
    pbuf_init();
    arp_init();
    ip_init();
    udp_init();
    raw_init();
    dhcp_init();
    tcp_init();
    socket_init();
    loopif_init();

    register_ether_netifs();
}


void main_readFile( char *fileName )
{
    struct file *tmp;
    char buffer[16];
    int ret;
    ret = open(fileName, 0, S_IREAD, &tmp);
    if (ret == 0)
    {
        kprintf("\n\n##########################\n## %s\n##########################\n", fileName);
        int count = 1;
        while (count != 0)
        {
            count = read(tmp, buffer, 15);
            buffer[count] = 0;
            kprintf("%s", buffer);
        }
        close(tmp);
    }
}


void main_userThreadProc( int a, int b, int c)
{
    while (1)
    {
        kprintf("user thread %s is waiting\n", kthread_self()->name);
        kthread_wait(0);
    }
}


void main(void *arg)
{
    unsigned long *stacktop;
    struct thread *t = kthread_self();

    void *imgbase;
    void *entrypoint;
    unsigned long stack_reserve;
    unsigned long stack_commit;
    struct image_header *imghdr;
    struct verinfo *ver;
    int rc;
    char *str;
    struct file *cons;
    char *sconsole;

    // Allocate and initialize PEB
    peb = vmalloc((void *) PEB_ADDRESS, PAGESIZE, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE, PFT_PEB, NULL);
    if (!peb) panic("unable to allocate PEB");
    memset(peb, 0, PAGESIZE);
    peb->fast_syscalls_supported = (cpuInfo.features & CPU_FEATURE_SEP) != 0;

    // Enumerate root host buses and units
    enum_host_bus();

    // Initialize boot device drivers
    init_hd();
    init_fd();
    init_vblk();

    // Initialize file systems
    init_filesystem();

    // Load kernel configuration
    load_kernel_config();

    // Determine kernel panic action
    str = get_property(krnlcfg, "kernel", "onpanic", "halt");
    if (strcmp(str, "halt") == 0)
    {
        onpanic = ONPANIC_HALT;
    }
    else
    if (strcmp(str, "reboot") == 0)
    {
        onpanic = ONPANIC_REBOOT;
    }
    else
    if (strcmp(str, "debug") == 0)
    {
        onpanic = ONPANIC_DEBUG;
    }
    else
    if (strcmp(str, "poweroff") == 0)
    {
        onpanic = ONPANIC_POWEROFF;
    }

    // Set path separator
    pathsep = *get_property(krnlcfg, "kernel", "pathsep", "");
    if (pathsep != PS1 && pathsep != PS2) pathsep = PS1;
    t->curdir[0] = pathsep;
    t->curdir[1] = 0;
    peb->pathsep = pathsep;

    // Initialize module loader
main_readFile("/proc/memusage");
    kmodule_initialize();
main_readFile("/proc/memusage");
    console(NULL, NULL);

    // get Machina version information
    strcpy(peb->osname, OS_NAME);
    peb->osversion.file_major_version = OS_VERSION_MAJOR;
    peb->osversion.file_minor_version = OS_VERSION_MINOR;
    peb->osversion.file_release_number = OS_RELEASE;
    peb->osversion.file_build_number = OS_BUILD;

    // Install device drivers
    install_drivers();

    // Initialize network
    init_net();

    // Install /proc/version and /proc/copyright handler
    //register_proc_inode("version", version_proc, NULL);
    register_proc_inode("copyright", copyright_proc, NULL);

    // Allocate handles for stdin, stdout and stderr
    sconsole = get_property(krnlcfg, "kernel", "console", serial_console ? "/dev/com1" : "/dev/console");
    rc = open(sconsole, O_RDWR, S_IREAD | S_IWRITE, &cons);
    if (rc < 0) panic("no console");
    cons->flags |= F_TTY;
    if (halloc(&cons->iob.object) != 0) panic("unexpected stdin handle");
    if (halloc(&cons->iob.object) != 1) panic("unexpected stdout handle");
    if (halloc(&cons->iob.object) != 2) panic("unexpected stderr handle");

    main_readFile("/proc/kmods");
    /*main_readFile("/proc/units");
    main_readFile("/proc/memmap");
    main_readFile("/proc/memusage");
    main_readFile("/proc/memstat");
    main_readFile("/proc/physmem");
    //main_readFile("/proc/pdir");
    main_readFile("/proc/virtmem");
    main_readFile("/proc/kmem");
    main_readFile("/proc/kmodmem");
    main_readFile("/proc/kheap");
    main_readFile("/proc/vmem");
    main_readFile("/proc/cpu");
    main_readFile("/proc/netif");
    main_readFile("/proc/handles");*/

    // Load kernel32.so in user address space
    /*imgbase = kloader_load(get_property(krnlcfg, "kernel", "osapi", "/boot/kernel32.so"), 1);
    if (!imgbase) panic("unable to load kernel32.so");
    imghdr = get_image_header(imgbase);
    stack_reserve = 8 * 1024;//imghdr->optional.size_of_stack_reserve;
    stack_commit = 4 * 1024;//imghdr->optional.size_of_stack_commit;
    //entrypoint = get_entrypoint(imgbase);
    entrypoint = main_userThreadProc;

    // Initialize initial user thread
    if (init_user_thread(t, entrypoint) < 0)
        panic("unable to initialize initial user thread");
    if (allocate_user_stack(t, stack_reserve, stack_commit) < 0)
        panic("unable to allocate stack for initial user thread");
    t->hndl = halloc(&t->object);
    hprotect(t->hndl);
    kthread_mark_running();

    kprintf(KERN_INFO "mem: %dMB total, %dKB used, %dKB free, %dKB reserved\n",
    maxmem * PAGESIZE / (1024 * 1024),
    (totalmem - freemem) * PAGESIZE / 1024,
    freemem * PAGESIZE / 1024, (maxmem - totalmem) * PAGESIZE / 1024);

    // Place arguments to start routine on stack
    stacktop = (unsigned long *) t->tib->stacktop;
    *(--stacktop) = 0;
    *(--stacktop) = 0;
    *(--stacktop) = (unsigned long) imgbase;
    *(--stacktop) = 0;

    // jump into user mode
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
    );*/

    while (1)
    {
        kprintf("%s is waiting\n", kthread_self()->name);
        kthread_wait(0);
    }
}
