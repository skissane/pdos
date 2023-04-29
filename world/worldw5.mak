# Produce Windows executables
# links with PDPCLIB created by makefile.msv
# Use occ as compiler

all: world.exe

world.exe: world3.o
  pdld --no-insert-timestamp -s -o world.exe ../pdpclib/w32start.o world.o ../pdpclib/msvcrt.a ../pdpclib/winsupc.obj

.c.o:
  pdcc -E -D__SUBC__ -D__WIN32__ -Dunsigned= -Dlong=int -Dshort=int -Dconst= -Ddouble=int -I ../pdpclib -o $*.i $<
  occ $*.i -o $*.s
  rm -f $*.i
  pdas -o $@ -O coff $*.s
  rm -f $*.s
