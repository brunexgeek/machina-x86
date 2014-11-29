#!/usr/bin/python

from makegen import MakefileGenerator

from makegen import FIELD_PLATFORM
from makegen import FIELD_OUTPUT_DIRECTORY
from makegen import FIELD_OUTPUT_FILE
from makegen import FIELD_SOURCES
from makegen import FIELD_SOURCE_DIRECTORY
from makegen import FIELD_OBJECT_DIRECTORY
from makegen import FIELD_PREFFIX
from makegen import FIELD_TYPE
from makegen import FIELD_DESCRIPTION
from makegen import FIELD_CFLAGS
from makegen import FIELD_LDFLAGS
from makegen import FIELD_NAME
from makegen import FIELD_DEPENDENCIES
from makegen import FIELD_NFLAGS
from makegen import FIELD_COMMANDS

from makegen import BIN_DYNAMIC
from makegen import BIN_STATIC
from makegen import BIN_EXECUTABLE

#
# This script generate the Machina's makefiles.
#

generator = MakefileGenerator()
generator.addVariable("override CFLAGS", "$(CFLAGS) -Wall -Werror=overflow -Werror-implicit-function-declaration -m32 -mtune=i686")
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
# Machina Kernel for x86 with Debug Symbols
#
target = {}
target[FIELD_NAME] = "kernel-debug"
target[FIELD_DESCRIPTION] = "Machina Kernel for x86 with Debug Symbols"
target[FIELD_PREFFIX] = "KRNLDBG32"
target[FIELD_TYPE] = BIN_EXECUTABLE
target[FIELD_CFLAGS] = "-g -I src/include -D KERNEL -D KRNL_LIB -nostdlib -masm=intel"
target[FIELD_LDFLAGS] = "-nostdlib -Wl,-T,src/arch/x86/sys/kernel/kernel.lds"
target[FIELD_OUTPUT_DIRECTORY] = "build/machina/kernel"
target[FIELD_OUTPUT_FILE] = "kernel32-dbg.so"
target[FIELD_OBJECT_DIRECTORY] = "build/machina/obj/kernel"
target[FIELD_SOURCE_DIRECTORY] = "src"
target[FIELD_SOURCES] = \
    [ \
    #"sys/kernel/apm.c", \
    "sys/kernel/kdebug.c", \
    "sys/kernel/buf.c", \
    "sys/kernel/cpu.c", \
    "sys/kernel/dbg.c", \
    "sys/kernel/elf32.c", \
    "sys/kernel/dev.c", \
    "sys/kernel/fpu.c", \
    "sys/kernel/hndl.c", \
    "sys/kernel/iomux.c", \
    "sys/kernel/rmap.c", \
    "sys/kernel/iovec.c", \
    "sys/kernel/kmalloc.c", \
    "sys/kernel/kmem.c", \
    "sys/kernel/mach.c", \
    "sys/kernel/object.c", \
    "sys/kernel/pci.c", \
    "sys/kernel/pdir.c", \
    "sys/kernel/pframe.c", \
    "sys/kernel/pic.c", \
    "sys/kernel/module.c", \
    "sys/kernel/pit.c", \
    "sys/kernel/pnpbios.c", \
    "sys/kernel/queue.c", \
    "sys/kernel/sched.c", \
    "arch/x86/sys/kernel/trap.c", \
    "arch/x86/sys/kernel/trap.s", \
    "arch/x86/sys/kernel/sched.c", \
    "arch/x86/sys/kernel/sched.s", \
    "arch/x86/sys/kernel/mach.s", \
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
    #"lib/libc/moddb.c", \
    "lib/libc/opts.c", \
    "lib/libc/string.c", \
    "lib/libc/strtol.c", \
    "lib/libc/tcccrt.c", \
    "lib/libc/time.c", \
    "lib/libc/verinfo.c", \
    "lib/libc/vsprintf.c" ]
generator.addTarget(target);


