/*********************************************************************/
/*                                                                   */
/*  This Program Written By Paul Edwards.                            */
/*  Released to the public domain.                                   */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  mfemul - emulate a mainframe                                     */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#define MAXPRGSZ (10 * 0x10000)

static unsigned char *base;
static unsigned char *p;
static int instr;

static int r0 = 0;
static int r1 = 0;
static int r2 = 0;
static int r3 = 0;
static int r4 = 0;
static int r5 = 0;
static int r6 = 0;
static int r7 = 0;
static int r8 = 0;
static int r9 = 0;
static int r10 = 0;
static int r11 = 0;
static int r12 = 0;
static int r13 = 0;
static int r14 = 0;
static int r15 = 0;
static int r16 = 0;

static void doemul(void);

int main(int argc, char **argv)
{
    FILE *fp;

    if (argc <= 1)
    {
        printf("usage: mfemul <COM>\n");
        printf("executes a mainframe .com file\n");
        return (EXIT_FAILURE);
    }
    
    fp = fopen(argv[1], "rb");
    if (fp == NULL)
    {
        printf("cannot open %s for reading\n", argv[1]);
        return (EXIT_FAILURE);
    }
    p = calloc(1, MAXPRGSZ + 0x10000);
    if (p == NULL)
    {
        printf("insufficient memory\n");
        return (EXIT_FAILURE);
    }
    fread(p + 0x10000, 1, MAXPRGSZ, fp);
    fclose(fp);
    base = p;
    p += 0x10000;
    doemul();
    return (EXIT_SUCCESS);
}

static void doemul(void)
{
    int x1;
    int x2;

    r13 = 0x100; /* save area */
    r15 = 0x10000; /* entry point */
    r14 = 0; /* branch to zero will terminate */
    while (1)
    {
        instr = *p;
        printf("instr is %02X\n", instr);
        if (instr == 0x07) /* bcr */
        {
            x1 = (p[1] >> 4) & 0x0f;
            x2 = p[1] & 0x0f;
            /* x1 is condition, x2 is register to branch to */
            if ((x1 == 0) || (x2 == 0))
            {
                printf("noop\n");
            }
            else
            {
                printf("unknown instruction %02X %02X\n", p[0], p[1]);
                exit(EXIT_FAILURE);
            }
            p += 2;
        }
        else if (instr == 0x05) /* balr */
        {
            x1 = (p[1] >> 4) & 0x0f;
            x2 = p[1] & 0x0f;
            if (x1 != 0)
            {
                /* regs[x1] = (p - base); */
            }
            exit(0);
        }
        else
        {
            printf("unknown instruction %02X\n", p[0]);
            exit(EXIT_FAILURE);
        }
    }
    return;
}
