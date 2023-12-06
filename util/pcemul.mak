# Released to the public domain.
#
# Anyone and anything may copy, edit, publish,
# use, compile, sell and distribute this work
# and all its parts in any form for any purpose,
# commercial and non-commercial, without any restrictions,
# without complying with any conditions
# and by any means.

CC=gccwin
CFLAGS=-O2
LD=pdld
LDFLAGS=-s
AS=pdas --oformat coff
COPTS=-S $(CFLAGS) -ansi -pedantic -fno-common -I../pdpclib -I../src -D__32BIT__ -D__WIN32__ -D__NOBIVA__

OBJS=pcemul.obj

TARGET=pcemul.exe

all: clean $(TARGET)

$(TARGET): $(OBJS)
  $(LD) $(LDFLAGS) -o $(TARGET) ../pdpclib/w32start.obj $(OBJS) ../pdpclib/msvcrt.lib

.c.obj:
  $(CC) $(COPTS) -o $*.s $<
  $(AS) -o $@ $*.s
  rm -f $*.s

clean:
  rm -f $(OBJS) $(TARGET)
