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

C:\devel\develop>hexdump samp.com
000000  90E81800 B8034CA0 3C38CD21 C3B80500  ......L.<8.!....
000010  89069000 B8070089 063C38C3 E8EEFFC3  .........<8.....

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

static short *symtbl;

static char *codegen_output_buffer;

static FILE *fp;
static FILE *fq;

static jmp_buf compile_jmpbuf;

static int skiptok = 0;

/********************************************************************/
/* binary operator table                                            */
/********************************************************************/
int binary_oper_tbl[] = {
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

int main(int argc, char **argv)
{
    if (argc <= 2)
    {
        printf("multisc <source> <COM>\n");
        printf("builds a 8086 COM file from source code in the\n");
        printf("subset of C90 originally defined by Sector C\n");
        return (EXIT_FAILURE);
    }

    symtbl = malloc(0x10000 * sizeof(short));
    codegen_output_buffer = malloc(0x10000);

    if ((symtbl == NULL)
        || (codegen_output_buffer == NULL))
    {
        printf("insufficient memory\n");
        return (EXIT_FAILURE);
    }
    
    fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        printf("failed to open %s for reading\n", argv[1]);
        return (EXIT_FAILURE);
    }
    
    fq = fopen(argv[2], "wb");
    if (fq == NULL)
    {
        printf("failed to open %s for writing\n", argv[2]);
        return (EXIT_FAILURE);
    }

    di = 0;    /* codegen index, zero'd */

    codegen_output_buffer[di++] = 0x90; /* emit "nop" instruction */
    codegen_output_buffer[di++] = 0xe8; /* emit "call" instruction */
    codegen_output_buffer[di++] = 0x00;
    codegen_output_buffer[di++] = 0x00;
    codegen_output_buffer[di++] = 0xb8; /* emit "mov ax,4c03h" instruction */
    codegen_output_buffer[di++] = 0x03;
    codegen_output_buffer[di++] = 0x4c;
    
    /* override return code with first variable defined */
    /* mov al,ds:[383ch] ret hashes to this */
    codegen_output_buffer[di++] = 0xa0;
    codegen_output_buffer[di++] = 0x3c;
    codegen_output_buffer[di++] = 0x38;
#if 0
    codegen_output_buffer[di++] = 0x00;
    codegen_output_buffer[di++] = 0x00;
#endif
    
    codegen_output_buffer[di++] = 0xcd; /* emit "int 21h" instruction */
    codegen_output_buffer[di++] = 0x21;
    codegen_output_buffer[di++] = 0xc3; /* emit "ret" instruction */

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
        tok_next2();  /* consume "int" and <ident> */
    }

    return (0);
}

/* parse and compile a function decl */
static void compile_function(void)
{
    int oldbx;

    tok_next(); /* consume "void" */
    oldbx = bx;   /* save function name token */
    symtbl[bx] = di; /* record function address in symtbl */
    compile_stmts_tok_next2(); /* compile function body */

    codegen_output_buffer[di++] = 0xc3; /* emit "ret" instruction */

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

#if 0
execute:
  push es                       ; push the codegen segment
  push word [bx]                ; push the offset to "_start()"
  push 0x4000                   ; load new segment for variable data
  pop ds
  retf                          ; jump into it via "retf"
#endif

    ax = symtbl[bx] - 4;
    codegen_output_buffer[2] = ax & 0xff;
    codegen_output_buffer[3] = (ax >> 8) & 0xff;

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
            /* emit "call" instruction */
            codegen_output_buffer[di++] = 0xe8;
            /* load function offset from symbol-table */
            ax = symtbl[bx];
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
            /* emit "jmp" instruction (backwards) */
            codegen_output_buffer[di++] = 0xe9;

            /* compute relative to this location: "dest - cur - 2" */
            /* dest is the start of the while loop, which we saved in tmp */
            /* current location is kept in di */
            /* the 2 is for the 2 bytes that we are just about to write */
            ax = tmp - di - 2;
            codegen_output_buffer[di++] = ax & 0xff;
            codegen_output_buffer[di++] = (ax >> 8) & 0xff;

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

    /* emit "test ax,ax" */
    codegen_output_buffer[di++] = 0x85;
    codegen_output_buffer[di++] = 0xc0;
    
    /* emit "je" instruction */
    codegen_output_buffer[di++] = 0x0f;
    codegen_output_buffer[di++] = 0x84;

    /* emit placeholder for target */
    codegen_output_buffer[di++] = 0x00;
    codegen_output_buffer[di++] = 0x00;
    
    /* ok, so basically what we have done is if you have
       if (something) { do this } then "something" has been
       evaluated to false, with 0 in ax, so in that situation,
       ie ax == 0, you need to skip over a bunch of code.
       So we've emitted the jmp already, and di is pointing
       to the location after that jmp. We normally want si
       pointing to the old location, but it will be clobbered
       by potential recursion, so we save di into a temporary
       variable and only set si when the recursion has finished */

    tmp = di; /* save forward patch location */
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
    ax = 0x0689;   /* code for "mov [imm],ax" */
    emit_var();
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

    ptr++;
    tmp = *ptr;  /* load 16-bit of machine-code */

    codegen_output_buffer[di++] = 0x50; /* code for "push ax" */
    tok_next();      /* consume operator token */
    compile_unary(); /* compile right-hand side */

    codegen_output_buffer[di++] = 0x59; /* code for "pop cx" */
    codegen_output_buffer[di++] = 0x91; /* code for "xchg ax,cx" */

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
        codegen_output_buffer[di++] = 0x00;
        codegen_output_buffer[di++] = 0x0f;
    }

    ax = bx;
    /* emit machine code for op */
    codegen_output_buffer[di++] = ax & 0xff;
    codegen_output_buffer[di++] = (ax >> 8) & 0xff;
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
        codegen_output_buffer[di++] = 0xb8; /* code for "mov ax,imm" */
        emit_tok(); /* emit imm */
        return;
    }
    
    /* compile var */
    /* code for "mov ax,[imm]" */
    codegen_output_buffer[di++] = 0x8b;
    codegen_output_buffer[di++] = 0x06;
    emit_var();
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
    codegen_output_buffer[di++] = ax & 0xff;
    codegen_output_buffer[di++] = (ax >> 8) & 0xff;
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
    while (ax <= 32)
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
            break;
        }

        /* shift this char into cx */
        cx <<= 8;
        cx |= ax;

        /* atoi computation: bx = 10 * bx + (ax - '0') */
        bx = 10 * bx + (ax - '0');

        getch();
    }

    cx &= 0xffffU;
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

    bx &= 0xffffU;
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
