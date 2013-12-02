#!/bin/make -f

AR := build/tools/ar
TCC := build/tools/cc
CFLAGS := $(CFLAGS) -Wimplicit
NASM := build/tools/as

help:
	@echo all
	@echo clean
	@echo $(MKDFS_OUT_FILE)
	@echo $(KERNEL32_OUT_FILE)
	@echo $(TCC_OUT_FILE)
	@echo $(NASM_OUT_FILE)
	@echo $(AR_OUT_FILE)
	@echo $(LIBC_OUT_FILE)
	@echo $(LIBKERNEL_OUT_FILE)


#
# MKDFS Tool for GNU/Linux 
#
MKDFS_WELCOME:
	@echo
	@echo Building MKDFS Tool for GNU/Linux

MKDFS_CFLAGS = $(CFLAGS)
MKDFS_LDFLAGS = $(LDFLAGS)
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
	$(CC) -O2 -m32 $(MKDFS_CFLAGS) -c $< -o $@

$(MKDFS_OUT_FILE):  MKDFS_WELCOME $(MKDFS_OBJ_FILES)
	@mkdir -p $(MKDFS_OUT_DIR)
	$(CC) -O2 -m32 $(MKDFS_CFLAGS) $(MKDFS_LDFLAGS) $(MKDFS_OBJ_FILES) -o $(MKDFS_OUT_FILE)


#
# Machina Kernel for x86 
#
KERNEL32_WELCOME:
	@echo
	@echo Building Machina Kernel for x86

KERNEL32_CFLAGS = $(CFLAGS) -I src/include -D KERNEL -D KRNL_LIB
KERNEL32_LDFLAGS = $(LDFLAGS) -shared -entry _start@12 -fixed 0x80000000 -filealign 4096 -nostdlib
KERNEL32_OUT_DIR = build/install/boot
KERNEL32_OUT_FILE = $(KERNEL32_OUT_DIR)/kernel32.img
KERNEL32_SRC_DIR = src
KERNEL32_SRC_FILES = \
	sys/kernel/apm.c \
	sys/kernel/buf.c \
	sys/kernel/cpu.c \
	sys/kernel/dbg.c \
	sys/kernel/dev.c \
	sys/kernel/fpu.c \
	sys/kernel/hndl.c \
	sys/kernel/iomux.c \
	sys/kernel/iop.c \
	sys/kernel/iovec.c \
	sys/kernel/kmalloc.c \
	sys/kernel/kmem.c \
	sys/kernel/ldr.c \
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
	lib/libc/rmap.c \
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
	@mkdir -p build/machina/obj/kernel/sys/fs/procfs
	@mkdir -p build/machina/obj/kernel/sys/fs/cdfs
	@mkdir -p build/machina/obj/kernel/sys/fs/devfs

$(KERNEL32_OBJ_DIR)/%.c.o: $(KERNEL32_SRC_DIR)/%.c
	$(TCC) $(KERNEL32_CFLAGS) -c $< -o $@

$(KERNEL32_OUT_FILE): build/tools/cc build/tools/as  KERNEL32_WELCOME $(KERNEL32_OBJ_FILES)
	@mkdir -p $(KERNEL32_OUT_DIR)
	$(TCC) $(KERNEL32_CFLAGS) $(KERNEL32_LDFLAGS) $(KERNEL32_OBJ_FILES) -o $(KERNEL32_OUT_FILE)


#
# Tiny C Compiler for GNU/Linux 
#
TCC_WELCOME:
	@echo
	@echo Building Tiny C Compiler for GNU/Linux

TCC_CFLAGS = $(CFLAGS)
TCC_LDFLAGS = $(LDFLAGS)
TCC_OUT_DIR = build/tools
TCC_OUT_FILE = $(TCC_OUT_DIR)/cc
TCC_SRC_DIR = src/bin/cc
TCC_SRC_FILES = \
	asm386.c \
	asm.c \
	cc.c \
	codegen386.c \
	codegen.c \
	compiler.c \
	elf.c \
	pe.c \
	preproc.c \
	symbol.c \
	type.c \
	util.c
TCC_OBJ_DIR = build/linux/obj/bin/cc
TCC_OBJ_FILES = $(patsubst %,$(TCC_OBJ_DIR)/%.o ,$(TCC_SRC_FILES))

$(TCC_OBJ_FILES): | TCC_OBJ_MKDIR

TCC_OBJ_MKDIR:
	@mkdir -p build/linux/obj/bin/cc

$(TCC_OBJ_DIR)/%.c.o: $(TCC_SRC_DIR)/%.c
	$(CC) -O2 -m32 $(TCC_CFLAGS) -c $< -o $@

$(TCC_OUT_FILE):  TCC_WELCOME $(TCC_OBJ_FILES)
	@mkdir -p $(TCC_OUT_DIR)
	$(CC) -O2 -m32 $(TCC_CFLAGS) $(TCC_LDFLAGS) $(TCC_OBJ_FILES) -o $(TCC_OUT_FILE)


#
# NASM x86 Assembler for GNU/Linux 
#
NASM_WELCOME:
	@echo
	@echo Building NASM x86 Assembler for GNU/Linux

NASM_CFLAGS = $(CFLAGS) -DOF_ONLY -DOF_ELF32 -DOF_WIN32 -DOF_COFF -DOF_OBJ -DOF_BIN -DOF_DBG -DOF_DEFAULT=of_elf32 -DHAVE_SNPRINTF -DHAVE_VSNPRINTF -Isrc/bin/as
NASM_LDFLAGS = $(LDFLAGS)
NASM_OUT_DIR = build/tools
NASM_OUT_FILE = $(NASM_OUT_DIR)/as
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
NASM_OBJ_DIR = build/linux/obj/bin/as
NASM_OBJ_FILES = $(patsubst %,$(NASM_OBJ_DIR)/%.o ,$(NASM_SRC_FILES))

