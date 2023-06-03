/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  multisc - multi-sector C compiler                                */
/*                                                                   */
/*  This is based on and designed to compile programs of the same    */
/*  language as Sector C, which was released under CC0 and can be    */
/*  obtained from here:                                              */
/*  https://github.com/xorvoid/sectorc                               */
/*                                                                   */
/*  It has been rewritten (from the original 8086 assembler) in C90  */
/*  so that it is understandable to someone who knows C90 rather     */
/*  than 8086 assembler.                                             */
/*                                                                   */
/*  It won't start life as self-compiling. This code will            */
/*  presumably need to be dumbed down to Sector C syntax at some     */
/*  point to provide the stepping stones.                            */
/*                                                                   */
/*  The program takes a source file in as input, and produces a COM  */
/*  file as output (ie suitable for running under MSDOS or a         */
/*  semi-clone like PDOS/86.                                         */
/*                                                                   */
/*  Although a COM file is normally tiny memory model and            */
/*  restricted to 64k, you could potentially make it small memory    */
/*  model instead with basically a very trivial form of              */
/*  self-relocation - simply loading ds with what is normally        */
/*  referred to as "DGROUP". Looks like this is already the case.    */
/*  Separate code and data. So that will break on some versions of   */
/*  Windows which have an artificial restriction of 64k for a COM    */
/*  file. But I won't be running under Windows anyway.               */
/*                                                                   */
/*********************************************************************/

#if 0

Here is a sample program:

int ret; /* must be this name */
int x;

void main()
{
    x = 5;
    ret = 7;
}

void _start()
{
    main();
}

Which produces:

C:\devel\pdos\util>hexdump samp.com
000000  908CDA81 C200108E DAE81800 B8034CA0  ..............L.
000010  3C38CD21 C3B80500 89069000 B8070089  <8.!............
000020  063C38C3 E8EEFFC3                    .<8.....

#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <ctype.h>


typedef struct {
    unsigned char magic[2]; /* "MZ" or "ZM". */ /* 0 */
    unsigned short num_last_page_bytes; /* 2 */
    unsigned short num_pages; /* 4 */
    unsigned short num_reloc_entries; /* 6 */
    unsigned short header_size; /* In paragraphs (16 byte). */ /* 8 */
    unsigned short min_alloc; /* 10 A */
    unsigned short max_alloc; /* 12 C */
    unsigned short init_ss; /* 14 E */
    unsigned short init_sp; /* 16 10 */
    unsigned short checksum; /* 18 12 */
    unsigned short init_ip; /* 20 14 */
    unsigned short init_cs; /* 22 16 */
    unsigned short reloc_tab_offset; /* 24 18 */
    unsigned short overlay; /* 26 1A */
    unsigned short reserved1[4]; /* First set of reserved words. */ /* 28 1C */
    unsigned short oem_id; /* 36 24 */
    unsigned short oem_info; /* 38 26 */
    unsigned short reserved2[10]; /* Second set of reserved words */ /* 40 28 */
    unsigned long e_lfanew; /* Offset to the PE header. */ /* 60 3C */
} Mz_hdr;


/********************************************************************/
/* Token values as computed by the tokenizer's                      */
/* atoi() calculation                                               */
/********************************************************************/

#define TOK_INT         6388
#define TOK_VOID        11386
#define TOK_ASM         5631
#define TOK_COMM        65532
#define TOK_SEMI        11
#define TOK_LPAREN      65528
#define TOK_RPAREN      65529
#define TOK_START       20697
#define TOK_DEREF       64653
#define TOK_WHILE_BEGIN 55810
#define TOK_IF_BEGIN    6232
#define TOK_BODY_BEGIN  5
#define TOK_BLK_BEGIN   75
#define TOK_BLK_END     77
#define TOK_ASSIGN      13
#define TOK_ADDR        65526
#define TOK_SUB         65533
#define TOK_ADD         65531
#define TOK_MUL         65530
#define TOK_AND         65526
#define TOK_OR          76
#define TOK_XOR         46
#define TOK_SHL         132
#define TOK_SHR         154
#define TOK_EQ          143
#define TOK_NE          65399
#define TOK_LT          12
#define TOK_GT          14
#define TOK_LE          133
#define TOK_GE          153

/* maximum number of bytes of code we can produce */
#ifdef TARGET_MVS
/* register usage: */
/* r7 = pointer to beginning of executable */
/* r13 = pointer to save area */
/* r6 = ax (I think this is called an "accumulator") */
/* r6 and all other registers can be trashed at any time */
/* note that this is in relation to the generated code, not
   this compiler code */

/* for MVS, include room for a 64k stack, and 64k for a little overhead */
/* and since we have a flat address space, allow for 256k of code */
/* plus another 256k for data */
#define MAXCODESZ (0x10000 + 0x10000 + 4 * 0x10000 + 4 * 0x10000)
#define DATASTART (6 * 0x10000)
#else
#define MAXCODESZ 0x10000
#endif

/* maximum number of symbols we support, and corresponding mask */
#define MAXSYM_MASK 0xffffU
#define MAXSYMS (MAXSYM_MASK + 1UL)

/********************************************************************/
/* Common variable uses                                             */
/********************************************************************/
/* ax: current token / scratch variable / emit val for stosw        */
/* bx: current token                                                */
/* cx: used by tok_next for trailing 2 bytes                        */
/* dl: flag for "tok_is_num"                                        */
/* dh: flags for "tok_is_call", trailing "()"                       */
/* bp: saved token for assigned variable                            */
/* sp: stack pointer, we don't mess with this                       */
/* si: used with lodsw for table scans                              */
/* ds: fn symbol table segment (occassionally set to "cs" to access */
/*     binary_oper_tbl)                                             */
/* di: codegen destination offset                                   */
/* es: codegen destination segment                                  */
/* cs: always 0x07c0                                                */
/********************************************************************/

static int ax;
static int bx;
static int cx;
static int dl;
static int dh;
static int bp;
static int si;
static int ds;
static int di;
static int es;

/* I'm not sure which bit of logic can't cope with large values */
/* so we squirrel that away */
static int bx_origval;

static int *symtbl;

static unsigned char *codegen_output_buffer;

static FILE *fp;
static FILE *fq;

static jmp_buf compile_jmpbuf;

static int skiptok = 0;

static int remember;

static int debug = 0;

/********************************************************************/
/* binary operator table                                            */
/********************************************************************/
static int binary_oper_tbl[] = {
   TOK_ADD, 0xc103,             /* add ax,cx */
   TOK_SUB, 0xc12b,             /* sub ax,cx */
   TOK_MUL, 0xe1f7,             /* mul ax,cx */
   TOK_AND, 0xc123,             /* and ax,cx */
   TOK_OR,  0xc10b,             /* or ax,cx  */
   TOK_XOR, 0xc133,             /* xor ax,cx */
   TOK_SHL, 0xe0d3,             /* shl ax,cx */
   TOK_SHR, 0xf8d3,             /* shr ax,cx */
   TOK_EQ,  0xc094,             /* sete al   */
   TOK_NE,  0xc095,             /* setne al  */
   TOK_LT,  0xc09c,             /* setl al   */
   TOK_GT,  0xc09f,             /* setg al   */
   TOK_LE,  0xc09e,             /* setle al  */
   TOK_GE,  0xc09d,             /* setge al  */
   0 };  /* [sentinel] */

static void compile_function(void);
static void compile_stmts_tok_next2(void);
static void control_flow_block(void);
static void compile_assign(void);
static void emit_common_ptr_op(void);
static void save_var_and_compile_expr(void);
static void compile_expr_tok_next(void);
static void compile_unary(void);
static void emit_var(void);
static void emit_tok(void);
static void tok_next2(void);
static void tok_next(void);
static void getch(void);
static void hashclash(void);
static int tasc(int loc);

