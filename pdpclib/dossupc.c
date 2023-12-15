/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  dossupc.c - some support routines for floating point work        */
/*  for MSDOS                                                        */
/*                                                                   */
/*********************************************************************/

#ifdef __OS2__
int __turboFloat;

long _ftol(double x)
{
    (void)x;
    return (0);
}
#endif

#if defined(__MSDOS__) || defined(__OS2__)

#ifdef __WATCOMC__
#define CTYP __cdecl
#else
#define CTYP
#endif

/* This seems to be the result of x / y */
unsigned long CTYP __divide(unsigned long x, unsigned long y)
{
    unsigned int a;
    unsigned int b;
    unsigned int c;
    unsigned int d;
    unsigned int e;
    unsigned int f;
    unsigned int g = 0;
    unsigned int h;

    b = (unsigned int)(y & 0xffffU);
    a = (unsigned int)(y >> 16);
    c = -b;
    d = a;
    if (b > 0)
    {
        d++;
    }
    f = 1;
    while (f != 0)
    {
        e = (unsigned int)(x >> 16);
        h = (unsigned int)(x & 0xffffU);
        f = e / d;
        g += f;
        e = e % d;
        x = ((unsigned long)e << 16) + (unsigned long)c * f + h;
    }
    if (x >= y)
    {
        g++;
    }
    return ((unsigned long)g);
}

unsigned long CTYP __modulo(unsigned long x, unsigned long y)
{
    unsigned int a;
    unsigned int b;
    unsigned int c;
    unsigned int d;
    unsigned int e;
    unsigned int f;
    unsigned int g = 0;
    unsigned int h;

    b = (unsigned int)(y & 0xffffU);
    a = (unsigned int)(y >> 16);
    c = -b;
    d = a;
    if (b > 0)
    {
        d++;
    }
    f = 1;
    while (f != 0)
    {
        e = (unsigned int)(x >> 16);
        h = (unsigned int)(x & 0xffffU);
        f = e / d;
        g += f;
        e = e % d;
        x = ((unsigned long)e << 16) + (unsigned long)c * f + h;
    }
    if (x >= y)
    {
        x -= y;
    }
    return (x);
}

unsigned int __shift = 12;
unsigned int __incr = 0x1000;

/* dx:ax and cx:bx are huge pointers, and the return should be
   the number of bytes between the two */
unsigned long CTYP __subhphp(unsigned int bx,
                             unsigned int cx,
                             unsigned int ax,
                             unsigned int dx)
{
    unsigned long first;
    unsigned long second;

    second = ((unsigned long)dx << (16 - __shift)) + ax;
    first = ((unsigned long)cx << (16 - __shift)) + bx;
    return (first - second);
}

#ifdef __WATCOMC__
/* dx:ax is a huge pointer to which a long is added, and
   the return should be a normalized huge pointer.
   Actually to support PM16 we need to preserve the
   segment (selector) as priority. So the offset is
   maximized, not minimized. That's the new normalized */
unsigned long CTYP __addhpi(unsigned int dx,
                            unsigned int ax,
                            unsigned int cx,
                            unsigned int bx)
{
    unsigned long first;
   
    first = (unsigned long)ax + bx;
    
    ax = first & 0xffffU;
    
    if (first >= 0x10000UL)
    {
        first = __incr;
    }
    else
    {
        first = 0;
    }

    first += dx;
    first += (cx << __shift);
    first <<= 16U;
    first += ax;

    return (first);
}

/* dx:ax is a huge pointer to which a long is subtracted, and
   the return should be a normalized huge pointer */
/* now we attempt to preserve the segment for PM16 */
unsigned long CTYP __subhpi(unsigned int dx,
                            unsigned int ax,
                            unsigned int cx,
                            unsigned int bx)
{
    unsigned long first;

    first = dx;
    if (ax < bx)
    {
        first -= __incr;
    }
    ax -= bx;

    first -= (cx << (16 - __shift));
    first <<= 16;
    first += ax;

    return (first);
}

/* dx:ax and cx:bx are huge pointers to be compared */
/* return 1, 2 or 3 */
int CTYP __cmphphp(unsigned int dx,
                   unsigned int ax,
                   unsigned int cx,
                   unsigned int bx)
{
    unsigned long first;
    unsigned long second;

    first = ((unsigned long)dx << (16 - __shift)) + ax;
    second = ((unsigned long)cx << (16 - __shift)) + bx;

    if (first < second) return (1);
    else if (first == second) return (2);
    return (3);
}
#endif


#ifdef __WATCOMC__

#if defined(__OS2__) && defined(__16BIT__)
/* do nothing */
#else
void __watcall _cstart(void)
{
    return;
}

int _argc, _8087;
#endif

#endif

#endif

