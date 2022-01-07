gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__WIN32__ motion.c
aswin -o motion.o motion.s
del motion.s

gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__32BIT__ bos.c
aswin -o bos.o bos.s
del bos.s

aswin -o support.o support.s

ldwin -o motion.exe ../pdpclib/w32start.o motion.o bos.o support.o ../pdpclib/msvcrt.a kernel32.a