int main(int argc, char **argv)
{
    if (argc <= 2)
    {
        printf("multisc [-d] <source> <COM>\n");
        printf("builds a 8086 COM file from source code in the\n");
        printf("subset of C90 originally defined by Sector C\n");
        return (EXIT_FAILURE);
    }

    symtbl = calloc(1, MAXSYMS * sizeof(int));
    /* initialize the codegen buffer as it is desirable for MVS
       where the stack is part of the executable */
    codegen_output_buffer = calloc(1, MAXCODESZ);

    if ((symtbl == NULL)
        || (codegen_output_buffer == NULL))
    {
        printf("insufficient memory\n");
        return (EXIT_FAILURE);
    }
    
    if (strcmp(argv[1], "-d") == 0)
    {
        debug = 1;
    }
    
    fp = fopen(argv[1+debug], "r");
    if (fp == NULL)
    {
        printf("failed to open %s for reading\n", argv[1+debug]);
        return (EXIT_FAILURE);
    }
    
    fq = fopen(argv[2+debug], "wb");
    if (fq == NULL)
    {
        printf("failed to open %s for writing\n", argv[2+debug]);
        return (EXIT_FAILURE);
    }

    /* reserve any names that clash with our tokens */
    symtbl[TOK_INT] = -1;
    symtbl[TOK_VOID] = -1;
    symtbl[TOK_ASM] = -1;
    symtbl[TOK_COMM] = -1;
    symtbl[TOK_SEMI] = -1;
    symtbl[TOK_LPAREN] = -1;
    symtbl[TOK_RPAREN] = -1;
    /* not this one - we need to allow it to be defined in the program */
    /* symtbl[TOK_START] = -1; */
    symtbl[TOK_DEREF] = -1;
    symtbl[TOK_WHILE_BEGIN] = -1;
    symtbl[TOK_IF_BEGIN] = -1;
    symtbl[TOK_BODY_BEGIN] = -1;
    symtbl[TOK_BLK_BEGIN] = -1;
    symtbl[TOK_BLK_END] = -1;
    symtbl[TOK_ASSIGN] = -1;
    symtbl[TOK_ADDR] = -1;
    symtbl[TOK_SUB] = -1;
    symtbl[TOK_ADD] = -1;
    symtbl[TOK_MUL] = -1;
    symtbl[TOK_AND] = -1;
    symtbl[TOK_OR] = -1;
    symtbl[TOK_XOR] = -1;
    symtbl[TOK_SHL] = -1;
    symtbl[TOK_SHR] = -1;
    symtbl[TOK_EQ] = -1;
    symtbl[TOK_NE] = -1;
    symtbl[TOK_LT] = -1;
    symtbl[TOK_GT] = -1;
    symtbl[TOK_LE] = -1;
    symtbl[TOK_GE] = -1;

    di = 0;    /* codegen index, zero'd */

#if TARGET_MVS

    /* give us some confidence it is working */
    codegen_output_buffer[di++] = 0x07; /* emit "nopr" instruction (bcr 0,0) */
    codegen_output_buffer[di++] = 0x00; /* for fun */

    /* establish location */
    codegen_output_buffer[di++] = 0x05; /* emit "balr r15,0" instruction */
    codegen_output_buffer[di++] = 0xf0; 

    /* subtract 4 without needing a constant */
    codegen_output_buffer[di++] = 0x06; /* emit "bctr r15,0" instruction */
    codegen_output_buffer[di++] = 0xf0; 

    codegen_output_buffer[di++] = 0x06; /* emit "bctr r15,0" instruction */
    codegen_output_buffer[di++] = 0xf0; 

    codegen_output_buffer[di++] = 0x06; /* emit "bctr r15,0" instruction */
    codegen_output_buffer[di++] = 0xf0; 

    codegen_output_buffer[di++] = 0x06; /* emit "bctr r15,0" instruction */
    codegen_output_buffer[di++] = 0xf0; 

    /* clean r15 for display purposes */
    codegen_output_buffer[di++] = 0x41; /* emit "la r15,0(r15)" instruction */
    codegen_output_buffer[di++] = 0xf0; /* target + source index */
    codegen_output_buffer[di++] = 0xf0; /* source base + offset */
    codegen_output_buffer[di++] = 0x00; /* rest of offset */

    /* save registers */
    codegen_output_buffer[di++] = 0x90; /* emit "stm r14,r12,12(r13)" */
    codegen_output_buffer[di++] = 0xec; 
    codegen_output_buffer[di++] = 0xd0; 
    codegen_output_buffer[di++] = 0x0c;     
    
    /* save original r13 in r9 */
    codegen_output_buffer[di++] = 0x18; /* emit "lr r9,r13" instruction */
    codegen_output_buffer[di++] = 0x9d; 

    /* r7 dedicated to load point */
    codegen_output_buffer[di++] = 0x18; /* emit "lr r7,r15" instruction */
    codegen_output_buffer[di++] = 0x7F; 

    /* we want a pc-relative jump, but don't have one (in s/370) unless you rely
       on being within 4095 bytes of the base. not a general solution.
       this is the next best (worst) thing. it's not that bad if you
       don't have any friends to lose anyway */
    /* oh - we also want to load a large immediate value, but don't have
       that either - again, s/370, not esa/390 or z/arch */
    codegen_output_buffer[di++] = 0x05; /* emit "balr r15,0" instruction */
    codegen_output_buffer[di++] = 0xf0; 

    /* remember = di; */
    codegen_output_buffer[di++] = 0x41; /* emit "la r15,10(r15)" instruction */
    codegen_output_buffer[di++] = 0xf0; /* target + source index */
    codegen_output_buffer[di++] = 0xf0; /* source base + offset */
    codegen_output_buffer[di++] = 0x0a; /* rest of offset */

    /* jump over constant, and r14 points to the constant */
    codegen_output_buffer[di++] = 0x05; /* emit "balr r14,r15" instruction */
    codegen_output_buffer[di++] = 0xef;

    /* 64k stack */
    codegen_output_buffer[di++] = 0x00;
    codegen_output_buffer[di++] = 0x01;
    codegen_output_buffer[di++] = 0x00;
    codegen_output_buffer[di++] = 0x00;

    /* di - remember */

    /* skip forward 64k from where we are now */
    /* remember = di; */
    codegen_output_buffer[di++] = 0x58; /* emit "l r2,0(,r14)" instruction */
    codegen_output_buffer[di++] = 0x20;
    codegen_output_buffer[di++] = 0xe0;
    codegen_output_buffer[di++] = 0x00;
    
    codegen_output_buffer[di++] = 0x1a; /* emit "ar r15,r2" instruction */
    codegen_output_buffer[di++] = 0xf2;

    /* r13 points to the register save area stack */
    codegen_output_buffer[di++] = 0x41; /* emit "la r13,18(,r14)" instruction */
    codegen_output_buffer[di++] = 0xd0;
    codegen_output_buffer[di++] = 0xe0;
    codegen_output_buffer[di++] = 0x12;

    /* branch over stack */
    codegen_output_buffer[di++] = 0x45; /* emit "bal r0,14(,r15)" instruction */
    codegen_output_buffer[di++] = 0x00;
    codegen_output_buffer[di++] = 0xf0;
    codegen_output_buffer[di++] = 0x0e;

    /* stack */
    di += 0x10000;
    
    /* di - remember */

    /* now we're ready to do a call to a particular (di) offset from r7 */

    codegen_output_buffer[di++] = 0x05; /* emit "balr r15,0" instruction */
    codegen_output_buffer[di++] = 0xf0; 

    codegen_output_buffer[di++] = 0x41; /* emit "la r15,10(r15)" instruction */
    codegen_output_buffer[di++] = 0xf0; /* target + source index */
    codegen_output_buffer[di++] = 0xf0; /* source base + offset */
    codegen_output_buffer[di++] = 0x0a; /* rest of offset */

    /* jump over constant, and r14 points to the constant */
    codegen_output_buffer[di++] = 0x05; /* emit "balr r14,r15" instruction */
    codegen_output_buffer[di++] = 0xef;

    remember = di;

    /* placeholder */
    codegen_output_buffer[di++] = 0x00;
    codegen_output_buffer[di++] = 0x00;
    codegen_output_buffer[di++] = 0x00;
    codegen_output_buffer[di++] = 0x00;

    codegen_output_buffer[di++] = 0x58; /* emit "l r15,0(,r14)" instruction */
    codegen_output_buffer[di++] = 0xf0;
    codegen_output_buffer[di++] = 0xe0;
    codegen_output_buffer[di++] = 0x00;
    
    codegen_output_buffer[di++] = 0x1a; /* emit "ar r15,r7" instruction */
    codegen_output_buffer[di++] = 0xf7;

    /* we now call the user's entry point */

    codegen_output_buffer[di++] = 0x05; /* emit "balr r14,r15" instruction */
    codegen_output_buffer[di++] = 0xef;

    /* restore original r13 */
    codegen_output_buffer[di++] = 0x18; /* emit "lr r13,r9" instruction */
    codegen_output_buffer[di++] = 0xd9; 

    /* load return address */
    codegen_output_buffer[di++] = 0x58; /* emit "l r14,12(,r13)" instruction */
    codegen_output_buffer[di++] = 0xe0;
    codegen_output_buffer[di++] = 0xd0;
    codegen_output_buffer[di++] = 0x0c;

    /* load registers */
    codegen_output_buffer[di++] = 0x98; /* emit "lm r0,r12,20(r13)" */
    codegen_output_buffer[di++] = 0x0c; 
    codegen_output_buffer[di++] = 0xd0; 
    codegen_output_buffer[di++] = 0x14;

    codegen_output_buffer[di++] = 0x07; /* emit "br r14" instruction */
    codegen_output_buffer[di++] = 0xfe; /* (bcr 15,14) */

#else
    codegen_output_buffer[di++] = 0x90; /* emit "nop" instruction for fun */
    
    /* adjust ds to the next 64k block */
    /* mov dx, ds */
    codegen_output_buffer[di++] = 0x8c;
    codegen_output_buffer[di++] = 0xda;
    /* add dx, 1000h */
    codegen_output_buffer[di++] = 0x81;
    codegen_output_buffer[di++] = 0xc2;
    codegen_output_buffer[di++] = 0x00;
    codegen_output_buffer[di++] = 0x10;
    /* mov ds, dx */
    codegen_output_buffer[di++] = 0x8e;
    codegen_output_buffer[di++] = 0xda;
    
    codegen_output_buffer[di++] = 0xe8; /* emit "call" instruction */
    codegen_output_buffer[di++] = 0x00;
    codegen_output_buffer[di++] = 0x00;
    remember = di;
    codegen_output_buffer[di++] = 0xb8; /* emit "mov ax,4c03h" instruction */
    codegen_output_buffer[di++] = 0x03;
    codegen_output_buffer[di++] = 0x4c;
    
    /* override return code with first variable defined */
    /* mov al,ds:[383ch] ret hashes to this */
    codegen_output_buffer[di++] = 0xa0;
    codegen_output_buffer[di++] = 0x3c;
    codegen_output_buffer[di++] = 0x38;
    
    codegen_output_buffer[di++] = 0xcd; /* emit "int 21h" instruction */
    codegen_output_buffer[di++] = 0x21;
    codegen_output_buffer[di++] = 0xc3; /* emit "ret" instruction for fun */
#endif

    while (setjmp(compile_jmpbuf) != 0)
    {
        /* we're here because of a longjmp */
        /* need to reinstate the setjmp */
        /* or is that really necessary? */
        /* the registers shouldn't have changed */
        /* since the last time, so maybe we would */
        /* just back here naturally? */
        /* so we would just need a setjmp and ignore */
        /* the return code */
    }

    /* main loop for parsing all decls */
    while (1)
    {
        /* advance to either "int" or "void" */
        tok_next();

        /* if "int" then skip a variable */
        if (ax != TOK_INT)
        {
            compile_function();
        }
        tok_next(); /* consume "int" */
        if (symtbl[(unsigned)bx] != 0)
        {
            hashclash();
        }
        symtbl[(unsigned)bx] = TOK_INT; /* this value won't be used,
                                           but we reserve it anyway */
        tok_next();  /* consume <ident> */
    }

    return (0);
}

