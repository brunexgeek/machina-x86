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
	@echo $(LIBKRNL_OUT_FILE)


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
MKDFS_OBJ_DIR = build/linux/obj/mkdfs
MKDFS_OBJ_FILES = \
	$(MKDFS_OBJ_DIR)/blockdev.o \
	$(MKDFS_OBJ_DIR)/vmdk.o \
	$(MKDFS_OBJ_DIR)/bitops.o \
	$(MKDFS_OBJ_DIR)/buf.o \
	$(MKDFS_OBJ_DIR)/dfs.o \
	$(MKDFS_OBJ_DIR)/dir.o \
	$(MKDFS_OBJ_DIR)/file.o \
	$(MKDFS_OBJ_DIR)/group.o \
	$(MKDFS_OBJ_DIR)/inode.o \
	$(MKDFS_OBJ_DIR)/mkdfs.o \
	$(MKDFS_OBJ_DIR)/super.o \
	$(MKDFS_OBJ_DIR)/vfs.o

$(MKDFS_OBJ_FILES): | MKDFS_OBJ_MKDIR

MKDFS_OBJ_MKDIR:
	@mkdir -p build/linux/obj/mkdfs

$(MKDFS_OBJ_DIR)/%.o: $(MKDFS_SRC_DIR)/%.c
	$(CC) -O2 -m32 -I $(MKDFS_SRC_DIR) $(MKDFS_CFLAGS) -c $< -o $@

$(MKDFS_OBJ_DIR)/%.o: $(MKDFS_SRC_DIR)/%.s
	$(TCC) -I $(MKDFS_SRC_DIR) $(MKDFS_CFLAGS) -c $< -o $@

$(MKDFS_OBJ_DIR)/%.o: $(MKDFS_SRC_DIR)/%.asm
	$(NASM) $< -o $@

