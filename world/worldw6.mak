# Produce Windows executables
# links with PDPCLIB created by makefile.std
# Provides an example of doing a Windows call

CC=gccwin
CFLAGS=-O2
LD=pdld --no-insert-timestamp
LDFLAGS=
AS=pdas --oformat coff
AR=xar
COPTS=-S $(CFLAGS) -fno-common -ansi -I. -I../pdpclib -I../src -D__WIN32__

all: clean worldw.exe

worldw.exe: worldw.obj
  $(LD) $(LDFLAGS) -s -o worldw.exe ../pdpclib/w32start.obj worldw.obj ../pdpclib/msvcrt.lib ../src/kernel32.lib

.c.obj:
  $(CC) $(COPTS) -o $*.s $<
  $(AS) -o $@ $*.s
  rm -f $*.s

clean:
  rm -f *.obj worldw.exe