/* parse and compile a function decl */
static void compile_function(void)
{
    int oldbx;

    tok_next(); /* consume "void" */
    oldbx = bx;   /* save function name token */
    if (symtbl[(unsigned)bx] != 0)
    {
        hashclash();
    }
    symtbl[(unsigned)bx] = di; /* record function address in symtbl */
#ifdef TARGET_MVS
    /* save registers */
    codegen_output_buffer[di++] = 0x90; /* emit "stm r14,r12,12(r13)" */
    codegen_output_buffer[di++] = 0xec; 
    codegen_output_buffer[di++] = 0xd0; 
    codegen_output_buffer[di++] = 0x0c;    

    /* save original r13 in r9 */
    codegen_output_buffer[di++] = 0x18; /* emit "lr r9,r13" instruction */
    codegen_output_buffer[di++] = 0x9d; 

    /* bump r13 up for the next guy */
    codegen_output_buffer[di++] = 0x41; /* emit "la r13,72(,r13)" instruction */
    codegen_output_buffer[di++] = 0xd0;
    codegen_output_buffer[di++] = 0xd0;
    codegen_output_buffer[di++] = 0x48;

    /* store old r13 (ie backchain) */
    codegen_output_buffer[di++] = 0x50; /* emit "st r9,4(,r13)" instruction */
    codegen_output_buffer[di++] = 0x90;
    codegen_output_buffer[di++] = 0xd0;
    codegen_output_buffer[di++] = 0x04;
    
    /* store new r13 (ie forward chain) */
    codegen_output_buffer[di++] = 0x50; /* emit "st r13,8(,r9)" instruction */
    codegen_output_buffer[di++] = 0xd0;
    codegen_output_buffer[di++] = 0x90;
    codegen_output_buffer[di++] = 0x08;
    
#endif

    compile_stmts_tok_next2(); /* compile function body */

#if TARGET_MVS
    /* load old r13 */
    codegen_output_buffer[di++] = 0x58; /* emit "l r13,4(,r13)" instruction */
    codegen_output_buffer[di++] = 0xd0;
    codegen_output_buffer[di++] = 0xd0;
    codegen_output_buffer[di++] = 0x04;

    /* load registers */
    codegen_output_buffer[di++] = 0x98; /* emit "lm r14,r12,12(r13)" */
    codegen_output_buffer[di++] = 0xec;
    codegen_output_buffer[di++] = 0xd0; 
    codegen_output_buffer[di++] = 0x0c;

    codegen_output_buffer[di++] = 0x07; /* emit "br r14" instruction */
    codegen_output_buffer[di++] = 0xfe; /* (bcr 15,14) */
#else
    codegen_output_buffer[di++] = 0xc3; /* emit "ret" instruction */
#endif

    bx = oldbx;

    /* if the function is _start(), we're done */
    /* otherwise, loop and compile another declaration */
    if (bx != TOK_START)
    {
        longjmp(compile_jmpbuf, 0); /* 0 return code will be changed */
    }

    /* done compiling, save the binary */
    /* ok, so i need some instructions at the beginning to
       convert cs:ip into having an ip of 0, get cs into ds,
       add 1000h to ds, and then do a jmp to the value we
       currently have in ds:bx i guess which is presumably
       one of those two tables? */
    /* also the stack pointer will point to the end of the
       64k code section when loaded by PDOS/86. And PDOS/86
       will need to be changed to be able to load a COM
       file up to 128k in size (or even bigger for people
       with more sophisticated self-relocation capability). */
    /* Change of plan - we'll write an MZ executable
       header instead. And the startup code will need to
       load ds with DGROUP and provide a relocation entry
       for that. */
    /* Another change of plan - I suspect that you can't have
       initialized variables. So we'll just mark the minalloc
       and maxalloc appropriately in the header. And the
       _start() code can have code to set ds to 64k above what
       it received from MSDOS (ie set to the PSP) and a call to
       INT 21H to terminate. */

#if TARGET_MVS
    {
        int ret;
        int (*foo)(void);

        ax = symtbl[(unsigned)bx];
        codegen_output_buffer[(unsigned)(remember)] = (ax >> 24) & 0xff;
        codegen_output_buffer[(unsigned)(remember + 1)] = (ax >> 16) & 0xff;
        codegen_output_buffer[(unsigned)(remember + 2)] = (ax >> 8) & 0xff;
        codegen_output_buffer[(unsigned)(remember + 3)] = ax & 0xff;
        foo = (int (*)(void))codegen_output_buffer;        
#if GO
        foo();
        ret = *(int *)(codegen_output_buffer + DATASTART + 0x7078);
        printf("return from called program is %d %x\n", ret, ret);
#else
        fwrite(codegen_output_buffer, 1, di, fq);
        fclose(fq);
#endif
        exit(EXIT_SUCCESS);
    }
#endif

#if 0
execute:
  push es                       ; push the codegen segment
  push word [bx]                ; push the offset to "_start()"
  push 0x4000                   ; load new segment for variable data
  pop ds
  retf                          ; jump into it via "retf"
#endif

    ax = symtbl[(unsigned)bx] - remember;
    codegen_output_buffer[(unsigned)(remember - 2)] = ax & 0xff;
    codegen_output_buffer[(unsigned)(remember - 1)] = (ax >> 8) & 0xff;

/*    codegen_output_buffer[1] = 0x90;
    codegen_output_buffer[2] = 0x90;
    codegen_output_buffer[3] = 0x90; */

    fwrite(codegen_output_buffer, 1, di, fq);
    exit(EXIT_SUCCESS);

    return;
}


