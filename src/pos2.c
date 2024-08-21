/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  pos - implementation of pos.h                                    */
/*                                                                   */
/*********************************************************************/

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "pos.h"


/* This is not an interrupt - we instead do multiple interrupts to create a
   full directory path */

int PosMakeFullDir(const char *dname)
{
    char name[FILENAME_MAX];
    char *upto;
    char *p;
    char *q;
    int slash;

    strcpy(name, dname);
    upto = name;
    while (1)
    {
        slash = 0;
        p = strchr(upto, '/');
        q = strchr(upto, '\\');
        if ((p != NULL) && (q != NULL))
        {
            if (p < q)
            {
                q = p;
                slash = 1;
            }
        }
        else if (p != NULL)
        {
            q = p;
            slash = 1;
        }
        if (q != NULL)
        {
            *q = '\0';
        }
        PosMakeDir(name);
        if (q == NULL)
        {
            break;
        }
        upto = q + 1;
        if (slash)
        {
            *q = '/';
        }
        else
        {
            *q = '\\';
        }
    }
    return (0);
}
