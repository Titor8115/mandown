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
BASENAME = mandown
LIB_A = lib$(BASENAME).a
LIB_SO = lib$(BASENAME).$(SOEXT)

O = build

UNAME_S := $(shell uname -s 2>/dev/null || echo not)

PREFIX ?= /usr/local
DESTDIR =
CONFIGDIR = $(if $(XDG_CONFIG_HOME),$(XDG_CONFIG_HOME)/mdn,$(HOME)/.config/mdn)

# autoconf compatible variables
prefix      = $(PREFIX)
exec_prefix = $(prefix)
bindir      = $(exec_prefix)/bin
incdir      = $(prefix)/include
libdir      = $(exec_prefix)/lib

# tools
CC         = gcc
LD         = gcc
GPERF      = gperf
MKDIR_P    = mkdir -p
RM_RF      = rm -rf
INSTALL    = install
# use make PKG_CONFIG=pkg-config to use pkg-config
PKG_CONFIG = pkg-config

# flags
OPT       = -O3
DBGSYMS   = -g
WARNINGS  = -Wall -Wextra -Wpointer-arith -Wundef -Wno-unused-parameter
DEPENDS   = -MMD -MP
INCLUDES  = -Iparser -Iblender -Isrc
DEFINES   =
CFLAGS    = $(OPT) -fPIC -pipe $(DBGSYMS) $(WARNINGS) $(DEPENDS) $(INCLUDES) $(DEFINES)
CFLAGS   += $(CURSES_CFLAGS) $(XML2_CFLAGS) $(CONFIG_CFLAGS)
LDFLAGS   = $(OPT) $(DBGSYMS)
LIBS      = $(CURSES_LIBS) $(XML2_LIBS) $(CONFIG_LIBS)

# OS-specific additions
ifeq ($(UNAME_S),Linux)
	ifeq ($(shell ldconfig -p | grep libncursesw),)
		CURSES_PKG   = ncurses
	else
		CURSES_PKG   = ncursesw
		DEFINES 	+= -DWIDE_NCURSES
	endif
	SOEXT    = so
	SHLFLAGS = -shared
else ifeq ($(UNAME_S),Darwin)
	CURSES_PKG   = ncurses
	SOEXT    	 = dylib
	SHLFLAGS 	 = -dynamiclib
else
CURSES_PKG   = ncurses
endif

# libraries
CURSES_CFLAGS := $(if $(PKG_CONFIG),$(shell $(PKG_CONFIG) --cflags $(CURSES_PKG)))
CURSES_LIBS   := $(if $(PKG_CONFIG),$(shell $(PKG_CONFIG) --libs $(CURSES_PKG)),-l$(CURSES_PKG))

XML2_PKG       = libxml-2.0
XML2_CFLAGS   := $(if $(PKG_CONFIG),$(shell $(PKG_CONFIG) --cflags $(XML2_PKG)),-I/usr/include/libxml2)
XML2_LIBS     := $(if $(PKG_CONFIG),$(shell $(PKG_CONFIG) --libs $(XML2_PKG)),-lxml2)

CONFIG_PKG	   = libconfig
CONFIG_CFLAGS := $(if $(PKG_CONFIG),$(shell $(PKG_CONFIG) --cflags $(CONFIG_PKG)))
CONFIG_LIBS   := $(if $(PKG_CONFIG),$(shell $(PKG_CONFIG) --libs $(CONFIG_PKG)),-lconfig)

# sources
LIB_SOURCES := $(sort $(wildcard src/*.c blender/*.c parser/*.c))
CLI_SOURCES := $(wildcard cli/*.c)
SOURCES     := $(LIB_SOURCES) $(CLI_SOURCES)
LIB_OBJECTS  = $(LIB_SOURCES:%.c=$O/%.o)
CLI_OBJECTS  = $(CLI_SOURCES:%.c=$O/%.o)
OBJECTS      = $(SOURCES:%.c=$O/%.o)

.SECONDEXPANSION:
.PRECIOUS: $O/%
.PHONY: all debug clean install uninstall

all: $(TARGET)
debug: DEFINES += -DDEBUG
debug: $(TARGET)
# executables
$(TARGET): $(CLI_OBJECTS) $(LIB_A)
	@$(MKDIR_P) $(CONFIGDIR)
	@echo
	@echo "Formula:    " $@
	@echo "Config dir: " $(CONFIGDIR)
	@$(LD) -o $@ $(LDFLAGS) $^ $(LIBS)

$(TARGET)_shlib: $(CLI_OBJECTS) $(LIB_SO)
	@$(LD) -o $@ $(CLI_OBJECTS) -L. -lmandown $(LIBS)

# libs
$(LIB_A): $(LIB_OBJECTS)
	ar r $@ $^

$(LIB_SO): $(LIB_OBJECTS)
ifeq ($(SOEXT), "")
	$(error "Unable to build shared lib on $(UNAME_S))
else
	$(CC) $(SHLFLAGS) -o $@ $^ $(LIBS)
endif

# perfect hashing
blender_blocks: parser/blender_blocks.h

parser/blender_blocks.h: blender_block_names.txt
	@echo Gperf $@
	@$(GPERF) -N find_block_tag -H hash_block_tag -C -c -E --ignore-case $^ > $@

$O/parser/markdown.o: parser/blender_blocks.h

# housekeeping
clean:
	@echo "Remove:   " $O $(TARGET) $(LIB_A) $(LIB_SO)
	@$(RM_RF) $O $(TARGET) $(LIB_A) $(LIB_SO)

install: $(TARGET) $(LIB_A) $(LIB_SO)
	$(INSTALL) -dm755 $(DESTDIR)$(bindir) $(DESTDIR)$(incdir) $(DESTDIR)$(libdir)
	$(INSTALL) -m755 $(TARGET) $(DESTDIR)$(bindir)/$(TARGET)
	$(INSTALL) -m644 src/$(BASENAME).h $(DESTDIR)$(incdir)/$(BASENAME).h
	$(INSTALL) -m644 $(LIB_A) $(LIB_SO) $(DESTDIR)$(libdir)

uninstall:
	$(RM_RF) $(DESTDIR)$(bindir)/$(TARGET)
	$(RM_RF) $(DESTDIR)$(incdir)/$(BASENAME).h
	$(RM_RF) $(DESTDIR)$(libdir)/$(LIB_A) $(DESTDIR)$(libdir)/$(LIB_SO)
	$(RM_RF) $(CONFIGDIR)

# automatic dependencies
-include $(wildcard $($O)/*.d)

# generic object compilations
$O/%.o:	%.c
	@$(MKDIR_P) $(@D)
	@echo "Compiling:" $<
	@$(CC) -o $@ $(CFLAGS) -c $<

# generic build directory creation
