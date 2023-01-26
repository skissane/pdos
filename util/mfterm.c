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

#if 0

Here is a list of control characters. 6 will need to be
requisitioned to support Vietnamese. VISCII used a set of
6 characters that interferes with my use of micro-emacs etc.

ctrl-B is common between ASCII and EBCDIC so we will reserve
that for future use, and the other 6 "AVAILABLE" can be used
for Vietnamese.

I would have used ctrl-B as the menu/mode switch character
here, but there is already prior art to use ctrl-], which is
0x1d, and also common between ASCII and EBCDIC.

0 - ctrl-@ - used by C etc.
1 - ctrl-A - used by micro-emacs
2 - ctrl-B - AVAILABLE
3 - ctrl-C - used to break programs
4 - ctrl-D - used to indicate EOF in Unix
5 - ctrl-E - used by micro-emacs
6 - ctrl-F - ACK - may be AVAILABLE
7 - ctrl-G - used by micro-emacs and to beep
8 - ctrl-H - used as backspace
9 - ctrl-I - tab
A - ctrl-J - newline
B - ctrl-K - used by micro-emacs
C - ctrl-L - used by micro-emacs
D - ctrl-M - carriage return
E - ctrl-N - AVAILABLE
F - ctrl-O - AVAILABLE
10 - ctrl-P - AVAILABLE
11 - ctrl-Q - used for XON
12 - ctrl-R - formfeed, AVAILABLE
13 - ctrl-S - XOFF and used by micro-emacs
14 - ctrl-T - AVAILABLE
15 - ctrl-U - NAK - may be AVAILABLE
16 - ctrl-V - used by micro-emacs, but if page-down works can be made available
17 - ctrl-W - used by nano - may be AVAILABLE
18 - ctrl-X - used by micro-emacs
19 - ctrl-Y - used by micro-emacs
1A - ctrl-Z - used for EOF by MSDOS
1B - ctrl-[ - ESC
1C - ctrl-\ - used by Unix to kill process
1D - ctrl-] - used by Telnet to get prompt
1E - ctrl-^ - AVAILABLE
1F - ctrl-_ - AVAILABLE

#endif


#include <stdio.h>
#include <stdlib.h>

#define XON 0x11
#define MENU 0x1d

static int noisy = 0;
static int card = 0;

static char cardbuf[80];

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
        printf("ctrl-] gets menu when keyboard is active\n");
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
    FILE *cf = NULL;

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
            if (card)
            {
                fread(cardbuf, 1, sizeof cardbuf, cf);
                if (feof(cf))
                {
                    printf("finished IPL, returning to keyboard\n");
                    fclose(cf);
                    card = 0;
                }
                else
                {
                    fwrite(cardbuf, 1, sizeof cardbuf, sf);
                    continue;
                }
            }
            c = fgetc(stdin);
            /* ctrl-] brings up a menu */
            if ((c != EOF) && (c != MENU))
            {
                fputc(c, sf);
                fflush(sf);
            }
            if (c == MENU)
            {
                char buf[50];

                printf("menu - options are noisy, quiet, card\n");
#if 0
                setvbuf(stdin, NULL, _IOLBF, 0);
                fgets(buf, sizeof buf, stdin);
                setvbuf(stdin, NULL, _IONBF, 0);
                if (strcmp(buf, "noisy\n") == 0) noisy = 1;
                if (strcmp(buf, "quiet\n") == 0) noisy = 0;
                if (strcmp(buf, "card\n") == 0) card = 1;
#endif
                card = 1;
                if (card)
                {
                    cf = fopen("iplcrd.dat", "rb");
                    if (cf == NULL)
                    {
                        printf("failed to open iplcrd.dat\n");
                        card = 0;
                        c = fgetc(stdin);
                        continue;
                    }
                    fread(cardbuf, 1, sizeof cardbuf, cf);
                    fseek(sf, 0, SEEK_CUR);
                    fwrite(cardbuf, 1, sizeof cardbuf, sf);
                    continue;
                }
            }
        }
    }
    return;
}
