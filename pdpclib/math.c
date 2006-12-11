/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*  9-April-2006 D.Wade                                              */
/*      Moved definitions for HUGE_VAL to math.h                   */
/*      Inserted argument rang checks in :-                          */
/*       acos                                                        */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*  2-April-2006 D.Wade added code for the :-                        */
/*                                                                   */
/*      acos(double x);                                              */
/*      asin(double x);                                              */
/*      atan(double x);                                              */
/*      cos(double x);                                               */
/*      sin(double x);                                               */
/*      tan(double x);                                               */
/*      cosh(double x);                                              */
/*      sinh(double x);                                              */
/*      tanh(double x);                                              */
/*      exp(double x);                                               */
/*      frexp(double value, int *exp);                               */
/*      ldexp(double x, int exp);                                    */
/*      log(double x);                                               */
/*      log10(double x);                                             */
/*      modf(double value, double *iptr);                            */
/*      pow(double x, double y);                                     */
/*      sqrt(double x);                                              */
/*                                                                   */
/* Note:-                                                            */
/*  In order to avoide Copyright these functions are generally       */
/*  implemented using Taylor Series. As a result they are a little   */
/*  slower that the equivalents in many maths packages.              */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  math.c - implementation of stuff in math.h                       */
/*                                                                   */
/*********************************************************************/

#include "math.h"
#include "float.h"
#include "errno.h"

/*

  Some constants to make life easier elsewhere
  (These should I guess be in math.h)

*/
static const double pi   = 3.1415926535897932384626433832795;
static const double ln10 = 2.3025850929940456840179914546844;


double ceil(double x)
{
    int y;

    y = (int)x;
    if ((double)y < x)
    {
        y++;
    }
    return ((double)y);
}

#ifdef fabs
#undef fabs
#endif
double fabs(double x)
{
    if (x < 0.0)
    {
        x = -x;
    }
    return (x);
}

double floor(double x)
{
    int y;

    if (x < 0.0)
    {
        y = (int)x;
        if ((double)y != x)
        {
            y--;
        }
    }
    else
    {
        y = (int)x;
    }
    return ((double)y);
}

double fmod(double x, double y)
{
    return (x / y);
}

#ifdef acos
#undef acos
#endif
/*

  For cos just use (sin(x)**2 + cos(x)**2)=1
  Note:- asin(x) decides which taylor series
  to use to ensure quickest convergence.

*/
double acos(double x)
{

/*
*/

    if ( fabs(x) > 1.0 ) /* is argument out of range */
    {
        errno=EDOM;
        return (HUGE_VAL);
    }
    if ( x < 0.0 ) return ( pi - acos(-x) ) ;

    return ( asin ( sqrt(1.0 - x*x) ) );

}

#ifdef asin
#undef asin
#endif
/*

   This routines Calculate arcsin(x) & arccos(x).

   Note if "x" is close to "1" the series converges slowly.
   To avoid this we use (sin(x)**2 + cos(x)**2)=1
   and fact cos(x)=sin(x+pi/2)

*/

double asin (double y)
{
    int i,scale;
    double term,answer,work,x,powx,coef;

    x = y;

/*
  if arg is -ve then we want "-asin(-x)"
*/

    if (x <0.0 ) return ( -asin(-x) );

/*
    If arg is > 1.0 we can't calculate
    (note also < -1.0 but previous statement removes this case)
*/
    if ( x > 1.0 )
    {
        errno=EDOM;
        return(HUGE_VAL);
    }

/*
 now check for large(ish) x > 0.6
*/

    if( x > 0.75 )
    {
        x = ( sqrt(1.0 - (x*x) ) );
        return((pi/2.0)-asin(x));
    }

/*
     arcsin(x) = x + 1/2 (x^3/3) + (1/2)(3/4)(x^5/5) +
        (1/2)(3/4)(5/6)(x^7/7) + ...
*/
    i=1;
    answer=x;
    term = 1;
    coef = 1;
    powx = x;

    while (1)
    {
        work = i;
        coef = (coef * work)/(work+1);
        powx = powx * x * x;
        term =  coef * powx / (work + 2.0);
        if ( answer == (answer + term) )break;
        answer = answer + (term);
        i+=2;
    }

    return(answer);
}


#ifdef atan
#undef atan
#endif
/*

     Because atan(x) is valid for large values of "x" &
     the taylor series converges more slowly for large "X"
     we use the following

     1. Reduce to the first octant by using :-

        atan(-x)=-atan(x),
        atan(1/x)=PI/2-atan(x)

     2. Reduce further so that |x| less than tan(PI/12)

        atan(x)=pi/6+atan((X*sqrt(3)-1)/(x+sqrt(3)))

     3. Then use the taylor series

        atan(x) = x - x**3 + x**5 - x**7
                      ----   ----   ----
                        3      5      7

*/

