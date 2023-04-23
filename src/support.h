/* public domain by Paul Edwards */

#ifndef SUPPORT_INCLUDED
#define SUPPORT_INCLUDED

#ifdef __32BIT__
typedef struct {
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
    unsigned int esi;
    unsigned int edi;
    unsigned int cflag;
    unsigned int flags;
    unsigned int ebp;
} DWORDREGS;

typedef struct {
    unsigned short ax;
    unsigned short fill1;
    unsigned short bx;
    unsigned short fill2;
    unsigned short cx;
    unsigned short fill3;
    unsigned short dx;
    unsigned short fill4;
    unsigned short si;
    unsigned short fill5;
    unsigned short di;
    unsigned short fill6;
    unsigned short cflag;
    unsigned short fill7;    
    unsigned short flags;
    unsigned short fill8;
    unsigned short bp;
    unsigned short fill9;
} WORDREGS;

typedef struct {
    unsigned char al;
    unsigned char ah;
    unsigned short fill1;
    unsigned char bl;
    unsigned char bh;
    unsigned short fill2;
    unsigned char cl;
    unsigned char ch;
    unsigned short fill3;
    unsigned char dl;
    unsigned char dh;
    unsigned short fill4;
} BYTEREGS;

#else

typedef struct {
    unsigned short ax;
    unsigned short bx;
    unsigned short cx;
    unsigned short dx;
    unsigned short si;
    unsigned short di;
    unsigned short cflag;
    unsigned short flags;
    unsigned short bp;
} WORDREGS;

typedef struct {
    unsigned char al;
    unsigned char ah;
    unsigned char bl;
    unsigned char bh;
    unsigned char cl;
    unsigned char ch;
    unsigned char dl;
    unsigned char dh;
} BYTEREGS;

#endif

union REGS
{
    BYTEREGS h;
    WORDREGS x;
#ifdef __32BIT__    
    DWORDREGS d;
#endif    
};

struct SREGS
{
    unsigned short es;
    unsigned short cs;
    unsigned short ss;
    unsigned short ds;
};

#ifdef __32BIT__
#define far
void enable(void);
void disable(void);
#else

#ifdef __SMALLERC__
#define ADDR2ABS(x) ((unsigned long)(x))
#define ABS2ADDR(x) ((void *)(x))
#else
/* ADDR2ABS - convert a 16:16 address into a flat absolute address */
#define ADDR2ABS(x) ((((unsigned long)(void far *)(x) >> 16) << 4) \
                   + ((unsigned long)(void far *)(x) & 0xffffU))
#define CADDR2ABS(x) ((((unsigned long)(void (far *)())(x) >> 16) << 4) \
                   + ((unsigned long)(void (far *)())(x) & 0xffffU))
/* ABS2ADDR - convert a flat address into a 16:16 pointer */
#define ABS2ADDR(x) ((void far *)((((unsigned long)(x) & 0xf0000UL) << 12) \
                   | ((unsigned long)(x) & 0xffff)))
#endif
#endif

#ifdef __SMALLERC__
/* Smaller C uses flat addresses */
#define MK_FP(x, y) (void *)(((unsigned long)(x) << 4) | (y))
#define FP_OFF(x) (unsigned int)(((unsigned long)(void *)(x)) & 0xfU)
#define FP_SEG(x) (unsigned int)((((unsigned long)(void *)(x)) & 0x000ffff0UL) >> 4)
#define FP_NORM(x) (void *)(x)
#else
#define MK_FP(x, y) (void far *)(((unsigned long)(x) << 16) | (y))
#define FP_OFF(x) (unsigned int)((unsigned long)(void far *)(x) & 0xffffU)
#define FP_SEG(x) (unsigned int)((unsigned long)(void far *)(x) >> 16)
#define FP_NORM(x) (void *)(((((unsigned long)(x) >> 16) \
                     + (((unsigned long)(x) & 0xffffU) / 16)) << 16) \
                     | (((unsigned long)(x) & 0xffffU) % 16))
#endif

#if defined(__WATCOMC__) && !defined(WATNATIVE)
#define CTYP __cdecl
#else
#define CTYP
#endif

int CTYP int86(int intno, union REGS *regsin, union REGS *regsout);
int CTYP int86x(int intno, union REGS *regsin,
           union REGS *regsout, struct SREGS *sregs);

#ifdef __32BIT__
unsigned char inp(unsigned int port);
void outp(unsigned int port, unsigned char data);
#endif

#endif
