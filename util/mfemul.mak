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

else ifeq "$(targ)" "std"

# This needs makefile.std to have been run in pdpclib
CC=gccwin
CFLAGS=-O2 -DPBEMUL
AS=pdas --oformat coff
COPTS=-S $(CFLAGS) -ansi -pedantic -fno-common -I../pdpclib -D__WIN32__ -D__NOBIVA__ \
    -DNEED_MVS
EXTRA=../pdpclib/w32start.obj
EXTRA2=../pdpclib/msvcrt.lib

else

# This needs makefile.p32 to have been run in pdpclib
# And you will need to change makefile.p32 to activate
# OLDWIN if you want to run on Windows 2000 or Freewindows
# and have cursor keys come through. And the COM routines
# won't work on those environments without this either.
# The keyboard routines won't work on PDOS/386 though
# And if you put this into C:\WINNT\system32\config.nt on a win2k system:
#  dosonly
#  dos=high, umb
#  device=%SystemRoot%\system32\himem.sys
#  device=%SystemRoot%\system32\ansi.sys /x
#  files=40
# (/x makes extended keys "independent")
# and then at a windows command prompt run command, then hxldr32
# you will be able to use ANSI
# but you may lose COM port support, or place a lot of stress on
# the system, so you can switch back to a windows prompt for that

CC=gccwin
CFLAGS=-O2 -DPBEMUL
AS=pdas --oformat coff
COPTS=-S $(CFLAGS) -ansi -pedantic -fno-common -I../pdpclib -D__WIN32__ -D__NOBIVA__ \
    -DNEED_MVS -D__STATIC__
EXTRA=../pdpclib/p32start.obj
EXTRA2=../pdpclib/pdpwin32.lib ../src/kernel32.lib

endif


LD=pdld
LDFLAGS=-s --no-insert-timestamp

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
