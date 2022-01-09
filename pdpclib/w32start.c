/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  w32start - startup code for WIN32 using MSVCRT.DLL               */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

int __getmainargs(int *_Argc, char ***_Argv, char ***_Env, int _DoWildCard,
                  int *_StartInfo);
int main(int argc, char **argv);

/* This is the main entry point of a console mode executable */

/* consider adding this line so that relocatables are generated
   for some/most/all versions of "ld". The reason for this is
   that, at least with binutils 2.14a, there is code in there
   that will only generate relocatables if there are some
   exported symbols (or some other conditions) */
/*__declspec(dllexport)*/
void mainCRTStartup(void)
{
    int argc;
    char **argv;
    char **environ;
    int startinfo = 0;
    int status;

    __getmainargs(&argc, &argv, &environ, 1, &startinfo);

    status = main(argc, argv);

    exit(status);
}

void __main(void)
{
    return;
}
