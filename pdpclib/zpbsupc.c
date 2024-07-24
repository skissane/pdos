/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  zpbsupc.c - support routines for z/PDOS pseudo-bios              */
/*                                                                   */
/*********************************************************************/

#include <stdlib.h>


typedef unsigned int UINT8[2];

#define DAT_ALIGN 4096 /* DAT tables need to be aligned on 4k boundary */

#define PAGES_PER_SEGMENT (1024U * 1024 / 4096)

typedef UINT8 SEG_ENTRY;

typedef UINT8 PAGE_ENTRY;



/* DAT tables */

typedef struct {
    /* this requires 4k alignment */
    UINT8 region1[512]; /* 1 entry plus padding */
    /* this requires 4k alignment */
    UINT8 region2[512]; /* 1 entry plus padding */
    /* this requires 4k alignment */
    UINT8 region3[512]; /* 4 lots of 2 GB plus padding */
    /* this requires 4k alignment */
    SEG_ENTRY segtable[4096 * 2]; /* we need double the mappings so that
                                         4-8 GiB gets mapped to 0-4 GiB */
    /* I think this requires 2k alignment, but will be 4k regardless */
    PAGE_ENTRY pagetable[4096 * PAGES_PER_SEGMENT];
} DATTAB;

static unsigned int creg1;


/* map the 4 GiB to 8 GiB region onto 0-4 GiB to provide
   effective AM32 to allow negative addresses */
void __map4gb(void)
{
    int s;
    int a;
    int p;
    DATTAB *dattab;

    dattab = malloc(sizeof(DATTAB) + DAT_ALIGN);
    if (dattab == NULL) return;

    /* align on 4k boundary */    
    dattab = (DATTAB *)
             (
                 (char *)dattab + DAT_ALIGN
                                - ((unsigned int)dattab % DAT_ALIGN)
             );

    /* 8196 MB needs to be mapped - the upper 4 GB to the lower 4 GB */
        for (s = 0; s < 4096; s++)
        {
            dattab->segtable[s][0] = 0;
            dattab->segtable[s][1] =
                  (unsigned int)
                  &dattab->pagetable[s * PAGES_PER_SEGMENT][0];
            dattab->segtable[4096 + s][0] = 0;
            dattab->segtable[4096 + s][1] = 
                  (unsigned int)
                  &dattab->pagetable[s * PAGES_PER_SEGMENT][0];
            for (p = 0; p < PAGES_PER_SEGMENT; p++)
            {
                dattab->pagetable[s
                                             * PAGES_PER_SEGMENT
                                             + p][0] = 0;
                dattab->pagetable[s
                                             * PAGES_PER_SEGMENT
                                             + p][1] =
                    (s * PAGES_PER_SEGMENT + p) << 12;
            }
        }
        dattab->region3[0][0] = 0;
        dattab->region3[0][1] =
            (unsigned int)&dattab->segtable[0]
            | (0x1 << 2) /* region 3 indicator */
            | 0x03; /* segment table is max size */
        dattab->region3[1][0] = 0;
        dattab->region3[1][1] =
            (unsigned int)&dattab->segtable[2048]
            | (0x1 << 2) /* region 3 indicator */
            | 0x03; /* segment table is max size */
        dattab->region3[2][0] = 0;
        dattab->region3[2][1] =
            (unsigned int)&dattab->segtable[0]
            | (0x1 << 2) /* region 3 indicator */
            | 0x03; /* segment table is max size */
        dattab->region3[3][0] = 0;
        dattab->region3[3][1] =
            (unsigned int)&dattab->segtable[2048]
            | (0x1 << 2) /* region 3 indicator */
            | 0x03; /* segment table is max size */
        /* pad to minimum size */
        for (p = 4; p < 512; p++)
        {
            dattab->region3[p][0] = 0;
            dattab->region3[p][1] = 1 << 5; /* region invalid bit */
        }

        dattab->region2[0][0] = 0;
        dattab->region2[0][1] =
            (unsigned int)&dattab->region3[0]
            | (0x2 << 2); /* region 2 indicator */
        /* pad to minimum size */
        for (p = 1; p < 512; p++)
        {
            dattab->region2[p][0] = 0;
            dattab->region2[p][1] = 1 << 5; /* region invalid bit */
        }

        dattab->region1[0][0] = 0;
        dattab->region1[0][1] =
            (unsigned int)&dattab->region2[0]
            | (0x3 << 2); /* region 1 indicator */
        /* pad to minimum size */
        for (p = 1; p < 512; p++)
        {
            dattab->region2[p][0] = 0;
            dattab->region2[p][1] = 1 << 5; /* region invalid bit */
        }

        creg1 =
            0
            | (unsigned int)&dattab->region1
            | (0x03 << 2); /* region 1 indicator */


    /* load CR1 */
    __lcreg1(creg1);
    /* switch DAT on */
    __daton();

    return;
}
