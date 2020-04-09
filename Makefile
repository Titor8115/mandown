# Makefile

# Copyright (c) 2009, Natacha Porté
# Copyright (c) 2019, Tianze Han
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

DEPDIR=depends

# "Machine-dependant" options
#MFLAGS=-fPIC

CFLAGS=-c -g -O3 -fPIC -Wall -Werror -Wsign-compare -Isrc -Iblender
LDFLAGS=-g -O3 -Wall -Werror
CC=gcc


MANDOWN_SRC=\
	src/markdown.o \
	src/stack.o \
	src/buffer.o \
	src/autolink.o \
	blender/blender.o \
	blender/houdini_blender_e.o \
	blender/houdini_href_e.o

# ifeq ($(UNAME_S),Linux)
# 	LSB_RELEASE := $(shell lsb_release -si 2>/dev/null || echo not)
# 	ifneq ($(filter $(LSB_RELEASE),Debian Ubuntu LinuxMint CrunchBang),)
# 		LDLIBS += -I/usr/include/ncursesw
# 	endif
# endif

all:		mandown blender_blocks

.PHONY:		all clean

# executables

mandown:	cli/mandown.o $(MANDOWN_SRC)
	$(CC) $(LDFLAGS) -lncursesw $^ -o $@

# perfect hashing
blender_blocks: src/blender_blocks.h

src/blender_blocks.h: blender_block_names.txt
	gperf -N find_block_tag -H hash_block_tag -C -c -E --ignore-case $^ > $@


# housekeeping
clean:
	rm -f src/*.o blender/*.o cli/*.o
	rm -rf $(DEPDIR)


# dependencies

include $(wildcard $(DEPDIR)/*.d)


# generic object compilations

%.o:	src/%.c blender/%.c
	@mkdir -p $(DEPDIR)
	@$(CC) -MM $< > $(DEPDIR)/$*.d
	$(CC) $(CFLAGS) -o $@ $<

cli.o:	cli/%.c 
	@mkdir -p $(DEPDIR)
	@$(CC) -MM $< > $(DEPDIR)/$*.d
	$(CC) $(CFLAGS) -I/usr/include/ncursesw -o $@ $<

