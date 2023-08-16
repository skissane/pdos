# This builds the demo program.
# Originally written in C, then converted to custom assembler
# while waiting for a C compiler that generates the desired code

CC=gccwin
AR=ar
AS=pdas
LD=pdld

all: msdemo32.exe

msdemo32.exe: msdemo32.obj
  $(LD) -s -nostdlib --no-insert-timestamp -o msdemo32.exe msdemo32.obj msvcrt.lib

msdemo32.obj: msdemo32.c
  $(CC) -S -masm=intel -O2 -D__WIN32__ -I . -I ../src -o $*.s $*.c
  $(AS) -o $*.obj --oformat coff $*.s
  rm -f $*.s

clean:
  rm -f *.obj
  rm -f msdemo32.exe
