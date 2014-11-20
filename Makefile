#!/bin/make -f

TARGET_MACHINE := x86
NASM := build/tools/nasm
LDFLAGS := -m32 -mtune=i686
override CFLAGS := $(CFLAGS) -Wall -Werror=overflow -Werror-implicit-function-declaration -O0 -m32 -mtune=i686

help:
	@echo "   all"
	@echo "   clean"
	@echo "   $(KERNEL32_OUT_FILE) (kernel) "
	@echo "   $(MKDFS_OUT_FILE) "
	@echo "   $(OSLDRS_OUT_FILE) (osloader-stub) "
	@echo "   $(LIBC_OUT_FILE) (libc) "
	@echo "   $(NETBOOT_OUT_FILE) (netboot) "
	@echo "   $(CDEMBOOT_OUT_FILE) (cdemboot) "
	@echo "   $(LIBKERNEL_OUT_FILE) "
	@echo "   $(ISO_OUT_FILE) (iso) "
	@echo "   $(DISKBOOT_OUT_FILE) (diskboot) "
	@echo "   $(KRNLIMG32_OUT_FILE) (kernel-image) "
	@echo "   $(OSLDR_OUT_FILE) (osloader) "
	@echo "   $(OSLDRM_OUT_FILE) (osloader-main) "
	@echo "   $(NASM_OUT_FILE) (nasm) "

.PHONY: all clean kernel osloader-stub libc netboot cdemboot iso diskboot kernel-image osloader osloader-main nasm

#
# Machina Kernel for x86 
#
KERNEL32_CFLAGS =  -g -O0 -I src/include -D KERNEL -D KRNL_LIB -nostdlib -masm=intel $(CFLAGS)
KERNEL32_LDFLAGS =  -nostdlib -Wl,-T,src/arch/x86/sys/kernel/kernel.lds $(LDFLAGS)
KERNEL32_NFLAGS = $(NFLAGS)
KERNEL32_OUT_DIR = build/machina/obj/kernel
KERNEL32_OUT_FILE = $(KERNEL32_OUT_DIR)/kernel32.elf
KERNEL32_SRC_DIR = src
KERNEL32_SRC_FILES = \
	sys/kernel/kdebug.c \
	sys/kernel/buf.c \
	sys/kernel/cpu.c \
	sys/kernel/dbg.c \
	sys/kernel/dev.c \
	sys/kernel/fpu.c \
	sys/kernel/hndl.c \
	sys/kernel/iomux.c \
	sys/kernel/rmap.c \
	sys/kernel/iovec.c \
	sys/kernel/kmalloc.c \
	sys/kernel/kmem.c \
	sys/kernel/loader.c \
	sys/kernel/mach.c \
	sys/kernel/object.c \
	sys/kernel/pci.c \
	sys/kernel/pdir.c \
	sys/kernel/pframe.c \
	sys/kernel/pic.c \
	sys/kernel/pit.c \
	sys/kernel/pnpbios.c \
	sys/kernel/queue.c \
	sys/kernel/sched.c \
	arch/x86/sys/kernel/trap.c \
	arch/x86/sys/kernel/trap.s \
	arch/x86/sys/kernel/sched.c \
	arch/x86/sys/kernel/sched.s \
	arch/x86/sys/kernel/mach.s \
	sys/kernel/start.c \
	sys/kernel/syscall.c \
	sys/kernel/timer.c \
	sys/kernel/trap.c \
	sys/kernel/user.c \
	sys/kernel/vfs.c \
	sys/kernel/virtio.c \
	sys/kernel/vmi.c \
	sys/kernel/vmm.c \
	sys/dev/cons.c \
	sys/dev/fd.c \
	sys/dev/hd.c \
	sys/dev/kbd.c \
	sys/dev/klog.c \
	sys/dev/null.c \
	sys/dev/nvram.c \
	sys/dev/ramdisk.c \
	sys/dev/rnd.c \
	sys/dev/serial.c \
	sys/dev/smbios.c \
	sys/dev/video.c \
	sys/dev/virtioblk.c \
	sys/dev/virtiocon.c \
	sys/net/arp.c \
	sys/net/dhcp.c \
	sys/net/ether.c \
	sys/net/icmp.c \
	sys/net/inet.c \
	sys/net/ipaddr.c \
	sys/net/ip.c \
	sys/net/loopif.c \
	sys/net/netif.c \
	sys/net/pbuf.c \
	sys/net/raw.c \
	sys/net/rawsock.c \
	sys/net/socket.c \
	sys/net/stats.c \
	sys/net/tcp.c \
	sys/net/tcp_input.c \
	sys/net/tcp_output.c \
	sys/net/tcpsock.c \
	sys/net/udp.c \
	sys/net/udpsock.c \
	sys/fs/cdfs/cdfs.c \
	sys/fs/devfs/devfs.c \
	sys/fs/dfs/dfs.c \
	sys/fs/dfs/dir.c \
	sys/fs/dfs/file.c \
	sys/fs/dfs/group.c \
	sys/fs/dfs/inode.c \
	sys/fs/dfs/super.c \
	sys/fs/pipefs/pipefs.c \
	sys/fs/procfs/procfs.c \
	sys/fs/smbfs/smbcache.c \
	sys/fs/smbfs/smbfs.c \
	sys/fs/smbfs/smbproto.c \
	sys/fs/smbfs/smbutil.c \
	lib/libc/bitops.c \
	lib/libc/ctype.c \
	lib/libc/inifile.c \
	lib/libc/moddb.c \
	lib/libc/opts.c \
	lib/libc/string.c \
	lib/libc/strtol.c \
	lib/libc/tcccrt.c \
	lib/libc/time.c \
	lib/libc/verinfo.c \
	lib/libc/vsprintf.c
