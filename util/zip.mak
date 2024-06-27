# Released to the public domain.
#
# Anyone and anything may copy, edit, publish,
# use, compile, sell and distribute this work
# and all its parts in any form for any purpose,
# commercial and non-commercial, without any restrictions,
# without complying with any conditions
# and by any means.

# Produces Windows executable.
# Links with PDPCLIB created by makefile.std.

CC=gccwin
CFLAGS=-O2
LD=pdld
LDFLAGS=
AS=as86
AR=ar
COPTS=-S $(CFLAGS) -fno-common -ansi -I../pdpclib -I../src -I../../pdcrc -D__WIN32__ -D__NOBIVA__ -D__32BIT__

all: clean zip.exe

zip.exe: zip.obj ../src/pos.obj ../src/supportf.obj
    $(LD) $(LDFLAGS) -s --no-insert-timestamp -o zip.exe ../pdpclib/w32start.obj zip.obj ../src/pos.obj ../src/supportf.obj ../pdpclib/msvcrt.lib

.c.obj:
    $(CC) $(COPTS) -o $*.s $<
    pdas --oformat coff -o $@ $*.s
    rm -f $*.s

.asm.obj:
    $(AS) -Dmemodel=flat -f coff -o $@ $<

clean:
    rm -f *.obj
    rm -f zip.s
