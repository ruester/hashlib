#    This file is part of hashlib.
#
#    Copyright 2012 Matthias Ruester
#
#    hashlib is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    hashlib is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with hashlib; if not, see <http://www.gnu.org/licenses>.

LIBRARY  = hashlib
VERSION  = 0
REVISION = 0

# compiling and linking
CC           = gcc
CFLAGS       = -Wall -Wextra -g -fpic -O3
LDFLAGS      =
TEST_CFLAGS  = -Wall -Wextra -g
TEST_LDFLAGS =

LIBNAME   = lib$(LIBRARY)
SOFILE    = $(LIBNAME).so
SONAME    = $(SOFILE).$(VERSION)
SOVERSION = $(SONAME).$(REVISION)

OBJECTS = $(LIBRARY).o

LDFLAGS_SO = -shared -fpic -lc -Wl,-soname,$(SONAME)

TEST_SRC     = test.c
TEST_OBJECT  = test.o
TEST_PROGRAM = test

# installing
DESTDIR    =
PREFIX     = /usr
LIBDIR     = $(PREFIX)/lib
INCLUDEDIR = $(PREFIX)/include
MANSECTION = 3
MANDIR     = $(PREFIX)/man/man$(MANSECTION)
MANPAGE    = $(LIBRARY).$(MANSECTION)

.PHONY: test install check shared all clean

all: shared

shared: $(SOVERSION)

$(SOVERSION): $(OBJECTS)
	$(CC) $(LDFLAGS_SO) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^

check: test

test: all
	ln -fs $(SOVERSION) $(SONAME)
	ln -fs $(SONAME) $(SOFILE)
	$(CC) -c $(TEST_SRC) $(TEST_CFLAGS)
	$(CC) -o $(TEST_PROGRAM) $(TEST_OBJECT) -Wl,-rpath,. -L. -l$(LIBRARY) $(TEST_LDFLAGS)
	@./$(TEST_PROGRAM)

install: all
	mkdir -p $(DESTDIR)$(LIBDIR)
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	mkdir -p $(DESTDIR)$(MANDIR)
	cp -f $(SOVERSION) $(DESTDIR)$(LIBDIR)
	cp -f $(LIBRARY).h $(DESTDIR)$(INCLUDEDIR)
	cp -f $(MANPAGE) $(DESTDIR)$(MANDIR)
	ln -fs $(SOVERSION) $(DESTDIR)$(LIBDIR)/$(SONAME)
	ln -fs $(SONAME) $(DESTDIR)$(LIBDIR)/$(SOFILE)

clean:
	rm -f $(OBJECTS)
	rm -f $(TEST_PROGRAM) $(TEST_OBJECT)
	rm -f $(SOVERSION) $(SONAME) $(SOFILE)
