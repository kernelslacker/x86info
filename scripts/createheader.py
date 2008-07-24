#!/usr/bin/python

# Author: Andreas Herrmann <andreas.herrmann3@amd.com>
#
# Copyright (C) 2008 Advanced Micro Devices, Inc.
#
# Licensed under the terms of the GNU GENERAL PUBLIC LICENSE version 2.
# See file COPYING for details.


# This script creates C header files from some register
# description files.
#
# The general format for those register descriptions is
#
#	{<name>=<address>;[<description>]
#	[<field name>]:<bit width>[;<value>=<string>|]
#	[<field name>]:<bit width>[;<value>=<string>|]
#	...
#	[<field name>]:<bit width>[;<value>=<string>|]
#       }
#
# If no field name is provided the field is reserved and might
# not be displayed with the msr viewing tool. The optional <value>=<string>
# data is for bit field decoding but it's currently not yet supported.

# Todos/Nice-to-have:
# - comments in this script (?)
# - create information for bit field decoding
# - warn if names contain whitespace characters
# - add include/insert statement to avoid dup definitions of common register
# - allow annotations for registers (e.g. write-only)
# - improve command line handling

import re
import sys
import os

progname=os.path.basename(sys.argv[0])

if len(sys.argv) < 3:
    print progname + ": invalid number of argumenst"
    print "\nUSAGE:"
    print progname + " <input file> <struct name>"
    print "\nEXAMPLE:"
    print progname + " k8regs.txt k8 >k8.h"
    sys.exit(1)

f=open(sys.argv[1])
family=sys.argv[2]

print \
"/*\n"\
" * Licensed under the terms of the GNU GENERAL PUBLIC LICENSE version 2.\n"\
" * See file COPYING for details.\n"\
" */\n"
print "#ifndef " + family + "_h"
print "#define " + family + "_h\n"
if (re.search("/", sys.argv[1])):
    print "#include \"../msr.h\"\n"
else:
    print "#include \"msr.h\"\n"
struct=[]
for line in f:
    if (re.search("^#",line)):
        continue
    if (re.search("^{",line)):
        start=1; end=0
        tmp=line.split("{")[1]
        tmp=tmp.split("=")
        reg_name=tmp[0].strip()
        tmp=tmp[1].split(";")
        reg_addr=tmp[0].strip()
        if len(tmp) > 1:
            reg_desc=tmp[1].strip()
        else:
            reg_desc=""
        spec=[]
    if (re.search("^}",line)):
        end=1
        str1="_RANGE(" + family + "_" + reg_name
        str2="_NAMES(" + family + "_" + reg_name
        for e in spec:
            if e[0]=="":
                str2 += ",0"
            else:
                str2 += ",\"" + e[0] +"\""
            str1 += "," + e[1]
        str1 += ",0);"
        str2 += ");"
        print str1
        print str2
        str3 = "_SPEC(" + reg_addr + ", " + reg_name + ", " 
        if reg_desc != "":
            str3 += "\"" + reg_desc + "\", "
        else:
             str3 += "NULL, "
        str3 +=  family + "_)"
        struct.append(str3)
    if (re.search(":", line)):
        tmp=line.split(";")
        field=tmp[0].split(":")
        fname=field[0].strip()
        fwidth=field[1].strip()
        spec.append([fname, fwidth])

print "\nstruct reg_spec " + family + "_spec [] = {"
for i in struct:
    print "\t" + i + ","
print "	{0, NULL, NULL, NULL, NULL},"
print "};\n"
print "#endif /* " + family + "_h */"
f.close()
