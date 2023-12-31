rem in order to run this, first copy pdp370.mac
rem (or possibly others) to pdptop.mac

rem del pdpvse.zip
gccvse -Os -DXXX_MEMMGR -S -I . start.c
gccvse -Os -DXXX_MEMMGR -S -I . stdio.c
gccvse -Os -DXXX_MEMMGR -S -I . stdlib.c
gccvse -Os -DXXX_MEMMGR -S -I . ctype.c
gccvse -Os -DXXX_MEMMGR -S -I . string.c
gccvse -Os -DXXX_MEMMGR -S -I . time.c
gccvse -Os -DXXX_MEMMGR -S -I . errno.c
gccvse -Os -DXXX_MEMMGR -S -I . assert.c
gccvse -Os -DXXX_MEMMGR -S -I . locale.c
gccvse -Os -DXXX_MEMMGR -S -I . math.c
gccvse -Os -DXXX_MEMMGR -S -I . setjmp.c
gccvse -Os -DXXX_MEMMGR -S -I . signal.c
gccvse -Os -DXXX_MEMMGR -S -I . __memmgr.c
gccvse -Os -DXXX_MEMMGR -S -I . pdptest.c
rem zip -0X pdpvse *.s *.exec *.asm *.mac

m4 -I . pdpvse.m4 >pdpvse.jcl
rem call sub pdpvse.jcl
sleep 2
call runvse pdpvse.jcl output.txt