/********************************************************************/
/* compile statements (optionally advancing tokens beforehand)      */
/********************************************************************/
static void compile_stmts_tok_next2(void)
{
    int tmp;

    while (1)
    {
        if (skiptok)
        {
            skiptok--;
        }
        else
        {
            tok_next();
        }
        /* compile_stmts_tok_next: */

        /* same test, repeated */
        if (skiptok)
        {
            skiptok--;
        }
        else
        {
            tok_next();
        }
        /* compile_stmts: */
        ax = bx;
        /* if we reach '}' then return */
        if (ax == TOK_BLK_END)
        {
            return;
        }

        /* if dh is 0, it's not a call */
        /* otherwise it is a call, so ... */
        if (dh != 0)
        {
#if TARGET_MVS
    /* now we're ready to do a call to a particular (di) offset from r7 */

    codegen_output_buffer[di++] = 0x05; /* emit "balr r15,0" instruction */
    codegen_output_buffer[di++] = 0xf0; 

    codegen_output_buffer[di++] = 0x41; /* emit "la r15,10(r15)" instruction */
    codegen_output_buffer[di++] = 0xf0; /* target + source index */
    codegen_output_buffer[di++] = 0xf0; /* source base + offset */
    codegen_output_buffer[di++] = 0x0a; /* rest of offset */

    /* jump over constant, and r14 points to the constant */
    codegen_output_buffer[di++] = 0x05; /* emit "balr r14,r15" instruction */
    codegen_output_buffer[di++] = 0xef;

    /* load function offset (from r7) from symbol-table */
    ax = symtbl[(unsigned)bx];

    /* write out the constant */
    codegen_output_buffer[di++] = (ax >> 24) & 0xff;
    codegen_output_buffer[di++] = (ax >> 16) & 0xff;
    codegen_output_buffer[di++] = (ax >> 8) & 0xff;
    codegen_output_buffer[di++] = ax & 0xff;

    codegen_output_buffer[di++] = 0x58; /* emit "l r15,0(,r14)" instruction */
    codegen_output_buffer[di++] = 0xf0;
    codegen_output_buffer[di++] = 0xe0;
    codegen_output_buffer[di++] = 0x00;
    
    codegen_output_buffer[di++] = 0x1a; /* emit "ar r15,r7" instruction */
    codegen_output_buffer[di++] = 0xf7;

    /* we now call the user's entry point */

    codegen_output_buffer[di++] = 0x05; /* emit "balr r14,r15" instruction */
    codegen_output_buffer[di++] = 0xef;

#else
            /* emit "call" instruction */
            codegen_output_buffer[di++] = 0xe8;
            /* load function offset from symbol-table */
            ax = symtbl[(unsigned)bx];
            /* compute relative to this location: "dest - cur - 2" */
            /* note that although a "call" is a 3 byte instruction
               (in this case), we have already incremented di when
               we output the "call" opcode, so that is why we subtract
               2 instead of 3. Also note that it is a relative offset
               that is branched to with the call, and that it is
               relative to the end of the call statement itself, not
               the beginning. */
            ax -= di;
            ax -= 2;
            /* emit target */
            codegen_output_buffer[di++] = ax & 0xff;
            codegen_output_buffer[di++] = (ax >> 8) & 0xff;
#endif

            continue; /* loop to compile next statement */
        }
        
        /* not a call, so next thing to check is ... */

        /* check for "asm" */
        if (ax == TOK_ASM)
        {
            tok_next(); /* tok_next to get literal byte */
            codegen_output_buffer[di++] = ax & 0xff;
            continue; /* loop to compile next statement */
        }

        /* check for "if" */
        if (ax == TOK_IF_BEGIN)
        {
            control_flow_block(); /* compile control-flow block */
            
            /* this code, patch_fwd, is duplicated, so maybe a macro
               or something else might be better. Or it may be
               somewhat coincidental and needs a separate life */

            /* compute relative fwd jump to this location: "dest - src" */
            ax = di - si;
            codegen_output_buffer[si-2] = ax & 0xff;
            codegen_output_buffer[si-2+1] = (ax >> 8) & 0xff;
            skiptok = 1;
            continue;
        }

        /* check for "while" */
        if (ax == TOK_WHILE_BEGIN)
        {
            tmp = di; /* save loop start location */
            control_flow_block(); /* compile control-flow block */
#if TARGET_MVS
            /* ok, so we want to jmp to tmp, offset from r7 */

    codegen_output_buffer[di++] = 0x05; /* emit "balr r15,0" instruction */
    codegen_output_buffer[di++] = 0xf0; 

    codegen_output_buffer[di++] = 0x41; /* emit "la r15,10(r15)" instruction */
    codegen_output_buffer[di++] = 0xf0; /* target + source index */
    codegen_output_buffer[di++] = 0xf0; /* source base + offset */
    codegen_output_buffer[di++] = 0x0a; /* rest of offset */

    /* jump over constant, and r14 points to the constant */
    codegen_output_buffer[di++] = 0x05; /* emit "balr r14,r15" instruction */
    codegen_output_buffer[di++] = 0xef;

    /* load function offset (from r7) from symbol-table */
    ax = tmp;

    /* write out the constant */
    codegen_output_buffer[di++] = (ax >> 24) & 0xff;
    codegen_output_buffer[di++] = (ax >> 16) & 0xff;
    codegen_output_buffer[di++] = (ax >> 8) & 0xff;
    codegen_output_buffer[di++] = ax & 0xff;

    /* emit "l r15,0(,r14)" instruction */
        codegen_output_buffer[di++] = 0x58; /* emit "l r15,0(,r14)" inst */
        codegen_output_buffer[di++] = 0xf0;
        codegen_output_buffer[di++] = 0xe0;
        codegen_output_buffer[di++] = 0x00;

    /* ar r15,r7 */
        codegen_output_buffer[di++] = 0x1a; /* emit "ar r15,r7" inst */
        codegen_output_buffer[di++] = 0xf7;
        
    /* emit "br r15" instruction */
    codegen_output_buffer[di++] = 0x07; /* emit "br r15" instruction */
    codegen_output_buffer[di++] = 0xff; /* (bcr 15,15) */
    

#else
            /* emit "jmp" instruction (backwards) */
            codegen_output_buffer[di++] = 0xe9;

            /* compute relative to this location: "dest - cur - 2" */
            /* dest is the start of the while loop, which we saved in tmp */
            /* current location is kept in di */
            /* the 2 is for the 2 bytes that we are just about to write */
            ax = tmp - di - 2;
            codegen_output_buffer[di++] = ax & 0xff;
            codegen_output_buffer[di++] = (ax >> 8) & 0xff;
#endif

#if TARGET_MVS
            ax = di;
            codegen_output_buffer[si] = (ax >> 24) & 0xff;
            codegen_output_buffer[si+1] = (ax >> 16) & 0xff;
            codegen_output_buffer[si+2] = (ax >> 8) & 0xff;
            codegen_output_buffer[si+3] = ax & 0xff;
#else
            /* now when the while loop terminates, we need to get to
               right here. We have apparently already saved the location
               after the (presumed) conditional jump in si. So the
               distance for that conditional jump is a simple di - si.
               But the bytes that need to be zapped will apparently be
               at the end of that conditional jump, so the zap location
               needs to have 2 subtracted from it so that we can zap the
               middle of the conditional jump. Honestly, it is amazing
               how brilliant this stuff (computers) is. There are some
               simple primitives for solving a Rubik's cube too I think.
               That is pointless. Just like solving crosswords. This isn't. */
            /* compute relative fwd jump to this location: "dest - src" */
            ax = di - si;
            codegen_output_buffer[si-2] = ax & 0xff;
            codegen_output_buffer[si-2+1] = (ax >> 8) & 0xff;
#endif

            skiptok = 1;
            continue;
        }

        compile_assign(); /* handle an assignment statement */
        /* loop to compile next statement */
        skiptok = 2;
        continue;
    }
    return;
}