double atan (double x)
{
    int i,scale;
    double term,answer,work,powx,coef;

/*
  if arg is -ve then we want "-atan(-x)"
*/

    if ( x<0.0 ) return ( -atan(-x) );

/*
 If arg is large we can't calculate
 use atan(1/x)=PI/2-atan(x)
*/

    if ( x > 1.0 ) return ((pi/2) - atan(1.0/x));

/*
 now check for large(ish) x > tan(15) (0.26794919243112)
 if so use atan(x)=pi/6+atan((X*SQRT3-1)/(X+SQRT3))
*/

    if( x > (2.0 - sqrt(3.0)))
    return( (pi/6.0) + atan( ( x * sqrt(3.0)-1.0 ) / (x + sqrt(3.0) ) ) );

/*
*       atan(x) = x - x**3 + x**5 - x**7
*                     ----   ----   ----
*                       3      5      7
*/

    i=1;
    answer=x;
    term = x;
    powx = x;

    while (1)
    {
        work = i;
        powx = 0.0 - powx * x * x;
        term = powx / (work + 2.0);
        if ( answer == (answer + term) )break;
        answer = answer + (term);
        i+=2;
    }

    return(answer);

}


/* atan2 was taken from libnix and modified slightly */

double atan2(double y,double x)
{ 
    return (x >= y) ?
               (x >= -y ? atan(y/x) : -pi/2 - atan(x/y))
              :
               (x >= -y ? pi/2 - atan(x/y) 
                        : (y >= 0) ? pi + atan(y/x)
                                   : -pi + atan(y/x));
}


#ifdef cos
#undef cos
#endif
double cos(double x)
{
/*

   Calculate COS using Taylor series.

   sin(x) = 1 - x**2  +  x**4  - x**6 + x**8
                ====     ====    ====   ====    .........
                  2!       4!      6!     8!

   Note whilst this is accurate it can be slow for large
   values of "X" so we scale

*/

    int i;
    double term,answer,work,x1;

/*
    Scale arguments to be in range 1 => pi
*/

    i = x/(2*pi);
    x1 =  x - (i * pi);

    i=1;
    term=answer=1;


    while (1)
    {
        work = i;
        term = -(term * x1 * x1)/(work * (work + 1.0));
        if ( answer == (answer + term) )break;
        answer = answer + term;
        i += 2;
    }

    return(answer);

}

#ifdef sin
#undef sin
#endif
double sin(double x)
{
/*

   Calculate SIN using Taylor series.

   sin(x) = x - x**3  +  x**5  - x**7 + x**9
                ====     ====    ====   ====
                  3!       5!      7!     9!

   Note whilst this is accurate it can be slow for large values
   of "X" so we scale

*/

    int i;
    double term,answer,work,x1;

/*
  scale so series converges pretty quickly
*/
    i = x/(2.0*pi);
    x1 =  x - (i * pi);

/*
 set up initial term
*/
    i=1;
    term=answer=x1;
/*
 loop until no more changes
*/
    while (1)
    {
        work = i+1;
        term = -(term * x1 * x1)/(work * (work + 1.0));
        if ( answer == (answer + term) )break;
        answer = answer + term;
        i = i+2;
    }

    return(answer);
}

#ifdef tan
#undef tan
#endif
double tan (double x)
{
/*

  use tan = sin(x)/cos(x)
  if cos(x) is 0 then return HUGE_VAL else return sin/cos

  *** need to set ERROR for overflow ***

*/
    double temp;

    temp=cos(x);
    if (temp == 0.0 )
    {
        /* errno=EDOM; don't seem to return an error here */
        return (HUGE_VAL); /* need to set error here */
    }
    return ( sin(x)/cos(x) );
}

/*

  Hyperbolic functions

  SINH(X) = (E**X-E**(-1))/2
  COSH(X) = (E**X+E**(-1))/2

*/
double cosh(double x)
{
    double dexpx;

    dexpx = exp(x);

    return( 0.5 * (dexpx + (1.0/dexpx) ) );

}

double sinh(double x)
{
    double dexpx;

    dexpx = exp(x);

    return( 0.5 * (dexpx - (1.0/dexpx) ) );
}

/*

    tanh returns the hyperbolic area tangent of floating point argument x.

*/

double tanh(double x)
{
    double y;
    double dexp2;

    if ( (x <= -1.0 ) || (x >= 1.0) ) return(0.0); /* need
                                   to set an error here */

    dexp2 = exp( -2.0 * x);
    return ( (1.0  - dexp2) /  (1.0 + dexp2) );
}

/*

exp(x) = 1 + x + x2/2 + x3/6 + x4/24 + x5/120 + ... + xn/n! + ...

*/
double exp (double x)
{
    int i;
    double term,answer,work;

    i=1;
    term=answer=1;

    while (1)
    {
        work = i;
        term =  (term * x)/work;
        if ( answer == (answer + term) )break;
        answer = answer + (term);
        i++;
    }

    return(answer);
}

