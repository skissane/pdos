# Released to the public domain.
#
# Anyone and anything may copy, edit, publish,
# use, compile, sell and distribute this work
# and all its parts in any form for any purpose,
# commercial and non-commercial, without any restrictions,
# without complying with any conditions
# and by any means.

VPATH=../pdld/src;../pdld/src/bytearray
CC=gccwin
CFLAGS=-O2
LD=pdld
LDFLAGS=-s
AS=pdas --oformat coff
# Uses -DNO_LONG_LONG because 64-bit object support is not needed.
COPTS=-S $(CFLAGS) -Wall -ansi -pedantic -fno-common -I../pdld/src -I../pdld/src/bytearray -I../pdpclib -D__WIN32__ -D__NOBIVA__ -DNO_LONG_LONG

OBJS=elftext.obj bytearray.obj elf_bytearray.obj xmalloc.obj

TARGET=elftext.exe

all: clean $(TARGET)

$(TARGET): $(OBJS)
  $(LD) $(LDFLAGS) -o $(TARGET) ../pdpclib/w32start.obj $(OBJS) ../pdpclib/msvcrt.lib

.c.obj:
  $(CC) $(COPTS) -o $*.s $<
  $(AS) -o $@ $*.s
  rm -f $*.s

clean:
  rm -f $(OBJS) $(TARGET)