static void control_flow_block(void)
{
    int tmp;

    compile_expr_tok_next();  /* compile loop or if condition expr */

    /* emit forward jump */

#if TARGET_MVS
    codegen_output_buffer[di++] = 0x05; /* emit "balr r15,0" instruction */
    codegen_output_buffer[di++] = 0xf0;
    
/* base */
    
    /* emit ltr r6,r6 */
    codegen_output_buffer[di++] = 0x12;
    codegen_output_buffer[di++] = 0x66;
    
    /* ok, so we want to do a jump to some location further
       down, which will be relative to r7. we don't have an
       instruction to do that, so we do it the hard way */
    /* bnz some_relative_value */

    /* bz bypass */

    /* it seems it is the other way around. we need bnz */

    /* emit bnz bypass */
    codegen_output_buffer[di++] = 0x47;
    codegen_output_buffer[di++] = 0x70;
    codegen_output_buffer[di++] = 0xf0;
    codegen_output_buffer[di++] = 0x16; /* 22 from base to bypass */

    /* emit bal r14, 14(r15) */
    /* branch over jump location */
    codegen_output_buffer[di++] = 0x45; /* emit "bal r14,14(,r15)" instruction */
    codegen_output_buffer[di++] = 0xe0;
    codegen_output_buffer[di++] = 0xf0;
    codegen_output_buffer[di++] = 0x0e;

    tmp = di; /* save forward patch location */
              /* not sure if we want it here, or after these NULs */
    codegen_output_buffer[di++] = 0x00;
    codegen_output_buffer[di++] = 0x00;
    codegen_output_buffer[di++] = 0x00;
    codegen_output_buffer[di++] = 0x00;

/* we no longer need r15 as a base register, so we can clobber it */

    codegen_output_buffer[di++] = 0x58; /* emit "l r15,0(,r14)" instruction */
    codegen_output_buffer[di++] = 0xf0;
    codegen_output_buffer[di++] = 0xe0;
    codegen_output_buffer[di++] = 0x00;
    
    codegen_output_buffer[di++] = 0x1a; /* emit "ar r15,r7" instruction */
    codegen_output_buffer[di++] = 0xf7;

    /* br r15 */
    codegen_output_buffer[di++] = 0x07;
    codegen_output_buffer[di++] = 0xff; /* i think first f means
                                           unconditional */

/* bypass */


#else
    /* emit "test ax,ax" */
    codegen_output_buffer[di++] = 0x85;
    codegen_output_buffer[di++] = 0xc0;
    
    /* emit "je" instruction */
    codegen_output_buffer[di++] = 0x0f;
    codegen_output_buffer[di++] = 0x84;

    /* emit placeholder for target */
    codegen_output_buffer[di++] = 0x00;
    codegen_output_buffer[di++] = 0x00;
    tmp = di; /* save forward patch location */
#endif

    /* ok, so basically what we have done is if you have
       if (something) { do this } then "something" has been
       evaluated to false, with 0 in ax, so in that situation,
       ie ax == 0, you need to skip over a bunch of code.
       So we've emitted the jmp already, and di is pointing
       to the location after that jmp. We normally want si
       pointing to the old location, but it will be clobbered
       by potential recursion, so we save di into a temporary
       variable and only set si when the recursion has finished */

    skiptok = 1;
    compile_stmts_tok_next2();   /* compile a block of statements */
    si = tmp; /* restore forward patch location */

    return;
}


/********************************************************************/
/* compile assignment statement                                     */
/********************************************************************/
static void compile_assign(void)
{
    int tmp;

    /* check for "*(int*)" */
    if (ax == TOK_DEREF)
    {
        tok_next(); /* consume "*(int*)" */
        save_var_and_compile_expr(); /* compile rhs first */

        /* compile_store_deref: */
        bx = bp; /* restore dest var token */
        ax = 0x0489; /* code for "mov [si],ax" */

        emit_common_ptr_op();
        return;
    }

    /* _not_deref_store: */
    save_var_and_compile_expr(); /* compile rhs first */

    /* compile_store: */
    bx = bp;   /* restore dest var token */
#ifdef TARGET_MVS
    codegen_output_buffer[di++] = 0x05; /* emit "balr r15,0" instruction */
    codegen_output_buffer[di++] = 0xf0; 

    codegen_output_buffer[di++] = 0x41; /* emit "la r15,10(r15)" instruction */
    codegen_output_buffer[di++] = 0xf0; /* target + source index */
    codegen_output_buffer[di++] = 0xf0; /* source base + offset */
    codegen_output_buffer[di++] = 0x0a; /* rest of offset */

    /* jump over constant, and r14 points to the constant */
    codegen_output_buffer[di++] = 0x05; /* emit "balr r14,r15" instruction */
    codegen_output_buffer[di++] = 0xef;

    bx *= 4; /* there are 65536 possible hash values, each occupying 4 bytes */
    bx += DATASTART; /* and the data space starts after the code */
    emit_tok();

    codegen_output_buffer[di++] = 0x58; /* emit "l r11,0(,r14)" instruction */
    codegen_output_buffer[di++] = 0xb0;
    codegen_output_buffer[di++] = 0xe0;
    codegen_output_buffer[di++] = 0x00;
    
    codegen_output_buffer[di++] = 0x1a; /* emit "ar r11,r7" instruction */
    codegen_output_buffer[di++] = 0xb7;

    codegen_output_buffer[di++] = 0x50; /* emit "st r6,0(,r11)" instruction */
    codegen_output_buffer[di++] = 0x60;
    codegen_output_buffer[di++] = 0xb0;
    codegen_output_buffer[di++] = 0x00;
    
#else
    ax = 0x0689;   /* code for "mov [imm],ax" */
    emit_var();
#endif
    return;
}


static void emit_common_ptr_op(void)
{
    int tmp;

    tmp = ax;
    ax = 0x368b; /* emit "mov si,[imm]" */
    emit_var();
        
    ax = tmp;
    codegen_output_buffer[di++] = ax & 0xff;
    codegen_output_buffer[di++] = (ax >> 8) & 0xff;

    return;
}

static void save_var_and_compile_expr(void)
{
    bp = bx;  /* save dest to bp */
    tok_next();  /* consume dest */

    /* this call will consume "=" before compiling expr */
    compile_expr_tok_next();
    return;
}