$(MKDFS_OUT_FILE):  MKDFS_WELCOME $(MKDFS_OBJ_FILES)
	@mkdir -p $(MKDFS_OUT_DIR)
	$(CC) -O2 -m32 -I $(MKDFS_SRC_DIR) $(MKDFS_CFLAGS) $(MKDFS_LDFLAGS) $(MKDFS_OBJ_FILES) -o $(MKDFS_OUT_FILE)


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
KERNEL32_OBJ_DIR = build/machina/obj/kernel32
KERNEL32_OBJ_FILES = \
	$(KERNEL32_OBJ_DIR)/sys/kernel/apm.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/buf.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/cpu.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/dbg.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/dev.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/fpu.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/hndl.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/iomux.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/iop.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/iovec.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/kmalloc.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/kmem.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/ldr.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/mach.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/object.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/pci.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/pdir.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/pframe.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/pic.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/pit.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/pnpbios.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/queue.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/sched.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/start.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/syscall.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/timer.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/trap.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/user.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/vfs.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/virtio.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/vmi.o \
	$(KERNEL32_OBJ_DIR)/sys/kernel/vmm.o \
	$(KERNEL32_OBJ_DIR)/sys/dev/cons.o \
	$(KERNEL32_OBJ_DIR)/sys/dev/fd.o \
	$(KERNEL32_OBJ_DIR)/sys/dev/hd.o \
	$(KERNEL32_OBJ_DIR)/sys/dev/kbd.o \
	$(KERNEL32_OBJ_DIR)/sys/dev/klog.o \
	$(KERNEL32_OBJ_DIR)/sys/dev/null.o \
	$(KERNEL32_OBJ_DIR)/sys/dev/nvram.o \
	$(KERNEL32_OBJ_DIR)/sys/dev/ramdisk.o \
	$(KERNEL32_OBJ_DIR)/sys/dev/rnd.o \
	$(KERNEL32_OBJ_DIR)/sys/dev/serial.o \
	$(KERNEL32_OBJ_DIR)/sys/dev/smbios.o \
	$(KERNEL32_OBJ_DIR)/sys/dev/video.o \
	$(KERNEL32_OBJ_DIR)/sys/dev/virtioblk.o \
	$(KERNEL32_OBJ_DIR)/sys/dev/virtiocon.o \
	$(KERNEL32_OBJ_DIR)/sys/net/arp.o \
	$(KERNEL32_OBJ_DIR)/sys/net/dhcp.o \
	$(KERNEL32_OBJ_DIR)/sys/net/ether.o \
	$(KERNEL32_OBJ_DIR)/sys/net/icmp.o \
	$(KERNEL32_OBJ_DIR)/sys/net/inet.o \
	$(KERNEL32_OBJ_DIR)/sys/net/ipaddr.o \
	$(KERNEL32_OBJ_DIR)/sys/net/ip.o \
	$(KERNEL32_OBJ_DIR)/sys/net/loopif.o \
	$(KERNEL32_OBJ_DIR)/sys/net/netif.o \
	$(KERNEL32_OBJ_DIR)/sys/net/pbuf.o \
	$(KERNEL32_OBJ_DIR)/sys/net/raw.o \
	$(KERNEL32_OBJ_DIR)/sys/net/rawsock.o \
	$(KERNEL32_OBJ_DIR)/sys/net/socket.o \
	$(KERNEL32_OBJ_DIR)/sys/net/stats.o \
	$(KERNEL32_OBJ_DIR)/sys/net/tcp.o \
	$(KERNEL32_OBJ_DIR)/sys/net/tcp_input.o \
	$(KERNEL32_OBJ_DIR)/sys/net/tcp_output.o \
	$(KERNEL32_OBJ_DIR)/sys/net/tcpsock.o \
	$(KERNEL32_OBJ_DIR)/sys/net/udp.o \
	$(KERNEL32_OBJ_DIR)/sys/net/udpsock.o \
	$(KERNEL32_OBJ_DIR)/sys/fs/cdfs/cdfs.o \
	$(KERNEL32_OBJ_DIR)/sys/fs/devfs/devfs.o \
	$(KERNEL32_OBJ_DIR)/sys/fs/dfs/dfs.o \
	$(KERNEL32_OBJ_DIR)/sys/fs/dfs/dir.o \
	$(KERNEL32_OBJ_DIR)/sys/fs/dfs/file.o \
	$(KERNEL32_OBJ_DIR)/sys/fs/dfs/group.o \
	$(KERNEL32_OBJ_DIR)/sys/fs/dfs/inode.o \
	$(KERNEL32_OBJ_DIR)/sys/fs/dfs/super.o \
	$(KERNEL32_OBJ_DIR)/sys/fs/pipefs/pipefs.o \
	$(KERNEL32_OBJ_DIR)/sys/fs/procfs/procfs.o \
	$(KERNEL32_OBJ_DIR)/sys/fs/smbfs/smbcache.o \
	$(KERNEL32_OBJ_DIR)/sys/fs/smbfs/smbfs.o \
	$(KERNEL32_OBJ_DIR)/sys/fs/smbfs/smbproto.o \
	$(KERNEL32_OBJ_DIR)/sys/fs/smbfs/smbutil.o \
	$(KERNEL32_OBJ_DIR)/lib/libc/bitops.o \
	$(KERNEL32_OBJ_DIR)/lib/libc/ctype.o \
	$(KERNEL32_OBJ_DIR)/lib/libc/inifile.o \
	$(KERNEL32_OBJ_DIR)/lib/libc/moddb.o \
	$(KERNEL32_OBJ_DIR)/lib/libc/opts.o \
	$(KERNEL32_OBJ_DIR)/lib/libc/rmap.o \
	$(KERNEL32_OBJ_DIR)/lib/libc/string.o \
	$(KERNEL32_OBJ_DIR)/lib/libc/strtol.o \
	$(KERNEL32_OBJ_DIR)/lib/libc/tcccrt.o \
	$(KERNEL32_OBJ_DIR)/lib/libc/time.o \
	$(KERNEL32_OBJ_DIR)/lib/libc/verinfo.o \
	$(KERNEL32_OBJ_DIR)/lib/libc/vsprintf.o

