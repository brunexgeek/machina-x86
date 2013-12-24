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
FIELD_NFLAGS = "FIELD_NFLAGS"
FIELD_COMMANDS = "FIELD_COMMANDS"

BIN_DYNAMIC = 1
BIN_STATIC = 2
BIN_EXECUTABLE = 4

LANG_C = 1
LANG_S = 2
LANG_ASM = 4

class MakefileTarget:

    def __init__(self, targetDefs):
        self.checkFields(targetDefs)
        self.target = targetDefs

    #
    # Returns the given suffix as a Makefile variable reference
    #
    @staticmethod
    def toVar(suffix, targetDefs):
        return "$(" + targetDefs[FIELD_PREFFIX] + "_" + suffix + ")"

    def _toVar(self, suffix):
        return MakefileTarget.toVar(suffix, self.target)

    #
    # Returns the given suffix as a Makefile variable name
    #
    @staticmethod
    def toVarName(suffix, targetDefs):
        return targetDefs[FIELD_PREFFIX] + "_" + suffix

    def _toVarName(self, suffix):
        return MakefileTarget.toVarName(suffix, self.target)

    #
    # Translate the file name from source to object.
    #
    @staticmethod
    def toObject(fileName):
        if (fileName.endswith(".c")):
            return re.sub("\.c$", ".o", fileName)
        if (fileName.endswith(".asm")):
            return re.sub("\.asm$", ".o", fileName)
        if (fileName.endswith(".s")):
            return re.sub("\.s$", ".o", fileName)
        raise AssertionError("Unknown file extension")

    @staticmethod
    def checkField(targetDefs, name):
        if (name not in targetDefs):
            raise KeyError("The field '" + name + "' must be present in target '" + targetDefs[FIELD_OUTPUT_FILE] + "'")

    @staticmethod
    def checkFields(targetDefs):
        MakefileTarget.checkField(targetDefs, FIELD_DESCRIPTION);
        MakefileTarget.checkField(targetDefs, FIELD_PREFFIX);
        MakefileTarget.checkField(targetDefs, FIELD_OBJECT_DIRECTORY);
        MakefileTarget.checkField(targetDefs, FIELD_SOURCES);
        MakefileTarget.checkField(targetDefs, FIELD_SOURCE_DIRECTORY);
        MakefileTarget.checkField(targetDefs, FIELD_OUTPUT_DIRECTORY);
        MakefileTarget.checkField(targetDefs, FIELD_OUTPUT_FILE);
        MakefileTarget.checkField(targetDefs, FIELD_TYPE);

    @staticmethod
    def getLanguages(targetDefs):
        # check which language targets will be necessary
        langs = 0
        if (FIELD_SOURCES not in targetDefs):
            return 0
        for entry in targetDefs[FIELD_SOURCES]:
            if (entry.endswith(".c")):
                langs |= LANG_C
            else:
                if (entry.endswith(".s")):
                    langs |= LANG_S
                else:
                    if (entry.endswith(".asm")):
                        langs |= LANG_ASM
        return langs;



