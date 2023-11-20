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


#if 0

For example, do this:

qemu-system-i386 -drive file=pdos.vhd,index=0,media=disk,format=raw -serial tcp:news.eternal-september.org:119

Then do:

pdpnntp -uusername -ppassword com1:

#endif


#include <stdio.h>
#include <stdlib.h>

static FILE *comm;
static char buf[1000];

static char user[50] = "";
static char password[50] = "";
static char group[100] = "";
static char fnm[FILENAME_MAX] = "";
static char readnum[50] = "";
static int writemsg = 0;
static int list = 1;

#define OPTINT 1
#define OPTSTR 2
#define OPTBOOL 3
#define OPTLONG 4
#define OPTFLOAT 5
typedef struct {char *sw;
               int opttyp;
               void *var;} opt_t;
static int getopts(int argc, char **argv, opt_t opttable[]);

opt_t opttable[] =
{
    { "u", OPTSTR, user },
    { "p", OPTSTR, password },
    { "g", OPTSTR, group },
    { "r", OPTSTR, readnum },
    { "w", OPTBOOL, &writemsg },
    { "f", OPTSTR, fnm },
    { NULL, 0, NULL }
};

static int optup;


static char *getline(FILE *comm, char *buf, size_t szbuf);
static char *put(FILE *comm, char *buf);
static char *putline(FILE *comm, char *buf);
static int fasc(int asc);
static int tasc(int loc);

