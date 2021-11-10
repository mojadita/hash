# Makefile -- makefile for the hash table library.
# Author: Luis Colorado <luiscoloradourcola@gmail.com>
# Date: Wed Nov 10 15:10:29 EET 2021
# Copyright: (C) 2021 Luis Colorado.  All rights reserved.
# License: BSD.

SONAME	= 1.0
VERSION = $(SONAME).1
RM		?= rm -f

targets = test_hash hash.so hash.so.$(SONAME) hash.so.$(VERSION)
toclean = $(targets)

all: $(targets)
clean:
	$(RM) $(toclean)
depend:
	mkdep *.c

.SUFFIXES: .pico

test_hash_objs = test_hash.o hash.so
toclean += $(test_hash_objs)

hash.so.$(VERSION)_objs = hash.pico
toclean += $(hash.so.$(VERSION)_objs)

hash.so: hash.so.$(SONAME)
	ln -s hash.so.$(SONAME) $@

hash.so.$(SONAME): hash.so.$(VERSION)
	ln -s hash.so.$(VERSION) hash.so.$(SONAME)

hash.so.$(VERSION): $(hash.so.$(VERSION)_objs)
	$(CC) $(CFLAGS) -o $@ -fPIC -shared -Wl,-soname=hash.so.$(SONAME) $(hash.so.$(VERSION)_objs)

test_hash: $(test_hash_objs)
	$(CC) $(CFLAGS) -o $@  $(test_hash_objs)

.c.pico:
	$(CC) $(CFLAGS) -c -fPIC -o $@ $<
