# Released to the public domain.
#
# Anyone and anything may copy, edit, publish,
# use, compile, sell and distribute this work
# and all its parts in any form for any purpose,
# commercial and non-commercial, without any restrictions,
# without complying with any conditions
# and by any means.

# Produces PDOS-generic executable.

# This builds a PDOS-generic executable for ARM
# Using the standard ARM32 (including UEFI) calling convention
# We don't have an appropriate subsystem for this executable
# as it is neither Windows nor UEFI - it is PDOS-generic

CC=gccprm
CFLAGS=-O0
#LD=ldprm -subsystem efi
LD=pdld --oformat coff --no-insert-timestamp --subsystem 10
LDFLAGS=
AS=asprm
AR=arprm
COPTS=-S $(CFLAGS) -ansi -I. -I../pdpclib -I../src \
  -I../generic -I../../pdcrc \
  -D__ARM__ -fno-common -mapcs-32 -fno-builtin \
  -msoft-float -DHAVE_DIR \
  -D__PDOS386__ -D__PDOSGEN__ -D__ARMGEN__ -D__NOBIVA__ 

all: clean zip.exe

zip.exe: ../pdpclib/pgastart.o zip.o ../pdpclib/armsupa.o ../pdpclib/string.o
  rm -f zip.exe
  $(LD) $(LDFLAGS) -s -e ___crt0 -o zip.exe ../pdpclib/pgastart.o zip.o ../pdpclib/string.o ../pdpclib/armsupa.o

.c.o:
    $(CC) $(COPTS) -o $*.s $<
    $(AS) -o $@ $*.s
    rm -f $*.s

.asm.o:
  $(AS) --defsym LINUX=0 --defsym ELF=0 --defsym STACKPARM=0 -o $@ $<

clean:
  rm -f *.o zip.exe
