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
DISTRIB_ID := $(shell cat /etc/*-release 2>/dev/null | grep -oP "(?<=DISTRIB_ID=).*" || echo not)

PREFIX ?= /usr/local
DESTDIR =
CONFIGDIR = ~/.config/mdn

# autoconf compatible variables
prefix      = $(PREFIX)
exec_prefix = $(prefix)
bindir      = $(exec_prefix)/bin

# tools
CC         = gcc
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
CFLAGS   += $(CURSES_CFLAGS) $(XML2_CFLAGS) $(CONFIG_CFLAGS)
LDFLAGS   = $(OPT) $(DBGSYMS)
LIBS      = $(CURSES_LIBS) $(XML2_LIBS) $(CONFIG_LIBS)

# OS-specific additions
ifeq ($(UNAME_S),Linux)
	ifeq ($(shell ldconfig -p | grep libncursesw),)
		CURSES   = ncurses
	else
		CURSES   = ncursesw
	endif
	ifeq ($(DISTRIB_ID),Gentoo)
		LDFLAGS  += -Wl,--copy-dt-needed-entries
	endif
else ifeq ($(UNAME_S),Darwin)
	CURSES   = ncurses
else
CURSES   = ncurses
endif

# libraries
CURSES_CFLAGS := $(if $(PKG_CONFIG),$(shell $(PKG_CONFIG) --cflags $(CURSES)))
CURSES_LIBS   := $(if $(PKG_CONFIG),$(shell $(PKG_CONFIG) --libs $(CURSES)),-l$(CURSES))

XML2            = libxml-2.0
XML2_CFLAGS    := $(if $(PKG_CONFIG),$(shell $(PKG_CONFIG) --cflags $(XML2)),-I/usr/include/libxml2)
XML2_LIBS      := $(if $(PKG_CONFIG),$(shell $(PKG_CONFIG) --libs $(XML2)),-lxml2)

CONFIG_CFLAGS := $(if $(PKG_CONFIG),$(shell $(PKG_CONFIG) --cflags libconfig))
CONFIG_LIBS   := $(if $(PKG_CONFIG),$(shell $(PKG_CONFIG) --libs libconfig),-lconfig)

# sources
SOURCES := $(sort $(wildcard src/*.c blender/*.c parser/*.c))
OBJECTS  = $(SOURCES:%.c=$O/%.o)

.SECONDEXPANSION:
.PRECIOUS: $O/%
.PHONY: all clean install uninstall

all: $(TARGET)

# executables
$(TARGET): $(OBJECTS)
	@$(MKDIR_P) $(CONFIGDIR)
	@echo "formula  " $@
	@$(LD) -o $@ $(LDFLAGS) $^ $(LIBS)

# perfect hashing
blender_blocks: parser/blender_blocks.h

parser/blender_blocks.h: blender_block_names.txt
	@echo Gperf $@
	@$(GPERF) -N find_block_tag -H hash_block_tag -C -c -E --ignore-case $^ > $@

$O/parser/markdown.o: parser/blender_blocks.h

# housekeeping
clean:
	@echo "Remove " $O $(TARGET)
	@$(RM_RF) $O $(TARGET)

install: $(TARGET)
	$(INSTALL) -dm755 $(DESTDIR)$(bindir)
	$(INSTALL) -m755 $(TARGET) $(DESTDIR)$(bindir)/$(TARGET)

uninstall:
	$(RM_RF) $(DESTDIR)$(bindir)/$(TARGET)
	$(RM_RF) $(CONFIGDIR)

# automatic dependencies
-include $(wildcard $($O)/*.d)

# generic object compilations
$O/%.o:	%.c
	@echo "Create   " $(@D)
	@$(MKDIR_P) $(@D)
	@echo "Compile  " $<
	@$(CC) -o $@ $(CFLAGS) -c $<

# generic build directory creation
