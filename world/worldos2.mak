# Produce OS/2 executables
# links with PDPCLIB created by makefile.sos
# Use SubC as compiler

LDFLAGS=-s --no-insert-timestamp -nostdlib --oformat lx \
    --stub ../pdpclib/needpdos.exe

all: world.exe

world.exe: world.obj
  pdld $(LDFLAGS) -o world.exe ../pdpclib/os2strt.obj world.obj ../pdpclib/pdpos2.lib ../pdpclib/os2.lib

.c.obj:
  pdcc -E -D__SUBC__ -U__WIN32__ -D__OS2__ -D__32BIT__ -Dunsigned= -Dlong=int -Dshort=int -Dconst= -Ddouble=int -I ../pdpclib -o $*.i $<
  sccwin -S $*.i
  rm -f $*.i
  pdas -o $@ --oformat coff $*.s
  rm -f $*.s
