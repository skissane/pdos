rm -f *.o pdmake.exe
gccwin -S -O2 -fno-common -ansi -I. -I./hashtab -I../pdpclib -D__WIN32__ -o main.s main.c
aswin -o main.o main.s
rm -f main.s
gccwin -S -O2 -fno-common -ansi -I. -I./hashtab -I../pdpclib -D__WIN32__ -o read.s read.c
aswin -o read.o read.s
rm -f read.s
gccwin -S -O2 -fno-common -ansi -I. -I./hashtab -I../pdpclib -D__WIN32__ -o rule.s rule.c
aswin -o rule.o rule.s
rm -f rule.s
gccwin -S -O2 -fno-common -ansi -I. -I./hashtab -I../pdpclib -D__WIN32__ -o variable.s variable.c
aswin -o variable.o variable.s
rm -f variable.s
gccwin -S -O2 -fno-common -ansi -I. -I./hashtab -I../pdpclib -D__WIN32__ -o xmalloc.s xmalloc.c
aswin -o xmalloc.o xmalloc.s
rm -f xmalloc.s
gccwin -S -O2 -fno-common -ansi -I. -I./hashtab -I../pdpclib -D__WIN32__ -o ./hashtab/hashtab.s ./hashtab/hashtab.c
aswin -o hashtab.o ./hashtab/hashtab.s
rm -f ./hashtab/hashtab.s
ldwin  -s -o pdmake.exe ../pdpclib/w32start.o main.o read.o rule.o variable.o xmalloc.o hashtab.o ../pdpclib/msvcrt.a
