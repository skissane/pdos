# Produce Windows executables
# links with PDPCLIB created by makefile.p32

all: world.exe

world.exe: world.o
  pdld -s -o world.exe ../pdpclib/p32start.o world.o ../pdpclib/pdpwin32.a ../src/kernel32.a

.c.o:
  pdcc -E -D__SUBC__ -D__WIN32__ -D__STATIC__ -Dunsigned= -Dlong=int -Dshort=int -Dconst= -Ddouble=int -I ../pdpclib -o $*.i $<
  sccwin -S $*.i
  rm -f $*.i
  pdas -o $@ -O coff $*.s
  rm -f $*.s
