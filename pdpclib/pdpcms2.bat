rem Produces a CMS executable (including RDWs) and then executes it

pdmake -B -f makefile.370

call runcms pdpcms2.exec output.txt pdptest.exe