/*

   Calculate LOG using Taylor series.

   log(1+ x) = x - x**2  +  x**3  - x**4 + x**5
                   ====     ====    ====   ====    .........
                    2         3       4     8

   Note this only works for small x so we scale....

*/
double log (double x)
{
    int i,scale;
    double term,answer,work,xs;

    if (x <= 0 )
    {
        /* need to set signal */
        errno=EDOM;
        return (HUGE_VAL);
    }

/*
  Scale arguments to be in range 1 < x <= 10
*/

    scale = 0;
    xs = x;
    while ( xs > 1.0 ) { scale ++; xs=xs/10.0;}
    while ( xs < 0.1 ) { scale --; xs=xs*10.0;}

    xs = xs - 1;

    i=2;
    term=answer=xs;

    while (1)
    {
        work = i;
        term = - (term * xs);
        if ( answer == (answer + (term/work)) )break;
        answer = answer + (term/work);
        i++;
    }

    answer = answer + (double)scale * ln10;
    return(answer);
}


double log10(double x)
{
    return ( log(x) / ln10 );
}


/*

   This code uses log and exp to calculate x to the power y.
   If

*/

double pow(double x,double y)
{
    if (x < 0.0)
    {
         errno=EDOM;
         return(0.0);
    }
    if (y == 0.0) return (1.0);

    return (exp(y*log(x)));
}

#ifdef sqrt
#undef sqrt
#endif
/*

   pretty tivial code here.

     1) Scale x such that 1 <= x <= 4.0

     2) Use newton Raphson to calculate root.

     4) multiply back up.

   Because we only scale by "4" this is pretty slow....

*/

double sqrt(double x)
{
    double xs,yn,ynn;
    double pow1;
    int i;

    if (x < 0.0)
    {
        errno=EDOM;
        return(0.0);
    }
    if (x == 0.0) return (0.0);

/*

  Scale argument 1 <= x <= 4

*/

    xs=x;
    pow1=1;

    while(xs<1.0){xs=xs*4.0;pow1=pow1/2.0;}
    while(xs>=4.0){xs=xs/4.0;pow1=pow1*2.0;}

/*
  calculate using Newton raphson
  use x0 = x/2.0
*/

    i=0;
    yn = xs/2.0;
    ynn = 0;
    while(1)
    {
        ynn = (yn + xs/yn)*0.5;
        if ( fabs(ynn-yn) <= 10.0 * DBL_MIN ) break; else yn=ynn;
        if ( i > 10  ) break; else i++ ;
    }
    return (ynn*pow1);
}


double frexp(double x, int *exp)
{
/*
  split float into fraction and mantissa
  note this is not so easy for IBM as it uses HEX float
*/
    double xf;
    union dblhex
    {
        double d;
        unsigned short s[4];
    };
    union dblhex split;

    split.d = x;
    *exp = (((split.s[0] >> 8) & 0x007f)-64) * 4;
    split.s[0] = split.s[0] & 0x80ff;
    split.s[0] = split.s[0] | 0x4000;
    /* following code adjust for fact IBM has hex float */
    while ( fabs(split.d) < 0.5 )
    {
        split.d = split.d * 2;
        *exp =( *exp ) - 1;
    }
    /*    */
    return(split.d);
}

double ldexp(double x, int exp)
{
/*
  note this is not so easy for IBM as it uses HEX float
*/
    double xf;
    int bin_exp,hex_exp,adj_exp;
    union dblhex
    {
        double d;
        unsigned short s[4];
    };
    union dblhex split;
/*
    note "X" mauy already have an exponent => extract it
*/
    split.d = frexp(x,&bin_exp);
    bin_exp = bin_exp + exp;  /* add in from caller */
/* need to test for sensible value here */
    hex_exp =  (bin_exp / 4); /* convert back to HEX */
    adj_exp =  bin_exp - (hex_exp * 4);
    if (adj_exp < 0){ hex_exp=hex_exp -1; adj_exp = 4 + adj_exp;}
    split.s[0] = split.s[0] & 0x80ff;
    split.s[0] = split.s[0] | (((hex_exp+64)  << 8) & 0x7f00);
    /* following code adjust for fact IBM has hex float */
    /* well it will I have done */
    while ( adj_exp > 0 )
    {
        split.d = split.d * 2;
        --adj_exp;
    }
    /**/
    return(split.d);
}

/* modf was taken from libnix and modified slightly */
double modf(double value, double *iptr)
{
    if (value < 0)
    {
        *iptr = ceil(value);
        return (*iptr-value);
    }
    else
    {
        *iptr=floor(value);
        return (value-*iptr);
    }
}
