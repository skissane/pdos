/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  atrsupc.c - support routines for Atari (TOS/GEMDOS)              */
/*                                                                   */
/*********************************************************************/


static long t1blng[6]; /* reserve 24 bytes, 4-byte-aligned */
static unsigned char *t1buf = (unsigned char *)&t1blng;

long __asmt1(short cnt, void *s);
long (*__trap1)(short cnt, void *s) = __asmt1;

long __Fwrite(int handle, long towrite, void *ptr)
{
    struct { short opcode;
             short handle;
             long count;
             void *buf; } *s = (void *)t1buf;

    s->opcode = 64;
    s->handle = handle;
    s->count = towrite;
    s->buf = ptr;
    return (__trap1(12, s));
}
