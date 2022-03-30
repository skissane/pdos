#include <stddef.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>

#include "unused.h"
#define fputs(s, b) (s)

void dumpbuf(void *, int);

typedef int FILE;

#define DBL_MANT_DIG 14

static int examine(const char **formt, FILE *fq, char *s, va_list *arg, 
                   int chcount);
static int vvprintf(const char *format, va_list arg, FILE *fq, char *s);
static void dblcvt(double num, char cnvtype, size_t nwidth,
                   int nprecision, char *result);


                   
static void outch(int ch)
{
    unsigned char buf[1];
    
    if (ch == '\n')
    {
        buf[0] = '\r';
        dumpbuf(buf, 1);
    }
    buf[0] = (unsigned char)ch;
    dumpbuf(buf, 1);
    return;
}

int printf(const char *format, ...)
{
    va_list arg;
    int ret;
    char buf[1];
  
    va_start(arg, format);
    ret = vsprintf(buf, format, arg);
    va_end(arg);
    return (ret);
}
    
static int vvprintf(const char *format, va_list arg, FILE *fq, char *s)
{
    int fin = 0;
    int vint;
    double vdbl;
    unsigned int uvint;
    const char *vcptr;
    int chcount = 0;
    size_t len;
    char numbuf[50];
    char *nptr;
    int *viptr;

    while (!fin)
    {
        if (*format == '\0')
        {
            fin = 1;
        }
        else if (*format == '%')
        {
            format++;
            if (*format == 'd')
            {
                vint = va_arg(arg, int);
                if (vint < 0)
                {
                    uvint = -vint;
                }
                else
                {
                    uvint = vint;
                }
                nptr = numbuf;
                do
                {
                    *nptr++ = (char)('0' + uvint % 10);
                    uvint /= 10;
                } while (uvint > 0);
                if (vint < 0)
                {
                    *nptr++ = '-';
                }
                do
                {
                    nptr--;
                    outch(*nptr);
                    chcount++;
                } while (nptr != numbuf);
            }
            else if (strchr("eEgGfF", *format) != NULL && *format != 0)
            {
                vdbl = va_arg(arg, double);
                dblcvt(vdbl, *format, 0, 6, numbuf);   /* 'e','f' etc. */
                len = strlen(numbuf);
                if (fq == NULL)
                {
                    memcpy(s, numbuf, len);
                    s += len;
                }
                else
                {
                    fputs(numbuf, fq);
                }
                chcount += len;
            }
            else if (*format == 's')
            {
                vcptr = va_arg(arg, const char *);
                if (vcptr == NULL)
                {
                    vcptr = "(null)";
                }
                if (fq == NULL)
                {
                    len = strlen(vcptr);
                    dumpbuf(vcptr, len);
/*                    memcpy(s, vcptr, len); */
                    s += len;
                    chcount += len;
                }
                else
                {
                    fputs(vcptr, fq);
                    chcount += strlen(vcptr);
                }
            }
            else if (*format == 'c')
            {
                vint = va_arg(arg, int);
                outch(vint);
                chcount++;
            }
            else if (*format == 'n')
            {
                viptr = va_arg(arg, int *);
                *viptr = chcount;
            }
            else if (*format == '%')
            {
                outch('%');
                chcount++;
            }
            else
            {
                int extraCh;

                extraCh = examine(&format, fq, s, &arg, chcount);
                chcount += extraCh;
                if (s != NULL)
                {
                    s += extraCh;
                }
            }
        }
        else
        {
            outch(*format);
            chcount++;
        }
        format++;
    }
    return (chcount);
}

