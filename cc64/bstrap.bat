gcc -O2 -DBLDGENERIC -o cc64 cc64.c

rem don't attempt to get cc64 to rebuild itself
rem because that requires the non-generic version
rem with the generic version we will have our own
rem build procedure to get cc64 to rebuild itself
rem interim_compiler cc64.c

rem Note that the original cc64.c had an extremely long
rem line in it that exceeded the capability of my editor
rem and may be detected as a binary file. It will be
rem restored to its former glory, with long lines split,
rem in due course

rem some of this should be converted into more standard
rem assembler to be put in x64supb.asm
del bcclib.asm

del interim_compiler.exe
