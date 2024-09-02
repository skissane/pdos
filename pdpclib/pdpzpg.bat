rem automatically build and run pdptest for z/pdos-generic
rem will probably change from executable to .bat file when
rem zpg becomes more sophisticated. probably one tape file
rem for the .bat and one tape file for whatever the bat
rem file needs - similar to VM/CMS

pdmake -f makefile.zpg
runzpg pdptest.exe output.txt
