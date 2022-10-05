/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  terminal - read and write a port, normally a serial port         */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <bos.h>

#ifdef EBCDIC
#define CHAR_ESC_CHAR '\x27'
#else
#define CHAR_ESC_CHAR '\x1b'
#endif

#define CHAR_XON_CHAR '\x11'

int main(int argc, char **argv)
{
    unsigned int port;
    int c;
    int last = 0;
    char *whofirst;

    if (argc <= 2)
    {
        printf("usage: terminal <port number> <who first>\n");
        printf("reads from keyboard, writes to port\n");
        printf("reads from port, writes to screen\n");
        printf("you need to decide who goes first - \"me\" or \"you\"\n");
        printf("ESC will exit when you are in control\n");
        printf("ctrl-Q (XON) will transfer control to the other guy\n");
        return (EXIT_FAILURE);
    }
    port = atoi(*(argv + 1));
    whofirst = *(argv + 2);
    if (strcmp(whofirst, "you") == 0) goto you;
    while (1)
    {
        while (1)
        {
            c = fgetc(stdin);
            if (c == CHAR_ESC_CHAR) exit(0);
            if (c == '\n')
            {
                BosSerialWriteChar(port, '\r');
            }
            BosSerialWriteChar(port, c);
            if (c == CHAR_XON_CHAR) break;
        }
        you:
        while (1)
        {
            while (1)
            {
                c = BosSerialReadChar(port);
                /* printf("got %x from bios\n", c); */
                if ((c & 0x8000U) == 0) break;
            }
            c &= 0xff;
            if (c == CHAR_XON_CHAR) break;
            /* printf("%02X %c\n", c, c); */
            if (c != 0x00)
            {
                if (last == 0x0d)
                {
                    if (c == 0x0a)
                    {
                        last = c;
                        continue;
                    }
                    last = c;
                }
                else if (c == 0x0d)
                {
                    last = c;
                    c = '\n';
                }
                else
                {
                    last = c;
                }
                fputc(c, stdout);
                fflush(stdout);
            }
        }
    }
    return (0);
}
