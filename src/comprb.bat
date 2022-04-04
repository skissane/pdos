@echo on
wasm -zq -zcm -Dmemodel=tiny reboot.asm
wlink File reboot.obj Name reboot.com Form dos com Option quiet
