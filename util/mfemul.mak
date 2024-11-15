# Released to the public domain.
#
# Anyone and anything may copy, edit, publish,
# use, compile, sell and distribute this work
# and all its parts in any form for any purpose,
# commercial and non-commercial, without any restrictions,
# without complying with any conditions
# and by any means.

# define COMEMUL for a COM file, or PBEMUL for a pseudo-bios


ifeq "$(targ)" "arm"

# This needs makefile.war to have been run in pdpclib
CC=gccprm
CFLAGS=-O0 -DPBEMUL
AS=asprm
AR=arprm
COPTS=-S $(CFLAGS) -ansi -pedantic -fno-common -I../pdpclib -D__WIN32__ -D__NOBIVA__ \
    -DNEED_MVS \
    -D__WIN32__ -D__ARM__ -msoft-float -mapcs-32 -fno-leading-underscore \
    -D__USEBIVA__
EXTRA=../pdpclib/w32start.obj
EXTRA2=../pdpclib/armsupa.obj ../pdpclib/fpfuncsa.obj ../pdpclib/msvcrt.lib

else ifeq "$(targ)" "x64"

# This needs makefile.s64 to have been run in pdpclib
CC=gccw64
CFLAGS=-O2 -DPBEMUL
AS=pdas --oformat coff --64
COPTS=-S $(CFLAGS) -fno-builtin -fno-common -I../pdpclib -D__WIN32__ -D__NOBIVA__ \
    -DNEED_MVS -D__64BIT__
EXTRA=../pdpclib/w32start.obj
EXTRA2=../pdpclib/msvcrt.lib

else

CC=gccwin
CFLAGS=-O2 -DPBEMUL
AS=pdas --oformat coff
COPTS=-S $(CFLAGS) -ansi -pedantic -fno-common -I../pdpclib -D__WIN32__ -D__NOBIVA__ \
    -DNEED_MVS
EXTRA=../pdpclib/w32start.obj
EXTRA2=../pdpclib/msvcrt.lib

endif


LD=pdld
LDFLAGS=-s

OBJS=mfemul.obj ../bios/exeload.obj

TARGET=mfemul.exe

all: clean $(TARGET)

$(TARGET): $(OBJS)
  $(LD) $(LDFLAGS) -o $(TARGET) $(EXTRA) $(OBJS) $(EXTRA2)

.c.obj:
  $(CC) $(COPTS) -o $*.s $<
  $(AS) -o $@ $*.s
  rm -f $*.s

clean:
  rm -f $(OBJS) $(TARGET)
