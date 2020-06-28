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

SOURCES = $(sort $(wildcard src/*.c blender/*.c parser/*.c))
OBJECTS = $(SOURCES:.c=.o)
PREFIX ?= /usr/local
TARGET = mdn
DESTDIR =

CURSES = ncursesw
OSFLAGS = -Wl,--copy-dt-needed-entries
DEFINE = -D HAS_NCURSESW_H

ifeq ($(UNAME_S),Darwin)
	CURSES := ncurses
	OSFLAGS :=
	DEFINE := -D HAS_NCURSES_H
endif

LDLIB = -l$(CURSES) -lxml2
CFLAGS = -c -g -Wall -Wsign-compare -Iparser -Iblender -I/usr/include/libxml2 $(DEFINE)
LDFLAGS = -g -O3 $(OSFLAGS) -Wall -Werror
# CC = gcc

SRC=\
	src/mandown.o \
	src/view.o \
	src/dom.o \
	parser/markdown.o \
	parser/stack.o \
	parser/buffer.o \
	parser/autolink.o \
	blender/blender.o \
	blender/houdini_blender_e.o \
	blender/houdini_href_e.o

all:		$(TARGET)

.PHONY:		all clean install uninstall

# executables

$(TARGET):	$(SRC)
	$(CC) $^ -o $@ $(LDLIB) $(LDFLAGS)


# perfect hashing
blender_blocks: parser/blender_blocks.h

parser/blender_blocks.h: blender_block_names.txt
	gperf -N find_block_tag -H hash_block_tag -C -c -E --ignore-case $^ > $@


# housekeeping
clean:
	$(RM) $(TARGET)
	$(RM) $(OBJECTS)

install: all
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)

uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/bin/$(TARGET)

# dependencies
include $(wildcard $(DEPDIR)/*.d)


# generic object compilations
%.o:	src/%.c parser/%.c blender/%.c
	@mkdir -p $(DEPDIR)
	@$(CC) -MM $< > $(DEPDIR)/$*.d
	$(CC) $(CFLAGS) -o $@ $<

