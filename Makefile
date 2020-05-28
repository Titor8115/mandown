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
UNAME_S := $(shell uname -s 2>/dev/null || echo not)
CURSES = ncursesw
OSFLAGS = -Wl,--copy-dt-needed-entries

ifeq ($(UNAME_S),Darwin)
	CURSES := ncurses
	OSFLAGS :=
endif

CFLAGS = -c -g -O3 -Wall -Wsign-compare -Iparser -Iblender -Iinclude -I/usr/include/libxml2
LDFLAGS = -g -O3 $(OSFLAGS) -l$(CURSES) -lxml2 -Wall -Werror
CC = gcc

MANDOWN_SRC=\
	src/mandown.o \
	src/view.o \
	parser/markdown.o \
	parser/stack.o \
	parser/buffer.o \
	parser/autolink.o \
	blender/blender.o \
	blender/houdini_blender_e.o \
	blender/houdini_href_e.o

all:		mandown

.PHONY:		all clean

# executables

mandown:	$(MANDOWN_SRC)
	$(CC) $^ -o $@ $(LDFLAGS)

# perfect hashing
blender_blocks: parser/blender_blocks.h

parser/blender_blocks.h: blender_block_names.txt
	gperf -N find_block_tag -H hash_block_tag -C -c -E --ignore-case $^ > $@


# housekeeping
clean:
	rm -f ./mandown
	rm -f parser/*.o blender/*.o src/*.o
	rm -rf $(DEPDIR)

# dependencies
include $(wildcard $(DEPDIR)/*.d)


# generic object compilations
%.o:	src/%.c parser/%.c blender/%.c
	@mkdir -p $(DEPDIR)
	@$(CC) -MM $< > $(DEPDIR)/$*.d
	$(CC) $(CFLAGS) -o $@ $<

