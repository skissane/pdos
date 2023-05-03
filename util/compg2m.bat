gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__WIN32__ gas2masm.c
aswin -o gas2masm.o gas2masm.s
del gas2masm.s

ldwin -o gas2masm.exe ../pdpclib/w32start.o gas2masm.o ../pdpclib/msvcrt.a
