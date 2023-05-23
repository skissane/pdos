# Released to the public domain.
#
# Anyone and anything may copy, edit, publish,
# use, compile, sell and distribute this work
# and all its parts in any form for any purpose,
# commercial and non-commercial, without any restrictions,
# without complying with any conditions
# and by any means.

# This makefile builds 32-bit pcomm.exe (COMMAND.EXE) on Windows.
# makefile.wcp in PDPCLIB must be used before using this makefile to produce the pdos.lib.

CC=wcl386
CFLAGS=-oneatx
COPTS=-zls -c -ecc $(CFLAGS) -zl -fpi87 -q -U__WIN32__ -D__PDOS386__ -D__32BIT__ -I..\pdpclib -wcd=2002 -wcd=2003 -wcd=2004 -wcd=1176
AS=wasm
LD=wlink

TARGET=pcomm.exe
OBJS=pcomm.obj dostime.obj

all: clean $(TARGET)

$(TARGET): $(OBJS)
  $(LD) File ..\pdpclib\pdosst32.obj,pcomm.obj,dostime.obj Name $(TARGET) Form windows nt Runtime con Library ..\pdpclib\pdos.lib Option quiet,nodef,start=___pdosst32

.c.obj:
  $(CC) $(COPTS) -fo=$@ $<

clean:
  rm -f *.obj
  rm -f $(TARGET)
