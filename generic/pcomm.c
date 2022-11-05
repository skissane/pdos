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

#include <pos.h>

static char buf[200];

int main(void)
{
    char *p;
    char *q;

    printf("welcome to pcomm\n");
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
            printf("unknown command\n");
        }
    }
    return (0);
}
