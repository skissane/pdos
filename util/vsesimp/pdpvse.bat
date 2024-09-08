rem the output of this is pdptest started/middle/ended and will appear in hercules.log

m4 -I . pdpvse.m4 >pdpvse.jcl
call runvse pdpvse.jcl output.txt none output.dat
