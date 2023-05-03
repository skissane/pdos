gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__WIN32__ killat.c
aswin -o killat.o killat.s
del killat.s

ldwin -o killat.exe ../pdpclib/w32start.o killat.o ../pdpclib/msvcrt.a
