/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  mkiplcrd - make an card deck containing the PDOS OS that can be  */
/*  IPLed                                                            */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* sapstart is expecting that 18432 bytes will have been loaded
   before it gets control (when dealing with cards). This is
   256 cards with usable data (72 bytes). We'll call the cards
   that load these cards the IPL4 CCW chain.

   In order to read 256 cards, ie 256 CCWs, you need 256/9 = 28.x = 29
   cards containing CCWs. We'll call the cards that load these cards
   the IPL3 CCW chain.

   In order to read those 29 cards you need 29/9 = 3.x = 4 cards.
   We'll call the card that loads these cards the IPL2 CCW chain.

   In order to read those 4 cards, you need 1 card, which is the
   only card loaded by the IPL1 CCW chain (which normally only
   has 2 CCWs).

   IPL2+3+4 are simply loaded consecutively (with allowance for
   the last card in each IPL chain not having a full set of CCWs),
   so no TIC is required, and the last CCW simply needs to stop chaining.
*/



static char ccw[24] =
    "\x00\x00\x00\x00" "\x00\x00\x00\x00" /* IPL PSW - not used */
    "\x02\x00\x00\x00" /* first CCW - read to address 0 */
    "\x20\x00\x48\x14" /* first CCW - ignore length errors, length 18452 */
    "\x00\x00\x00\x00" "\x00\x00\x00\x00" /* 2nd CCW - not used */
    ;

int main(int argc, char **argv)
{
    int i;
    FILE *fq;
    FILE *fp;
    char *buf;
    long imgsize;

    if (argc <= 1)
    {
        printf("usage: mkiplcrd <output file>\n");
        printf("e.g. mkiplcrd dev10004:\n");
        return (EXIT_FAILURE);
    }
    buf = malloc(18432);
    if (buf == NULL)
    {
        printf("couldn't allocate 18432 bytes\n");
        return (EXIT_FAILURE);
    }
    fq = fopen(*(argv + 1), "wb");
    if (fq == NULL)
    {
        printf("failed to open tape %s\n", *(argv + 1));
        return (EXIT_FAILURE);
    }
    fp = fopen("PDOS.IMG", "rb");
    if (fp == NULL)
    {
        printf("failed to open PDOS.IMG for reading\n");
        return (EXIT_FAILURE);
    }

    fseek(fp, 0, SEEK_END);
    imgsize = ftell(fp);
    printf("imgsize is %ld\n", imgsize);
    rewind(fp);
#if 0
    fwrite(ccw, 1, sizeof ccw, fq);

    i = fread(buf, 1, 18432, fp);
    /* set the IPL PSW to point to where it is located */
    *(int *)(buf + 4) = *(int *)(buf + 8 + 8192);
    fwrite(buf, 1, i + 4, fq);

    while ((i = fread(buf + 4, 1, 18452, fp)) > 0)
    {
        *(short *)buf = i + 4;
        fwrite(buf, 1, i + 4, fq);
    }
    fclose(fp);

    fp = fopen("CONFIG.SYS", "rb");
    if (fp == NULL)
    {
        printf("failed to open CONFIG.SYS for reading\n");
        return (EXIT_FAILURE);
    }

    i = fread(buf + 4, 1, 18452, fp);
    *(short *)buf = i + 4;
    fwrite(buf, 1, i + 4, fq);
    fclose(fp);

    fp = fopen("COMMAND.EXE", "rb");
    if (fp == NULL)
    {
        printf("failed to open COMMAND.EXE for reading\n");
        return (EXIT_FAILURE);
    }
    while ((i = fread(buf + 4, 1, 18452, fp)) > 0)
    {
        *(short *)buf = i + 4;
        fwrite(buf, 1, i + 4, fq);
    }
    fclose(fp);

    if (ferror(fq))
    {
        printf("write error\n");
        return (EXIT_FAILURE);
    }
    fclose(fq);
#endif
    printf("finished writing output file\n");
    return (EXIT_SUCCESS);
}
