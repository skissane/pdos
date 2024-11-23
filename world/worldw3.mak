# Produce Windows executables
# links with PDPCLIB created by makefile.std

all: world2.exe

world2.exe: world2.obj
  pdld -s -nostdlib -o world2.exe ../pdpclib/w32start.obj world2.obj ../pdpclib/msvcrt.lib

.c.obj:
  pdcc -D__WIN32__ -I ../pdpclib -o $*.s $<
  pdas -o $@ --oformat coff $*.s
  rm -f $*.s