$(KERNEL32_OBJ_FILES): | KERNEL32_OBJ_MKDIR

KERNEL32_OBJ_MKDIR:
	@mkdir -p build/machina/obj/kernel32
	@mkdir -p build/machina/obj/kernel32/sys/fs/smbfs
	@mkdir -p build/machina/obj/kernel32/sys/fs/pipefs
	@mkdir -p build/machina/obj/kernel32/lib/libc
	@mkdir -p build/machina/obj/kernel32/sys/fs/dfs
	@mkdir -p build/machina/obj/kernel32/sys/net
	@mkdir -p build/machina/obj/kernel32/sys/kernel
	@mkdir -p build/machina/obj/kernel32/sys/dev
	@mkdir -p build/machina/obj/kernel32/sys/fs/procfs
	@mkdir -p build/machina/obj/kernel32/sys/fs/cdfs
	@mkdir -p build/machina/obj/kernel32/sys/fs/devfs

$(KERNEL32_OBJ_DIR)/%.o: $(KERNEL32_SRC_DIR)/%.c
	$(TCC) -I $(KERNEL32_SRC_DIR) $(KERNEL32_CFLAGS) -c $< -o $@

$(KERNEL32_OBJ_DIR)/%.o: $(KERNEL32_SRC_DIR)/%.s
	$(TCC) -I $(KERNEL32_SRC_DIR) $(KERNEL32_CFLAGS) -c $< -o $@

$(KERNEL32_OBJ_DIR)/%.o: $(KERNEL32_SRC_DIR)/%.asm
	$(NASM) $< -o $@

$(KERNEL32_OUT_FILE): build/tools/cc build/tools/as  KERNEL32_WELCOME $(KERNEL32_OBJ_FILES)
	@mkdir -p $(KERNEL32_OUT_DIR)
	$(TCC) -I $(KERNEL32_SRC_DIR) $(KERNEL32_CFLAGS) $(KERNEL32_LDFLAGS) $(KERNEL32_OBJ_FILES) -o $(KERNEL32_OUT_FILE)


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
TCC_OBJ_DIR = build/linux/obj/cc
TCC_OBJ_FILES = \
	$(TCC_OBJ_DIR)/asm386.o \
	$(TCC_OBJ_DIR)/asm.o \
	$(TCC_OBJ_DIR)/cc.o \
	$(TCC_OBJ_DIR)/codegen386.o \
	$(TCC_OBJ_DIR)/codegen.o \
	$(TCC_OBJ_DIR)/compiler.o \
	$(TCC_OBJ_DIR)/elf.o \
	$(TCC_OBJ_DIR)/pe.o \
	$(TCC_OBJ_DIR)/preproc.o \
	$(TCC_OBJ_DIR)/symbol.o \
	$(TCC_OBJ_DIR)/type.o \
	$(TCC_OBJ_DIR)/util.o

$(TCC_OBJ_FILES): | TCC_OBJ_MKDIR

TCC_OBJ_MKDIR:
	@mkdir -p build/linux/obj/cc

$(TCC_OBJ_DIR)/%.o: $(TCC_SRC_DIR)/%.c
	$(CC) -O2 -m32 -I $(TCC_SRC_DIR) $(TCC_CFLAGS) -c $< -o $@

$(TCC_OBJ_DIR)/%.o: $(TCC_SRC_DIR)/%.s
	$(TCC) -I $(TCC_SRC_DIR) $(TCC_CFLAGS) -c $< -o $@

$(TCC_OBJ_DIR)/%.o: $(TCC_SRC_DIR)/%.asm
	$(NASM) $< -o $@