static int examine(const char **formt, FILE *fq, char *s, va_list *arg,
                   int chcount)
{
    int extraCh = 0;
    int flagMinus = 0;
    int flagPlus = 0;
    int flagSpace = 0;
    int flagHash = 0;
    int flagZero = 0;
    int width = 0;
    int precision = -1;
    int half = 0;
    int lng = 0;
    int specifier = 0;
    int fin;
    long lvalue;
    short int hvalue;
    int ivalue;
    unsigned long ulvalue;
    double vdbl;
    char *svalue;
    char work[50];
    int x;
    int y;
    int rem;
    const char *format;
    int base;
    int fillCh;
    int neg;
    int length;
    size_t slen;

    unused(chcount);
    format = *formt;
    /* processing flags */
    fin = 0;
    while (!fin)
    {
        switch (*format)
        {
            case '-': flagMinus = 1;
                      break;
            case '+': flagPlus = 1;
                      break;
            case ' ': flagSpace = 1;
                      break;
            case '#': flagHash = 1;
                      break;
            case '0': flagZero = 1;
                      break;
            case '*': width = va_arg(*arg, int);
                      if (width < 0)
                      {
                          flagMinus = 1;
                          width = -width;
                      }
                      break;
            default:  fin = 1;
                      break;
        }
        if (!fin)
        {
            format++;
        }
        else
        {
            if (flagSpace && flagPlus)
            {
                flagSpace = 0;
            }
            if (flagMinus)
            {
                flagZero = 0;
            }
        }
    }

    /* processing width */
    if (isdigit((unsigned char)*format))
    {
        while (isdigit((unsigned char)*format))
        {
            width = width * 10 + (*format - '0');
            format++;
        }
    }

    /* processing precision */
    if (*format == '.')
    {
        format++;
        if (*format == '*')
        {
            precision = va_arg(*arg, int);
            format++;
        }
        else
        {
            precision = 0;
            while (isdigit((unsigned char)*format))
            {
                precision = precision * 10 + (*format - '0');
                format++;
            }
        }
    }

    /* processing h/l/L */
    if (*format == 'h')
    {
        /* all environments should promote shorts to ints,
           so we should be able to ignore the 'h' specifier.
           It will create problems otherwise. */
        /* half = 1; */
    }
    else if (*format == 'l')
    {
        lng = 1;
    }
    else if (*format == 'L')
    {
        lng = 1;
    }
    else
    {
        format--;
    }
    format++;

    /* processing specifier */
    specifier = *format;

    if (strchr("dxXuiop", specifier) != NULL && specifier != 0)
    {
        if (precision < 0)
        {
            precision = 1;
        }
#if defined(__MSDOS__) && \
    !(defined(__PDOS__) && !defined(__MVS__)) && \
    !defined(__gnu_linux__)
        if (specifier == 'p')
        {
            lng = 1;
        }
#endif
        if (lng)
        {
            lvalue = va_arg(*arg, long);
        }
        else if (half)
        {
            /* short is promoted to int, so use int */
            hvalue = va_arg(*arg, int);
            if (specifier == 'u') lvalue = (unsigned short)hvalue;
            else lvalue = hvalue;
        }
        else
        {
            ivalue = va_arg(*arg, int);
            if (specifier == 'u') lvalue = (unsigned int)ivalue;
            else lvalue = ivalue;
        }
        ulvalue = (unsigned long)lvalue;
        if ((lvalue < 0) && ((specifier == 'd') || (specifier == 'i')))
        {
            neg = 1;
            ulvalue = -lvalue;
        }
        else
        {
            neg = 0;
        }
#if defined(__MSDOS__) && \
    !(defined(__PDOS__) && !defined(__MVS__)) && \
    !defined(__gnu_linux__)
        if (!lng)
        {
            ulvalue &= 0xffff;
        }
#endif
        if ((specifier == 'X') || (specifier == 'x') || (specifier == 'p'))
        {
            base = 16;
        }
        else if (specifier == 'o')
        {
            base = 8;
        }
        else
        {
            base = 10;
        }
        if (specifier == 'p')
        {
#if defined(__MSDOS__) && \
    !(defined(__PDOS__) && !defined(__MVS__)) && \
    !defined(__gnu_linux__)
            precision = 9;
#else
            precision = 8;
#endif
        }
        x = 0;
        while (ulvalue > 0)
        {
            rem = (int)(ulvalue % base);
            if (rem < 10)
            {
                work[x] = (char)('0' + rem);
            }
            else
            {
                if ((specifier == 'X') || (specifier == 'p'))
                {
                    work[x] = (char)('A' + (rem - 10));
                }
                else
                {
                    work[x] = (char)('a' + (rem - 10));
                }
            }
            x++;
#if defined(__MSDOS__) && \
    !(defined(__PDOS__) && !defined(__MVS__)) && \
    !defined(__gnu_linux__)
            if ((x == 4) && (specifier == 'p'))
            {
                work[x] = ':';
                x++;
            }
#endif
            ulvalue = ulvalue / base;
        }
#if defined(__MSDOS__) && \
    !(defined(__PDOS__) && !defined(__MVS__)) && \
    !defined(__gnu_linux__)
        if (specifier == 'p')
        {
            while (x < 5)
            {
                work[x] = (x == 4) ? ':' : '0';
                x++;
            }
        }
#endif
        while (x < precision)
        {
            work[x] = '0';
            x++;
        }
        if (neg)
        {
            work[x++] = '-';
        }
        else if (flagPlus)
        {
            work[x++] = '+';
        }
        if (flagZero)
        {
            fillCh = '0';
        }
        else
        {
            fillCh = ' ';
        }
        y = x;
        if (!flagMinus)
        {
            while (y < width)
            {
                outch(fillCh);
                extraCh++;
                y++;
            }
        }
        if (flagHash && (toupper((unsigned char)specifier) == 'X'))
        {
            outch('0');
            outch('x');
            extraCh += 2;
        }
        x--;
        while (x >= 0)
        {
            outch(work[x]);
            extraCh++;
            x--;
        }
        if (flagMinus)
        {
            while (y < width)
            {
                outch(fillCh);
                extraCh++;
                y++;
            }
        }
    }
    else if (strchr("eEgGfF", specifier) != NULL && specifier != 0)
    {
        if (precision < 0)
        {
            precision = 6;
        }
        vdbl = va_arg(*arg, double);
        dblcvt(vdbl, specifier, width, precision, work);   /* 'e','f' etc. */
        slen = strlen(work);
        if (fq == NULL)
        {
            memcpy(s, work, slen);
            s += slen;
        }
        else
        {
            fputs(work, fq);
        }
        extraCh += slen;
    }
    else if (specifier == 's')
    {
        svalue = va_arg(*arg, char *);
        fillCh = ' ';
        if (precision > 0)
        {
            char *p;

            p = memchr(svalue, '\0', precision);
            if (p != NULL)
            {
                length = (int)(p - svalue);
            }
            else
            {
                length = precision;
            }
        }
        else if (precision < 0)
        {
            length = strlen(svalue);
        }
        else
        {
            length = 0;
        }
        if (!flagMinus)
        {
            if (length < width)
            {
                extraCh += (width - length);
                for (x = 0; x < (width - length); x++)
                {
                    outch(fillCh);
                }
            }
        }
        for (x = 0; x < length; x++)
        {
            outch(svalue[x]);
        }
        extraCh += length;
        if (flagMinus)
        {
            if (length < width)
            {
                extraCh += (width - length);
                for (x = 0; x < (width - length); x++)
                {
                    outch(fillCh);
                }
            }
        }
    }
    *formt = format;
    return (extraCh);
}


