gcc386 -S -O0 -ansi -DNOVM -DNEED_AOUT -DNEED_MZ -fno-common -D__PDOS386__ -D__32BIT__ -I../bios -I../pdpclib pdos.c
gcc386 -S -O2 -ansi -DNOVM -DNEED_AOUT -DNEED_MZ -fno-common -D__PDOS386__ -D__32BIT__ -I../bios -I../pdpclib bos.c
gcc386 -S -O2 -ansi -DNOVM -DNEED_AOUT -DNEED_MZ -fno-common -D__PDOS386__ -D__32BIT__ -I../bios -I../pdpclib fat.c
gcc386 -S -O2 -ansi -DNOVM -DNEED_AOUT -DNEED_MZ -fno-common -D__PDOS386__ -D__32BIT__ -I../bios -I../pdpclib patmat.c
gcc386 -S -O2 -ansi -DNOVM -DNEED_AOUT -DNEED_MZ -fno-common -D__PDOS386__ -D__32BIT__ -I../bios -I../pdpclib memmgr.c
gcc386 -S -O2 -ansi -DNOVM -DNEED_AOUT -DNEED_MZ -fno-common -D__PDOS386__ -D__32BIT__ -I../bios -I../pdpclib protintp.c
gcc386 -S -O2 -ansi -DNOVM -DNEED_AOUT -DNEED_MZ -fno-common -D__PDOS386__ -D__32BIT__ -I../bios -I../pdpclib ../bios/exeload.c
gcc386 -S -O2 -ansi -DNOVM -DNEED_AOUT -DNEED_MZ -fno-common -D__PDOS386__ -D__32BIT__ -I../bios -I../pdpclib physmem.c
gcc386 -S -O2 -ansi -DNOVM -DNEED_AOUT -DNEED_MZ -fno-common -D__PDOS386__ -D__32BIT__ -I../bios -I../pdpclib vmm.c
gcc386 -S -O2 -ansi -DNOVM -DNEED_AOUT -DNEED_MZ -fno-common -D__PDOS386__ -D__32BIT__ -I../bios -I../pdpclib liballoc.c
gcc386 -S -O2 -ansi -DNOVM -DNEED_AOUT -DNEED_MZ -fno-common -D__PDOS386__ -D__32BIT__ -I../bios -I../pdpclib process.c
gcc386 -S -O2 -ansi -DNOVM -DNEED_AOUT -DNEED_MZ -fno-common -D__PDOS386__ -D__32BIT__ -I../bios -I../pdpclib int21.c
gcc386 -S -O2 -ansi -DNOVM -DNEED_AOUT -DNEED_MZ -fno-common -D__PDOS386__ -D__32BIT__ -I../bios -I../pdpclib int80.c
gcc386 -S -O2 -ansi -DNOVM -DNEED_AOUT -DNEED_MZ -fno-common -D__PDOS386__ -D__32BIT__ -I../bios -I../pdpclib log.c
gcc386 -S -O2 -ansi -DNOVM -DNEED_AOUT -DNEED_MZ -fno-common -D__PDOS386__ -D__32BIT__ -I../bios -I../pdpclib helper.c

as386 -o pdos.o pdos.s
del pdos.s
as386 -o bos.o bos.s
del bos.s
as386 -o fat.o fat.s
del fat.s
as386 -o patmat.o patmat.s
del patmat.s
as386 -o memmgr.o memmgr.s
del memmgr.s
as386 -o protintp.o protintp.s
del protintp.s
as386 -o exeload.o exeload.s
del exeload.s
as386 -o physmem.o physmem.s
del physmem.s
as386 -o vmm.o vmm.s
del vmm.s
as386 -o liballoc.o liballoc.s
del liballoc.s
as386 -o process.o process.s
del process.s
as386 -o int21.o int21.s
del int21.s
as386 -o int80.o int80.s
del int80.s
as386 -o log.o log.s
del log.s
as386 -o helper.o helper.s
del helper.s

pdas -o strt32.o strt32.s
pdas -o support.o support.s
pdas -o protints.o protints.s
pdas -o pdoss.o pdoss.s

del os.a
ar386 -r os.a bos.o fat.o exeload.o
ar386 -r os.a physmem.o vmm.o process.o
ar386 -r os.a int21.o int80.o log.o helper.o
ar386 -r os.a memmgr.o patmat.o support.o protintp.o protints.o pdoss.o

ld386 -N -s -e start -o pdos.exe strt32.o pdos.o os.a ../pdpclib/pdos.a
del os.a
