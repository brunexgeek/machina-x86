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

COLOR_BLUE="$(COLOR_BLUE)"
COLOR_RESET="$(COLOR_RESET)"


BIN_DYNAMIC = 1
BIN_STATIC = 2
BIN_EXECUTABLE = 4

LANG_C = 1
LANG_S = 2
LANG_ASM = 4

class MakefileTarget:

    def __init__(self, targetDefs, otherTargets):
        self.checkFields(targetDefs)
        self.target = targetDefs
        self.otherTargets = otherTargets

    @staticmethod
    def println( text ):
        print "\t@echo -e '" + text + "'"

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
        # target header
        print "\n#\n#", self.target[FIELD_DESCRIPTION], "\n#"

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
            self.println(COLOR_BLUE + "Compiling $< " + COLOR_RESET)
            print "\t" + compiler, self._toVar("CFLAGS"), "-DTARGET_MACHINE=$(TARGET_MACHINE)", "-c $< -o $@"
            print
        # target to compile each S source file
        if ((langs & LANG_S) > 0):
            print self._toVar("OBJ_DIR") + "/%.s.o:", self._toVar("SRC_DIR") + "/%.s"
            self.println(COLOR_BLUE + "Compiling $<" + COLOR_RESET)
            print "\t" + compiler, "-x assembler-with-cpp", self._toVar("CFLAGS"), "-c $< -o $@"
            print
        # target to compile each ASM source file
        if ((langs & LANG_ASM) > 0):
            print self._toVar("OBJ_DIR") + "/%.asm.o:", self._toVar("SRC_DIR") + "/%.asm"
            self.println(COLOR_BLUE + "Compiling $<" + COLOR_RESET)
            print "\t$(NASM)", self._toVar("NFLAGS"), "$< -o $@"
            print

        # target to clean de compilation
        print self._toVarName("CLEAN"), ":"
        print "\trm -f", self._toVar("OBJ_FILES"), self._toVar("OUT_FILE")
        print

        # choose the main target name
        mainTarget = self._toVar("OUT_FILE")
        if (FIELD_NAME in self.target):
            mainTarget += " " + self.target[FIELD_NAME]

        # find the dependencies
        depends = ""
        if (FIELD_DEPENDENCIES in self.target):
            for entry in self.target[FIELD_DEPENDENCIES]:
                if (entry in self.otherTargets and FIELD_OUTPUT_FILE in self.otherTargets[entry]):
                    entry = self.otherTargets[entry][FIELD_OUTPUT_DIRECTORY] + '/' + self.otherTargets[entry][FIELD_OUTPUT_FILE]
                depends += entry + " "

        # target to compile the binary file
        print mainTarget, ":", depends, self._toVar("OBJ_FILES")
        self.println(COLOR_BLUE + "Building " + self.target[FIELD_DESCRIPTION] + COLOR_RESET)
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
        # target header
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

        # target to clean de compilation
        print self._toVarName("CLEAN"), ":"
        print "\trm -f", self._toVar("OBJ_FILES"), self._toVar("OUT_FILE")
        print

        # choose the main target name
        mainTarget = self._toVar("OUT_FILE")
        if (FIELD_NAME in self.target):
            mainTarget += " " + self.target[FIELD_NAME]

        # find the dependencies
        depends = ""
        if (FIELD_DEPENDENCIES in self.target):
            for entry in self.target[FIELD_DEPENDENCIES]:
                if (entry in self.otherTargets and FIELD_OUTPUT_FILE in self.otherTargets[entry]):
                    entry = self.otherTargets[entry][FIELD_OUTPUT_DIRECTORY] + '/' + self.otherTargets[entry][FIELD_OUTPUT_FILE]
                depends += entry + " "

        # target to compile the binary file
        print mainTarget, ":", depends
        self.println(COLOR_BLUE + "Building " + self.target[FIELD_DESCRIPTION] + COLOR_RESET)
        print "\t@mkdir -p", self._toVar("OUT_DIR")

        # check if the current target is for BIN_DYNAMIC or BIN_EXECUTABLE
        print "\t$(NASM)", \
            self._toVar("NFLAGS"), \
            self._toVar("SRC_FILES") , \
            "-o", self._toVar("OUT_FILE")
        print



class CustomMakefileTarget(MakefileTarget):

    def __init__(self, targetDefs, otherTargets):
        self.target = targetDefs
        self.otherTargets = otherTargets

    @staticmethod
    def isCompatible(targetDefs):
        return (FIELD_SOURCES not in targetDefs and FIELD_COMMANDS in targetDefs)

    #
    # Generate the NASM Makefile target
    #
    def generateTarget(self):
        # target header
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
                if (entry in self.otherTargets and FIELD_OUTPUT_FILE in self.otherTargets[entry]):
                    entry = self.otherTargets[entry][FIELD_OUTPUT_DIRECTORY] + '/' + self.otherTargets[entry][FIELD_OUTPUT_FILE]
                depends += entry + " "

        # target to compile the binary file
        print mainTarget, ":", depends
        self.println(COLOR_BLUE + "Building " + self.target[FIELD_DESCRIPTION] + COLOR_RESET)
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

        # include the code for interactive shell detection
        print "INTERACTIVE:=$(shell [ -t 0 ] && echo 1)"
        print """
            ifdef INTERACTIVE
                COLOR_BLUE=\\x1b[34;1m
                COLOR_RESET=\\x1b[0m
            else
                COLOR_BLUE=\\#\\#\\#
            endif"""
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
                temp = AsmMakefileTarget(current, self.targets)
            else:
                if (CMakefileTarget.isCompatible(current)):
                    temp = CMakefileTarget(current, self.targets)
                else:
                    if (CustomMakefileTarget.isCompatible(current)):
                        temp = CustomMakefileTarget(current, self.targets)
                    else:
                        raise RuntimeError("Invalid Makefile target")
            temp.generateTarget()
        # create the "all" target
        print "all:",
        for key, current in self.targets.iteritems():
            print MakefileTarget.toVar("OUT_FILE", current),
        print "\n"
        # create the "clean" target
        print "clean:",
        for key, current in self.targets.iteritems():
            if FIELD_COMMANDS not in current:
                print MakefileTarget.toVarName("CLEAN", current),
        print "\n"
