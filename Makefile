# Makefile -- makefile for the hash table library.
# Author: Luis Colorado <luiscoloradourcola@gmail.com>
# Date: Wed Nov 10 15:10:29 EET 2021
# Copyright: (C) 2021 Luis Colorado.  All rights reserved.
# License: BSD.

MAJOR         = 2  # must change this if API changes
MINOR         != git rev-parse HEAD | cut -c 1-10

DEVNAME       = libhash.so
SO_NAME	      = $(DEVNAME).$(MAJOR)
LIBNAME       = $(SO_NAME).$(MINOR)
toclean      += $(DEVNAME) $(SO_NAME) $(LIBNAME)

RM		     ?= rm -f

prefix       ?= /usr/local
exec_prefix  ?= $(prefix)
bindir       ?= $(exec_prefix)/bin
sbindir      ?= $(exec_prefix)/sbin
libdir       ?= $(exec_prefix)/lib
includedir   ?= $(prefix)/include
datarootdir  ?= $(prefix)/share
datadir      ?= $(datarootdir)/$(PACKAGE)
mandir       ?= $(datarootdir)/man
infodir      ?= $(datarootdir)/info
docdir       ?= $(datarootdir)/doc/$(PACKAGE)
man3dir      ?= $(mandir)/man3

OWN          ?= root
GRP          ?= bin
DMOD         ?= 755
FMOD         ?= 644
XMOD         ?= 755
LMOD         ?= 644

targets       = test_hash
toclean      += $(targets)

.PHONY: all clean uninstall

all: $(targets)

clean:
	$(RM) $(toclean)

.depend:
	mkdep $(SRCS)

$(libdir)/$(DEVNAME): $(libdir)/$(SO_NAME)
	-ln -sf $(SO_NAME) $(libdir)/$(DEVNAME)
$(libdir)/$(SO_NAME): $(libdir)/$(LIBNAME)
	-ln -sf $(LIBNAME) $(libdir)/$(SO_NAME)

$(libdir)/$(LIBNAME): $(libdir) $(LIBNAME)
	-$(INSTALL) $(IFLAGS) -o $(OWN) -g $(GRP) -m $(DMOD) $(LIBNAME) $(libdir)

$(libdir):
	-$(INSTALL) $(IFLAGS) -o $(OWN) -g $(GRP) -m $(DMOD) -d $(libdir)
to_uninstall  += $(libdir)/$(DEVNAME) $(libdir)/$(SO_NAME) $(Libdir)/$(LIBNAME) 
to_rmdir      += $(libdir)

install: $(libdir)/$(DEVNAME)

uninstall:
	-$(RM) $(to_uninstall)
	-rmdir $(to_rmdir)

.SUFFIXES: .pico

test_hash_deps = $(libdir)/$(DEVNAME)
test_hash_objs =  test_hash.o
test_hash_ldfl = -L$(libdir)
test_hash_libs = -lhash

toclean += $(test_hash_objs)

OBJS    += $(test_hash_objs)

libhash_deps =
libhash_objs = hash.pico
libhash_ldfl =
libhash_libs =

OBJS    += $(libhash_objs)


toclean += $(libhash_objs)

SRCS    := $(OBJS:.o=.c)
SRCS	:= $(SRCS:.pico=.c)

$(DEVNAME): $(SO_NAME)
	ln -sf $(SO_NAME) $(DEVNAME)

$(SO_NAME): $(LIBNAME)
	ln -sf $(LIBNAME) $(SO_NAME)

$(LIBNAME): $(libhash_deps) $(libhash_objs)
	$(CC) $(LDFLAGS) -o $@ -shared -Wl,-soname=$(SO_NAME) \
	    $(libhash_ldfl) $(libhash_objs) $(libhash_libs) $(LIBS)

test_hash: $(test_hash_deps) $(test_hash_objs)
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $($@_ldfl) \
	    $($@_objs) $($@_libs) $(LIBS)

.c.pico:
	$(CC) $(CFLAGS) -fPIC -DPIC -o $@ -c $<

.cc.pico:
	$(CXX) $(CXXFLAGS) -fPIC -DPIC -o $@ -c $<
