/*********************************************************************/
/*                                                                   */
/*  This Program Written By Paul Edwards.                            */
/*  Released to the public domain.                                   */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  fc - compare two files - binary only                             */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    FILE *fp1;
    FILE *fp2;
    int c1;
    int c2;
    long n = 0;

    if (argc <= 3)
    {
        printf("usage: fc /b file1 file2\n");
        printf("does a binary compare of two files\n");
        return (EXIT_FAILURE);
    }
    
    fp1 = fopen(argv[2], "rb");
    if (fp1 == NULL)
    {
        printf("cannot open %s for reading\n", argv[2]);
        return (EXIT_FAILURE);
    }
    fp2 = fopen(argv[3], "rb");
    if (fp2 == NULL)
    {
        printf("cannot open %s for reading\n", argv[3]);
	fclose(fp1);
        return (EXIT_FAILURE);
    }
    while (1)
    {
        c1 = getc(fp1);
	c2 = getc(fp2);
	if ((c1 == EOF) && (c2 != EOF))
	{
	    printf("premature end of file 1\n");
	    break;
        }
	if ((c1 != EOF) && (c2 == EOF))
	{
	    printf("premature end of file 2\n");
	    break;
        }
	if (c1 == EOF)
	{
	    break;
        }
	if (c1 != c2)
	{
	    printf("%08lX: %02X %02X\n", n, c1, c2);
        }
	n++;
    }
    
    fclose(fp1);
    fclose(fp2);
    return (EXIT_SUCCESS);
}
