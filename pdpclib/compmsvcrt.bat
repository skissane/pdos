gcc -c -O2 -D__WIN32__ -D__PDPCLIB_DLL -D__EXPORT__ -I . -I../src dllcrt.c
gcc -c -O2 -D__WIN32__ -D__PDPCLIB_DLL -I . -I../src stdio.c
gcc -c -O2 -D__WIN32__ -D__PDPCLIB_DLL -I . -I../src string.c
gcc -c -O2 -D__WIN32__ -D__PDPCLIB_DLL -I . -I../src stdlib.c
gcc -c -O2 -D__WIN32__ -D__PDPCLIB_DLL -I . -I../src start.c
gcc -c -O2 -D__WIN32__ -D__PDPCLIB_DLL -I . -I../src time.c
gcc -c -O2 -D__WIN32__ -D__PDPCLIB_DLL -I . -I../src errno.c
gcc -c -O2 -D__WIN32__ -D__PDPCLIB_DLL -I . -I../src assert.c
gcc -c -O2 -D__WIN32__ -D__PDPCLIB_DLL -I . -I../src signal.c
gcc -c -O2 -D__WIN32__ -D__PDPCLIB_DLL -I . -I../src locale.c
gcc -c -O2 -D__WIN32__ -D__PDPCLIB_DLL -I . -I../src ctype.c
gcc -c -O2 -D__WIN32__ -D__PDPCLIB_DLL -I . -I../src setjmp.c
gcc -c -O2 -D__WIN32__ -D__PDPCLIB_DLL -I . -I../src math.c
jwasm -c -coff winsupa.asm
gcc -shared -s -nostdlib -o msvcrt.dll dllcrt.o stdio.o string.o stdlib.o start.o time.o errno.o assert.o signal.o locale.o ctype.o setjmp.o math.o winsupa.obj ../src/kernel32.a
echo stdio.o string.o stdlib.o start.o time.o errno.o >dlltool.in
echo assert.o signal.o locale.o ctype.o setjmp.o math.o >>dlltool.in
dlltool -D msvcrt.dll -l msvcrt.a @dlltool.in
del dlltool.in
gcc -c -O2 -D__WIN32__ -D__PDPCLIB_DLL -I . -I../src w32start.c
