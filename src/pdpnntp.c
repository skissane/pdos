/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards                             */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  pdpnntp - a nntp newsgroup reader                                */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>

static FILE *comm;
static char buf[1000];

static char *getline(FILE *comm, char *buf, size_t szbuf);
static char *put(FILE *comm, char *buf);
static char *putline(FILE *comm, char *buf);
static int fasc(int asc);
static int tasc(int loc);

int main(int argc, char **argv)
{
    char *user = NULL;
    char *pw = NULL;

    if (argc == 4)
    {
        user = *(argv + 2);
        pw = *(argv + 3);
    }
    else if (argc != 2)
    {
        printf("usage: pdpnntp <comm channel> [user] [password]\n");
        return (EXIT_FAILURE);
    }

#ifdef __MVS__
    comm = fopen(*(argv + 1), "rb");
#else
    comm = fopen(*(argv + 1), "r+b");
#endif
    if (comm == NULL)
    {
        printf("can't open %s\n", *(argv + 1));
        return (EXIT_FAILURE);
    }

    setvbuf(comm, NULL, _IONBF, 0);

    getline(comm, buf, sizeof buf); 
    printf("%s\n", buf);

#ifndef __MVS__
    fseek(comm, 0, SEEK_CUR);

    putline(comm, "LIST");

    fseek(comm, 0, SEEK_CUR);

    while (1)
    {
        getline(comm, buf, sizeof buf);
        if (buf[0] == '.') break;
        printf("%s\n", buf);
    }
#endif
    if (user != NULL)
    {
        printf("putting user\n");
        fseek(comm, 0, SEEK_CUR);
        put(comm, "AUTHINFO USER ");
        putline(comm, user);
        fseek(comm, 0, SEEK_CUR);
        getline(comm, buf, sizeof buf);
        printf("%s\n", buf);
        printf("putting password\n");
        fseek(comm, 0, SEEK_CUR);
        put(comm, "AUTHINFO PASS ");
        putline(comm, pw);
        fseek(comm, 0, SEEK_CUR);
        getline(comm, buf, sizeof buf);
        printf("%s\n", buf);
    }
    printf("doing quit\n");
    fseek(comm, 0, SEEK_CUR);
    putline(comm, "QUIT");
    fseek(comm, 0, SEEK_CUR);
    getline(comm, buf, sizeof buf);
    printf("%s\n", buf);
    fclose(comm);
    return (0);
}

static char *getline(FILE *comm, char *buf, size_t szbuf)
{
    size_t x;
    int c;

    for (x = 0; x < szbuf - 2; x++)
    {
        c = getc(comm);
        /* ignore virtualbox returning NULs for timeout */
        if (c == 0)
        {
            x--;
            continue;
        }
        if (c == 0x0d)
        {
            getc(comm);
            break;
        }
        buf[x] = fasc(c);
    }
    buf[x] = '\0';
    return (buf);
}

static char *put(FILE *comm, char *buf)
{
    size_t x;

    x = 0;
    while (buf[x] != '\0')
    {
        putc(tasc(buf[x]), comm);
        x++;
    }
    return (buf);
}

static char *putline(FILE *comm, char *buf)
{
    size_t x;

    x = 0;
    while (buf[x] != '\0')
    {
        putc(tasc(buf[x]), comm);
        x++;
    }
    putc(0x0d, comm);
    putc(0x0a, comm);
    return (buf);
}

/*********************************************************************/
/*                                                                   */
/*  This program takes an integer as a parameter.  The integer       */
/*  should have a representation of an ascii character.  An integer  */
/*  is returned, containing the representation of that character     */
/*  in whatever character set you are using on this system.          */
/*                                                                   */
/*********************************************************************/
static int fasc(int asc)
{
  switch (asc)
  {
    case 0x09 : return('\t');
    case 0x0a : return('\n');
    case 0x0c : return('\f');
    case 0x0d : return('\r');
    case 0x20 : return(' ');
    case 0x21 : return('!');
    case 0x22 : return('\"');
    case 0x23 : return('#');
    case 0x24 : return('$');
    case 0x25 : return('%');
    case 0x26 : return('&');
    case 0x27 : return('\'');
    case 0x28 : return('(');
    case 0x29 : return(')');
    case 0x2a : return('*');
    case 0x2b : return('+');
    case 0x2c : return(',');
    case 0x2d : return('-');
    case 0x2e : return('.');
    case 0x2f : return('/');
    case 0x30 : return('0');
    case 0x31 : return('1');
    case 0x32 : return('2');
    case 0x33 : return('3');
    case 0x34 : return('4');
    case 0x35 : return('5');
    case 0x36 : return('6');
    case 0x37 : return('7');
    case 0x38 : return('8');
    case 0x39 : return('9');
    case 0x3a : return(':');
    case 0x3b : return(';');
    case 0x3c : return('<');
    case 0x3d : return('=');
    case 0x3e : return('>');
    case 0x3f : return('?');
    case 0x40 : return('@');
    case 0x41 : return('A');
    case 0x42 : return('B');
    case 0x43 : return('C');
    case 0x44 : return('D');
    case 0x45 : return('E');
    case 0x46 : return('F');
    case 0x47 : return('G');
    case 0x48 : return('H');
    case 0x49 : return('I');
    case 0x4a : return('J');
    case 0x4b : return('K');
    case 0x4c : return('L');
    case 0x4d : return('M');
    case 0x4e : return('N');
    case 0x4f : return('O');
    case 0x50 : return('P');
    case 0x51 : return('Q');
    case 0x52 : return('R');
    case 0x53 : return('S');
    case 0x54 : return('T');
    case 0x55 : return('U');
    case 0x56 : return('V');
    case 0x57 : return('W');
    case 0x58 : return('X');
    case 0x59 : return('Y');
    case 0x5a : return('Z');
    case 0x5b : return('[');
    case 0x5c : return('\\');
    case 0x5d : return(']');
    case 0x5e : return('^');
    case 0x5f : return('_');
    case 0x60 : return('`');
    case 0x61 : return('a');
    case 0x62 : return('b');
    case 0x63 : return('c');
    case 0x64 : return('d');
    case 0x65 : return('e');
    case 0x66 : return('f');
    case 0x67 : return('g');
    case 0x68 : return('h');
    case 0x69 : return('i');
    case 0x6a : return('j');
    case 0x6b : return('k');
    case 0x6c : return('l');
    case 0x6d : return('m');
    case 0x6e : return('n');
    case 0x6f : return('o');
    case 0x70 : return('p');
    case 0x71 : return('q');
    case 0x72 : return('r');
    case 0x73 : return('s');
    case 0x74 : return('t');
    case 0x75 : return('u');
    case 0x76 : return('v');
    case 0x77 : return('w');
    case 0x78 : return('x');
    case 0x79 : return('y');
    case 0x7a : return('z');
    case 0x7b : return('{');
    case 0x7c : return('|');
    case 0x7d : return('}');
    case 0x7e : return('~');
    default   : return(0);
  }
}

