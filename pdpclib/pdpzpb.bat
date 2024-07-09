gccmvs -Os -U__MVS__ -D__MF32__ -D__ZPDOSGPB__ -DUSE_MEMMGR -S -I . start.c
gccmvs -Os -U__MVS__ -D__MF32__ -D__ZPDOSGPB__ -DUSE_MEMMGR -S -I . stdio.c
gccmvs -Os -U__MVS__ -D__MF32__ -D__ZPDOSGPB__ -DUSE_MEMMGR -S -I . stdlib.c
gccmvs -Os -U__MVS__ -D__MF32__ -D__ZPDOSGPB__ -DUSE_MEMMGR -S -I . ctype.c
gccmvs -Os -U__MVS__ -D__MF32__ -D__ZPDOSGPB__ -DUSE_MEMMGR -S -I . string.c
gccmvs -Os -U__MVS__ -D__MF32__ -D__ZPDOSGPB__ -DUSE_MEMMGR -S -I . time.c
gccmvs -Os -U__MVS__ -D__MF32__ -D__ZPDOSGPB__ -DUSE_MEMMGR -S -I . errno.c
gccmvs -Os -U__MVS__ -D__MF32__ -D__ZPDOSGPB__ -DUSE_MEMMGR -S -I . assert.c
gccmvs -Os -U__MVS__ -D__MF32__ -D__ZPDOSGPB__ -DUSE_MEMMGR -S -I . locale.c
gccmvs -Os -U__MVS__ -D__MF32__ -D__ZPDOSGPB__ -DUSE_MEMMGR -S -I . math.c
gccmvs -Os -U__MVS__ -D__MF32__ -D__ZPDOSGPB__ -DUSE_MEMMGR -S -I . setjmp.c
gccmvs -Os -U__MVS__ -D__MF32__ -D__ZPDOSGPB__ -DUSE_MEMMGR -S -I . signal.c
gccmvs -Os -U__MVS__ -D__MF32__ -D__ZPDOSGPB__ -DUSE_MEMMGR -S -I . __memmgr.c
gccmvs -Os -U__MVS__ -D__MF32__ -D__ZPDOSGPB__ -DUSE_MEMMGR -S -I . pdptest.c
m4 -I . pdpzpb.m4 >pdpzpb.jcl
rem call runmvs pdpzpb.jcl output.txt
