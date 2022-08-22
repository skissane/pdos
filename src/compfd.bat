gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__WIN32__ fdisk.c
aswin -o fdisk.o fdisk.s
del fdisk.s

ldwin -o fdisk.exe ../pdpclib/w32start.o fdisk.o ../pdpclib/msvcrt.a
