rem tasm -l needpdos.asm
rem tlink needpdos

rem wasm needpdos.asm
rem wlink file needpdos.obj name needpdos.exe form dos option quiet

as86 -o needpdos.o needpdos.asm
ld86 -e top --oformat msdos-mz -o needpdos.exe needpdos.o
