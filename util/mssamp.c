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
/*  This program just prints ABCDE                                   */
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
    /* mov ah, 2    B4 02 */
    asm 180; asm 2;
    /* mov dl, ds:[0xxyy]    8A 16 yy xx */
    /* 383ch is "ret" variable */
    asm 138; asm 22; asm 60; asm 56;
    /* int 21h    CD 21 */
    asm 205; asm 33;
}

/* this is the only function you really need to read */

void main()
{
    x = 60;
    ret = x + 5;
    while( ret < 70 ){
        display();
        ret = ret + 1;
    }
    ret = 13;
    display();
    ret = 10;
    display();
    ret = 0; /* return code to operating system */
}

/* this probably belongs in a separate file, for pdcc to
   merge last */

void _start()
{
    main();
}
