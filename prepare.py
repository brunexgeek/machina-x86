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

class MakefileGenerator:

	def toVar(self, name):
		return "$(" + name + ")"

	def toObject(self, fileName):
		return re.sub("\.c$", ".o", fileName)

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

		# print the target to compile each source file
		print self.toVar(TARGET_OBJ_DIR) + "/%.o:", self.toVar(TARGET_SRC_DIR) + "/%.c"
		if (target[FIELD_PLATFORM] == "machina"):
			compiler = "$(TCC)"
		else:
			compiler = "$(CC)"
		print "\t" + compiler, "-I", \
			self.toVar(TARGET_SRC_DIR), \
			self.toVar(TARGET_CFLAGS), \
			"-c $< -o $@"
		print

		# print the target to compile the binary file
		if (FIELD_NAME in target):
			mainTarget = target[FIELD_NAME]
		else:
			mainTarget = self.toVar(TARGET_OUT_FILE)
		print mainTarget + ":", TARGET_WELCOME, self.toVar(TARGET_OBJ_FILES)
		print "\t@mkdir -p", self.toVar(TARGET_OUT_DIR)
		if (target[FIELD_PLATFORM] == "machina"):
			compiler = "$(TCC)"
		else:
			compiler = "$(CC)"
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
		print "CFLAGS:=$(CFLAGS) -O2 -m32 -Wimplicit"
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
# This script generate the Machina's makefile.
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
target[FIELD_DESCRIPTION] = "Native Tiny C Compiler"
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
target[FIELD_DESCRIPTION] = "Native NASM x86 Assembler"
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
target[FIELD_NAME] = target[FIELD_PREFFIX] + "mmm"
target[FIELD_DESCRIPTION] = "Native MKDFS"
TARGETS["native_mkdfs"] = target

generator = MakefileGenerator()
generator.generateMakefile(TARGETS)