$(TCC_OUT_FILE):  TCC_WELCOME $(TCC_OBJ_FILES)
	@mkdir -p $(TCC_OUT_DIR)
	$(CC) -O2 -m32 -I $(TCC_SRC_DIR) $(TCC_CFLAGS) $(TCC_LDFLAGS) $(TCC_OBJ_FILES) -o $(TCC_OUT_FILE)


#
# NASM x86 Assembler for GNU/Linux 
#
NASM_WELCOME:
	@echo
	@echo Building NASM x86 Assembler for GNU/Linux

NASM_CFLAGS = $(CFLAGS) -DOF_ONLY -DOF_ELF32 -DOF_WIN32 -DOF_COFF -DOF_OBJ -DOF_BIN -DOF_DBG -DOF_DEFAULT=of_elf32 -DHAVE_SNPRINTF -DHAVE_VSNPRINTF
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
NASM_OBJ_DIR = build/linux/obj/as
NASM_OBJ_FILES = \
	$(NASM_OBJ_DIR)/nasm.o \
	$(NASM_OBJ_DIR)/nasmlib.o \
	$(NASM_OBJ_DIR)/ver.o \
	$(NASM_OBJ_DIR)/raa.o \
	$(NASM_OBJ_DIR)/saa.o \
	$(NASM_OBJ_DIR)/rbtree.o \
	$(NASM_OBJ_DIR)/float.o \
	$(NASM_OBJ_DIR)/insnsa.o \
	$(NASM_OBJ_DIR)/insnsb.o \
	$(NASM_OBJ_DIR)/directiv.o \
	$(NASM_OBJ_DIR)/assemble.o \
	$(NASM_OBJ_DIR)/labels.o \
	$(NASM_OBJ_DIR)/hashtbl.o \
	$(NASM_OBJ_DIR)/crc64.o \
	$(NASM_OBJ_DIR)/parser.o \
	$(NASM_OBJ_DIR)/preproc.o \
	$(NASM_OBJ_DIR)/quote.o \
	$(NASM_OBJ_DIR)/pptok.o \
	$(NASM_OBJ_DIR)/macros.o \
	$(NASM_OBJ_DIR)/listing.o \
	$(NASM_OBJ_DIR)/eval.o \
	$(NASM_OBJ_DIR)/exprlib.o \
	$(NASM_OBJ_DIR)/stdscan.o \
	$(NASM_OBJ_DIR)/strfunc.o \
	$(NASM_OBJ_DIR)/tokhash.o \
	$(NASM_OBJ_DIR)/regvals.o \
	$(NASM_OBJ_DIR)/regflags.o \
	$(NASM_OBJ_DIR)/ilog2.o \
	$(NASM_OBJ_DIR)/strlcpy.o \
	$(NASM_OBJ_DIR)/output/outform.o \
	$(NASM_OBJ_DIR)/output/outlib.o \
	$(NASM_OBJ_DIR)/output/nulldbg.o \
	$(NASM_OBJ_DIR)/output/nullout.o \
	$(NASM_OBJ_DIR)/output/outbin.o \
	$(NASM_OBJ_DIR)/output/outcoff.o \
	$(NASM_OBJ_DIR)/output/outelf.o \
	$(NASM_OBJ_DIR)/output/outelf32.o \
	$(NASM_OBJ_DIR)/output/outobj.o \
	$(NASM_OBJ_DIR)/output/outdbg.o

$(NASM_OBJ_FILES): | NASM_OBJ_MKDIR

NASM_OBJ_MKDIR:
	@mkdir -p build/linux/obj/as
	@mkdir -p build/linux/obj/as/output

$(NASM_OBJ_DIR)/%.o: $(NASM_SRC_DIR)/%.c
	$(CC) -O2 -m32 -I $(NASM_SRC_DIR) $(NASM_CFLAGS) -c $< -o $@

$(NASM_OBJ_DIR)/%.o: $(NASM_SRC_DIR)/%.s
	$(TCC) -I $(NASM_SRC_DIR) $(NASM_CFLAGS) -c $< -o $@