KERNEL32_OBJ_DIR = build/machina/obj/kernel
KERNEL32_OBJ_FILES = $(patsubst %,$(KERNEL32_OBJ_DIR)/%.o ,$(KERNEL32_SRC_FILES))

$(KERNEL32_OBJ_FILES): | KERNEL32_OBJ_MKDIR

KERNEL32_OBJ_MKDIR:
	@mkdir -p build/machina/obj/kernel
	@mkdir -p build/machina/obj/kernel/sys/fs/smbfs
	@mkdir -p build/machina/obj/kernel/sys/fs/pipefs
	@mkdir -p build/machina/obj/kernel/lib/libc
	@mkdir -p build/machina/obj/kernel/sys/fs/dfs
	@mkdir -p build/machina/obj/kernel/sys/net
	@mkdir -p build/machina/obj/kernel/sys/kernel
	@mkdir -p build/machina/obj/kernel/sys/dev
	@mkdir -p build/machina/obj/kernel/arch/x86/sys/kernel
	@mkdir -p build/machina/obj/kernel/sys/fs/procfs
	@mkdir -p build/machina/obj/kernel/sys/fs/cdfs
	@mkdir -p build/machina/obj/kernel/sys/fs/devfs

$(KERNEL32_OBJ_DIR)/%.c.o: $(KERNEL32_SRC_DIR)/%.c
	@echo -e '\x1b[34;1mCompiling $< \x1b[0m'
	$(CC) $(KERNEL32_CFLAGS) -DTARGET_MACHINE=$(TARGET_MACHINE) -c $< -o $@

$(KERNEL32_OBJ_DIR)/%.s.o: $(KERNEL32_SRC_DIR)/%.s
	@echo -e '\x1b[34;1mCompiling $<\x1b[0m'
	$(CC) -x assembler-with-cpp $(KERNEL32_CFLAGS) -c $< -o $@

KERNEL32_CLEAN :
	@rm -f $(KERNEL32_OBJ_FILES)

