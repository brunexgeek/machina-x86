#!/usr/bin/python

import re

FIELD_PLATFORM = "platform"
FIELD_OUTPUT_DIRECTORY = "output_dir"
FIELD_OUTPUT_FILE = "output"
FIELD_SOURCES = "sources"
FIELD_SOURCE_DIRECTORY = "source_dir"
FIELD_OBJECT_DIRECTORY = "object_dir"
FIELD_PREFFIX = "preffix"
FIELD_TYPE = "type"
FIELD_DESCRIPTION = "description"
FIELD_CFLAGS = "cflags"
FIELD_LDFLAGS = "ldflags"
FIELD_NAME = "name"
FIELD_DEPENDENCIES = "dependencies"

class MakefileGenerator:

	def toVar(self, name):
		return "$(" + name + ")"

	def toObject(self, fileName):
		if (fileName.endswith(".c")):
			return re.sub("\.c$", ".o", fileName)
		if (fileName.endswith(".asm")):
			return re.sub("\.asm$", ".o", fileName)

	def generateTarget(self, target):
		print "#\n#", target[FIELD_DESCRIPTION], "\n#"

		# print the welcome target
		TARGET_WELCOME = target[FIELD_PREFFIX] + "_WELCOME"
		print TARGET_WELCOME + ":"
		print "\t@echo ' '"
		print "\t@echo Building", target[FIELD_DESCRIPTION]
		print

		# print the CFLAGS definition
		TARGET_CFLAGS = target[FIELD_PREFFIX] + "_CFLAGS"
		if (FIELD_CFLAGS in target):
			print TARGET_CFLAGS, "= $(CFLAGS)", target[FIELD_CFLAGS]
		else:
			print TARGET_CFLAGS, "= $(CFLAGS)"

		# print the LDFLAGS definition
		TARGET_LDFLAGS = target[FIELD_PREFFIX] + "_LDFLAGS"
		if (FIELD_LDFLAGS in target):
			print TARGET_LDFLAGS, "= $(LDFLAGS)", target[FIELD_LDFLAGS]
		else:
			print TARGET_LDFLAGS, "= $(LDFLAGS)"

		TARGET_OUT_DIR = target[FIELD_PREFFIX] + "_OUT_DIR"
		print TARGET_OUT_DIR, "=", target[FIELD_OUTPUT_DIRECTORY]
		TARGET_OUT_FILE = target[FIELD_PREFFIX] + "_OUT_FILE"
		print TARGET_OUT_FILE, "=", self.toVar(TARGET_OUT_DIR) + "/" + target[FIELD_OUTPUT_FILE]

		# print the source dir variable
		TARGET_SRC_DIR = target[FIELD_PREFFIX] + "_SRC_DIR"
		print TARGET_SRC_DIR, "=", target[FIELD_SOURCE_DIRECTORY]

		# print the source files list
		TARGET_SRC_FILES = target[FIELD_PREFFIX] + "_SRC_FILES"
		print TARGET_SRC_FILES, "= \\"
		for index in range(len(target[FIELD_SOURCES])-1):
			print "\t", target[FIELD_SOURCES][index], "\\"
		print "\t", target[FIELD_SOURCES][len(target[FIELD_SOURCES])-1]

		# print the object dir variable
		TARGET_OBJ_DIR = target[FIELD_PREFFIX] + "_OBJ_DIR"
		print TARGET_OBJ_DIR, "=", target[FIELD_OBJECT_DIRECTORY]

		# print the object files list
		TARGET_OBJ_FILES = target[FIELD_PREFFIX] + "_OBJ_FILES"
		print TARGET_OBJ_FILES, "= \\"
		for index in range(len(target[FIELD_SOURCES])-1):
			print "\t" + self.toVar(TARGET_OBJ_DIR) + "/" + self.toObject(target[FIELD_SOURCES][index]), "\\"
		print "\t" + self.toVar(TARGET_OBJ_DIR) + "/" + self.toObject(target[FIELD_SOURCES][len(target[FIELD_SOURCES])-1])
		print

		# print the target from object directory creation
		TARGET_OBJ_MKDIR = target[FIELD_PREFFIX] + "_OBJ_MKDIR"
		print self.toVar(TARGET_OBJ_FILES) + ": | " + TARGET_OBJ_MKDIR
		print
		print TARGET_OBJ_MKDIR + ":"
		print "\t" + "@mkdir -p", target[FIELD_OBJECT_DIRECTORY]
		extraPaths = set()
		for entry in target[FIELD_SOURCES]:
			pos = entry.rfind("/")
			if (pos >= 0):
				extraPaths.add(entry[:pos])
		for entry in extraPaths:
			print "\t" + "@mkdir -p", target[FIELD_OBJECT_DIRECTORY] + "/" + entry
		print

		# print the target to compile each C source file
		print self.toVar(TARGET_OBJ_DIR) + "/%.o:", self.toVar(TARGET_SRC_DIR) + "/%.c"
		if (target[FIELD_PLATFORM] == "machina"):
			compiler = "$(TCC)"
		else:
			compiler = "$(CC) -O2 -m32"
		print "\t" + compiler, "-I", \
			self.toVar(TARGET_SRC_DIR), \
			self.toVar(TARGET_CFLAGS), \
			"-c $< -o $@"
		print

		# print the target to compile each ASM source file
		print self.toVar(TARGET_OBJ_DIR) + "/%.o:", self.toVar(TARGET_SRC_DIR) + "/%.asm"
		print "\t$(NASM) $< -o $@"
		print

		# print the target to compile the binary file
		if (FIELD_NAME in target):
			mainTarget = target[FIELD_NAME]
		else:
			mainTarget = self.toVar(TARGET_OUT_FILE)

		depends = ""
		if (FIELD_DEPENDENCIES in target):
			for entry in target[FIELD_DEPENDENCIES]:
				depends += entry + " "

		print mainTarget + ":", TARGET_WELCOME, depends, self.toVar(TARGET_OBJ_FILES)
		print "\t@mkdir -p", self.toVar(TARGET_OUT_DIR)
		if (target[FIELD_PLATFORM] == "machina"):
			compiler = "$(TCC)"
		else:
			compiler = "$(CC) -O2 -m32"
		print "\t" + compiler, "-I", \
			self.toVar(TARGET_SRC_DIR), \
			self.toVar(TARGET_CFLAGS), \
			self.toVar(TARGET_LDFLAGS), \
			self.toVar(TARGET_OBJ_FILES) , \
			"-o", self.toVar(TARGET_OUT_FILE)
		print

	def generateMakefile(self, targets):
		print "#!/bin/make -f"
		print
		print "CFLAGS:=$(CFLAGS) -Wimplicit"
		print "TCC = build/tools/cc"
		print "NASM = build/tools/as"
		print
		# create the "help" target
		print "help:"
		for key, current in targets.iteritems():
			if (FIELD_NAME in current):
				label = current[FIELD_NAME]
			else:
				label = self.toVar(current[FIELD_PREFFIX] + "_OUT_FILE")
			print "\t@echo", label
		print
		# create the all other targets
		for key, current in targets.iteritems():
			self.generateTarget(current)
		# create the "all" target
		print "all:",
		for key, current in targets.iteritems():
			if (FIELD_NAME in current):
				label = current[FIELD_NAME]
			else:
				label = self.toVar(current[FIELD_PREFFIX] + "_OUT_FILE")
			print label,
		print "\n"




