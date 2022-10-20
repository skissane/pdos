# Produce Windows executables
# links with PDPCLIB created by makefile.p32

all: world2.exe

world2.exe: world2.o
  ldwin -s -nostdlib -o world2.exe ../pdpclib/p32start.o world2.o ../pdpclib/pdpwin32.a ../src/kernel32.a

.c.o:
  pdcc -D__WIN32__ -D__STATIC__ -I ../pdpclib -o $*.s $<
  pdas -o $@ -O coff $*.s
  rm -f $*.s
