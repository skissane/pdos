rem wasmr -zq -zcm -Dmemodel=tiny mbr.asm
rem tasm -m2 -Dmemodel=tiny mbr.asm
rem tlink -t -x mbr,mbr.com,,,

rem wasm is incorrectly treating warnings as errors
wasm -zq -zcm -Dmemodel=tiny mbr.asm
wlink File mbr.obj Name mbr.com Form dos com Option quiet