#
# Machina Kernel for x86
#
target = {}
target[FIELD_NAME] = "kernel"
target[FIELD_DESCRIPTION] = "Machina Kernel for x86"
target[FIELD_PREFFIX] = "KERNEL32"
target[FIELD_DEPENDENCIES] = ["kernel-debug"]
target[FIELD_OUTPUT_DIRECTORY] = "build/install/boot"
target[FIELD_OUTPUT_FILE] = "kernel32.so"
target[FIELD_COMMANDS] = [
    "objcopy -O elf32-i386 -j .text -j .rodata -j .bss -j .data" \
    " $(KRNLDBG32_OUT_FILE) $(KERNEL32_OUT_FILE)" ]
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
target[FIELD_DEPENDENCIES] = []
target[FIELD_OUTPUT_DIRECTORY] = "build/install/boot"
target[FIELD_OUTPUT_FILE] = "machina.so"
target[FIELD_OBJECT_DIRECTORY] = "build/machina/obj/machina"
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
#generator.addTarget(target);


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
target[FIELD_DEPENDENCIES] = ["nasm", "build/tools/ar"]
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
#generator.addTarget(target);


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
target[FIELD_SOURCES] = ["arch/x86/boot/boot.asm"]
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
target[FIELD_SOURCES] = ["arch/x86/boot/cdemboot.asm"]
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
target[FIELD_SOURCES] = ["arch/x86/boot/netboot.asm"]
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
target[FIELD_OUTPUT_DIRECTORY] = "build/machina/osloader"
target[FIELD_OUTPUT_FILE] = "osloader-stub.bin"
target[FIELD_OBJECT_DIRECTORY] = "build/machina/obj/osloader"
target[FIELD_SOURCE_DIRECTORY] = "src"
target[FIELD_SOURCES] = \
    ["arch/x86/sys/osloader/stub.asm" ]
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
target[FIELD_LDFLAGS] = "-Wl,-e,start -Wl,-T,src/arch/x86/sys/osloader/osloader.lds -nostdlib" #-Wl,-Ttext,0x90800"
target[FIELD_DEPENDENCIES] = ["nasm", "osloader-stub"]
target[FIELD_OUTPUT_DIRECTORY] = "build/machina/osloader"
target[FIELD_OUTPUT_FILE] = "osloader-main.elf"
target[FIELD_OBJECT_DIRECTORY] = "build/machina/obj/osloader"
target[FIELD_SOURCE_DIRECTORY] = "src"
target[FIELD_SOURCES] = \
    ["arch/x86/sys/osloader/osloader.c", \
    "arch/x86/sys/osloader/kernel.c", \
    "arch/x86/sys/osloader/unzip.c", \
    "lib/libc/vsprintf.c", \
    "lib/libc/string.c", \
    "arch/x86/sys/osloader/bioscall.asm" ]
generator.addTarget(target);


#
# Machina OS Loader
#
target = {}
target[FIELD_NAME] = "osloader"
target[FIELD_DESCRIPTION] = "Machina OS Loader"
target[FIELD_PREFFIX] = "OSLDR"
target[FIELD_DEPENDENCIES] = ["osloader-stub", "osloader-main"]
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
target[FIELD_DEPENDENCIES] = ["cdemboot", "osloader", "mkdfs", "kernel"]
target[FIELD_OUTPUT_DIRECTORY] = "build"
target[FIELD_OUTPUT_FILE] = "machina.iso"
target[FIELD_COMMANDS] = [
    "mkdir -p build/install/dev", \
    "mkdir -p build/install/proc", \
    "build/tools/mkdfs -d build/install/BOOTIMG.BIN -b $(CDEMBOOT_OUT_FILE) -l $(OSLDR_OUT_FILE)" \
    " -k $(KERNEL32_OUT_FILE) -c 1024 -C 1440 -I 8192 -i -f -K rootdev=cd0,rootfs=cdfs", \
    "genisoimage -J -quiet -c BOOTCAT.BIN -b BOOTIMG.BIN -o $(ISO_OUT_FILE) build/install" ]
generator.addTarget(target);

# Driver for NIC 3C905C
target = {}
target[FIELD_NAME] = "nic3C905c"
target[FIELD_DESCRIPTION] = "Machina 3C905C NIC driver for x86"
target[FIELD_PREFFIX] = "LIB3C905C"
target[FIELD_TYPE] = BIN_DYNAMIC
target[FIELD_CFLAGS] = "-I src/include -D KERNEL"
target[FIELD_LDFLAGS] = "-shared -nostdlib -Lbuild/install/boot -lkernel32-dbg"
target[FIELD_OUTPUT_DIRECTORY] = "build/install/sys"
target[FIELD_OUTPUT_FILE] = "lib3c905c.sys"
target[FIELD_OBJECT_DIRECTORY] = "build/machina/obj/dev/3c905c"
target[FIELD_SOURCE_DIRECTORY] = "src"
target[FIELD_SOURCES] = \
   ["sys/dev/3c905c.c", \
   "lib/libs/string.c" ]
generator.addTarget(target);


generator.generateMakefile()
