/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  deltree - delete an entire directory tree                        */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pos.h>

static char from[FILENAME_MAX];

static DTA *dta;

static int dolevel(void);

int main(int argc, char **argv)
{
    char *p;

    if (argc != 3)
    {
        printf("usage: deltree /y <dirname>\n");
        printf("delete an entire directory tree without prompting\n");
        return (EXIT_FAILURE);
    }
    if (strcmp(argv[1], "/y") != 0)
    {
        printf("you need to specify /y\n");
        return (EXIT_FAILURE);
    }
    dta = PosGetDTA();
    strcpy(from, *(argv + 2));
    dolevel();
    return (0);
}

static int dolevel(void)
{
    char oldfrom[FILENAME_MAX];
    DTA olddta;
    int ret;
    char *p;

    strcpy(oldfrom, from);

    p = from + strlen(from);
    strcpy(p, "\\");
    strcpy(p + 1, "*.*");
    ret = PosFindFirst(from, 0x10);
    *p = '\0';
    while (ret == 0)
    {
        if ((dta->attrib & FILE_ATTR_DIRECTORY) != 0)
        {
            strcat(from, "\\");
            strcat(from, dta->lfn);
            if (dta->lfn[0] == '\0')
            {
                strcat(from, dta->file_name);
            }
            if ((strcmp(dta->file_name, ".") == 0)
                || (strcmp(dta->file_name, "..") == 0))
            {
                /* ignore */
            }
            else
            {
                olddta = *dta;
                dolevel();
                *dta = olddta;
            }
            strcpy(from, oldfrom);
        }
        else
        {
            char in[FILENAME_MAX];

            strcpy(in, from);
            strcat(in, "\\");
            strcat(in, dta->lfn);
            if (dta->lfn[0] == '\0')
            {
                strcat(in, dta->file_name);
            }
            /* printf("%s\n", in); */
            remove(in);
        }
        ret = PosFindNext();
    }
    PosRemoveDir(from);
    return (0);
}
