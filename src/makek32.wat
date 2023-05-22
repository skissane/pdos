# Released to the public domain.
#
# Anyone and anything may copy, edit, publish,
# use, compile, sell and distribute this work
# and all its parts in any form for any purpose,
# commercial and non-commercial, without any restrictions,
# without complying with any conditions
# and by any means.

# This makefile builds kernel32.dll and kernel32.lib on Windows,
# using the Open Watcom toolchain.

VPATH=..\pdpclib

CC=wcl386
CFLAGS=-oneatx
COPTS=-zls -c -ecc $(CFLAGS) -DNOLIBALLOC -zl -fpi87 -q -D__32BIT__ -D__WIN32__ -D__STATIC__ -I. -I..\pdpclib -wcd=2002 -wcd=2003 -wcd=2004 -wcd=1176
AS=wasm
LD=wlink

EXPORT_OBJS=dllcrt.obj kernel32.obj
OBJS=supportf.obj pdossupc.obj pos.obj string.obj

all: clean kernel32.dll

kernel32.dll: $(EXPORT_OBJS) $(OBJS)
  $(LD) File dllcrt.obj,kernel32.obj,supportf.obj,pdossupc.obj,pos.obj,string.obj Name kernel32.dll Form windows nt dll Runtime con Option quiet,nod,nostdcall,map,start='__DllMainCRTStartup@12'
  rm -f kernel32.lib
  wlib -q kernel32.lib @kernel32.wat

dllcrt.obj: ..\pdpclib\dllcrt.c
  $(CC) -c $(COPTS) -D__EXPORT__ ..\pdpclib\dllcrt.c

kernel32.obj: kernel32.c
  $(CC) -c $(COPTS) -D__EXPORT__ kernel32.c

.asm.obj:
  $(AS) -q -DWATCOM $<

.c.obj:
  $(CC) $(COPTS) $<

clean:
  rm -f *.obj
  rm -f kernel32.dll
  rm -f kernel32.lib