class CMakefileTarget(MakefileTarget):

    @staticmethod
    def isCompatible(targetDefs):
        langs = MakefileTarget.getLanguages(targetDefs)
        return ((langs & LANG_C) > 0 or (langs & LANG_S) > 0)

    #
    # Generate the C Makefile target
    #
    def generateTarget(self):
        # welcome message
        print "\n#\n#", self.target[FIELD_DESCRIPTION], "\n#"

        # welcome target
        print self._toVarName("WELCOME") + ":"
        print "\t@echo"
        print "\t@echo Building", self.target[FIELD_DESCRIPTION]
        print

        # print the CFLAGS definition
        if (FIELD_CFLAGS in self.target):
            print self._toVarName("CFLAGS"), "= ", self.target[FIELD_CFLAGS], "$(CFLAGS)"
        else:
            print self._toVarName("CFLAGS"), "= $(CFLAGS)"

        # print the LDFLAGS definition
        if (FIELD_LDFLAGS in self.target):
            print self._toVarName("LDFLAGS"), "= ", self.target[FIELD_LDFLAGS], "$(LDFLAGS)"
        else:
            print self._toVarName("LDFLAGS"), "= $(LDFLAGS)"

        # print the NFLAGS definition
        if (FIELD_NFLAGS in self.target):
            print self._toVarName("NFLAGS"), "= $(NFLAGS)", self.target[FIELD_NFLAGS]
        else:
            print self._toVarName("NFLAGS"), "= $(NFLAGS)"

        # output directory and file
        print self._toVarName("OUT_DIR"), "=", self.target[FIELD_OUTPUT_DIRECTORY]
        print self._toVarName("OUT_FILE"), "=", self._toVar("OUT_DIR") + "/" + self.target[FIELD_OUTPUT_FILE]
        # source directory
        print self._toVarName("SRC_DIR"), "=", self.target[FIELD_SOURCE_DIRECTORY]
        # source file list
        print self._toVarName("SRC_FILES"), "= \\"
        for index in range(len(self.target[FIELD_SOURCES])-1):
            print "\t", self.target[FIELD_SOURCES][index], "\\"
        print "\t", self.target[FIELD_SOURCES][len(self.target[FIELD_SOURCES])-1]
        # object directory
        print self._toVarName("OBJ_DIR"), "=", self.target[FIELD_OBJECT_DIRECTORY]
        # object file list
        print self._toVarName("OBJ_FILES"), "=", "$(patsubst %," + self._toVar("OBJ_DIR") + "/%.o ," + self._toVar("SRC_FILES") + ")"
        print

        # target to object directory creation
        print self._toVar("OBJ_FILES") + ": | " + self._toVarName("OBJ_MKDIR")
        print
        print self._toVarName("OBJ_MKDIR") + ":"
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

        # check which language targets will be necessary
        langs = 0
        for entry in self.target[FIELD_SOURCES]:
            if (entry.endswith(".c")):
                langs |= LANG_C
            else:
                if (entry.endswith(".s")):
                    langs |= LANG_S
                else:
                    if (entry.endswith(".asm")):
                        langs |= LANG_ASM
        # set the current C compiler
        compiler = "$(CC)"
        # target to compile each C source file
        if ((langs & LANG_C) > 0):
            print self._toVar("OBJ_DIR") + "/%.c.o:", self._toVar("SRC_DIR") + "/%.c"
            print "\t" + compiler, self._toVar("CFLAGS"), "-DTARGET_MACHINE=$(TARGET_MACHINE)", "-c $< -o $@"
            print
        # target to compile each S source file
        if ((langs & LANG_S) > 0):
            print self._toVar("OBJ_DIR") + "/%.s.o:", self._toVar("SRC_DIR") + "/%.s"
            print "\t" + compiler, "-x assembler-with-cpp", self._toVar("CFLAGS"), "-c $< -o $@"
            print
        # target to compile each ASM source file
        if ((langs & LANG_ASM) > 0):
            print self._toVar("OBJ_DIR") + "/%.asm.o:", self._toVar("SRC_DIR") + "/%.asm"
            print "\t$(NASM)", self._toVar("NFLAGS"), "$< -o $@"
            print

        # target to clean de compilation
        print self._toVarName("CLEAN"), ":"
        print "\t@rm -f", self._toVar("OBJ_FILES")
        print

        # choose the main target name
        mainTarget = self._toVar("OUT_FILE")
        if (FIELD_NAME in self.target):
            mainTarget += " " + self.target[FIELD_NAME]
        # find the dependencies
        depends = ""
        if (FIELD_DEPENDENCIES in self.target):
            for entry in self.target[FIELD_DEPENDENCIES]:
                depends += entry + " "
        # target to compile the binary file
        print mainTarget, ":", depends, self._toVarName("WELCOME"), self._toVar("OBJ_FILES")
        print "\t@mkdir -p", self._toVar("OUT_DIR")
        # check if the current target is for BIN_DYNAMIC or BIN_EXECUTABLE
        if ((self.target[FIELD_TYPE] & BIN_STATIC) == 0):
            compiler = "$(CC)"
            print "\t" + compiler, \
                "-DTARGET_MACHINE=$(TARGET_MACHINE)", \
                self._toVar("LDFLAGS"), \
                self._toVar("OBJ_FILES") , \
                "-o", self._toVar("OUT_FILE")
        else:
            print "\t$(AR) -s -m", self._toVar("OUT_FILE"), self._toVar("OBJ_FILES")
        print



