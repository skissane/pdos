# Produce Windows executables
# links with PDPCLIB created by makefile.std

CC=gccwin
CFLAGS=-O2
LD=pdld
LDFLAGS=
AS=pdas
AR=xar
COPTS=-S $(CFLAGS) -fno-common -ansi -I. -I../pdpclib -D__WIN32__

all: clean world.exe

world.exe: world.obj
  $(LD) $(LDFLAGS) -s -o world.exe ../pdpclib/w32start.obj world.obj ../pdpclib/msvcrt.lib

.c.obj:
  $(CC) $(COPTS) -o $*.s $<
  $(AS) -o $@ $*.s
  rm -f $*.s

clean:
  rm -f *.obj world.exe
