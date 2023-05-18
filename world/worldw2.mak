# Produce Windows executables
# links with PDPCLIB created by makefile.p32

all: world.exe

world.exe: world.obj
  pdld -s -o world.exe ../pdpclib/p32start.obj world.obj ../pdpclib/pdpwin32.lib ../src/kernel32.lib

.c.obj:
  pdcc -E -D__SUBC__ -D__WIN32__ -D__STATIC__ -Dunsigned= -Dlong=int -Dshort=int -Dconst= -Ddouble=int -I ../pdpclib -o $*.i $<
  sccwin -S $*.i
  rm -f $*.i
  pdas -o $@ --oformat coff $*.s
  rm -f $*.s
