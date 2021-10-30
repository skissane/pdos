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

#include <pos.h>

static char buf[200];

int main(void)
{
    char *p;

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
        else if (strncmp(buf, "dir", 3) == 0)
        {
            DTA *dta;
            int ret;

            dta = PosGetDTA();
            ret = PosFindFirst("*.*", 0x10);
            while (ret == 0)
            {
                printf("%s\n", dta->file_name);
                ret = PosFindNext();
            }
        }
        else
        {
            printf("unknown command\n");
        }
    }
    return (0);
}
