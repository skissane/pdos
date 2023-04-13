/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards, 3:711/934@fidonet.         */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  uart.h - see uart.c for docs                                     */
/*                                                                   */
/*********************************************************************/

#ifndef UART_INCLUDED
#define UART_INCLUDED

#include <stddef.h>

#include "lldos.h"

#define UART_NO_PARITY 0
#define UART_ODD_PARITY 1
#define UART_EVEN_PARITY 3
#define UART_NO_PENDING 0
#define UART_RxRDY 1
#define UART_TBE 2
#define UART_OTHER_INT 3
#define UART_RS232 4
#define UART_ERRBRK 5

typedef struct
{
    unsigned baseAddr;
    unsigned rec;
    unsigned tx;
    unsigned ienable;
    unsigned intid;
    unsigned lcont;
    unsigned mcont;
    unsigned lstat;
    unsigned istat;
    unsigned scratch;
    unsigned baudl;
    unsigned baudm;    
} UART;

void uartInit(UART *uart);
void uartAddress(UART *uart, int baseAddr);
void uartDLABOff(UART *uart);
void uartDLABOn(UART *uart);
void uartTerm(UART *uart);
void uartReset(UART *uart);
void uartDisableInts(UART *uart);
void uartEnableRxRDY(UART *uart);
void uartDisableRxRDY(UART *uart);
void uartDisableTBE(UART *uart);
void uartEnableTBE(UART *uart);
void uartDisableModem(UART *uart);
void uartEnableModem(UART *uart);
void uartRaiseDTR(UART *uart);
void uartDisable16550(UART *uart);
void uartDropDTR(UART *uart);
void uartRaiseRTS(UART *uart);
void uartDropRTS(UART *uart);
void uartEnableGPO2(UART *uart);
void uartDisableGPO2(UART *uart);
void uartSetBaud(UART *uart, long bps);
void uartSetParity(UART *uart, int parity);
void uartSetDataBits(UART *uart, int databits);
void uartSetStopBits(UART *uart, int stopbits);
void uartTxCh(UART *uart, int ch);
int uartRecCh(UART *uart);
int uartGetIntType(UART *uart);
int uartGetLineStatus(UART *uart);
int uartGetInputStatus(UART *uart);
int uartCTS(UART *uart);

#endif