#
# This script generate the Machina's makefiles.
#

TARGETS = {}

# Native Tiny C Compiler
target = {}
target[FIELD_PLATFORM] = "native"
target[FIELD_OUTPUT_DIRECTORY] = "build/tools"
target[FIELD_OUTPUT_FILE] = "cc"
target[FIELD_SOURCES] = \
	["asm386.c", \
	"asm.c", \
	"cc.c", \
	"codegen386.c", \
	"codegen.c", \
	"compiler.c", \
	"elf.c", \
	"pe.c", \
	"preproc.c", \
	"symbol.c", \
	"type.c", \
	"util.c" ]
target[FIELD_SOURCE_DIRECTORY] = "src/bin/cc"
target[FIELD_OBJECT_DIRECTORY] = "build/linux/obj/cc"
target[FIELD_PREFFIX] = "TCC"
target[FIELD_DESCRIPTION] = "Tiny C Compiler for GNU/Linux"
TARGETS["native_tcc"] = target
# Native NASM
target = {}
target[FIELD_PLATFORM] = "native"
target[FIELD_OUTPUT_DIRECTORY] = "build/tools"
target[FIELD_OUTPUT_FILE] = "as"
target[FIELD_SOURCES] = \
	["nasm.c", \
	"nasmlib.c", \
	"ver.c", \
	"raa.c", \
	"saa.c", \
	"rbtree.c", \
	"float.c", \
	"insnsa.c", \
	"insnsb.c", \
	"directiv.c", \
	"assemble.c", \
	"labels.c", \
	"hashtbl.c", \
	"crc64.c", \
	"parser.c", \
	"preproc.c", \
	"quote.c", \
	"pptok.c", \
	"macros.c", \
	"listing.c", \
	"eval.c", \
	"exprlib.c", \
	"stdscan.c", \
	"strfunc.c", \
	"tokhash.c", \
	"regvals.c", \
	"regflags.c", \
	"ilog2.c", \
	"strlcpy.c", \
	"output/outform.c", \
	"output/outlib.c", \
	"output/nulldbg.c", \
	"output/nullout.c", \
	"output/outbin.c", \
	"output/outcoff.c", \
	"output/outelf.c", \
	"output/outelf32.c", \
	"output/outobj.c", \
	"output/outdbg.c" ]
