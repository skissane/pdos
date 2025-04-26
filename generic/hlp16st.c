/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  hlp16st - helper16 start of C code                               */
/*                                                                   */
/*********************************************************************/

unsigned long hlp16st(int val, void *parms)
{
    unsigned long *pblk = parms;
    long (*callb)(int x, void *y);

    /* return (val + 2); */
    callb = (long (*)(int, void *))pblk[5];
    *(char **)&pblk[12] = "hi from helper\n";
    /* return ((unsigned long)callb); */
    /* add a dummy string to force a NUL terminator for the
       previous string instead of relying on BSS to be cleared */
    return (callb(0, parms, "dummy"));
}
