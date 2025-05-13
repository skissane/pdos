gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__WIN32__ xcopy.c
pdas -o xcopy.obj xcopy.s
del xcopy.s

gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__32BIT__ pos.c
pdas -o pos.obj pos.s
del pos.s

pdas -o support.obj support.s

pdld -o xcopy.exe ../pdpclib/w32start.obj xcopy.obj pos.obj support.obj ../pdpclib/msvcrt.lib
