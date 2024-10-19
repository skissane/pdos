/*********************************************************************/
/*                                                                   */
/*  This Program Written By Paul Edwards.                            */
/*  Released to the public domain.                                   */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  mfemul - emulate a mainframe                                     */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define DUMPREGS 1

#if COMEMUL
#define MAXPRGSZ (10 * 0x10000)
#endif

#if PBEMUL
static size_t memsize = 64*1024*1024L;
#endif

typedef unsigned int U32;

#define getfullword(p) (((p)[0] << 24) | ((p)[1] << 16) | ((p)[2] << 8) | (p)[3])
#define putfullword(p, val) ((p)[0] = (unsigned char)(((val) >> 24) & 0xff), \
                         (p)[1] = (unsigned char)(((val) >> 16) & 0xff), \
                         (p)[2] = (unsigned char)(((val) >> 8) & 0xff), \
                         (p)[3] = (unsigned char)((val) & 0xff))

#define gethalfword(p) (((p)[0] << 8) | (p)[1])
#define puthalfword(p, val) ((p)[0] = (unsigned char)(((val) >> 8) & 0xff), \
                         (p)[1] = (unsigned char)((val) & 0xff))

extern int use_arbitrary;
extern unsigned long arbitrary_base;

static unsigned char *base;
static unsigned char *p;
static int instr;

static int regs[16];

static int x1;
static int x2;
static int t;
static int i;
static int b;
static int d;
static int b1;
static int d1;
static int b2;
static int d2;
static int l;

static int lt;
static int gt;
static int eq;
static int zero;

static void doemul(void);
static void splitrx(void);
static void splitrs(void);
static void splitrr(void);
static void splitssl(void);
static void writereg(unsigned char *z, int x);
static void updatereg(int *x, unsigned char *z);
static int febc(int ebc);

int main(int argc, char **argv)
{
    FILE *fp;
    unsigned char *entry_point;
    char *exe;

    if (argc <= 1)
    {
        printf("usage: mfemul <COM>\n");
        printf("executes a mainframe .com file\n");
        return (EXIT_FAILURE);
    }

#if COMEMUL
    fp = fopen(argv[1], "rb");
    if (fp == NULL)
    {
        printf("cannot open %s for reading\n", argv[1]);
        return (EXIT_FAILURE);
    }
#endif

#if COMEMUL
    p = calloc(1, MAXPRGSZ + 0x10000);
#endif

#if PBEMUL
    p = calloc(1, memsize);
    printf("memory starts at %p\n", p);
#endif

    if (p == NULL)
    {
        printf("insufficient memory\n");
        return (EXIT_FAILURE);
    }

#if COMEMUL
    fread(p + 0x10000, 1, MAXPRGSZ, fp);
    fclose(fp);
    base = p;
    p += 0x10000;
#endif

#if PBEMUL
    exe = argv[1];
    base = p;
    p = base + 1024 * 1024L;
    {
        int x;
    
        for (x = 1; x < 300; x++)
        {
            putfullword(p, x);
            p += sizeof(U32);
        }
    }

    arbitrary_base = 2 * 1024 * 1024L;
    use_arbitrary = 1;
    p = base + arbitrary_base;
    if (exeloadDoload(&entry_point, exe, &p) != 0)
    {
        printf("failed to load %s\n", exe);
        exit(EXIT_FAILURE);
    }
    regs[15] = (int)(entry_point - p + arbitrary_base);
    putfullword(base + 0x80, 1 * 1024 * 1024L);
    regs[1] = 0x80;
    p = entry_point;
#endif

    doemul();
#if PBEMUL
    printf("program returned with hex %x\n", regs[15]);
#endif
    return (EXIT_SUCCESS);
}