int vsprintf(char *s, const char *format, va_list arg)
{
    int ret;
    
    ret = vvprintf(format, arg, NULL, s);
    return (ret);
}


/*

 The truely cludged piece of code was concocted by Dave Wade

 His erstwhile tutors are probably turning in their graves.

 It is however placed in the Public Domain so that any one
 who wishes to improve is free to do so

*/

static void dblcvt(double num, char cnvtype, size_t nwidth,
            int nprecision, char *result)
{
    double b,round;
    int i,j,exp,pdigits,format;
    char sign, work[45];

    /* save original data & set sign */

    if ( num < 0 )
    {
        b = -num;
        sign = '-';
    }
    else
    {
        b = num;
        sign = ' ';
    }

    /*
      Now scale to get exponent
    */

    exp = 0;
    if( b > 1.0 )
    {
        while ((b >= 10.0) && (exp < 35))
        {
            ++exp;
            b=b / 10.0;
        }
    }
    else if ( b == 0.0 )
    {
        exp=0;
    }
    /* 1.0 will get exp = 0 */
    else if ( b < 1.0 )
    {
        while ((b < 1.0) && (exp > -35))
        {
            --exp;
            b=b*10.0;
        }
    }
    if ((exp <= -35) || (exp >= 35))
    {
        exp = 0;
        b = 0.0;
    }

    /*
      now decide how to print and save in FORMAT.
         -1 => we need leading digits
          0 => print in exp
         +1 => we have digits before dp.
    */

    switch (cnvtype)
    {
        case 'E':
        case 'e':
            format = 0;
            break;
        case 'f':
        case 'F':
            if ( exp >= 0 )
            {
                format = 1;
            }
            else
            {
                format = -1;
            }
            break;
        default:
            /* Style e is used if the exponent from its
               conversion is less than -4 or greater than
               or equal to the precision.
            */
            if ( exp >= 0 )
            {
                if ( nprecision > exp )
                {
                    format=1;
                }
                else
                {
                    format=0;
                }
            }
            else
            {
                /*  if ( nprecision > (-(exp+1) ) ) { */
                if ( exp >= -4)
                {
                    format=-1;
                }
                else
                {
                    format=0;
                }
            }
            break;
    }
    /*
    Now round
    */
    switch (format)
    {
        case 0:    /* we are printing in standard form */
            if (nprecision < DBL_MANT_DIG) /* we need to round */
            {
                j = nprecision;
            }
            else
            {
                j=DBL_MANT_DIG;
            }
            round = 1.0/2.0;
            i = 0;
            while (++i <= j)
            {
                round = round/10.0;
            }
            b = b + round;
            if (b >= 10.0)
            {
                b = b/10.0;
                exp = exp + 1;
            }
            break;

        case 1:      /* we have a number > 1  */
                         /* need to round at the exp + nprecisionth digit */
                if (exp + nprecision < DBL_MANT_DIG) /* we need to round */
                {
                    j = exp + nprecision;
                }
                else
                {
                    j = DBL_MANT_DIG;
                }
                round = 0.5;
                i = 0;
                while (i++ < j)
                {
                    round = round/10;
                }
                b = b + round;
                if (b >= 10.0)
                {
                    b = b/10.0;
                    exp = exp + 1;
                }
                break;

        case -1:   /* we have a number that starts 0.xxxx */
            if (nprecision < DBL_MANT_DIG) /* we need to round */
            {
                j = nprecision + exp + 1;
            }
            else
            {
                j = DBL_MANT_DIG;
            }
            round = 5.0;
            i = 0;
            while (i++ < j)
            {
                round = round/10;
            }
            if (j >= 0)
            {
                b = b + round;
            }
            if (b >= 10.0)
            {
                b = b/10.0;
                exp = exp + 1;
            }
            if (exp >= 0)
            {
                format = 1;
            }
            break;
    }
    /*
       Now extract the requisite number of digits
    */

    if (format==-1)
    {
        /*
             Number < 1.0 so we need to print the "0."
             and the leading zeros...
        */
        result[0]=sign;
        result[1]='0';
        result[2]='.';
        result[3]=0x00;
        while (++exp)
        {
            --nprecision;
            strcat(result,"0");
        }
        i=b;
        --nprecision;
        work[0] = (char)('0' + i % 10);
        work[1] = 0x00;
        strcat(result,work);

        pdigits = nprecision;

        while (pdigits-- > 0)
        {
            b = b - i;
            b = b * 10.0;
            i = b;
            work[0] = (char)('0' + i % 10);
            work[1] = 0x00;
            strcat(result,work);
        }
    }
    /*
       Number >= 1.0 just print the first digit
    */
    else if (format==+1)
    {
        i = b;
        result[0] = sign;
        result[1] = '\0';
        work[0] = (char)('0' + i % 10);
        work[1] = 0x00;
        strcat(result,work);
        nprecision = nprecision + exp;
        pdigits = nprecision ;

        while (pdigits-- > 0)
        {
            if ( ((nprecision-pdigits-1)==exp)  )
            {
                strcat(result,".");
            }
            b = b - i;
            b = b * 10.0;
            i = b;
            work[0] = (char)('0' + i % 10);
            work[1] = 0x00;
            strcat(result,work);
        }
    }
    /*
       printing in standard form
    */
    else
    {
        i = b;
        result[0] = sign;
        result[1] = '\0';
        work[0] = (char)('0' + i % 10);
        work[1] = 0x00;
        strcat(result,work);
        strcat(result,".");

        pdigits = nprecision;

        while (pdigits-- > 0)
        {
            b = b - i;
            b = b * 10.0;
            i = b;
            work[0] = (char)('0' + i % 10);
            work[1] = 0x00;
            strcat(result,work);
        }
    }

    if (format==0)
    { /* exp format - put exp on end */
        work[0] = 'E';
        if ( exp < 0 )
        {
            exp = -exp;
            work[1]= '-';
        }
        else
        {
            work[1]= '+';
        }
        work[2] = (char)('0' + (exp/10) % 10);
        work[3] = (char)('0' + exp % 10);
        work[4] = 0x00;
        strcat(result, work);
    }
    else
    {
        /* get rid of trailing zeros for g specifier */
        if (cnvtype == 'G' || cnvtype == 'g')
        {
            char *p;

            p = strchr(result, '.');
            if (p != NULL)
            {
                p++;
                p = p + strlen(p) - 1;
                while (*p != '.' && *p == '0')
                {
                    *p = '\0';
                    p--;
                }
                if (*p == '.')
                {
                    *p = '\0';
                }
            }
        }
     }
    /* printf(" Final Answer = <%s> fprintf gives=%g\n",
                result,num); */
    /*
     do we need to pad
    */
    if(result[0] == ' ')strcpy(work,result+1); else strcpy(work,result);
    pdigits=nwidth-strlen(work);
    result[0]= 0x00;
    while(pdigits>0)
    {
        strcat(result," ");
        pdigits--;
    }
    strcat(result,work);
    return;
}

