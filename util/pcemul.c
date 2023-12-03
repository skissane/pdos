/*********************************************************************/
/*                                                                   */
/*  This Program Written By Paul Edwards.                            */
/*  Released to the public domain.                                   */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  pcemul - emulate a PC                                            */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <support.h>

static size_t memsize = 640*1024L;

static unsigned char *base;
static unsigned char *p;
static int instr;

static int r1;
static int r2;

#define REG_AL 0
#define REG_AH 1
#define REG_BL 2
#define REG_BH 3
#define REG_CL 4
#define REG_CH 5
#define REG_DL 6
#define REG_DH 7

#define REG_AX 0
#define REG_SS 8
#define REG_DS 9
#define REG_ES 10

typedef unsigned char BYTE;
typedef unsigned short WORD;

static char *namesb[] = {
    "al",
    "ah",
    "bl",
    "bh",
    "cl",
    "ch",
    "dl",
    "dh",
};

static char *namesw[] = {
    "ax",
    "bx",
    "cx",
    "dx",
    "di",
    "si",
    "sp",
    "ip",
    "ss",
    "ds",
    "es",
    "cs",
};

int regsize = 0;

#define BYTE_SIZE 1
#define WORD_SIZE 2

#if 0
static BYTE *destb;
static BYTE *srcb;

static WORD *destw;
static WORD *srcw;
#endif

static void *dest;
static void *src;

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

static int int_enable = 0;

static FILE *fp;

static FILE *logf;

static union REGS regs;
static struct SREGS sregs;
static unsigned int sp;

static void doemul(void);
static void splitregs(unsigned int raw);

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        printf("usage: pcemul <VHD>\n");
        printf("boots a VHD disk image\n");
        return (EXIT_FAILURE);
    }
    
    /* one day this will be r+b */
    fp = fopen(argv[1], "rb");
    if (fp == NULL)
    {
        printf("cannot open %s for reading\n", argv[1]);
        return (EXIT_FAILURE);
    }
    p = calloc(1, memsize);
    if (p == NULL)
    {
        printf("insufficient memory\n");
        return (EXIT_FAILURE);
    }
    base = p;
    logf = stdout;
    doemul();
    fclose(fp);
    return (EXIT_SUCCESS);
}

static void doemul(void)
{
    unsigned char *watching = base + 0x10086 + 3;
    
    regs.h.dl = 0x80;
    p = base + 0x7c00;
    sregs.cs = 0;
    sregs.ss = 0;
    sp = 0x7c00;
    
    fread(base + 0x7c00, 1, 512, fp);

    while (1)
    {
        instr = *p;
        fprintf(logf, "\n");
        fprintf(logf, "instr is %02X at %08X watching %02X\n",
               instr, p - base, *watching);
        fprintf(logf,
                "ax %04X, bx %04X, cx %04X, dx %04X, di %04X, si %04X\n",
                regs.x.ax, regs.x.bx, regs.x.cx, regs.x.dx,
                regs.x.di, regs.x.si);
        fprintf(logf,
                "ss %04X, sp %04X, cs %04X, ip %04X, ds %04X, es %04X\n",
                sregs.ss, sp, sregs.cs, (unsigned int)(p - base), sregs.ds, sregs.es);

        if (instr == 0xfa)
        {
            fprintf(logf, "cli\n");
            int_enable = 0;
            p++;
        }
        /* 33C0            0001     xor ax,ax */
        else if (instr == 0x33)
        {
            fprintf(logf, "xor ax,ax\n");
            regs.x.ax ^= regs.x.ax;
            p += 2;
        }
        /* 8ED8            0003     mov ds,ax */
        /* 8EC0            0005     mov es,ax */
        else if (instr == 0x8e)
        {

            regsize = WORD_SIZE;
            splitregs(*(p + 1));
            fprintf(logf, "mov %s,%s\n", namesw[r1], namesw[r2]);

            *(WORD *)dest = *(WORD *)src;
            p += 2;
        }
        else
        {
            fprintf(logf, "unknown instruction %02X at %08X\n", p[0],
                   p - base);
            exit(EXIT_FAILURE);
        }
    }
    return;
}

        /* 8ED8            0003     mov ds,ax */
        /* 8EC0            0005     mov es,ax */
static void splitregs(unsigned int raw)
{
    unsigned int first;
    unsigned int second;
    
    first = raw >> 4;
    second = raw & 0x7;
    
    if (first == 0xd)
    {
        r1 = REG_DS;
    }
    else if (first == 0xc)
    {
        r1 = REG_ES;
    }
    else
    {
        fprintf(logf, "unknown first register %x\n", raw);
        exit(EXIT_FAILURE);
    }

    if (second == 0)
    {
        r2 = REG_AX;
    }
    else
    {
        fprintf(logf, "unknown second register %x\n", raw);
        exit(EXIT_FAILURE);
    }

    if (regsize == WORD_SIZE)
    {
        if (r1 == REG_DS)
        {
            dest = &sregs.ds;
        }
        else if (r1 == REG_ES)
        {
            dest = &sregs.es;
        }

        if (r2 == REG_DS)
        {
            src = &sregs.ds;
        }
        else if (r2 == REG_ES)
        {
            src = &sregs.es;
        }
        else if (r2 == REG_AX)
        {
            src = &regs.x.ax;
        }
    }

    return;
}
