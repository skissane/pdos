# Produce a.out executables
# links with PDPCLIB created by makefile.pdw

all: world.exe

world.exe: world.o
  pdld386 -N -s -nostdlib -o world.exe ../pdpclib/pdosst32.o world.o ../pdpclib/pdos.a

.c.o:
  pdcc -E -D__SUBC__ -D__WIN32__ -D__STATIC__ -Dunsigned= -Dlong=int -Dshort=int -Dconst= -Ddouble=int -I ../pdpclib -o $*.i $<
  sccwin -S $*.i
  rm -f $*.i
  pdas -o $@ $*.s
  rm -f $*.s
