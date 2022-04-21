rem wasm -zq -zcm fninit.asm
rem wlink File fninit.obj Name fninit.exe Form dos Option quiet

as386 -o fninit.o fninit.s
ld386 -s -N -e start -o fninit.exe fninit.o
