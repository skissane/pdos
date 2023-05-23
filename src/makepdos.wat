# Released to the public domain.
#
# Anyone and anything may copy, edit, publish,
# use, compile, sell and distribute this work
# and all its parts in any form for any purpose,
# commercial and non-commercial, without any restrictions,
# without complying with any conditions
# and by any means.

# This makefile builds 32-bit pdos.exe (aka PDOS.SYS) on Windows.
# makefile.wcp in PDPCLIB must be used before using this makefile to produce the pdos.lib.

CC=wcl386
CFLAGS=-oneatx
COPTS=-zls -c -ecc $(CFLAGS) -zl -fpi87 -q -D__32BIT__ -U__WIN32__ -DNOVM -DNEED_AOUT -DNEED_MZ -D__PDOS386__ -D__32BIT__ -I..\bios -I..\pdpclib -wcd=2002 -wcd=2003 -wcd=2004 -wcd=1176
AS=wasm
LD=wlink
#LD=pdld


TARGET=pdos.exe
TEMP_ARCHIVE=os.lib
ARCHIVE_OBJS=bos.obj fat.obj ..\bios\exeload.obj physmem.obj vmm.obj process.obj int21.obj uart.obj \
             int80.obj log.obj helper.obj memmgr.obj patmat.obj supportf.obj protintp.obj protintf.obj pdosf.obj
OBJS=strt32.obj pdos.obj

all: clean $(TARGET)

$(TARGET): $(OBJS) $(TEMP_ARCHIVE)
  $(LD) File strt32.obj,pdos.obj Library $(TEMP_ARCHIVE),..\pdpclib\pdos.lib Name pdos.exe Form windows nt dll Runtime con Output raw offset=0x20000 order clname CODE offset=0x21000 clname DATA clname BSS Option quiet,nod,nostdcall,map,start='_start'
  zap pdos.exe 0 0xe9
  zap pdos.exe 1 0xfb
  zap pdos.exe 2 0x0f
  zap pdos.exe 3 0x00
  zap pdos.exe 4 0x00
#  $(LD) -Map map.txt --no-insert-timestamp --image-base 0x20000 --file-alignment 4096 --convert-to-flat --disable-reloc-section -s -e _start -o $(TARGET) $(OBJS) $(TEMP_ARCHIVE) ../pdpclib/pdos.lib
#  link -map -nologo -fixed -nodefaultlib -entry:start -out:$@ $(OBJS) $(TEMP_ARCHIVE) ../pdpclib/pdos.lib
  rm -f $(TEMP_ARCHIVE)

$(TEMP_ARCHIVE): $(ARCHIVE_OBJS)
  rm -f $(TEMP_ARCHIVE)
  wlib -q $(TEMP_ARCHIVE) +$(ARCHIVE_OBJS)
#  $(AR) -nologo -out:$(TEMP_ARCHIVE) $(ARCHIVE_OBJS)

.c.obj:
  $(CC) $(COPTS) -fo=$@ $<

.asm.obj:
#  as86 -f coff -o $@ $<
#  ml -c -nologo -coff $<
  $(AS) -zcm -q -fo=$@ $<

clean:
  rm -f *.obj
  rm -f $(TARGET)
  rm -f $(TEMP_ARCHIVE)
