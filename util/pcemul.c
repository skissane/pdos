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

#define REG_AX 0
#define REG_SS 8
#define REG_DS 9
#define REG_ES 10

static char *names[] = {
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
        fprintf(logf, "instr is %02X at %08X watching %02X\n",
               instr, p - base, *watching);
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
            unsigned short *dest;
            unsigned short *src;
            
            splitregs(*(p + 1));
            fprintf(logf, "mov %s,%s\n", names[r1], names[r2]);

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
            *dest = *src;
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
}