/********************************************************************/
/* compile expression                                               */
/********************************************************************/
static void compile_expr_tok_next(void)
{
    int *ptr; /* there should be a better name than this */
    int tmp;

    tok_next();

    compile_unary();   /* compile left-hand side */

    /* load ptr to operator table (biased backwards) */
    ptr = binary_oper_tbl;
    ptr--;

    /*_check_next:*/
    while (1)
    {
        ptr++;  /* discard 16-bit of machine-code */
        ax = *ptr++; /* load 16-bit token value */
        
        /* matches token? */
        if (ax == bx)
        {
            break;
        }
        
        /* end of table? */
        if (ax == 0)
        {
            /* all-done, not found */
            return;
        }
    }

#if TARGET_MVS
    tmp = ax; /* use just token on MVS */
#else
    tmp = *ptr;  /* load 16-bit of machine-code */
#endif

#if TARGET_MVS
    /* we could reserve some space on the stack for internal
       compiler use, but we have enough registers that we
       don't need to */
    /* save original r6 in r2 */
    /* note that I avoid 8 becaue of my poor eyesight
       when looking at dumps */
    codegen_output_buffer[di++] = 0x18; /* emit "lr r2,r6" instruction */
    codegen_output_buffer[di++] = 0x26;
#else
    codegen_output_buffer[di++] = 0x50; /* code for "push ax" */
#endif
    tok_next();      /* consume operator token */
    compile_unary(); /* compile right-hand side */

#if TARGET_MVS
    /* r3 is used for cx */
    codegen_output_buffer[di++] = 0x18; /* emit "lr r3,r6" instruction */
    codegen_output_buffer[di++] = 0x36;

    /* get r6 restored */
    codegen_output_buffer[di++] = 0x18; /* emit "lr r6,r2" instruction */
    codegen_output_buffer[di++] = 0x62;

/* ok, so the ax/R6 in the program that is being generated is set.
   the cx/R3 in the program that is being generated is set.
   these variables are unrelated to the use of ax here in the
   compiler itself */

#else
    codegen_output_buffer[di++] = 0x59; /* code for "pop cx" */
    codegen_output_buffer[di++] = 0x91; /* code for "xchg ax,cx" */
#endif

#if TARGET_MVS
    if (tmp == TOK_ADD)
    {
        codegen_output_buffer[di++] = 0x1a; /* emit "ar r6,r3" instruction */
        codegen_output_buffer[di++] = 0x63;
    }
    else if (tmp == TOK_SUB)
    {
        codegen_output_buffer[di++] = 0x1b; /* emit "sr r6,r3" instruction */
        codegen_output_buffer[di++] = 0x63;
    }
    else if (tmp == TOK_AND)
    {
        codegen_output_buffer[di++] = 0x14; /* emit "nr r6,r3" instruction */
        codegen_output_buffer[di++] = 0x63;
    }
    else if (tmp == TOK_XOR)
    {
        codegen_output_buffer[di++] = 0x17; /* emit "xr r6,r3" instruction */
        codegen_output_buffer[di++] = 0x63;
    }
    else if (tmp == TOK_MUL)
    {
        /* from memory, multiply uses a register pair, starting with an
           even number */
        /* so we will be using r4 + r5 */
        /* and the result goes into r5 i believe */
        
        /* move something from r6 to r5 */
        codegen_output_buffer[di++] = 0x18; /* emit "lr r3,r6" instruction */
        codegen_output_buffer[di++] = 0x56; 
        
        codegen_output_buffer[di++] = 0x41; /* emit "la r4,0(0,0)" instruction */
        codegen_output_buffer[di++] = 0x40; /* target + source index */
        codegen_output_buffer[di++] = 0x00; /* source base + offset */
        codegen_output_buffer[di++] = 0x00; /* rest of offset */

        codegen_output_buffer[di++] = 0x1c; /* emit "mr r4,r3" instruction */
        codegen_output_buffer[di++] = 0x43;

        /* move result from r5 to r6 */
        codegen_output_buffer[di++] = 0x18; /* emit "lr r6,r5" instruction */
        codegen_output_buffer[di++] = 0x65; 
    }
    else if (tmp == TOK_EQ)
    {
        codegen_output_buffer[di++] = 0x05; /* emit "balr r15,0" instruction */
        codegen_output_buffer[di++] = 0xf0; 

/* base: */

        codegen_output_buffer[di++] = 0x19; /* emit "cr r6,r3" instruction */
        codegen_output_buffer[di++] = 0x63; 

        /* if it is equal to, then go and set the 1 */
        codegen_output_buffer[di++] = 0x47; /* emit "be 14(,r15)" instruction */
        codegen_output_buffer[di++] = 0x80;
        codegen_output_buffer[di++] = 0xf0; 
        codegen_output_buffer[di++] = 0x0e; 

        /* not less than, so set to 0 */
        codegen_output_buffer[di++] = 0x41; /* emit "la r6,0(0,0)" instruction */
        codegen_output_buffer[di++] = 0x60; /* target + source index */
        codegen_output_buffer[di++] = 0x00; /* source base + offset */
        codegen_output_buffer[di++] = 0x00; /* rest of offset */

        /* should this have a comma? ie is that an index register? */
        codegen_output_buffer[di++] = 0x47; /* emit "b 18(,r15)" instruction */
        codegen_output_buffer[di++] = 0xf0; 
        codegen_output_buffer[di++] = 0xf0; 
        codegen_output_buffer[di++] = 0x12;

        /* less than, so set to 1 */
        codegen_output_buffer[di++] = 0x41; /* emit "la r6,0(0,1)" instruction */
        codegen_output_buffer[di++] = 0x60; /* target + source index */
        codegen_output_buffer[di++] = 0x00; /* source base + offset */
        codegen_output_buffer[di++] = 0x01; /* rest of offset */
    }
    else if (tmp == TOK_NE)
    {
        codegen_output_buffer[di++] = 0x05; /* emit "balr r15,0" instruction */
        codegen_output_buffer[di++] = 0xf0; 

/* base: */

        codegen_output_buffer[di++] = 0x19; /* emit "cr r6,r3" instruction */
        codegen_output_buffer[di++] = 0x63; 

        /* if it is not equal to, then go and set the 1 */
        codegen_output_buffer[di++] = 0x47; /* emit "bne 14(,r15)" instr */
        codegen_output_buffer[di++] = 0x70;
        codegen_output_buffer[di++] = 0xf0; 
        codegen_output_buffer[di++] = 0x0e; 

        /* not less than, so set to 0 */
        codegen_output_buffer[di++] = 0x41; /* emit "la r6,0(0,0)" instruction */
        codegen_output_buffer[di++] = 0x60; /* target + source index */
        codegen_output_buffer[di++] = 0x00; /* source base + offset */
        codegen_output_buffer[di++] = 0x00; /* rest of offset */

        /* should this have a comma? ie is that an index register? */
        codegen_output_buffer[di++] = 0x47; /* emit "b 18(,r15)" instruction */
        codegen_output_buffer[di++] = 0xf0; 
        codegen_output_buffer[di++] = 0xf0; 
        codegen_output_buffer[di++] = 0x12;

        /* less than, so set to 1 */
        codegen_output_buffer[di++] = 0x41; /* emit "la r6,0(0,1)" instruction */
        codegen_output_buffer[di++] = 0x60; /* target + source index */
        codegen_output_buffer[di++] = 0x00; /* source base + offset */
        codegen_output_buffer[di++] = 0x01; /* rest of offset */
    }
    /* the goal is to set ax (r6) = 1 if ax (r6) < cx (r3) */
    /* we do that in the most unsophisticated way possible */
    else if (tmp == TOK_LT)
    {
        codegen_output_buffer[di++] = 0x05; /* emit "balr r15,0" instruction */
        codegen_output_buffer[di++] = 0xf0; 

/* base: */

        codegen_output_buffer[di++] = 0x19; /* emit "cr r6,r3" instruction */
        codegen_output_buffer[di++] = 0x63; 

        /* if it is less than, then go and set the 1 */
        codegen_output_buffer[di++] = 0x47; /* emit "bl 14(,r15)" instruction */
        codegen_output_buffer[di++] = 0x40; 
        codegen_output_buffer[di++] = 0xf0; 
        codegen_output_buffer[di++] = 0x0e; 

        /* not less than, so set to 0 */
        codegen_output_buffer[di++] = 0x41; /* emit "la r6,0(0,0)" instruction */
        codegen_output_buffer[di++] = 0x60; /* target + source index */
        codegen_output_buffer[di++] = 0x00; /* source base + offset */
        codegen_output_buffer[di++] = 0x00; /* rest of offset */

        /* should this have a comma? ie is that an index register? */
        codegen_output_buffer[di++] = 0x47; /* emit "b 18(,r15)" instruction */
        codegen_output_buffer[di++] = 0xf0; 
        codegen_output_buffer[di++] = 0xf0; 
        codegen_output_buffer[di++] = 0x12;

        /* less than, so set to 1 */
        codegen_output_buffer[di++] = 0x41; /* emit "la r6,0(0,1)" instruction */
        codegen_output_buffer[di++] = 0x60; /* target + source index */
        codegen_output_buffer[di++] = 0x00; /* source base + offset */
        codegen_output_buffer[di++] = 0x01; /* rest of offset */
    }
    else if (tmp == TOK_LE)
    {
        codegen_output_buffer[di++] = 0x05; /* emit "balr r15,0" instruction */
        codegen_output_buffer[di++] = 0xf0; 

/* base: */

        codegen_output_buffer[di++] = 0x19; /* emit "cr r6,r3" instruction */
        codegen_output_buffer[di++] = 0x63; 

        /* if it is less than, then go and set the 1 */
        codegen_output_buffer[di++] = 0x47; /* emit "bnh 14(,r15)" instruction */
        codegen_output_buffer[di++] = 0xd0;
        codegen_output_buffer[di++] = 0xf0; 
        codegen_output_buffer[di++] = 0x0e; 

        /* not less than, so set to 0 */
        codegen_output_buffer[di++] = 0x41; /* emit "la r6,0(0,0)" instruction */
        codegen_output_buffer[di++] = 0x60; /* target + source index */
        codegen_output_buffer[di++] = 0x00; /* source base + offset */
        codegen_output_buffer[di++] = 0x00; /* rest of offset */

        /* should this have a comma? ie is that an index register? */
        codegen_output_buffer[di++] = 0x47; /* emit "b 18(,r15)" instruction */
        codegen_output_buffer[di++] = 0xf0; 
        codegen_output_buffer[di++] = 0xf0; 
        codegen_output_buffer[di++] = 0x12;

        /* less than, so set to 1 */
        codegen_output_buffer[di++] = 0x41; /* emit "la r6,0(0,1)" instruction */
        codegen_output_buffer[di++] = 0x60; /* target + source index */
        codegen_output_buffer[di++] = 0x00; /* source base + offset */
        codegen_output_buffer[di++] = 0x01; /* rest of offset */
    }
    else if (tmp == TOK_GT)
    {
        codegen_output_buffer[di++] = 0x05; /* emit "balr r15,0" instruction */
        codegen_output_buffer[di++] = 0xf0; 

/* base: */

        codegen_output_buffer[di++] = 0x19; /* emit "cr r6,r3" instruction */
        codegen_output_buffer[di++] = 0x63; 

        /* if it is greater than, then go and set the 1 */
        codegen_output_buffer[di++] = 0x47; /* emit "bh 14(,r15)" instruction */
        codegen_output_buffer[di++] = 0x20;
        codegen_output_buffer[di++] = 0xf0; 
        codegen_output_buffer[di++] = 0x0e; 

        /* not less than, so set to 0 */
        codegen_output_buffer[di++] = 0x41; /* emit "la r6,0(0,0)" instruction */
        codegen_output_buffer[di++] = 0x60; /* target + source index */
        codegen_output_buffer[di++] = 0x00; /* source base + offset */
        codegen_output_buffer[di++] = 0x00; /* rest of offset */

        /* should this have a comma? ie is that an index register? */
        codegen_output_buffer[di++] = 0x47; /* emit "b 18(,r15)" instruction */
        codegen_output_buffer[di++] = 0xf0; 
        codegen_output_buffer[di++] = 0xf0; 
        codegen_output_buffer[di++] = 0x12;

        /* less than, so set to 1 */
        codegen_output_buffer[di++] = 0x41; /* emit "la r6,0(0,1)" instruction */
        codegen_output_buffer[di++] = 0x60; /* target + source index */
        codegen_output_buffer[di++] = 0x00; /* source base + offset */
        codegen_output_buffer[di++] = 0x01; /* rest of offset */
    }
    else if (tmp == TOK_GE)
    {
        codegen_output_buffer[di++] = 0x05; /* emit "balr r15,0" instruction */
        codegen_output_buffer[di++] = 0xf0; 

/* base: */

        codegen_output_buffer[di++] = 0x19; /* emit "cr r6,r3" instruction */
        codegen_output_buffer[di++] = 0x63; 

        /* if it is greater than or equal to, then go and set the 1 */
        codegen_output_buffer[di++] = 0x47; /* emit "bnl 14(,r15)" instruction */
        codegen_output_buffer[di++] = 0xb0;
        codegen_output_buffer[di++] = 0xf0; 
        codegen_output_buffer[di++] = 0x0e; 

        /* not less than, so set to 0 */
        codegen_output_buffer[di++] = 0x41; /* emit "la r6,0(0,0)" instruction */
        codegen_output_buffer[di++] = 0x60; /* target + source index */
        codegen_output_buffer[di++] = 0x00; /* source base + offset */
        codegen_output_buffer[di++] = 0x00; /* rest of offset */

        /* should this have a comma? ie is that an index register? */
        codegen_output_buffer[di++] = 0x47; /* emit "b 18(,r15)" instruction */
        codegen_output_buffer[di++] = 0xf0; 
        codegen_output_buffer[di++] = 0xf0; 
        codegen_output_buffer[di++] = 0x12;

        /* less than, so set to 1 */
        codegen_output_buffer[di++] = 0x41; /* emit "la r6,0(0,1)" instruction */
        codegen_output_buffer[di++] = 0x60; /* target + source index */
        codegen_output_buffer[di++] = 0x00; /* source base + offset */
        codegen_output_buffer[di++] = 0x01; /* rest of offset */
    }
    else
    {
        printf("uncoded operation %d\n", tmp);
        exit(EXIT_FAILURE);
    }
#else
    /* we could just use *ptr instead of tmp */
    bx = tmp; /* restore 16-bit of machine-code */
 
    /* detect the special case for comparison ops */
    if ((bx & 0xff00) == 0xc000)
    {
        /* code for "cmp ax,cx" */
        codegen_output_buffer[di++] = 0x39;
        codegen_output_buffer[di++] = 0xc8;

        /* code for "mov ax,0x00" */
        codegen_output_buffer[di++] = 0xb8;
        codegen_output_buffer[di++] = 0x00;

        /* code for the rest of imm and prefix for "setX" instrs */
        codegen_output_buffer[di++] = 0x00; /* ie this is part of the b8 inst */
        codegen_output_buffer[di++] = 0x0f;
    }

    ax = bx;
    /* emit machine code for op */
    codegen_output_buffer[di++] = ax & 0xff;
    codegen_output_buffer[di++] = (ax >> 8) & 0xff;
#endif
    return;
}


