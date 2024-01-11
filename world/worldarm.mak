# Produce pdos-generic ARM executables
# links with PDPCLIB created by makefile.aga
# Uses SubC as compiler (not working)

#CC=gccarm -fno-common -mapcs-32 -fno-builtin
CC=sccarm
LD=ldarm
AS=asarm
AR=ararm


all: world.exe

world.exe: world.o
  $(LD) -N -s -nostdlib -o world.exe ../pdpclib/pgastart.o world.o ../pdpclib/armsupa.o

.c.o:
  pdcc -N -D__SUBC__ -D__UNOPT__ -D__PDOS386__ -D__PDOSGEN__ -D__ARMGEN__ -D__NOBIVA__ -Dunsigned= -Dlong=int -Dshort=int -Dconst= -Ddouble=int -I ../../pdos/pdpclib -I . -I ../../pdos/generic -o $*.i $<
  $(CC) -S -o $*.s $*.i
  rm -f $*.i
  $(AS) -o $@ $*.s
#  rm -f $*.s

clean:
  rm -f *.o
  rm -f world.exe
