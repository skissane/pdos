del mvsinc.zip
del mvssrc.zip
del mvsjcl.zip
del all.zip
del alljcl.jcl
del output.txt
zip -9 -X -ll -j mvssrc.zip *.c cclib/*.c cpplib/*.c
zip -9 -X -ll -j mvsinc.zip *.h cclib/*.h cclib/include/*.h cpplib/*.h cpplib/include/*.h
zip -9 -X -ll -j mvsjcl.zip *.jcl
zip -9 -X all *.zip

del alljcl.jcl
type makemvs.jcl >>alljcl.jcl
call runmvs alljcl.jcl output.txt all.zip