$(KERNEL32_OUT_FILE) kernel :  $(KERNEL32_OBJ_FILES)
	@echo -e '\x1b[34;1mBuilding Machina Kernel for x86\x1b[0m'
	@mkdir -p $(KERNEL32_OUT_DIR)
	$(CC) -DTARGET_MACHINE=$(TARGET_MACHINE) $(KERNEL32_LDFLAGS) $(KERNEL32_OBJ_FILES) -o $(KERNEL32_OUT_FILE)


#
# MKDFS Tool for GNU/Linux 
#
MKDFS_CFLAGS = $(CFLAGS)
MKDFS_LDFLAGS = $(LDFLAGS)
MKDFS_NFLAGS = $(NFLAGS)
MKDFS_OUT_DIR = build/tools
MKDFS_OUT_FILE = $(MKDFS_OUT_DIR)/mkdfs
MKDFS_SRC_DIR = utils/dfs
MKDFS_SRC_FILES = \
	blockdev.c \
	vmdk.c \
	bitops.c \
	buf.c \
	dfs.c \
	dir.c \
	file.c \
	group.c \
	inode.c \
	mkdfs.c \
	super.c \
	vfs.c
MKDFS_OBJ_DIR = build/linux/obj/utils/dfs
MKDFS_OBJ_FILES = $(patsubst %,$(MKDFS_OBJ_DIR)/%.o ,$(MKDFS_SRC_FILES))

$(MKDFS_OBJ_FILES): | MKDFS_OBJ_MKDIR

MKDFS_OBJ_MKDIR:
	@mkdir -p build/linux/obj/utils/dfs

$(MKDFS_OBJ_DIR)/%.c.o: $(MKDFS_SRC_DIR)/%.c
	@echo -e '\x1b[34;1mCompiling $< \x1b[0m'
	$(CC) $(MKDFS_CFLAGS) -DTARGET_MACHINE=$(TARGET_MACHINE) -c $< -o $@

MKDFS_CLEAN :
	@rm -f $(MKDFS_OBJ_FILES)

$(MKDFS_OUT_FILE) :  $(MKDFS_OBJ_FILES)
	@echo -e '\x1b[34;1mBuilding MKDFS Tool for GNU/Linux\x1b[0m'
	@mkdir -p $(MKDFS_OUT_DIR)
	$(CC) -DTARGET_MACHINE=$(TARGET_MACHINE) $(MKDFS_LDFLAGS) $(MKDFS_OBJ_FILES) -o $(MKDFS_OUT_FILE)


#
# Machina OS Loader Stub 
#
OSLDRS_NFLAGS = $(NFLAGS) -f bin
OSLDRS_OUT_DIR = build/machina/obj/osloader
OSLDRS_OUT_FILE = $(OSLDRS_OUT_DIR)/osloader-stub.bin
OSLDRS_SRC_FILES = \
	src/arch/x86/sys/osloader/stub.asm

$(OSLDRS_OUT_FILE) osloader-stub : $(NASM_OUT_FILE) 
	@echo -e '\x1b[34;1mBuilding Machina OS Loader Stub\x1b[0m'
	@mkdir -p $(OSLDRS_OUT_DIR)
	$(NASM) $(OSLDRS_NFLAGS) $(OSLDRS_SRC_FILES) -o $(OSLDRS_OUT_FILE)


