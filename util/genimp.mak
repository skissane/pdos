# Released to the public domain.
#
# Anyone and anything may copy, edit, publish,
# use, compile, sell and distribute this work
# and all its parts in any form for any purpose,
# commercial and non-commercial, without any restrictions,
# without complying with any conditions
# and by any means.

VPATH=../pdld/src;../pdld/src/bytearray


ifeq "$(targ)" "zpg"
# You need to have run makefile.zpg first
CC=gccmvs
CFLAGS=-O2
AS=as370 -mhlasm -mebcdic
AR=ar370
COPTS=-S $(CFLAGS) -fno-common -ansi -I. \
    -I../pdld/src -I../pdld/src/bytearray -I../pdpclib \
    -I../generic -I../src -U__MVS__ -D__MF32__ -D__PDOSGEN__ \
    -D__NOBIVA__
EXTRA1=--oformat mvs -e __crt0 ../pdpclib/pgastart.o

else
CC=gccwin
CFLAGS=-O2
AS=pdas --oformat coff
COPTS=-S $(CFLAGS) -Wall -ansi -pedantic -fno-common \
    -I../pdld/src -I../pdld/src/bytearray -I../pdpclib -D__WIN32__ -D__NOBIVA__
EXTRA1=../pdpclib/w32start.obj
EXTRA2=../pdpclib/msvcrt.lib
endif


LD=pdld
LDFLAGS=-s

OBJS=genimp.obj coff_bytearray.obj bytearray.obj xmalloc.obj

TARGET=genimp.exe

all: clean $(TARGET)

$(TARGET): $(OBJS)
  $(LD) $(LDFLAGS) -o $(TARGET) $(EXTRA1) $(OBJS) $(EXTRA2)

.c.obj:
  $(CC) $(COPTS) -o $*.s $<
  $(AS) -o $@ $*.s
  rm -f $*.s

clean:
  rm -f $(OBJS) $(TARGET)
