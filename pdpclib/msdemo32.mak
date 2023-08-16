# This builds the demo program.
# Originally written in C, then converted to custom assembler
# while waiting for a C compiler that generates the desired code

AS=as86
LD=pdld

all: msdemo32.exe

msdemo32.exe: msdemo32.obj
  $(LD) -s -nostdlib --no-insert-timestamp -o msdemo32.exe msdemo32.obj msvcrt.lib

msdemo32.obj: msdemo32.asm
  $(AS) -o $*.obj -f coff $<

clean:
  rm -f *.obj
  rm -f msdemo32.exe
