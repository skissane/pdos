# Produce Windows executables
# links with PDPCLIB created by makefile.std
# Use occ as compiler

all: world.exe

world.exe: world.obj
  pdld --no-insert-timestamp -s -o world.exe ../pdpclib/w32start.obj world.obj ../pdpclib/msvcrt.lib ../pdpclib/winsupa.obj

.c.obj:
  pdcc -E -D__NODECLSPEC__ -D__WIN32__ -I ../pdpclib -o $*.i $<
  occ $*.i -o $*.s
  rm -f $*.i
  pdas -o $@ --oformat coff $*.s
  rm -f $*.s
