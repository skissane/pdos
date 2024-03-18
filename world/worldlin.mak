# Produce 32-bit Linux ELF executables
# links with PDPCLIB created by makefile.lnp
# Use SubC as compiler

LDFLAGS=-s --no-insert-timestamp -nostdlib --oformat elf

all: world.exe

world.exe: world.obj
  pdld $(LDFLAGS) -o world.exe ../pdpclib/linstart.o world.obj ../pdpclib/pdplinux.a

.c.obj:
  pdcc -E -D__SUBC__ -U__WIN32__ -D__OS2__ -D__32BIT__ -Dunsigned= -Dlong=int -Dshort=int -Dconst= -Ddouble=int -I ../pdpclib -o $*.i $<
  sccwin -S $*.i
  rm -f $*.i
  pdas -o $@ --oformat coff $*.s
  rm -f $*.s
