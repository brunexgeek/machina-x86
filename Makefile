#!/bin/make -f

INTERACTIVE:=$(shell [ -t 0 ] && echo 1)

            ifdef INTERACTIVE
                COLOR_BLUE=\x1b[34;1m
                COLOR_RESET=\x1b[0m
            else
                COLOR_BLUE=\#\#\#
            endif
TARGET_MACHINE := x86
NASM := build/tools/nasm
LDFLAGS := -m32 -mtune=i686
override CFLAGS := $(CFLAGS) -Wall -Werror=overflow -Werror-implicit-function-declaration -m32 -mtune=i686

help:
	@echo "   all"
	@echo "   clean"
	@echo "   $(KERNEL32_OUT_FILE) (kernel) "
	@echo "   $(LIB3C905C_OUT_FILE) (nic3C905c) "
	@echo "   $(MKDFS_OUT_FILE) "
	@echo "   $(OSLDRS_OUT_FILE) (osloader-stub) "
	@echo "   $(KRNLDBG32_OUT_FILE) (kernel-debug) "
	@echo "   $(NETBOOT_OUT_FILE) (netboot) "
	@echo "   $(CDEMBOOT_OUT_FILE) (cdemboot) "
	@echo "   $(OSLDR_OUT_FILE) (osloader) "
	@echo "   $(ISO_OUT_FILE) (iso) "
	@echo "   $(DISKBOOT_OUT_FILE) (diskboot) "
	@echo "   $(OSLDRM_OUT_FILE) (osloader-main) "
	@echo "   $(NASM_OUT_FILE) (nasm) "

.PHONY: all clean kernel nic3C905c osloader-stub kernel-debug netboot cdemboot osloader iso diskboot osloader-main nasm

#
# Machina Kernel for x86 
#
KERNEL32_OUT_DIR = build/install/boot
KERNEL32_OUT_FILE = $(KERNEL32_OUT_DIR)/kernel32.so
$(KERNEL32_OUT_FILE) kernel : build/machina/kernel/kernel32-dbg.so 
	@echo -e '$(COLOR_BLUE)Building Machina Kernel for x86$(COLOR_RESET)'
	@mkdir -p $(KERNEL32_OUT_DIR)
	objcopy -O elf32-i386 -j .text -j .rodata -j .bss -j .data $(KRNLDBG32_OUT_FILE) $(KERNEL32_OUT_FILE)


#
# Machina 3C905C NIC driver for x86 
#
LIB3C905C_CFLAGS =  -I src/include -D KERNEL $(CFLAGS)
LIB3C905C_LDFLAGS =  -shared -nostdlib $(LDFLAGS)
LIB3C905C_NFLAGS = $(NFLAGS)
LIB3C905C_OUT_DIR = build/install/sys
LIB3C905C_OUT_FILE = $(LIB3C905C_OUT_DIR)/lib3c905c.sys
LIB3C905C_SRC_DIR = src
LIB3C905C_SRC_FILES = \
	sys/dev/3c905c.c \
	lib/libc/string.c
LIB3C905C_OBJ_DIR = build/machina/obj/dev/3c905c
LIB3C905C_OBJ_FILES = $(patsubst %,$(LIB3C905C_OBJ_DIR)/%.o ,$(LIB3C905C_SRC_FILES))

$(LIB3C905C_OBJ_FILES): | LIB3C905C_OBJ_MKDIR

LIB3C905C_OBJ_MKDIR:
	@mkdir -p build/machina/obj/dev/3c905c
	@mkdir -p build/machina/obj/dev/3c905c/lib/libc
	@mkdir -p build/machina/obj/dev/3c905c/sys/dev

$(LIB3C905C_OBJ_DIR)/%.c.o: $(LIB3C905C_SRC_DIR)/%.c
	@echo -e '$(COLOR_BLUE)Compiling $< $(COLOR_RESET)'
	$(CC) $(LIB3C905C_CFLAGS) -DTARGET_MACHINE=$(TARGET_MACHINE) -c $< -o $@

LIB3C905C_CLEAN :
	rm -f $(LIB3C905C_OBJ_FILES) $(LIB3C905C_OUT_FILE)

$(LIB3C905C_OUT_FILE) nic3C905c :  $(LIB3C905C_OBJ_FILES)
	@echo -e '$(COLOR_BLUE)Building Machina 3C905C NIC driver for x86$(COLOR_RESET)'
	@mkdir -p $(LIB3C905C_OUT_DIR)
	$(CC) -DTARGET_MACHINE=$(TARGET_MACHINE) $(LIB3C905C_LDFLAGS) $(LIB3C905C_OBJ_FILES) -o $(LIB3C905C_OUT_FILE)


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
	@echo -e '$(COLOR_BLUE)Compiling $< $(COLOR_RESET)'
	$(CC) $(MKDFS_CFLAGS) -DTARGET_MACHINE=$(TARGET_MACHINE) -c $< -o $@

MKDFS_CLEAN :
	rm -f $(MKDFS_OBJ_FILES) $(MKDFS_OUT_FILE)

$(MKDFS_OUT_FILE) :  $(MKDFS_OBJ_FILES)
	@echo -e '$(COLOR_BLUE)Building MKDFS Tool for GNU/Linux$(COLOR_RESET)'
	@mkdir -p $(MKDFS_OUT_DIR)
	$(CC) -DTARGET_MACHINE=$(TARGET_MACHINE) $(MKDFS_LDFLAGS) $(MKDFS_OBJ_FILES) -o $(MKDFS_OUT_FILE)


#
# Machina OS Loader Stub 
#
OSLDRS_NFLAGS = $(NFLAGS) -f bin
OSLDRS_OUT_DIR = build/machina/osloader
OSLDRS_OUT_FILE = $(OSLDRS_OUT_DIR)/osloader-stub.bin
OSLDRS_SRC_FILES = \
	src/arch/x86/sys/osloader/stub.asm

OSLDRS_CLEAN :
	rm -f $(OSLDRS_OBJ_FILES) $(OSLDRS_OUT_FILE)

$(OSLDRS_OUT_FILE) osloader-stub : build/tools/nasm 
	@echo -e '$(COLOR_BLUE)Building Machina OS Loader Stub$(COLOR_RESET)'
	@mkdir -p $(OSLDRS_OUT_DIR)
	$(NASM) $(OSLDRS_NFLAGS) $(OSLDRS_SRC_FILES) -o $(OSLDRS_OUT_FILE)


#
# Machina Kernel for x86 with Debug Symbols 
#
KRNLDBG32_CFLAGS =  -g -I src/include -D KERNEL -D KRNL_LIB -nostdlib -masm=intel $(CFLAGS)
KRNLDBG32_LDFLAGS =  -nostdlib -Wl,-T,src/arch/x86/sys/kernel/kernel.lds $(LDFLAGS)
KRNLDBG32_NFLAGS = $(NFLAGS)
KRNLDBG32_OUT_DIR = build/machina/kernel
KRNLDBG32_OUT_FILE = $(KRNLDBG32_OUT_DIR)/kernel32-dbg.so
KRNLDBG32_SRC_DIR = src
KRNLDBG32_SRC_FILES = \
	sys/kernel/kdebug.c \
	sys/kernel/buf.c \
	sys/kernel/cpu.c \
	sys/kernel/dbg.c \
	sys/kernel/elf32.c \
	sys/kernel/dev.c \
	sys/kernel/fpu.c \
	sys/kernel/hndl.c \
	sys/kernel/iomux.c \
	sys/kernel/rmap.c \
	sys/kernel/iovec.c \
	sys/kernel/kmalloc.c \
	sys/kernel/kmem.c \
	sys/kernel/mach.c \
	sys/kernel/object.c \
	sys/kernel/pci.c \
	sys/kernel/pdir.c \
	sys/kernel/pframe.c \
	sys/kernel/pic.c \
	sys/kernel/module.c \
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
	lib/libc/opts.c \
	lib/libc/string.c \
	lib/libc/strtol.c \
	lib/libc/tcccrt.c \
	lib/libc/time.c \
	lib/libc/verinfo.c \
	lib/libc/vsprintf.c
KRNLDBG32_OBJ_DIR = build/machina/obj/kernel
KRNLDBG32_OBJ_FILES = $(patsubst %,$(KRNLDBG32_OBJ_DIR)/%.o ,$(KRNLDBG32_SRC_FILES))

$(KRNLDBG32_OBJ_FILES): | KRNLDBG32_OBJ_MKDIR

KRNLDBG32_OBJ_MKDIR:
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

$(KRNLDBG32_OBJ_DIR)/%.c.o: $(KRNLDBG32_SRC_DIR)/%.c
	@echo -e '$(COLOR_BLUE)Compiling $< $(COLOR_RESET)'
	$(CC) $(KRNLDBG32_CFLAGS) -DTARGET_MACHINE=$(TARGET_MACHINE) -c $< -o $@

$(KRNLDBG32_OBJ_DIR)/%.s.o: $(KRNLDBG32_SRC_DIR)/%.s
	@echo -e '$(COLOR_BLUE)Compiling $<$(COLOR_RESET)'
	$(CC) -x assembler-with-cpp $(KRNLDBG32_CFLAGS) -c $< -o $@

KRNLDBG32_CLEAN :
	rm -f $(KRNLDBG32_OBJ_FILES) $(KRNLDBG32_OUT_FILE)

$(KRNLDBG32_OUT_FILE) kernel-debug : src/arch/x86/sys/kernel/kernel.lds  $(KRNLDBG32_OBJ_FILES)
	@echo -e '$(COLOR_BLUE)Building Machina Kernel for x86 with Debug Symbols$(COLOR_RESET)'
	@mkdir -p $(KRNLDBG32_OUT_DIR)
	$(CC) -DTARGET_MACHINE=$(TARGET_MACHINE) $(KRNLDBG32_LDFLAGS) $(KRNLDBG32_OBJ_FILES) -o $(KRNLDBG32_OUT_FILE)


#
# Machina PXE Stage 1 Bootloader 
#
NETBOOT_NFLAGS = $(NFLAGS) -f bin
NETBOOT_OUT_DIR = build/install/boot
NETBOOT_OUT_FILE = $(NETBOOT_OUT_DIR)/netboot.bin
NETBOOT_SRC_FILES = \
	src/arch/x86/boot/netboot.asm

NETBOOT_CLEAN :
	rm -f $(NETBOOT_OBJ_FILES) $(NETBOOT_OUT_FILE)

$(NETBOOT_OUT_FILE) netboot : build/tools/nasm 
	@echo -e '$(COLOR_BLUE)Building Machina PXE Stage 1 Bootloader$(COLOR_RESET)'
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

CDEMBOOT_CLEAN :
	rm -f $(CDEMBOOT_OBJ_FILES) $(CDEMBOOT_OUT_FILE)

$(CDEMBOOT_OUT_FILE) cdemboot : build/tools/nasm 
	@echo -e '$(COLOR_BLUE)Building Machina CD-ROM Stage 1 Bootloader$(COLOR_RESET)'
	@mkdir -p $(CDEMBOOT_OUT_DIR)
	$(NASM) $(CDEMBOOT_NFLAGS) $(CDEMBOOT_SRC_FILES) -o $(CDEMBOOT_OUT_FILE)


#
# Machina OS Loader 
#
OSLDR_OUT_DIR = build/install/boot
OSLDR_OUT_FILE = $(OSLDR_OUT_DIR)/osloader.bin
$(OSLDR_OUT_FILE) osloader : build/machina/osloader/osloader-stub.bin build/machina/osloader/osloader-main.elf 
	@echo -e '$(COLOR_BLUE)Building Machina OS Loader$(COLOR_RESET)'
	@mkdir -p $(OSLDR_OUT_DIR)
	objcopy -O binary -j .text -j .rodata -j .bss -j .data --set-section-flags .bss=alloc,load,contents $(OSLDRM_OUT_FILE) $(OSLDRM_OUT_FILE).bin
	cat $(OSLDRS_OUT_FILE) $(OSLDRM_OUT_FILE).bin > $(OSLDR_OUT_FILE)


#
# Machina CD image 
#
ISO_OUT_DIR = build
ISO_OUT_FILE = $(ISO_OUT_DIR)/machina.iso
$(ISO_OUT_FILE) iso : build/install/boot/cdemboot.bin build/install/boot/osloader.bin build/tools/mkdfs build/install/boot/kernel32.so 
	@echo -e '$(COLOR_BLUE)Building Machina CD image$(COLOR_RESET)'
	@mkdir -p $(ISO_OUT_DIR)
	mkdir -p build/install/dev
	mkdir -p build/install/proc
	build/tools/mkdfs -d build/install/BOOTIMG.BIN -b $(CDEMBOOT_OUT_FILE) -l $(OSLDR_OUT_FILE) -k $(KERNEL32_OUT_FILE) -c 1024 -C 1440 -I 8192 -i -f -K rootdev=cd0,rootfs=cdfs
	genisoimage -J -quiet -c BOOTCAT.BIN -b BOOTIMG.BIN -o $(ISO_OUT_FILE) build/install


#
# Machina Stage 1 Bootloader 
#
DISKBOOT_NFLAGS = $(NFLAGS) -f bin
DISKBOOT_OUT_DIR = build/install/boot
DISKBOOT_OUT_FILE = $(DISKBOOT_OUT_DIR)/diskboot.bin
DISKBOOT_SRC_FILES = \
	src/arch/x86/boot/boot.asm

DISKBOOT_CLEAN :
	rm -f $(DISKBOOT_OBJ_FILES) $(DISKBOOT_OUT_FILE)

$(DISKBOOT_OUT_FILE) diskboot : build/tools/nasm 
	@echo -e '$(COLOR_BLUE)Building Machina Stage 1 Bootloader$(COLOR_RESET)'
	@mkdir -p $(DISKBOOT_OUT_DIR)
	$(NASM) $(DISKBOOT_NFLAGS) $(DISKBOOT_SRC_FILES) -o $(DISKBOOT_OUT_FILE)


#
# Machina OS Loader Main 
#
OSLDRM_CFLAGS =  -D OSLDR -D KERNEL -I src/include -masm=intel -nostdlib $(CFLAGS)
OSLDRM_LDFLAGS =  -Wl,-e,start -Wl,-T,src/arch/x86/sys/osloader/osloader.lds -nostdlib $(LDFLAGS)
OSLDRM_NFLAGS = $(NFLAGS)
OSLDRM_OUT_DIR = build/machina/osloader
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
	@echo -e '$(COLOR_BLUE)Compiling $< $(COLOR_RESET)'
	$(CC) $(OSLDRM_CFLAGS) -DTARGET_MACHINE=$(TARGET_MACHINE) -c $< -o $@

$(OSLDRM_OBJ_DIR)/%.asm.o: $(OSLDRM_SRC_DIR)/%.asm
	@echo -e '$(COLOR_BLUE)Compiling $<$(COLOR_RESET)'
	$(NASM) $(OSLDRM_NFLAGS) $< -o $@

OSLDRM_CLEAN :
	rm -f $(OSLDRM_OBJ_FILES) $(OSLDRM_OUT_FILE)

$(OSLDRM_OUT_FILE) osloader-main : build/tools/nasm build/machina/osloader/osloader-stub.bin  $(OSLDRM_OBJ_FILES)
	@echo -e '$(COLOR_BLUE)Building Machina OS Loader Main$(COLOR_RESET)'
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
	@echo -e '$(COLOR_BLUE)Compiling $< $(COLOR_RESET)'
	$(CC) $(NASM_CFLAGS) -DTARGET_MACHINE=$(TARGET_MACHINE) -c $< -o $@

NASM_CLEAN :
	rm -f $(NASM_OBJ_FILES) $(NASM_OUT_FILE)

$(NASM_OUT_FILE) nasm :  $(NASM_OBJ_FILES)
	@echo -e '$(COLOR_BLUE)Building NASM x86 Assembler for GNU/Linux$(COLOR_RESET)'
	@mkdir -p $(NASM_OUT_DIR)
	$(CC) -DTARGET_MACHINE=$(TARGET_MACHINE) $(NASM_LDFLAGS) $(NASM_OBJ_FILES) -o $(NASM_OUT_FILE)

all: $(KERNEL32_OUT_FILE) $(LIB3C905C_OUT_FILE) $(MKDFS_OUT_FILE) $(OSLDRS_OUT_FILE) $(KRNLDBG32_OUT_FILE) $(NETBOOT_OUT_FILE) $(CDEMBOOT_OUT_FILE) $(OSLDR_OUT_FILE) $(ISO_OUT_FILE) $(DISKBOOT_OUT_FILE) $(OSLDRM_OUT_FILE) $(NASM_OUT_FILE) 

clean: LIB3C905C_CLEAN MKDFS_CLEAN OSLDRS_CLEAN KRNLDBG32_CLEAN NETBOOT_CLEAN CDEMBOOT_CLEAN DISKBOOT_CLEAN OSLDRM_CLEAN NASM_CLEAN 

