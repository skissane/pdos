rem wasm -zq -zcm fninit.asm
rem wlink File fninit.obj Name fninit.exe Form dos Option quiet

rem as86 -DAS86 -o fninit.o fninit.asm
rem ld86 --oformat msdos-mz -o fninit.exe fninit.o

as386 -o fninit.o fninit.s
ld386 -s -N -e start -o fninit.exe fninit.o
