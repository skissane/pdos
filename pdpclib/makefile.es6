# This builds EFI executables for x86_64 using standard method
# We put no-builtin to stop the optimizer from making memset call memset
# We don't enable the BIOS as these are meant to be applications

CC=x86_64-w64-mingw32-gcc
AR=ar
LD=pdld --no-insert-timestamp
AS=pdas --64 --oformat coff
COPTS=-S -O2 -mno-red-zone -D__64BIT__ -D__EFI__ -U__WIN32__ -I. -fno-builtin -fno-common -ansi -Wno-builtin-declaration-mismatch

TARGET=pdptest.efi
OBJS=efistart.obj stdio.obj string.obj stdlib.obj start.obj time.obj errno.obj \
    assert.obj signal.obj locale.obj ctype.obj setjmp.obj math.obj x64supb.obj

$(TARGET): clean pdptest.obj $(OBJS)
  rm -f pdpefi.lib
  $(AR) r pdpefi.lib $(OBJS)
  $(AR) s pdpefi.lib
  $(LD) -s -subsystem 10 -e efimain -nostdlib -o $(TARGET) pdptest.obj $(OBJS)

.c.obj:
  $(CC) $(COPTS) -o $*.s $<
  $(AS) -o $@ $*.s
  rm -f $*.s

.asm.obj:
  $(AS) -o $@ $<

clean:
  rm -f *.obj
  rm -f $(TARGET)
