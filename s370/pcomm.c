/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  pcomm - command processor for pdos                               */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

static char buf[200];
static size_t len;
static char drive[7] = "PDOS00";
static char cwd[65];
static char prompt[50] = ">";
static int singleCommand = 0;
static int primary = 0;
static int term = 0;
static int showrc = 0;
static int echo = 1;

static void parseArgs(int argc, char **argv);
static void readAutoExec(void);
static void processInput(void);
static void putPrompt(void);
static void dotype(char *file);
static void docopy(char *p);
static void dofill(char *p);
static void dodir(char *pattern);
static void dohelp(void);
static void changedir(char *to);
static void changedisk(int drive);
static int ins_strcmp(char *one, char *two);
static int ins_strncmp(char *one, char *two, size_t len);

int main(int argc, char **argv)
{
    parseArgs(argc, argv);
    if (singleCommand)
    {
        processInput();
        return (0);
    }
    if (primary)
    {
        printf("welcome to pcomm\n");
        readAutoExec();
    }
    else
    {
        printf("welcome to pcomm - exit to return\n");
    }
    while (!term)
    {
        putPrompt();
        fgets(buf, sizeof buf, stdin);

        processInput();
    }
    printf("thankyou for using pcomm!\n");
    return (0);
}

static void parseArgs(int argc, char **argv)
{
    int x;
    
    if (argc > 1)
    {
        if ((argv[1][0] == '-') || (argv[1][0] == '/'))
        {
            if ((argv[1][1] == 'C') || (argv[1][1] == 'c'))
            {
                singleCommand = 1;
            }            
            if ((argv[1][1] == 'P') || (argv[1][1] == 'p'))
            {
                primary = 1;
            }            
        }
    }
    if (singleCommand)
    {
        strcpy(buf, "");
        for (x = 2; x < argc; x++)
        {
            strcat(buf, *(argv + x));
            strcat(buf, " ");
        }
        len = strlen(buf);
        if (len > 0)
        {
            buf[len - 1] = '\0';
        }
    }
    return;
}

static void readAutoExec(void)
{
    FILE *fp;

    fp = fopen("AUTOEXEC.BAT", "r");
    if (fp != NULL)
    {
        while (fgets(buf, sizeof buf, fp) != NULL)
        {
            processInput();
            if (term) break;
        }
        fclose(fp);
    }
    return;
}

static void processInput(void)
{
    char *p;
    int rc;
    char fnm[FILENAME_MAX];
    FILE *fp;    

    if (echo)
    {
        printf("%s", buf);
    }
    len = strlen(buf);
    if ((len > 0) && (buf[len - 1] == '\n'))
    {
        len--;
        buf[len] = '\0';
    }
    p = strchr(buf, ' ');
    if (p != NULL)
    {
        *p++ = '\0';
    }
    else
    {
        p = buf + len;
    }
    len -= (size_t)(p - buf);
    if (ins_strcmp(buf, "exit") == 0)
    {
#ifdef CONTINUOUS_LOOP
        primary = 0;
#endif
        if (1) /* (!primary) */
        {
            term = 1;
        }
    }
    else if (ins_strcmp(buf, "type") == 0)
    {
        dotype(p);
    }
    else if (ins_strcmp(buf, "copy") == 0)
    {
        docopy(p);
    }
    else if (ins_strcmp(buf, "fill") == 0)
    {
        dofill(p);
    }
/* for now, let PDOS handle this */
/*    else if (ins_strcmp(buf, "dir") == 0)
    {
        dodir(p);
    } */
    else if (ins_strcmp(buf, "echo") == 0)
    {
        if (ins_strcmp(p, "off") == 0)
        {
            echo = 0;
        }
        else if (ins_strcmp(p, "on") == 0)
        {
            echo = 1;
        }
        else
        {
            printf("%s\n", p);
        }
    }
    else if (p == buf)
    {
        /* do nothing if blank line */
    }
    else if (ins_strcmp(buf, "rem") == 0)
    {
        /* ignore comments */
    }
    else if (ins_strcmp(buf, "cd") == 0)
    {
        changedir(p);
    }
    else if (ins_strncmp(buf, "cd.", 3) == 0)
    {
        changedir(buf + 2);
    }
    else if (ins_strncmp(buf, "cd\\", 3) == 0)
    {
        changedir(buf + 2);
    }
    else if (ins_strcmp(buf, "reboot") == 0)
    {
        /* PosReboot(); */
    }
    else if (ins_strcmp(buf, "showrc") == 0)
    {
        showrc = (showrc == 0);
    }
    else if (ins_strcmp(buf, "help") == 0)
    {
        dohelp();
    }
#if 0
    else if ((strlen(buf) == 2) && (buf[1] == ':'))
    {
        changedisk(buf[0]);
    }
#endif
    else
    {
        /* see if batch file exists */
        strcpy(fnm, buf);
        strcat(fnm, ".BAT");
        fp = fopen(fnm, "r");
        if (fp != NULL)
        {
            while (fgets(buf, sizeof buf, fp) != NULL)
            {
                /* recursive call */
                processInput();
                if (term) break;
            }
            fclose(fp);
            return;
        }
        
        /* restore parameter if there is one*/
        if (*p != '\0')
        {
            p--;
            *p = ' ';
        }
        /* printf("pcomm is calling %s\n", buf); */
        rc = system(buf);
        if (showrc)
        {
            printf("rc from program is %d\n", rc);
        }
    }
    return;
}

