# Produce Windows executables
# links with PDPCLIB created by makefile.msv
# Provides an example of doing a Windows call

CC=gccwin
CFLAGS=-O2
LD=ldwin
LDFLAGS=
AS=aswin
AR=arwin
COPTS=-S $(CFLAGS) -fno-common -ansi -I. -I../pdpclib -I../src -D__WIN32__

all: clean worldw.exe

worldw.exe: worldw.o
  $(LD) $(LDFLAGS) -s -o worldw.exe ../pdpclib/w32start.o worldw.o ../pdpclib/msvcrt.a ../src/kernel32.a

.c.o:
  $(CC) $(COPTS) -o $*.s $<
  $(AS) -o $@ $*.s
  rm -f $*.s

clean:
  rm -f *.o worldw.exe
