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
#define REG_BP 12
#define REG_AL 13
#define REG_AH 14
#define REG_BL 15
#define REG_BH 16
#define REG_CL 17
#define REG_CH 18
#define REG_DL 19
#define REG_DH 20


typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

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
    "bp",
    "al",
    "ah",
    "bl",
    "bh",
    "cl",
    "ch",
    "dl",
    "dh",
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

#define TO_BYTE(d, s) (*(BYTE *)d = (s))
#define FROM_BYTE(s) (*(BYTE *)s)

#define GET_DEST_REG(y) \
   { \
   if (y == REG_SI) dest = &regs.x.si; \
   else if (y == REG_DI) dest = &regs.x.di; \
   else if (y == REG_AX) dest = &regs.x.ax; \
   else if (y == REG_BX) dest = &regs.x.bx; \
   else if (y == REG_CX) dest = &regs.x.cx; \
   else if (y == REG_DX) dest = &regs.x.dx; \
   else if (y == REG_BP) dest = &regs.x.bp; \
   else if (y == REG_AL) dest = &regs.h.al; \
   else if (y == REG_BL) dest = &regs.h.bl; \
   else if (y == REG_CL) dest = &regs.h.cl; \
   else if (y == REG_DL) dest = &regs.h.dl; \
   else if (y == REG_AH) dest = &regs.h.ah; \
   else if (y == REG_BH) dest = &regs.h.bh; \
   else if (y == REG_CH) dest = &regs.h.ch; \
   else if (y == REG_DH) dest = &regs.h.dh; \
   else if (y == REG_DS) dest = &sregs.ds; \
   else if (y == REG_ES) dest = &sregs.es; \
   else if (y == REG_SS) dest = &sregs.ss; \
   else if (y == REG_CS) dest = &sregs.cs; \
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
   else if (y == REG_BP) src = &regs.x.bp; \
   else if (y == REG_AL) src = &regs.h.al; \
   else if (y == REG_BL) src = &regs.h.bl; \
   else if (y == REG_CL) src = &regs.h.cl; \
   else if (y == REG_DL) src = &regs.h.dl; \
   else if (y == REG_AH) src = &regs.h.ah; \
   else if (y == REG_BH) src = &regs.h.bh; \
   else if (y == REG_CH) src = &regs.h.ch; \
   else if (y == REG_DH) src = &regs.h.dh; \
   else if (y == REG_DS) src = &sregs.ds; \
   else if (y == REG_ES) src = &sregs.es; \
   else if (y == REG_SS) src = &sregs.ss; \
   else if (y == REG_CS) src = &sregs.cs; \
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

#define BYTE_TO_MEM(toamt, val) \
    (base[toamt] = (BYTE)(val))

#define WORD_AT_MEM(fromamt) \
    ((WORD)((base[fromamt + 1] << 8) | base[fromamt]))

#define BYTE_AT_MEM(fromamt) \
    ((BYTE)base[fromamt])

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
static int cflag = 0;
static int dflag = 0;

static int int_enable = 0;

static FILE *fp;

static FILE *logf;

static union REGS regs;
static struct SREGS sregs;
static WORD sp;

static void doemul(void);
static void splitregs(unsigned int raw);
static void dobios(int intnum);

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
    unsigned char *watching = base + 0xFDC6;
    
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
        fprintf(logf, "instr is %02X at %08lX watching %05lX %02X %02X %02X %02X\n",
               instr, (long)(p - base),
               (long)(watching - base),
               watching[0], watching[1], watching[2], watching[3]);
        fprintf(logf,
                "ax %04X, bx %04X, cx %04X, dx %04X, di %04X, si %04X, bp %04X\n",
                regs.x.ax, regs.x.bx, regs.x.cx, regs.x.dx,
                regs.x.di, regs.x.si, regs.x.bp);
        fprintf(logf,
                "ss %04X, sp %04X, cs %04X, ip %04X, ds %04X, es %04X\n",
                sregs.ss, sp, sregs.cs, (unsigned int)(p - base), sregs.ds, sregs.es);

        if (instr == 0xfa)
        {
            fprintf(logf, "cli\n");
            int_enable = 0;
            p++;
        }
        /* FC              0041      cld */
        else if (instr == 0xfc)
        {
            fprintf(logf, "cld\n");
            dflag = 0;
            p++;
        }
        /* 33C0            0001     xor ax,ax */
        else if ((instr == 0x33) && (p[1] == 0xc0))
        {
            fprintf(logf, "xor ax,ax\n");
            regs.x.ax ^= regs.x.ax;
            p += 2;
        }
        /* 33D2            013A      xor  dx, dx */
        /* 33C9            00C0      xor cx, cx */
        else if (instr == 0x33)
        {
            int r;
            
            if (p[1] == 0xd2)
            {
                r = REG_DX;
            }
            else if (p[1] == 0xc9)
            {
                r = REG_CX;
            }
            else
            {
                printf("unknown register %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            fprintf(logf, "xor %s,%s\n", names[r], names[r]);
            
            GET_SRC_REG(r);
            dest = src;
            TO_WORD(dest, FROM_WORD(src) ^ FROM_WORD(src));
            p += 2;
        }
        
        /* 8ED8            0003     mov ds,ax */
        /* 8EC0            0005     mov es,ax */
        /* 8EDA            0023     mov ds, dx */
        else if (instr == 0x8e)
        {
            regsize = WORD_SIZE;
            splitregs(*(p + 1));
            fprintf(logf, "mov %s,%s\n", names[r1], names[r2]);

            TO_WORD(dest, FROM_WORD(src));
            p += 2;
        }
        /* 8AD6            0066      mov dl,dh */
        else if ((instr == 0x8a) && (p[1] == 0xd6))
        {
            regsize = BYTE_SIZE;
            /* splitregs(*(p + 1)); */
            fprintf(logf, "mov %s,%s\n", names[REG_DL], names[REG_DH]);

            GET_DEST_REG(REG_DL);
            GET_SRC_REG(REG_DH);
            TO_BYTE(dest, FROM_BYTE(src));
            p += 2;
        }
        /* 8ACA            0136      mov  cl,  dl */
        else if ((instr == 0x8a) && (p[1] == 0xca))
        {
            regsize = BYTE_SIZE;
            fprintf(logf, "mov %s,%s\n", names[REG_CL], names[REG_DL]);

            GET_DEST_REG(REG_CL);
            GET_SRC_REG(REG_DL);
            TO_BYTE(dest, FROM_BYTE(src));
            p += 2;
        }
        /* 8AF2            0140      mov  dh,  dl */
        else if ((instr == 0x8a) && (p[1] == 0xf2))
        {
            regsize = BYTE_SIZE;
            fprintf(logf, "mov %s,%s\n", names[REG_DH], names[REG_DL]);

            GET_DEST_REG(REG_DH);
            GET_SRC_REG(REG_DL);
            TO_BYTE(dest, FROM_BYTE(src));
            p += 2;
        }
        /* 8AF0            0143      mov  dh, al */
        else if ((instr == 0x8a) && (p[1] == 0xf0))
        {
            regsize = BYTE_SIZE;
            fprintf(logf, "mov %s,%s\n", names[REG_DH], names[REG_AL]);

            GET_DEST_REG(REG_DH);
            GET_SRC_REG(REG_AL);
            TO_BYTE(dest, FROM_BYTE(src));
            p += 2;
        }
        /* 8AE6            014D      mov  ah, dh */
        else if ((instr == 0x8a) && (p[1] == 0xe6))
        {
            regsize = BYTE_SIZE;
            /* splitregs(*(p + 1)); */
            fprintf(logf, "mov %s,%s\n", names[REG_AH], names[REG_DH]);

            GET_DEST_REG(REG_AH);
            GET_SRC_REG(REG_DH);
            TO_BYTE(dest, FROM_BYTE(src));
            p += 2;
        }
        /* 8A07            00AD           mov     al,[bx] */
        else if ((instr == 0x8a) && (p[1] == 0x07))
        {
            FLATAMT fromamt;
            WORD seg;
            WORD offs;
            
            regsize = BYTE_SIZE;
            fprintf(logf, "mov %s,[%s]\n", names[REG_AL], names[REG_BX]);

            GET_SRC_REG(REG_BX);
            offs = FROM_WORD(src);

            GET_SRC_REG(REG_DS);
            seg = FROM_WORD(src);

            fromamt = GET_FLATAMT(seg, offs);

            GET_DEST_REG(REG_AL);
            TO_BYTE(dest, BYTE_AT_MEM(fromamt));
            p += 2;
        }
        /* 8A4600          00B0     mov al,[bp] */
        else if ((instr == 0x8a) && (p[1] == 0x46) && (p[2] == 0))
        {
            FLATAMT fromamt;
            WORD seg;
            WORD offs;
            
            regsize = BYTE_SIZE;
            fprintf(logf, "mov %s,[%s]\n", names[REG_AL], names[REG_BP]);

            GET_SRC_REG(REG_BP);
            offs = FROM_WORD(src);

            GET_SRC_REG(REG_DS); /* I think this should be SS and thus mbr.asm has a bug */
            seg = FROM_WORD(src);

            fromamt = GET_FLATAMT(seg, offs);

            GET_DEST_REG(REG_AL);
            TO_BYTE(dest, BYTE_AT_MEM(fromamt));
            p += 3;
        }
        /* 8A16B501 mov  dl, [BootDisk] */
        else if ((instr == 0x8a) && (p[1] == 0x16))
        {
            FLATAMT fromamt;
            WORD seg;
            WORD offs;
            
            regsize = BYTE_SIZE;

            offs = (p[3] << 8) | p[2];

            GET_SRC_REG(REG_DS);
            seg = FROM_WORD(src);

            fromamt = GET_FLATAMT(seg, offs);

            fprintf(logf, "mov %s,[0%05X]\n", names[REG_DL], fromamt);
            GET_DEST_REG(REG_DL);
            TO_BYTE(dest, BYTE_AT_MEM(fromamt));
            p += 4;
        }
        /* A11600          00BB      mov ax, [FatSize16] */
        else if (instr == 0xa1)
        {
            FLATAMT fromamt;
            WORD seg;
            WORD offs;
            
            regsize = WORD_SIZE;

            offs = (p[2] << 8) | p[1];

            GET_SRC_REG(REG_DS);
            seg = FROM_WORD(src);

            fromamt = GET_FLATAMT(seg, offs);

            fprintf(logf, "mov %s,[0%05Xh]\n", names[REG_AX], fromamt);
            GET_DEST_REG(REG_AX);
            TO_WORD(dest, WORD_AT_MEM(fromamt));
            p += 3;
        }
        /* A3130C          0016           mov     _dseg,ax */
        else if (instr == 0xa3)
        {
            FLATAMT toamt;
            WORD seg;
            WORD offs;
            
            regsize = WORD_SIZE;

            offs = (p[2] << 8) | p[1];

            GET_SRC_REG(REG_DS);
            seg = FROM_WORD(src);

            toamt = GET_FLATAMT(seg, offs);

            fprintf(logf, "mov 0%05Xh, %s\n", toamt, names[REG_AX]);
            GET_SRC_REG(REG_AX);
            WORD_TO_MEM(toamt, FROM_WORD(src));
            p += 3;
        }
        /* D3E8            014A      shr  ax, cl */
        /* D3E0            0412           shl     ax,cl */
        /* D3F8            0633           sar     ax,cl */
        else if (instr == 0xd3)
        {
            BYTE val;
            WORD val2;
            int left;
            int arith = 0;
            
            regsize = WORD_SIZE;
            if (p[1] == 0xe8)
            {
                left = 0;
            }
            else if (p[1] ==0xf8)
            {
                left = 0;
                arith = 1;
            }
            else if (p[1] == 0xe0)
            {
                left = 1;
            }
            else
            {
                printf("unknown register %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            if (left)
            {
                fprintf(logf, "shl %s,%s\n", names[REG_AX], names[REG_CL]);
            }
            else if (arith)
            {
                fprintf(logf, "sar %s,%s - converted to shr for now\n",
                        names[REG_AX], names[REG_CL]);
            }
            else
            {
                fprintf(logf, "shr %s,%s\n", names[REG_AX], names[REG_CL]);
            }

            GET_SRC_REG(REG_CL);
            val = FROM_BYTE(src);
            
            GET_SRC_REG(REG_AX);
            val2 = FROM_WORD(src);
            
            if (left)
            {
                val2 <<= val;
            }
            else
            {
                val2 >>= val;
            }
            
            dest = src;
            TO_WORD(dest, val2);
            p += 2;
        }
        /* 0BC8            0150      or   cx, ax */
        /* 0BC1            0415           or      ax,cx */
        else if (instr == 0x0b)
        {
            WORD val;
            WORD val2;
            int r1;
            int r2;
            
            regsize = WORD_SIZE;
            if (p[1] == 0xc8)
            {
                r1 = REG_AX;
                r2 = REG_CX;
            }
            else if (p[1] == 0xc1)
            {
                r1 = REG_CX;
                r2 = REG_AX;
            }
            else
            {
                printf("unknown register %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            fprintf(logf, "or %s,%s\n", names[r2], names[r1]);

            GET_SRC_REG(r1);
            val = FROM_WORD(src);
            
            GET_SRC_REG(r2);
            val2 = FROM_WORD(src);
            
            val2 |= val;
            
            dest = src;
            TO_WORD(dest, val2);
            p += 2;
        }
        /* 39C1            006E           cmp     cx,ax */
        else if (instr == 0x39)
        {
            WORD val;
            WORD val2;
            
            regsize = WORD_SIZE;
            if (p[1] != 0xc1)
            {
                printf("unknown register %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            fprintf(logf, "cmp %s,%s\n", names[REG_CX], names[REG_AX]);

            GET_SRC_REG(REG_AX);
            val = FROM_WORD(src);
            
            GET_SRC_REG(REG_CX);
            val2 = FROM_WORD(src);

            /* I think these are the right way around */
            zero = 0;
            if (val2 == val)
            {
                lt = 0;
                gt = 0;
                eq = 1;
                zero = 1; /* just a guess */
            }
            else if (val2 < val)
            {
                lt = 1;
                gt = 0;
                eq = 0;
            }
            else
            {
                lt = 0;
                gt = 1;
                eq = 0;
            }
            p += 2;
        }
        /* 014EFC   add     [bp-4],cx */
        else if ((instr == 0x01) && (p[1] == 0x4e))
        {
            WORD val;
            WORD val2;
            WORD oldval2;
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
            int r = REG_CX;
            int r2 = REG_BP;
                
            regsize = WORD_SIZE;

            GET_SRC_REG(REG_CX);
            val = FROM_WORD(src);
                        
            GET_SRC_REG(REG_DS);
            seg = FROM_WORD(src);

            GET_SRC_REG(REG_BP);
            offs = FROM_WORD(src);
            
            fromamt = GET_FLATAMT(seg, offs);
            
            if (p[2] >= 0x80)
            {
                fromamt -= (0x100U - p[2]);
                fprintf(logf, "add [%s-0%02Xh],%s\n",
                        names[r2], (0x100U - p[2]), names[r]);
            }
            else
            {
                fromamt += p[2];
                fprintf(logf, "add [%s+0%02Xh],%s\n",
                        names[r2], p[2], names[r]);
            }

            val2 = WORD_AT_MEM(fromamt);

            oldval2 = val2;
            val2 += val;
            if (val2 < oldval2)
            {
                cflag = 1;
            }
            else
            {
                cflag = 0;
            }
            dest = src;
            TO_WORD(dest, val2);
            p += 3;
        }
        /* 03060E00 add ax, [ReservedSectors] */
        /* 031E0B00 add bx,  [BytesPerSector] */
        else if ((instr == 0x03) && ((p[1] == 0x06) || (p[1] == 0x1e)))
        {
            WORD val;
            WORD val2;
            WORD oldval2;
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
            int r;
                
            regsize = WORD_SIZE;
            
            if (p[1] == 0x06)
            {
                r = REG_AX;
            }
            else if (p[1] == 0x1e)
            {
                r = REG_BX;
            }
            
            GET_SRC_REG(REG_DS);
            seg = FROM_WORD(src);

            offs = (p[3] << 8) | p[2];

            fromamt = GET_FLATAMT(seg, offs);
            val = WORD_AT_MEM(fromamt);

            fprintf(logf, "add %s, [%05Xh]\n", names[r], fromamt);
            
            GET_SRC_REG(r);

            val2 = FROM_WORD(src);
            oldval2 = val2;
            val2 += val;
            if (val2 < oldval2)
            {
                cflag = 1;
            }
            else
            {
                cflag = 0;
            }
            dest = src;
            TO_WORD(dest, val2);
            p += 4;
        }
        /* 03C3            00DC      add ax, bx */
        /* 03C1            00A7           add     ax,cx */
        else if (instr == 0x03)
        {
            WORD val;
            WORD val2;
            WORD oldval2;
            int r;
            
            regsize = WORD_SIZE;
            if (p[1] == 0xc3)
            {
                r = REG_BX;
            }
            else if (p[1] == 0xc1)
            {
                r = REG_CX;
            }
            else
            {
                printf("unknown register %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            fprintf(logf, "add %s,%s\n", names[REG_AX], names[r]);

            GET_SRC_REG(r);
            val = FROM_WORD(src);
            
            GET_SRC_REG(REG_AX);
            val2 = FROM_WORD(src);
            
            oldval2 = val2;
            val2 += val;
            if (val2 < oldval2)
            {
                cflag = 1;
            }
            else
            {
                cflag = 0;
            }
            
            dest = src;
            TO_WORD(dest, val2);
            p += 2;
        }
        /* 13161E00   adc dx, [HiddenSectors_High] */
        else if ((instr == 0x13) && (p[1] == 0x16))
        {
            WORD val;
            WORD val2;
            WORD oldval2;
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
                
            regsize = WORD_SIZE;
            
            GET_SRC_REG(REG_DS);
            seg = FROM_WORD(src);

            offs = (p[3] << 8) | p[2];

            fromamt = GET_FLATAMT(seg, offs);
            val = WORD_AT_MEM(fromamt);

            fprintf(logf, "adc %s, [%05Xh]\n", names[REG_DX], fromamt);
            
            GET_SRC_REG(REG_DX);

            val2 = FROM_WORD(src);
            oldval2 = val2;
            val2 += (val + cflag);
            if (val2 < oldval2)
            {
                cflag = 1;
            }
            else
            {
                cflag = 0;
            }
            dest = src;
            TO_WORD(dest, val2);
            p += 4;
        }
        /* 13D1            00DE      adc dx, cx */
        else if (instr == 0x13)
        {
            WORD val;
            WORD val2;
            WORD oldval2;
            
            regsize = WORD_SIZE;
            if (p[1] != 0xd1)
            {
                printf("unknown register %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            fprintf(logf, "adc %s,%s\n", names[REG_DX], names[REG_CX]);

            GET_SRC_REG(REG_CX);
            val = FROM_WORD(src);
            
            GET_SRC_REG(REG_DX);
            val2 = FROM_WORD(src);
            
            oldval2 = val2;
            val2 += (val + cflag);
            if (val2 < oldval2)
            {
                cflag = 1;
            }
            else
            {
                cflag = 0;
            }
            
            dest = src;
            TO_WORD(dest, val2);
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
            fprintf(logf, "mov %s,0%04Xh\n", names[r1], (unsigned int)val);
            TO_WORD(dest, val);
            p += 3;
        }
        /* BB007C          008A     mov bx, 07c00h */
        else if (instr == 0xbb)
        {
            WORD val;

            regsize = WORD_SIZE;
            GET_DEST_REG(REG_BX);
            val = p[2] << 8 | p[1];
            fprintf(logf, "mov %s,0%04Xh\n", names[REG_BX], (unsigned int)val);
            TO_WORD(dest, val);
            p += 3;
        }
        /* BA0000          00A9      mov  dx, 0 */
        else if (instr == 0xba)
        {
            WORD val;

            regsize = WORD_SIZE;
            GET_DEST_REG(REG_DX);
            val = p[2] << 8 | p[1];
            fprintf(logf, "mov %s,0%04Xh\n", names[REG_DX], (unsigned int)val);
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
            fprintf(logf, "mov %s,0%04Xh\n", names[r1], (int)val);
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
            fprintf(logf, "mov %s,0%04Xh\n", names[REG_SI], (int)val);
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
            fprintf(logf, "mov %s,0%04Xh\n", names[REG_DI], (int)val);
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
            fprintf(logf, "mov %s,0%04Xh\n", names[REG_CX], (int)val);
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
            toamt -= 2;

            fprintf(logf, "push %s\n", names[REG_AX]);
            dest = src;
            TO_WORD(dest, FROM_WORD(src) - 2);
            WORD_TO_MEM(toamt, val);
            p++;
        }
        /* 51              0147      push cx */
        else if (instr == 0x51)
        {
            WORD val;
            WORD seg;
            WORD offs;
            FLATAMT toamt;
                
            regsize = WORD_SIZE;
            GET_SRC_REG(REG_CX);
            val = FROM_WORD(src);

            GET_SRC_REG(REG_SS);
            seg = FROM_WORD(src);
                
            GET_SRC_REG(REG_SP);
            offs = FROM_WORD(src);

            toamt = GET_FLATAMT(seg, offs);
            toamt -= 2;

            fprintf(logf, "push %s\n", names[REG_CX]);
            dest = src;
            TO_WORD(dest, FROM_WORD(src) - 2);
            WORD_TO_MEM(toamt, val);
            p++;
        }
        /* 52              0142      push dx */
        else if (instr == 0x52)
        {
            WORD val;
            WORD seg;
            WORD offs;
            FLATAMT toamt;
                
            regsize = WORD_SIZE;
            GET_SRC_REG(REG_DX);
            val = FROM_WORD(src);

            GET_SRC_REG(REG_SS);
            seg = FROM_WORD(src);
                
            GET_SRC_REG(REG_SP);
            offs = FROM_WORD(src);

            toamt = GET_FLATAMT(seg, offs);
            toamt -= 2;

            fprintf(logf, "push %s\n", names[REG_DX]);
            dest = src;
            TO_WORD(dest, FROM_WORD(src) - 2);
            WORD_TO_MEM(toamt, val);
            p++;
        }
        /* 53              0164      push bx */
        else if (instr == 0x53)
        {
            WORD val;
            WORD seg;
            WORD offs;
            FLATAMT toamt;
                
            regsize = WORD_SIZE;
            GET_SRC_REG(REG_BX);
            val = FROM_WORD(src);

            GET_SRC_REG(REG_SS);
            seg = FROM_WORD(src);
                
            GET_SRC_REG(REG_SP);
            offs = FROM_WORD(src);

            toamt = GET_FLATAMT(seg, offs);
            toamt -= 2;

            fprintf(logf, "push %s\n", names[REG_BX]);
            dest = src;
            TO_WORD(dest, FROM_WORD(src) - 2);
            WORD_TO_MEM(toamt, val);
            p++;
        }
        /* 55              0000     _main:        push    bp */
        else if (instr == 0x55)
        {
            WORD val;
            WORD seg;
            WORD offs;
            FLATAMT toamt;
                
            regsize = WORD_SIZE;
            GET_SRC_REG(REG_BP);
            val = FROM_WORD(src);

            GET_SRC_REG(REG_SS);
            seg = FROM_WORD(src);
                
            GET_SRC_REG(REG_SP);
            offs = FROM_WORD(src);

            toamt = GET_FLATAMT(seg, offs);
            toamt -= 2;

            fprintf(logf, "push %s\n", names[REG_BP]);
            dest = src;
            TO_WORD(dest, FROM_WORD(src) - 2);
            WORD_TO_MEM(toamt, val);
            p++;
        }
        /* 57              0042     push di */
        else if (instr == 0x57)
        {
            WORD val;
            WORD seg;
            WORD offs;
            FLATAMT toamt;
                
            regsize = WORD_SIZE;
            GET_SRC_REG(REG_DI);
            val = FROM_WORD(src);

            GET_SRC_REG(REG_SS);
            seg = FROM_WORD(src);
                
            GET_SRC_REG(REG_SP);
            offs = FROM_WORD(src);

            toamt = GET_FLATAMT(seg, offs);
            toamt -= 2;

            fprintf(logf, "push %s\n", names[REG_DI]);
            dest = src;
            TO_WORD(dest, FROM_WORD(src) - 2);
            WORD_TO_MEM(toamt, val);
            p++;
        }
        /* 06              0167      push es */
        else if (instr == 0x06)
        {
            WORD val;
            WORD seg;
            WORD offs;
            FLATAMT toamt;
                
            regsize = WORD_SIZE;
            GET_SRC_REG(REG_ES);
            val = FROM_WORD(src);

            GET_SRC_REG(REG_SS);
            seg = FROM_WORD(src);
                
            GET_SRC_REG(REG_SP);
            offs = FROM_WORD(src);

            toamt = GET_FLATAMT(seg, offs);
            toamt -= 2;

            fprintf(logf, "push %s\n", names[REG_ES]);
            dest = src;
            TO_WORD(dest, FROM_WORD(src) - 2);
            WORD_TO_MEM(toamt, val);
            p++;
        }
        /* 1E              003E     push ds */
        else if (instr == 0x1e)
        {
            WORD val;
            WORD seg;
            WORD offs;
            FLATAMT toamt;
                
            regsize = WORD_SIZE;
            GET_SRC_REG(REG_DS);
            val = FROM_WORD(src);

            GET_SRC_REG(REG_SS);
            seg = FROM_WORD(src);
                
            GET_SRC_REG(REG_SP);
            offs = FROM_WORD(src);

            toamt = GET_FLATAMT(seg, offs);
            toamt -= 2;

            fprintf(logf, "push %s\n", names[REG_DS]);
            dest = src;
            TO_WORD(dest, FROM_WORD(src) - 2);
            WORD_TO_MEM(toamt, val);
            p++;
        }
        /* CB              0023     retf */
        else if (instr == 0xcb)
        {
            WORD seg;
            WORD offs;
            WORD newoffs;
            WORD newseg;
            FLATAMT fromamt;
            FLATAMT newamt;
                
            fprintf(logf, "retf\n");
            
            GET_SRC_REG(REG_SS);
            seg = FROM_WORD(src);
                
            GET_SRC_REG(REG_SP);
            offs = FROM_WORD(src);

            fromamt = GET_FLATAMT(seg, offs);

            newoffs = WORD_AT_MEM(fromamt);
            newseg = WORD_AT_MEM(fromamt + 2);
            
            dest = src;
            TO_WORD(dest, offs + 4);
            
            GET_DEST_REG(REG_CS);
            TO_WORD(dest, newseg);

            newamt = GET_FLATAMT(newseg, newoffs);
            p = base + newamt;
        }
        /* C3              0152      ret */
        else if (instr == 0xc3)
        {
            WORD seg;
            WORD offs;
            WORD newoffs;
            WORD newseg;
            FLATAMT fromamt;
            FLATAMT newamt;
                
            fprintf(logf, "ret\n");
            
            GET_SRC_REG(REG_SS);
            seg = FROM_WORD(src);
                
            GET_SRC_REG(REG_SP);
            offs = FROM_WORD(src);

            fromamt = GET_FLATAMT(seg, offs);

            newoffs = WORD_AT_MEM(fromamt);
            
            dest = src;
            TO_WORD(dest, offs + 2);
            
            GET_SRC_REG(REG_CS);
            newseg = FROM_WORD(src);

            newamt = GET_FLATAMT(newseg, newoffs);
            p = base + newamt;
        }
        /* E8B600          0077     call Lba2Chs */
        /* 0130     Lba2Chs proc */
        /* 77 + 3 + b6 = 130 */
        else if (instr == 0xe8)
        {
            WORD val;
            WORD seg;
            WORD offs;
            WORD newoffs;
            WORD newseg;
            WORD ip;
            FLATAMT toamt;
            FLATAMT newamt;
                
            fprintf(logf, "call 0%02X%02Xh\n", p[2], p[1]);
            
            GET_SRC_REG(REG_SS);
            seg = FROM_WORD(src);
                
            GET_SRC_REG(REG_SP);
            offs = FROM_WORD(src);

            dest = src;
            offs -= 2;
            TO_WORD(dest, offs);
            
            toamt = GET_FLATAMT(seg, offs);

            GET_SRC_REG(REG_CS);
            seg = FROM_WORD(src);
            
            ip = (WORD)(p - base + 3 - ((FLATAMT)seg << 4));
            base[toamt] = ip & 0xff;
            base[toamt + 1] = (ip >> 8) & 0xff;
            
            if (p[2] < 0x80)
            { 
                p += (p[2] << 8) + p[1] + 3;
            }
            else
            {
                p -= 0x10000UL - ((p[2] << 8) + p[1]);
                p += 3;
            }
        }
        /* 0E              0024     push cs */
        else if (instr == 0x0e)
        {
            WORD val;
            WORD seg;
            WORD offs;
            FLATAMT toamt;
                
            regsize = WORD_SIZE;
            GET_SRC_REG(REG_CS);
            val = FROM_WORD(src);

            GET_SRC_REG(REG_SS);
            seg = FROM_WORD(src);
                
            GET_SRC_REG(REG_SP);
            offs = FROM_WORD(src);

            toamt = GET_FLATAMT(seg, offs);
            toamt -= 2;

            fprintf(logf, "push %s\n", names[REG_CS]);
            dest = src;
            TO_WORD(dest, FROM_WORD(src) - 2);
            WORD_TO_MEM(toamt, val);
            p++;
        }
        /* 1F              0025     pop ds */
        else if (instr == 0x1f)
        {
            WORD val;
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
                
            fprintf(logf, "pop %s\n", names[REG_DS]);
            
            regsize = WORD_SIZE;
            GET_SRC_REG(REG_SS);
            seg = FROM_WORD(src);
                
            GET_SRC_REG(REG_SP);
            offs = FROM_WORD(src);

            fromamt = GET_FLATAMT(seg, offs);

            val = WORD_AT_MEM(fromamt);
            
            dest = src;
            TO_WORD(dest, FROM_WORD(src) + 2);

            GET_DEST_REG(REG_DS);
            TO_WORD(dest, val);

            p++;
        }
        /* 07              0178      pop es */
        else if (instr == 0x07)
        {
            WORD val;
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
                
            fprintf(logf, "pop %s\n", names[REG_ES]);
            
            regsize = WORD_SIZE;
            GET_SRC_REG(REG_SS);
            seg = FROM_WORD(src);
                
            GET_SRC_REG(REG_SP);
            offs = FROM_WORD(src);

            fromamt = GET_FLATAMT(seg, offs);

            val = WORD_AT_MEM(fromamt);
            
            dest = src;
            TO_WORD(dest, FROM_WORD(src) + 2);

            GET_DEST_REG(REG_ES);
            TO_WORD(dest, val);

            p++;
        }
        /* 5D pop bp */
        else if (instr == 0x5d)
        {
            WORD val;
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
                
            fprintf(logf, "pop %s\n", names[REG_BP]);
            
            regsize = WORD_SIZE;
            GET_SRC_REG(REG_SS);
            seg = FROM_WORD(src);
                
            GET_SRC_REG(REG_SP);
            offs = FROM_WORD(src);

            fromamt = GET_FLATAMT(seg, offs);

            val = WORD_AT_MEM(fromamt);
            
            dest = src;
            TO_WORD(dest, FROM_WORD(src) + 2);

            GET_DEST_REG(REG_BP);
            TO_WORD(dest, val);

            p++;
        }
        /* 58              017C      pop ax */
        else if (instr == 0x58)
        {
            WORD val;
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
                
            fprintf(logf, "pop %s\n", names[REG_AX]);
            
            regsize = WORD_SIZE;
            GET_SRC_REG(REG_SS);
            seg = FROM_WORD(src);
                
            GET_SRC_REG(REG_SP);
            offs = FROM_WORD(src);

            fromamt = GET_FLATAMT(seg, offs);

            val = WORD_AT_MEM(fromamt);
            
            dest = src;
            TO_WORD(dest, FROM_WORD(src) + 2);

            GET_DEST_REG(REG_AX);
            TO_WORD(dest, val);

            p++;
        }
        /* 59              014C      pop  cx */
        else if (instr == 0x59)
        {
            WORD val;
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
                
            fprintf(logf, "pop %s\n", names[REG_CX]);
            
            regsize = WORD_SIZE;
            GET_SRC_REG(REG_SS);
            seg = FROM_WORD(src);
                
            GET_SRC_REG(REG_SP);
            offs = FROM_WORD(src);

            fromamt = GET_FLATAMT(seg, offs);

            val = WORD_AT_MEM(fromamt);
            
            dest = src;
            TO_WORD(dest, FROM_WORD(src) + 2);

            GET_DEST_REG(REG_CX);
            TO_WORD(dest, val);

            p++;
        }
        /* 5A              014F      pop  dx */
        else if (instr == 0x5a)
        {
            WORD val;
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
                
            fprintf(logf, "pop %s\n", names[REG_DX]);
            
            regsize = WORD_SIZE;
            GET_SRC_REG(REG_SS);
            seg = FROM_WORD(src);
                
            GET_SRC_REG(REG_SP);
            offs = FROM_WORD(src);

            fromamt = GET_FLATAMT(seg, offs);

            val = WORD_AT_MEM(fromamt);
            
            dest = src;
            TO_WORD(dest, FROM_WORD(src) + 2);

            GET_DEST_REG(REG_DX);
            TO_WORD(dest, val);

            p++;
        }
        /* 5B              017B      pop bx */
        else if (instr == 0x5b)
        {
            WORD val;
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
                
            fprintf(logf, "pop %s\n", names[REG_BX]);
            
            regsize = WORD_SIZE;
            GET_SRC_REG(REG_SS);
            seg = FROM_WORD(src);
                
            GET_SRC_REG(REG_SP);
            offs = FROM_WORD(src);

            fromamt = GET_FLATAMT(seg, offs);

            val = WORD_AT_MEM(fromamt);
            
            dest = src;
            TO_WORD(dest, FROM_WORD(src) + 2);

            GET_DEST_REG(REG_BX);
            TO_WORD(dest, val);

            p++;
        }
        /* 8807            mov     [bx],al */
        else if ((instr == 0x88) && (p[1] == 0x07))
        {
            BYTE val;
            WORD seg;
            WORD offs;
            FLATAMT toamt;
            int r;
                
            regsize = BYTE_SIZE;
            r = REG_AL;
            GET_SRC_REG(r);
            val = FROM_BYTE(src);

            GET_SRC_REG(REG_DS);
            seg = FROM_WORD(src);

            GET_SRC_REG(REG_BX);
            offs = FROM_WORD(src);

            fprintf(logf, "mov [%s], %s\n", names[REG_BX], names[r]);

            toamt = GET_FLATAMT(seg, offs);

            BYTE_TO_MEM(toamt, val);
            p += 2;
        }
        /* 8816B501 mov  [BootDisk], dl */
        /* 01B5 is address, offset from ds */
        /* actually 02B5 in mbr.com file because of org 100h */
        else if (instr == 0x88)
        {
            BYTE val;
            WORD seg;
            WORD offs;
            FLATAMT toamt;
                
            regsize = BYTE_SIZE;
            GET_SRC_REG(REG_DL);
            val = FROM_BYTE(src);

            GET_SRC_REG(REG_DS);
            seg = FROM_WORD(src);

            offs = (p[3] << 8) | p[2];

            toamt = GET_FLATAMT(seg, offs);

            if (p[1] != 0x16)
            {
                printf("unknown register %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            fprintf(logf, "mov [%05Xh], %s\n", toamt, names[REG_DL]);
            
            BYTE_TO_MEM(toamt, val);
            p += 4;
        }
        /* 8A0E1000 mov cl, [FatCount] */
        else if (instr == 0x8a)
        {
            BYTE val;
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
                
            regsize = BYTE_SIZE;
            GET_DEST_REG(REG_CL);

            GET_SRC_REG(REG_DS);
            seg = FROM_WORD(src);

            offs = (p[3] << 8) | p[2];

            fromamt = GET_FLATAMT(seg, offs);
            val = BYTE_AT_MEM(fromamt);

            if (p[1] != 0x0e)
            {
                printf("unknown register %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            fprintf(logf, "mov %s, [%05Xh]\n", names[REG_CL], fromamt);
            
            TO_BYTE(dest, val);
            p += 4;
        }
        /* 8907            0418           mov     [bx],ax */
        else if ((instr == 0x89) && (p[1] == 0x07))
        {
            WORD val;
            WORD seg;
            WORD offs;
            FLATAMT toamt;
            int r;
                
            regsize = WORD_SIZE;
            r = REG_AX;
            GET_SRC_REG(r);
            val = FROM_WORD(src);

            GET_SRC_REG(REG_DS);
            seg = FROM_WORD(src);

            GET_SRC_REG(REG_BX);
            offs = FROM_WORD(src);

            fprintf(logf, "mov [%s], %s\n", names[REG_BX], names[r]);

            toamt = GET_FLATAMT(seg, offs);

            WORD_TO_MEM(toamt, val);
            p += 2;
        }
        
        /* 8946FE          001C           mov     [bp-2],ax */
        else if ((instr == 0x89) && (p[1] == 0x46))
        {
            WORD val;
            WORD seg;
            WORD offs;
            FLATAMT toamt;
            int r;
                
            regsize = WORD_SIZE;
            r = REG_AX;
            GET_SRC_REG(r);
            val = FROM_WORD(src);

            GET_SRC_REG(REG_DS);
            seg = FROM_WORD(src);

            GET_SRC_REG(REG_BP);
            offs = FROM_WORD(src);

            if (p[2] >= 0x80)
            {
                offs -= (0x100 - p[2]);
                fprintf(logf, "mov [bp-0%02Xh], %s\n", (0x100 - p[2]), names[r]);
            }
            else
            {
                offs += p[2];
                fprintf(logf, "mov [bp+0%02Xh], %s\n", p[2], names[r]);
            }

            toamt = GET_FLATAMT(seg, offs);

            WORD_TO_MEM(toamt, val);
            p += 3;
        }
        /* 8916B301 mov [Heads], dx */
        /* 890EB101 mov [SectorsPerTrack], cx */
        /* 01B1 is target */
        else if (instr == 0x89)
        {
            WORD val;
            WORD seg;
            WORD offs;
            FLATAMT toamt;
            int r;
                
            regsize = WORD_SIZE;
            if (p[1] == 0x16)
            {
                r = REG_DX;
            }
            else
            {
                r = REG_CX;
            }
            GET_SRC_REG(r);
            val = FROM_WORD(src);

            GET_SRC_REG(REG_DS);
            seg = FROM_WORD(src);

            offs = (p[3] << 8) | p[2];

            toamt = GET_FLATAMT(seg, offs);

            if ((p[1] != 0x16) && (p[1] != 0x0e))
            {
                printf("unknown register %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            fprintf(logf, "mov [%05Xh], %s\n", toamt, names[r]);
            
            BYTE_TO_MEM(toamt, val);
            p += 4;
        }
        /* 8BEE            00A0     mov bp,si */
        else if ((instr == 0x8b) && (p[1] == 0xee))
        {
            regsize = WORD_SIZE;
            fprintf(logf, "mov %s,%s\n", names[REG_BP], names[REG_SI]);

            GET_DEST_REG(REG_BP);
            GET_SRC_REG(REG_SI);
            TO_WORD(dest, FROM_WORD(src));
            
            p += 2;
        }
        /* 8BDA            001E     mov bx, dx */
        else if ((instr == 0x8b) && (p[1] == 0xda))
        {
            regsize = WORD_SIZE;
            fprintf(logf, "mov %s,%s\n", names[REG_BX], names[REG_DX]);

            GET_DEST_REG(REG_BX);
            GET_SRC_REG(REG_DX);
            TO_WORD(dest, FROM_WORD(src));
            
            p += 2;
        }
        /* 8BD8            00A9           mov     bx,ax */
        else if ((instr == 0x8b) && (p[1] == 0xd8))
        {
            regsize = WORD_SIZE;
            fprintf(logf, "mov %s,%s\n", names[REG_BX], names[REG_AX]);

            GET_DEST_REG(REG_BX);
            GET_SRC_REG(REG_AX);
            TO_WORD(dest, FROM_WORD(src));
            
            p += 2;
        }
        /* 8BEC            0001           mov     bp,sp */
        else if ((instr == 0x8b) && (p[1] == 0xec))
        {
            regsize = WORD_SIZE;
            fprintf(logf, "mov %s,%s\n", names[REG_BP], names[REG_SP]);

            GET_DEST_REG(REG_BP);
            GET_SRC_REG(REG_SP);

            TO_WORD(dest, FROM_WORD(src));
            
            p += 2;
        }
        /* 8B07            0586           mov     ax,[bx] */
        else if ((instr == 0x8b) && (p[1] == 0x07))
        {
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
            int r;
            int r2;
                
            regsize = WORD_SIZE;
            if (p[1] == 0x07)
            {
                r = REG_BX;
                r2 = REG_AX;
            }
            else
            {
                printf("unknown register %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            GET_SRC_REG(r);
            offs = FROM_WORD(src);

            GET_SRC_REG(REG_DS);
            seg = FROM_WORD(src);

            fromamt = GET_FLATAMT(seg, offs);
            
            fprintf(logf, "mov %s, [%s]\n", names[r2], names[r]);
            
            GET_DEST_REG(r2);
            TO_WORD(dest, WORD_AT_MEM(fromamt));
            
            p += 2;
        }
        /* 8B4408          0071     mov ax,[si+8] */
        /* 8B540A          0074     mov dx,[si+10] */
        /* 8B4608          0013           mov     ax,[bp+8] */
        else if (instr == 0x8b)
        {
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
            int r;
            int r2;
                
            regsize = WORD_SIZE;
            if ((p[1] == 0x44) || (p[1] == 0x54))
            {
                r = REG_SI;
            }
            else if (p[1] == 0x46)
            {
                r = REG_BP;
            }
            else
            {
                printf("unknown register %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            
            if ((p[1] == 0x44) || (p[1] == 0x46))
            {
                r2 = REG_AX;
            }
            else if (p[1] == 0x54)
            {
                r2 = REG_DX;
            }
            
            GET_SRC_REG(r);
            offs = FROM_WORD(src);

            GET_SRC_REG(REG_DS);
            seg = FROM_WORD(src);

            fromamt = GET_FLATAMT(seg, offs);

            if (p[2] >= 0x80)
            {
                fromamt -= (0x100 - p[2]);
                fprintf(logf, "mov %s, [%s-0%02Xh]\n",
                        names[r2], names[r], (0x100 - p[2]));
            }
            else
            {
                fromamt += p[2];
                fprintf(logf, "mov %s, [%s+0%02Xh]\n",
                        names[r2], names[r], p[2]);
            }
            
            GET_DEST_REG(r2);
            TO_WORD(dest, WORD_AT_MEM(fromamt));
            
            p += 3;
        }
        /* F60480          0030     test byte ptr [si],080h */
        else if (instr == 0xf6)
        {
            BYTE val;
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
                
            if (p[1] != 0x04)
            {
                printf("unknown register %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            
            regsize = WORD_SIZE;
            GET_SRC_REG(REG_SI);
            offs = FROM_WORD(src);

            GET_SRC_REG(REG_DS);
            seg = FROM_WORD(src);

            fromamt = GET_FLATAMT(seg, offs);

            val = BYTE_AT_MEM(fromamt);
            
            fprintf(logf, "test byte ptr [si], 0%02Xh\n", p[2]);
            
            val &= p[2];
            
            zero = (val == 0);
            
            p += 3;
        }
        /* 750A            0033     jnz found_active */
        else if (instr == 0x75)
        {
            BYTE val;
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
            int dir;

            if (p[1] >= 0x80)
            {
                dir = -(0x100 - p[1]);
            }
            else
            {
                dir = p[1];
            }
            fprintf(logf, "jnz 0%05lXh\n", (unsigned long)((p - base) + 2 + dir));

            if (!zero)
            {
                p = p + 2 + dir;
            }
            else
            {
                p += 2; /* this is a 2-byte instruction */
            }
        }
        /* EB09            007A     jmp donemanual */
        else if (instr == 0xeb)
        {
            BYTE val;
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
            int dir;

            if (p[1] >= 0x80)
            {
                dir = -(0x100 - p[1]);
            }
            else
            {
                dir = p[1];
            }
            fprintf(logf, "jmp 0%05lXh\n", (unsigned long)((p - base) + 2 + dir));

            p = p + 2 + dir;
        }
        /* 807C0400 cmp byte ptr [si+4],0; */
        /* don't confuse with 80E13F   and cl,3fh */
        else if ((instr == 0x80) && (p[1] == 0x7c))
        {
            BYTE val;
            WORD seg;
            WORD offs;
            unsigned int extra;
            FLATAMT fromamt;
                
            if (p[1] != 0x7c)
            {
                printf("unknown register %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            
            regsize = WORD_SIZE;
            GET_SRC_REG(REG_SI);
            offs = FROM_WORD(src);

            GET_SRC_REG(REG_DS);
            seg = FROM_WORD(src);

            fromamt = GET_FLATAMT(seg, offs);
            
            extra = p[2]; /* is this signed? */
            fromamt += extra;

            val = BYTE_AT_MEM(fromamt);
            
            fprintf(logf, "cmp byte ptr [si+0%04Xh], 0%02Xh\n", extra, p[3]);
            
            /* are these the right way around? */
            zero = 0;
            if (val == p[3])
            {
                lt = 0;
                gt = 0;
                eq = 1;
                if (val == 0)
                {
                    zero = 1; /* just a guess */
                }
            }
            else if (val < p[3])
            {
                lt = 1;
                gt = 0;
                eq = 0;
            }
            else
            {
                lt = 0;
                gt = 1;
                eq = 0;
            }
            
            p += 4;
        }
        /* 80FA80          007A     cmp dl, 080h */
        else if ((instr == 0x80) && (p[1] == 0xfa))
        {
            BYTE val;
            BYTE val2;

            regsize = BYTE_SIZE;
            GET_SRC_REG(REG_DL);
            val = p[2];
            fprintf(logf, "cmp %s,0%02Xh\n", names[REG_DL], (unsigned int)val);
            val2 = FROM_BYTE(src);
            zero = 0;
            /* is this the right way around? */
            if (val == val2)
            {
                lt = 0;
                gt = 0;
                eq = 1;
                zero = 1; /* it seems that equal also sets zero */
            }
            else if (val < val2)
            {
                lt = 1;
                gt = 0;
                eq = 0;
            }
            else
            {
                lt = 0;
                gt = 1;
                eq = 0;
            }
            p += 3;
        }
        /* 80E13F   and cl,3fh */
        else if (instr == 0x80)
        {
            BYTE val;

            if (p[1] != 0xe1)
            {
                printf("unknown register %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            
            regsize = BYTE_SIZE;
            GET_DEST_REG(REG_CL);
            val = p[2];
            fprintf(logf, "and %s,0%02Xh\n", names[REG_CL], (unsigned int)val);
            TO_BYTE(dest, val);
            p += 3;
        }
        /* 2500FF          0046      and ax, 0ff00h */
        else if (instr == 0x25)
        {
            WORD val;

            regsize = WORD_SIZE;
            GET_DEST_REG(REG_AX);
            val = (p[2] << 8) | p[1];
            fprintf(logf, "and %s,0%04Xh\n", names[REG_AX], (unsigned int)val);
            TO_WORD(dest, val);
            p += 3;
        }
        /* FEC1            0138      inc  cl */
        else if (instr == 0xfe)
        {
            BYTE val;

            if (p[1] != 0xc1)
            {
                printf("unknown register %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            
            regsize = BYTE_SIZE;
            GET_SRC_REG(REG_CL);
            val = FROM_BYTE(src);
            val++;
            dest = src;
            fprintf(logf, "inc cl\n");
            TO_BYTE(dest, val);
            p += 2;
        }
        /* 45              00AD     inc bp */
        else if (instr == 0x45)
        {
            BYTE val;

            regsize = BYTE_SIZE;
            GET_SRC_REG(REG_BP);
            val = FROM_BYTE(src);
            val++;
            dest = src;
            fprintf(logf, "inc bp\n");
            TO_BYTE(dest, val);
            p++;
        }
        /* 83C201          006A      add dx,1 */
        /* 83C001          014B       add  ax,1 */
        /* 83C4FE          0010           add     sp,-2 */
        else if ((instr == 0x83)
                 && ((p[1] == 0xc2)
                     || (p[1] == 0xc0)
                     || (p[1] == 0xc4)
                    )
                )
        {
            BYTE val;
            WORD val2;
            WORD oldval2;
            int r;

            if (p[1] == 0xc2)
            {
                r = REG_DX;
            }
            else if (p[1] == 0xc0)
            {
                r = REG_AX;
            }
            else if (p[1] == 0xc4)
            {
                r = REG_SP;
            }
            else
            {
                printf("unknown register %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            
            regsize = WORD_SIZE;
            GET_SRC_REG(r);
            val2 = FROM_WORD(src);
            dest = src;
            val = p[2];
            oldval2 = val2;
            if (val >= 0x80)
            {
                val2 -= (0x100 - val);
                if (val2 > oldval2)
                {
                    cflag = 1;
                }
                else
                {
                    cflag = 0;
                }
            }
            else
            {
                val2 += val;
                if (val2 < oldval2)
                {
                    cflag = 1;
                }
                else
                {
                    cflag = 0;
                }
            }
            fprintf(logf, "add %s,0%02Xh\n", names[r], (unsigned int)val);
            TO_WORD(dest, val2);
            p += 3;
        }
        /* 83D200          014E       adc  dx,0 */
        else if ((instr == 0x83) && (p[1] == 0xd2))
        {
            BYTE val;
            WORD val2;
            WORD oldval2;
            int r;

            if (p[1] == 0xd2)
            {
                r = REG_DX;
            }
            else
            {
                printf("unknown register %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            
            regsize = WORD_SIZE;
            GET_SRC_REG(r);
            val2 = FROM_WORD(src);
            dest = src;
            val = p[2];
            oldval2 = val2;
            val2 += (val + cflag);
            if (val2 < oldval2)
            {
                cflag = 1;
            }
            else
            {
                cflag = 0;
            }
            fprintf(logf, "adc %s,0%02Xh\n", names[r], (unsigned int)val);
            TO_WORD(dest, val2);
            p += 3;
        }
        /* B600            0068      mov dh,0 */
        else if (instr == 0xb6)
        {
            BYTE val;

            regsize = BYTE_SIZE;
            GET_DEST_REG(REG_DH);
            val = p[1];
            fprintf(logf, "mov %s,0%02Xh\n", names[REG_DH], (unsigned int)val);
            TO_BYTE(dest, val);
            p += 2;
        }
        /* B000            0145      mov  al, 0 */
        else if (instr == 0xb0)
        {
            BYTE val;

            regsize = BYTE_SIZE;
            GET_DEST_REG(REG_AL);
            val = p[1];
            fprintf(logf, "mov %s,0%02Xh\n", names[REG_AL], (unsigned int)val);
            TO_BYTE(dest, val);
            p += 2;
        }
        /* B102            0148      mov  cl, 2 */
        else if (instr == 0xb1)
        {
            BYTE val;

            regsize = BYTE_SIZE;
            GET_DEST_REG(REG_CL);
            val = p[1];
            fprintf(logf, "mov %s,0%02Xh\n", names[REG_CL], (unsigned int)val);
            TO_BYTE(dest, val);
            p += 2;
        }
        /* BDD800          0043     mov bp,offset active_partition_invalid */
        else if (instr == 0xbd)
        {
            WORD val;

            regsize = WORD_SIZE;
            GET_DEST_REG(REG_BP);
            val = p[2] << 8 | p[1];
            fprintf(logf, "mov %s,0%04Xh\n", names[REG_BP], (int)val);
            TO_WORD(dest, val);
            p += 3;
        }
        /* 7466            0046     jz fatal_error */
        else if (instr == 0x74)
        {
            BYTE val;
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
            int dir;

            if (p[1] >= 0x80)
            {
                dir = -(0x100 - p[1]);
            }
            else
            {
                dir = p[1];
            }
            fprintf(logf, "jz 0%05lXh\n", (unsigned long)((p - base) + 2 + dir));

            if (zero)
            {
                p = p + 2 + dir;
            }
            else
            {
                p += 2; /* this is a 2-byte instruction */
            }
        }
        /* 26C706FE 7D0000    mov word ptr es:[07dfeh],0h */
        else if ((instr == 0x26) && (p[1] == 0xc7))
        {
            WORD val;
            WORD seg;
            WORD offs;
            FLATAMT toamt;
                
            regsize = WORD_SIZE;
            
            val = (p[6] << 8) + p[5];
            
            GET_SRC_REG(REG_ES);
            seg = FROM_WORD(src);

            offs = (p[4] << 8) | p[3];

            toamt = GET_FLATAMT(seg, offs);

            if (p[1] != 0xc7)
            {
                printf("unknown register %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            if (p[2] != 0x06)
            {
                printf("unknown register %x\n", p[2]);
                exit(EXIT_FAILURE);
            }
            fprintf(logf, "mov word ptr es:[%04Xh], 0%04Xh\n", offs, val);
            
            WORD_TO_MEM(toamt, val);
            p += 7;
        }
        /* 26813EFE  7D55AA   cmp word ptr es:[07dfeh],0aa55h */
        else if ((instr == 0x26) && (p[1] == 0x81))
        {
            WORD val;
            WORD val2;
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
                
            regsize = WORD_SIZE;
            
            val = (p[6] << 8) + p[5];
            
            GET_SRC_REG(REG_ES);
            seg = FROM_WORD(src);

            offs = (p[4] << 8) | p[3];

            fromamt = GET_FLATAMT(seg, offs);

            if (p[1] != 0x81)
            {
                printf("unknown register %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            if (p[2] != 0x3e)
            {
                printf("unknown register %x\n", p[2]);
                exit(EXIT_FAILURE);
            }
            fprintf(logf, "cmp word ptr es:[%04Xh], 0%04Xh\n", offs, val);
            
            val2 = WORD_AT_MEM(fromamt);
            
            zero = 0;
            /* is this the right way around? */
            if (val == val2)
            {
                lt = 0;
                gt = 0;
                eq = 1;
                zero = 1; /* it seems that equal also sets zero */
            }
            else if (val < val2)
            {
                lt = 1;
                gt = 0;
                eq = 0;
            }
            else
            {
                lt = 0;
                gt = 1;
                eq = 0;
            }
            
            p += 7;
        }
        /* 26881602 7C mov es:[07C02h],dl */
        else if ((instr == 0x26) && (p[1] == 0x88))
        {
            BYTE val;
            BYTE val2;
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
                
            regsize = BYTE_SIZE;
            
            GET_SRC_REG(REG_DL);
            val = FROM_BYTE(src);

            GET_SRC_REG(REG_ES);
            seg = FROM_WORD(src);

            offs = (p[4] << 8) | p[3];

            fromamt = GET_FLATAMT(seg, offs);

            if (p[1] != 0x88)
            {
                printf("unknown register %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            if (p[2] != 0x16)
            {
                printf("unknown register %x\n", p[2]);
                exit(EXIT_FAILURE);
            }
            fprintf(logf, "mov es:[0%04Xh], dl\n", offs);
            
            val2 = BYTE_AT_MEM(fromamt);

            zero = 0;
            /* is this the right way around? */
            if (val == val2)
            {
                lt = 0;
                gt = 0;
                eq = 1;
                zero = 1; /* it seems that equal also sets zero */
            }
            else if (val < val2)
            {
                lt = 1;
                gt = 0;
                eq = 0;
            }
            else
            {
                lt = 0;
                gt = 1;
                eq = 0;
            }
            
            p += 5;
        }
        /* 26881602 7C  mov es:[07C02h],dl */
        /* F736B101 div  word ptr [SectorsPerTrack] */
        else if ((instr == 0xf7) && (p[1] == 0x36))
        {
            WORD val;
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
            DWORD pair;
                
            regsize = WORD_SIZE;
            
            val = (p[6] << 8) + p[5];
            
            GET_SRC_REG(REG_DS);
            seg = FROM_WORD(src);

            offs = (p[3] << 8) | p[2];

            fromamt = GET_FLATAMT(seg, offs);
            val = WORD_AT_MEM(fromamt);

            if (p[1] != 0x36)
            {
                printf("unknown register %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            fprintf(logf, "div word ptr [0%04Xh]\n", offs);
            
            GET_SRC_REG(REG_DX);
            pair = FROM_WORD(src);
            GET_SRC_REG(REG_AX);
            pair = (pair << 16) + FROM_WORD(src);

            /* testing says these are the right way around */            
            GET_DEST_REG(REG_AX);
            TO_WORD(dest, pair / val);

            GET_DEST_REG(REG_DX);
            TO_WORD(dest, pair % val);
            
            p += 4;
        }
        /* F7E1            00C6      mul cx */
        /* F7E9            05B4           imul    cx */
        /* F7F9            0638           idiv    cx */        
        else if (instr == 0xf7)
        {
            WORD val;
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
            DWORD pair;
            int i = 0; /* this says whether it is imul - presumably
                      signed multiply */
            int div = 0;
                
            regsize = WORD_SIZE;
            
            if (p[1] == 0xe1)
            {
                /* nothing */
            }
            else if (p[1] == 0xe9)
            {
                i = 1;
            }
            else if (p[1] == 0xf9)
            {
                i = 1;
                div = 1;
            }
            else
            {
                printf("unknown register %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            if (i)
            {
                if (div)
                {
                    /* i don't think we need idiv */
                    fprintf(logf, "idiv cx - changed to div\n");
                }
                else
                {
                    /* i dont think we need imul */
                    fprintf(logf, "imul cx - changed to mul\n");
                }
            }
            else
            {
                fprintf(logf, "mul cx\n");
            }
            
            GET_SRC_REG(REG_DX);
            pair = FROM_WORD(src);
            GET_SRC_REG(REG_AX);
            pair = (pair << 16) + FROM_WORD(src);

            GET_SRC_REG(REG_CX);
            val = FROM_WORD(src);
            
            if (div)
            {
                DWORD temppair;
                
                if (val == 0)
                {
                    printf("divide by zero\n");
                    exit(EXIT_FAILURE);
                }
                /* are these the right way around? */
                temppair = pair;
                pair %= val;
                temppair /= val;
                pair <<= 16;
                pair |= temppair & 0xffffU;
            }
            else
            {
                pair *= val;
            }
            
            /* guess this is the right way around */
            GET_DEST_REG(REG_AX);
            TO_WORD(dest, pair & 0xffffU);

            GET_DEST_REG(REG_DX);
            TO_WORD(dest, (pair >> 16) & 0xffffU);
            
            p += 2;
        }
        /* B408            0054      mov ah, 08h */
        else if (instr == 0xb4)
        {
            BYTE val;

            regsize = BYTE_SIZE;
            GET_DEST_REG(REG_AH);
            val = p[1];
            fprintf(logf, "mov %s,0%02Xh\n", names[REG_AH], (unsigned int)val);
            TO_BYTE(dest, val);
            p += 2;
        }
        /* B500            0060      mov ch, 0 */
        else if (instr == 0xb5)
        {
            BYTE val;

            regsize = BYTE_SIZE;
            GET_DEST_REG(REG_CH);
            val = p[1];
            fprintf(logf, "mov %s,0%02Xh\n", names[REG_CH], (unsigned int)val);
            TO_BYTE(dest, val);
            p += 2;
        }
        /* CD13            0059      int 13h */
        else if (instr == 0xcd)
        {
            if ((p[1] == 0x13) || (p[1] == 0x10))
            {
                dobios(p[1]);
            }
            else
            {
                printf("unknown interrupt %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            p += 2;
        }
        /* 721F            005B      jc ignorec */
        else if (instr == 0x72)
        {
            BYTE val;
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
            int dir;

            if (p[1] >= 0x80)
            {
                dir = -(0x100 - p[1]);
            }
            else
            {
                dir = p[1];
            }
            fprintf(logf, "jc 0%05lXh\n", (unsigned long)((p - base) + 2 + dir));

            if (cflag)
            {
                p = p + 2 + dir;
            }
            else
            {
                p += 2; /* this is a 2-byte instruction */
            }
        }
        /* 7C02            0070           jl      L8 */
        else if (instr == 0x7c)
        {
            BYTE val;
            WORD seg;
            WORD offs;
            int dir;

            if (p[1] >= 0x80)
            {
                dir = -(0x100 - p[1]);
            }
            else
            {
                dir = p[1];
            }
            fprintf(logf, "jl 0%05lXh\n", (unsigned long)((p - base) + 2 + dir));

            if (lt)
            {
                p = p + 2 + dir;
            }
            else
            {
                p += 2; /* this is a 2-byte instruction */
            }
        }
        /* 7305            0171       jnc  fin */
        else if (instr == 0x73)
        {
            BYTE val;
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
            int dir;

            if (p[1] >= 0x80)
            {
                dir = -(0x100 - p[1]);
            }
            else
            {
                dir = p[1];
            }
            fprintf(logf, "jnc 0%05lXh\n", (unsigned long)((p - base) + 2 + dir));

            if (!cflag)
            {
                p = p + 2 + dir;
            }
            else
            {
                p += 2; /* this is a 2-byte instruction */
            }
        }
        /* 3C00            00B3     cmp al,0 */
        else if (instr == 0x3c)
        {
            BYTE val;
            BYTE val2;

            regsize = BYTE_SIZE;
            GET_SRC_REG(REG_AL);
            val = p[1];
            fprintf(logf, "cmp %s,0%02Xh\n", names[REG_AL], (unsigned int)val);
            val2 = FROM_BYTE(src);
            zero = 0;
            /* is this the right way around? */
            if (val == val2)
            {
                lt = 0;
                gt = 0;
                eq = 1;
                zero = 1; /* it seems that equal also sets zero */
            }
            else if (val < val2)
            {
                lt = 1;
                gt = 0;
                eq = 0;
            }
            else
            {
                lt = 0;
                gt = 1;
                eq = 0;
            }
            p += 2;
        }
        /* 3D0001          0049      cmp  ax, 0100h */
        else if (instr == 0x3d)
        {
            WORD val;
            WORD val2;

            regsize = WORD_SIZE;
            GET_SRC_REG(REG_AX);
            val = (p[2] << 8) | p[1];
            fprintf(logf, "cmp %s,0%04Xh\n", names[REG_AX], (unsigned int)val);
            val2 = FROM_BYTE(src);
            zero = 0;
            /* is this the right way around? */
            if (val == val2)
            {
                lt = 0;
                gt = 0;
                eq = 1;
                zero = 1; /* it seems that equal also sets zero */
            }
            else if (val < val2)
            {
                lt = 1;
                gt = 0;
                eq = 0;
            }
            else
            {
                lt = 0;
                gt = 1;
                eq = 0;
            }
            p += 3;
        }
        /* 81C4CCFD add     sp,-564 */
        else if ((instr == 0x81) && (p[1] == 0xc4))
        {
            WORD val;
            WORD val2;

            regsize = WORD_SIZE;
            val = (p[3] << 8) | p[2];
            
            fprintf(logf, "add %s,0%04Xh\n", names[REG_SP], (unsigned int)val);
            
            GET_SRC_REG(REG_SP);
            val2 = FROM_WORD(src);
            
            dest = src;
            
            if (p[3] >= 0x80)
            {
                TO_WORD(dest, val2 - (0x10000UL - val));
            }
            else
            {
                TO_WORD(dest, val2 + val);
            }
            
            p += 4;
        }
        /* 81FB55AA cmp bx,0aa55h */
        else if (instr == 0x81)
        {
            WORD val;
            WORD val2;

            if (p[1] != 0xfb)
            {
                printf("unknown register %x\n", p[1]);
                exit(EXIT_FAILURE);
            }
            
            regsize = WORD_SIZE;
            GET_SRC_REG(REG_BX);
            val = (p[3] << 8) | p[2];
            fprintf(logf, "cmp %s,0%04Xh\n", names[REG_BX], (unsigned int)val);
            val2 = FROM_BYTE(src);
            zero = 0;
            /* is this the right way around? */
            if (val == val2)
            {
                lt = 0;
                gt = 0;
                eq = 1;
                zero = 1; /* it seems that equal also sets zero */
            }
            else if (val < val2)
            {
                lt = 1;
                gt = 0;
                eq = 0;
            }
            else
            {
                lt = 0;
                gt = 1;
                eq = 0;
            }
            p += 4;
        }
        /* 87DB            00B7     xchg bx,bx */
        else if ((instr == 0x87) && (p[1] == 0xdb))
        {
            fprintf(logf, "xchg bx, bx\n");
            /* effective nop */
            p += 2;
        }
        /* 91              0411           xchg    ax,cx */
        else if (instr == 0x91)
        {
            WORD temp;
            
            fprintf(logf, "xchg ax, cx\n");
            GET_SRC_REG(REG_CX);
            temp = FROM_WORD(src);
            dest = src;
            GET_SRC_REG(REG_AX)
            TO_WORD(dest, FROM_WORD(src));
            dest = src;
            TO_WORD(dest, temp);
            p++;
        }
        /* 90 nop */
        else if (instr == 0x90)
        {
            fprintf(logf, "nop\n");
            p++;
        }
        /* 99              0637           cwd */
        else if (instr == 0x99)
        {
            WORD val;
            
            fprintf(logf, "cwd\n");
            
            GET_SRC_REG(REG_AX);
            val = FROM_WORD(src);
            if (val & 0x8000U)
            {
                val = 0xffffU;
            }
            else
            {
                val = 0;
            }
            GET_DEST_REG(REG_DX);
            TO_WORD(dest, val);
            p++;
        }
        /* F4              00BA     hlt */
        else if (instr == 0xF4)
        {
            fprintf(logf, "hlt\n");
            exit(EXIT_FAILURE);
            p++;
        }
        /* E2F1            0151       loop ReadNextSector */
        else if (instr == 0xe2)
        {
            WORD val;
            int dir;

            if (p[1] >= 0x80)
            {
                dir = -(0x100 - p[1]);
            }
            else
            {
                dir = p[1];
            }
            fprintf(logf, "loop 0%05lXh\n", (unsigned long)((p - base) + 2 + dir));

            GET_SRC_REG(REG_CX);
            dest = src;
            val = FROM_WORD(src);
            val--;
            TO_WORD(dest, val);
            if (val != 0)
            {
                p = p + 2 + dir;
            }
            else
            {
                p += 2; /* this is a 2-byte instruction */
            }
        }
        /* 8D1E1E00 lea bx, newstart */
        /* not sure why this instruction is any different from mov -
           maybe it just doesn't generate relocation info */
        else if ((instr == 0x8d) && (p[1] == 0x1e))
        {
            WORD val;

            regsize = WORD_SIZE;
            GET_DEST_REG(REG_BX);
            val = p[3] << 8 | p[2];
            fprintf(logf, "lea %s,0%04Xh\n", names[REG_BX], (unsigned int)val);
            TO_WORD(dest, val);
            p += 4;
        }
        /* 8D46E0          00D2           lea     ax,[bp-32] */
        else if ((instr == 0x8d) && (p[1] == 0x46))
        {
            WORD val;
            BYTE val2;

            regsize = WORD_SIZE;
            GET_SRC_REG(REG_BP);
            val = FROM_WORD(src);
            val2 = p[2];
            if (p[2] >= 0x80)
            {
                val -= (0x100U - val2);
                fprintf(logf, "lea %s,[bp-0%02Xh]\n",
                        names[REG_AX], (unsigned int)(0x100U - val2));
            }
            else
            {
                val += val2;
                fprintf(logf, "lea %s,[bp+0%02Xh]\n",
                        names[REG_AX], (unsigned int)val2);
            }
            /* even though it looks like I'm supposed to
               indirect, that's not what lea does */
            GET_DEST_REG(REG_AX);
            TO_WORD(dest, val);
            p += 3;
        }
        /* 8D86D2FD lea     ax,[bp-558] */
        else if ((instr == 0x8d) && (p[1] == 0x86))
        {
            WORD val;
            WORD val2;

            regsize = WORD_SIZE;
            GET_SRC_REG(REG_BP);
            val = FROM_WORD(src);
            val2 = p[3] << 8 | p[2];
            if (p[3] >= 0x80)
            {
                val -= (0x10000UL - val2);
                fprintf(logf, "lea %s,[bp-0%04Xh]\n",
                        names[REG_AX], (unsigned int)(0x10000UL - val2));
            }
            else
            {
                val += val2;
                fprintf(logf, "lea %s,[bp+0%04Xh]\n",
                        names[REG_AX], (unsigned int)val2);
            }
            /* do I need to indirect the value against ss or ds or
               does that defeat the purpose of lea vs mov? */
            GET_DEST_REG(REG_AX);
            TO_WORD(dest, val);
            p += 4;
        }
        /* C55E04       lds bx,xaddr */
        /* load ds and bx from the address at bp plus offset */
        else if ((instr == 0xc5) && (p[1] == 0x5e))
        {
            WORD val;
            WORD seg;
            WORD offs;
            FLATAMT fromamt;
                
            regsize = WORD_SIZE;
            
            GET_SRC_REG(REG_BP);
            offs = FROM_WORD(src);
            
            GET_SRC_REG(REG_DS);
            seg = FROM_WORD(src);

            fromamt = GET_FLATAMT(seg, offs);
            
            fromamt += p[2];
            
            val = WORD_AT_MEM(fromamt);

            GET_DEST_REG(REG_BX);
            TO_WORD(dest, val);
            
            fromamt += 2;
            
            val = WORD_AT_MEM(fromamt);

            GET_DEST_REG(REG_DS);
            TO_WORD(dest, val);
            
            fprintf(logf, "lds bx, [bp + 0%02Xh]\n", p[2]);
            
            p += 3;
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

        /* 8ED8            mov ds,ax */
        /* 8EC0            mov es,ax */
        /* 8ED0            mov ss,ax */
        /* 8EDA            mov ds,dx */
        /* 8ED2            mov ss,dx */
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

    if (second == 0x0)
    {
        r2 = REG_AX;
    }
    else if (second == 0x2)
    {
        r2 = REG_DX;
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
        else if (r1 == REG_SS)
        {
            dest = &sregs.ss;
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
        else if (r2 == REG_DX)
        {
            src = &regs.x.dx;
        }
    }

    return;
}

static void dobios(int intnum)
{
    if (intnum == 0x13)
    {
        cflag = 0;
        /* get drive parms */
        if (regs.h.ah == 0x08)
        {
            regs.h.cl = 63; /* sectors */
            regs.h.dh = 255-1; /* 255 heads */
            /* ch and cl again should have tracks */
        }
        /* read */
        else if (regs.h.ah == 0x02)
        {
            unsigned int track;
            unsigned int head;
            unsigned int sector;
            unsigned int num_sectors = 1;
            unsigned long lba;
            
            head = regs.h.dh;
            track = (unsigned int)(regs.h.cl & 0xc0) << 2;
            track += regs.h.ch;
            sector = regs.h.cl & 0x3f;
#if 1
            printf("track %d\n", track);
            printf("head %d\n", head);
            printf("sector %d\n", sector);
#endif
            lba = track * 255 * 63;
            lba += head * 63;
            lba += sector;
            lba--;
            lba *= 512;
            fseek(fp, lba, SEEK_SET);
#if 1
            printf("lba is %lx\n", lba);
            printf("READING to %lx\n", ((FLATAMT)sregs.es << 4) + regs.x.bx);
#endif
            fread(base + ((FLATAMT)sregs.es << 4) + regs.x.bx, 512, 1, fp);
        }
        else if (regs.h.ah == 0x41)
        {
            regs.x.bx = 0xaa55; /* signify we know the call */
            regs.x.cx = 0; /* but no capabilities for now */
        }
        else
        {
            printf("unknown int 13h %x\n", regs.h.ah);
            exit(EXIT_FAILURE);
        }
    }
    else if (intnum == 0x10)
    {
        if (regs.h.ah == 0x0e)
        {
            printf("%c", regs.h.al);
        }
        else
        {
            printf("unknown int10h\n");
            exit(EXIT_FAILURE);
        }
    }
    return;
}
