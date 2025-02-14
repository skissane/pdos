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

/*
You need to do:
D:\zpg>dasdcopy cfba1b1.vhd fba1b1.vhd
copy (or build with makecomm.zpg) pcomm.exe from tapes to this
  local directory, then:
mfemul ..\generic\pdos.exe \zpg\fba1b1.vhd
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


#if 0
#define DEBUG 1
#define DUMPREGS 1
#endif

#if COMEMUL
#define MAXPRGSZ (10 * 0x10000)
#endif

#if PBEMUL
static size_t memsize = 64*1024*1024L;
#endif

/* we shouldn't be relying on exact types */
typedef int I32;
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

static char *boot_name;

static unsigned long icount = 0;

static U32 regs[16];

static int x1;
static int x2;
static int t;
static int i;
static int imm;
static int b;
static int d;
static int b1;
static int d1;
static int b2;
static int d2;
static int l;

/*
static int lt;
static int gt;
static int eq;
*/
/* static int zero; */ /* i don't think this is a separate flag - just use eq */

/* cc of 2 is +, cc of 1 is -, cc of 0 is 0 */
static int cc = 0;

static FILE *handles[FOPEN_MAX];

static char cmd_line[400] = "prog !BOOT";

static void doemul(void);
static void splitrx(void);
static void splitsi(void);
static void splitrs(void);
static void splitrr(void);
static void splitssl(void);
static void writereg(unsigned char *z, int x);
static void updatereg(U32 *x, unsigned char *z);
static int febc(int ebc);
static int tebc(int local);
static int check_add32(int a, int b);
static int check_sub32(int a, int b);

int main(int argc, char **argv)
{
    FILE *fp;
    unsigned char *entry_point;
    char *exe;

#if COMEMUL
    if (argc <= 1)
    {
        printf("usage: mfemul <COM>\n");
        printf("executes a mainframe .com file\n");
        return (EXIT_FAILURE);
    }
#endif

#if PBEMUL
    if (argc <= 2)
    {
        printf("usage: mfemul <OS> <disk>\n");
        printf("executes a mainframe OS that relies on a pseudo-bios\n");
        return (EXIT_FAILURE);
    }
    /* this probably needs to change */
    setvbuf(stdin, NULL, _IONBF, 0);
#endif

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
    {
        int x = 0;

        while (cmd_line[x] != '\0')
        {
            cmd_line[x] = tebc(cmd_line[x]);
            x++;
        }
    }
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
    boot_name = argv[2];
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
    
    handles[0] = stdin;
    handles[1] = stdout;
    handles[2] = stderr;
    
    p = base + 1024 * 1024L;
    /* exception code so that we don't have a NULL pointer */
    /* putfullword(p + 7 * sizeof(U32), 0); */ /* stdin handle */
    putfullword(p + 7 * sizeof(U32), FOPEN_MAX); /* stdin handle */
    putfullword(p + 8 * sizeof(U32), 1); /* stdout handle */
    putfullword(p + 38 * sizeof(U32), 2); /* stderr handle */

    /* where to store address of main() */
    putfullword(p + 5 * sizeof(U32), 0x7C);

    /* copy in command line */
    strncpy((char *)base + 0x40, cmd_line, 0x3c - 1);
    /* should probably do the conversion to EBCDIC here */
    putfullword(p + 3 * sizeof(U32), 0x40);

    /* memory size */
    putfullword(p + 1 * sizeof(U32), memsize - 3 * 1024 * 1024L);

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
    regs[14] = 0; /* branch to zero will terminate */
    regs[13] = 0x100; /* save area */
    putfullword(base + 0x100 + 76, 0x100 + 80);
    p = entry_point;
#endif

    doemul();
#if PBEMUL
    printf("program returned with hex %x\n", regs[15]);
    printf("instruction count is %ld\n", icount);
    /* this may need to change */
    setvbuf(stdin, NULL, _IOLBF, 0);
#endif
    return (EXIT_SUCCESS);
}

