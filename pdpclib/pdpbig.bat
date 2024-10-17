rem Produces an ELF32 executable for Linux Bigfoot and then executes it
rem For the execution to complete, you will need to do the following things:
rem telnet localhost 3270
rem ipl bigfoot.ins
rem quit

pdmake -f makefile.big

sleep 2

call runbig pdptest.exe output.txt
