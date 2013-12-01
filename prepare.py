#!/usr/bin/python

import re

FIELD_PLATFORM = "FIELD_PLATFORM"
FIELD_OUTPUT_DIRECTORY = "FIELD_OUTPUT_DIRECTORY"
FIELD_OUTPUT_FILE = "FIELD_OUTPUT_FILE"
FIELD_SOURCES = "FIELD_SOURCES"
FIELD_SOURCE_DIRECTORY = "FIELD_SOURCE_DIRECTORY"
FIELD_OBJECT_DIRECTORY = "FIELD_OBJECT_DIRECTORY"
FIELD_PREFFIX = "FIELD_PREFFIX"
FIELD_TYPE = "FIELD_TYPE"
FIELD_DESCRIPTION = "FIELD_DESCRIPTION"
FIELD_CFLAGS = "FIELD_CFLAGS"
FIELD_LDFLAGS = "FIELD_LDFLAGS"
FIELD_NAME = "FIELD_NAME"
FIELD_DEPENDENCIES = "FIELD_DEPENDENCIES"

BIN_DYNAMIC = 1
BIN_STATIC = 2
BIN_EXECUTABLE = 4


class MakefileTarget:

	def __init__(self, targetDefs):
		self.checkFields(targetDefs)
		self.target = targetDefs

	#
	# Returns the given suffix as a Makefile variable reference
	#
	def toVar(self, suffix):
		return "$(" + self.target[FIELD_PREFFIX] + "_" + suffix + ")"

	#
	# Returns the given suffix as a Makefile variable name
	#
	def toVarName(self, suffix):
		return self.target[FIELD_PREFFIX] + "_" + suffix

	#
	# Translate the file name from source to object.
	#
	def toObject(self, fileName):
		if (fileName.endswith(".c")):
			return re.sub("\.c$", ".o", fileName)
		if (fileName.endswith(".asm")):
			return re.sub("\.asm$", ".o", fileName)
		if (fileName.endswith(".s")):
			return re.sub("\.s$", ".o", fileName)
		raise AssertionError("Unknown file extension")

	def checkField(self, targetDefs, name):
		if (name not in targetDefs):
			raise KeyError("The field '" + name + "' must be present in target '" + targetDefs[FIELD_OUTPUT_FILE] + "'")

	def checkFields(self, targetDefs):
		self.checkField(targetDefs, FIELD_DESCRIPTION);
		self.checkField(targetDefs, FIELD_PREFFIX);
		self.checkField(targetDefs, FIELD_OBJECT_DIRECTORY);
		self.checkField(targetDefs, FIELD_SOURCES);
		self.checkField(targetDefs, FIELD_SOURCE_DIRECTORY);
		self.checkField(targetDefs, FIELD_OUTPUT_DIRECTORY);
		self.checkField(targetDefs, FIELD_OUTPUT_FILE);
		self.checkField(targetDefs, FIELD_PLATFORM);
		self.checkField(targetDefs, FIELD_TYPE);

	#
	# Generate the Makefile target
	#
	def generateTarget(self):
		# welcome message
		print "\n#\n#", self.target[FIELD_DESCRIPTION], "\n#"

		# welcome target
		print self.toVarName("WELCOME") + ":"
		print "\t@echo"
		print "\t@echo Building", self.target[FIELD_DESCRIPTION]
		print

		# print the CFLAGS definition
		if (FIELD_CFLAGS in self.target):
			print self.toVarName("CFLAGS"), "= $(CFLAGS)", self.target[FIELD_CFLAGS]
		else:
			print self.toVarName("CFLAGS"), "= $(CFLAGS)"

		# print the LDFLAGS definition
		if (FIELD_LDFLAGS in self.target):
			print self.toVarName("LDFLAGS"), "= $(LDFLAGS)", self.target[FIELD_LDFLAGS]
		else:
			print self.toVarName("LDFLAGS"), "= $(LDFLAGS)"

		# output directory and file
		print self.toVarName("OUT_DIR"), "=", self.target[FIELD_OUTPUT_DIRECTORY]
		print self.toVarName("OUT_FILE"), "=", self.toVar("OUT_DIR") + "/" + self.target[FIELD_OUTPUT_FILE]
		# source directory
		print self.toVarName("SRC_DIR"), "=", self.target[FIELD_SOURCE_DIRECTORY]
		# source file list
		print self.toVarName("SRC_FILES"), "= \\"
		for index in range(len(self.target[FIELD_SOURCES])-1):
			print "\t", self.target[FIELD_SOURCES][index], "\\"
		print "\t", self.target[FIELD_SOURCES][len(self.target[FIELD_SOURCES])-1]
		# object directory
		print self.toVarName("OBJ_DIR"), "=", self.target[FIELD_OBJECT_DIRECTORY]
		# object file list
		print self.toVarName("OBJ_FILES"), "= \\"
		for index in range(len(self.target[FIELD_SOURCES])-1):
			print "\t" + self.toVar("OBJ_DIR") + "/" + self.toObject(self.target[FIELD_SOURCES][index]), "\\"
		print "\t" + self.toVar("OBJ_DIR") + "/" + self.toObject(self.target[FIELD_SOURCES][len(self.target[FIELD_SOURCES])-1])
		print

		# target to object directory creation
		print self.toVar("OBJ_FILES") + ": | " + self.toVarName("OBJ_MKDIR")
		print
		print self.toVarName("OBJ_MKDIR") + ":"
		print "\t" + "@mkdir -p", self.target[FIELD_OBJECT_DIRECTORY]
		# find extra paths to create
		extraPaths = set()
		for entry in self.target[FIELD_SOURCES]:
			pos = entry.rfind("/")
			if (pos >= 0):
				extraPaths.add(entry[:pos])
		# include the extra paths creation in the Makefile
		for entry in extraPaths:
			print "\t" + "@mkdir -p", self.target[FIELD_OBJECT_DIRECTORY] + "/" + entry
		print

		# target to compile each C source file
		print self.toVar("OBJ_DIR") + "/%.o:", self.toVar("SRC_DIR") + "/%.c"
		if (self.target[FIELD_PLATFORM] == "machina"):
			compiler = "$(TCC)"
		else:
			compiler = "$(CC) -O2 -m32"
		print "\t" + compiler, "-I", \
			self.toVar("SRC_DIR"), \
			self.toVar("CFLAGS"), \
			"-c $< -o $@"
		print
		# target to compile each S source file
		print self.toVar("OBJ_DIR") + "/%.o:", self.toVar("SRC_DIR") + "/%.s"
		print "\t$(TCC) -I", \
			self.toVar("SRC_DIR"), \
			self.toVar("CFLAGS"), \
			"-c $< -o $@"
		print
		# target to compile each ASM source file
		print self.toVar("OBJ_DIR") + "/%.o:", self.toVar("SRC_DIR") + "/%.asm"
		print "\t$(NASM) $< -o $@"
		print

		# choose the main target name
		if (FIELD_NAME in self.target):
			mainTarget = self.target[FIELD_NAME]
		else:
			mainTarget = self.toVar("OUT_FILE")
		# find the dependencies
		depends = ""
		if (FIELD_DEPENDENCIES in self.target):
			for entry in self.target[FIELD_DEPENDENCIES]:
				depends += entry + " "
		# target to compile the binary file
		print mainTarget + ":", depends, self.toVarName("WELCOME"), self.toVar("OBJ_FILES")
		print "\t@mkdir -p", self.toVar("OUT_DIR")
		if (self.target[FIELD_PLATFORM] == "machina"):
			compiler = "$(TCC)"
		else:
			compiler = "$(CC) -O2 -m32"
		if (self.target[FIELD_TYPE] == BIN_STATIC):
			print "\t$(AR) -s -m", self.toVar("OUT_FILE"), self.toVar("OBJ_FILES")
		else:
			print "\t" + compiler, "-I", \
				self.toVar("SRC_DIR"), \
				self.toVar("CFLAGS"), \
				self.toVar("LDFLAGS"), \
				self.toVar("OBJ_FILES") , \
				"-o", self.toVar("OUT_FILE")
		print


