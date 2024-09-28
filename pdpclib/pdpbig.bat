rem Produces an ELF32 executable for Linux Bigfoot and then executes it

pdmake -f makefile.big

sleep 2

call runbig pdptest.exe output.txt
