/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*  startup code for EFI applications                                */
/*                                                                   */
/*********************************************************************/

typedef struct {
    char junk1[50];
    int (*print_func)(void *x);
} EFI_SIMPLE_TEXT;

typedef struct {
    char junk1[50];
    int stdout_handle;
    EFI_SIMPLE_TEXT *simple;
} EFI_SYSTEM;

static char onechar[3];

static int __main(void);

int main(int junk, EFI_SYSTEM *system)
{
    onechar[0] = 'X';
    system->simple->print_func(onechar);
    for (;;) ;
    return (5);
}

static int __main(void)
{
    return (3);
}
