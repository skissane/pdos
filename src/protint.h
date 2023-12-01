/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  protint - protected mode interface                               */
/*                                                                   */
/*  There are 3 interfaces defined to allow a real mode program to   */
/*  execute protected mode code:                                     */
/*                                                                   */
/*  rawprot - this is the simplest function, designed to allow you   */
/*            to switch to protected mode and call the code, all     */
/*            done with interrupts enabled.                          */
/*                                                                   */
/*            Here is an example of some code that moves 0x1245678   */
/*            into EAX (the return code) and then prints the result. */
/*                                                                   */
/*  #include "protint.h"                                             */
/*  unsigned char buf[] = { 0xb8, 0x78, 0x56, 0x34, 0x12, 0xc3 };    */
/*  static unsigned char stack[2000];                                */
/*  printf("%lx\n", rawprot(0, ADDR2ABS(buf), 0,                     */
/*                          ADDR2ABS(stack + sizeof stack), 0, 0);   */
/*                                                                   */
/*  runprot - this function will reenable interrupts when it calls   */
/*            the provided code, so sets up interrupt handlers that  */
/*            can pass the interrupts down to whatever was managing  */
/*            the interrupts in real mode previously (BIOS?)         */
/*                                                                   */
/*  runaout - this function loads a particular sort of executable    */
/*            known as a.out into memory and then runs it.  This     */
/*            function may be removed from this particular           */
/*            interface as being not generally applicable.           */
/*                                                                   */
/*********************************************************************/

#ifndef PROTINT_INCLUDED
#define PROTINT_INCLUDED
unsigned long rawprot(unsigned long csbase,
                      unsigned long ip,
                      unsigned long dsbase,
                      unsigned long prot_sp,
                      unsigned long userparm,
                      unsigned long intloc);
unsigned long runprot(unsigned long csbase,
                      unsigned long ip,
                      unsigned long dsbase,
                      unsigned long prot_sp,
                      unsigned long userparm);
unsigned long runaout(char *fnm, 
                      unsigned long absaddr,
                      unsigned long userparm);

typedef struct {
    unsigned long runreal;
#ifdef __SUBC__
    int dummy1;
#endif
    unsigned long intbuffer;
#ifdef __SUBC__
    int dummy2;
#endif
    unsigned long dorealint;
#ifdef __SUBC__
    int dummy3;
#endif
    unsigned long userparm;    
#ifdef __SUBC__
    int dummy4;
#endif
} runprot_parms;

typedef struct {
    unsigned long gdt;
#ifdef __SUBC__
    int dummy1;
#endif
    unsigned long freem_start;
#ifdef __SUBC__
    int dummy2;
#endif
    unsigned long intloc;
#ifdef __SUBC__
    int dummy3;
#endif
    unsigned long csbase;
#ifdef __SUBC__
    int dummy4;
#endif
    unsigned long dsbase;
#ifdef __SUBC__
    int dummy5;
#endif
    unsigned long userparm;
#ifdef __SUBC__
    int dummy6;
#endif
} rawprot_parms;

extern unsigned long (*runreal_p)(unsigned long func, unsigned short *regs);
extern rawprot_parms *rp_parms;

unsigned long rawprot_p(rawprot_parms *parmlist);
unsigned long runprot_p(rawprot_parms *parmlist);
unsigned long runaout_p(rawprot_parms *parmlist);
void protintHandler(int interrpt, int (*func)(unsigned int *prm));
void *protintGetHandler(int interrpt);

#endif
