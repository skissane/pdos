as386 -o fninit.o fninit.s
ld386 -s -N -e start -o fninit.exe fninit.o