class MakefileGenerator:

	def __init__(self):
		self.targets = {}
		self.variables = {}

	#
	# Returns the given suffix as a Makefile variable reference
	#
	def toVar(self, preffix, suffix):
		return "$(" + preffix + "_" + suffix + ")"

	#
	# Returns the given suffix as a Makefile variable name
	#
	def toVarName(self, suffix):
		return preffix + "_" + suffix

	def addTarget(self, targetDefs):
		if (FIELD_NAME in targetDefs):
			self.targets[targetDefs[FIELD_NAME]] = targetDefs
		else:
			if ((FIELD_OUTPUT_DIRECTORY in targetDefs) and \
			    (FIELD_OUTPUT_FILE in targetDefs)):
				name = targetDefs[FIELD_OUTPUT_FILE] + "/" + FIELD_OUTPUT_FILE
				self.targets[targetDefs[FIELD_OUTPUT_FILE]] = targetDefs
			else:
				raise KeyError("The fields 'FIELD_OUTPUT_DIRECTORY' and 'FIELD_OUTPUT_FILE' must be present")

	def addVariable(self, name, value):
		self.variables[name] = value

	def generateMakefile(self):
		print "#!/bin/make -f"
		print

		# print the variables
		for name, value in self.variables.iteritems():
			print name, ":=", value
		print
		# create the "help" target
		print "help:"
		print "\t@echo all"
		print "\t@echo clean"
		for key, current in self.targets.iteritems():
			if (FIELD_NAME in current):
				label = current[FIELD_NAME]
			else:
				label = self.toVar(current[FIELD_PREFFIX], "OUT_FILE")
			print "\t@echo", label
		print
		# create the all other targets
		for key, current in self.targets.iteritems():
			temp = MakefileTarget(current)
			temp.generateTarget()
		# create the "all" target
		print "all:",
		for key, current in self.targets.iteritems():
			if (FIELD_NAME in current):
				label = current[FIELD_NAME]
			else:
				label = self.toVar(current[FIELD_PREFFIX], "OUT_FILE")
			print label,
		print "\n"




