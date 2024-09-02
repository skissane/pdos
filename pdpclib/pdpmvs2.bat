rem Produces an MVS PE executable and then executes it

as370 -a=list.txt -o mfsupa.obj mfsupa.asm
pdld --oformat mainframe -o pdptest.exe mfsupa.obj
call runmvs pdpmvs2.jcl output.txt pdptest.exe
