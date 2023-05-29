/* ***************************************************************** */
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/* ***************************************************************** */
/* ***************************************************************** */
/*                                                                   */
/*  mssamp - sample program that can be handled by multisc           */
/*                                                                   */
/*  This program just prints C using WTO                             */
/*                                                                   */
/*  compile like this:                                               */
/*  multisc mssamp.c mssamp.com                                      */
/*                                                                   */
/* ***************************************************************** */

int ret; /* must be this name */
    /* reason being that the hash is calculated as:
       ( (114-48)*100+(101-48)*10+(116-48) ) * 2 = 14396 = x'383c'
       which you can see referenced in the assembler below,
       and also in the startup code which is embedded in the compiler */
int x;

/* this will probably be in a separate file, and pdcc used to
   merge them together this first. It needs to be seen before
   "main" calls it */

void display()
{
    /* can't use r7 - points to base of code */
    /* can't use r13 - points to save area */
    /* all other registers are free to use */
    
    /* we want to load r9 with data at a fixed address (ret) from r7 */
    
    /* balr r15, 0 */
    asm 5; asm 240; /* 05 f0 */
    
/* base1: */
    
    /* bal r14, 8(,r15) */ /* call bypass1 */
    asm 69; asm 224; asm 240; asm 8; /* 45 e0 f0 08 */
    
    /* 6 * 0x10000 (DATASTART) + 0x7078 = 0x67078 */
    asm 0; asm 6; asm 112; asm 120;

/* bypass1: */
        
    /* l r9, 0(,r14) */
    asm 88; asm 144; asm 224; asm 0; /* 58 90 e0 00 */

    /* ar r9, r7 */
    asm 26; asm 151; /* 1a 97 */
        
    /* l r9, 0(,r9) */
    asm 88; asm 144; asm 144; asm 0; /* 58 90 90 00 */


    /* now we have the data - store it where the WTO is */
    /* reestablish addressability in case someone wants to
       put more code above here */
            
    /* balr r15, 0 */
    asm 5; asm 240; /* 05 f0 */

/* base2: */
        
    /* stc r9, 12(,r15) */ 
    asm 66; asm 144; asm 240; asm 12; /* 42 90 f0 0c */
    
    /* bal r1, 18(,r15) */  /* call bypass2 */
    asm 69; asm 16; asm 240; asm 18; /* 45 10 f0 12 */
    
    /* this is the WTO parameters */
    asm 0; asm 5; /* text length of 1 requires 5 */
    asm 128; asm 0; /* mcs flags */
    asm 193; /* placeholder ('A') */
    asm 0; asm 0; /* descriptor codes */
    asm 0; asm 32; /* routing codes */
    asm 0; /* one byte padding to get 2-byte alignment because we
              output a single character */
    
/* bypass2: */
    
    asm 10; asm 35; /* svc 35 */

    /* asm 0; asm 0; */ /* dc h'0' to force a s0c1 abend - can't easily
                           use ex 0,* to get s0c3 instead */
    
}

/* this is the only function you really need to read */

void main()
{
    ret = 195; /* 'C' in EBCDIC */
    display();
    ret = 0; /* return code to operating system */
}

/* this probably belongs in a separate file, for pdcc to
   merge last */

void _start()
{
    main();
}