static void spec_call(int val)
{
    unsigned long parms;

#if DEBUG
    printf("got a special call %d\n", val);
#endif
    parms = regs[1];
#if DEBUG
    printf("parms are %08lX\n", parms);
#endif

    if (val == 14) /* fwrite */
    {
        unsigned char *buf;
        size_t sz;
        size_t num;
        int fq;
        
        buf = base + getfullword(base + parms);
#if DEBUG
        printf("buf is %p\n", buf);
#endif

        sz = (size_t)getfullword(base + parms + sizeof(U32));
#if DEBUG
        printf("sz is %x\n", (int)sz);
#endif
        num = (size_t)getfullword(base + parms + sizeof(U32)*2);
#if DEBUG
        printf("num is %x\n", (int)num);
#endif
        fq = getfullword(base + parms + sizeof(U32)*3);
#if DEBUG
        printf("fq is %d\n", fq);
#endif
        if ((handles[fq] == stdout) || (handles[fq] == stderr))
        {
            size_t tot;
            size_t x;

            tot = sz * num;
            for (x = 0; x < tot; x++)
            {
                fputc(febc(buf[x]), handles[fq]);
                fflush(handles[fq]);
            }
        }
        else
        {
            fwrite(buf, sz, num, handles[fq]);
        }
    }
    else if (val == 10) /* fopen */
    {
        static int cnt = 0;

        if (cnt == 0)
        {        
            /* assume main disk */
            handles[3] = fopen(boot_name, "r+b");
#if DEBUG
            printf("fopen got %p\n", handles[3]);
#endif
            regs[15] = 3;
        }
        else if (cnt == 1)
        {
            /* assume pcomm.exe */
            handles[4] = fopen("pcomm.exe", "r+b");
#if DEBUG
            printf("fopen got %p\n", handles[4]);
#endif
            regs[15] = 4;
        }
        cnt++;
    }
    else if (val == 13) /* fclose */
    {
        static int cnt = 0;

        if (cnt == 0)
        {
            /* assume pcomm.exe */
            fclose(handles[4]);
            handles[4] = NULL;
            regs[15] = 0;
        }
        else if (cnt == 1)
        {        
            /* assume main disk */
            fclose(handles[3]);
            handles[3] = NULL;
            regs[15] = 0;
        }
        cnt++;
    }
    else if (val == 11) /* fseek */
    {
        long offset;
        int whence;
        int fq;
        
        fq = getfullword(base + parms + sizeof(U32)*0);
#if DEBUG
        printf("fq is %d\n", fq);
#endif
        offset = getfullword(base + parms + sizeof(U32)*1);
#if DEBUG
        printf("offset is %d\n", offset);
#endif
        whence = getfullword(base + parms + sizeof(U32)*2);
#if DEBUG
        printf("whence is %d\n", whence);
#endif
        regs[15] = fseek(handles[fq], offset, whence);
    }
    else if (val == 12) /* fread */
    {
        int handle;
        size_t sz;
        size_t nr;
        unsigned char *ptr;

        ptr = base + getfullword(base + parms + sizeof(U32)*0);
        sz = getfullword(base + parms + sizeof(U32)*1);
        nr = getfullword(base + parms + sizeof(U32)*2);
        handle = getfullword(base + parms + sizeof(U32)*3);
        /* exception code so that we don't have a NULL pointer */
        if (handle == FOPEN_MAX)
        {
            handle = 0;
            /* exception code for dealing with unbuffered input */
            sz = 1;
            nr = 1;
        }
        regs[15] = fread((char *)ptr,
                         sz,
                         nr,
                         handles[handle]);
        if (handle == 0)
        {
            *ptr = tebc(*ptr);
        }
    }
    else if (val == 23) /* fflush */
    {
        /* ignore for now */
    }
    else if (val == 24) /* setvbuf */
    {
        /* ignore for now */
    }
    else if (val == 40) /* free */
    {
        /* ignore for now */
        /* they will be freeing their memory block */
    }
    else if (val == 7) /* malloc */
    {
        /* they will be doing a single alloc - point them to
           the 3 MiB location */
        regs[15] = 3 * 1024 * 1024L;
    }
    else if (val == 1) /* start */
    {
        U32 oldrx1;
        int oldx1;

        oldx1 = x1;
        oldrx1 = regs[x1];
        /* we don't have a C library to call in this environment,
           so just immediately call their main program */
        putfullword(base + 0x80, 0); /* argc = 0 */
        regs[1] = 0x80;
        regs[14] = 0;
        /* regs[15] = *(U32 *)(base + 1 * 1024 * 1024L + 5 * sizeof(U32)); */

        /* called OS should have stored address here */
        regs[15] = getfullword(base + 0x7C);
#if DEBUG
        printf("r15 is %08x\n", regs[15]);
#endif
        p = base + regs[15];
        /*exit(0);*/
        doemul();
#if DEBUG
        printf("returning from emulation\n");
#endif
        x1 = oldx1;
        regs[x1] = oldrx1;
    }
    else
    {
        printf("unknown callback %d\n", val);
        exit(EXIT_FAILURE);
    }

    return;

}

