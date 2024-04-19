# This builds a PDOS-generic executable
# 64-bit PE/COFF format, with 64-bit long
# No need to build a library, this is self-contained

CC=gccw64_l64
COPTS=-S -D__LONG64__ -D__USEBIVA__ -D__64BIT__ \
    -D__PDOS386__ -DHAVE_DIR -D__PDOSGEN__ -DNEED_EXIT \
    -I. -I../pdpclib -I../src -fno-builtin -fno-common
AS=pdas --64 --oformat coff
LD=pdld --no-insert-timestamp

all: clean pcomm.exe

pcomm.exe: ../pdpclib/pgastart.obj pcomm.obj
  rm -f pcomm.exe
  $(LD) -s -e __crt0 -nostdlib -o pcomm.exe ../pdpclib/pgastart.obj pcomm.obj

.c.obj:
  $(CC) $(COPTS) -o $*.s $<
  $(AS) -o $@ $*.s
  rm -f $*.s

clean:
  rm -f pcomm.exe