class AsmMakefileTarget(MakefileTarget):

    @staticmethod
    def isCompatible(targetDefs):
        langs = MakefileTarget.getLanguages(targetDefs)
        return ((langs & LANG_C) == 0 and (langs & LANG_S) == 0 and (langs & LANG_ASM) > 0)

    #
    # Generate the NASM Makefile target
    #
    def generateTarget(self):
        # welcome message
        print "\n#\n#", self.target[FIELD_DESCRIPTION], "\n#"

        # print the NFLAGS definition
        if (FIELD_NFLAGS in self.target):
            print self._toVarName("NFLAGS"), "= $(NFLAGS)", self.target[FIELD_NFLAGS]
        else:
            print self._toVarName("NFLAGS"), "= $(NFLAGS)"

        # output directory and file
        print self._toVarName("OUT_DIR"), "=", self.target[FIELD_OUTPUT_DIRECTORY]
        print self._toVarName("OUT_FILE"), "=", self._toVar("OUT_DIR") + "/" + self.target[FIELD_OUTPUT_FILE]
        # source file list
        print self._toVarName("SRC_FILES"), "= \\"
        for index in range(len(self.target[FIELD_SOURCES])-1):
            print "\t", self.target[FIELD_SOURCE_DIRECTORY] + "/" + self.target[FIELD_SOURCES][index], "\\"
        print "\t", self.target[FIELD_SOURCE_DIRECTORY] + "/" + self.target[FIELD_SOURCES][len(self.target[FIELD_SOURCES])-1]
        print

        # choose the main target name
        mainTarget = self._toVar("OUT_FILE")
        if (FIELD_NAME in self.target):
            mainTarget += " " + self.target[FIELD_NAME]
        # find the dependencies
        depends = ""
        if (FIELD_DEPENDENCIES in self.target):
            for entry in self.target[FIELD_DEPENDENCIES]:
                depends += entry + " "
        # target to compile the binary file
        print mainTarget, ":", depends
        print "\t@echo"
        print "\t@echo Building", self.target[FIELD_DESCRIPTION]
        print "\t@mkdir -p", self._toVar("OUT_DIR")
        # check if the current target is for BIN_DYNAMIC or BIN_EXECUTABLE
        print "\t$(NASM)", \
            self._toVar("NFLAGS"), \
            self._toVar("SRC_FILES") , \
            "-o", self._toVar("OUT_FILE")
        print



class CustomMakefileTarget(MakefileTarget):

    def __init__(self, targetDefs):
        self.target = targetDefs

    @staticmethod
    def isCompatible(targetDefs):
        return (FIELD_SOURCES not in targetDefs and FIELD_COMMANDS in targetDefs)

    #
    # Generate the NASM Makefile target
    #
    def generateTarget(self):
        # welcome message
        print "\n#\n#", self.target[FIELD_DESCRIPTION], "\n#"

        # output directory and file
        print self._toVarName("OUT_DIR"), "=", self.target[FIELD_OUTPUT_DIRECTORY]
        print self._toVarName("OUT_FILE"), "=", self._toVar("OUT_DIR") + "/" + self.target[FIELD_OUTPUT_FILE]

        # choose the main target name
        mainTarget = self._toVar("OUT_FILE")
        if (FIELD_NAME in self.target):
            mainTarget += " " + self.target[FIELD_NAME]
        # find the dependencies
        depends = ""
        if (FIELD_DEPENDENCIES in self.target):
            for entry in self.target[FIELD_DEPENDENCIES]:
                depends += entry + " "
        # target to compile the binary file
        print mainTarget, ":", depends
        print "\t@echo"
        print "\t@echo Building", self.target[FIELD_DESCRIPTION]
        print "\t@mkdir -p", self._toVar("OUT_DIR")
        # include the cursom commands
        for command in self.target[FIELD_COMMANDS]:
            print "\t" + command
        print



