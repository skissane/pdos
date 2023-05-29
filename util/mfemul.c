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

static void doemul(void);
static void split4(void);

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
    regs[13] = 0x100; /* save area */
    regs[15] = 0x10000; /* entry point */
    regs[14] = 0; /* branch to zero will terminate */

    while (1)
    {
        instr = *p;
        printf("instr is %02X\n", instr);
        if (instr == 0x07) /* bcr */
        {
            x1 = (p[1] >> 4) & 0x0f;
            x2 = p[1] & 0x0f;
            /* x1 is condition, x2 is register to branch to */
            if ((x1 == 0) || (x2 == 0))
            {
                printf("noop\n");
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
            x1 = (p[1] >> 4) & 0x0f;
            x2 = p[1] & 0x0f;
            if (x1 != 0)
            {
                regs[x1] = (p - base) + 2;
            }
            if (x2 != 0)
            {
                p = base + regs[x2];
                printf("new address is %08X\n", regs[x2]);
            }
            p += 2;
        }
        /* this is used for looping. subtract 1 from first register.
           if 0, fall through. otherwise branch to second register */
        else if (instr == 0x06) /* bctr */
        {
            x1 = (p[1] >> 4) & 0x0f;
            x2 = p[1] & 0x0f;
            regs[x1]--;
            printf("new value of %x is %08X\n", x1, regs[x1]);
            if (regs[x1] != 0)
            {
                if (x2 != 0)
                {
                    p = base + regs[x2];
                    continue;
                }
            }
            p += 2;
        }
        else if (instr == 0x41) /* la */
        {
            t = (p[1] >> 4) & 0x0f;
            i = p[1] & 0x0f;
            b = (p[2] >> 4) & 0x0f;
            d = (p[3] & 0xf) << 8 | p[4];
            regs[t] = regs[b] + regs[i] + d;
            printf("new value of %x is %08X\n", t, regs[t]);
            p += 4;
        }
#if 0
        else if (instr == 0x90) /* stm */
        {
            int start;
            int end = 16;
            
            split4();
            if (i < b)
            {
        }
#endif            
        else
        {
            printf("unknown instruction %02X\n", p[0]);
            exit(EXIT_FAILURE);
        }
    }
    return;
}

static void split4(void)
{
    t = (p[1] >> 4) & 0x0f;
    i = p[1] & 0x0f;
    b = (p[2] >> 4) & 0x0f;
    d = (p[3] & 0xf) << 8 | p[4];
    return;
}
