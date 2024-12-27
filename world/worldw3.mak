ifeq "$(targ)" "x64"
# Produce Win64 executables
# links with PDPCLIB created by makefile.s64
AS=pdas --64

else
# Produce Windows executables
# links with PDPCLIB created by makefile.std
AS=pdas

endif

all: world.exe

world.exe: world.obj
  pdld -s -nostdlib -o world.exe ../pdpclib/w32start.obj world.obj ../pdpclib/msvcrt.lib

.c.obj:
  pdcc -D__NODECLSPEC__ -D__WIN32__ -I ../pdpclib -o $*.s $<
  $(AS) -o $@ --oformat coff $*.s
  rm -f $*.s
