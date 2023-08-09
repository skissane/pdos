# Released to the public domain.
#
# Anyone and anything may copy, edit, publish,
# use, compile, sell and distribute this work
# and all its parts in any form for any purpose,
# commercial and non-commercial, without any restrictions,
# without complying with

CC=x86_64-w64-mingw32-gcc
LD=pdld
LDFLAGS=-s -nostdlib
AS=pdas --oformat coff --64
COPTS=-S -O2 -mno-red-zone -D__64BIT__ -D__EFI__ -D__EFIBIOS__ -U__WIN32__ -I. -I../pdpclib -fno-builtin -fno-common -ansi -Wno-builtin-declaration-mismatch

OBJS=efigraph.obj

TARGET=efigraph.efi

all: clean $(TARGET)

$(TARGET): $(OBJS)
  $(LD) $(LDFLAGS) -subsystem 10 -e efimain -o $(TARGET) $(OBJS)

.c.obj:
  $(CC) $(COPTS) -o $*.s $<
  $(AS) -o $@ $*.s
  rm -f $*.s

clean:
  rm -f $(OBJS) $(TARGET)

