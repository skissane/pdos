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

#define MAXPRGSZ (10 * 0x10000)

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

static int lt;
static int gt;
static int eq;
static int zero;

static void doemul(void);
static void splitrx(void);
static void splitrs(void);
static void splitrr(void);
static void writereg(unsigned char *z, int x);
static void updatereg(int *x, unsigned char *z);

int main(int argc, char **argv)
{
    FILE *fp;

    if (argc <= 1)
    {
        printf("usage: mfemul <COM>\n");
        printf("executes a mainframe .com file\n");
        return (EXIT_FAILURE);
    }
    
    fp = fopen(argv[1], "rb");
    if (fp == NULL)
    {
        printf("cannot open %s for reading\n", argv[1]);
        return (EXIT_FAILURE);
    }
    p = calloc(1, MAXPRGSZ + 0x10000);
    if (p == NULL)
    {
        printf("insufficient memory\n");
        return (EXIT_FAILURE);
    }
    fread(p + 0x10000, 1, MAXPRGSZ, fp);
    fclose(fp);
    base = p;
    p += 0x10000;
    doemul();
    return (EXIT_SUCCESS);
}

static void doemul(void)
{
    unsigned char *watching = base + 0x1007a;
    
    regs[13] = 0x100; /* save area */
    regs[15] = 0x10000; /* entry point */
    regs[14] = 0; /* branch to zero will terminate */

    while (1)
    {
        instr = *p;
        printf("instr is %02X at %08X watching %02X, r14 %08X\n", instr, p - base - 0x10000, *watching, regs[14]);
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
                p = base + regs[x2];
                printf("new address is %08X\n", regs[x2] - 0x10000);
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
        else if (instr == 0x1a) /* ar */
        {
            splitrr();
            regs[x1] += regs[x2];
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

            splitrx();
            /* not sure if this is subject to r0 check */
            if (b != 0)
            {
                one = regs[b];
            }
            /* bl */
            if (i == 0x40)
            {
                if (lt)
                {
                    p = base + one + d;
                    continue;
                }
            }
            /* bnz */
            else if (i == 0x70)
            {
                if (!zero)
                {
                    p = base + one + d;
                    continue;
                }
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
                   updatereg(&regs[x], target);
                   target += 4;
                }
                start = 0;
            }
            for (x = start; x <= end; x++)
            {
                updatereg(&regs[x], target);
                target += 4;
            }
            p += 4;
        }
        else
        {
            printf("unknown instruction %02X at %08X\n", p[0],
                   p - base - 0x10000);
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
