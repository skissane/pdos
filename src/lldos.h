/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  lldos.h - see lldos.c for doco                                   */
/*                                                                   */
/*********************************************************************/

#ifndef LLDOS_INCLUDED
#define LLDOS_INCLUDED

#ifdef __32BIT__
#define PWRITEB(x, y) outp(x, y)
#define PREADB(x) inp(x)
#else
#define PWRITEB(x, y) wportb(x, y)
#define PREADB(x) rportb(x)
#endif
#define INTSTOP() disable()
#define INTALLOW() enable()

#if (defined(MSDOS) && defined(__WATCOMC__))
#define DOSPREF cdecl
#else
#define DOSPREF
#endif

int DOSPREF getfar(long address);
void DOSPREF putfar(long address, unsigned ch);
int DOSPREF rportb(int port);
void DOSPREF wportb(int port, unsigned ch);
void DOSPREF disable(void);
void DOSPREF enable(void);
void DOSPREF callfar(unsigned long address);
int DOSPREF callwithpsp(char *address,
                        char *psp,
                        unsigned int ss_new,
                        unsigned int sp_new);
void DOSPREF callwithbypass(int retcode);
void DOSPREF a20e(void);
void DOSPREF reboot(void);
void DOSPREF boot(unsigned long drive);
int DOSPREF getabs(long address);
void DOSPREF putabs(long address, unsigned ch);
void DOSPREF poweroff(void);

#endif