#
# Machina Standard C Library for x86 
#
LIBC_CFLAGS =  -I src/include -D OS_LIB $(CFLAGS)
LIBC_LDFLAGS =  -nostdlib $(LDFLAGS)
LIBC_NFLAGS = $(NFLAGS)
LIBC_OUT_DIR = build/install/usr/lib
LIBC_OUT_FILE = $(LIBC_OUT_DIR)/libc.a
LIBC_SRC_DIR = src
LIBC_SRC_FILES = \
	lib/libc/assert.c \
	lib/libc/tcccrt.c \
	lib/libc/bsearch.c \
	lib/libc/conio.c \
	lib/libc/crt0.c \
	lib/libc/ctype.c \
	lib/libc/dirent.c \
	lib/libc/fcvt.c \
	lib/libc/fnmatch.c \
	lib/libc/fork.c \
	lib/libc/getopt.c \
	lib/libc/glob.c \
	lib/libc/hash.c \
	lib/libc/inifile.c \
	lib/libc/input.c \
	lib/libc/math.c \
	lib/libc/mman.c \
	lib/libc/opts.c \
	lib/libc/output.c \
	lib/libc/qsort.c \
	lib/libc/random.c \
	lib/libc/readline.c \
	lib/libc/rmap.c \
	lib/libc/rtttl.c \
	lib/libc/sched.c \
	lib/libc/semaphore.c \
	lib/libc/stdio.c \
	lib/libc/shlib.c \
	lib/libc/scanf.c \
	lib/libc/printf.c \
	lib/libc/tmpfile.c \
	lib/libc/popen.c \
	lib/libc/stdlib.c \
	lib/libc/strftime.c \
	lib/libc/string.c \
	lib/libc/strtod.c \
	lib/libc/strtol.c \
	lib/libc/termios.c \
	lib/libc/time.c \
	lib/libc/xtoa.c \
	lib/libc/regex/regcomp.c \
	lib/libc/regex/regexec.c \
	lib/libc/regex/regerror.c \
	lib/libc/regex/regfree.c \
	lib/libc/pthread/barrier.c \
	lib/libc/pthread/condvar.c \
	lib/libc/pthread/mutex.c \
	lib/libc/pthread/pthread.c \
	lib/libc/pthread/rwlock.c \
	lib/libc/pthread/spinlock.c \
	lib/libc/setjmp.c \
	lib/libc/chkstk.s \
	lib/libc/math/acos.asm \
	lib/libc/math/asin.asm \
	lib/libc/math/atan.asm \
	lib/libc/math/atan2.asm \
	lib/libc/math/ceil.asm \
	lib/libc/math/cos.asm \
	lib/libc/math/cosh.asm \
	lib/libc/math/exp.asm \
	lib/libc/math/fabs.asm \
	lib/libc/math/floor.asm \
	lib/libc/math/fmod.asm \
	lib/libc/math/fpconst.asm \
	lib/libc/math/fpreset.asm \
	lib/libc/math/frexp.asm \
	lib/libc/math/ftol.asm \
	lib/libc/math/ldexp.asm \
	lib/libc/math/log.asm \
	lib/libc/math/log10.asm \
	lib/libc/math/modf.asm \
	lib/libc/math/pow.asm \
	lib/libc/math/sin.asm \
	lib/libc/math/sinh.asm \
	lib/libc/math/sqrt.asm \
	lib/libc/math/tan.asm \
	lib/libc/math/tanh.asm
LIBC_OBJ_DIR = build/machina/obj/libc
LIBC_OBJ_FILES = $(patsubst %,$(LIBC_OBJ_DIR)/%.o ,$(LIBC_SRC_FILES))

$(LIBC_OBJ_FILES): | LIBC_OBJ_MKDIR

LIBC_OBJ_MKDIR:
	@mkdir -p build/machina/obj/libc
	@mkdir -p build/machina/obj/libc/lib/libc/pthread
	@mkdir -p build/machina/obj/libc/lib/libc/regex
	@mkdir -p build/machina/obj/libc/lib/libc/math
	@mkdir -p build/machina/obj/libc/lib/libc

$(LIBC_OBJ_DIR)/%.c.o: $(LIBC_SRC_DIR)/%.c
	@echo -e '\x1b[34;1mCompiling $< \x1b[0m'
	$(CC) $(LIBC_CFLAGS) -DTARGET_MACHINE=$(TARGET_MACHINE) -c $< -o $@

$(LIBC_OBJ_DIR)/%.s.o: $(LIBC_SRC_DIR)/%.s
	@echo -e '\x1b[34;1mCompiling $<\x1b[0m'
	$(CC) -x assembler-with-cpp $(LIBC_CFLAGS) -c $< -o $@

