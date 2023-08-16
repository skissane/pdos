/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  msdemo32 - demonstrate a 32-bit program dependent on msvcrt.dll  */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>

void mainCRTStartup(void)
{
    /* Initialization will already have happened in the
       msvcrt.dll. Here you can obtain arguments by calling
       __getmainargs if you want, but we're not using
       arguments so we'll skip that.
       
       You would then normally call main(), but we're skipping
       that too for demo purposes.
    */
       
    puts("hello, world 32\n");
       
    exit(0);

    return;
}
