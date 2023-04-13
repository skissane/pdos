/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards, 3:711/934@fidonet.         */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  uart.c - primitives to drive uart                                */
/*                                                                   */
/*********************************************************************/

#include "uart.h"
#include "lldos.h"

static void uartEnableInt(UART *uart, int id);
static void uartDisableInt(UART *uart, int id);

void uartInit(UART *uart)
{
    uart->baseAddr = 0;
    return;
}

void uartAddress(UART *uart, int baseAddr)
{
    uart->baseAddr = baseAddr;
    uart->rec = baseAddr;
    uart->tx = baseAddr;
    uart->ienable = baseAddr + 1;
    uart->intid = baseAddr + 2;
    uart->lcont = baseAddr + 3;
    uart->mcont = baseAddr + 4;
    uart->lstat = baseAddr + 5;
    uart->istat = baseAddr + 6;
    uart->scratch = baseAddr + 7;
    uart->baudl = baseAddr;
    uart->baudm = baseAddr + 1;    

    uartDLABOff(uart);

    return;
}

void uartDLABOff(UART *uart)
{
    int ch;
    
    ch = PREADB(uart->lcont);
    ch &= ~0x80;
    PWRITEB(uart->lcont, ch);
    return;
}

void uartDLABOn(UART *uart)
{
    int ch;
    
    ch = PREADB(uart->lcont);
    ch |= 0x80;
    PWRITEB(uart->lcont, ch);
    return;
}

void uartTerm(UART *uart)
{
    uart->baseAddr = 0;
    return;
}

void uartReset(UART *uart)
{
    uart->baseAddr = 0;
    return;
}

void uartDisableInts(UART *uart)
{
    PWRITEB(uart->ienable, 0x00);
    return;
}

void uartEnableRxRDY(UART *uart)
{
    uartEnableInt(uart, 0x01);
    return;
}

void uartDisableRxRDY(UART *uart)
{
    uartDisableInt(uart, 0x01);
    return;
}

void uartDisableTBE(UART *uart)
{
    uartDisableInt(uart, 0x02);
    return;
}

void uartEnableTBE(UART *uart)
{
    uartEnableInt(uart, 0x02);
    return;
}

void uartDisableModem(UART *uart)
{
    uartDisableInt(uart, 0x08);
    return;
}

void uartEnableModem(UART *uart)
{
    uartEnableInt(uart, 0x08);
    return;
}

static void uartEnableInt(UART *uart, int id)
{
    int ch;
    
    ch = PREADB(uart->ienable);
    ch |= id;
    PWRITEB(uart->ienable, ch);
    return;
}

static void uartDisableInt(UART *uart, int id)
{
    int ch;
    
    ch = PREADB(uart->ienable);
    ch &= ~id;
    PWRITEB(uart->ienable, ch);
    return;
}

void uartRaiseDTR(UART *uart)
{
    int ch;
    
    ch = PREADB(uart->mcont);
    ch |= 0x01;
    PWRITEB(uart->mcont, ch);
    return;
}

void uartDisable16550(UART *uart)
{
    int ch;
    
    ch = PREADB(uart->intid);
    if ((ch & 0xc0) != 0)
    {
        PWRITEB(uart->intid, 0x00);
    }  
    return;
}

void uartDropDTR(UART *uart)
{
    int ch;
    
    ch = PREADB(uart->mcont);
    ch &= ~0x01;
    PWRITEB(uart->mcont, ch);
    return;
}

void uartRaiseRTS(UART *uart)
{
    int ch;
    
    ch = PREADB(uart->mcont);
    ch |= 0x02;
    PWRITEB(uart->mcont, ch);
    return;
}

void uartDropRTS(UART *uart)
{
    int ch;
    
    ch = PREADB(uart->mcont);
    ch &= ~0x02;
    PWRITEB(uart->mcont, ch);
    return;
}

void uartEnableGPO2(UART *uart)
{
    int ch;
    
    ch = PREADB(uart->mcont);
    ch |= 0x08;
    PWRITEB(uart->mcont, ch);
    return;
}

void uartDisableGPO2(UART *uart)
{
    int ch;
    
    ch = PREADB(uart->mcont);
    ch &= ~0x08;
    PWRITEB(uart->mcont, ch);
    return;
}

void uartSetBaud(UART *uart, long bps)
{
    unsigned int divisor;
    
    divisor = (unsigned int)(115200L/bps);
    uartDLABOn(uart);
    PWRITEB(uart->baudl, divisor % 256);
    PWRITEB(uart->baudm, divisor / 256);
    uartDLABOff(uart);
    return;
}

void uartSetParity(UART *uart, int parity)
{
    int ch;
    
    ch = PREADB(uart->lcont);
    ch &= ~(0x07 << 3);
    ch |= (parity << 3);
    PWRITEB(uart->lcont, ch);
    return;
}

void uartSetDataBits(UART *uart, int databits)
{
    int ch;
    
    ch = PREADB(uart->lcont);
    ch &= ~0x03;
    ch |= (databits - 5) ;
    PWRITEB(uart->lcont, ch);
    return;
}

void uartSetStopBits(UART *uart, int stopbits)
{
    int ch;
    
    ch = PREADB(uart->lcont);
    ch &= ~(0x01 << 2);
    ch |= ((stopbits - 1) << 2);
    PWRITEB(uart->lcont, ch);
    return;
}

void uartTxCh(UART *uart, int ch)
{
    PWRITEB(uart->tx, ch);
    return;
}

int uartRecCh(UART *uart)
{
    int ch;
    
    ch = PREADB(uart->rec);
    return (ch);
}

int uartGetIntType(UART *uart)
{
    int ch;
    int ret;
    
    ch = PREADB(uart->intid) & 0x07;
    switch (ch)
    {
        case 0: ret = UART_RS232;
                break;
        case 1: ret = UART_NO_PENDING;
                break;        
        case 2: ret = UART_TBE;
                break;
        case 3: ret = UART_ERRBRK;
                break;
        case 4: ret = UART_RxRDY;
                break;
        default: ret = UART_OTHER_INT;
                 break;
    }
    return (ret);
}

int uartGetLineStatus(UART *uart)
{
    int ch;
    
    ch = PREADB(uart->lstat);
    return (ch);
}

int uartGetInputStatus(UART *uart)
{
    int ch;
    
    ch = PREADB(uart->istat);
    return (ch);
}

int uartCTS(UART *uart)
{
    int ch;
    int ret;

    ch = PREADB(uart->istat);    
    if ((ch & 0x10) != 0)
    {
        ret = 1;
    }
    else
    {
        ret = 0;
    }
    return (ret);
}

