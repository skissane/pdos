rem Produces a VSE executable and then executes it

pdcc -N -D VSE -o mfsupa.s mfsupa.asm
as370 -mhlasm -mebcdic -a=list.txt -o mfsupa.obj mfsupa.s

gccmvs -U__MVS__ -D__VSE__ -I. -I../generic -I../src -S -o mfsupc.s mfsupc.c
as370 -mhlasm -mebcdic -a=list2.txt -o mfsupc.obj mfsupc.s
rm -f mfsupc.s

gccmvs -S -I . -o string.s string.c
as370 -mhlasm -mebcdic -o string.obj string.s
rm -f string.s

pdld -Map map.txt --image-base 0x7000c8 -e __crt0 --oformat vse -o pdptest.exe mfsupa.obj mfsupc.obj

call runvse pdpvse2.jcl output.txt pdptest.exe
