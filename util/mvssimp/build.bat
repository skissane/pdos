m4 -I . build.m4 >build.jcl

call runmvs build.jcl output.txt none pdptest.exe
