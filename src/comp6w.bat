gcc386 -S -O2 -ansi -fno-common -D__PDOS386__ -D__32BIT__ -I../pdpclib pcomm.c
gcc386 -S -O2 -ansi -fno-common -D__PDOS386__ -D__32BIT__ -I../pdpclib dostime.c

as386 -o pcomm.o pcomm.s
del pcomm.s
as386 -o dostime.o dostime.s
del dostime.s

ld386 -N -s -e ___pdosst32 -o pcomm.exe ../pdpclib/pdosst32.o pcomm.o dostime.o ../pdpclib/pdos.a