$(LIBC_OBJ_DIR)/%.asm.o: $(LIBC_SRC_DIR)/%.asm
	@echo -e '\x1b[34;1mCompiling $<\x1b[0m'
	$(NASM) $(LIBC_NFLAGS) $< -o $@

LIBC_CLEAN :
	@rm -f $(LIBC_OBJ_FILES)

$(LIBC_OUT_FILE) libc : $(NASM_OUT_FILE) build/tools/ar  $(LIBC_OBJ_FILES)
	@echo -e '\x1b[34;1mBuilding Machina Standard C Library for x86\x1b[0m'
	@mkdir -p $(LIBC_OUT_DIR)
	$(AR) -s -m $(LIBC_OUT_FILE) $(LIBC_OBJ_FILES)


#
# Machina PXE Stage 1 Bootloader 
#
NETBOOT_NFLAGS = $(NFLAGS) -f bin
NETBOOT_OUT_DIR = build/install/boot
NETBOOT_OUT_FILE = $(NETBOOT_OUT_DIR)/netboot.bin
NETBOOT_SRC_FILES = \
	src/arch/x86/boot/netboot.asm

$(NETBOOT_OUT_FILE) netboot : $(NASM_OUT_FILE) 
	@echo -e '\x1b[34;1mBuilding Machina PXE Stage 1 Bootloader\x1b[0m'
	@mkdir -p $(NETBOOT_OUT_DIR)
	$(NASM) $(NETBOOT_NFLAGS) $(NETBOOT_SRC_FILES) -o $(NETBOOT_OUT_FILE)


#
# Machina CD-ROM Stage 1 Bootloader 
#
CDEMBOOT_NFLAGS = $(NFLAGS) -f bin
CDEMBOOT_OUT_DIR = build/install/boot
CDEMBOOT_OUT_FILE = $(CDEMBOOT_OUT_DIR)/cdemboot.bin
CDEMBOOT_SRC_FILES = \
	src/arch/x86/boot/cdemboot.asm

$(CDEMBOOT_OUT_FILE) cdemboot : $(NASM_OUT_FILE) 
	@echo -e '\x1b[34;1mBuilding Machina CD-ROM Stage 1 Bootloader\x1b[0m'
	@mkdir -p $(CDEMBOOT_OUT_DIR)
	$(NASM) $(CDEMBOOT_NFLAGS) $(CDEMBOOT_SRC_FILES) -o $(CDEMBOOT_OUT_FILE)


#
# Machina Kernel Library for x86 
#
LIBKERNEL_CFLAGS =  -I src/include -D OS_LIB $(CFLAGS)
LIBKERNEL_LDFLAGS =  -shared -entry _start@12 -fixed 0x7FF00000 -nostdlib $(LDFLAGS)
LIBKERNEL_NFLAGS = $(NFLAGS)
LIBKERNEL_OUT_DIR = build/install/boot
LIBKERNEL_OUT_FILE = $(LIBKERNEL_OUT_DIR)/kernel32.so
LIBKERNEL_SRC_DIR = src
LIBKERNEL_SRC_FILES = \
	sys/os/critsect.c \
	sys/os/environ.c \
	sys/os/heap.c \
	sys/os/netdb.c \
	sys/os/os.c \
	sys/os/resolv.c \
	sys/os/signal.c \
	sys/os/sntp.c \
	sys/os/sysapi.c \
	sys/os/syserr.c \
	sys/os/syslog.c \
	sys/os/thread.c \
	sys/os/tls.c \
	sys/os/userdb.c \
	lib/libc/bitops.c \
	lib/libc/crypt.c \
	lib/libc/ctype.c \
	lib/libc/fcvt.c \
	lib/libc/inifile.c \
	lib/libc/moddb.c \
	lib/libc/opts.c \
	lib/libc/strftime.c \
	lib/libc/string.c \
	lib/libc/strtol.c \
	lib/libc/tcccrt.c \
	lib/libc/time.c \
	lib/libc/verinfo.c \
	lib/libc/vsprintf.c \
	lib/libc/math/modf.asm