static void spec_call(int val)
{
    unsigned long parms;

    printf("got a special call %d\n", val);
    parms = regs[1];
    printf("parms are %08lX\n", parms);

    if (val == 14) /* fwrite */
    {
        unsigned char *buf;
        size_t sz;
        size_t num;
        FILE *fq;
        
        buf = base + getfullword(base + parms);
        printf("buf is %p\n", buf);

        sz = (size_t)getfullword(base + parms + sizeof(U32));
        printf("sz is %x\n", (int)sz);
        num = (size_t)getfullword(base + parms + sizeof(U32)*2);
        printf("num is %x\n", (int)num);
        fq = (FILE *)getfullword(base + parms + sizeof(U32)*3);
        printf("fq is %p\n", (int)fq);
        /* This should instead be a table of handles, where stdout
           should have been set to 1 in the POS structure */
        if (fq == (void *)9)
        {
            fq = stdout;
            {
                size_t tot;
                size_t x;

                tot = sz * num;
                for (x = 0; x < tot; x++)
                {
                    fputc(febc(buf[x]), fq);
                }
            }
            /* fwrite(buf, sz, num, fq); */
        }
    }

    return;

}

static void doemul(void)
{
    unsigned char *watching = base + 0x10086 + 3; /*0x1007a; */
    
#if COMEMUL
    regs[13] = 0x100; /* save area */
    regs[15] = 0x10000; /* entry point */
#endif
#if PBEMUL
    regs[13] = 0x100; /* save area */
    putfullword(base + 0x100 + 76, 0x100 + 80);
#endif
    regs[14] = 0; /* branch to zero will terminate */

    while (1)
    {
        instr = *p;
#if COMEMUL
        printf("instr is %02X at %08X watching %02X, r14 %08X\n", instr, p - base - 0x10000, *watching, regs[14]);
#endif
#if PBEMUL
        printf("\ninstr is %02X at %08X watching %02X, r14 %08X\n", instr, p - base, *watching, regs[14]);
#endif
#if DUMPREGS
        printf("R0: %08X, R1: %08X, R2: %08X, R3: %08X\n", regs[0], regs[1], regs[2], regs[3]);
        printf("R4: %08X, R5: %08X, R6: %08X, R7: %08X\n", regs[4], regs[5], regs[6], regs[7]);
        printf("R8: %08X, R9: %08X, R10: %08X, R11: %08X\n", regs[8], regs[9], regs[10], regs[11]);
        printf("R12: %08X, R13: %08X, R14: %08X, R15: %08X\n", regs[12], regs[13], regs[14], regs[15]);
#endif
        if (instr == 0x07) /* bcr */
        {
            splitrr();
            /* x1 is condition, x2 is register to branch to */
            if ((x1 == 0) || (x2 == 0))
            {
                printf("noop\n");
            }
            /* unconditional branch */
            else if (x1 == 0xf)
            {
                p = base + regs[x2];
                printf("updating with %x %x\n", x2, regs[x2]);
                printf("base %p, p %p, at p is %x\n", base, p, *p);
                if ((p - base) < 0x10000)
                {
                    printf("branched below - terminating\n");
#if PBEMUL
                    return;
#endif
                    exit(EXIT_SUCCESS);
                }
                continue;
            }
            else
            {
                printf("unknown instruction %02X %02X\n", p[0], p[1]);
                exit(EXIT_FAILURE);
            }
            p += 2;
        }
        else if (instr == 0x05) /* balr */
        {
            splitrr();
            if (x1 != 0)
            {
                regs[x1] = (p - base) + 2;
            }
            if (x2 != 0)
            {
                printf("x2 is %x, regsx2 is %x\n", x2, regs[x2]);
#if PBEMUL
                if (regs[x2] < 300)
                {
                    printf("special call\n");
                    spec_call(regs[x2]);
                }
                p += 2;
#endif
#if COMEMUL
                p = base + regs[x2];
                printf("new address is %08X\n", regs[x2] - 0x10000);
#endif
                continue;
            }
            p += 2;
        }
        /* this is used for looping. subtract 1 from first register.
           if 0, fall through. otherwise branch to second register */
        else if (instr == 0x06) /* bctr */
        {
            splitrr();
            regs[x1]--;
            printf("new value of %x is %08X\n", x1, regs[x1]);
            if (regs[x1] != 0)
            {
                if (x2 != 0)
                {
                    p = base + regs[x2];
                    if ((p - base) < 0x10000)
                    {
                        printf("branched below - terminating\n");
                        exit(EXIT_SUCCESS);
                    }
                    continue;
                }
            }
            p += 2;
        }
        else if (instr == 0x41) /* la */
        {
            int one = 0;
            int two = 0;

            splitrx();            
            if (b != 0)
            {
                one = regs[b];
            }
            if (i != 0)
            {
                two = regs[i];
            }
            regs[t] = one + two + d;
            printf("new value of %x is %08X\n", t, regs[t]);
            p += 4;
        }
        else if (instr == 0x90) /* stm */
        {
            int start;
            int end;
            int x;
            unsigned char *target;
            int one = 0;
            
            splitrs();
            start = x1;
            end = x2;
            if (b != 0)
            {
                one = regs[b];
            }
            target = base + one + d;
            printf("storing to offset %x\n", (target - base));
            printf("base %x, displacement %x\n", one, d);
            if (x2 < x1)
            {
                end = 15;
                for (x = start; x <= end; x++)
                {
                   writereg(target, regs[x]);
                   target += 4;
                }
                start = 0;
                end = x2;
            }
            for (x = start; x <= end; x++)
            {
                writereg(target, regs[x]);
                target += 4;
            }
            p += 4;
        }
        else if (instr == 0x18) /* lr */
        {
            splitrr();
            regs[x1] = regs[x2];
            p += 2;
        }
        else if (instr == 0x58) /* l */
        {
            int one = 0;
            int two = 0;
            unsigned char *v;

            splitrx();
            if (b != 0)
            {
                one = regs[b];
            }
            if (i != 0)
            {
                two = regs[i];
            }
            v = base + one + two + d;
            regs[t] = (v[0] << 24) | (v[1] << 16) | (v[2] << 8) | v[3];
            printf("new value of %x is %08X\n", t, regs[t]);
            p += 4;
        }
        else if (instr == 0x5a) /* a */
        {
            int one = 0;
            int two = 0;
            unsigned char *v;

            splitrx();
            if (b != 0)
            {
                one = regs[b];
            }
            if (i != 0)
            {
                two = regs[i];
            }
            v = base + one + two + d;
            regs[t] += (v[0] << 24) | (v[1] << 16) | (v[2] << 8) | v[3];
            printf("new value of %x is %08X\n", t, regs[t]);
            p += 4;
        }
        else if (instr == 0x1a) /* ar */
        {
            splitrr();
            regs[x1] += regs[x2];
            p += 2;
        }
        else if (instr == 0x1b) /* sr */
        {
            splitrr();
            regs[x1] -= regs[x2];
            p += 2;
        }
        else if (instr == 0x45) /* bal */
        {
            int one = 0;
            int two = 0;
            int dest;

            splitrx();            
            if (b != 0)
            {
                one = regs[b];
            }
            if (i != 0)
            {
                two = regs[i];
            }
            dest = one + two + d;
            if (t != 0)
            {
                regs[t] = p + 4 - base;
                printf("new value of %x is %08X\n", t, regs[t]);
            }
            printf("new dest is %08X\n", dest - 0x10000);
            p = base + dest;
        }
        else if (instr == 0x50) /* st */
        {
            int one = 0;
            int two = 0;
            unsigned char *v;

            splitrx();
            if (b != 0)
            {
                one = regs[b];
            }
            if (i != 0)
            {
                two = regs[i];
            }
            v = base + one + two + d;
            v[0] = (regs[t] >> 24) & 0xff;
            v[1] = (regs[t] >> 16) & 0xff;
            v[2] = (regs[t] >> 8) & 0xff;
            v[3] = regs[t] & 0xff;
            p += 4;
        }
        else if (instr == 0xd2) /* mvc */
        {
            int one = 0;
            int two = 0;
            unsigned char *v;

            splitssl();
            if (b1 != 0)
            {
                one = regs[b1];
            }
            one += d1;
            if (b2 != 0)
            {
                two = regs[b2];
            }
            two += d2;
            memcpy(base + one, base + two, l + 1);
            printf("writing to address %x %p\n", one, base + one);
            p += 6;
        }
        else if (instr == 0x19) /* cr */
        {
            splitrr();
            lt = (regs[x1] < regs[x2]);
            gt = (regs[x1] > regs[x2]);
            eq = (regs[x1] == regs[x2]);
            p += 2;
        }
        else if (instr == 0x47) /* bc */
        {
            int branch = 0;
            int one = 0;
            int cond;

            splitrx();
            /* not sure if this is subject to r0 check */
            if (b != 0)
            {
                one = regs[b];
            }
            cond = (t << 4) | i;
            /* bl */
            if (cond == 0x40)
            {
                if (lt)
                {
                    p = base + one + d;
                    continue;
                }
            }
            /* bnz */
            else if (cond == 0x70)
            {
                if (!zero)
                {
                    p = base + one + d;
                    continue;
                }
            }
            /* unconditional */
            else if (cond == 0xf0)
            {
                p = base + one + d;
                continue;
            }
            else
            {
                printf("unknown condition %x\n", cond);
                exit(EXIT_FAILURE);
            }
            p += 4;
        }
        else if (instr == 0x12) /* ltr */
        {
            splitrr();
            regs[x1] = regs[x2];
            zero = (regs[x1] == 0);
            p += 2;
        }
        else if (instr == 0x98) /* lm */
        {
            int start;
            int end;
            int x;
            unsigned char *target;
            int one = 0;
            
            splitrs();
            start = x1;
            end = x2;
            if (b != 0)
            {
                one = regs[b];
            }
            target = base + one + d;
            printf("loading from offset %x\n", (target - base));
            printf("displacement %x\n", d);
            if (x2 < x1)
            {
                end = 15;
                for (x = start; x <= end; x++)
                {
                   printf("updating reg %x currently %x\n", x, regs[x]);
                   updatereg(&regs[x], target);
                   printf("new value %x\n", regs[x]);
                   target += 4;
                }
                start = 0;
                end = x2;
            }
            for (x = start; x <= end; x++)
            {
                updatereg(&regs[x], target);
                target += 4;
            }
            p += 4;
        }
        else if (instr == 0x42) /* stc */
        {
            int one = 0;
            int two = 0;
            int dest;

            splitrx();            
            if (b != 0)
            {
                one = regs[b];
            }
            if (i != 0)
            {
                two = regs[i];
            }
            dest = one + two + d;
            base[dest] = regs[t] & 0xff;
            p += 4;
        }
        else if (instr == 0x0a) /* svc */
        {
            int c;
            
            splitrr();
            /* assume SVC 35 writing one character */
            c = base[regs[1] + 4];
            printf("WTO: %c\n", febc(c));
            p += 2;
        }
        else
        {
#if COMEMUL
            printf("unknown instruction %02X at %08X\n", p[0],
                   p - base - 0x10000);
#endif
#if PBEMUL
            printf("unknown instruction %02X at %08X\n", p[0],
                   p - base);
#endif
            exit(EXIT_FAILURE);
        }
    }
    return;
}

