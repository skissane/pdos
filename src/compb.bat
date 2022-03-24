@echo off
rem wasmr -zq -zcm -Dmemodel=tiny pbootsec.asm
rem rem tasm requires 2 passes to generate the desired near jump
rem rem for first instruction
rem rem tasm -m2 -Dmemodel=tiny pbootsec.asm
rem tlink -t -x pbootsec,pbootsec.com,,,

wasm -zq -zcm -Dmemodel=tiny pbootsec.asm
wlink File pbootsec.obj Name pbootsec.com Form dos com Option quiet
