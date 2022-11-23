rem wasmr -zq -zcm -Dmemodel=tiny mbr.asm
rem tasm -m2 -Dmemodel=tiny mbr.asm
rem tlink -t -x mbr,mbr.com,,,

rem wasm is incorrectly treating warnings as errors
rem so be careful if you see any warnings
rem wasm -zq -zcm -Dmemodel=tiny mbr.asm
rem wlink File mbr.obj Name mbr.com Form dos com Option quiet

as86 -Dmemodel=tiny -o mbr.o mbr.asm
rem need to use binary format because ld86 will pad a com
rem to a 16-byte boundary
ld86 -o mbr.com --oformat binary mbr.o
