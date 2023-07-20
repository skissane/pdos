rem wasmr -zq -zcm -Dmemodel=tiny pbootsec.asm
rem rem tasm requires 2 passes to generate the desired near jump
rem rem for first instruction
rem rem tasm -m2 -Dmemodel=tiny pbootsec.asm
rem tlink -t -x pbootsec,pbootsec.com,,,

rem wasm -zq -zcm -Dmemodel=tiny pbootsec.asm
rem wlink File pbootsec.obj Name pbootsec.com Form dos com Option quiet

as86 -Dmemodel=tiny -o pbootsec.o pbootsec.asm
ld86 -o pbootsec.com --oformat msdos pbootsec.o

rem ml -c -Dmemodel=tiny pbootsec.asm
rem wlink File pbootsec.obj Name pbootsec.com Form dos com Option quiet
