rem Produces a VSE executable and then executes it

as370 -a=list.txt -o mfsupa.obj mfsupa.asm

gcc370 -I . -S -o mfsupc.s mfsupc.c
as370 -a=list2.txt -o mfsupc.obj mfsupc.s
rm -f mfsupc.s

gcc370 -S -I . -o string.s string.c
as370 -o string.obj string.s
rm -f string.s

pdld -Map map.txt --image-base 0x7000c8 -e __crt0 --oformat vse -o pdptest.exe mfsupa.obj mfsupc.obj

call runvse pdpvse2.jcl output.txt pdptest.exe