$(NASM_OBJ_DIR)/%.o: $(NASM_SRC_DIR)/%.asm
	$(NASM) $< -o $@

$(NASM_OUT_FILE):  NASM_WELCOME $(NASM_OBJ_FILES)
	@mkdir -p $(NASM_OUT_DIR)
	$(CC) -O2 -m32 -I $(NASM_SRC_DIR) $(NASM_CFLAGS) $(NASM_LDFLAGS) $(NASM_OBJ_FILES) -o $(NASM_OUT_FILE)


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
AR_OBJ_DIR = build/linux/obj/ar
AR_OBJ_FILES = \
	$(AR_OBJ_DIR)/ar.o

$(AR_OBJ_FILES): | AR_OBJ_MKDIR

AR_OBJ_MKDIR:
	@mkdir -p build/linux/obj/ar

$(AR_OBJ_DIR)/%.o: $(AR_SRC_DIR)/%.c
	$(CC) -O2 -m32 -I $(AR_SRC_DIR) $(AR_CFLAGS) -c $< -o $@

$(AR_OBJ_DIR)/%.o: $(AR_SRC_DIR)/%.s
	$(TCC) -I $(AR_SRC_DIR) $(AR_CFLAGS) -c $< -o $@

$(AR_OBJ_DIR)/%.o: $(AR_SRC_DIR)/%.asm
	$(NASM) $< -o $@

$(AR_OUT_FILE):  AR_WELCOME $(AR_OBJ_FILES)
	@mkdir -p $(AR_OUT_DIR)
	$(CC) -O2 -m32 -I $(AR_SRC_DIR) $(AR_CFLAGS) $(AR_LDFLAGS) $(AR_OBJ_FILES) -o $(AR_OUT_FILE)


#
# Machina Standard C Library for x86 
#
LIBC_WELCOME:
	@echo
	@echo Building Machina Standard C Library for x86

LIBC_CFLAGS = $(CFLAGS) -I src/include -D OS_LIB
LIBC_LDFLAGS = $(LDFLAGS) -nostdlib
LIBC_OUT_DIR = build/install/lib
LIBC_OUT_FILE = $(LIBC_OUT_DIR)/libc.a
LIBC_SRC_DIR = src/lib/libc
LIBC_SRC_FILES = \
	assert.c \
	tcccrt.c \
	bsearch.c \
	conio.c \
	crt0.c \
	ctype.c \
	dirent.c \
	fcvt.c \
	fnmatch.c \
	fork.c \
	getopt.c \
	glob.c \
	hash.c \
	inifile.c \
	input.c \
	math.c \
	mman.c \
	opts.c \
	output.c \
	qsort.c \
	random.c \
	readline.c \
	rmap.c \
	rtttl.c \
	sched.c \
	semaphore.c \
	stdio.c \
	shlib.c \
	scanf.c \
	printf.c \
	tmpfile.c \
	popen.c \
	stdlib.c \
	strftime.c \
	string.c \
	strtod.c \
	strtol.c \
	termios.c \
	time.c \
	xtoa.c \
	regex/regcomp.c \
	regex/regexec.c \
	regex/regerror.c \
	regex/regfree.c \
	pthread/barrier.c \
	pthread/condvar.c \
	pthread/mutex.c \
	pthread/pthread.c \
	pthread/rwlock.c \
	pthread/spinlock.c \
	setjmp.c \
	chkstk.s \
	math/acos.asm \
	math/asin.asm \
	math/atan.asm \
	math/atan2.asm \
	math/ceil.asm \
	math/cos.asm \
	math/cosh.asm \
	math/exp.asm \
	math/fabs.asm \
	math/floor.asm \
	math/fmod.asm \
	math/fpconst.asm \
	math/fpreset.asm \
	math/frexp.asm \
	math/ftol.asm \
	math/ldexp.asm \
	math/log.asm \
	math/log10.asm \
	math/modf.asm \
	math/pow.asm \
	math/sin.asm \
	math/sinh.asm \
	math/sqrt.asm \
	math/tan.asm \
	math/tanh.asm
