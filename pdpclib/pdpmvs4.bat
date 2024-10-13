rem Produces an MVS PE executable and then executes it

call mvsasm mvsstart.asm mvsstart.obj

call mvsasm mvssupa.asm mvssupa.obj

pdmake -f makefile.370

sleep 2

call runmvs pdpmvs2.jcl output.txt pdptest.exe
