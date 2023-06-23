/*********************************************************************/
/*                                                                   */
/*  This Program Written By Paul Edwards.                            */
/*  Released to the public domain.                                   */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  exe2bin - convert executable to binary                           */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <mz.h>

#define MAXPRGSZ 0x50000

static FILE *fp;
static FILE *fq;
static long base;
static char *p;
static size_t sz;
static char *codestart;
static unsigned short *rlstart;
static int nreloc;
static int x;
static Mz_hdr *hdr;
static unsigned short *fixseg;

int main(int argc, char **argv)
{
    if (argc <= 3)
    {
        printf("usage: exe2bin -base=<base> <inexe> <outbin>\n");
        printf("converts 16-bit dos executable into binary\n");
        printf("e.g. exe2bin -base=700 pload.tmp pload.com\n");
        printf("where base address is hex 700\n");
        printf("this is a flat address where the image will be loaded\n");
        return (EXIT_FAILURE);
    }

    base = strtol(&argv[1][6], NULL, 16);
    printf("base is hex %x\n", base);
    
    fp = fopen(argv[2], "rb");
    if (fp == NULL)
    {
        printf("cannot open %s for reading\n", argv[2]);
        return (EXIT_FAILURE);
    }
    
    fq = fopen(argv[3], "wb");
    if (fq == NULL)
    {
        printf("cannot open %s for writing\n", argv[3]);
        return (EXIT_FAILURE);
    }
    
    p = malloc(MAXPRGSZ);
    if (p == NULL)
    {
        printf("insufficient memory\n");
        return (EXIT_FAILURE);
    }

    sz = fread(p, 1, MAXPRGSZ, fp);
    fclose(fp);

    hdr = (Mz_hdr *)p;
    codestart = p + hdr->header_size * 16;
    rlstart = (unsigned short *)(p + hdr->reloc_tab_offset);
    nreloc = hdr->num_reloc_entries;
    
    printf("number of relocations is %d\n", nreloc);

    for (x = 0; x < nreloc; x++)
    {
        fixseg = (unsigned short *)(codestart + rlstart[x * 2]
                 + (rlstart[x * 2 + 1] << 4));
#if 0
        printf("first %d\n", rlstart[x*2]);
        printf("second %d\n", rlstart[x * 2 + 1] << 4);
#endif
        if (base != 0x700)
        {
            *fixseg += (base >> 4);
        }
    }

    /* because we are now using org, the relocations don't
       need to be done, and instead we just need to strip
       the loading NULs - this needs to be revisited */
    if (base == 0x700)
    {
        fwrite(codestart + base, 1, sz - hdr->header_size * 16 - base, fq);
    }
    else
    {
        fwrite(codestart, 1, sz - hdr->header_size * 16, fq);
    }

    printf("done\n");
    return (EXIT_SUCCESS);
}