LIBC_OBJ_DIR = build/machina/obj/libc
LIBC_OBJ_FILES = \
	$(LIBC_OBJ_DIR)/assert.o \
	$(LIBC_OBJ_DIR)/tcccrt.o \
	$(LIBC_OBJ_DIR)/bsearch.o \
	$(LIBC_OBJ_DIR)/conio.o \
	$(LIBC_OBJ_DIR)/crt0.o \
	$(LIBC_OBJ_DIR)/ctype.o \
	$(LIBC_OBJ_DIR)/dirent.o \
	$(LIBC_OBJ_DIR)/fcvt.o \
	$(LIBC_OBJ_DIR)/fnmatch.o \
	$(LIBC_OBJ_DIR)/fork.o \
	$(LIBC_OBJ_DIR)/getopt.o \
	$(LIBC_OBJ_DIR)/glob.o \
	$(LIBC_OBJ_DIR)/hash.o \
	$(LIBC_OBJ_DIR)/inifile.o \
	$(LIBC_OBJ_DIR)/input.o \
	$(LIBC_OBJ_DIR)/math.o \
	$(LIBC_OBJ_DIR)/mman.o \
	$(LIBC_OBJ_DIR)/opts.o \
	$(LIBC_OBJ_DIR)/output.o \
	$(LIBC_OBJ_DIR)/qsort.o \
	$(LIBC_OBJ_DIR)/random.o \
	$(LIBC_OBJ_DIR)/readline.o \
	$(LIBC_OBJ_DIR)/rmap.o \
	$(LIBC_OBJ_DIR)/rtttl.o \
	$(LIBC_OBJ_DIR)/sched.o \
	$(LIBC_OBJ_DIR)/semaphore.o \
	$(LIBC_OBJ_DIR)/stdio.o \
	$(LIBC_OBJ_DIR)/shlib.o \
	$(LIBC_OBJ_DIR)/scanf.o \
	$(LIBC_OBJ_DIR)/printf.o \
	$(LIBC_OBJ_DIR)/tmpfile.o \
	$(LIBC_OBJ_DIR)/popen.o \
	$(LIBC_OBJ_DIR)/stdlib.o \
	$(LIBC_OBJ_DIR)/strftime.o \
	$(LIBC_OBJ_DIR)/string.o \
	$(LIBC_OBJ_DIR)/strtod.o \
	$(LIBC_OBJ_DIR)/strtol.o \
	$(LIBC_OBJ_DIR)/termios.o \
	$(LIBC_OBJ_DIR)/time.o \
	$(LIBC_OBJ_DIR)/xtoa.o \
	$(LIBC_OBJ_DIR)/regex/regcomp.o \
	$(LIBC_OBJ_DIR)/regex/regexec.o \
	$(LIBC_OBJ_DIR)/regex/regerror.o \
	$(LIBC_OBJ_DIR)/regex/regfree.o \
	$(LIBC_OBJ_DIR)/pthread/barrier.o \
	$(LIBC_OBJ_DIR)/pthread/condvar.o \
	$(LIBC_OBJ_DIR)/pthread/mutex.o \
	$(LIBC_OBJ_DIR)/pthread/pthread.o \
	$(LIBC_OBJ_DIR)/pthread/rwlock.o \
	$(LIBC_OBJ_DIR)/pthread/spinlock.o \
	$(LIBC_OBJ_DIR)/setjmp.o \
	$(LIBC_OBJ_DIR)/chkstk.o \
	$(LIBC_OBJ_DIR)/math/acos.o \
	$(LIBC_OBJ_DIR)/math/asin.o \
	$(LIBC_OBJ_DIR)/math/atan.o \
	$(LIBC_OBJ_DIR)/math/atan2.o \
	$(LIBC_OBJ_DIR)/math/ceil.o \
	$(LIBC_OBJ_DIR)/math/cos.o \
	$(LIBC_OBJ_DIR)/math/cosh.o \
	$(LIBC_OBJ_DIR)/math/exp.o \
	$(LIBC_OBJ_DIR)/math/fabs.o \
	$(LIBC_OBJ_DIR)/math/floor.o \
	$(LIBC_OBJ_DIR)/math/fmod.o \
	$(LIBC_OBJ_DIR)/math/fpconst.o \
	$(LIBC_OBJ_DIR)/math/fpreset.o \
	$(LIBC_OBJ_DIR)/math/frexp.o \
	$(LIBC_OBJ_DIR)/math/ftol.o \
	$(LIBC_OBJ_DIR)/math/ldexp.o \
	$(LIBC_OBJ_DIR)/math/log.o \
	$(LIBC_OBJ_DIR)/math/log10.o \
	$(LIBC_OBJ_DIR)/math/modf.o \
	$(LIBC_OBJ_DIR)/math/pow.o \
	$(LIBC_OBJ_DIR)/math/sin.o \
	$(LIBC_OBJ_DIR)/math/sinh.o \
	$(LIBC_OBJ_DIR)/math/sqrt.o \
	$(LIBC_OBJ_DIR)/math/tan.o \
	$(LIBC_OBJ_DIR)/math/tanh.o