static void doemul(void)
{
    unsigned char *watching = base + 0x21943C;
    
#if COMEMUL
    regs[13] = 0x100; /* save area */
    regs[15] = 0x10000; /* entry point */
    regs[14] = 0; /* branch to zero will terminate */
#endif

    while (1)
    {
        instr = *p;
        icount++;
#if COMEMUL
        printf("instr is %02X at %08X watching %02X, r14 %08X\n", instr, p - base - 0x10000, *watching, regs[14]);
#endif
#if PBEMUL
#if DEBUG
        printf("\ninstr is %02X at %08X watching %02X, r14 %08X\n", instr, p - base, *watching, regs[14]);
        printf("icount is %ld\n", icount);
#endif
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
#if DEBUG
            printf("cond %02x, eq %d, lt %d, gt %d\n", x1, eq, lt, gt);
#endif
            if ((x1 == 0) || (x2 == 0))
            {
#if DEBUG
                printf("noop\n");
#endif
            }
            /* unconditional branch */
            else if (x1 == 0xf)
            {
                p = base + regs[x2];
#if DEBUG
                printf("updating with %x %x\n", x2, regs[x2]);
                printf("base %p, p %p, at p is %x\n", base, p, *p);
#endif
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
            /* BHR */
            else if (x1 == 0x2)
            {
                if (cc == 2)
                {
                    p = base + regs[x2];
#if DEBUG
                    printf("updating with %x %x\n", x2, regs[x2]);
                    printf("base %p, p %p, at p is %x\n", base, p, *p);
#endif
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
            }
            /* BLR */
            else if (x1 == 0x4)
            {
                if (cc == 1)
                {
                    p = base + regs[x2];
#if DEBUG
                    printf("updating with %x %x\n", x2, regs[x2]);
                    printf("base %p, p %p, at p is %x\n", base, p, *p);
#endif
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
            }
            /* BNHR */
            else if (x1 == 0xd)
            {
                if (!(cc == 2))
                {
                    p = base + regs[x2];
#if DEBUG
                    printf("updating with %x %x\n", x2, regs[x2]);
                    printf("base %p, p %p, at p is %x\n", base, p, *p);
#endif
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
            }
            /* BNER */
            else if (x1 == 0x7)
            {
                if (!(cc == 0))
                {
                    p = base + regs[x2];
#if DEBUG
                    printf("updating with %x %x\n", x2, regs[x2]);
                    printf("base %p, p %p, at p is %x\n", base, p, *p);
#endif
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
            }
            /* BER */
            else if (x1 == 0x8)
            {
                if (cc == 0)
                {
                    p = base + regs[x2];
#if DEBUG
                    printf("updating with %x %x\n", x2, regs[x2]);
                    printf("base %p, p %p, at p is %x\n", base, p, *p);
#endif
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
            }
            /* BNLR */
            else if (x1 == 0xb)
            {
               if (!(cc == 1))
                {
                    p = base + regs[x2];
#if DEBUG
                    printf("updating with %x %x\n", x2, regs[x2]);
                    printf("base %p, p %p, at p is %x\n", base, p, *p);
#endif
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
#if DEBUG
                printf("x2 is %x, regsx2 is %x\n", x2, regs[x2]);
#endif
#if PBEMUL
                if (regs[x2] < 300)
                {
#if DEBUG
                    printf("special call\n");
#endif
                    spec_call(regs[x2]);
                    p = base + regs[x1];
                }
                else
                {
                    p = base + regs[x2];
#if DEBUG
                    printf("new address is %08X\n", regs[x2]);
#endif
                }
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
#if DEBUG
            printf("new value of %x is %08X\n", x1, regs[x1]);
#endif
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
#if DEBUG
            printf("new value of %x is %08X\n", t, regs[t]);
#endif
            p += 4;
        }
        else if (instr == 0x43) /* ic */
        {
            int one = 0;
            int two = 0;
            unsigned int val;

            splitrx();            
            if (b != 0)
            {
                one = regs[b];
            }
            if (i != 0)
            {
                two = regs[i];
            }
            val = base[one + two + d];
            regs[t] &= 0xffffff00UL;
            regs[t] |= val;
#if DEBUG
            printf("new value of %x is %08X\n", t, regs[t]);
#endif
            p += 4;
        }
        else if (instr == 0x4c) /* mh */
        {
            int one = 0;
            int two = 0;
            int val;

            splitrx();
            if (b != 0)
            {
                one = regs[b];
            }
            if (i != 0)
            {
                two = regs[i];
            }
            val = (short)gethalfword(&base[one + two + d]);
            regs[t] *= val;
#if DEBUG
            printf("new value of %x is %08X\n", t, regs[t]);
#endif
            p += 4;
        }
        else if (instr == 0x5c) /* m */
        {
            int one = 0;
            int two = 0;
            int val;

            splitrx();
            if (b != 0)
            {
                one = regs[b];
            }
            if (i != 0)
            {
                two = regs[i];
            }
            val = getfullword(&base[one + two + d]);
            /* +++ this takes and returns a result in a register pair, but
               I don't know how to do that, so I am just ignoring the
               first register, ie regs[t] */
            regs[t + 1] = (I32)regs[t + 1] * val;
#if DEBUG
            printf("new value of %x is %08X\n", t, regs[t+1]);
#endif
            p += 4;
        }
        else if (instr == 0x6c) /* md */
        {
            /*a dummy instruction, not implement yet */
            splitrx();
            p += 4;
        }
        else if (instr == 0x6d) /* dd */
        {
            /*a dummy instruction, not implement yet */
            splitrx();
            p += 4;
        }
        else if (instr == 0xbd) /* clm */
        {
            int x;
            unsigned char *target;
            int one = 0;
            int mask;
            int val;
            
            splitrs();
            mask = x2;
            if (b != 0)
            {
                one = regs[b];
            }
            target = base + one + d;
#if DEBUG
            printf("comparing at offset %x\n", (target - base));
            printf("base %x, displacement %x\n", one, d);
            printf("mask %x\n", mask);
#endif
            p += 4;
            x = 0;
            cc = 0;
            if ((mask & 0x8) != 0)
            {
                val = (regs[x1] >> 24) & 0xff;
                if (val > target[x])
                {
                    cc = 2;
                    continue;
                }
                else if (val < target[x])
                {
                    cc = 1;
                    continue;
                }
                x++;
            }
            if ((mask & 0x4) != 0)
            {
                val = (regs[x1] >> 16) & 0xff;
                if (val > target[x])
                {
                    cc = 2;
                    continue;
                }
                else if (val < target[x])
                {
                    cc = 1;
                    continue;
                }
                x++;
            }
            if ((mask & 0x2) != 0)
            {
                val = (regs[x1] >> 8) & 0xff;
                if (val > target[x])
                {
                    cc = 2;
                    continue;
                }
                else if (val < target[x])
                {
                    cc = 1;
                    continue;
                }
                x++;
            }
            if ((mask & 0x1) != 0)
            {
                val = regs[x1] & 0xff;
#if DEBUG
                printf("val is %x, target is %x\n", val, target[x]);
#endif
                if (val > target[x])
                {
                    cc = 2;
#if DEBUG
                    printf("gt detected\n");
#endif
                    continue;
                }
                else if (val < target[x])
                {
                    cc = 1;
                    continue;
                }
                x++;
            }
            cc = 0;
        }
        else if (instr == 0x88) /* srl */
        {
            int x = 0;
            
            splitrs();
            if (b != 0)
            {
                x = regs[b];
            }
            x += d;
            x &= 0x3f;
            regs[x1] >>= x;
            p += 4;
        }
        else if (instr == 0x8a) /* sra */
        {
            int x = 0;
            /* +++ suggest */
            splitrs();
            if (b != 0) 
            {
                x = regs[b];
            }
            x += d;
            x &= 0x3f;
            regs[x1] = (((I32)regs[x1]) >> x);
            cc = ((I32)regs[x1] > 0) ? 2 : ((I32)regs[x1] < 0) ? 1 : 0; 
            p += 4;
        }
        else if (instr == 0x8b) /* sla */
        {
            int i, j;
            int x = 0;
            U32 t1 = 0;
            U32 t2 = 0;

            /* +++ suggest */
            splitrs();
            t1 = regs[x1] & 0x7FFFFFFF;
            /*the sign of the first operand remains unchanged.
            All 31 numeric bits of the operand participate in the left shift.
            */
            t2 = regs[x1] & 0x80000000; 

            if (b != 0)
            {
                x = regs[b];
            }
            x += d;
            x &= 0x3f;

            j = 0;
            for (i = 0; i < x; i++)
            {
                t1 <<= 1;
                if ((t1 & 0x80000000) != t2) j = 1;
            }
            regs[x1] = (t1 & 0x7FFFFFFF) | t2;
            cc = (j == 1) ? 3 : ((I32)regs[x1] > 0) ? 2 : ((I32)regs[x1] < 0) ? 1 : 0;
            p += 4;
        }
        else if (instr == 0x8e) /* srda */
        {
            int x = 0;

            splitrs();
            if (b != 0)
            {
                x = regs[b];
            }
            x += d;
            x &= 0x3f;
            
            if (x >= 32)
            {
                regs[x1+1] = regs[x1];
                regs[x1] = 0;
                regs[x1+1] >>= (x-32);
            }
            else
            {
                regs[x1+1] >>= x;
                regs[x1+1] |= (regs[x1] << (32-x));
                regs[x1] >>= x;
            }
            /*need handle overflow case*/
            cc = ((I32)regs[x1] > 0) ? 2 : ((I32)regs[x1] < 0) ? 1 : 0;
            p += 4;
        }
        else if (instr == 0x8f) /* slda */
        {
            int x = 0;

            splitrs();
            if (b != 0)
            {
                x = regs[b];
            }
            x += d;
            x &= 0x3f;
            
            if (x >= 32)
            {
                regs[x1] = regs[x1+1];
                regs[x1+1] = 0;
                regs[x1] <<= (x-32);
            }
            else
            {
                regs[x1] <<= x;
                regs[x1] |= (regs[x1+1] >> (32-x));
                regs[x1+1] <<= x;
            }
            /*need handle overflow case*/
            cc = ((I32)regs[x1+1] > 0) ? 2 : ((I32)regs[x1+1] < 0) ? 1 : 0;
            p += 4;
        }
        else if (instr == 0x89) /* sll */
        {
            int x = 0;

            splitrs();
            if (b != 0)
            {
                x = regs[b];
            }
            x += d;
            x &= 0x3f;
            regs[x1] <<= x;
            
            p += 4;
        }
        else if (instr == 0x8c) /* srdl */
        {
            int x;
            int amt;

            splitrs();
            amt = p[3];
            if (amt >= 32)
            {
                regs[x1+1] = regs[x1];
                regs[x1] = 0;
                regs[x1+1] >>= (amt-32);
            }
            else
            {
                regs[x1+1] >>= amt;
                regs[x1+1] |= (regs[x1] << (32-amt));
                regs[x1] >>= amt;
            }
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
#if DEBUG
            printf("storing to offset %x\n", (target - base));
            printf("base %x, displacement %x\n", one, d);
#endif
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
        else if (instr == 0x0e) /* mvcl */
        {
            splitrr();
            /* +++ I think the x1+1 and x2+2 can be different lengths
               and padding should be done - not sure if a pad byte is
               included - I think it is - which is why the length is
               restricted to 16 MiB */
            memcpy(base + regs[x1], base + regs[x2], regs[x1+1]);
            /* and the length registers may count down to 0 */
            /* I think the main registers count up */
            /* not sure what overlapping addresses do either, so not */
            /* sure whether to use memmove */
            regs[x1] += regs[x1+1];
            regs[x2] += regs[x2+1];
            cc = (regs[x1+1] > regs[x2+1]) ? 2 : (regs[x1+1] < regs[x2+1]) ? 1 : 0;
            p += 2;
        }
        else if (instr == 0x1d) /* dr */
        {
            U32 value;
            U32 remainder;

            splitrr();
            /* +++ I believe the x1 and x1+1 are a pair, but I don't
               know how to do that, so I'll just ignore x1 */
            value = regs[x1+1] / regs[x2];
            remainder = regs[x1+1] % regs[x2];
            regs[x1] = remainder;
            regs[x1+1] = value;
            p += 2;
        }
        else if (instr == 0x1c) /* mr */
        {
            splitrr();
            /* +++ This takes and stores result in a register pair,
               but I don't know how to do that, so we ignore the
               first register, ie regs[x1] */
            regs[x1 + 1] = (I32)regs[x1 +  1] * (I32)regs[x2];
            p += 2;
        }
        else if (instr == 0x18) /* lr */
        {
            splitrr();
            regs[x1] = regs[x2];
            p += 2;
        }
        else if (instr == 0x13) /* lcr */
        {
            int r1 = 0;
            splitrr();
            r1 = regs[x1];
            regs[x1] = -regs[x2];
            cc = (check_sub32(r1, (I32)regs[x2]) != 0) ? 3 : ((I32)regs[x1] > 0) ? 2 : ((I32)regs[x1] < 0) ? 1 : 0;
            p += 2;
        }
        else if (instr == 0x10) /* lpr */
        {
            int r1;
            splitrr();
            r1 = regs[x1];
            /* +++ just guessing */
            if ((I32)regs[x2] < 0)
            {
                regs[x1] = -regs[x2];
            }
            else
            {
                regs[x1] = regs[x2];
            }
            cc = (check_sub32(r1, (I32)regs[x2]) != 0) ? 3 : ((I32)regs[x1] > 0) ? 2 : 0;
            p += 2;
        }
        else if (instr == 0x11) /* lnr */
        {
            splitrr();
            /* +++ just guessing */
            if ((I32)regs[x2] > 0)
            {
                regs[x1] = -regs[x2];
            }
            else
            {
                regs[x1] = regs[x2];
            }
            cc = ((I32)regs[x1] < 0) ? 1 : 0;
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
#if DEBUG
            printf("new value of %x is %08X\n", t, regs[t]);
#endif
            p += 4;
        }
        else if (instr == 0x68) /* ld */
        {
            /*a dummy instruction, not implement yet */
            splitrx();
            p += 4;
        }
        else if (instr == 0x69) /* cd */
        {
            /*a dummy instruction, not implement yet */
            splitrx();
            cc = 0;
            p += 4;
        }
        else if (instr == 0x48) /* lh */
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
            regs[t] = (short)((v[0] << 8) | v[1]);
#if DEBUG
            printf("new value of %x is %08X\n", t, regs[t]);
#endif
            p += 4;
        }
        else if (instr == 0x40) /* sth */
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
            v[0] = (regs[t] >> 8) & 0xff;
            v[1] = regs[t] & 0xff;

#if DEBUG
            printf("new value of %x is %08X\n", t, regs[t]);
#endif
            p += 4;
        }
        else if (instr == 0x49) /* ch */
        {
            int one = 0;
            int two = 0;
            unsigned char *v;
            int val;

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
            val = (short)gethalfword(v);
            cc = ((I32)regs[t] > val) ? 2 : ((I32)regs[t] < val) ? 1 : 0;
            p += 4;
        }
        else if (instr == 0x59) /* c */
        {
            int one = 0;
            int two = 0;
            unsigned char *v;
            I32 val;

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
            val = getfullword(v);
            cc = ((I32)regs[t] > val) ? 2 : ((I32)regs[t] < val) ? 1 : 0;
            p += 4;
        }
        else if (instr == 0x28) /* ldr */
        {
            /*a dummy instruction, not implement yet */
            splitrr();
            p += 2;
        }
        else if (instr == 0x29) /* cdr */
        {
            /*a dummy instruction, not implement yet */
            splitrr();
            cc = 0;
            p += 2;
        }
        else if (instr == 0x2a) /* adr */
        {
            /*a dummy instruction, not implement yet */
            splitrr();
            cc = 0;
            p += 2;
        }
        else if (instr == 0x2b) /* sdr */
        {
            /*a dummy instruction, not implement yet */
            splitrr();
            cc = 0;
            p += 2;
        }
        else if (instr == 0x2c) /* mdr */
        {
            /*a dummy instruction, not implement yet */
            splitrr();
            p += 2;
        }
        else if (instr == 0x2d) /* ddr */
        {
            /*a dummy instruction, not implement yet */
            splitrr();
            p += 2;
        }
        else if (instr == 0x5a) /* a */
        {
            int one = 0;
            int two = 0;
            int x;
            int r1;
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
            r1 = regs[t];
            x = (v[0] << 24) | (v[1] << 16) | (v[2] << 8) | v[3];
            regs[t] += x;
            cc = (check_add32(r1, x) != 0) ? 3 : ((I32)regs[t] > 0) ? 2 : ((I32)regs[t] < 0) ? 1 : 0;
#if DEBUG
            printf("new value of %x is %08X\n", t, regs[t]);
#endif    
            p += 4;
        }
        else if (instr == 0x4a) /* ah */
        {
            int one = 0;
            int two = 0;
            unsigned char *v;
            int val = 0;
            int r1;

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
            val = (short)gethalfword(v);
            r1 = regs[t];
            regs[t] += val;
            cc = (check_add32(r1, val) != 0) ? 3 : ((I32)regs[t] > 0) ? 2 : ((I32)regs[t] < 0) ? 1 : 0;
#if DEBUG
            printf("new value of %x is %08X\n", t, regs[t]);
#endif
            p += 4;
        }
        else if (instr == 0x5b) /* s */
        {
            int one = 0;
            int two = 0;
            int x;
            int r1;
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
            x = (v[0] << 24) | (v[1] << 16) | (v[2] << 8) | v[3];
            r1 = regs[t];
            regs[t] -= x;
            cc = (check_sub32(r1, x)) ? 3 : ((I32)regs[t] > 0) ? 2 : ((I32)regs[t] < 0) ? 1 : 0;
#if DEBUG
            printf("new value of %x is %08X\n", t, regs[t]);
#endif

            /* we need to set the eq flag at least */
            /* for when the result is 0 */
            p += 4;
        }
        else if (instr == 0x5f) /* sl */
        {
            int one = 0;
            int two = 0;
            int x;
            int r1;
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
            x = (v[0] << 24) | (v[1] << 16) | (v[2] << 8) | v[3];
            r1 = regs[t];
            regs[t] -= x;
            cc = (check_sub32(r1, x)) ? 3 : (regs[t] > 0) ? 2 : 0;
#if DEBUG
            printf("new value of %x is %08X\n", t, regs[t]);
#endif

            /* we need to set the eq flag at least */
            /* for when the result is 0 */
            p += 4;
        }
        else if (instr == 0x6a) /* ad */
        {
            /*a dummy instruction, not implement yet */
            splitrx();
            cc = 0;
            p += 4;
        }
        else if (instr == 0x6b) /* sd */
        {
            /*a dummy instruction, not implement yet */
            splitrx();
            p += 4;
        }
        else if (instr == 0x4b) /* sh */
        {
            int one = 0;
            int two = 0;
            int val = 0;
            int r1;
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
            val = (short)gethalfword(v);
            r1 = regs[t];
            regs[t] -= val;
            cc = (check_sub32(r1, val) != 0) ? 3 : ((I32)regs[t] > 0) ? 2 : ((I32)regs[t] < 0) ? 1 : 0;
#if DEBUG
            printf("new value of %x is %08X\n", t, regs[t]);
#endif

            /* we need to set the eq flag at least */
            /* for when the result is 0 */
            p += 4;
        }
        else if (instr == 0x55) /* cl */
        {
            int one = 0;
            int two = 0;
            unsigned char *v;
            unsigned long val;

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
            val = getfullword(v);
            cc = ((I32)regs[t] > val) ? 2 : ((I32)regs[t] < val) ? 1 : 0;
            p += 4;
        }
        else if (instr == 0x5d) /* d */
        {
            int one = 0;
            int two = 0;
            unsigned char *v;
            I32 op2;
            I32 quot;
            I32 rem;

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
            op2 = getfullword(v);
            if (op2 != 0)
            {
                /* I assume reges[t] to 0 since we are only 
                dealing with 32-bit C-generated code at the moment.*/
                quot = ((I32)regs[t + 1]) / op2;
                rem = ((I32)regs[t + 1]) % op2;
                regs[t] = (U32)quot;
                regs[t + 1] = (U32)rem;
            }
            p += 4;
        }
        else if (instr == 0x54) /* n */
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
            regs[t] &= (v[0] << 24) | (v[1] << 16) | (v[2] << 8) | v[3];
            cc = (regs[t] != 0) ? 1 : 0;
#if DEBUG
            printf("new value of %x is %08X\n", t, regs[t]);
#endif
            p += 4;
        }
        else if (instr == 0x57) /* x */
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
            regs[t] ^= (v[0] << 24) | (v[1] << 16) | (v[2] << 8) | v[3];
            cc = (regs[t] != 0) ? 1 : 0;
#if DEBUG
            printf("new value of %x is %08X\n", t, regs[t]);
#endif
            p += 4;
        }
        else if (instr == 0x56) /* o */
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
            regs[t] |= (v[0] << 24) | (v[1] << 16) | (v[2] << 8) | v[3];
            cc = (regs[t] != 0) ? 1 : 0;
#if DEBUG
            printf("new value of %x is %08X\n", t, regs[t]);
#endif
            p += 4;
        }
        else if (instr == 0x92) /* mvi */
        {
            unsigned long one = 0;
            unsigned char *v;

            splitsi();
            if (b != 0)
            {
                one = regs[b];
            }
            one += d;
            v = base + one;
            *v = imm;
#if DEBUG
            printf("moved %x to %08X\n", imm, one);
#endif
            p += 4;
        }
        else if (instr == 0x94) /* ni */
        {
            unsigned long one = 0;
            unsigned char *v;

            splitsi();
            if (b != 0)
            {
                one = regs[b];
            }
            one += d;
            v = base + one;
            *v &= imm;
            cc = ((int)*v != 0) ? 1 : 0;
            p += 4;
        }
        else if (instr == 0x95) /* cli */
        {
            unsigned long one = 0;
            unsigned char *v;
            int val;

            splitsi();
            if (b != 0)
            {
                one = regs[b];
            }
            one += d;
            v = base + one;
            val = *v;
            cc = (val > imm) ? 2 : (val < imm) ? 1 : 0;
            p += 4;
        }
        else if (instr == 0x96) /* oi */
        {
            unsigned long one = 0;
            unsigned char *v;

            splitsi();
            if (b != 0)
            {
                one = regs[b];
            }
            one += d;
            v = base + one;
            *v |= imm;
            cc = ((int)*v != 0) ? 1 : 0;
            p += 4;
        }
        else if (instr == 0x97) /* xi */
        {
            unsigned long one = 0;
            unsigned char *v;

            splitsi();
            if (b != 0)
            {
                one = regs[b];
            }
            one += d;
            v = base + one;
            *v ^= imm;
            cc = ((int)*v != 0) ? 1 : 0;
            p += 4;
        }
        else if (instr == 0x1a) /* ar */
        {
            int r1;
            splitrr();
            r1 = regs[x1];
            regs[x1] += regs[x2];
            cc = (check_add32(r1, (I32)regs[x2]) != 0) ? 3 : ((I32)regs[x1] > 0) ? 2 : ((I32)regs[x1] < 0) ? 1 : 0;
            p += 2;
        }
        else if (instr == 0x1b) /* sr */
        {
            int r1;
            splitrr();
            r1 = regs[x1];
            regs[x1] -= regs[x2];
            cc = (check_sub32(r1, (I32)regs[x2]) != 0) ? 3 : ((I32)regs[x1] > 0) ? 2 : ((I32)regs[x1] < 0) ? 1 : 0;
            p += 2;
        }
        else if (instr == 0x14) /* nr */
        {
            splitrr();
            regs[x1] &= regs[x2];
            cc = (regs[x1] != 0) ? 1 : 0;
            p += 2;
        }
        else if (instr == 0x16) /* or */
        {
            splitrr();
            regs[x1] |= regs[x2];
            cc = (regs[x1] != 0) ? 1 : 0;
            p += 2;
        }
        else if (instr == 0x17) /* xr */
        { 
            splitrr();
            regs[x1] ^= regs[x2];
            cc = (regs[x1] != 0) ? 1 : 0;
            p += 2;
        }
        else if (instr == 0x1f) /* slr */
        {
            int r;
            splitrr();
            r = regs[x1];
            regs[x1] -= regs[x2];
            cc = ((I32)regs[x1] == 0 ? 0 : 1) | (r < (I32)regs[x1] ? 0 : 2);
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
#if DEBUG
            printf("new dest is %08X\n", dest - 0x10000);
#endif
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
        else if (instr == 0x60) /* std */
        {
            /*a dummy instruction, not implement yet */
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
            memset(v, '\x00', 8);
            p += 4;
        }
        else if (instr == 0xb2) /* stck */
        {
            /*a dummy instruction, not implement yet */
            int one = 0;
            unsigned char *v;

            splitsi(); /* this should be splits but we don't have one yet */
               /* also - this should be further distinguished to just x'05'
                  as STCK - ie B205 */
            if (b != 0)
            {
                one = regs[b];
            }
            v = base + one + d;
            memset(v, '\x00', 8);
            p += 4;
        }
        else if (instr == 0xd2) /* mvc */
        {
            int one = 0;
            int two = 0;

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
#if DEBUG
            printf("writing to address %x %p\n", one, base + one);
#endif
            p += 6;
        }
        else if (instr == 0xd4) /* nc */
        {
            int one = 0;
            int two = 0;
            int i;
            unsigned char *v, *z;

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

            v = base + one;
            z = base + two;
            cc = 0;
            for(i = 0; i < l + 1; i++)
            {
                v[i] = (v[i] & z[i]);
                if (v[i] != 0) cc = 1;
            }
            p += 6;
        }
        else if (instr == 0xd5) /* clc */
        {
            int one = 0;
            int two = 0;
            int ret;

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
            ret = memcmp(base + one, base + two, l + 1);
            cc = (ret > 0) ? 2 : (ret < 0) ? 1 : 0;
            p += 6;
        }
        else if (instr == 0xd6) /* oc */
        {
            int one = 0;
            int two = 0;
            int i;
            unsigned char *v, *z;

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

            v = base + one;
            z = base + two;
            cc = 0;
            for(i = 0; i < l + 1; i++)
            {
                v[i] = (v[i] | z[i]);
                if (v[i] != 0) cc = 1;
            }
            p += 6;
        }
        else if (instr == 0xd7) /* xc */
        {
            int one = 0;
            int two = 0;
            int i;
            unsigned char *v, *z;

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

            v = base + one;
            z = base + two;
            cc = 0;
            for(i = 0; i < l + 1; i++)
            {
                v[i] = (v[i] ^= z[i]);
                if (v[i] != 0) cc = 1;
            }
            p += 6;
        }
        else if (instr == 0x19) /* cr */
        {
            splitrr();
            cc = ((I32)regs[x1] > (I32)regs[x2]) ? 2 : ((I32)regs[x1] < (I32)regs[x2]) ? 1 : 0;
            p += 2;
        }
        else if (instr == 0x15) /* clr */
        {
            splitrr();
            cc = (regs[x1] > regs[x2]) ? 2 : (regs[x1] < regs[x2]) ? 1 : 0;
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
#if DEBUG
            printf("cond %02x, eq %d, lt %d, gt %d\n", cond, eq, lt, gt);
#endif
            /* bl */
            if (cond == 0x40)
            {
                if (cc == 1)
                {
                    p = base + one + d;
                    continue;
                }
            }
            /* bnz or bne */
            else if (cond == 0x70)
            {
                if (!(cc == 0))
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
            /* bnh */
            else if (cond == 0xd0)
            {
                if (cc == 1 || cc == 0)
                {
                    p = base + one + d;
                    continue;
                }
            }
            /* bnl */
            else if (cond == 0xb0)
            {
                if (cc == 2 || cc == 0)
                {
                    p = base + one + d;
                    continue;
                }
            }
            /* be */
            else if (cond == 0x80)
            {
                if (cc == 0)
                {
                    p = base + one + d;
                    continue;
                }
            }
            /* bh */
            else if (cond == 0x20)
            {
                if (cc == 2)
                {
                    p = base + one + d;
                    continue;
                }
            }
            else
            {
                printf("unknown condition %x at %08X\n", cond, p - base);
                exit(EXIT_FAILURE);
            }
            p += 4;
        }
        else if (instr == 0x12) /* ltr */
        {
            splitrr();
            regs[x1] = regs[x2];
            cc = ((I32)regs[x1] > 0) ? 2 : ((I32)regs[x1] < 0) ? 1 : 0;
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
#if DEBUG
            printf("loading from offset %x\n", (target - base));
            printf("displacement %x\n", d);
#endif
            if (x2 < x1)
            {
                end = 15;
                for (x = start; x <= end; x++)
                {
#if DEBUG
                   printf("updating reg %x currently %x\n", x, regs[x]);
#endif
                   updatereg(&regs[x], target);
#if DEBUG
                   printf("new value %x\n", regs[x]);
#endif
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
            unsigned long dest;

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

static void splitsi(void)
{
    imm = p[1];
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

static void updatereg(U32 *x, unsigned char *z)
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

/*********************************************************************/
/*                                                                   */
/*  This Program Written By Paul Edwards.                            */
/*  Released to the public domain.                                   */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  This program takes an integer as a parameter.  The integer       */
/*  should have a character in the local character set.  It will be  */
/*  converted into the EBCDIC equivalent.                            */
/*                                                                   */
/*********************************************************************/

#if 0
static int tebc(int local)
{
  switch (local)
  {
    case '\n' : return (0x15);
    case '\f' : return (0x0c);
    case ' '  : return (0x40);
    case '!'  : return (0x5A);
    case '\"' : return (0x7f);
    case '#'  : return (0x7b);
    case '$'  : return (0x5b);
    case '%'  : return (0x6c);
    case '&'  : return (0x50);
    case '\'' : return (0x7d);
    case '('  : return (0x4d);
    case ')'  : return (0x5d);
    case '*'  : return (0x5c);
    case '+'  : return (0x4e);
    case ','  : return (0x6b);
    case '-'  : return (0x60);
    case '.'  : return (0x4b);
    case '/'  : return (0x61);
    case '0'  : return (0xf0);
    case '1'  : return (0xf1);
    case '2'  : return (0xf2);
    case '3'  : return (0xf3);
    case '4'  : return (0xf4);
    case '5'  : return (0xf5);
    case '6'  : return (0xf6);
    case '7'  : return (0xf7);
    case '8'  : return (0xf8);
    case '9'  : return (0xf9);
    case ':'  : return (0x7a);
    case ';'  : return (0x5e);
    case '<'  : return (0x4c);
    case '='  : return (0x7e);
    case '>'  : return (0x6e);
    case '?'  : return (0x6f);
    case '@'  : return (0x7c);
    case 'A'  : return (0xc1);
    case 'B'  : return (0xc2);
    case 'C'  : return (0xc3);
    case 'D'  : return (0xc4);
    case 'E'  : return (0xc5);
    case 'F'  : return (0xc6);
    case 'G'  : return (0xc7);
    case 'H'  : return (0xc8);
    case 'I'  : return (0xc9);
    case 'J'  : return (0xd1);
    case 'K'  : return (0xd2);
    case 'L'  : return (0xd3);
    case 'M'  : return (0xd4);
    case 'N'  : return (0xd5);
    case 'O'  : return (0xd6);
    case 'P'  : return (0xd7);
    case 'Q'  : return (0xd8);
    case 'R'  : return (0xd9);
    case 'S'  : return (0xe2);
    case 'T'  : return (0xe3);
    case 'U'  : return (0xe4);
    case 'V'  : return (0xe5);
    case 'W'  : return (0xe6);
    case 'X'  : return (0xe7);
    case 'Y'  : return (0xe8);
    case 'Z'  : return (0xe9);
    case '['  : return (0xad);
    case '\\' : return (0xe0);
    case ']'  : return (0xbd);
    case '^'  : return (0x5f);
    case '_'  : return (0x6d);
    case '`'  : return (0x79);
    case 'a'  : return (0x81);
    case 'b'  : return (0x82);
    case 'c'  : return (0x83);
    case 'd'  : return (0x84);
    case 'e'  : return (0x85);
    case 'f'  : return (0x86);
    case 'g'  : return (0x87);
    case 'h'  : return (0x88);
    case 'i'  : return (0x89);
    case 'j'  : return (0x91);
    case 'k'  : return (0x92);
    case 'l'  : return (0x93);
    case 'm'  : return (0x94);
    case 'n'  : return (0x95);
    case 'o'  : return (0x96);
    case 'p'  : return (0x97);
    case 'q'  : return (0x98);
    case 'r'  : return (0x99);
    case 's'  : return (0xa2);
    case 't'  : return (0xa3);
    case 'u'  : return (0xa4);
    case 'v'  : return (0xa5);
    case 'w'  : return (0xa6);
    case 'x'  : return (0xa7);
    case 'y'  : return (0xa8);
    case 'z'  : return (0xa9);
    case '{'  : return (0xc0);
    case '|'  : return (0x4f);
    case '}'  : return (0xd0);
    case '~'  : return (0xa1);
    case 0x1b : return (0x27);  /* ESC character */
    default   : return (0);
  }
}
#endif

static unsigned char atoe[256] = {
"\x00\x01\x02\x03\x37\x2D\x2E\x2F\x16\x05\x15\x0B\x0C\x0D\x0E\x0F"
"\x10\x11\x12\x13\x3C\x3D\x32\x26\x18\x19\x3F\x27\x1C\x1D\x1E\x1F"
"\x40\x5A\x7F\x7B\x5B\x6C\x50\x7D\x4D\x5D\x5C\x4E\x6B\x60\x4B\x61"
"\xF0\xF1\xF2\xF3\xF4\xF5\xF6\xF7\xF8\xF9\x7A\x5E\x4C\x7E\x6E\x6F"
"\x7C\xC1\xC2\xC3\xC4\xC5\xC6\xC7\xC8\xC9\xD1\xD2\xD3\xD4\xD5\xD6"
"\xD7\xD8\xD9\xE2\xE3\xE4\xE5\xE6\xE7\xE8\xE9\xAD\xE0\xBD\x5F\x6D"
"\x79\x81\x82\x83\x84\x85\x86\x87\x88\x89\x91\x92\x93\x94\x95\x96"
"\x97\x98\x99\xA2\xA3\xA4\xA5\xA6\xA7\xA8\xA9\xC0\x4F\xD0\xA1\x07"
"\x20\x21\x22\x23\x24\x25\x06\x17\x28\x29\x2A\x2B\x2C\x09\x0A\x1B"
"\x30\x31\x1A\x33\x34\x35\x36\x08\x38\x39\x3A\x3B\x04\x14\x3E\xFF"
"\x41\xAA\x4A\xB1\x9F\xB2\x6A\xB5\xBB\xB4\x9A\x8A\xB0\xCA\xAF\xBC"
"\x90\x8F\xEA\xFA\xBE\xA0\xB6\xB3\x9D\xDA\x9B\x8B\xB7\xB8\xB9\xAB"
"\x64\x65\x62\x66\x63\x67\x9E\x68\x74\x71\x72\x73\x78\x75\x76\x77"
"\xAC\x69\xED\xEE\xEB\xEF\xEC\xBF\x80\xFD\xFE\xFB\xFC\xBA\xAE\x59"
"\x44\x45\x42\x46\x43\x47\x9C\x48\x54\x51\x52\x53\x58\x55\x56\x57"
"\x8C\x49\xCD\xCE\xCB\xCF\xCC\xE1\x70\xDD\xDE\xDB\xDC\x8D\x8E\xDF"
};

static int tebc(int local)
{
    return atoe[local];
}

/* return 1 if overflow
 * return -1 if underflow
 * else return 0
 */
static int check_add32(int a, int b) 
{
    if(b > 0 && a > (INT_MAX - b)) 
        return 1;
    else if (b < 0 && a < (INT_MIN - b)) 
        return -1;
    else 
        return 0; 
}

static int check_sub32(int a, int b)
{
    if(b < 0 && a > (INT_MAX + b)) 
        return 1;
    else if (b > 0 && a < (INT_MIN + b)) 
        return -1;
    else 
        return 0; 
}
