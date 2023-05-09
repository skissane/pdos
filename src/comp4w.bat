@echo on
wasm -zq -zcm -Dmemodel=tiny support.asm
rem as86 -DAS86 -Dmemodel=tiny -o support.obj support.asm
rem ml -c -Dmemodel=tiny support.asm
wasm -zq -zcm -Dmemodel=tiny -DWATCOM -DMAKECOM ploadst.asm
wasm -zq -zcm -Dmemodel=tiny -DWATCOMC nearw.asm
wasm -zq -zcm -Dmemodel=tiny lldos.asm
wasm -zq -zcm -Dmemodel=tiny -DWATCOM protinta.asm
wasm -zq -zcm -Dmemodel=tiny int13x.asm
wcl -oneatx -ecc -q -w -c -I. -mt -zl -D__MSDOS__ -fpi87 -s -zdp -zu -DPDOS32 -I..\pdpclib pload.c protint.c file.c minifat.c bos.c
wcl -oneatx -ecc -q -w -c -I. -mt -zl -D__MSDOS__ -fpi87 -s -zdp -zu -DPDOS32 -DNEED_DUMP -I..\pdpclib pdosload.c
wcl -oneatx -ecc -q -w -c -I. -mt -zl -D__MSDOS__ -fpi87 -s -zdp -zu -I..\pdpclib ..\pdpclib\string.c ..\pdpclib\dossupc.c
wcl -oneatx -ecc -q -w -c -I. -mt -zl -D__MSDOS__ -fpi87 -s -zdp -zu -I..\pdpclib ..\pdpclib\ctype.c
if exist watcom.lib del watcom.lib
wlib -b -q watcom +support.obj
wlib -b -q watcom +bos.obj
wlib -b -q watcom +dossupc.obj
wlib -b -q watcom +minifat.obj
wlib -b -q watcom +string.obj
wlib -b -q watcom +pdosload.obj
wlib -b -q watcom +lldos.obj
wlib -b -q watcom +protinta.obj
wlib -b -q watcom +file.obj
wlib -b -q watcom +protint.obj
wlib -b -q watcom +ctype.obj

rem the below documentation is no longer valid
rem the order of the following stuff is critical so that the first 3*512
rem bytes have sufficient stuff to do the rest of the load of itself!!!
rem it is ploadst, pload, int13x and near that need to be in the first 3*512
rem and only the first 3 functions of near
rem wcl ploadst.obj pload.obj
rem wlink ploadst+pload+int13x+near+support+bos+dossupc+fat+string+pdosload+lldos+protinta+file+protint,pload.com,,,

wlink File ploadst.obj,pload.obj,int13x.obj,nearw.obj Name pload.com Form dos com Library watcom.lib Option quiet,map Disable 1019,1080
