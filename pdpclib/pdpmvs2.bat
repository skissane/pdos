rem Produces an MVS PE executable and then executes it

as370 --defsym MVS=1 -o mfsupa.obj mfsupa.asm

gcc370 -I. -I../generic -I../src -S -o mfsupc.s mfsupc.c
as370 -o mfsupc.obj mfsupc.s
rm -f mfsupc.s

gcc370 -S -I . -o string.s string.c
as370 -o string.obj string.s
rm -f string.s

pdld -e __crt0 --oformat mvs -o pdptest.exe mfsupa.obj mfsupc.obj

call runmvs pdpmvs2.jcl output.txt pdptest.exe
