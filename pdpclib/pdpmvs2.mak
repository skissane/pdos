all:
  pdcc -N -D MVS -o MFSUPA.S mfsupa.asm
  as370 -mhlasm -o MFSUPA.OBJ mfsupa.s
  gcc370 -Os -I. -I../generic -I../src -S -o MFSUPC.S mfsupc.c
  as370 -o MFSUPC.OBJ mfsupc.s
  pdld -e __crt0 --oformat mvs -o PDPTEST.EXE mfsupa.obj mfsupc.obj
