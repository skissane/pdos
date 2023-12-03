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
#define REG_BX 1
#define REG_CX 2
#define REG_DX 3
#define REG_DI 4
#define REG_SI 5
#define REG_SP 6
#define REG_IP 7
#define REG_SS 8
#define REG_DS 9
#define REG_ES 10
#define REG_CS 11

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

#define TO_WORD(d, s) (*(WORD *)d = (s))
#define FROM_WORD(s) (*(WORD *)s)

#define GET_DEST_REG(y) \
   { \
   if (y == REG_SI) dest = &regs.x.si; \
   else if (y == REG_DI) dest = &regs.x.di; \
   else if (y == REG_AX) dest = &regs.x.ax; \
   else if (y == REG_BX) dest = &regs.x.bx; \
   else if (y == REG_CX) dest = &regs.x.cx; \
   else if (y == REG_DX) dest = &regs.x.dx; \
   else if (y == REG_DS) dest = &sregs.ds; \
   else if (y == REG_ES) dest = &sregs.es; \
   else if (y == REG_SS) dest = &sregs.ss; \
   else if (y == REG_SP) dest = &sp; \
   else { fprintf(logf, "unknown\n"); exit(0); } \
   }

#define GET_SRC_REG(y) \
   { \
   if (y == REG_SI) src = &regs.x.si; \
   else if (y == REG_DI) src = &regs.x.di; \
   else if (y == REG_AX) src = &regs.x.ax; \
   else if (y == REG_BX) src = &regs.x.bx; \
   else if (y == REG_CX) src = &regs.x.cx; \
   else if (y == REG_DX) src = &regs.x.dx; \
   else if (y == REG_DS) src = &sregs.ds; \
   else if (y == REG_ES) src = &sregs.es; \
   else if (y == REG_SS) src = &sregs.ss; \
   else if (y == REG_SP) src = &sp; \
   else { fprintf(logf, "unknown\n"); exit(0); } \
   }

typedef size_t FLATAMT;

#define GET_FLATAMT(seg, offs) \
    (((FLATAMT)seg << 4) + offs)

#define WORD_TO_MEM(toamt, val) \
    { \
        base[toamt] = (BYTE)(val & 0xff); \
        base[toamt + 1] = (BYTE)((val >> 8) & 0xff); \
    }

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
static WORD sp;

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

            TO_WORD(dest, FROM_WORD(src));
            p += 2;
        }
        /* B85000          0007     mov ax, 050h */
        else if (instr == 0xb8)
        {
            WORD val;

            regsize = WORD_SIZE;
            r1 = REG_AX;
            dest = &regs.x.ax;
            val = p[2] << 8 | p[1];
            fprintf(logf, "mov %s,0%04Xh\n", namesw[r1], (int)val);
            TO_WORD(dest, val);
            p += 3;
        }
        /* BC0077          000C     mov sp,07700h */
        else if (instr == 0xbc)
        {
            WORD val;

            regsize = WORD_SIZE;
            r1 = REG_SP;
            dest = &sp;
            val = p[2] << 8 | p[1];
            fprintf(logf, "mov %s,0%04Xh\n", namesw[r1], (int)val);
            TO_WORD(dest, val);
            p += 3;
        }
        /* FB              000F     sti */
        else if (instr == 0xfb)
        {
            fprintf(logf, "sti\n");
            int_enable = 1;
            p++;
        }
        /* BE007C          0010     mov si,07c00h */
        else if (instr == 0xbe)
        {
            WORD val;

            regsize = WORD_SIZE;
            GET_DEST_REG(REG_SI);
            val = p[2] << 8 | p[1];
            fprintf(logf, "mov %s,0%04Xh\n", namesw[REG_SI], (int)val);
            TO_WORD(dest, val);
            p += 3;
        }
        /* BF0006          0013     mov di,0600h */
        else if (instr == 0xbf)
        {
            WORD val;

            regsize = WORD_SIZE;
            GET_DEST_REG(REG_DI);
            val = p[2] << 8 | p[1];
            fprintf(logf, "mov %s,0%04Xh\n", namesw[REG_DI], (int)val);
            TO_WORD(dest, val);
            p += 3;
        }
        /* B90001          0016     mov cx,0100h */
        else if (instr == 0xb9)
        {
            WORD val;

            regsize = WORD_SIZE;
            GET_DEST_REG(REG_CX);
            val = p[2] << 8 | p[1];
            fprintf(logf, "mov %s,0%04Xh\n", namesw[REG_CX], (int)val);
            TO_WORD(dest, val);
            p += 3;
        }
        /* F3A5            0019     rep movsw */
        /* di is target. guessing target segment register is es.
        /* si is source. guessing source segment register is ds */
        /* length is cx */
        else if (instr == 0xf3)
        {
            if (p[1] == 0xa5)
            {
                WORD len;
                WORD seg;
                WORD offs;
                FLATAMT fromamt;
                FLATAMT toamt;
                unsigned char *from;
                unsigned char *to;
                
                GET_SRC_REG(REG_CX);
                len = FROM_WORD(src);
                
                GET_SRC_REG(REG_ES);
                seg = FROM_WORD(src);
                
                GET_SRC_REG(REG_DI);
                offs = FROM_WORD(src);

                toamt = GET_FLATAMT(seg, offs);
                to = base + toamt;
                
                GET_SRC_REG(REG_DS);
                seg = FROM_WORD(src);
                
                GET_SRC_REG(REG_SI);
                offs = FROM_WORD(src);
                
                fromamt = GET_FLATAMT(seg, offs);
                from = base + fromamt;

#if 0
                printf("base %p, from %p, to %p, len %d\n", base, from, to, len * 2);
                exit(0);
#endif
                fprintf(logf, "rep movsw\n");
                memcpy(to, from, len * 2);
                
                p += 2;
            }
            else
            {
                fprintf(logf, "unknown f3\n");
                exit(EXIT_FAILURE);
            }
        }
        /* 50              001E     push ax */
        else if (instr == 0x50)
        {
            WORD val;
            WORD seg;
            WORD offs;
            FLATAMT toamt;
                
            regsize = WORD_SIZE;
            GET_SRC_REG(REG_AX);
            val = FROM_WORD(src);

            GET_SRC_REG(REG_SS);
            seg = FROM_WORD(src);
                
            GET_SRC_REG(REG_SP);
            offs = FROM_WORD(src);

            toamt = GET_FLATAMT(seg, offs);

            fprintf(logf, "push %s\n", namesw[REG_AX]);
            WORD_TO_MEM(toamt, val);
            p++;
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
        /* 8ED0            000A     mov ss,ax */
static void splitregs(unsigned int raw)
{
    unsigned int first;
    unsigned int second;
    
    first = raw >> 3;
    second = raw & 0x7;
    
    if (first == (0xd8 >> 3))
    {
        r1 = REG_DS;
    }
    else if (first == (0xc0 >> 3))
    {
        r1 = REG_ES;
    }
    else if (first == (0xd0 >> 3))
    {
        r1 = REG_SS;
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
