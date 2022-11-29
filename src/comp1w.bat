@echo on
wasm -zq -zcm -Dmemodel=tiny -DWATCOM ploadst.asm
wasm -zq -zcm -Dmemodel=tiny int13x.asm
wasm -zq -zcm -Dmemodel=tiny -DWATCOM nearw.asm

rem these two files are shared between modules
wasm -zq -zcm -Dmemodel=tiny support.asm 
wasm -zq -zcm -Dmemodel=tiny lldos.asm
wcl -oneatx -ecc -q -w -c -I. -mt -zl -D__MSDOS__ -fpi87 -s -zdp -zu -DNEED_DUMP -I..\pdpclib pload.c minifat.c bos.c pdosload.c ..\pdpclib\string.c ..\pdpclib\dossupc.c
wcl -oneatx -ecc -q -w -c -I. -mt -zl -D__MSDOS__ -fpi87 -s -zdp -zu -I..\pdpclib ..\pdpclib\ctype.c

if exist watcom.lib del watcom.lib
wlib -b -q watcom +support.obj
wlib -b -q watcom +bos.obj
wlib -b -q watcom +dossupc.obj
wlib -b -q watcom +minifat.obj
wlib -b -q watcom +string.obj
wlib -b -q watcom +pdosload.obj
wlib -b -q watcom +lldos.obj
wlib -b -q watcom +ctype.obj

rem the order of the following stuff is critical so that the first 3*512
rem bytes have sufficient stuff to do the rest of the load of itself!!!
rem it is ploadst, pload, int13x and near that need to be in the first 3*512
rem and only the first 3 functions of near
wlink File ploadst.obj,pload.obj,int13x.obj,nearw.obj Name pload.com Form dos com Library watcom.lib Option quiet,map,verbose
