del pminc.zip
del pmsrc.zip
del pmjcl.zip
del all.zip
del alljcl.jcl
del output.txt
zip -9 -X -ll -j pmsrc.zip *.txt *.c hashtab/*.c
zip -9 -X -ll -j pminc.zip *.h hashtab/*.h
zip -9 -X -ll -j pmjcl.zip *.jcl
zip -9 -X all *.zip
