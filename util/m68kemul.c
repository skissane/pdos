/*********************************************************************/
/*                                                                   */
/*  This Program Written By Paul Edwards.                            */
/*  Released to the public domain.                                   */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  m68kemul - emulate Linux on a Motorola 68000                     */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <exeload.h>

static unsigned int dregs[8];
static unsigned int aregs[8];
static unsigned char *ip;
static unsigned char stack[4096];
static unsigned char *sp = stack + sizeof stack - 4;

static void doemul(void);

int main(void)
{
    unsigned char *entry_point = NULL;
    unsigned char *load_point = NULL;
    int rc;

    rc = exeloadDoload(&entry_point, "m68ktest.exe", &load_point);
    if (rc != 0)
    {
        printf("failed to load\n");
        return (EXIT_FAILURE);
    }
    printf("entry point is %p\n", entry_point);
    printf("first byte %x\n", *entry_point);
#if 0
{
unsigned char *ptr = entry_point;
int x;
        for (x = -4; x < 70; x++)
        {
            printf("%p %x\n", ptr + x, ptr[x]);
        }
}
#endif
    ip = entry_point;
    doemul();
    return (0);
}

static void doemul(void)
{
while (1)
{
    printf("instruction is %x\n", *ip);
    /* 2f 38 references a 2-byte address. 3c a 4-byte value */
    if ((*ip == 0x2f) && (ip[1] == 0x3c))
    {
        sp -= 4;
        *(unsigned int *)sp = ((unsigned int)ip[2] << 24)
                              | ((unsigned int)ip[3] << 16)
                              | ((unsigned int)ip[4] << 8)
                              | ip[5];
        ip += 6;
    }
    else if ((*ip == 0x2f) && (ip[1] == 0x08))
    {
        sp -= 4;
        *(unsigned int *)sp = aregs[0];
        printf("saving %x\n", *(unsigned int *)sp);
        ip += 2;
    }
    else if ((*ip == 0x24) && (ip[1] == 0x17))
    {
        dregs[2] = *(unsigned int *)sp;
        printf("retrieving %x\n", dregs[2]);
        ip += 2;
    }
    else if ((*ip == 0x41) && (ip[1] == 0xfa))
    {
        aregs[0] = (unsigned int)
                   (ip + (ip[2] << 8) + ip[3] + 2);
        printf("referencing1 %p\n", aregs[0]);
        ip += 4;
    }
    else if (*ip == 0x41)
    {
        aregs[0] = ((unsigned int)ip[2] << 24)
                   | ((unsigned int)ip[3] << 16)
                   | ((unsigned int)ip[4] << 8)
                   | ip[5];
        printf("referencing2 %p\n", aregs[0]);
        ip += 6;
    }
    else if (*ip == 0x70)
    {
        dregs[0] = ip[1];
        ip += 2;
    }
    else if (*ip == 0x72)
    {
        dregs[1] = ip[1];
        ip += 2;
    }
    else if (*ip == 0x76)
    {
        dregs[3] = ip[1];
        ip += 2;
    }
/*    else if (*ip == 0x20)
    {
        dregs[0] = *(unsigned short *)(ip + 2);
        ip += 4;
    } */
    else if ((ip[0] == 0x4e) && (ip[1] == 0x40))
    {
        unsigned char *ptr;
        int x;

        printf("got trap %x!\n", dregs[0]);
        printf("parms are %x %x %x\n",
               dregs[1],
               dregs[2],
               dregs[3]
              );
        ptr = (unsigned char *)dregs[2];
        if (dregs[1] == 1)
        {
            printf("%.*s\n",
                   dregs[3],
                   dregs[2]);
        }
#if 0
        for (x = -50; x < 500; x++)
        {
            printf("%p %x\n", ptr + x, ptr[x]);
        }
#endif
        ip += 2;
    }
    else if ((ip[0] == 0x4e) && (ip[1] == 0xfa))
    {
        printf("infinite loop detected - exiting\n");
        exit(EXIT_SUCCESS);
    }
    else
    {
        printf("unknown instruction %x\n", *ip);
        exit(EXIT_FAILURE);
    }
}
}
