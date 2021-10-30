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
        printf("you entered X%sX\n", buf);
        if (strcmp(buf, "exit") == 0) break;
    }
    return (0);
}