LIBKERNEL_OBJ_DIR = build/machina/obj/kernel32
LIBKERNEL_OBJ_FILES = $(patsubst %,$(LIBKERNEL_OBJ_DIR)/%.o ,$(LIBKERNEL_SRC_FILES))

$(LIBKERNEL_OBJ_FILES): | LIBKERNEL_OBJ_MKDIR

LIBKERNEL_OBJ_MKDIR:
	@mkdir -p build/machina/obj/kernel32
	@mkdir -p build/machina/obj/kernel32/lib/libc/math
	@mkdir -p build/machina/obj/kernel32/sys/os
	@mkdir -p build/machina/obj/kernel32/lib/libc

$(LIBKERNEL_OBJ_DIR)/%.c.o: $(LIBKERNEL_SRC_DIR)/%.c
	@echo -e '\x1b[34;1mCompiling $< \x1b[0m'
	$(CC) $(LIBKERNEL_CFLAGS) -DTARGET_MACHINE=$(TARGET_MACHINE) -c $< -o $@

$(LIBKERNEL_OBJ_DIR)/%.asm.o: $(LIBKERNEL_SRC_DIR)/%.asm
	@echo -e '\x1b[34;1mCompiling $<\x1b[0m'
	$(NASM) $(LIBKERNEL_NFLAGS) $< -o $@

LIBKERNEL_CLEAN :
	@rm -f $(LIBKERNEL_OBJ_FILES)

$(LIBKERNEL_OUT_FILE) : $(NASM_OUT_FILE)  $(LIBKERNEL_OBJ_FILES)
	@echo -e '\x1b[34;1mBuilding Machina Kernel Library for x86\x1b[0m'
	@mkdir -p $(LIBKERNEL_OUT_DIR)
	$(CC) -DTARGET_MACHINE=$(TARGET_MACHINE) $(LIBKERNEL_LDFLAGS) $(LIBKERNEL_OBJ_FILES) -o $(LIBKERNEL_OUT_FILE)


#
# Machina CD image 
#
ISO_OUT_DIR = build
ISO_OUT_FILE = $(ISO_OUT_DIR)/machina.iso
$(ISO_OUT_FILE) iso : $(CDEMBOOT_OUT_FILE) $(OSLDR_OUT_FILE) $(MKDFS_OUT_FILE) kernel-image 
	@echo -e '\x1b[34;1mBuilding Machina CD image\x1b[0m'
	@mkdir -p $(ISO_OUT_DIR)
	build/tools/mkdfs -d build/install/BOOTIMG.BIN -b $(CDEMBOOT_OUT_FILE) -l $(OSLDR_OUT_FILE) -k $(KRNLIMG32_OUT_FILE) -c 1024 -C 1440 -I 8192 -i -f -K rootdev=cd0,rootfs=cdfs
	genisoimage -J -f -c BOOTCAT.BIN -b BOOTIMG.BIN -o $(ISO_OUT_FILE) build/install


#
# Machina Stage 1 Bootloader 
#
DISKBOOT_NFLAGS = $(NFLAGS) -f bin
DISKBOOT_OUT_DIR = build/install/boot
DISKBOOT_OUT_FILE = $(DISKBOOT_OUT_DIR)/diskboot.bin
DISKBOOT_SRC_FILES = \
	src/arch/x86/boot/boot.asm

$(DISKBOOT_OUT_FILE) diskboot : $(NASM_OUT_FILE) 
	@echo -e '\x1b[34;1mBuilding Machina Stage 1 Bootloader\x1b[0m'
	@mkdir -p $(DISKBOOT_OUT_DIR)
	$(NASM) $(DISKBOOT_NFLAGS) $(DISKBOOT_SRC_FILES) -o $(DISKBOOT_OUT_FILE)


