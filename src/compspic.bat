gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__WIN32__ showpic.c
aswin -o showpic.o showpic.s
del showpic.s

gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__32BIT__ bos.c
aswin -o bos.o bos.s
del bos.s

aswin -o support.o support.s

ldwin -o showpic.exe ../pdpclib/w32start.o showpic.o bos.o support.o ../pdpclib/msvcrt.a kernel32.a
