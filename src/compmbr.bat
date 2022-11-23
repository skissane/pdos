rem wasmr -zq -zcm -Dmemodel=tiny mbr.asm
rem tasm -m2 -Dmemodel=tiny mbr.asm
rem tlink -t -x mbr,mbr.com,,,

rem wasm is incorrectly treating warnings as errors
rem so be careful if you see any warnings
wasm -zq -zcm -Dmemodel=tiny mbr.asm
wlink File mbr.obj Name mbr.com Form dos com Option quiet

rem as86 -Dmemodel=tiny -o mbr.o mbr.asm
rem ld86 -o mbr.com --oformat msdos mbr.o
