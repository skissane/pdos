del pdpcms.zip
gcccms -DXXX_MEMMGR -S -I . start.c
gcccms -DXXX_MEMMGR -S -I . stdio.c
gcccms -DXXX_MEMMGR -S -I . stdlib.c
gcccms -DXXX_MEMMGR -S -I . ctype.c
gcccms -DXXX_MEMMGR -S -I . string.c
gcccms -DXXX_MEMMGR -S -I . time.c
gcccms -DXXX_MEMMGR -S -I . errno.c
gcccms -DXXX_MEMMGR -S -I . assert.c
gcccms -DXXX_MEMMGR -S -I . locale.c
gcccms -DXXX_MEMMGR -S -I . math.c
gcccms -DXXX_MEMMGR -S -I . setjmp.c
gcccms -DXXX_MEMMGR -S -I . signal.c
gcccms -DXXX_MEMMGR -S -I . pdptest.c
zip -0X pdpcms *.s *.exec *.asm *.mac
call runcms pdpcms.exec output.txt pdpcms.zip
