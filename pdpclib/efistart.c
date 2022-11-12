/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*  startup code for EFI applications                                */
/*                                                                   */
/*********************************************************************/

typedef struct {
    char junk1[8];
    int (*print_func)(void *x, void *y);
} EFI_SIMPLE_TEXT;

typedef struct {
    char junk1[32];
    void *stdout_handle;
    EFI_SIMPLE_TEXT *simple;
} EFI_SYSTEM;

static char onechar[4];

static int __main(void);

int main(int junk, EFI_SYSTEM *system)
{
    onechar[0] = 'X';
    system->simple->print_func(system->simple, onechar);
    onechar[0] = '\r';
    system->simple->print_func(system->simple, onechar);
    onechar[0] = '\n';
    system->simple->print_func(system->simple, onechar);
    for (;;) ;
    return (5);
}

static int __main(void)
{
    return (3);
}
