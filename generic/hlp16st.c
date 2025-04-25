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

int hlp16st(int val, void *parms)
{
    unsigned long *pblk = parms;
    int (*callb)(int x);

    /* return (val + 2); */
    callb = (int (*)(int))pblk[5];
    return (callb(0));
}
