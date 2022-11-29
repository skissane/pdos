wasm -zq -zcm -D__SZ4__ -Dmemodel=huge -DWATCOM pdosstrt.asm
wasm -zq -zcm -D__SZ4__ -Dmemodel=huge support.asm
wasm -zq -zcm -D__SZ4__ -Dmemodel=huge lldos.asm
wasm -zq -zcm -D__SZ4__ -Dmemodel=huge handlers.asm
rem ml -c -D__SZ4__ -Dmemodel=huge handlers.asm
rem as86 -D__SZ4__ -Dmemodel=huge handlers.asm
wcl -oneatx -D__SZ4__ -zp1 -ecc -q -w -c -mh -zl -fpi87 -s -zdp -zu -D__MSDOS__ -DBUFSIZ=512 -I..\pdpclib memmgr.c patmat.c process.c
wcl -oneatx -D__SZ4__ -zp1 -ecc -q -w -c -mh -zl -fpi87 -s -zdp -zu -D__MSDOS__ -DBUFSIZ=512 -I..\pdpclib int21.c log.c helper.c
wcl -oneatx -D__SZ4__ -zp1 -ecc -q -w -c -mh -zl -fpi87 -s -zdp -zu -D__MSDOS__ -DBUFSIZ=512 -I..\pdpclib bos.c fat.c
rem bcc -w- to switch off warnings
wcl -oneatx -D__SZ4__ -zp1 -ecc -q -w -c -mh -zl -fpi87 -s -zdp -zu -D__MSDOS__ -DBUFSIZ=512 -I..\pdpclib pdos.c

if exist os.lib del os.lib
wlib -b -q os +bos.obj
wlib -b -q os +support.obj
wlib -b -q os +fat.obj
wlib -b -q os +lldos.obj
wlib -b -q os +handlers.obj
wlib -b -q os +memmgr.obj
wlib -b -q os +patmat.obj
wlib -b -q os +process.obj
wlib -b -q os +int21.obj
wlib -b -q os +log.obj
wlib -b -q os +helper.obj

wlink File pdosstrt.obj,pdos.obj Name pdos.exe Form dos Library os.lib,..\pdpclib\watcomo.lib Option quiet,map,verbose,dosseg