class MakefileGenerator:

    def __init__(self):
        self.targets = {}
        self.variables = {}

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
        print "\t@echo \"   all\""
        print "\t@echo \"   clean\""
        for key, current in self.targets.iteritems():
            label = MakefileTarget.toVar("OUT_FILE", current)
            if (FIELD_NAME in current):
                label += " (" + current[FIELD_NAME] + ")"
            print "\t@echo \"  ", label, "\""
        print
        # mark as PHONY all target names
        print ".PHONY: all clean",
        for key, current in self.targets.iteritems():
            if (FIELD_NAME in current):
                print current[FIELD_NAME],
        print
        # create the all other targets
        for key, current in self.targets.iteritems():
            if (AsmMakefileTarget.isCompatible(current)):
                temp = AsmMakefileTarget(current)
            else:
                if (CMakefileTarget.isCompatible(current)):
                    temp = CMakefileTarget(current)
                else:
                    if (CustomMakefileTarget.isCompatible(current)):
                        temp = CustomMakefileTarget(current)
                    else:
                        raise RuntimeError("Invalid Makefile target")
            temp.generateTarget()
        # create the "all" target
        print "all:",
        for key, current in self.targets.iteritems():
            print MakefileTarget.toVar("OUT_FILE", current),
        print "\n"




#
# This script generate the Machina's makefiles.
#

generator = MakefileGenerator()
generator.addVariable("CFLAGS", "-O0 -m32 -mtune=i686")
generator.addVariable("LDFLAGS", "-m32 -mtune=i686")
generator.addVariable("NASM", "build/tools/nasm")
generator.addVariable("TARGET_MACHINE", "x86")

#
# NASM x86 Assembler for GNU/Linux
#
target = {}
target[FIELD_NAME] = "nasm"
target[FIELD_DESCRIPTION] = "NASM x86 Assembler for GNU/Linux"
target[FIELD_PREFFIX] = "NASM"
target[FIELD_TYPE] = BIN_EXECUTABLE
target[FIELD_CFLAGS] = "-DOF_ONLY -DOF_ELF32 -DOF_WIN32 -DOF_COFF -DOF_OBJ -DOF_BIN " \
    "-DOF_DBG -DOF_DEFAULT=of_elf32 -DHAVE_SNPRINTF -DHAVE_VSNPRINTF -Isrc/bin/as"
