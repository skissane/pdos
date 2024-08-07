rem build z/pdos-generic pseudo-bios
rem You need to have run pdpzpb.bat in pdpclib first

gccmvs -Os -U__MVS__ -DNEED_MVS -D__MF32__ -D__ZPDOSGPB__ -DUSE_MEMMGR -S -I . -I ../pdpclib -I ../bios -I ../src bios.c
gccmvs -Os -U__MVS__ -DNEED_MVS -D__MF32__ -D__ZPDOSGPB__ -DUSE_MEMMGR -S -I . -I ../pdpclib -I ../bios -I ../src ../bios/exeload.c
m4 -I . -I ../pdpclib mfbios.m4 >mfbios.jcl
call runmvs mfbios.jcl output.txt none bios.exe
rem we need to signal to sapstart that this is already loaded
rem in memory so it doesn't need to attempt to self-load
rem this offset will change if sapstart ever changes
zap bios.exe 0x23cb 0x01
