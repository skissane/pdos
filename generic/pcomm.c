/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  pcomm - command processor                                        */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include <pos.h>

static char buf[200];

static void dofill(char *p);
static void dofzap(char *p);

int main(void)
{
    char *p;
    char *q;

    printf("welcome to pcomm\n");
    printf("type help for help\n");
    while (1)
    {
        printf("\nenter a command\n");
        fgets(buf, sizeof buf, stdin);
        p = strchr(buf, '\n');
        if (p != NULL) *p = '\0';
        if (strcmp(buf, "exit") == 0)
        {
            break;
        }
        else if (strcmp(buf, "help") == 0)
        {
            printf("commands that might be available are:\n");
            printf("exit, type, dir, md, rd, cd, date, time, del, copy\n");
            printf("fill, fzap\n");
        }
        else if (strncmp(buf, "type", 4) == 0)
        {
            p = strchr(buf, ' ');
            if (p != NULL)
            {
                FILE *fp;

                p++;
                fp = fopen(p, "r");
                if (fp != NULL)
                {
                    int c;

                    while ((c = fgetc(fp)) != EOF)
                    {
                        fputc(c, stdout);
                    }
                    fclose(fp);
                }
                else
                {
                    printf("file not found\n");
                }
            }
        }
        else if (strncmp(buf, "fill", 4) == 0)
        {
            dofill(buf + 4);
        }
        else if (strncmp(buf, "fzap", 4) == 0)
        {
            dofzap(buf + 4);
        }
        else if (strncmp(buf, "copy", 4) == 0)
        {
            p = strchr(buf, ' ');
            if (p != NULL)
            {
                FILE *fp;
                FILE *fq;

                p++;
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
            }
        }
#if HAVE_DIR
        else if (strncmp(buf, "dir", 3) == 0)
        {
            DTA *dta;
            int ret;

            dta = PosGetDTA();
            if (dta == NULL)
            {
                printf("dir unavailable\n");
                continue;
            }
            ret = PosFindFirst("*.*", 0x10);
            while (ret == 0)
            {
                if (dta->lfn[0] != '\0')
                {
                    printf("%s\n", dta->lfn);
                }
                else
                {
                    printf("%s\n", dta->file_name);
                }
                ret = PosFindNext();
            }
        }
#endif
        else if ((strncmp(buf, "date", 4) == 0)
                 || (strncmp(buf, "time", 4) == 0))
        {
            time_t timer;

            time(&timer);
            printf("%s", ctime(&timer));
        }
        else if (strncmp(buf, "cd", 2) == 0)
        {
            p = strchr(buf, ' ');
            if (p != NULL)
            {
                PosChangeDir(p + 1);
            }
        }
        else if (strncmp(buf, "md", 2) == 0)
        {
            p = strchr(buf, ' ');
            if (p != NULL)
            {
                PosMakeDir(p + 1);
            }
        }
        else if (strncmp(buf, "rd", 2) == 0)
        {
            p = strchr(buf, ' ');
            if (p != NULL)
            {
                PosRemoveDir(p + 1);
            }
        }
        else if (strncmp(buf, "del", 3) == 0)
        {
            p = strchr(buf, ' ');
            if (p != NULL)
            {
                remove(p + 1);
            }
        }
        else
        {
            POSEXEC_PARMBLOCK pb = { 0 };
            char progname[50];

            strncpy(progname, buf, sizeof progname);
            progname[49] = '\0';
            p = strchr(progname, ' ');
            if (p != NULL)
            {
                *p = '\0';
            }
            pb.cmdtail = (unsigned char *)buf;
            if (strchr(progname, ':') == NULL)
            {
                strcat(progname, ".exe");
            }
            PosExec(progname, &pb);
            /* printf("unknown command\n"); */
        }
    }
    return (0);
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
    p++;
    q = strchr(p, ' ');
    if (q != NULL)
    {
        *q++ = '\0';
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

static void dofzap(char *p)
{
    FILE *fu;
    long offs;
    int ok = 0;
    int val;
    char *fnm;

    if (*p != '\0')
    {
        p++;
        fnm = p;
        p = strchr(p, ' ');
        if (p != NULL)
        {
            *p = '\0';
            p++;
            offs = strtol(p, NULL, 0);
            p = strchr(p, ' ');
            if (p != NULL)
            {
                p++;
                val = (int)strtol(p, NULL, 0);
                ok = 1;
            }
        }
    }
    if (!ok)
    {
        printf("usage: fzap <fnm> <offs> <val>\n");
        return;
    }
    fu = fopen(fnm, "r+b");
    if (fu == NULL)
    {
        printf("unable to open file %s\n", fnm);
        return;
    }
    fseek(fu, offs, SEEK_SET);
    fputc(val, fu);
    fclose(fu);
    return;
}
