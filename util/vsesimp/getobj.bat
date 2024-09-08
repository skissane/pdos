rem extract an object file

m4 -I . getobj.m4 >getobj.jcl
call runvse getobj.jcl output.txt none output.dat