/*********************************************************************/
/*                                                                   */
/*  This program takes an integer as a parameter.  The integer       */
/*  should have a representation of a character.  An integer         */
/*  is returned, containing the representation of that character     */
/*  in the ASCII character set.                                      */
/*                                                                   */
/*********************************************************************/
static int tasc(int loc)
{
  switch (loc)
  {
    case '\t' : return (0x09);
    case '\n' : return (0x0a);
    case '\f' : return (0x0c);
    case '\r' : return (0x0d);
    case ' '  : return (0x20);
    case '!'  : return (0x21);
    case '\"' : return (0x22);
    case '#'  : return (0x23);
    case '$'  : return (0x24);
    case '%'  : return (0x25);
    case '&'  : return (0x26);
    case '\'' : return (0x27);
    case '('  : return (0x28);
    case ')'  : return (0x29);
    case '*'  : return (0x2a);
    case '+'  : return (0x2b);
    case ','  : return (0x2c);
    case '-'  : return (0x2d);
    case '.'  : return (0x2e);
    case '/'  : return (0x2f);
    case '0'  : return (0x30);
    case '1'  : return (0x31);
    case '2'  : return (0x32);
    case '3'  : return (0x33);
    case '4'  : return (0x34);
    case '5'  : return (0x35);
    case '6'  : return (0x36);
    case '7'  : return (0x37);
    case '8'  : return (0x38);
    case '9'  : return (0x39);
    case ':'  : return (0x3a);
    case ';'  : return (0x3b);
    case '<'  : return (0x3c);
    case '='  : return (0x3d);
    case '>'  : return (0x3e);
    case '?'  : return (0x3f);
    case '@'  : return (0x40);
    case 'A'  : return (0x41);
    case 'B'  : return (0x42);
    case 'C'  : return (0x43);
    case 'D'  : return (0x44);
    case 'E'  : return (0x45);
    case 'F'  : return (0x46);
    case 'G'  : return (0x47);
    case 'H'  : return (0x48);
    case 'I'  : return (0x49);
    case 'J'  : return (0x4a);
    case 'K'  : return (0x4b);
    case 'L'  : return (0x4c);
    case 'M'  : return (0x4d);
    case 'N'  : return (0x4e);
    case 'O'  : return (0x4f);
    case 'P'  : return (0x50);
    case 'Q'  : return (0x51);
    case 'R'  : return (0x52);
    case 'S'  : return (0x53);
    case 'T'  : return (0x54);
    case 'U'  : return (0x55);
    case 'V'  : return (0x56);
    case 'W'  : return (0x57);
    case 'X'  : return (0x58);
    case 'Y'  : return (0x59);
    case 'Z'  : return (0x5a);
    case '['  : return (0x5b);
    case '\\' : return (0x5c);
    case ']'  : return (0x5d);
    case '^'  : return (0x5e);
    case '_'  : return (0x5f);
    case '`'  : return (0x60);
    case 'a'  : return (0x61);
    case 'b'  : return (0x62);
    case 'c'  : return (0x63);
    case 'd'  : return (0x64);
    case 'e'  : return (0x65);
    case 'f'  : return (0x66);
    case 'g'  : return (0x67);
    case 'h'  : return (0x68);
    case 'i'  : return (0x69);
    case 'j'  : return (0x6a);
    case 'k'  : return (0x6b);
    case 'l'  : return (0x6c);
    case 'm'  : return (0x6d);
    case 'n'  : return (0x6e);
    case 'o'  : return (0x6f);
    case 'p'  : return (0x70);
    case 'q'  : return (0x71);
    case 'r'  : return (0x72);
    case 's'  : return (0x73);
    case 't'  : return (0x74);
    case 'u'  : return (0x75);
    case 'v'  : return (0x76);
    case 'w'  : return (0x77);
    case 'x'  : return (0x78);
    case 'y'  : return (0x79);
    case 'z'  : return (0x7a);
    case '{'  : return (0x7b);
    case '|'  : return (0x7c);
    case '}'  : return (0x7d);
    case '~'  : return (0x7e);
    default   : return(0);
  }
}