#
# Machina Kernel Image for x86 
#
KRNLIMG32_OUT_DIR = build/install/boot
KRNLIMG32_OUT_FILE = $(KRNLIMG32_OUT_DIR)/kernel32.bin
$(KRNLIMG32_OUT_FILE) kernel-image : $(KERNEL32_OUT_FILE) 
	@echo -e '\x1b[34;1mBuilding Machina Kernel Image for x86\x1b[0m'
	@mkdir -p $(KRNLIMG32_OUT_DIR)
	objcopy -O binary -j .text -j .rodata -j .bss -j .data --set-section-flags .bss=alloc,load,contents $(KERNEL32_OUT_FILE) $(KRNLIMG32_OUT_FILE)


#
# Machina OS Loader 
#
OSLDR_OUT_DIR = build/install/boot
OSLDR_OUT_FILE = $(OSLDR_OUT_DIR)/osloader.bin
$(OSLDR_OUT_FILE) osloader : $(NASM_OUT_FILE) $(OSLDRM_OUT_FILE) 
	@echo -e '\x1b[34;1mBuilding Machina OS Loader\x1b[0m'
	@mkdir -p $(OSLDR_OUT_DIR)
	objcopy -O binary -j .text -j .rodata -j .bss -j .data --set-section-flags .bss=alloc,load,contents $(OSLDRM_OUT_FILE) $(OSLDRM_OUT_FILE).bin
	cat $(OSLDRS_OUT_FILE) $(OSLDRM_OUT_FILE).bin > $(OSLDR_OUT_FILE)


#
# Machina OS Loader Main 
#
OSLDRM_CFLAGS =  -D OSLDR -D KERNEL -I src/include -masm=intel -nostdlib $(CFLAGS)
OSLDRM_LDFLAGS =  -Wl,-e,start -Wl,-T,src/arch/x86/sys/osloader/osloader.lds -nostdlib $(LDFLAGS)
OSLDRM_NFLAGS = $(NFLAGS)
OSLDRM_OUT_DIR = build/machina/obj/osloader
OSLDRM_OUT_FILE = $(OSLDRM_OUT_DIR)/osloader-main.elf
OSLDRM_SRC_DIR = src
OSLDRM_SRC_FILES = \
	arch/x86/sys/osloader/osloader.c \
	arch/x86/sys/osloader/kernel.c \
	arch/x86/sys/osloader/unzip.c \
	lib/libc/vsprintf.c \
	lib/libc/string.c \
	arch/x86/sys/osloader/bioscall.asm
OSLDRM_OBJ_DIR = build/machina/obj/osloader
OSLDRM_OBJ_FILES = $(patsubst %,$(OSLDRM_OBJ_DIR)/%.o ,$(OSLDRM_SRC_FILES))

$(OSLDRM_OBJ_FILES): | OSLDRM_OBJ_MKDIR

OSLDRM_OBJ_MKDIR:
	@mkdir -p build/machina/obj/osloader
	@mkdir -p build/machina/obj/osloader/arch/x86/sys/osloader
	@mkdir -p build/machina/obj/osloader/lib/libc

$(OSLDRM_OBJ_DIR)/%.c.o: $(OSLDRM_SRC_DIR)/%.c
	@echo -e '\x1b[34;1mCompiling $< \x1b[0m'
	$(CC) $(OSLDRM_CFLAGS) -DTARGET_MACHINE=$(TARGET_MACHINE) -c $< -o $@

$(OSLDRM_OBJ_DIR)/%.asm.o: $(OSLDRM_SRC_DIR)/%.asm
	@echo -e '\x1b[34;1mCompiling $<\x1b[0m'
	$(NASM) $(OSLDRM_NFLAGS) $< -o $@

OSLDRM_CLEAN :
	@rm -f $(OSLDRM_OBJ_FILES)