$(NASM_OBJ_FILES): | NASM_OBJ_MKDIR

NASM_OBJ_MKDIR:
	@mkdir -p build/linux/obj/bin/as
	@mkdir -p build/linux/obj/bin/as/output

$(NASM_OBJ_DIR)/%.c.o: $(NASM_SRC_DIR)/%.c
	$(CC) -O2 -m32 $(NASM_CFLAGS) -c $< -o $@

$(NASM_OUT_FILE):  NASM_WELCOME $(NASM_OBJ_FILES)
	@mkdir -p $(NASM_OUT_DIR)
	$(CC) -O2 -m32 $(NASM_CFLAGS) $(NASM_LDFLAGS) $(NASM_OBJ_FILES) -o $(NASM_OUT_FILE)


#
# Native AR 
#
AR_WELCOME:
	@echo
	@echo Building Native AR

AR_CFLAGS = $(CFLAGS)
AR_LDFLAGS = $(LDFLAGS)
AR_OUT_DIR = build/tools
AR_OUT_FILE = $(AR_OUT_DIR)/ar
AR_SRC_DIR = src/bin/ar
AR_SRC_FILES = \
	ar.c
AR_OBJ_DIR = build/linux/obj/bin/ar
AR_OBJ_FILES = $(patsubst %,$(AR_OBJ_DIR)/%.o ,$(AR_SRC_FILES))

$(AR_OBJ_FILES): | AR_OBJ_MKDIR

AR_OBJ_MKDIR:
	@mkdir -p build/linux/obj/bin/ar

$(AR_OBJ_DIR)/%.c.o: $(AR_SRC_DIR)/%.c
	$(CC) -O2 -m32 $(AR_CFLAGS) -c $< -o $@

$(AR_OUT_FILE):  AR_WELCOME $(AR_OBJ_FILES)
	@mkdir -p $(AR_OUT_DIR)
	$(CC) -O2 -m32 $(AR_CFLAGS) $(AR_LDFLAGS) $(AR_OBJ_FILES) -o $(AR_OUT_FILE)


#
# Machina Standard C Library for x86 
#
LIBC_WELCOME:
	@echo
	@echo Building Machina Standard C Library for x86

LIBC_CFLAGS = $(CFLAGS) -I src/include -D OS_LIB
LIBC_LDFLAGS = $(LDFLAGS) -nostdlib
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
	$(TCC) $(LIBC_CFLAGS) -c $< -o $@

$(LIBC_OBJ_DIR)/%.asm.o: $(LIBC_SRC_DIR)/%.asm
	$(NASM) $< -o $@

$(LIBC_OUT_FILE): build/tools/cc build/tools/as build/tools/ar  LIBC_WELCOME $(LIBC_OBJ_FILES)
	@mkdir -p $(LIBC_OUT_DIR)
	$(AR) -s -m $(LIBC_OUT_FILE) $(LIBC_OBJ_FILES)


#
# Machina Kernel Library for x86 
#
LIBKERNEL_WELCOME:
	@echo
	@echo Building Machina Kernel Library for x86

LIBKERNEL_CFLAGS = $(CFLAGS) -I src/include -D OS_LIB
LIBKERNEL_LDFLAGS = $(LDFLAGS) -shared -entry _start@12 -fixed 0x7FF00000 -nostdlib
LIBKERNEL_OUT_DIR = build/install/boot
LIBKERNEL_OUT_FILE = $(LIBKERNEL_OUT_DIR)/libkernel32.so
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
LIBKERNEL_OBJ_DIR = build/machina/obj/libkernel32
LIBKERNEL_OBJ_FILES = $(patsubst %,$(LIBKERNEL_OBJ_DIR)/%.o ,$(LIBKERNEL_SRC_FILES))

$(LIBKERNEL_OBJ_FILES): | LIBKERNEL_OBJ_MKDIR

LIBKERNEL_OBJ_MKDIR:
	@mkdir -p build/machina/obj/libkernel32
	@mkdir -p build/machina/obj/libkernel32/lib/libc/math
	@mkdir -p build/machina/obj/libkernel32/sys/os
	@mkdir -p build/machina/obj/libkernel32/lib/libc

$(LIBKERNEL_OBJ_DIR)/%.c.o: $(LIBKERNEL_SRC_DIR)/%.c
	$(TCC) $(LIBKERNEL_CFLAGS) -c $< -o $@

$(LIBKERNEL_OBJ_DIR)/%.asm.o: $(LIBKERNEL_SRC_DIR)/%.asm
	$(NASM) $< -o $@

$(LIBKERNEL_OUT_FILE): build/tools/cc build/tools/as  LIBKERNEL_WELCOME $(LIBKERNEL_OBJ_FILES)
	@mkdir -p $(LIBKERNEL_OUT_DIR)
	$(TCC) $(LIBKERNEL_CFLAGS) $(LIBKERNEL_LDFLAGS) $(LIBKERNEL_OBJ_FILES) -o $(LIBKERNEL_OUT_FILE)

all: $(MKDFS_OUT_FILE) $(KERNEL32_OUT_FILE) $(TCC_OUT_FILE) $(NASM_OUT_FILE) $(AR_OUT_FILE) $(LIBC_OUT_FILE) $(LIBKERNEL_OUT_FILE) 

