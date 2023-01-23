/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  mfterm - terminal mainly for mainframe                           */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#define XON 0x11

static void negotiate(FILE *sf);
static void interact(FILE *sf);
static void expect(FILE *sf, unsigned char *buf, size_t buflen);

int main(int argc, char **argv)
{
    FILE *sf;

    if (argc != 2)
    {
        printf("usage: mfterm <serial file>\n");
        printf("establishes a telnet ANSI terminal connection to mainframe\n");
        printf("e.g. mfterm com1:\n");
        return (EXIT_FAILURE);
    }
    sf = fopen(*(argv + 1), "r+b");
    if (sf == NULL)
    {
        printf("failed to open serial file\n");
        return (EXIT_FAILURE);
    }

    negotiate(sf);
    setvbuf(stdin, NULL, _IONBF, 0);
    interact(sf);
    setvbuf(stdin, NULL, _IOLBF, 0);

    fclose(sf);
    return (0);
}

static void negotiate(FILE *sf)
{
    /* IAC DO TERM_TYPE */
    expect(sf, "\xff\xfd\x18", 3);

    fseek(sf, 0, SEEK_CUR);
    printf("writing\n");
    /* IAC WILL TERM_TYPE */
    fwrite("\xff\xfb\x18", 1, 3, sf);

    fseek(sf, 0, SEEK_CUR);
    /* IAC SB TERM_TYPE SEND IAC SE */
    expect(sf, "\xff\xfa\x18\x01\xff\xf0", 6);

    fseek(sf, 0, SEEK_CUR);
    printf("writing\n");
    /* IAC SB TERM_TYPE IS (ANSI) IAC SE */
    fwrite("\xff\xfa\x18\x00" "ANSI" "\xff\xf0", 1, 10, sf);

    fseek(sf, 0, SEEK_CUR);
    /* IAC WILL ECHO */
    expect(sf, "\xff\xfb\x01", 3);

    fseek(sf, 0, SEEK_CUR);
    printf("writing\n");
    /* IAC DO ECHO */
    fwrite("\xff\xfd\x01", 1, 3, sf);

    return;
}

static void expect(FILE *sf, unsigned char *buf, size_t buflen)
{
    int c;
    size_t x;

    for (x = 0; x < buflen; x++)
    {
        c = fgetc(sf);
        printf("c is ... %x\n", c);
        if (c != buf[x])
        {
            printf("got %x, expected %x\n", c, buf[x]);
            exit(EXIT_FAILURE);
        }
    }
    return;
}

static void interact(FILE *sf)
{
    int c = 0;

    while (c != EOF)
    {
        fseek(sf, 0, SEEK_CUR);
        while (1)
        {
            c = fgetc(sf);
            if ((c == EOF) || (c == XON)) break;
            fputc(c, stdout);
            fflush(stdout);
        }
        if (c == XON)
        {
            fseek(sf, 0, SEEK_CUR);
            c = fgetc(stdin);
            /* in case of synchronization issues, pressing ctrl-Q allows
               reversion to reading from serial port */
            if ((c != EOF) && (c != XON))
            {
                fputc(c, sf);
                fflush(sf);
            }
        }
    }
    return;
}
