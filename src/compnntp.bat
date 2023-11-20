gccwin -O2 -S -fno-common -ansi -I. -I../pdpclib -D__WIN32__ pdpnntp.c
pdas -o pdpnntp.obj --oformat coff pdpnntp.s
del pdpnntp.s

pdld -s -nostdlib --no-insert-timestamp -o pdpnntp.exe ../pdpclib/w32start.obj pdpnntp.obj ../pdpclib/msvcrt.lib
