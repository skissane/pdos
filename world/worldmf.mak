# Produce z/PDOS-generic executables
# links with PDPCLIB created by makefile.zpg


COPTS=-D__MF32__ -D__PDOSGEN__ -D__NOBIVA__ \
    -I../pdpclib -I../generic -I../src

all: world.exe

world.exe: world.obj
  pdld -s -nostdlib -o world.exe ../pdpclib/pgastart.o world.obj ../pdpclib/zpgsupa.o

.c.obj:
  pdcc $(COPTS) -o $*.s $<
  as370 -mhlasm -mebcdic -o $@ $*.s
#  rm -f $*.s
