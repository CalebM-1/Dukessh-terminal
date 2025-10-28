#
# Simple Makefile
# Mike Lam, James Madison University, August 2016
#
# This makefile builds a simple application that contains a main module
# (specified by the EXE variable) and a predefined list of additional modules
# (specified by the MODS variable). If there are any external library
# dependencies (e.g., the math library, "-lm"), list them in the LIBS variable.
# If there are any precompiled object files, list them in the OBJS variable.
#
# By default, this makefile will build the project with debugging symbols and
# without optimization. To change this, edit or remove the "-g" and "-O0"
# options in CFLAGS and LDFLAGS accordingly.
#
# By default, this makefile build the application using the GNU C compiler,
# adhering to the C99 standard with all warnings enabled.

EXES=ls chmod head cut repeat env cat

# compiler/linker settings

CC=gcc
CFLAGS=-g -O0 -Wall -Werror -std=c99 -pedantic -D_POSIX_C_SOURCE=200809L
LDFLAGS=-O0

# build targets

all: ../bin $(EXES)

.c:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<
	mv $@ ../bin

../bin:
	mkdir ../bin

clean:
	rm -rf ../bin

.PHONY: all clean