long int strtol(const char *nptr, char **endptr, int base);

int atoi(const char *nptr)
{
    return ((int)strtol(nptr, (char **)NULL, 10));
}

long int atol(const char *nptr)
{
    return (strtol(nptr, (char **)NULL, 10));
}

/* this logic is also in vvscanf - if you update this, update
   that one too */

unsigned long int strtoul(
    const char *nptr, char **endptr, int base)
{
    unsigned long x = 0;
    int undecided = 0;

    if (base == 0)
    {
        undecided = 1;
    }
    while (isspace((unsigned char)*nptr))
    {
        nptr++;
    }
    while (1)
    {
        if (isdigit((unsigned char)*nptr))
        {
            if (base == 0)
            {
                if (*nptr == '0')
                {
                    base = 8;
                }
                else
                {
                    base = 10;
                    undecided = 0;
                }
            }
            x = x * base + (*nptr - '0');
            nptr++;
        }
        else if (isalpha((unsigned char)*nptr))
        {
            if ((*nptr == 'X') || (*nptr == 'x'))
            {
                if ((base == 0) || ((base == 8) && undecided))
                {
                    base = 16;
                    undecided = 0;
                    nptr++;
                }
                else if (base == 16)
                {
                    /* hex values are allowed to have an optional 0x */
                    nptr++;
                }
                else
                {
                    break;
                }
            }
            else if (base <= 10)
            {
                break;
            }
            else
            {
                x = x * base + (toupper((unsigned char)*nptr) - 'A') + 10;
                nptr++;
            }
        }
        else
        {
            break;
        }
    }
    if (endptr != NULL)
    {
        *endptr = (char *)nptr;
    }
    return (x);
}

long int strtol(const char *nptr, char **endptr, int base)
{
    unsigned long y;
    long x;
    int neg = 0;

    while (isspace((unsigned char)*nptr))
    {
        nptr++;
    }
    if (*nptr == '-')
    {
        neg = 1;
        nptr++;
    }
    else if (*nptr == '+')
    {
        nptr++;
    }
    y = strtoul(nptr, endptr, base);
    if (neg)
    {
        x = (long)-y;
    }
    else
    {
        x = (long)y;
    }
    return (x);
}
