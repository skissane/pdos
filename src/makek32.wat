# Released to the public domain.
#
# Anyone and anything may copy, edit, publish,
# use, compile, sell and distribute this work
# and all its parts in any form for any purpose,
# commercial and non-commercial, without any restrictions,
# without complying with any conditions
# and by any means.

# This makefile builds kernel32.dll and kernel32.lib on Windows,
# using the Open Watcom toolchain.

CC=wcl386
CFLAGS=-oneatx
COPTS=-zls -c -ecc $(CFLAGS) -DNOLIBALLOC -zl -fpi87 -q -D__32BIT__ -D__WIN32__ -D__STATIC__ -I. -I..\pdpclib -wcd=2002 -wcd=2003 -wcd=2004 -wcd=1176
AS=wasm
LD=wlink

EXPORT_OBJS=dllcrt.obj kernel32.obj
OBJS=supportf.obj pdossupc.obj pos.obj string.obj

all: clean kernel32.dll

kernel32.dll: $(EXPORT_OBJS) $(OBJS)
  $(LD) File dllcrt.obj,kernel32.obj,supportf.obj,pdossupc.obj,pos.obj,string.obj Name kernel32.dll Form windows nt dll Runtime con Option quiet,nod,map,NOSTDCall,start='__DllMainCRTStartup@12'
  del kernel32.lib
  echo wlib -q kernel32.lib kernel32.dll
  wlib -q kernel32.lib ++CloseHandle.kernel32._CloseHandle@4
  wlib -q kernel32.lib ++GetCommandLineA.kernel32._GetCommandLineA@0
  wlib -q kernel32.lib ++GetStdHandle.kernel32._GetStdHandle@4
  wlib -q kernel32.lib ++GetConsoleMode.kernel32._GetConsoleMode@8
  wlib -q kernel32.lib ++SetConsoleMode.kernel32._SetConsoleMode@8
  wlib -q kernel32.lib ++ExitProcess.kernel32._ExitProcess@4
  wlib -q kernel32.lib ++CreateFileA.kernel32._CreateFileA@28
  wlib -q kernel32.lib ++GetLastError.kernel32._GetLastError@0
  wlib -q kernel32.lib ++ReadFile.kernel32._ReadFile@20
  wlib -q kernel32.lib ++WriteFile.kernel32._WriteFile@20
  wlib -q kernel32.lib ++DeleteFileA.kernel32._DeleteFileA@4
  wlib -q kernel32.lib ++MoveFileA.kernel32._MoveFileA@8
  wlib -q kernel32.lib ++SetFilePointer.kernel32._SetFilePointer@16
  wlib -q kernel32.lib ++GetConsoleMode.kernel32._GetConsoleMode@8
  wlib -q kernel32.lib ++SetConsoleMode.kernel32._SetConsoleMode@8
  wlib -q kernel32.lib ++GlobalAlloc.kernel32._GlobalAlloc@8
  wlib -q kernel32.lib ++GlobalFree.kernel32._GlobalFree@4
  wlib -q kernel32.lib ++GetEnvironmentStrings.kernel32._GetEnvironmentStrings@0
  wlib -q kernel32.lib ++CreateProcessA.kernel32._CreateProcessA@40
  wlib -q kernel32.lib ++WaitForSingleObject.kernel32._WaitForSingleObject@8
  wlib -q kernel32.lib ++GetExitCodeProcess.kernel32._GetExitCodeProcess@8

dllcrt.obj: ..\pdpclib\dllcrt.c
  $(CC) -c $(COPTS) -D__EXPORT__ ..\pdpclib\dllcrt.c

kernel32.obj: kernel32.c
  $(CC) -c $(COPTS) -D__EXPORT__ kernel32.c

pdossupc.obj: ..\pdpclib\pdossupc.c
  $(CC) $(COPTS) ..\pdpclib\pdossupc.c

.asm.obj:
  $(AS) -q -DWATCOM $<

.c.obj:
  $(CC) $(COPTS) $<

clean:
  rm -f *.obj
  rm -f kernel32.dll
  rm -f kernel32.lib