int main(int argc, char **argv)
{
    optup = getopts(argc,argv,opttable);
    /* if we have exhausted all arguments, positional parameter
       must not exist */
    if (optup == argc)
    {
        printf("usage: pdpnntp [-u<user>] [-p<password>] <comm channel>\n");
        return (EXIT_FAILURE);
    }
    
    if ((strcmp(readnum, "") != 0) && writemsg)
    {
        printf("can't use read and write at same time\n");
        return (EXIT_FAILURE);
    }
    
    if ((strcmp(readnum, "") != 0) || writemsg)
    {
        if (strcmp(fnm, "") == 0)
        {
            printf("read and write both require an associated filename\n");
            return (EXIT_FAILURE);
        }
    }
    if (strcmp(readnum, "") != 0)
    {
        if (strcmp(group, "") == 0)
        {
            printf("read requires a relevant group\n");
            return (EXIT_FAILURE);
        }
    }

#ifdef __MVS__
    comm = fopen(*(argv + optup), "rb");
#else
    comm = fopen(*(argv + optup), "r+b");
#endif
    if (comm == NULL)
    {
        printf("can't open %s\n", *(argv + optup));
        return (EXIT_FAILURE);
    }

    setvbuf(comm, NULL, _IONBF, 0);

    getline(comm, buf, sizeof buf); 
    printf("%s\n", buf);

#ifndef __MVS__
    if (list)
    {
        fseek(comm, 0, SEEK_CUR);

        putline(comm, "LIST");

        fseek(comm, 0, SEEK_CUR);

        while (1)
        {
            getline(comm, buf, sizeof buf);
            if (buf[0] == '.') break;
            printf("%s\n", buf);
        }
    }
#endif
    if (strcmp(user, "") != 0)
    {
        printf("putting user\n");
        fseek(comm, 0, SEEK_CUR);
        put(comm, "AUTHINFO USER ");
        putline(comm, user);
        fseek(comm, 0, SEEK_CUR);
        getline(comm, buf, sizeof buf);
        printf("%s\n", buf);

        if (strcmp(password, "") != 0)
        {
            printf("putting password\n");
            fseek(comm, 0, SEEK_CUR);
            put(comm, "AUTHINFO PASS ");
            putline(comm, password);
            fseek(comm, 0, SEEK_CUR);
            getline(comm, buf, sizeof buf);
            printf("%s\n", buf);
        }
    }

    if (strcmp(group, "") != 0)
    {
        printf("selecting group\n");
        fseek(comm, 0, SEEK_CUR);
        put(comm, "GROUP ");
        putline(comm, group);
        fseek(comm, 0, SEEK_CUR);
        getline(comm, buf, sizeof buf);
        printf("%s\n", buf);
    }

    if (strcmp(readnum, "") != 0)
    {
        printf("reading article\n");
        fseek(comm, 0, SEEK_CUR);
        put(comm, "ARTICLE ");
        putline(comm, readnum);
        fseek(comm, 0, SEEK_CUR);
        getline(comm, buf, sizeof buf);
        printf("%s\n", buf);
        if (strncmp(buf, "423", 3) != 0)
        {
            FILE *fq;

            fq = fopen(fnm, "w");
            if (fq == NULL)
            {
                printf("failed to open %s for writing\n", fnm);
            }
            else
            {
                while (1)
                {
                    getline(comm, buf, sizeof buf);
                    printf("%s\n", buf);
                    if (strcmp(buf, ".") == 0) break;
                    if (strcmp(buf, "..") == 0)
                    {
                        /* is this correct or do all beginning
                           dots get doubled? */
                        buf[1] = '\0';
                    }
                    fprintf(fq, "%s\n", buf);
                }
                fclose(fq);
            }
        }
    }

    /* When writing a message, include the newsgroup in
       the file itself - no need to select the newsgroup
       as a parameter.
       Subject and From are the only things required on
       top of that */
       
#if 0

So this (which is easy to edit - just change the subject):

Subject: test 2, please ignore
Newsgroups: eternal-september.test
From: Joe Bloggs <joe@bloggs.comm>

TEST NUMBER 2
line nr two
Do not read this.
Go away.


Gets converted (by INN) into:

Path: eternal-september.org!news.eternal-september.org!.POSTED!not-for-mail
From: Joe Bloggs <joe@bloggs.comm>
Newsgroups: eternal-september.test
Subject: test 2, please ignore
Date: Mon, 20 Nov 2023 13:04:48 -0000 (UTC)
Organization: A noiseless patient Spider
Lines: 4
Message-ID: <ujflhf$aoii$1@dont-email.me>
Injection-Date: Mon, 20 Nov 2023 13:04:48 -0000 (UTC)
Injection-Info: dont-email.me; posting-host="d768e5f4df19c52b9564161c184d27b3";
	logging-data="352850"; mail-complaints-to="abuse@eternal-september.org";	posting-account="U2FsdGVkX18Z8DNUHrp7zidBcJ9jtyd24MEwUx1VE1A="
Cancel-Lock: sha1:suX8JLIyn9VU6eVGq2gb7U3yty0=
Xref: news.eternal-september.org eternal-september.test:11624

TEST NUMBER 2
line nr two
Do not read this.
Go away.


#endif

/* This may be useful one day:
strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S +0000", gmtime(&tt));
*/

    if (writemsg)
    {
        printf("posting article\n");
        fseek(comm, 0, SEEK_CUR);
        putline(comm, "POST");
        fseek(comm, 0, SEEK_CUR);
        getline(comm, buf, sizeof buf);
        printf("%s\n", buf);
        /* 340 Ok, recommended Message-ID <ujfhd5$a4jb$1@dont-email.me>
        I used: Message-ID: <1.joe@bloggs.comm>
        exit(0); */
        if (strncmp(buf, "340", 3) == 0)
        {
            FILE *fp;
            char *p;

            fp = fopen(fnm, "r");
            if (fp == NULL)
            {
                printf("failed to open %s for reading\n", fnm);
            }
            else
            {
                while (1)
                {
                    if (fgets(buf, sizeof buf, fp) == NULL)
                    {
                        printf("unexpected EOF\n");
                        exit(EXIT_FAILURE);
                    }
                    p = strchr(buf, '\n');
                    if (p != NULL)
                    {
                        *p = '\0';
                    }
                    putline(comm, buf);
                    if (strcmp(p, "") == 0) break;
                }
                while (1)
                {
                    if (fgets(buf, sizeof buf, fp) == NULL)
                    {
                        putline(comm, ".");
                        break;
                    }
                    p = strchr(buf, '\n');
                    if (p != NULL)
                    {
                        *p = '\0';
                    }
                    if (strcmp(buf, ".") == 0)
                    {
                        put(comm, ".");
                    }
                    putline(comm, buf);
                }
                fclose(fp);
            }
            printf("finished sending from here\n");
            fseek(comm, 0, SEEK_CUR);
            getline(comm, buf, sizeof buf);
            printf("%s\n", buf);
            /* 240 Article received <ujfkdm$ajes$1@dont-email.me> */
        }
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

/*********************************************************************/
/*                                                                   */
/*  getopts - scan the command line for switches.                    */
/*                                                                   */
/*  This program takes the following parameters:                     */
/*                                                                   */
/*  1) argc (which was given to main)                                */
/*  2) argv (which was given to main)                                */
/*  3) Array of options                                              */
/*                                                                   */
/*  Returns the number of the argument that is next to be processed  */
/*  that wasn't recognised as an option.                             */
/*  Example of use:                                                  */
/*                                                                   */
/*  #include <getopts.h>                                             */
/*  int baud = 2400;                                                 */
/*  char fon[13] = "telix.fon";                                      */
/*  opt_t opttable[] =                                               */
/*  {                                                                */
/*    { "b", OPTINT, &baud },                                        */
/*    { "f", OPTSTR, fon },                                          */
/*    { NULL, 0, NULL }                                              */
/*  };                                                               */
/*  optup = getopts(argc,argv,opttable);                             */
/*                                                                   */
/*  The OPTINT means that an integer is being supplied.  OPTSTR      */
/*  means a string (with no check for overflow).  Also there is      */
/*  OPTBOOL which means it is a switch that is being passed, and an  */
/*  OPTLONG to specify a long.  Also OPTFLOAT for float.             */
/*                                                                   */
/*  This program was inspired by a description of a getargs function */
/*  written by Dr Dobbs Small-C Handbook.  Naturally I didn't get    */
/*  to see the code, otherwise I wouldn't be writing this!           */
/*                                                                   */
/*  This program is dedicated to the public domain.  It would be     */
/*  nice but not necessary if you gave me credit for it.  I would    */
/*  like to thank the members of the International C Conference      */
/*  (in Fidonet) for the help they gave me in writing this.          */
/*                                                                   */
/*  Written 16-Feb-1990.                                             */
/*                                                                   */
/*********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

static int getopts(int argc, char **argv, opt_t opttable[])
{
    int i, j;

    argv++;
    argc--;
    for (i = 1; i <= argc; i++)
    {
        if ((*(*argv) != '-') && (*(*argv) != '/'))
            return (i);
        for (j = 0; opttable[j].sw != NULL; j++)
        {
            if (strncmp(*argv + 1, opttable[j].sw,
                        strlen(opttable[j].sw)) == 0)
            {
                switch ((int)opttable[j].opttyp)
                {
                case OPTINT:
                    *((int *)opttable[j].var) =
                        (int)strtol(*argv + 1 + strlen(opttable[j].sw), NULL, 10);
                    if (errno == ERANGE)
                        return (i);
                    break;
                case OPTSTR:
                    strcpy((char *)opttable[j].var,
                           *argv + 1 + strlen((char *)opttable[j].sw));
                    break;
                case OPTBOOL:
                    *((int *)opttable[j].var) = 1;
                    break;
                case OPTLONG:
                    *((long *)opttable[j].var) =
                        strtol(*argv + 1 + strlen(opttable[j].sw), NULL, 10);
                    if (errno == ERANGE)
                        return (i);
                    break;
#ifdef DOFLOAT
                case OPTFLOAT:
                    *((float *)opttable[j].var) =
                        (float)strtod(*argv + 1 + strlen(opttable[j].sw), NULL);
                    break;
#endif
                }
                break;
            }
        }
        argv++;
    }
    return (i);
}
