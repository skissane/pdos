/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  extra - extra funtions for Rust                                  */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>

#define lang_start_internal _ZN3std2rt19lang_start_internal17hfefa8592b280fcd4E
#define stdio_print _ZN3std2io5stdio6_print17h06e585553ed71412E

void lang_start_internal(void (**x)(void))
{
    (*x)();
    return;
}

void stdio_print(int x, long *y, int z, char *str)
{
    printf("%.*s", y[1], str);
    return;
}

#ifdef NEED_UNDMAIN
void __main(void)
{
    return;
}
#endif