static void splitrx(void)
{
    t = (p[1] >> 4) & 0x0f;
    i = p[1] & 0x0f;
    b = (p[2] >> 4) & 0x0f;
    d = (p[2] & 0xf) << 8 | p[3];
    return;
}

static void splitrs(void)
{
    x1 = (p[1] >> 4) & 0x0f;
    x2 = p[1] & 0x0f;
    b = (p[2] >> 4) & 0x0f;
    d = (p[2] & 0xf) << 8 | p[3];
    return;
}

static void splitrr(void)
{
    x1 = (p[1] >> 4) & 0x0f;
    x2 = p[1] & 0x0f;
    return;
}

static void splitssl(void)
{
    l = p[1];
    b1 = (p[2] >> 4) & 0x0f;
    d1 = (p[2] & 0xf) << 8 | p[3];
    b2 = (p[4] >> 4) & 0x0f;
    d2 = (p[4] & 0xf) << 8 | p[5];
    return;
}

static void writereg(unsigned char *z, int x)
{
    z[0] = (x >> 24) & 0xff;
    z[1] = (x >> 16) & 0xff;
    z[2] = (x >> 8) & 0xff;
    z[3] = x & 0xff;
    return;
}

static void updatereg(int *x, unsigned char *z)
{
    *x = (z[0] << 24) | (z[1] << 16) | (z[2] << 8) | z[3];
    return;
}