$(LIBC_OBJ_FILES): | LIBC_OBJ_MKDIR

LIBC_OBJ_MKDIR:
	@mkdir -p build/machina/obj/libc
	@mkdir -p build/machina/obj/libc/regex
	@mkdir -p build/machina/obj/libc/math
	@mkdir -p build/machina/obj/libc/pthread

$(LIBC_OBJ_DIR)/%.o: $(LIBC_SRC_DIR)/%.c
	$(TCC) -I $(LIBC_SRC_DIR) $(LIBC_CFLAGS) -c $< -o $@

$(LIBC_OBJ_DIR)/%.o: $(LIBC_SRC_DIR)/%.s
	$(TCC) -I $(LIBC_SRC_DIR) $(LIBC_CFLAGS) -c $< -o $@

$(LIBC_OBJ_DIR)/%.o: $(LIBC_SRC_DIR)/%.asm
	$(NASM) $< -o $@

$(LIBC_OUT_FILE): build/tools/cc build/tools/as build/tools/ar  LIBC_WELCOME $(LIBC_OBJ_FILES)
	@mkdir -p $(LIBC_OUT_DIR)
	$(AR) -s -m $(LIBC_OUT_FILE) $(LIBC_OBJ_FILES)


#
# Machina Kernel Library for x86 
#
LIBKRNL_WELCOME:
	@echo
	@echo Building Machina Kernel Library for x86

