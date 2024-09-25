rem Produces an ELF32 executable for Linux Bigfoot and then executes it

pdcc -N -D BIGFOOT -o mfsupa.s mfsupa.asm
as370 -mhlasm -mebcdic -a=list.txt -o mfsupa.obj mfsupa.s

gcc370 -Os -I. -I../generic -I../src -S -o mfsupc.s mfsupc.c
as370 -o mfsupc.obj mfsupc.s
rm -f mfsupc.s

gcc370 -Os -S -I . -o string.s string.c
as370 -o string.obj string.s
rm -f string.s

pdld -e __crt0 --oformat elf -o pdptest.exe mfsupa.obj mfsupc.obj string.obj

sleep 2

call runbig pdptest.exe output.txt