target[FIELD_SOURCE_DIRECTORY] = "src/bin/as"
target[FIELD_OBJECT_DIRECTORY] = "build/linux/obj/as"
target[FIELD_PREFFIX] = "NASM"
target[FIELD_DESCRIPTION] = "NASM x86 Assembler for GNU/Linux"
target[FIELD_CFLAGS] = "-DOF_ONLY -DOF_ELF32 -DOF_WIN32 -DOF_COFF -DOF_OBJ -DOF_BIN " \
	"-DOF_DBG -DOF_DEFAULT=of_elf32 -DHAVE_SNPRINTF -DHAVE_VSNPRINTF"
TARGETS["native_nasm"] = target
# Native AR
target = {}
target[FIELD_PLATFORM] = "native"
target[FIELD_OUTPUT_DIRECTORY] = "build/tools"
target[FIELD_OUTPUT_FILE] = "ar"
target[FIELD_SOURCES] = ["ar.c" ]
target[FIELD_SOURCE_DIRECTORY] = "src/bin/ar"
target[FIELD_OBJECT_DIRECTORY] = "build/linux/obj/ar"
target[FIELD_PREFFIX] = "AR"
target[FIELD_DESCRIPTION] = "Native AR"
TARGETS["native_ar"] = target
# Native MKDFS
target = {}
target[FIELD_PLATFORM] = "native"
target[FIELD_OUTPUT_DIRECTORY] = "build/tools"
target[FIELD_OUTPUT_FILE] = "mkdfs"
target[FIELD_SOURCES] = \
	["blockdev.c", \
	"vmdk.c", \
	"bitops.c", \
	"buf.c", \
	"dfs.c", \
	"dir.c", \
	"file.c", \
	"group.c", \
	"inode.c", \
	"mkdfs.c", \
	"super.c", \
	"vfs.c" ]
target[FIELD_SOURCE_DIRECTORY] = "utils/dfs"
target[FIELD_OBJECT_DIRECTORY] = "build/linux/obj/mkdfs"
target[FIELD_PREFFIX] = "MKDFS"
target[FIELD_DESCRIPTION] = "MKDFS Tool for GNU/Linux"
TARGETS["native_mkdfs"] = target
# Machina Kernel
target = {}
target[FIELD_PLATFORM] = "machina"
target[FIELD_DEPENDENCIES] = ["build/tools/cc", "build/tools/as"]
target[FIELD_CFLAGS] = "-I src/include -D KERNEL -D KRNL_LIB"
target[FIELD_LDFLAGS] = "-shared -entry _start@12 -fixed 0x80000000 -filealign 4096 -nostdlib"
target[FIELD_OUTPUT_DIRECTORY] = "build/install/boot"
target[FIELD_OUTPUT_FILE] = "kernel32.img"
target[FIELD_SOURCES] = \
	["sys/kernel/apm.c", \
	"sys/kernel/buf.c", \
	"sys/kernel/cpu.c", \
	"sys/kernel/dbg.c", \
	"sys/kernel/dev.c", \
	"sys/kernel/fpu.c", \
	"sys/kernel/hndl.c", \
	"sys/kernel/iomux.c", \
	"sys/kernel/iop.c", \
	"sys/kernel/iovec.c", \
	"sys/kernel/kmalloc.c", \
	"sys/kernel/kmem.c", \
	"sys/kernel/ldr.c", \
	"sys/kernel/mach.c", \
	"sys/kernel/object.c", \
	"sys/kernel/pci.c", \
	"sys/kernel/pdir.c", \
	"sys/kernel/pframe.c", \
	"sys/kernel/pic.c", \
	"sys/kernel/pit.c", \
	"sys/kernel/pnpbios.c", \
	"sys/kernel/queue.c", \
	"sys/kernel/sched.c", \
	"sys/kernel/start.c", \
	"sys/kernel/syscall.c", \
	"sys/kernel/timer.c", \
	"sys/kernel/trap.c", \
	"sys/kernel/user.c", \
	"sys/kernel/vfs.c", \
	"sys/kernel/virtio.c", \
	"sys/kernel/vmi.c", \
	"sys/kernel/vmm.c", \
	# devices
	"sys/dev/cons.c", \
	"sys/dev/fd.c", \
	"sys/dev/hd.c", \
	"sys/dev/kbd.c", \
	"sys/dev/klog.c", \
	"sys/dev/null.c", \
	"sys/dev/nvram.c", \
	"sys/dev/ramdisk.c", \
	"sys/dev/rnd.c", \
	"sys/dev/serial.c", \
	"sys/dev/smbios.c", \
	"sys/dev/video.c", \
	"sys/dev/virtioblk.c", \
	"sys/dev/virtiocon.c", \
	# network
	"sys/net/arp.c", \
	"sys/net/dhcp.c", \
	"sys/net/ether.c", \
	"sys/net/icmp.c", \
	"sys/net/inet.c", \
	"sys/net/ipaddr.c", \
	"sys/net/ip.c", \
	"sys/net/loopif.c", \
	"sys/net/netif.c", \
	"sys/net/pbuf.c", \
	"sys/net/raw.c", \
	"sys/net/rawsock.c", \
	"sys/net/socket.c", \
	"sys/net/stats.c", \
	"sys/net/tcp.c", \
	"sys/net/tcp_input.c", \
	"sys/net/tcp_output.c", \
	"sys/net/tcpsock.c", \
	"sys/net/udp.c", \
	"sys/net/udpsock.c", \
	# file system
	"sys/fs/cdfs/cdfs.c", \
	"sys/fs/devfs/devfs.c", \
	"sys/fs/dfs/dfs.c", \
	"sys/fs/dfs/dir.c", \
	"sys/fs/dfs/file.c", \
	"sys/fs/dfs/group.c", \
	"sys/fs/dfs/inode.c", \
	"sys/fs/dfs/super.c", \
	"sys/fs/pipefs/pipefs.c", \
	"sys/fs/procfs/procfs.c", \
	"sys/fs/smbfs/smbcache.c", \
	"sys/fs/smbfs/smbfs.c", \
	"sys/fs/smbfs/smbproto.c", \
	"sys/fs/smbfs/smbutil.c", \
	# internal libc
	"lib/libc/bitops.c", \
	"lib/libc/ctype.c", \
	"lib/libc/inifile.c", \
	"lib/libc/moddb.c", \
	"lib/libc/opts.c", \
	"lib/libc/rmap.c", \
	"lib/libc/string.c", \
	"lib/libc/strtol.c", \
	"lib/libc/tcccrt.c", \
	"lib/libc/time.c", \
	"lib/libc/verinfo.c", \
	"lib/libc/vsprintf.c" ]
