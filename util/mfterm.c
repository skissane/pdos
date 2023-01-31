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

#define IAC 0xff /* telnet */

static int noisy = 0;
static int card = 0;
static int termtype = 1052; /* ASCII ANSI */

static char cardbuf[80];

static void negotiate(FILE *sf);
static void interact(FILE *sf);
static void expect(FILE *sf, unsigned char *buf, size_t buflen);

int febc(int ebc);

int main(int argc, char **argv)
{
    FILE *sf;
    int x = 1;

    if (argc < 2)
    {
        printf("usage: mfterm <serial file>\n");
        printf("establishes a telnet ANSI terminal connection to mainframe\n");
        printf("e.g. mfterm [-3270] com1:\n");
        printf("ctrl-] gets menu when keyboard is active\n");
        return (EXIT_FAILURE);
    }
    if (argc > 2)
    {
        if (strcmp(argv[x], "-3270") == 0)
        {
            termtype = 3270;
        }
        else
        {
            printf("unknown argument %s\n", argv[x]);
            return (EXIT_FAILURE);
        }
        x++;
    }
    sf = fopen(*(argv + x), "r+b");
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
    /* printf("writing\n"); */
    /* IAC WILL TERM_TYPE */
    fwrite("\xff\xfb\x18", 1, 3, sf);

    fseek(sf, 0, SEEK_CUR);
    /* IAC SB TERM_TYPE SEND IAC SE */
    expect(sf, "\xff\xfa\x18\x01\xff\xf0", 6);

    fseek(sf, 0, SEEK_CUR);
    /* printf("writing\n"); */
    /* IAC SB TERM_TYPE IS (ANSI) IAC SE */
    fwrite("\xff\xfa\x18\x00", 1, 4, sf);
    if (termtype == 3270)
    {
        printf("writing IBM-3270\n");
        fwrite("IBM-3270", 1, 8, sf);
    }
    else
    {
        fwrite("ANSI", 1, 4, sf);
    }
    fwrite("\xff\xf0", 1, 2, sf);

    if (termtype == 1052)
    {
    fseek(sf, 0, SEEK_CUR);
    /* IAC WILL ECHO */
    expect(sf, "\xff\xfb\x01", 3);

    fseek(sf, 0, SEEK_CUR);
    /* printf("writing\n"); */
    /* IAC DO ECHO */
    fwrite("\xff\xfd\x01", 1, 3, sf);
    }

    if (termtype == 3270)
    {
        fseek(sf, 0, SEEK_CUR);
        /* do eor and will eor */
        expect(sf, "\xff\xfd\x19" "\xff\xfb\x19", 6);
        fseek(sf, 0, SEEK_CUR);
        fwrite("\xff\xfb\x19" "\xff\xfd\x19", 6, 1, sf);
        fseek(sf, 0, SEEK_CUR);
        printf("binary\n");
        expect(sf, "\xff\xfd\x00" "\xff\xfb\x00", 6);
        fseek(sf, 0, SEEK_CUR);
        fwrite("\xff\xfb\x00" "\xff\xfd\x00", 6, 1, sf);
        fseek(sf, 0, SEEK_CUR);
        {
            int x;

            for (x = 0; x < 1019; x++)
            {
                fgetc(sf);
            }
            printf("read and discarded 1019 bytes of logo\n");
        }
    }

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
    int cnt;
    int keybcode = 0;

    while (c != EOF)
    {
        fseek(sf, 0, SEEK_CUR);
        cnt = 0;
        while (1)
        {
            c = fgetc(sf);
            cnt++;
            /* printf("ccc is %x %d\n", c, cnt++); */
            if (termtype == 1052)
            {
                if ((c == EOF) || (c == XON)) break;
                fputc(c, stdout);
                fflush(stdout);
            }
            else if (termtype == 3270)
            {
                if (c == IAC)
                {
                    c = fgetc(sf);
                    /* EOR is end of transmission */
                    if (c == 0xef)
                    {
                        cnt = 0;
                        continue;
                    }
                    /* ignore any IAC commands */
                    if (c != IAC) continue;
                }
                /* the first character received is x'f1' which is a write
                   command, added by Hercules, not PDOS. That's what the
                   +1 is about */
                /* ignore first 6+1 characters */
                if (cnt == 2)
                {
                    keybcode = c;
                    printf("keyboard code is %x\n", c);
                }
                if (cnt <= (6+1)) continue;
                /* ignore stuff after actual data */
                if (cnt > (1766+1)) continue;
                c = febc(c);
                if (c != 0)
                {
                    fputc(c, stdout);
                    fflush(stdout);
                }
            }
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

/*********************************************************************/
/*                                                                   */
/*  This Program Written By Paul Edwards.                            */
/*  Released to the public domain.                                   */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  This program takes an integer as a parameter.  The integer       */
/*  should have a representation of an EBCDIC character.  An integer */
/*  is returned, containing the representation of that character     */
/*  in whatever character set you are using on this system.          */
/*                                                                   */
/*********************************************************************/
int febc(int ebc)
{
  switch (ebc)
  {
    case 0x15 : return('\n');
    case 0x25 : return('\n');
    case 0x40 : return(' ');
    case 0x5A : return('!');
    case 0x7f : return('\"');
    case 0x7b : return('#');
    case 0x5b : return('$');
    case 0x6c : return('%');
    case 0x50 : return('&');
    case 0x7d : return('\'');
    case 0x4d : return('(');
    case 0x5d : return(')');
    case 0x5c : return('*');
    case 0x4e : return('+');
    case 0x6b : return(',');
    case 0x60 : return('-');
    case 0x4b : return('.');
    case 0x61 : return('/');
    case 0xf0 : return('0');
    case 0xf1 : return('1');
    case 0xf2 : return('2');
    case 0xf3 : return('3');
    case 0xf4 : return('4');
    case 0xf5 : return('5');
    case 0xf6 : return('6');
    case 0xf7 : return('7');
    case 0xf8 : return('8');
    case 0xf9 : return('9');
    case 0x7a : return(':');
    case 0x5e : return(';');
    case 0x4c : return('<');
    case 0x7e : return('=');
    case 0x6e : return('>');
    case 0x6f : return('?');
    case 0x7c : return('@');
    case 0xc1 : return('A');
    case 0xc2 : return('B');
    case 0xc3 : return('C');
    case 0xc4 : return('D');
    case 0xc5 : return('E');
    case 0xc6 : return('F');
    case 0xc7 : return('G');
    case 0xc8 : return('H');
    case 0xc9 : return('I');
    case 0xd1 : return('J');
    case 0xd2 : return('K');
    case 0xd3 : return('L');
    case 0xd4 : return('M');
    case 0xd5 : return('N');
    case 0xd6 : return('O');
    case 0xd7 : return('P');
    case 0xd8 : return('Q');
    case 0xd9 : return('R');
    case 0xe2 : return('S');
    case 0xe3 : return('T');
    case 0xe4 : return('U');
    case 0xe5 : return('V');
    case 0xe6 : return('W');
    case 0xe7 : return('X');
    case 0xe8 : return('Y');
    case 0xe9 : return('Z');
    case 0xad : return('[');
    case 0xe0 : return('\\');
    case 0xbd : return(']');
    case 0x5f : return('^');
    case 0x6d : return('_');
    case 0x79 : return('`');
    case 0x81 : return('a');
    case 0x82 : return('b');
    case 0x83 : return('c');
    case 0x84 : return('d');
    case 0x85 : return('e');
    case 0x86 : return('f');
    case 0x87 : return('g');
    case 0x88 : return('h');
    case 0x89 : return('i');
    case 0x91 : return('j');
    case 0x92 : return('k');
    case 0x93 : return('l');
    case 0x94 : return('m');
    case 0x95 : return('n');
    case 0x96 : return('o');
    case 0x97 : return('p');
    case 0x98 : return('q');
    case 0x99 : return('r');
    case 0xa2 : return('s');
    case 0xa3 : return('t');
    case 0xa4 : return('u');
    case 0xa5 : return('v');
    case 0xa6 : return('w');
    case 0xa7 : return('x');
    case 0xa8 : return('y');
    case 0xa9 : return('z');
    case 0xc0 : return('{');
    case 0x4f : return('|');
    case 0xd0 : return('}');
    case 0xa1 : return('~');
    default   : return(0);
  }
}
