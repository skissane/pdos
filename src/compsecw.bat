gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__WIN32__ sectwrit.c
aswin -o sectwrit.o sectwrit.s
del sectwrit.s

gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__32BIT__ pos.c
aswin -o pos.o pos.s
del pos.s

gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__32BIT__ bos.c
aswin -o bos.o bos.s
del bos.s

aswin -o support.o support.s

ldwin -o sectwrit.exe ../pdpclib/w32start.o sectwrit.o pos.o bos.o support.o ../pdpclib/msvcrt.a