target[FIELD_OUTPUT_DIRECTORY] = "build/tools"
target[FIELD_OUTPUT_FILE] = "nasm"
target[FIELD_OBJECT_DIRECTORY] = "build/linux/obj/bin/nasm"
target[FIELD_SOURCE_DIRECTORY] = "src/bin/as"
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
generator.addTarget(target);
#
# MKDFS Tool for GNU/Linux
#
target = {}
target[FIELD_DESCRIPTION] = "MKDFS Tool for GNU/Linux"
target[FIELD_PREFFIX] = "MKDFS"
target[FIELD_TYPE] = BIN_EXECUTABLE
target[FIELD_OUTPUT_DIRECTORY] = "build/tools"
target[FIELD_OUTPUT_FILE] = "mkdfs"
target[FIELD_OBJECT_DIRECTORY] = "build/linux/obj/utils/dfs"
target[FIELD_SOURCE_DIRECTORY] = "utils/dfs"
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
generator.addTarget(target);
#
# Machina Kernel for x86
#
target = {}
target[FIELD_NAME] = "kernel"
target[FIELD_DESCRIPTION] = "Machina Kernel for x86"
target[FIELD_PREFFIX] = "KERNEL32"
target[FIELD_TYPE] = BIN_EXECUTABLE
target[FIELD_CFLAGS] = "-g -O0 -I src/include -D KERNEL -D KRNL_LIB -nostdlib -masm=intel"
target[FIELD_LDFLAGS] = "-nostdlib -Wl,-T,src/sys/arch/x86/kernel/kernel.lds"
target[FIELD_OUTPUT_DIRECTORY] = "build/machina/obj/kernel"
target[FIELD_OUTPUT_FILE] = "kernel32.elf"
target[FIELD_OBJECT_DIRECTORY] = "build/machina/obj/kernel"
target[FIELD_SOURCE_DIRECTORY] = "src"
target[FIELD_SOURCES] = \
    [ \
    #"sys/kernel/apm.c", \
    "sys/kernel/assert.c", \
    "sys/kernel/buf.c", \
    "sys/kernel/cpu.c", \
    "sys/kernel/dbg.c", \
    "sys/kernel/dev.c", \
    "sys/kernel/fpu.c", \
    "sys/kernel/hndl.c", \
    "sys/kernel/iomux.c", \
    #"sys/kernel/iop.c", \
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
    "sys/arch/x86/kernel/sched.c", \
    "sys/arch/x86/kernel/sched.s", \
    "sys/arch/x86/kernel/mach.c", \
    "sys/arch/x86/kernel/mach.s", \
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
generator.addTarget(target);
#
# Machina Kernel Image for x86
#
target = {}
target[FIELD_NAME] = "kernel-image"
target[FIELD_DESCRIPTION] = "Machina Kernel Image for x86"
target[FIELD_PREFFIX] = "KRNLIMG32"
target[FIELD_DEPENDENCIES] = ["$(KERNEL32_OUT_FILE)"]
target[FIELD_OUTPUT_DIRECTORY] = "build/install/boot"
target[FIELD_OUTPUT_FILE] = "kernel32.bin"
target[FIELD_COMMANDS] = [
    "objcopy -O binary -j .text -j .rodata -j .bss -j .data --set-section-flags" \
    " .bss=alloc,load,contents $(KERNEL32_OUT_FILE) $(KRNLIMG32_OUT_FILE)" ]
generator.addTarget(target);
#
# Machina Kernel Library for x86
#
target = {}
target[FIELD_DESCRIPTION] = "Machina Kernel Library for x86"
target[FIELD_PREFFIX] = "LIBKERNEL"
target[FIELD_TYPE] = BIN_DYNAMIC
target[FIELD_CFLAGS] = "-I src/include -D OS_LIB"
target[FIELD_LDFLAGS] = "-shared -entry _start@12 -fixed 0x7FF00000 -nostdlib"
target[FIELD_DEPENDENCIES] = ["build/tools/nasm"]
target[FIELD_OUTPUT_DIRECTORY] = "build/install/boot"
target[FIELD_OUTPUT_FILE] = "kernel32.so"
target[FIELD_OBJECT_DIRECTORY] = "build/machina/obj/kernel32"
target[FIELD_SOURCE_DIRECTORY] = "src"
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
generator.addTarget(target);
#
# Machina Standard C Library for x86
#
target = {}
target[FIELD_NAME] = "libc"
target[FIELD_DESCRIPTION] = "Machina Standard C Library for x86"
target[FIELD_PREFFIX] = "LIBC"
target[FIELD_TYPE] = BIN_STATIC
target[FIELD_CFLAGS] = "-I src/include -D OS_LIB"
target[FIELD_LDFLAGS] = "-nostdlib"
target[FIELD_DEPENDENCIES] = ["build/tools/cc", "build/tools/as", "build/tools/ar"]
target[FIELD_OUTPUT_DIRECTORY] = "build/install/usr/lib"
target[FIELD_OUTPUT_FILE] = "libc.a"
target[FIELD_OBJECT_DIRECTORY] = "build/machina/obj/libc"
target[FIELD_SOURCE_DIRECTORY] = "src"
target[FIELD_SOURCES] = \
    ["lib/libc/assert.c", \
    "lib/libc/tcccrt.c", \
    "lib/libc/bsearch.c", \
    "lib/libc/conio.c", \
    "lib/libc/crt0.c", \
    "lib/libc/ctype.c", \
    "lib/libc/dirent.c", \
    "lib/libc/fcvt.c", \
    "lib/libc/fnmatch.c", \
    "lib/libc/fork.c", \
    "lib/libc/getopt.c", \
    "lib/libc/glob.c", \
    "lib/libc/hash.c", \
    "lib/libc/inifile.c", \
    "lib/libc/input.c", \
    "lib/libc/math.c", \
    "lib/libc/mman.c", \
    "lib/libc/opts.c", \
    "lib/libc/output.c", \
    "lib/libc/qsort.c", \
    "lib/libc/random.c", \
    "lib/libc/readline.c", \
    "lib/libc/rmap.c", \
    "lib/libc/rtttl.c", \
    "lib/libc/sched.c", \
    "lib/libc/semaphore.c", \
    "lib/libc/stdio.c", \
    "lib/libc/shlib.c", \
    "lib/libc/scanf.c", \
    "lib/libc/printf.c", \
    "lib/libc/tmpfile.c", \
    "lib/libc/popen.c", \
    "lib/libc/stdlib.c", \
    "lib/libc/strftime.c", \
    "lib/libc/string.c", \
    "lib/libc/strtod.c", \
    "lib/libc/strtol.c", \
    "lib/libc/termios.c", \
    "lib/libc/time.c", \
    "lib/libc/xtoa.c", \
    "lib/libc/regex/regcomp.c", \
    "lib/libc/regex/regexec.c", \
    "lib/libc/regex/regerror.c", \
    "lib/libc/regex/regfree.c", \
    "lib/libc/pthread/barrier.c", \
    "lib/libc/pthread/condvar.c", \
    "lib/libc/pthread/mutex.c", \
    "lib/libc/pthread/pthread.c", \
    "lib/libc/pthread/rwlock.c", \
    "lib/libc/pthread/spinlock.c", \
    "lib/libc/setjmp.c", \
    "lib/libc/chkstk.s", \
    # math
    "lib/libc/math/acos.asm", \
    "lib/libc/math/asin.asm", \
    "lib/libc/math/atan.asm", \
    "lib/libc/math/atan2.asm", \
    "lib/libc/math/ceil.asm", \
    "lib/libc/math/cos.asm", \
    "lib/libc/math/cosh.asm", \
    "lib/libc/math/exp.asm", \
    "lib/libc/math/fabs.asm", \
    "lib/libc/math/floor.asm", \
    "lib/libc/math/fmod.asm", \
    "lib/libc/math/fpconst.asm", \
    "lib/libc/math/fpreset.asm", \
    "lib/libc/math/frexp.asm", \
    "lib/libc/math/ftol.asm", \
    "lib/libc/math/ldexp.asm", \
    "lib/libc/math/log.asm", \
    "lib/libc/math/log10.asm", \
    "lib/libc/math/modf.asm", \
    "lib/libc/math/pow.asm", \
    "lib/libc/math/sin.asm", \
    "lib/libc/math/sinh.asm", \
    "lib/libc/math/sqrt.asm", \
    "lib/libc/math/tan.asm", \
    "lib/libc/math/tanh.asm" ]
generator.addTarget(target);
#
# Machina HD Stage 1 Bootloader
#
target = {}
target[FIELD_NAME] = "diskboot"
target[FIELD_DESCRIPTION] = "Machina Stage 1 Bootloader"
target[FIELD_PREFFIX] = "DISKBOOT"
target[FIELD_TYPE] = BIN_EXECUTABLE
target[FIELD_NFLAGS] = "-f bin"
target[FIELD_DEPENDENCIES] = ["nasm"]
target[FIELD_OUTPUT_DIRECTORY] = "build/install/boot"
target[FIELD_OUTPUT_FILE] = "diskboot.bin"
target[FIELD_OBJECT_DIRECTORY] = "build/machina/obj/boot"
target[FIELD_SOURCE_DIRECTORY] = "src"
target[FIELD_SOURCES] = ["sys/arch/x86/boot/boot.asm"]
generator.addTarget(target);
#
# Machina CD-ROM Stage 1 Bootloader
#
target = {}
target[FIELD_NAME] = "cdemboot"
target[FIELD_DESCRIPTION] = "Machina CD-ROM Stage 1 Bootloader"
target[FIELD_PREFFIX] = "CDEMBOOT"
target[FIELD_TYPE] = BIN_EXECUTABLE
target[FIELD_NFLAGS] = "-f bin"
target[FIELD_DEPENDENCIES] = ["nasm"]
target[FIELD_OUTPUT_DIRECTORY] = "build/install/boot"
target[FIELD_OUTPUT_FILE] = "cdemboot.bin"
target[FIELD_OBJECT_DIRECTORY] = "build/machina/obj/boot"
target[FIELD_SOURCE_DIRECTORY] = "src"
target[FIELD_SOURCES] = ["sys/arch/x86/boot/cdemboot.asm"]
generator.addTarget(target);
#
# Machina PXE Stage 1 Bootloader
#
target = {}
target[FIELD_NAME] = "netboot"
target[FIELD_DESCRIPTION] = "Machina PXE Stage 1 Bootloader"
target[FIELD_PREFFIX] = "NETBOOT"
target[FIELD_TYPE] = BIN_EXECUTABLE
target[FIELD_NFLAGS] = "-f bin"
target[FIELD_DEPENDENCIES] = ["nasm"]
target[FIELD_OUTPUT_DIRECTORY] = "build/install/boot"
target[FIELD_OUTPUT_FILE] = "netboot.bin"
target[FIELD_OBJECT_DIRECTORY] = "build/machina/obj/boot"
target[FIELD_SOURCE_DIRECTORY] = "src"
target[FIELD_SOURCES] = ["sys/arch/x86/boot/netboot.asm"]
generator.addTarget(target);
#
# Machina OS Loader Stub
#
target = {}
target[FIELD_NAME] = "osloader-stub"
target[FIELD_DESCRIPTION] = "Machina OS Loader Stub"
target[FIELD_PREFFIX] = "OSLDRS"
target[FIELD_TYPE] = BIN_EXECUTABLE
target[FIELD_NFLAGS] = "-f bin"
target[FIELD_DEPENDENCIES] = ["nasm"]
target[FIELD_OUTPUT_DIRECTORY] = "build/machina/obj/osloader"
target[FIELD_OUTPUT_FILE] = "osloader-stub.bin"
target[FIELD_OBJECT_DIRECTORY] = "build/machina/obj/osloader"
target[FIELD_SOURCE_DIRECTORY] = "src"
target[FIELD_SOURCES] = \
    ["sys/arch/x86/osloader/stub.asm" ]
generator.addTarget(target);
#
# Machina OS Loader Main
#
target = {}
target[FIELD_NAME] = "osloader-main"
target[FIELD_DESCRIPTION] = "Machina OS Loader Main"
target[FIELD_PREFFIX] = "OSLDRM"
target[FIELD_TYPE] = BIN_EXECUTABLE
target[FIELD_CFLAGS] = "-D OSLDR -D KERNEL -I src/include -masm=intel -nostdlib"
target[FIELD_LDFLAGS] = "-Wl,-e,start -Wl,-T,src/sys/arch/x86/osloader/osloader.lds -nostdlib" #-Wl,-Ttext,0x90800"
target[FIELD_DEPENDENCIES] = ["nasm", "osloader-stub"]
target[FIELD_OUTPUT_DIRECTORY] = "build/machina/obj/osloader"
target[FIELD_OUTPUT_FILE] = "osloader-main.elf"
target[FIELD_OBJECT_DIRECTORY] = "build/machina/obj/osloader"
target[FIELD_SOURCE_DIRECTORY] = "src"
target[FIELD_SOURCES] = \
    ["sys/arch/x86/osloader/osloader.c", \
    "sys/arch/x86/osloader/kernel.c", \
    "sys/arch/x86/osloader/unzip.c", \
    "lib/libc/vsprintf.c", \
    "lib/libc/string.c", \
    "sys/arch/x86/osloader/bioscall.asm" ]
generator.addTarget(target);
#
# Machina OS Loader
#
target = {}
target[FIELD_NAME] = "osloader"
target[FIELD_DESCRIPTION] = "Machina OS Loader"
target[FIELD_PREFFIX] = "OSLDR"
target[FIELD_DEPENDENCIES] = ["nasm", "osloader-stub", "osloader-main"]
target[FIELD_OUTPUT_DIRECTORY] = "build/install/boot"
target[FIELD_OUTPUT_FILE] = "osloader.bin"
target[FIELD_COMMANDS] = [
    "objcopy -O binary -j .text -j .rodata -j .bss -j .data --set-section-flags" \
    " .bss=alloc,load,contents $(OSLDRM_OUT_FILE) $(OSLDRM_OUT_FILE).bin", \
    "cat $(OSLDRS_OUT_FILE) $(OSLDRM_OUT_FILE).bin > $(OSLDR_OUT_FILE)" ]
generator.addTarget(target);

#
# Machina CD image
#
target = {}
target[FIELD_NAME] = "iso"
target[FIELD_DESCRIPTION] = "Machina CD image"
target[FIELD_PREFFIX] = "ISO"
target[FIELD_DEPENDENCIES] = ["cdemboot", "osloader", "kernel-image"]
target[FIELD_OUTPUT_DIRECTORY] = "build"
target[FIELD_OUTPUT_FILE] = "machina.iso"
target[FIELD_COMMANDS] = [
    "build/tools/mkdfs -d build/install/BOOTIMG.BIN -b $(CDEMBOOT_OUT_FILE) -l $(OSLDR_OUT_FILE)" \
#   " -k ../sanos/linux/install/boot/krnl.dll -c 1024 -C 1440 -I 8192 -i -f -K rootdev=cd0,rootfs=cdfs", \
    " -k $(KRNLIMG32_OUT_FILE) -c 1024 -C 1440 -I 8192 -i -f -K rootdev=cd0,rootfs=cdfs", \
    "genisoimage -J -f -c BOOTCAT.BIN -b BOOTIMG.BIN -o $(ISO_OUT_FILE) build/install" ]
generator.addTarget(target);

# Driver for NIC 3C905C
#target = {}
#target[FIELD_DESCRIPTION] = "Machina 3C905C NIC driver for x86"
#target[FIELD_PREFFIX] = "LIB3C905C"
#target[FIELD_TYPE] = BIN_DYNAMIC | BIN_MACHINA
#target[FIELD_CFLAGS] = "-I src/include -D KERNEL"
#target[FIELD_LDFLAGS] = "-shared -entry _start@12 -nostdlib -Lbuild/install/boot -lkernel32"
#target[FIELD_DEPENDENCIES] = ["build/tools/cc", "build/tools/as", "build/tools/ar", "build/install/boot/libkernel32.so"]
#target[FIELD_OUTPUT_DIRECTORY] = "build/install/sys"
#target[FIELD_OUTPUT_FILE] = "lib3c905c.sys"
#target[FIELD_OBJECT_DIRECTORY] = "build/machina/obj/dev/3c905c"
#target[FIELD_SOURCE_DIRECTORY] = "src"
#target[FIELD_SOURCES] = \
#   ["sys/dev/3c905c.c", \
#   "lib/libs/string.c" ]
#generator.addTarget(target);


generator.generateMakefile()
