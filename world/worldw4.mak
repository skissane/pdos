# Produce Windows executables
# links with PDPCLIB created by makefile.std
# Use SubC as compiler

all: world.exe

world.exe: world.obj
  pdld --no-insert-timestamp -s -o world.exe ../pdpclib/w32start.obj world.obj ../pdpclib/msvcrt.lib ../pdpclib/winsupc.obj

.c.obj:
  pdcc -E -D__SUBC__ -D__WIN32__ -Dunsigned= -Dlong=int -Dshort=int -Dconst= -Ddouble=int -I ../pdpclib -o $*.i $<
  sccwin -S $*.i
  rm -f $*.i
  pdas -o $@ --oformat coff $*.s
  rm -f $*.s
