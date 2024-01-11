# Produce pdos-generic ARM executables
# used to link with PDPCLIB created by makefile.aga
# but for now is self-contained instead
# Uses SubC as compiler

#CC=gccarm -fno-common -mapcs-32 -fno-builtin
CC=sccarm
LD=ldarm
AS=asarm
AR=ararm


all: world.exe

world.exe: world.o ../pdpclib/pgastart.o ../pdpclib/armsupa.o
  $(LD) -N -s -nostdlib -o world.exe ../pdpclib/pgastart.o world.o ../pdpclib/armsupa.o

.c.o:
  pdcc -N -D__SUBC__ -D__UNOPT__ -D__PDOS386__ -D__PDOSGEN__ -D__ARMGEN__ -D__NOBIVA__ -Dunsigned= -Dlong=int -Dshort=int -Dconst= -Ddouble=int -I ../pdpclib -I . -I ../generic -o $*.i $<
  $(CC) -S -o $*.s $*.i
  rm -f $*.i
  $(AS) -o $@ $*.s
  rm -f $*.s

.asm.o:
  $(AS) --defsym LINUX=0 --defsym ELF=0 --defsym STACKPARM=1 -o $@ $<

clean:
  rm -f *.o
  rm -f world.exe