target[FIELD_SOURCE_DIRECTORY] = "src"
target[FIELD_OBJECT_DIRECTORY] = "build/machina/obj/kernel32"
target[FIELD_PREFFIX] = "KERNEL32"
target[FIELD_DESCRIPTION] = "Machina Kernel for x86"
TARGETS["kernel"] = target
# Machina "libsys.so"
target = {}
target[FIELD_PLATFORM] = "machina"
target[FIELD_DEPENDENCIES] = ["build/tools/cc", "build/tools/as"]
target[FIELD_CFLAGS] = "-I src/include -D OS_LIB"
target[FIELD_LDFLAGS] = "-shared -entry _start@12 -fixed 0x7FF00000 -nostdlib"
target[FIELD_OUTPUT_DIRECTORY] = "build/install/boot"
target[FIELD_OUTPUT_FILE] = "libsys.so"
target[FIELD_SOURCES] = \
	["sys/os/critsect.c", \
	"sys/os/environ.c", \
	"sys/os/heap.c", \
	"sys/os/netdb.c", \
	"sys/os/os.c", \
	"sys/os/resolv.c", \
	"sys/os/signal.c", \
	"sys/os/sntp.c", \
	"sys/os/sysapi.c", \
	"sys/os/syserr.c", \
	"sys/os/syslog.c", \
	"sys/os/thread.c", \
	"sys/os/tls.c", \
	"sys/os/userdb.c", \
	"lib/libc/bitops.c", \
	"lib/libc/crypt.c", \
	"lib/libc/ctype.c", \
	"lib/libc/fcvt.c", \
	"lib/libc/inifile.c", \
	"lib/libc/moddb.c", \
	"lib/libc/opts.c", \
	"lib/libc/strftime.c", \
	"lib/libc/string.c", \
	"lib/libc/strtol.c", \
	"lib/libc/tcccrt.c", \
	"lib/libc/time.c", \
	"lib/libc/verinfo.c", \
	"lib/libc/vsprintf.c",
	"lib/libc/math/modf.asm" ]
target[FIELD_SOURCE_DIRECTORY] = "src"
target[FIELD_OBJECT_DIRECTORY] = "build/machina/obj/libsys"
target[FIELD_PREFFIX] = "LIBSYS"
target[FIELD_DESCRIPTION] = "Machina System Library for x86"
TARGETS["libsys"] = target

generator = MakefileGenerator()
generator.generateMakefile(TARGETS)