$(OSLDRM_OUT_FILE) osloader-main : $(NASM_OUT_FILE) $(OSLDRS_OUT_FILE)  $(OSLDRM_OBJ_FILES)
	@echo -e '\x1b[34;1mBuilding Machina OS Loader Main\x1b[0m'
	@mkdir -p $(OSLDRM_OUT_DIR)
	$(CC) -DTARGET_MACHINE=$(TARGET_MACHINE) $(OSLDRM_LDFLAGS) $(OSLDRM_OBJ_FILES) -o $(OSLDRM_OUT_FILE)


#
# NASM x86 Assembler for GNU/Linux 
#
NASM_CFLAGS =  -DOF_ONLY -DOF_ELF32 -DOF_WIN32 -DOF_COFF -DOF_OBJ -DOF_BIN -DOF_DBG -DOF_DEFAULT=of_elf32 -DHAVE_SNPRINTF -DHAVE_VSNPRINTF -Isrc/bin/as $(CFLAGS)
NASM_LDFLAGS = $(LDFLAGS)
NASM_NFLAGS = $(NFLAGS)
NASM_OUT_DIR = build/tools
NASM_OUT_FILE = $(NASM_OUT_DIR)/nasm
NASM_SRC_DIR = src/bin/as
NASM_SRC_FILES = \
	nasm.c \
	nasmlib.c \
	ver.c \
	raa.c \
	saa.c \
	rbtree.c \
	float.c \
	insnsa.c \
	insnsb.c \
	directiv.c \
	assemble.c \
	labels.c \
	hashtbl.c \
	crc64.c \
	parser.c \
	preproc.c \
	quote.c \
	pptok.c \
	macros.c \
	listing.c \
	eval.c \
	exprlib.c \
	stdscan.c \
	strfunc.c \
	tokhash.c \
	regvals.c \
	regflags.c \
	ilog2.c \
	strlcpy.c \
	output/outform.c \
	output/outlib.c \
	output/nulldbg.c \
	output/nullout.c \
	output/outbin.c \
	output/outcoff.c \
	output/outelf.c \
	output/outelf32.c \
	output/outobj.c \
	output/outdbg.c
NASM_OBJ_DIR = build/linux/obj/bin/nasm
NASM_OBJ_FILES = $(patsubst %,$(NASM_OBJ_DIR)/%.o ,$(NASM_SRC_FILES))

$(NASM_OBJ_FILES): | NASM_OBJ_MKDIR

NASM_OBJ_MKDIR:
	@mkdir -p build/linux/obj/bin/nasm
	@mkdir -p build/linux/obj/bin/nasm/output

$(NASM_OBJ_DIR)/%.c.o: $(NASM_SRC_DIR)/%.c
	@echo -e '\x1b[34;1mCompiling $< \x1b[0m'
	$(CC) $(NASM_CFLAGS) -DTARGET_MACHINE=$(TARGET_MACHINE) -c $< -o $@

NASM_CLEAN :
	@rm -f $(NASM_OBJ_FILES)

$(NASM_OUT_FILE) nasm :  $(NASM_OBJ_FILES)
	@echo -e '\x1b[34;1mBuilding NASM x86 Assembler for GNU/Linux\x1b[0m'
	@mkdir -p $(NASM_OUT_DIR)
	$(CC) -DTARGET_MACHINE=$(TARGET_MACHINE) $(NASM_LDFLAGS) $(NASM_OBJ_FILES) -o $(NASM_OUT_FILE)

all: $(KERNEL32_OUT_FILE) $(MKDFS_OUT_FILE) $(OSLDRS_OUT_FILE) $(LIBC_OUT_FILE) $(NETBOOT_OUT_FILE) $(CDEMBOOT_OUT_FILE) $(LIBKERNEL_OUT_FILE) $(ISO_OUT_FILE) $(DISKBOOT_OUT_FILE) $(KRNLIMG32_OUT_FILE) $(OSLDR_OUT_FILE) $(OSLDRM_OUT_FILE) $(NASM_OUT_FILE) 