LIBKRNL_CFLAGS = $(CFLAGS) -I src/include -D OS_LIB
LIBKRNL_LDFLAGS = $(LDFLAGS) -shared -entry _start@12 -fixed 0x7FF00000 -nostdlib
LIBKRNL_OUT_DIR = build/install/boot
LIBKRNL_OUT_FILE = $(LIBKRNL_OUT_DIR)/libkrnl.so
LIBKRNL_SRC_DIR = src
LIBKRNL_SRC_FILES = \
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
LIBKRNL_OBJ_DIR = build/machina/obj/libkrnl
LIBKRNL_OBJ_FILES = \
	$(LIBKRNL_OBJ_DIR)/sys/os/critsect.o \
	$(LIBKRNL_OBJ_DIR)/sys/os/environ.o \
	$(LIBKRNL_OBJ_DIR)/sys/os/heap.o \
	$(LIBKRNL_OBJ_DIR)/sys/os/netdb.o \
	$(LIBKRNL_OBJ_DIR)/sys/os/os.o \
	$(LIBKRNL_OBJ_DIR)/sys/os/resolv.o \
	$(LIBKRNL_OBJ_DIR)/sys/os/signal.o \
	$(LIBKRNL_OBJ_DIR)/sys/os/sntp.o \
	$(LIBKRNL_OBJ_DIR)/sys/os/sysapi.o \
	$(LIBKRNL_OBJ_DIR)/sys/os/syserr.o \
	$(LIBKRNL_OBJ_DIR)/sys/os/syslog.o \
	$(LIBKRNL_OBJ_DIR)/sys/os/thread.o \
	$(LIBKRNL_OBJ_DIR)/sys/os/tls.o \
	$(LIBKRNL_OBJ_DIR)/sys/os/userdb.o \
	$(LIBKRNL_OBJ_DIR)/lib/libc/bitops.o \
	$(LIBKRNL_OBJ_DIR)/lib/libc/crypt.o \
	$(LIBKRNL_OBJ_DIR)/lib/libc/ctype.o \
	$(LIBKRNL_OBJ_DIR)/lib/libc/fcvt.o \
	$(LIBKRNL_OBJ_DIR)/lib/libc/inifile.o \
	$(LIBKRNL_OBJ_DIR)/lib/libc/moddb.o \
	$(LIBKRNL_OBJ_DIR)/lib/libc/opts.o \
	$(LIBKRNL_OBJ_DIR)/lib/libc/strftime.o \
	$(LIBKRNL_OBJ_DIR)/lib/libc/string.o \
	$(LIBKRNL_OBJ_DIR)/lib/libc/strtol.o \
	$(LIBKRNL_OBJ_DIR)/lib/libc/tcccrt.o \
	$(LIBKRNL_OBJ_DIR)/lib/libc/time.o \
	$(LIBKRNL_OBJ_DIR)/lib/libc/verinfo.o \
	$(LIBKRNL_OBJ_DIR)/lib/libc/vsprintf.o \
	$(LIBKRNL_OBJ_DIR)/lib/libc/math/modf.o

$(LIBKRNL_OBJ_FILES): | LIBKRNL_OBJ_MKDIR

LIBKRNL_OBJ_MKDIR:
	@mkdir -p build/machina/obj/libkrnl
	@mkdir -p build/machina/obj/libkrnl/lib/libc/math
	@mkdir -p build/machina/obj/libkrnl/sys/os
	@mkdir -p build/machina/obj/libkrnl/lib/libc

$(LIBKRNL_OBJ_DIR)/%.o: $(LIBKRNL_SRC_DIR)/%.c
	$(TCC) -I $(LIBKRNL_SRC_DIR) $(LIBKRNL_CFLAGS) -c $< -o $@

$(LIBKRNL_OBJ_DIR)/%.o: $(LIBKRNL_SRC_DIR)/%.s
	$(TCC) -I $(LIBKRNL_SRC_DIR) $(LIBKRNL_CFLAGS) -c $< -o $@

$(LIBKRNL_OBJ_DIR)/%.o: $(LIBKRNL_SRC_DIR)/%.asm
	$(NASM) $< -o $@

$(LIBKRNL_OUT_FILE): build/tools/cc build/tools/as  LIBKRNL_WELCOME $(LIBKRNL_OBJ_FILES)
	@mkdir -p $(LIBKRNL_OUT_DIR)
	$(TCC) -I $(LIBKRNL_SRC_DIR) $(LIBKRNL_CFLAGS) $(LIBKRNL_LDFLAGS) $(LIBKRNL_OBJ_FILES) -o $(LIBKRNL_OUT_FILE)

all: $(MKDFS_OUT_FILE) $(KERNEL32_OUT_FILE) $(TCC_OUT_FILE) $(NASM_OUT_FILE) $(AR_OUT_FILE) $(LIBC_OUT_FILE) $(LIBKRNL_OUT_FILE) 