/*********************************************************************/
/*                                                                   */
/*  This Program Written By Paul Edwards.                            */
/*  Released to the public domain.                                   */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  This program takes an integer as a parameter.  The integer       */
/*  should have a representation of an EBCDIC character.  An integer */
/*  is returned, containing the representation of that character     */
/*  in whatever character set you are using on this system.          */
/*                                                                   */
/*********************************************************************/
static int febc(int ebc)
{
  switch (ebc)
  {
    case 0x15 : return('\n');
    case 0x25 : return('\n');
    case 0x40 : return(' ');
    case 0x5A : return('!');
    case 0x7f : return('\"');
    case 0x7b : return('#');
    case 0x5b : return('$');
    case 0x6c : return('%');
    case 0x50 : return('&');
    case 0x7d : return('\'');
    case 0x4d : return('(');
    case 0x5d : return(')');
    case 0x5c : return('*');
    case 0x4e : return('+');
    case 0x6b : return(',');
    case 0x60 : return('-');
    case 0x4b : return('.');
    case 0x61 : return('/');
    case 0xf0 : return('0');
    case 0xf1 : return('1');
    case 0xf2 : return('2');
    case 0xf3 : return('3');
    case 0xf4 : return('4');
    case 0xf5 : return('5');
    case 0xf6 : return('6');
    case 0xf7 : return('7');
    case 0xf8 : return('8');
    case 0xf9 : return('9');
    case 0x7a : return(':');
    case 0x5e : return(';');
    case 0x4c : return('<');
    case 0x7e : return('=');
    case 0x6e : return('>');
    case 0x6f : return('?');
    case 0x7c : return('@');
    case 0xc1 : return('A');
    case 0xc2 : return('B');
    case 0xc3 : return('C');
    case 0xc4 : return('D');
    case 0xc5 : return('E');
    case 0xc6 : return('F');
    case 0xc7 : return('G');
    case 0xc8 : return('H');
    case 0xc9 : return('I');
    case 0xd1 : return('J');
    case 0xd2 : return('K');
    case 0xd3 : return('L');
    case 0xd4 : return('M');
    case 0xd5 : return('N');
    case 0xd6 : return('O');
    case 0xd7 : return('P');
    case 0xd8 : return('Q');
    case 0xd9 : return('R');
    case 0xe2 : return('S');
    case 0xe3 : return('T');
    case 0xe4 : return('U');
    case 0xe5 : return('V');
    case 0xe6 : return('W');
    case 0xe7 : return('X');
    case 0xe8 : return('Y');
    case 0xe9 : return('Z');
    case 0xad : return('[');
    case 0xe0 : return('\\');
    case 0xbd : return(']');
    case 0x5f : return('^');
    case 0x6d : return('_');
    case 0x79 : return('`');
    case 0x81 : return('a');
    case 0x82 : return('b');
    case 0x83 : return('c');
    case 0x84 : return('d');
    case 0x85 : return('e');
    case 0x86 : return('f');
    case 0x87 : return('g');
    case 0x88 : return('h');
    case 0x89 : return('i');
    case 0x91 : return('j');
    case 0x92 : return('k');
    case 0x93 : return('l');
    case 0x94 : return('m');
    case 0x95 : return('n');
    case 0x96 : return('o');
    case 0x97 : return('p');
    case 0x98 : return('q');
    case 0x99 : return('r');
    case 0xa2 : return('s');
    case 0xa3 : return('t');
    case 0xa4 : return('u');
    case 0xa5 : return('v');
    case 0xa6 : return('w');
    case 0xa7 : return('x');
    case 0xa8 : return('y');
    case 0xa9 : return('z');
    case 0xc0 : return('{');
    case 0x4f : return('|');
    case 0xd0 : return('}');
    case 0xa1 : return('~');
    case 0x27 : return(0x1b); /* ESC character */
    default   : return(0);
  }
}
