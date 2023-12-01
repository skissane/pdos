/* public domain by Paul Edwards */

/* contains definitions required by both pload and pdos */

typedef struct {
    unsigned long transferbuf;
#ifdef __SUBC__
    int dummy1;
#endif
    unsigned long doreboot;
#ifdef __SUBC__
    int dummy2;
#endif
    unsigned long bpb;
#ifdef __SUBC__
    int dummy3;
#endif
    unsigned long dopoweroff;
#ifdef __SUBC__
    int dummy4;
#endif
    unsigned long doboot;
#ifdef __SUBC__
    int dummy5;
#endif
} pdos_parms;
