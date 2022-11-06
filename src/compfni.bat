rem rem ml /c /nologo fninit.asm
rem wasm -zq -zcm fninit.asm
rem wlink File fninit.obj Name fninit.exe Form dos Option quiet,start=start

rem as86 --nowarn -o fninit.o fninit.asm
rem ld86 --oformat msdos-mz -e start -o fninit.exe fninit.o

as386 -o fninit.o fninit.s
ld386 -s -N -e start -o fninit.exe fninit.o