/********************************************************************/
/* compile unary                                                    */
/********************************************************************/
static void compile_unary(void)
{
    /* check for "*(int*)" */
    if (ax == TOK_DEREF)
    {
        /* compile deref (load) */
        tok_next();  /* consume "*(int*)" */
        ax = 0x048b; /* code for "mov ax,[si]" */
        emit_common_ptr_op();
        return;
    }

    /* check for "*(int*)" */
    if (ax == TOK_LPAREN)
    {
        compile_expr_tok_next();    /* consume "(" and compile expr */
        tok_next();                 /* consume ")" */
        return;
    }
    
    /* check for "&" */
    if (ax == TOK_ADDR)
    {
        tok_next();  /* consume "&" */
        ax = 0x068d;   /* code for "lea ax,[imm]" */
        emit_var();   /* emit code */
        return;
    }

    /* check for tok_is_num */
    if (dl)
    {
#if TARGET_MVS
        codegen_output_buffer[di++] = 0x05; /* emit "balr r15,0" instruction */
        codegen_output_buffer[di++] = 0xf0; 

        codegen_output_buffer[di++] = 0x41; /* emit "la r15,10(r15)" instruction */
        codegen_output_buffer[di++] = 0xf0; /* target + source index */
        codegen_output_buffer[di++] = 0xf0; /* source base + offset */
        codegen_output_buffer[di++] = 0x0a; /* rest of offset */

        /* jump over constant, and r14 points to the constant */
        codegen_output_buffer[di++] = 0x05; /* emit "balr r14,r15" instruction */
        codegen_output_buffer[di++] = 0xef;
        
        bx = bx_origval;
#else
        codegen_output_buffer[di++] = 0xb8; /* code for "mov ax,imm" */
#endif
        emit_tok(); /* emit imm */
#if TARGET_MVS
        codegen_output_buffer[di++] = 0x58; /* emit "l r6,0(,r14)" instruction */
        codegen_output_buffer[di++] = 0x60;
        codegen_output_buffer[di++] = 0xe0;
        codegen_output_buffer[di++] = 0x00;
#endif
        return;
    }
    
    /* compile var */
#if TARGET_MVS
        codegen_output_buffer[di++] = 0x05; /* emit "balr r15,0" instruction */
        codegen_output_buffer[di++] = 0xf0; 

        codegen_output_buffer[di++] = 0x41; /* emit "la r15,10(r15)" instruction */
        codegen_output_buffer[di++] = 0xf0; /* target + source index */
        codegen_output_buffer[di++] = 0xf0; /* source base + offset */
        codegen_output_buffer[di++] = 0x0a; /* rest of offset */

        /* jump over constant, and r14 points to the constant */
        codegen_output_buffer[di++] = 0x05; /* emit "balr r14,r15" instruction */
        codegen_output_buffer[di++] = 0xef;
        
        bx *= 4; /* there are 65536 possible hash values, each occupying 4 bytes */
        bx += DATASTART; /* and the data space starts after the code */
        emit_tok(); /* emit imm */

        codegen_output_buffer[di++] = 0x58; /* emit "l r6,0(,r14)" instruction */
        codegen_output_buffer[di++] = 0x60;
        codegen_output_buffer[di++] = 0xe0;
        codegen_output_buffer[di++] = 0x00;

        codegen_output_buffer[di++] = 0x1a; /* emit "ar r6,r7" instruction */
        codegen_output_buffer[di++] = 0x67;

        codegen_output_buffer[di++] = 0x58; /* emit "l r6,0(,r6)" instruction */
        codegen_output_buffer[di++] = 0x60;
        codegen_output_buffer[di++] = 0x60;
        codegen_output_buffer[di++] = 0x00;
#else
    ax = 0x068b; /* code for "mov ax,[imm]" */
    emit_var();
#endif
    return;
}

