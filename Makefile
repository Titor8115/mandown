# Makefile

# Copyright (c) 2009, Natacha Porté
# Copyright (c) 2019, Tianze Han
# Copyright (c) 2020, Emil Renner Berthing
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

MAKEFLAGS += -rR

TARGET = mdn
O = build

UNAME_S := $(shell uname -s 2>/dev/null || echo not)

PREFIX ?= /usr/local
DESTDIR =

# autoconf compatible variables
prefix      = $(PREFIX)
exec_prefix = $(prefix)
bindir      = $(exec_prefix)/bin

# tools
CC         = gcc -std=gnu11
LD         = gcc
GPERF      = gperf
MKDIR_P    = mkdir -p
RM_RF      = rm -rf
INSTALL    = install
# use make PKG_CONFIG=pkg-config to use pkg-config
#PKG_CONFIG = pkg-config

# flags
OPT       = -O3
DBGSYMS   = -g
WARNINGS  = -Wall -Wextra -Wpointer-arith -Wundef -Wno-unused-parameter
DEPENDS   = -MMD -MP
INCLUDES  = -Iparser -Iblender
DEFINES   =
CFLAGS    = $(OPT) -pipe $(DBGSYMS) $(WARNINGS) $(DEPENDS) $(INCLUDES) $(DEFINES)
CFLAGS   += $(NCURSES_CFLAGS) $(XML2_CFLAGS)
LDFLAGS   = $(OPT) $(DBGSYMS)
LIBS      = $(NCURSES_LIBS) $(XML2_LIBS)

# OS-specific additions
ifeq ($(UNAME_S),Darwin)
NCURSES   = ncurses
else
NCURSES   = ncurses
LDFLAGS  += -Wl,--copy-dt-needed-entries
endif

# libraries
NCURSES_CFLAGS := $(if $(PKG_CONFIG),$(shell $(PKG_CONFIG) --cflags $(NCURSES)))
NCURSES_LIBS   := $(if $(PKG_CONFIG),$(shell $(PKG_CONFIG) --libs $(NCURSES)),-l$(NCURSES))

XML2            = libxml-2.0
XML2_CFLAGS    := $(if $(PKG_CONFIG),$(shell $(PKG_CONFIG) --cflags $(XML2)),-I/usr/include/libxml2)
XML2_LIBS      := $(if $(PKG_CONFIG),$(shell $(PKG_CONFIG) --libs $(XML2)),-lxml2)

# sources
SOURCES := $(sort $(wildcard src/*.c blender/*.c parser/*.c))
OBJECTS  = $(SOURCES:%.c=$O/%.o)

# use make V=1 to see the raw commands or make -s for silence
ifeq (,$V$(findstring s,$(word 1,$(MAKEFLAGS))))
E := @echo
Q := @
else
E := @:
Q :=
endif

.SECONDEXPANSION:
.PRECIOUS: $O/%/ $O/%
.PHONY: all clean install uninstall

all: $(TARGET)

# executables
$(TARGET): $(OBJECTS)
	$E '  LD    $@'
	$Q$(LD) -o $@ $(LDFLAGS) $^ $(LIBS)

# perfect hashing
blender_blocks: parser/blender_blocks.h

parser/blender_blocks.h: blender_block_names.txt
	$E '  GPERF $@'
	$Q$(GPERF) -N find_block_tag -H hash_block_tag -C -c -E --ignore-case $^ > $@

$O/parser/markdown.o: parser/blender_blocks.h

# housekeeping
clean:
	$E '  RM    $O $(TARGET)'
	$Q$(RM_RF) $O $(TARGET)

install: $(TARGET)
	$(INSTALL) -dm755 $(DESTDIR)$(bindir)
	$(INSTALL) -m755 $(TARGET) $(DESTDIR)$(bindir)/$(TARGET)

uninstall:
	$(RM_RF) $(DESTDIR)$(bindir)/$(TARGET)

# automatic dependencies
-include $O/*.d $O/*/*.d

# generic object compilations
$O/%.o:	%.c | $$(@D)/
	$E '  CC    $<'
	$Q$(CC) -o $@ $(CFLAGS) -c $<

# generic build directory creation
$O/%/:
	$E '  MKDIR $@'
	$Q$(MKDIR_P) $@