static void putPrompt(void)
{
    printf("\n%s:\\%s%s", drive, cwd, prompt);
    fflush(stdout);
    return;
}

static void dotype(char *file)
{
    FILE *fp;
    
    fp = fopen(file, "r");
    if (fp != NULL)
    {
        while (fgets(buf, sizeof buf, fp) != NULL)
        {
            fputs(buf, stdout);
        }
        fclose(fp);
    }
    else
    {
       printf("file not found: %s\n", file);
    }
    return;
}

static void docopy(char *p)
{
                FILE *fp;
                FILE *fq;
                char *q;

                q = strchr(p, ' ');
                if (q == NULL)
                {
                    printf("two files needed\n");
                }
                else
                {
                    *q = '\0';
                    q++;
                    fp = fopen(p, "rb");
                    if (fp == NULL)
                    {
                        printf("failed to open input file\n");
                    }
                    else
                    {
                        fq = fopen(q, "wb");
                        if (fq == NULL)
                        {
                            printf("failed to open output file\n");
                            fclose(fp);
                        }
                        else
                        {
                            int c;

                            while ((c = fgetc(fp)) != EOF)
                            {
                                fputc(c, fq);
                            }
                            fclose(fp);
                            fclose(fq);
                        }
                    }
                }
    return;
}

static void dofill(char *p)
{
    FILE *fq;
    char *q;
    unsigned long max = 0;
    int infinite = 0;

    if (*p == '\0')
    {
        printf("enter filename and number of bytes to generate\n");
        printf("leave number of bytes blank or zero for infinity\n");
        return;
    }
    q = strchr(p, ' ');
    if (q != NULL)
    {
        max = strtoul(q, NULL, 0);
    }
    if (max == 0) infinite = 1;
    fq = fopen(p, "wb");
    if (fq == NULL)
    {
        printf("failed to open %s for output\n", p);
        return;
    }
    while ((max > 0) || infinite)
    {
        putc(0x00, fq);
        if (ferror(fq))
        {
            printf("write error\n");
            break;
        }
        max--;
    }
    fclose(fq);
    return;
}

static void dodir(char *pattern)
{
    return;
}

static void dohelp(void)
{
    printf("The following commands are available:\n\n");
    printf("HELP - display this help\n");
    printf("TYPE - display contents of a file\n");
    printf("DUMPBLK - dump a block on disk\n");
    printf("ZAPBLK - zap a block on disk\n");
    printf("DIR - display directory\n");
    printf("SHOWRC - display return code from programs\n");
    printf("EXIT - exit operating system\n");
    printf("ECHO - display provided text\n");
    printf("anything else will be assumed to be a .EXE program\n");
    return;
}

static void changedir(char *to)
{
    /* PosChangeDir(to); */
    return;
}

static void changedisk(int drive)
{
    /* PosSelectDisk(toupper(drive) - 'A'); */
    return;
}

static int ins_strcmp(char *one, char *two)
{
    while (toupper(*one) == toupper(*two))
    {
        if (*one == '\0')
        {
            return (0);
        }
        one++;
        two++;
    }
    if (toupper(*one) < toupper(*two))
    {
        return (-1);
    }
    return (1);
}

static int ins_strncmp(char *one, char *two, size_t len)
{
    size_t x = 0;
    
    if (len == 0) return (0);
    while ((x < len) && (toupper(*one) == toupper(*two)))
    {
        if (*one == '\0')
        {
            return (0);
        }
        one++;
        two++;
        x++;
    }
    if (x == len) return (0);
    return (toupper(*one) - toupper(*two));
}
