pdcc -N -D MVS -DREALHLASM -o mfsupa.s mfsupa.asm
gccmvs -Os -DXXX_MEMMGR -S -I . -I ../generic -I ../src mfsupc.c
m4 -I . pdpmvs3.m4 >pdpmvs3.jcl
sleep 2
rem call sub pdpmvs.jcl
call runmvs pdpmvs3.jcl output.txt
