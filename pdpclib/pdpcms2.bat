rem Produces a CMS executable (including RDWs) and then executes it

as370 -o mfsupa.obj mfsupa.asm

gcc370 -S -o mfsupc.s mfsupc.c
as370 -o mfsupc.obj mfsupc.s
rm -f mfsupc.s

gcc370 -S -I . -o string.s string.c
as370 -o string.obj string.s
rm -f string.s

pdld -e __crt0 --oformat cms -o pdptest.exe mfsupa.obj mfsupc.obj
call runcms pdpcms2.exec output.txt pdptest.exe