static void emit_var(void)
{
    codegen_output_buffer[di++] = ax & 0xff;
    codegen_output_buffer[di++] = (ax >> 8) & 0xff;
    bx *= 2;   /* bx = 2*bx (scale up for 16-bit) */
    emit_tok();
    return;
}

static void emit_tok(void)
{
    ax = bx;
    /* emit token value */
#if TARGET_MVS
    codegen_output_buffer[di++] = (ax >> 24) & 0xff;
    codegen_output_buffer[di++] = (ax >> 16) & 0xff;
    codegen_output_buffer[di++] = (ax >> 8) & 0xff;
    codegen_output_buffer[di++] = ax & 0xff;
#else
    codegen_output_buffer[di++] = ax & 0xff;
    codegen_output_buffer[di++] = (ax >> 8) & 0xff;
#endif
    tok_next();
    return;
}

/* read and discard one token, then read the next token */
static void tok_next2(void)
{
    tok_next();
    tok_next();
    return;
}

/********************************************************************/
/* get next token, setting the following:                           */
/*   ax: token                                                      */
/*   bx: token                                                      */
/*   dl: tok_is_num                                                 */
/*   dh: tok_is_call                                                */
/********************************************************************/
static void tok_next(void)
{
    getch();

    /* skip spaces (anything <= ' ' is considered space) */
    while (ax <= ' ')
    {
        getch();
    }
    
    bx = 0; /* zero token reg */
    cx = 0; /* zero last-two chars reg */

    /* tok_is_num = (al <= '9') */
    dl = isdigit((unsigned char)ax);

    while (1)
    {
        /* if char is space then break */
        /* also if a control character */
        /* should probably use isspace() and iscntrl() */
        if (ax <= ' ')
        {
            bx_origval = bx;
            bx &= MAXSYM_MASK;
            if (debug)
            {
                printf(" is hash %x\n", bx);
            }
            break;
        }

        if (debug)
        {
            printf("%c", ax);
        }
        
        /* shift this char into cx */
        cx <<= 8;
        cx |= tasc(ax);

        /* Note that this is the real genius of the original Sector C.
           Just hope that a simple atoi implementation, when used on
           non-numeric fields, would produce a value that didn't clash
           with anything important. And the result was - no clash! If
           you happen to use a variable name that clashes with an
           existing token or variable, then you will have to change your
           variable name, or make a much more complex compiler. And
           when bootstrapping, you get to zap in every hex code yourself.
           */

        /* atoi computation: bx = 10 * bx + (ax - '0') */
        bx = 10 * bx + (tasc(ax) - tasc('0'));

        getch();
    }

    cx &= 0xffffU; /* only 2 characters should be inspected */
    ax = cx;
    /* check for single-line comment "//" */
    if (ax == 0x2f2f)
    {
        getch(); /* get next char */
        /* check for newline '\n' */
        while (ax != '\n')
        {
            getch();
        }
        tok_next();
        return;
    }
    
    /* check for multi-line comment "/ *" (without space) */
    if (ax == 0x2f2a)
    {
        tok_next(); /* get next token */
        while (ax != 65475)  /* check for token "* /" (without space) */
        {
            tok_next();
        }
        tok_next();
        return;
    }

    /* check for call parens "()" */
    dh = (ax == 0x2829);

    ax = bx;  /* return token in ax also */
    return;
}



/********************************************************************/
/* get next char: returned in ax (ah == 0, al == ch)                */
/********************************************************************/
static void getch(void)
{
    static int semi = 0;
    
    ax = semi;
    semi = 0;
    if (ax == ';')
    {
        return;
    }

    ax = getc(fp);
    if (ax == EOF)
    {
        printf("not supposed to reach EOF\n");
        exit(EXIT_FAILURE);
    }

    if (ax != ';')
    {
        return;
    }
    
    semi = ax;
    ax = 0;   /* return 0 instead, treated as whitespace */
    return;
}

static void hashclash(void)
{
    printf("duplicate name (or at least, hash clash)\n");
    printf("variables and functions now share the same namespace\n");
    printf("run with debug on to figure out which names hashed\n");
    printf("to the same value\n");
    exit(EXIT_FAILURE);
}

/*********************************************************************/
/*                                                                   */
/*  This program takes an integer as a parameter.  The integer       */
/*  should have a representation of a character.  An integer         */
/*  is returned, containing the representation of that character     */
/*  in the ASCII character set.                                      */
/*                                                                   */
/*********************************************************************/
static int tasc(int loc)
{
  switch (loc)
  {
    case '\t' : return (0x09);
    case '\n' : return (0x0a);
    case '\f' : return (0x0c);
    case '\r' : return (0x0d);
    case ' '  : return (0x20);
    case '!'  : return (0x21);
    case '\"' : return (0x22);
    case '#'  : return (0x23);
    case '$'  : return (0x24);
    case '%'  : return (0x25);
    case '&'  : return (0x26);
    case '\'' : return (0x27);
    case '('  : return (0x28);
    case ')'  : return (0x29);
    case '*'  : return (0x2a);
    case '+'  : return (0x2b);
    case ','  : return (0x2c);
    case '-'  : return (0x2d);
    case '.'  : return (0x2e);
    case '/'  : return (0x2f);
    case '0'  : return (0x30);
    case '1'  : return (0x31);
    case '2'  : return (0x32);
    case '3'  : return (0x33);
    case '4'  : return (0x34);
    case '5'  : return (0x35);
    case '6'  : return (0x36);
    case '7'  : return (0x37);
    case '8'  : return (0x38);
    case '9'  : return (0x39);
    case ':'  : return (0x3a);
    case ';'  : return (0x3b);
    case '<'  : return (0x3c);
    case '='  : return (0x3d);
    case '>'  : return (0x3e);
    case '?'  : return (0x3f);
    case '@'  : return (0x40);
    case 'A'  : return (0x41);
    case 'B'  : return (0x42);
    case 'C'  : return (0x43);
    case 'D'  : return (0x44);
    case 'E'  : return (0x45);
    case 'F'  : return (0x46);
    case 'G'  : return (0x47);
    case 'H'  : return (0x48);
    case 'I'  : return (0x49);
    case 'J'  : return (0x4a);
    case 'K'  : return (0x4b);
    case 'L'  : return (0x4c);
    case 'M'  : return (0x4d);
    case 'N'  : return (0x4e);
    case 'O'  : return (0x4f);
    case 'P'  : return (0x50);
    case 'Q'  : return (0x51);
    case 'R'  : return (0x52);
    case 'S'  : return (0x53);
    case 'T'  : return (0x54);
    case 'U'  : return (0x55);
    case 'V'  : return (0x56);
    case 'W'  : return (0x57);
    case 'X'  : return (0x58);
    case 'Y'  : return (0x59);
    case 'Z'  : return (0x5a);
    case '['  : return (0x5b);
    case '\\' : return (0x5c);
    case ']'  : return (0x5d);
    case '^'  : return (0x5e);
    case '_'  : return (0x5f);
    case '`'  : return (0x60);
    case 'a'  : return (0x61);
    case 'b'  : return (0x62);
    case 'c'  : return (0x63);
    case 'd'  : return (0x64);
    case 'e'  : return (0x65);
    case 'f'  : return (0x66);
    case 'g'  : return (0x67);
    case 'h'  : return (0x68);
    case 'i'  : return (0x69);
    case 'j'  : return (0x6a);
    case 'k'  : return (0x6b);
    case 'l'  : return (0x6c);
    case 'm'  : return (0x6d);
    case 'n'  : return (0x6e);
    case 'o'  : return (0x6f);
    case 'p'  : return (0x70);
    case 'q'  : return (0x71);
    case 'r'  : return (0x72);
    case 's'  : return (0x73);
    case 't'  : return (0x74);
    case 'u'  : return (0x75);
    case 'v'  : return (0x76);
    case 'w'  : return (0x77);
    case 'x'  : return (0x78);
    case 'y'  : return (0x79);
    case 'z'  : return (0x7a);
    case '{'  : return (0x7b);
    case '|'  : return (0x7c);
    case '}'  : return (0x7d);
    case '~'  : return (0x7e);
    default   : return(0);
  }
}