#
# This script generate the Machina's makefiles.
#

generator = MakefileGenerator()
generator.addVariable("CFLAGS", "$(CFLAGS) -Wimplicit")
generator.addVariable("TCC", "build/tools/cc")
generator.addVariable("NASM", "build/tools/as")
generator.addVariable("AR", "build/tools/ar")

# Native Tiny C Compiler
target = {}
target[FIELD_PLATFORM] = "native"
target[FIELD_TYPE] = BIN_EXECUTABLE
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
generator.addTarget(target);
# Native NASM
target = {}
target[FIELD_PLATFORM] = "native"
target[FIELD_TYPE] = BIN_EXECUTABLE
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
generator.addTarget(target);
# Native AR
target = {}
target[FIELD_PLATFORM] = "native"
target[FIELD_TYPE] = BIN_EXECUTABLE
target[FIELD_OUTPUT_DIRECTORY] = "build/tools"
target[FIELD_OUTPUT_FILE] = "ar"
target[FIELD_SOURCES] = ["ar.c" ]
target[FIELD_SOURCE_DIRECTORY] = "src/bin/ar"
target[FIELD_OBJECT_DIRECTORY] = "build/linux/obj/ar"
target[FIELD_PREFFIX] = "AR"
target[FIELD_DESCRIPTION] = "Native AR"
generator.addTarget(target);
# Native MKDFS
target = {}
target[FIELD_PLATFORM] = "native"
target[FIELD_TYPE] = BIN_EXECUTABLE
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
generator.addTarget(target);
# Machina Kernel
target = {}
target[FIELD_PLATFORM] = "machina"
target[FIELD_TYPE] = BIN_EXECUTABLE
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
generator.addTarget(target);
# Machina "libsys.so"
target = {}
target[FIELD_PLATFORM] = "machina"
target[FIELD_TYPE] = BIN_DYNAMIC
target[FIELD_DEPENDENCIES] = ["build/tools/cc", "build/tools/as"]
target[FIELD_CFLAGS] = "-I src/include -D OS_LIB"
target[FIELD_LDFLAGS] = "-shared -entry _start@12 -fixed 0x7FF00000 -nostdlib"
target[FIELD_OUTPUT_DIRECTORY] = "build/install/boot"
target[FIELD_OUTPUT_FILE] = "libkrn32.so"
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
target[FIELD_OBJECT_DIRECTORY] = "build/machina/obj/libkrn32"
target[FIELD_PREFFIX] = "LIBKERNEL"
target[FIELD_DESCRIPTION] = "Machina Kernel Library for x86"
generator.addTarget(target);
# Machina "libc.so"
target = {}
target[FIELD_PLATFORM] = "machina"
target[FIELD_TYPE] = BIN_STATIC
target[FIELD_DEPENDENCIES] = ["build/tools/cc", "build/tools/as", "build/tools/ar"]
target[FIELD_CFLAGS] = "-I src/include -D OS_LIB"
target[FIELD_LDFLAGS] = "-nostdlib"
target[FIELD_OUTPUT_DIRECTORY] = "build/install/lib"
target[FIELD_OUTPUT_FILE] = "libc.a"
target[FIELD_SOURCES] = \
	["assert.c", \
	"tcccrt.c", \
	"bsearch.c", \
	"conio.c", \
	"crt0.c", \
	"ctype.c", \
	"dirent.c", \
	"fcvt.c", \
	"fnmatch.c", \
	"fork.c", \
	"getopt.c", \
	"glob.c", \
	"hash.c", \
	"inifile.c", \
	"input.c", \
	"math.c", \
	"mman.c", \
	"opts.c", \
	"output.c", \
	"qsort.c", \
	"random.c", \
	"readline.c", \
	"rmap.c", \
	"rtttl.c", \
	"sched.c", \
	"semaphore.c", \
	"stdio.c", \
	"shlib.c", \
	"scanf.c", \
	"printf.c", \
	"tmpfile.c", \
	"popen.c", \
	"stdlib.c", \
	"strftime.c", \
	"string.c", \
	"strtod.c", \
	"strtol.c", \
	"termios.c", \
	"time.c", \
	"xtoa.c", \
	"regex/regcomp.c", \
	"regex/regexec.c", \
	"regex/regerror.c", \
	"regex/regfree.c", \
	"pthread/barrier.c", \
	"pthread/condvar.c", \
	"pthread/mutex.c", \
	"pthread/pthread.c", \
	"pthread/rwlock.c", \
	"pthread/spinlock.c", \
	"setjmp.c", \
	"chkstk.s", \
	# math
	"math/acos.asm", \
	"math/asin.asm", \
	"math/atan.asm", \
	"math/atan2.asm", \
	"math/ceil.asm", \
	"math/cos.asm", \
	"math/cosh.asm", \
	"math/exp.asm", \
	"math/fabs.asm", \
	"math/floor.asm", \
	"math/fmod.asm", \
	"math/fpconst.asm", \
	"math/fpreset.asm", \
	"math/frexp.asm", \
	"math/ftol.asm", \
	"math/ldexp.asm", \
	"math/log.asm", \
	"math/log10.asm", \
	"math/modf.asm", \
	"math/pow.asm", \
	"math/sin.asm", \
	"math/sinh.asm", \
	"math/sqrt.asm", \
	"math/tan.asm", \
	"math/tanh.asm" ]
target[FIELD_SOURCE_DIRECTORY] = "src/lib/libc"
target[FIELD_OBJECT_DIRECTORY] = "build/machina/obj/libc"
target[FIELD_PREFFIX] = "LIBC"
target[FIELD_DESCRIPTION] = "Machina Standard C Library for x86"
generator.addTarget(target);

generator.generateMakefile()
