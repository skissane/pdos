/*********************************************************************/
/*                                                                   */
/*  This Program Written by Alica Okano.                             */
/*  Released to the Public Domain as discussed here:                 */
/*  http://creativecommons.org/publicdomain/zero/1.0/                */
/*                                                                   */
/*  Reworked and AmigaOS and MVS added by Paul Edwards               */
/*  All changes remain public domain                                 */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  exeload.c - functions for loading executables                    */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <assert.h>

/* moved these defines here to reduce pressure on
   the size of command line */
#if defined(__MSC__) && defined(__OS2__) && defined(__16BIT__)
#define NEED_MZ 1
#define NO_DLLENTRY 1
#endif

#ifdef __OS2__
#include <os2.h>
#endif

#include "exeload.h"

/* Headers for executable support. */
#if NEED_AOUT
#include "a_out.h"
#endif
#if NEED_MZ
#include "mz.h"
#include "pecoff.h"
char msvcrt[FILENAME_MAX] = "\\MSVCRT.DLL";
char kernel32[FILENAME_MAX] = "\\KERNEL32.DLL";
#endif
#if NEED_ELF
#include "elf.h"
#endif

/* For reading files Pos API must be used directly
 * as PDPCLIB allocates memory from the process address space. */

#if NEED_AOUT
static int exeloadLoadAOUT(unsigned char **entry_point,
                           FILE *fp,
                           unsigned char **loadloc);

#endif
#if NEED_MZ
static int exeloadLoadMZ(unsigned char **entry_point,
                         FILE *fp,
                         unsigned char **loadloc);
/* Subfunctions of exeloadLoadMZ() for loading extensions to MZ. */
static int exeloadLoadPE(unsigned char **entry_point,
                         FILE *fp,
                         unsigned char **loadloc,
                         unsigned long e_lfanew);

static int exeloadLoadLX(unsigned char **entry_point,
                         FILE *fp,
                         unsigned long e_lfanew);
static int exeloadLoadNE(unsigned char **entry_point,
                         FILE *fp,
                         unsigned char **loadloc,
                         unsigned long e_lfanew);

/* Support function for loading DLLs. */
static int exeloadLoadPEDLL(unsigned char *exeStart,
                            IMAGE_IMPORT_DESCRIPTOR *import_desc);
#endif

#if NEED_ELF
static int exeloadLoadELF(unsigned char **entry_point,
                          FILE *fp,
                          unsigned char **loadloc);
#endif
#if NEED_MVS
static int exeloadLoadMVS(unsigned char **entry_point,
                          FILE *fp,
                          unsigned char **loadloc);
static int fixPE(unsigned char *buf,
                 size_t *len,
                 unsigned char **entry,
                 unsigned long rlad);
static int processRLD(unsigned char *buf,
                      unsigned long rlad,
                      unsigned char *rld,
                      int len);
#endif
#if NEED_AMIGA
static int exeloadLoadAmiga(unsigned char **entry_point,
                            FILE *fp,
                            unsigned char **loadloc);

#endif
int exeloadDoload(unsigned char **entry_point,
                  char *progname,
                  unsigned char **loadloc)
{
    FILE *fp;
    int ret = 1;

    fp = fopen(progname, "rb");
    if (fp == NULL)
    {
        printf("Failed to open %s for loading\n", progname);
        return (1);
    }
    /* Tries to load the executable as different formats.
     * Returned 0 means the executable was loaded successfully.
     * 1 means it is not the format the function loads.
     * 2 means correct format, but error occured. */
#if NEED_AOUT
    if (ret == 1) ret = exeloadLoadAOUT(entry_point, fp, loadloc);
#endif
#if NEED_MZ
    if (ret == 1) ret = exeloadLoadMZ(entry_point, fp, loadloc);
#endif
#if NEED_AMIGA
    if (ret == 1) ret = exeloadLoadAmiga(entry_point, fp, loadloc);
#endif
#if NEED_MVS
    if (ret == 1) ret = exeloadLoadMVS((unsigned char **)entry_point,
                                       fp,
                                       (unsigned char **)loadloc);
#endif
#if NEED_ELF
    if (ret == 1) ret = exeloadLoadELF(entry_point, fp, loadloc);
#endif

    fclose(fp);
    if (ret != 0)
    {
        return (1);
    }

    return (0);
}

#if NEED_AOUT
static int exeloadLoadAOUT(unsigned char **entry_point,
                           FILE *fp,
                           unsigned char **loadloc)
{
    struct exec firstbit;
    unsigned long exeLen;
    unsigned char *exeStart;
    unsigned char *bss;
    unsigned int *corrections;
    unsigned int i;
    unsigned int offs;
    unsigned int type;
    unsigned int zapdata;
    unsigned char *zap;


    if ((fseek(fp, 0, SEEK_SET) != 0)
        || (fread(&firstbit, sizeof(firstbit), 1, fp) != 1))
    {
        return (1);
    }
    /* ZMAGIC and NMAGIC are currently not supported
     * as they should be loaded at 0x10000,
     * but the kernel reserves the first 4 MiB. */
    if ((firstbit.a_info & 0xffff) == ZMAGIC)
    {
        printf("a.out ZMAGIC is not supported\n");
        return (2);
    }
    else if ((firstbit.a_info & 0xffff) == NMAGIC)
    {
        printf("a.out NMAGIC is not supported\n");
        return (2);
    }
    else if ((firstbit.a_info & 0xffff) == QMAGIC)
    {
        printf("a.out QMAGIC is not supported\n");
        return (2);
    }
    else if ((firstbit.a_info & 0xffff) != OMAGIC)
    {
        /* The file is not A.OUT. */
        return (1);
    }
    exeLen = firstbit.a_text + firstbit.a_data + firstbit.a_bss;
    if (*loadloc != NULL)
    {
        exeStart = *loadloc;
    }
    else
    {
        exeStart = malloc(exeLen);
        if (exeStart == NULL)
        {
            printf("Insufficient memory to load A.OUT program\n");
            return (2);
        }
    }

    /* printf("loading %lu bytes of text\n", firstbit.a_text); */
    if (fread(exeStart, firstbit.a_text, 1, fp) != 1)
    {
        printf("Error occured while reading A.OUT text\n");
        return (2);
    }
    if (firstbit.a_data != 0)
    {
        /* printf("loading %lu bytes of data\n", firstbit.a_data); */
        if (fread(exeStart + firstbit.a_text,
                  firstbit.a_data,
                  1,
                  fp) != 1)
        {
            printf("Error occured while reading A.OUT data\n");
            return (2);
        }
    }

    /* initialise BSS */
    bss = exeStart + firstbit.a_text + firstbit.a_data;
    memset(bss, '\0', firstbit.a_bss);
    /* Relocations. */
    {
        zap = exeStart;
        zapdata = (unsigned int)zap;
        if (firstbit.a_trsize != 0)
        {
            corrections = malloc(firstbit.a_trsize);
            if (corrections == NULL)
            {
                printf("insufficient memory %lu\n", firstbit.a_trsize);
                return (2);
            }
            if (fread(corrections, firstbit.a_trsize, 1, fp) != 1)
            {
                printf("Error occured while reading A.OUT text relocations\n");
                free(corrections);
                return (2);
            }
            for (i = 0; i < firstbit.a_trsize / 4; i += 2)
            {
                offs = corrections[i];
                type = corrections[i + 1];
                if (((type >> 24) & 0xff) != 0x04)
                {
                    continue;
                }
                *(unsigned int *)(zap + offs) += zapdata;
            }
            free(corrections);
        }
        if (firstbit.a_drsize != 0)
        {
            corrections = malloc(firstbit.a_drsize);
            if (corrections == NULL)
            {
                printf("insufficient memory %lu\n", firstbit.a_drsize);
                return (2);
            }
            if (fread(corrections, firstbit.a_drsize, 1, fp) != 1)
            {
                printf("Error occured while reading A.OUT data relocations\n");
                free(corrections);
                return (2);
            }
            zap = exeStart + firstbit.a_text;
            for (i = 0; i < firstbit.a_drsize / 4; i += 2)
            {
                offs = corrections[i];
                type = corrections[i + 1];
                if (((type >> 24) & 0xff) != 0x04)
                {
                    continue;
                }
                *(unsigned int *)(zap + offs) += zapdata;
            }
            free(corrections);
        }
    }

    *entry_point = exeStart + firstbit.a_entry;
    if (*loadloc == NULL)
    {
        *loadloc = exeStart;
    }

    return (0);
}
#endif

#if NEED_MVS
static int exeloadLoadMVS(unsigned char **entry_point,
                          FILE *fp,
                          unsigned char **loadloc)
{
    size_t readbytes;
    unsigned char *entry;
    int didalloc = 0;

    /* printf("in LoadMVS\n"); */
    if (*loadloc == NULL)
    {
        *loadloc = malloc(1000000);
        if (*loadloc == NULL)
        {
            return (1);
        }
        didalloc = 1;
    }
    rewind(fp);
    readbytes = fread(*loadloc, 1, 1000000, fp);
    /* printf("read %d bytes\n", (int)readbytes); */

    /* the last parameter is an unsigned long because the code allows
       relocations to be done for a theoretical load point rather than
       requiring the actual load point. In case we wanted to do the
       calculations to later store the executable in ROM or something
       like that. */

    if (fixPE(*loadloc, &readbytes, &entry, (unsigned long)*loadloc) != 0)
    {
        if (didalloc)
        {
            free(*loadloc);
            *loadloc = NULL;
        }
        return (1);
    }
    *entry_point = entry;

    return (0);
}
#endif

#if NEED_AMIGA
#define getword(p) ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3])
#define putword(p, val) (p[0] = (unsigned char)((val >> 24) & 0xff), \
                         p[1] = (unsigned char)((val >> 16) & 0xff), \
                         p[2] = (unsigned char)((val >> 8) & 0xff), \
                         p[3] = (unsigned char)(val & 0xff))

#define HUNK_HEADER 0x3F3
#define HUNK_CODE 0x3E9
#define HUNK_DATA 0x3EA
#define HUNK_BSS 0x3EB
#define HUNK_RELOC32 0x3EC
#define HUNK_SYMBOL 0x3F0
#define HUNK_END 0x3F2

static int exeloadLoadAmiga(unsigned char **entry_point,
                            FILE *fp,
                            unsigned char **loadloc)
{
    size_t readbytes;
    unsigned char *p;
    unsigned long temp;
    int datahunk = 1;
    int codehunk = 0;
    int bsshunk;
    unsigned char *beg;
    unsigned char *codestart;
    unsigned char *datastart = NULL;
    unsigned char *bssstart;
    unsigned char *genstart; /* generic start location when doing relocations */
    unsigned char *codereloc = NULL;
    unsigned char *datareloc = NULL;
    int iter;
    int didalloc = 0;

    /* printf("in LoadAmiga\n"); */
    if (*loadloc == NULL)
    {
        *loadloc = malloc(1000000);
        if (*loadloc == NULL)
        {
            return (1);
        }
        didalloc = 1;
    }
    p = beg = *loadloc;
    rewind(fp);
    readbytes = fread(p, 1, 1000000, fp);
    /* printf("read %d bytes\n", (int)readbytes); */
    if (readbytes < 4)
    {
        if (didalloc)
        {
            free(*loadloc);
            *loadloc = NULL;
        }
        return (1);
    }
    bssstart = p + readbytes;
    temp = getword(p);
    if (temp != HUNK_HEADER)
    {
        if (didalloc)
        {
            free(*loadloc);
            *loadloc = NULL;
        }
        return (1);
    }
    p += 4;
    temp = getword(p);
    if (temp != 0)
    {
        printf("can't handle Amiga Hunk 1\n");
        if (didalloc)
        {
            free(*loadloc);
            *loadloc = NULL;
        }
        return (1);
    }
    p += 4;
    temp = getword(p);
    if (temp == 2)
    {
        /* printf("no data section\n"); */
        bsshunk = 1;
        datahunk = -1;
        /* printf("bsshunk determined to be %d\n", bsshunk); */
        p += 8;
    }
    else if (temp == 3)
    {
        /* looks like this:
        000000  000003F3 00000000 00000003 00000000  ................
        000010  00000002 0000002A 00000001 00000002  .......*........

        So that 3 is the number of hunks, or highest hunk number
        plus 1. Then comes 0, the first hunk number. Then comes
        2, the last hunk number, then comes x'2A', the length of
        hunk number 0 (CODE), then comes 1, the length of
        hunk number 1 (DATA), then comes 2, the length of hunk
        number 2 (BSS since we have a data section, otherwise
        it would have been 1). Lengths are the number of 4-byte
        words.
        */
        /* printf("we have a data section\n"); */
        bsshunk = 2;
        /* printf("bsshunk determined to be %d\n", bsshunk); */
        p += 12;
    }
    else
    {
        printf("can't handle Amiga Hunk 2\n");
        if (didalloc)
        {
            free(*loadloc);
            *loadloc = NULL;
        }
        return (1);
    }
    p += 12;

    temp = getword(p);
    if (temp != HUNK_CODE)
    {
        printf("Amiga Hunk expecting code at hex %lx\n",
               (unsigned long)(p - beg));
        if (didalloc)
        {
            free(*loadloc);
            *loadloc = NULL;
        }
        return (1);
    }
    p += 4;
    temp = getword(p);
    /* printf("length of code is hex %lx\n", temp); */
    p += 4;
    codestart = p;
    /* printf("at codestart is %02X\n", *(unsigned char *)codestart); */
    p += temp * 4;

    while (1)
    {
        if (p >= bssstart)
        {
            /* printf("reached end of executable\n"); */
            break;
        }
        temp = getword(p);
        if (temp == HUNK_RELOC32)
        {
            p += 4;
            /* if we haven't yet encountered a HUNK_DATA, then this
               must be a code relocation */
            if (datastart == NULL)
            {
                codereloc = p;
            }
            else
            {
                datareloc = p;
            }
            /* skip relocations for now, come back later */
            temp = getword(p);
            while (temp != 0)
            {
                p += 4; /* skip count of relocations */
                p += 4; /* skip hunk number */
                p += temp * 4; /* skip relocations */
                temp = getword(p);
            }
            /* printf("skipped relocations\n"); */
            p += 4;
        }
        else if (temp == HUNK_DATA)
        {
            p += 4; /* skip hunk id */
            temp = getword(p);
            p += 4; /* skip number of words */
            datastart = p;
            /* printf("got data start for real\n"); */
            p += temp * 4;
        }
        else if (temp == HUNK_SYMBOL)
        {
            /* printf("got symbol\n"); */
            p += 4; /* skip hunk id */
            temp = getword(p);
            while (temp != 0) /* until hunk number is 0 */
            {
                long len;
                long rem;

                p += 4; /* skip hunk number */
                /* printf("symbol %s found\n", p); */
                len = strlen((char *)p);
                len++;
                rem = len % 4;
                if (rem != 0)
                {
                    len = len - rem + 4; /* move up to 4-byte boundary */
                }
                p += len;
                p += 4; /* skip offset */
                temp = getword(p);
            }
            p += 4; /* skip hunk number */
        }
        else if (temp == HUNK_END)
        {
            /* printf("got hunk end\n"); */
            p += 4;
        }
        else if (temp == HUNK_BSS)
        {
            /* printf("got BSS hunk\n"); */
            p += 8;
        }
        else
        {
            printf("unexpected Amiga Hunk id hex %lx\n", temp);
            if (didalloc)
            {
                free(*loadloc);
                *loadloc = NULL;
            }
            return (1);
        }
    }

    /* apply relocations */
    /* printf("applying relocations\n"); */
    for (iter = 0; iter < 2; iter++)
    {
        if ((iter == 0) && (codereloc == NULL)) continue;
        if ((iter == 1) && (datareloc == NULL)) continue;
        if (iter == 0)
        {
            /* printf("applying code relocations\n"); */
            genstart = codestart;
            p = codereloc;
        }
        else if (iter == 1)
        {
            /* printf("applying data relocations\n"); */
            genstart = datastart;
            p = datareloc;
        }
        temp = getword(p);
        while (temp != 0)
        {
            unsigned long count;
            int hunk_nbr;
            unsigned char *correction;
            unsigned long x;

            count = temp;
            p += 4; /* skip count of relocations */
            hunk_nbr = getword(p);
            p += 4; /* skip hunk number */
            if (hunk_nbr == codehunk)
            {
                correction = codestart;
                /* printf("got %lu CODE relocations\n", count); */
            }
            else if (hunk_nbr == datahunk)
            {
                correction = datastart;
                /* printf("got %lu DATA relocations\n", count); */
            }
            else if (hunk_nbr == bsshunk)
            {
                correction = bssstart;
                /* printf("got %lu BSS relocations\n", count); */
            }
            else
            {
                printf("got unexpected Amiga Hunk number hex %x\n", hunk_nbr);
                printf("code hunk is %d\n", codehunk);
                printf("data hunk is %d\n", datahunk);
                printf("bss hunk is %d\n", bsshunk);
                if (didalloc)
                {
                    free(*loadloc);
                    *loadloc = NULL;
                }
                return (1);
            }
            /* WARNING - the following is not entirely accurate. There
               ARE data relocations, and the above code caters for that,
               and that's why there are two iterations of the loop. But
               pogi points for trying. */

            /* Note that all relocations are applied to the code
               section, but the code needs different corrections
               depending on whether it is referencing another location
               in code, a data reference, e.g. a global variable with a
               value of 5, or a BSS reference, e.g. an uninitialized
               global variable. */
            /* At the various offsets in the code there will be values,
               such as 48, which are the correct offsets if the module
               was loaded at location 0 in memory, and also the data
               section started at location 0 in memory, and also the
               bss section (variables that need to be set to 0 before
               the executable runs) is set to location 0 in memory.
               Note that the BSS takes up no space in the executable
               because it would just be a whole lot of zeros serving
               no purpose. So the onus is on the program loading the
               executable to initialize that area. That area is
               normally placed at the end of the executable, but you
               can do a separate malloc and put it anywhere if you
               want. You can also put the data section anywhere you
               want to, with a separate malloc, so long as you actually
               copy all that data across to the new location. This is
               what needs to happen with reentrant modules. There needs
               to be a register (such as ds: in 8086) pointing to where
               the global variables are. And normally the BSS comes
               after that, so you can use a single register to point to
               both data and BSS. The code corrections only need to be
               done once for the code section itself, and the data
               offsets are all given in reference to some dedicated
               register (such as ds) so that no more corrections are
               required to the code to reference the correct offsets.
               In fact, you don't need to correct them at all, they are
               all set to the correct offsets for the dedicated
               register. But if you're not using a dedicated register,
               or you are using the huge, large or compact memory models
               of the 8086, you can't use a dedicated register, as there
               is too much data (more than 64k) for ds to reference, so
               ds keeps on being reloaded, and thus the code corrections
               need to be made to point to the full address
               (segment:offset) of where the variable can be found.
               For the 68000 you also need the full address (as the
               compiler is not using a dedicated register and dynamically
               providing the offsets), but it is a flat 32-bit pointer.
               If you are using a compiler that DOES use a dedicated
               register, then there will simply be no "data corrections"
               found in the relocation table, ie the relocation table
               will only have references to locations in the code where
               somewhere else in the code needs to be referenced (e.g.
               to call a function) but because the code is not loaded
               at location 0 (or effectively done so via virtual memory,
               or some other fixed location like Windows used to use),
               so it needs to know where exactly in memory it has been
               loaded. */

            for (x = 0; x < count; x++)
            {
                unsigned char *zaploc;
                unsigned char *xxx;
                unsigned long value;
                unsigned long oldref;
                unsigned char *properref;
                unsigned long properref_val;

                xxx = &p[x * 4]; /* point to a value, such as 12,
                    which is where in the code section will need
                    to be adjusted */
                value = getword(xxx); /* now we have that value 12 */
                zaploc = genstart + value; /* now we point to the
                    exact location in the code section that needs
                    to have a correction applied. Note that the
                    corrected value to be applied depends on whether
                    it is a code, data or BSS reference, but we have
                    already determined that, and are already pointing
                    to the relevant memory address. */
                oldref = getword(zaploc); /* now we have the old
                    value of the function or whatever, e.g. if the
                    called function is at offset 40 in the code
                    section, then it will have the value 40. */
                properref = correction + oldref; /* This is where we
                    want the code to start looking for this code, data
                    or BSS reference. Even if we are on a 64-bit system
                    and we are loading this 32-bit module, someone
                    else will have made sure that we are effectively
                    on a 4 GiB boundary so that this pointer will be
                    valid, even though we're just about to truncate
                    it to 4 bytes. ie on a 64-bit system, properref
                    could be 0x1 0000 0000 - but that's OK so long
                    as the high 32 bits of all registers have been
                    set to 0x1 0000 0000. None of this 68000 code
                    knows about the high 32-bits of 64-bit registers
                    so will not disturb any data stored there. */
                properref_val = (unsigned long)properref; /* this
                    probably won't truncate the pointer, but we are
                    putting it in integer form, ready for manipulation.
                    Note that if this program is running on a 8086,
                    and you are trying to load this 68000 executable
                    (prior to switching to the 68000 coprocessor),
                    using far data pointers, you will need to use a
                    compiler that converts far pointers into flat
                    references when converting from pointer to long,
                    because
                    the 68000 program will require a flat reference.
                    ie the compiler needs to do the equivalent of
                    ((properref >> 4) << 16) | (properref & 0xffff)
                    in order to create properref_val. Or you need
                    to provide your own routine to do that. In my
                    opinion this is the job of the compiler, it
                    should know that you are wanting a flat reference.
                    So too when you go:
                    char *p = (char *)0xb8000UL;
                    it should convert that into b800:0000 itself.
                    Speak to your compiler vendor!
                    If we were doing things the other way around, ie
                    you have a 68000 loading an 8086 large memory model
                    program, then this is a "known quantity" (the same
                    as loading a small memory model program) and it is
                    this loader code (perhaps compiled with a 68000)
                    that would need to convert a flat memory pointer (ie
                    where it loaded the 8086 program into 68000 memory)
                    that would need to convert a flat pointer into a
                    segmented pointer suitable for the 8086 prior to
                    switching to the 8086 coprocessor. So it is reasonable
                    for you to do the bit shifting in your own code. After
                    first of all converting into an unsigned long, because
                    maybe the loader is not 68000 either, but another
                    segmented architecture with different segmentation rules,
                    and it too needs to convert to a flat address (as
                    represented by unsigned long) prior to being manually
                    converted into an address suitable for the 8086. */

                putword(zaploc, properref_val);
            }
            p += temp * 4; /* skip relocations */
            temp = getword(p);
        }
    }
    /* printf("finished relocations\n"); */

    *entry_point = codestart;
    /* printf("just set entry point, first byte %02X\n",
           *(unsigned char *)codestart); */

    return (0);
}
#endif

#if NEED_ELF
static int exeloadLoadELF(unsigned char **entry_point,
                          FILE *fp,
                          unsigned char **loadloc)
{
    int doing_elf_rel = 0;
    int doing_elf_exec = 0;
    Elf32_Ehdr *elfHdr;
    Elf32_Phdr *program_table = NULL;
    Elf32_Phdr *segment;
    Elf32_Shdr *section_table = NULL;
    Elf32_Shdr *section;
    unsigned char *elf_other_sections = NULL;
    Elf32_Addr lowest_p_vaddr = 0;
    Elf32_Word lowest_segment_align = 0;
    unsigned char *exeStart = NULL;
    unsigned char *bss;
    unsigned long exeLen;
    unsigned char firstbit[4];
    long newpos;
    size_t readbytes;

    if ((fseek(fp, 0, SEEK_SET) != 0)
        || (fread(&firstbit, sizeof firstbit, 1, fp) != 1)
        || (memcmp(&firstbit, "\x7f" "ELF", 4) != 0))
    {
        return (1);
    }
    {
        int elf_invalid = 0;

        /* Loads entire ELF header into memory. */
        elfHdr = malloc(sizeof(Elf32_Ehdr));
        if (elfHdr == NULL)
        {
            printf("Insufficient memory for ELF header\n");
            return (2);
        }
        if ((fseek(fp, 0, SEEK_SET) != 0)
            || (fread(elfHdr, sizeof(Elf32_Ehdr), 1, fp) != 1))
        {
            printf("Error occured while reading ELF header\n");
            free(elfHdr);
            return (2);
        }

        /* Checks e_ident if the program can be used on PDOS-32. */
        if (elfHdr->e_ident[EI_CLASS] != ELFCLASS32)
        {
            if (elfHdr->e_ident[EI_CLASS] == ELFCLASS64)
            {
                printf("64-bit ELF is not supported\n");
            }
            else if (elfHdr->e_ident[EI_CLASS] == ELFCLASSNONE)
            {
                printf("Invalid ELF class\n");
            }
            else
            {
                printf("Unknown ELF class: %u\n", elfHdr->e_ident[EI_CLASS]);
            }
            elf_invalid = 1;
        }
        if (elfHdr->e_ident[EI_DATA] != ELFDATA2LSB)
        {
            if (elfHdr->e_ident[EI_DATA] == ELFDATA2MSB)
            {
                printf("Big-endian ELF encoding is not supported\n");
            }
            else if (elfHdr->e_ident[EI_DATA] == ELFDATANONE)
            {
                printf("Invalid ELF data encoding\n");
            }
            else
            {
                printf("Unknown ELF data encoding: %u\n",
                       elfHdr->e_ident[EI_DATA]);
            }
            elf_invalid = 1;
        }
        if (elfHdr->e_ident[EI_OSABI] != ELFOSABI_NONE)
        {
            printf("No OS or ABI specific extensions for ELF supported\n");
            elf_invalid = 1;
        }
        /* Checks other parts of the header if the file can be loaded. */
        if (elfHdr->e_type == ET_REL)
        {
            doing_elf_rel = 1;
        }
        else if (elfHdr->e_type == ET_EXEC)
        {
            doing_elf_exec = 1;
        }
        else
        {
            printf("Only ELF relocatable "
                   "and executable "
                   "files are supported\n");
            elf_invalid = 1;
        }
        if (elfHdr->e_machine != EM_386)
        {
            printf("Only Intel 386 architecture is supported\n");
            elf_invalid = 1;
        }
        if (doing_elf_exec)
        {
            if (elfHdr->e_phoff == 0 || elfHdr->e_phnum == 0)
            {
                printf("Executable file is missing Program Header Table\n");
                elf_invalid = 1;
            }
            if (elfHdr->e_phnum >= SHN_LORESERVE)
            {
                printf("Reserved indexes for e_phnum are not supported\n");
                printf("e_phnum is %04x\n", elfHdr->e_phnum);
                elf_invalid = 1;
            }
            if (elfHdr->e_phentsize != sizeof(Elf32_Phdr))
            {
                printf("Program Header Table entries have unsupported size\n");
                printf("e_phentsize: %u supported size: %lu\n",
                       elfHdr->e_phentsize, sizeof(Elf32_Phdr));
                elf_invalid = 1;
            }
        }
        else if (doing_elf_rel)
        {
            if (elfHdr->e_shoff == 0 || elfHdr->e_shnum == 0)
            {
                printf("Relocatable file is missing Section Header Table\n");
                elf_invalid = 1;
            }
            if (elfHdr->e_shnum >= SHN_LORESERVE)
            {
                printf("Reserved indexes for e_shnum are not supported\n");
                printf("e_shnum is %04x\n", elfHdr->e_shnum);
                elf_invalid = 1;
            }
            if (elfHdr->e_shentsize != sizeof(Elf32_Shdr))
            {
                printf("Section Header Table entries have unsupported size\n");
                printf("e_shentsize: %u supported size: %lu\n",
                       elfHdr->e_shentsize, sizeof(Elf32_Shdr));
                elf_invalid = 1;
            }
        }
        if (elf_invalid)
        {
            /* All problems with ELF header are reported
             * and loading is stopped. */
            printf("This ELF file cannot be loaded\n");
            free(elfHdr);
            return (2);
        }
        /* Loads Program Header Table if it is present. */
        if (!(elfHdr->e_phoff == 0 || elfHdr->e_phnum == 0))
        {
            program_table = malloc(elfHdr->e_phnum * elfHdr->e_phentsize);
            if (program_table == NULL)
            {
                printf("Insufficient memory for ELF Program Header Table\n");
                free(elfHdr);
                return (2);
            }
            if ((fseek(fp, elfHdr->e_phoff, SEEK_SET) != 0)
                || (fread(program_table,
                          elfHdr->e_phnum * elfHdr->e_phentsize,
                          1,
                          fp) != 1))
            {
                printf("Error occured while reading "
                       "ELF Program Header Table\n");
                free(elfHdr);
                free(program_table);
                return (2);
            }
        }
        /* Loads Section Header Table if it is present. */
        if (!(elfHdr->e_shoff == 0 || elfHdr->e_shnum == 0))
        {
            section_table = malloc(elfHdr->e_shnum * elfHdr->e_shentsize);
            if (section_table == NULL)
            {
                printf("Insufficient memory for ELF Section Header Table\n");
                free(elfHdr);
                free(program_table);
                return (2);
            }
            if ((fseek(fp, elfHdr->e_shoff, SEEK_SET) != 0)
                || (fread(section_table,
                          elfHdr->e_shnum * elfHdr->e_shentsize,
                          1,
                          fp) != 1))
            {
                printf("Error occured while reading "
                       "ELF Section Header Table\n");
                free(elfHdr);
                free(program_table);
                free(section_table);
                return (2);
            }
        }
    }

    if (1)
    {
        /* Calculates how much memory is needed
         * and allocates memory for sections used only for loading. */
        unsigned long otherLen = 0;

        exeLen = 0;
        if (doing_elf_exec)
        {
            Elf32_Addr highest_p_vaddr = 0;
            Elf32_Word highest_segment_memsz = 0;

            for (segment = program_table;
                 segment < program_table + elfHdr->e_phnum;
                 segment++)
            {
                if (segment->p_type == PT_LOAD)
                {
                    if (!lowest_p_vaddr || lowest_p_vaddr > segment->p_vaddr)
                    {
                        lowest_p_vaddr = segment->p_vaddr;
                        lowest_segment_align = segment->p_align;
                    }
                    if (highest_p_vaddr < segment->p_vaddr)
                    {
                        highest_p_vaddr = segment->p_vaddr;
                        highest_segment_memsz = segment->p_memsz;
                    }
                }
            }
            exeLen = highest_p_vaddr - lowest_p_vaddr + highest_segment_memsz;
            if (lowest_segment_align > 1)
            {
                /* Ensures alignment of the lowest segment.
                 * 0 and 1 mean no alignment restrictions. */
                exeLen += lowest_segment_align;
            }
        }
        if (section_table)
        {
            for (section = section_table;
                 section < section_table + elfHdr->e_shnum;
                 section++)
            {
                unsigned long section_size = section->sh_size;
                if (section->sh_addralign > 1)
                {
                    /* Some sections must be aligned
                     * on sh_addralign byte boundaries.
                     * 0 and 1 mean no alignment restrictions. */
                    section_size += section->sh_addralign;
                }
                if ((section->sh_flags & SHF_ALLOC)
                    && (section->sh_type != SHT_RELA))
                {
                    /* Section is needed while the program is running,
                     * but if we are loading an executable file,
                     * the memory is already counted
                     * using Program Header Table. */
                    if (doing_elf_exec) continue;
                    exeLen += section_size;
                }
                else
                {
                    /* Section is used only for loading. */
                   otherLen += section_size;
                }
            }
            elf_other_sections = malloc(otherLen);
            if (elf_other_sections == NULL)
            {
                printf("Insufficient memory to load ELF sections\n");
                free(elfHdr);
                free(program_table);
                free(section_table);
                return (2);
            }
        }
    }
    /* Allocates memory for the process. */
    if (*loadloc != NULL)
    {
        exeStart = *loadloc;
    }
    else
    {
        exeStart = malloc(exeLen);
    }
    if (exeStart == NULL)
    {
        printf("Insufficient memory to load ELF program\n");
        free(elfHdr);
        free(program_table);
        free(section_table);
        free(elf_other_sections);
        return (2);
    }

    if (1)
    {
        /* Loads all sections of ELF file with proper alignment,
         * clears all SHT_NOBITS sections and stores the addresses
         * in sh_addr of each section.
         * bss is set now too. */
        unsigned char *exe_addr = exeStart;
        unsigned char *other_addr = elf_other_sections;

        bss = NULL;
        if (doing_elf_exec)
        {
            /* Aligns the exeStart on lowest segment alignment boundary. */
            /*exeStart = (unsigned char *)((((unsigned long)exeStart
                                           / lowest_segment_align) + 1)
                                         * lowest_segment_align);*/
            /* +++Enable aligning. */
            for (segment = program_table;
                 segment < program_table + elfHdr->e_phnum;
                 segment++)
            {
                if (segment->p_type == PT_LOAD)
                {
                    exe_addr = exeStart + (segment->p_vaddr - lowest_p_vaddr);

                    if ((fseek(fp, segment->p_offset, SEEK_SET) != 0)
                        || (fread(exe_addr,
                                  segment->p_filesz,
                                  1,
                                  fp) != 1))
                    {
                        printf("Error occured while reading ELF segment\n");
                        free(elfHdr);
                        free(program_table);
                        free(section_table);
                        free(elf_other_sections);
                        return (2);
                    }

                    /* Bytes that are not present in file,
                     * but must be present in memory must be set to 0. */
                    if (segment->p_filesz < segment->p_memsz)
                    {
                        bss = exe_addr + (segment->p_filesz);
                        memset(bss, '\0',
                               segment->p_memsz - segment->p_filesz);
                    }
                }
            }
        }

        for (section = section_table;
             section < section_table + elfHdr->e_shnum;
             section++)
        {
            if ((section->sh_flags & SHF_ALLOC)
                && (section->sh_type != SHT_RELA))
            {
                /* If we are loading executable file,
                 * SHF_ALLOC sections are already loaded in segments. */
                if (doing_elf_exec) continue;
                if (section->sh_addralign > 1)
                {
                    exe_addr = (void *)((((unsigned long)exe_addr
                                          / (section->sh_addralign)) + 1)
                                        * (section->sh_addralign));
                }
                if (section->sh_type != SHT_NOBITS)
                {
                    if ((fseek(fp, section->sh_offset, SEEK_SET) != 0)
                        || (fread(exe_addr,
                                  section->sh_size,
                                  1,
                                  fp) != 1))
                    {
                        printf("Error occured while reading ELF section\n");
                        free(elfHdr);
                        free(program_table);
                        free(section_table);
                        free(elf_other_sections);
                        return (2);
                    }
                }
                else
                {
                    /* The section is probably BSS. */
                    if (bss != 0)
                    {
                        printf("Multiple SHT_NOBITS with SHF_ALLOC "
                               "present in ELF file\n");
                    }
                    bss = exe_addr;
                    /* All SHT_NOBITS should be cleared to 0. */
                    memset(bss, '\0', section->sh_size);
                }
                /* sh_addr is 0 in relocatable files,
                 * so we can use it to store the real address. */
                section->sh_addr = (Elf32_Addr)exe_addr;
                exe_addr += section->sh_size;
            }
            else
            {
                if (section->sh_addralign > 1)
                {
                    other_addr = (void *)((((unsigned long)other_addr
                                            / (section->sh_addralign)) + 1)
                                          * (section->sh_addralign));
                }
                if (section->sh_type != SHT_NOBITS)
                {
                    if (section->sh_size != 0)
                    {
                        if ((fseek(fp, section->sh_offset, SEEK_SET) != 0)
                            || (fread(other_addr,
                                      section->sh_size,
                                      1,
                                      fp) != 1))
                        {
                            printf("Error occured while reading ELF section\n");
                            free(elfHdr);
                            free(program_table);
                            free(section_table);
                            free(elf_other_sections);
                            return (2);
                        }
                    }
                }
                else
                {
                    /* All SHT_NOBITS should be cleared to 0. */
                    memset(other_addr, '\0', section->sh_size);
                }
                /* sh_addr is 0 in relocatable files,
                 * so we can use it to store the real address. */
                section->sh_addr = (Elf32_Addr)other_addr;
                other_addr += section->sh_size;
            }
        }
    }
    /* Program was successfully loaded from the file,
     * no more errors can occur. */

    /* Relocations. */
    if (1)
    {
        for (section = section_table;
             section < section_table + elfHdr->e_shnum;
             section++)
        {
            if (section->sh_type == SHT_RELA)
            {
                /* sh_link specifies the symbol table
                 * and sh_info section being modified. */
                Elf32_Sym *sym_table = (Elf32_Sym *)(section_table
                                        + (section->sh_link))->sh_addr;
                unsigned char *target_base = exeStart +
                                             (((unsigned char *)(section_table
                                             + (section->sh_info))->sh_addr)
                                             - (unsigned char *)lowest_p_vaddr);
                Elf32_Rela *startrel = (Elf32_Rela *)section->sh_addr;
                Elf32_Rela *currel;

                if (section->sh_entsize != sizeof(Elf32_Rela))
                {
                    printf("Invalid size of relocation entries in ELF file\n");
                    continue;
                }

                for (currel = startrel;
                     currel < (startrel
                               + ((section->sh_size) / (section->sh_entsize)));
                     currel++)
                {
                    long *target = (long *)(target_base + currel->r_offset);
                    Elf32_Sym *symbol = (sym_table
                                         + ELF32_R_SYM(currel->r_info));
                    Elf32_Addr sym_value = 0;

                    if (ELF32_R_SYM(currel->r_info) != STN_UNDEF)
                    {
                        if (symbol->st_shndx == SHN_ABS)
                        {
                            /* Absolute symbol, stores absolute value. */
                            sym_value = symbol->st_value;
                        }
                        else if (symbol->st_shndx == SHN_UNDEF)
                        {
                            /* Dynamic linker should fill this symbol. */
                            printf("Undefined symbol in ELF file\n");
                            continue;
                        }
                        else if (symbol->st_shndx == SHN_XINDEX)
                        {
                            printf("Unsupported value in ELF symbol\n");
                            printf("symbol->st_shndx: %x\n", symbol->st_shndx);
                            continue;
                        }
                        else
                        {
                            /* Internal symbol. Must be converted
                             * to absolute symbol.*/
                            sym_value = symbol->st_value;
                            /* Adds the address of the related section
                             * so the symbol stores absolute address. */
                            sym_value += ((section_table
                                           + symbol->st_shndx)->sh_addr);
                        }
                    }
                    switch (ELF32_R_TYPE(currel->r_info))
                    {
                        case R_386_NONE:
                            break;
                        case R_386_32:
                            /* Symbol value + offset. */
                            *target = *target - lowest_p_vaddr + (long)exeStart;
                            break;
                        case R_386_PC32:
                            /* Symbol value + offset - absolute address
                             * of the modified field. */
                            *target = (sym_value + (*target)
                                       - (unsigned long)target);
                            break;
                        default:
                            printf("Unknown relocation type in ELF file\n");
                    }
                }
            }
            else if (section->sh_type == SHT_REL)
            {
                /* sh_link specifies the symbol table
                 * and sh_info section being modified. */
                Elf32_Sym *sym_table = (Elf32_Sym *)(section_table
                                        + (section->sh_link))->sh_addr;
                unsigned char *target_base = (unsigned char *)(section_table
                                             + (section->sh_info))->sh_addr;
                Elf32_Rel *startrel = (Elf32_Rel *)section->sh_addr;
                Elf32_Rel *currel;

                if (section->sh_entsize != sizeof(Elf32_Rel))
                {
                    printf("Invalid size of relocation entries in ELF file\n");
                    continue;
                }

                for (currel = startrel;
                     currel < (startrel
                               + ((section->sh_size) / (section->sh_entsize)));
                     currel++)
                {
                    long *target = (long *)(target_base + currel->r_offset);
                    Elf32_Sym *symbol = (sym_table
                                         + ELF32_R_SYM(currel->r_info));
                    Elf32_Addr sym_value = 0;

                    if (ELF32_R_SYM(currel->r_info) != STN_UNDEF)
                    {
                        if (symbol->st_shndx == SHN_ABS)
                        {
                            /* Absolute symbol, stores absolute value. */
                            sym_value = symbol->st_value;
                        }
                        else if (symbol->st_shndx == SHN_UNDEF)
                        {
                            /* Dynamic linker should fill this symbol. */
                            printf("Undefined symbol in ELF file\n");
                            continue;
                        }
                        else if (symbol->st_shndx == SHN_XINDEX)
                        {
                            printf("Unsupported value in ELF symbol\n");
                            printf("symbol->st_shndx: %x\n", symbol->st_shndx);
                            continue;
                        }
                        else
                        {
                            /* Internal symbol. Must be converted
                             * to absolute symbol.*/
                            sym_value = symbol->st_value;
                            /* Adds the address of the related section
                             * so the symbol stores absolute address. */
                            sym_value += ((section_table
                                           + symbol->st_shndx)->sh_addr);
                        }
                    }
                    switch (ELF32_R_TYPE(currel->r_info))
                    {
                        case R_386_NONE:
                            break;
                        case R_386_32:
                            /* Symbol value + offset. */
                            *target = sym_value + *target;
                            break;
                        case R_386_PC32:
                            /* Symbol value + offset - absolute address
                             * of the modified field. */
                            *target = (sym_value + (*target)
                                       - (unsigned long)target);
                            break;
                        default:
                            printf("Unknown relocation type in ELF file\n");
                    }
                }
            }
        }
    }

    *entry_point = exeStart + (elfHdr->e_entry - lowest_p_vaddr);

    /* Frees memory not needed by the process. */
    free(elfHdr);
    free(program_table);
    free(section_table);
    free(elf_other_sections);

    if (*loadloc == NULL)
    {
        *loadloc = exeStart;
    }
    return (0);
}
#endif

#if NEED_MZ
static int exeloadLoadMZ(unsigned char **entry_point,
                         FILE *fp,
                         unsigned char **loadloc)
{
    Mz_hdr firstbit;

    /* The header size is in paragraphs,
     * so the smallest possible header is 16 bytes (paragraph) long.
     * Next is the magic number checked. */
    if ((fseek(fp, 0, SEEK_SET) != 0)
        || (fread(&firstbit, 16, 1, fp) != 1)
        || (memcmp(firstbit.magic, "MZ", 2) != 0
            && memcmp(&firstbit.magic, "ZM", 2) != 0))
    {
        return (1);
    }
    if (firstbit.header_size == 0)
    {
        printf("MZ Header has 0 size\n");
        return (2);
    }
    if (firstbit.header_size * 16 > sizeof(firstbit))
    {
        printf("MZ Header is too large, size: %u\n",
               (unsigned int)firstbit.header_size * 16);
        return (2);
    }
    /* Loads the rest of the header. */
    if (fread(((char *)&firstbit) + 16,
              (firstbit.header_size - 1) * 16,
              1,
              fp) != 1)
    {
        printf("Error occured while reading MZ header\n");
        return (2);
    }
    /* Determines whether the executable has extensions or is a pure MZ.
     * Extensions are at offset in e_lfanew,
     * so the header must have at least 4 paragraphs. */
    if ((firstbit.header_size >= 4)
        && (firstbit.e_lfanew != 0))
    {
        int ret;

        /* Same logic as in exeloadDoload(). */
        ret = exeloadLoadPE(entry_point, fp, loadloc, firstbit.e_lfanew);
        if (ret == 1)
            ret = exeloadLoadLX(entry_point, fp, firstbit.e_lfanew);
        if (ret == 1)
            ret = exeloadLoadNE(entry_point, fp, loadloc, firstbit.e_lfanew);
        if (ret == 1)
            printf("Unknown MZ extension\n");
        return (ret);
    }
    /* Pure MZ executables are for 16-bit DOS, so we cannot run them. */
    printf("Pure MZ executables are not supported\n");

    return (2);
}

#ifdef __CC64__
$callback
#endif
static int dummyfunc(void)
{
    printf("in dummyfunc\n");
    return (0);
}

#ifdef __CC64__
$callback
#endif
static void undmain(void)
{
    return;
}

#ifdef W32HACK
void w64exit(int status);
void w32puts(void);
#endif

#ifdef W64HACK
int getmainargs(int *_Argc, char ***_Argv);
void w64exit(int status);
#endif

static int exeloadLoadPE(unsigned char **entry_point,
                         FILE *fp,
                         unsigned char **loadloc,
                         unsigned long e_lfanew)
{
    Coff_hdr coff_hdr;
    Pe32_optional_hdr *optional_hdr;
    Coff_section *section_table, *section;
    unsigned char *exeStart;
    int ret;

    {
        unsigned char firstbit[4];

        if ((fseek(fp, e_lfanew, SEEK_SET) != 0)
            || (fread(firstbit, 4, 1, fp) != 1)
            || (memcmp(firstbit, "PE\0\0", 4) != 0))
        {
            return (1);
        }
    }
    if (fread(&coff_hdr, sizeof(coff_hdr), 1, fp) != 1)
    {
        printf("Error occured while reading COFF header\n");
        return (2);
    }
#if TARGET_64BIT
    if (coff_hdr.Machine != IMAGE_FILE_MACHINE_AMD64)
    {
        printf("only 64-bit PE programs are supported\n");
        return (2);
    }
#else
    if ((coff_hdr.Machine != IMAGE_FILE_MACHINE_UNKNOWN)
        && (coff_hdr.Machine != IMAGE_FILE_MACHINE_I386))
    {
        if (coff_hdr.Machine == IMAGE_FILE_MACHINE_AMD64)
        {
            printf("64-bit PE programs are not supported\n");
        }
        else
        {
            printf("Unknown PE machine type: %04x\n", coff_hdr.Machine);
        }
        return (2);
    }
#endif

    if ((coff_hdr.Characteristics & IMAGE_FILE_RELOCS_STRIPPED) != 0)
    {
        printf("only relocatable executables supported\n");
        return (2);
    }

    optional_hdr = malloc(coff_hdr.SizeOfOptionalHeader);
    if (optional_hdr == NULL)
    {
        printf("Insufficient memory to load PE optional header\n");
        return (2);
    }
    if (fread(optional_hdr, coff_hdr.SizeOfOptionalHeader, 1, fp) != 1)
    {
        printf("Error occured while reading PE optional header\n");
        free(optional_hdr);
        return (2);
    }
#if TARGET_64BIT
    if (optional_hdr->Magic != MAGIC_PE32PLUS)
    {
        printf("Unknown PE optional header magic: %04x\n",
               optional_hdr->Magic);
        free(optional_hdr);
        return (2);
    }
#else
    if (optional_hdr->Magic != MAGIC_PE32)
    {
        printf("Unknown PE optional header magic: %04x\n",
               optional_hdr->Magic);
        free(optional_hdr);
        return (2);
    }
#endif

    section_table = malloc(coff_hdr.NumberOfSections * sizeof(Coff_section));
    if (section_table == NULL)
    {
        printf("Insufficient memory to load PE section headers\n");
        free(optional_hdr);
        return (2);
    }
    if (fread(section_table,
              coff_hdr.NumberOfSections * sizeof(Coff_section),
              1,
              fp) != 1)
    {
        printf("Error occured while reading PE optional header\n");
        free(section_table);
        free(optional_hdr);
        return (2);
    }

    /* Allocates memory for the process.
     * Size of image is obtained from the optional header. */
    if (*loadloc != NULL)
    {
        exeStart = *loadloc;
    }
    else
    {
        exeStart = malloc(optional_hdr->SizeOfImage);
    }
    if (exeStart == NULL)
    {
        printf("Insufficient memory to load PE program\n");
        free(section_table);
        free(optional_hdr);
        return (2);
    }

    /* Loads all sections at their addresses. */
    for (section = section_table;
         section < section_table + coff_hdr.NumberOfSections;
         section++)
    {
        /* Virtual size of a section is larger than in file,
         * so the difference is filled with 0. */
        if (section->VirtualSize > section->SizeOfRawData)
        {
            memset(exeStart
                   + section->VirtualAddress
                   + section->SizeOfRawData,
                   0,
                   section->VirtualSize - section->SizeOfRawData);
        }
        if (section->SizeOfRawData != 0)
        {
            if ((fseek(fp, section->PointerToRawData, SEEK_SET) != 0)
                || (fread(exeStart + section->VirtualAddress,
                         section->SizeOfRawData,
                         1,
                         fp) != 1))
            {
                printf("Error occured while reading PE section\n");
                free(section_table);
                free(optional_hdr);
                return (2);
            }
        }
    }

    if (exeStart != optional_hdr->ImageBase)
    {
        /* Relocations are not stripped, so the executable can be relocated. */
        if (optional_hdr->NumberOfRvaAndSizes > DATA_DIRECTORY_REL)
        {
            IMAGE_DATA_DIRECTORY *data_dir = (((IMAGE_DATA_DIRECTORY *)
                                               (optional_hdr + 1))
                                              + DATA_DIRECTORY_REL);
            /* Difference between preferred address and real address. */
            ptrdiff_t image_diff;
            int lower_exeStart;
            Base_relocation_block *rel_block = ((Base_relocation_block *)
                                                (exeStart
                                                 + (data_dir
                                                    ->VirtualAddress)));
            Base_relocation_block *end_rel_block;

            if (optional_hdr->ImageBase > exeStart)
            {
                /* Image is loaded at lower address than preferred. */
#ifdef W32HACK
                image_diff = (unsigned char *)optional_hdr->ImageBase
                             - exeStart;
#else
                image_diff = optional_hdr->ImageBase - exeStart;
#endif
                lower_exeStart = 1;
            }
            else
            {
                image_diff = exeStart - optional_hdr->ImageBase;
                lower_exeStart = 0;
            }

            end_rel_block = rel_block + ((data_dir->Size)
                                         / sizeof(Base_relocation_block));

            for (; rel_block < end_rel_block;)
            {
                unsigned short *cur_rel = (unsigned short *)rel_block;
                unsigned short *end_rel;

                end_rel = (cur_rel
                           + ((rel_block->BlockSize)
                              / sizeof(unsigned short)));
                cur_rel = (unsigned short *)(rel_block + 1);

                for (; cur_rel < end_rel; cur_rel++)
                {
                    /* Top 4 bits indicate the type of relocation. */
                    unsigned char rel_type = (unsigned char)((*cur_rel) >> 12);
                    unsigned char *rel_target = (exeStart + (rel_block->PageRva)
                                        + ((*cur_rel) & 0x0fff));

                    if (rel_type == IMAGE_REL_BASED_ABSOLUTE) continue;
                    if (rel_type == IMAGE_REL_BASED_HIGHLOW)
                    {
                        if (lower_exeStart)
                            *(unsigned char **)rel_target -= image_diff;
                        else *(unsigned char **)rel_target += image_diff;
                    }
#if TARGET_64BIT
                    else if (rel_type == IMAGE_REL_BASED_DIR64)
                    {
                        if (lower_exeStart)
                            *(unsigned char **)rel_target -= image_diff;
                        else *(unsigned char **)rel_target += image_diff;
                    }
#endif
                    else
                    {
                        printf("Unknown PE relocation type: %u\n",
                               rel_type);
                    }
                }

                rel_block = (Base_relocation_block *)cur_rel;
            }
        }
    }

    if (optional_hdr->NumberOfRvaAndSizes > DATA_DIRECTORY_IMPORT_TABLE)
    {
        IMAGE_DATA_DIRECTORY *data_dir = (((IMAGE_DATA_DIRECTORY *)
                                           (optional_hdr + 1))
                                          + DATA_DIRECTORY_IMPORT_TABLE);
        if (data_dir->Size != 0)
        {
            IMAGE_IMPORT_DESCRIPTOR *import_desc = ((void *)
                                                    (exeStart
                                                     + (data_dir
                                                        ->VirtualAddress)));

            /* Each descriptor is for one DLL
             * and the array has a null terminator. */
            for (; import_desc->OriginalFirstThunk; import_desc++)
            {
#ifdef W64HACK
                unsigned long long *thunk;
                
                for (thunk = (void *)(exeStart + (import_desc->FirstThunk));
                     *thunk != 0;
                     thunk++)
                {
                    if ((*thunk) & 0x8000000000000000ULL)
                    {
                        /* Bit 63 set, import by ordinal. */
                        printf("ordinal import not supported\n");
                        return (2);
                    }
                    else
                    {
                        /* Import by name. */
                        unsigned char *hintname = exeStart + ((*thunk) & 0x7fffffffffffffffULL);
                        int i;

                        /* The first 2 bytes are hint index,
                         * so they are skipped to get the name. */
                        hintname += 2;
                        /* printf("hintname is X%sX\n", hintname); */
                        if (strcmp((char *)hintname, "puts") == 0)
                        {
                            *thunk = (unsigned long long)puts;
                        }
                        else if (strcmp((char *)hintname, "exit") == 0)
                        {
                            *thunk = (unsigned long long)w64exit;
                        }
                        else if (strcmp((char *)hintname, "printf") == 0)
                        {
                            *thunk = (unsigned long long)printf;
                        }
                        else if (strcmp((char *)hintname, "malloc") == 0)
                        {
                            *thunk = (unsigned long long)malloc;
                        }
                        else if (strcmp((char *)hintname, "abort") == 0)
                        {
                            *thunk = (unsigned long long)abort;
                        }
                        else if (strcmp((char *)hintname, "atexit") == 0)
                        {
                            *thunk = (unsigned long long)atexit;
                        }
                        else if (strcmp((char *)hintname, "atoi") == 0)
                        {
                            *thunk = (unsigned long long)atoi;
                        }
                        else if (strcmp((char *)hintname, "strcpy") == 0)
                        {
                            *thunk = (unsigned long long)strcpy;
                        }
                        else if (strcmp((char *)hintname, "strcmp") == 0)
                        {
                            *thunk = (unsigned long long)strcmp;
                        }
                        else if (strcmp((char *)hintname, "strerror") == 0)
                        {
                            *thunk = (unsigned long long)strerror;
                        }
                        else if (strcmp((char *)hintname, "strlen") == 0)
                        {
                            *thunk = (unsigned long long)strlen;
                        }
                        else if (strcmp((char *)hintname, "strncmp") == 0)
                        {
                            *thunk = (unsigned long long)strncmp;
                        }
                        else if (strcmp((char *)hintname, "strrchr") == 0)
                        {
                            *thunk = (unsigned long long)strrchr;
                        }
                        else if (strcmp((char *)hintname, "strstr") == 0)
                        {
                            *thunk = (unsigned long long)strstr;
                        }
                        else if (strcmp((char *)hintname, "system") == 0)
                        {
                            *thunk = (unsigned long long)system;
                        }
                        else if (strcmp((char *)hintname, "fgets") == 0)
                        {
                            *thunk = (unsigned long long)fgets;
                        }
                        else if (strcmp((char *)hintname, "fputs") == 0)
                        {
                            *thunk = (unsigned long long)fputs;
                        }
                        else if (strcmp((char *)hintname, "feof") == 0)
                        {
                            *thunk = (unsigned long long)feof;
                        }
                        else if (strcmp((char *)hintname, "ferror") == 0)
                        {
                            *thunk = (unsigned long long)ferror;
                        }
                        else if (strcmp((char *)hintname, "fread") == 0)
                        {
                            *thunk = (unsigned long long)fread;
                        }
                        else if (strcmp((char *)hintname, "fseek") == 0)
                        {
                            *thunk = (unsigned long long)fseek;
                        }
                        else if (strcmp((char *)hintname, "ftell") == 0)
                        {
                            *thunk = (unsigned long long)ftell;
                        }
                        else if (strcmp((char *)hintname, "fwrite") == 0)
                        {
                            *thunk = (unsigned long long)fwrite;
                        }
                        else if (strcmp((char *)hintname, "fflush") == 0)
                        {
                            *thunk = (unsigned long long)fflush;
                        }
                        else if (strcmp((char *)hintname, "setvbuf") == 0)
                        {
                            *thunk = (unsigned long long)setvbuf;
                        }
                        else if (strcmp((char *)hintname, "fputc") == 0)
                        {
                            *thunk = (unsigned long long)fputc;
                        }
                        else if (strcmp((char *)hintname, "tmpfile") == 0)
                        {
                            *thunk = (unsigned long long)tmpfile;
                        }
                        else if (strcmp((char *)hintname, "vfprintf") == 0)
                        {
                            *thunk = (unsigned long long)vfprintf;
                        }
                        else if (strcmp((char *)hintname, "vsprintf") == 0)
                        {
                            *thunk = (unsigned long long)vsprintf;
                        }
                        else if (strcmp((char *)hintname, "isalnum") == 0)
                        {
                            *thunk = (unsigned long long)isalnum;
                        }
                        else if (strcmp((char *)hintname, "isalpha") == 0)
                        {
                            *thunk = (unsigned long long)isalpha;
                        }
                        else if (strcmp((char *)hintname, "tolower") == 0)
                        {
                            *thunk = (unsigned long long)tolower;
                        }
                        else if (strcmp((char *)hintname, "toupper") == 0)
                        {
                            *thunk = (unsigned long long)toupper;
                        }
                        else if (strcmp((char *)hintname, "iscntrl") == 0)
                        {
                            *thunk = (unsigned long long)iscntrl;
                        }
                        else if (strcmp((char *)hintname, "isdigit") == 0)
                        {
                            *thunk = (unsigned long long)isdigit;
                        }
                        else if (strcmp((char *)hintname, "islower") == 0)
                        {
                            *thunk = (unsigned long long)islower;
                        }
                        else if (strcmp((char *)hintname, "isprint") == 0)
                        {
                            *thunk = (unsigned long long)isprint;
                        }
                        else if (strcmp((char *)hintname, "isupper") == 0)
                        {
                            *thunk = (unsigned long long)isupper;
                        }
                        else if (strcmp((char *)hintname, "isxdigit") == 0)
                        {
                            *thunk = (unsigned long long)isxdigit;
                        }
                        else if (strcmp((char *)hintname, "fopen") == 0)
                        {
                            *thunk = (unsigned long long)fopen;
                        }
                        else if (strcmp((char *)hintname, "fprintf") == 0)
                        {
                            *thunk = (unsigned long long)fprintf;
                        }
                        else if (strcmp((char *)hintname, "fgetc") == 0)
                        {
                            *thunk = (unsigned long long)fgetc;
                        }
                        else if (strcmp((char *)hintname, "getc") == 0)
                        {
                            *thunk = (unsigned long long)getc;
                        }
                        else if (strcmp((char *)hintname, "putc") == 0)
                        {
                            *thunk = (unsigned long long)putc;
                        }
                        else if (strcmp((char *)hintname, "getchar") == 0)
                        {
                            *thunk = (unsigned long long)getchar;
                        }
                        else if (strcmp((char *)hintname, "ungetc") == 0)
                        {
                            *thunk = (unsigned long long)ungetc;
                        }
                        else if (strcmp((char *)hintname, "sscanf") == 0)
                        {
                            *thunk = (unsigned long long)sscanf;
                        }
                        else if (strcmp((char *)hintname, "sprintf") == 0)
                        {
                            *thunk = (unsigned long long)sprintf;
                        }
                        else if (strcmp((char *)hintname, "remove") == 0)
                        {
                            *thunk = (unsigned long long)remove;
                        }
                        else if (strcmp((char *)hintname, "qsort") == 0)
                        {
                            *thunk = (unsigned long long)qsort;
                        }
                        else if (strcmp((char *)hintname, "rewind") == 0)
                        {
                            *thunk = (unsigned long long)rewind;
                        }
                        else if (strcmp((char *)hintname, "free") == 0)
                        {
                            *thunk = (unsigned long long)free;
                        }
                        else if (strcmp((char *)hintname, "isspace") == 0)
                        {
                            *thunk = (unsigned long long)isspace;
                        }
                        else if (strcmp((char *)hintname, "memcpy") == 0)
                        {
                            *thunk = (unsigned long long)memcpy;
                        }
                        else if (strcmp((char *)hintname, "memcmp") == 0)
                        {
                            *thunk = (unsigned long long)memcmp;
                        }
                        else if (strcmp((char *)hintname, "memset") == 0)
                        {
                            *thunk = (unsigned long long)memset;
                        }
                        else if (strcmp((char *)hintname, "memmove") == 0)
                        {
                            *thunk = (unsigned long long)memmove;
                        }
                        else if (strcmp((char *)hintname, "realloc") == 0)
                        {
                            *thunk = (unsigned long long)realloc;
                        }
                        else if (strcmp((char *)hintname, "strcat") == 0)
                        {
                            *thunk = (unsigned long long)strcat;
                        }
                        else if (strcmp((char *)hintname, "strchr") == 0)
                        {
                            *thunk = (unsigned long long)strchr;
                        }
                        else if (strcmp((char *)hintname, "strcspn") == 0)
                        {
                            *thunk = (unsigned long long)strcspn;
                        }
                        else if (strcmp((char *)hintname, "strtoul") == 0)
                        {
                            *thunk = (unsigned long long)strtoul;
                        }
                        else if (strcmp((char *)hintname, "strncpy") == 0)
                        {
                            *thunk = (unsigned long long)strncpy;
                        }
                        else if (strcmp((char *)hintname, "strtod") == 0)
                        {
                            *thunk = (unsigned long long)strtod;
                        }
                        else if (strcmp((char *)hintname, "ceil") == 0)
                        {
                            *thunk = (unsigned long long)ceil;
                        }
                        else if (strcmp((char *)hintname, "labs") == 0)
                        {
                            *thunk = (unsigned long long)labs;
                        }
                        else if (strcmp((char *)hintname, "sqrt") == 0)
                        {
                            *thunk = (unsigned long long)sqrt;
                        }
                        else if (strcmp((char *)hintname, "clock") == 0)
                        {
                            *thunk = (unsigned long long)clock;
                        }
                        else if (strcmp((char *)hintname, "time") == 0)
                        {
                            *thunk = (unsigned long long)time;
                        }
                        else if (strcmp((char *)hintname, "localtime") == 0)
                        {
                            *thunk = (unsigned long long)localtime;
                        }
                        else if (strcmp((char *)hintname, "fclose") == 0)
                        {
                            *thunk = (unsigned long long)fclose;
                        }
                        else if (strcmp((char *)hintname, "__iob_func") == 0)
                        {
                            *thunk = (unsigned long long)__iob_func;
                        }
                        else if (strcmp((char *)hintname, "_errno") == 0)
                        {
                            *thunk = (unsigned long long)_errno;
                        }
                        else if (strcmp((char *)hintname, "_assert") == 0)
                        {
                            *thunk = (unsigned long long)_assert;
                        }
                        else if (strcmp((char *)hintname, "__getmainargs") == 0)
                        {
                            *thunk = (unsigned long long)getmainargs;
                        }
                        /* mingw64 is generating this, so it must be in msvcrt.dll */
                        else if (strcmp((char *)hintname, "__main") == 0)
                        {
                            *thunk = (unsigned long long)undmain;
                        }
                        else
                        {
                            printf("unknown hintname %s\n", hintname);
                            *thunk = (unsigned long long)dummyfunc;
                        }
                    }
                }
#elif defined(W32HACK)
                unsigned long *thunk;
                
                for (thunk = (void *)(exeStart + (import_desc->FirstThunk));
                     *thunk != 0;
                     thunk++)
                {
                    if ((*thunk) & 0x80000000UL)
                    {
                        /* Bit 31 set, import by ordinal. */
                        printf("ordinal import not supported\n");
                        return (2);
                    }
                    else
                    {
                        /* Import by name. */
                        unsigned char *hintname = exeStart + ((*thunk) & 0x7fffffffUL);
                        int i;

                        /* The first 2 bytes are hint index,
                         * so they are skipped to get the name. */
                        hintname += 2;
                        /* printf("hintname is X%sX\n", hintname); */
                        if (strcmp(hintname, "exit") == 0)
                        {
                            *thunk = (unsigned long)w64exit;
                        }
                        else if (strcmp(hintname, "puts") == 0)
                        {
                            *thunk = (unsigned long)w32puts;
                        }
#if 0
                        if (strcmp(hintname, "puts") == 0)
                        {
                            *thunk = (unsigned long long)puts;
                        }
                        else if (strcmp(hintname, "exit") == 0)
                        {
                            *thunk = (unsigned long long)w64exit;
                        }
                        else if (strcmp(hintname, "printf") == 0)
                        {
                            *thunk = (unsigned long long)printf;
                        }
                        else if (strcmp(hintname, "malloc") == 0)
                        {
                            *thunk = (unsigned long long)malloc;
                        }
                        else if (strcmp(hintname, "strcpy") == 0)
                        {
                            *thunk = (unsigned long long)strcpy;
                        }
                        else if (strcmp(hintname, "__getmainargs") == 0)
                        {
                            *thunk = (unsigned long long)getmainargs;
                        }
#endif
                        else
                        {
                            *thunk = (unsigned long)dummyfunc;
                        }
                    }
                }
#else
                ret = exeloadLoadPEDLL(exeStart, import_desc);
                if (ret != 0)
                {
                    printf("Cannot load DLL %s, ret %d\n",
                           exeStart + import_desc->Name, ret);
                    return (2);
                }
#endif
            }
        }
    }

    *entry_point = exeStart + optional_hdr->AddressOfEntryPoint;
    /* Frees memory not needed by the process. */
    free(section_table);
    free(optional_hdr);
    if (*loadloc == NULL)
    {
        *loadloc = exeStart;
    }
    return (0);
}


static int exeloadLoadPEDLL(unsigned char *exeStart,
                            IMAGE_IMPORT_DESCRIPTOR *import_desc)
{
    Mz_hdr mzFirstbit;
    FILE *fp;
    Coff_hdr coff_hdr;
    Pe32_optional_hdr *optional_hdr;
    Coff_section *section_table, *section;
    unsigned char *dllStart;
    long newpos;
    size_t readbytes;
    char *name1;
    char *name2;

    /* PE DLL is being loaded,
     * so it is loaded in the same way as PE executable,
     * but the MZ stage is integrated. */
    name1 = (char *)exeStart + import_desc->Name;
    if ((strcmp(name1, "kernel32.dll") == 0)
        || (strcmp(name1, "KERNEL32.dll") == 0)
        || (strcmp(name1, "kernel32") == 0))
    {
        name2 = kernel32;
    }
    else if ((strcmp(name1, "msvcrt.dll") == 0)
             || (strcmp(name1, "msvcrt") == 0))
    {
        name2 = msvcrt;
    }
    else
    {
        name2 = name1;
    }
    fp = fopen(name2, "rb");
    if (fp == NULL)
    {
        printf("Failed to open %s for loading\n",
               exeStart + (import_desc->Name));
        return (1);
    }

    /* The header size is in paragraphs,
     * so the smallest possible header is 16 bytes (paragraph) long.
     * Next is the magic number checked. */
    if ((fseek(fp, 0, SEEK_SET) != 0)
        || (fread(&mzFirstbit, 16, 1, fp) != 1)
        || (memcmp(mzFirstbit.magic, "MZ", 2) != 0)
            && (memcmp(&mzFirstbit.magic, "ZM", 2) != 0))
    {
        fclose(fp);
        return (2);
    }
    if (mzFirstbit.header_size == 0)
    {
        printf("MZ Header has 0 size\n");
        fclose(fp);
        return (3);
    }
    if (mzFirstbit.header_size * 16 > sizeof(mzFirstbit))
    {
        printf("MZ Header is too large, size: %u\n",
               mzFirstbit.header_size * 16);
        fclose(fp);
        return (4);
    }
    /* Loads the rest of the header. */
    if (fread(((char *)&mzFirstbit) + 16,
              (mzFirstbit.header_size - 1) * 16,
              1,
              fp) != 1)
    {
        printf("Error occured while reading MZ header\n");
        return (5);
    }
    /* Determines whether the executable has extensions or is a pure MZ.
     * Extensions are at offset in e_lfanew,
     * so the header must have at least 4 paragraphs. */
    if ((mzFirstbit.header_size < 4)
        || (mzFirstbit.e_lfanew == 0))
    {
        fclose(fp);
        return (6);
    }

    /* PE stage begins. */
    {
        unsigned char firstbit[4];

        if ((fseek(fp, mzFirstbit.e_lfanew, SEEK_SET) != 0)
            || (fread(firstbit, 4, 1, fp) != 1)
            || (memcmp(firstbit, "PE\0\0", 4) != 0))
        {
            fclose(fp);
            return (7);
        }
    }
    if (fread(&coff_hdr, sizeof(coff_hdr), 1, fp) != 1)
    {
        printf("Error occured while reading COFF header\n");
        fclose(fp);
        return (8);
    }
    if ((coff_hdr.Machine != IMAGE_FILE_MACHINE_UNKNOWN)
        && (coff_hdr.Machine != IMAGE_FILE_MACHINE_I386))
    {
        if (coff_hdr.Machine == IMAGE_FILE_MACHINE_AMD64)
        {
            printf("64-bit PE programs are not supported\n");
        }
        else
        {
            printf("Unknown PE machine type: %04x\n", coff_hdr.Machine);
        }
        fclose(fp);
        return (9);
    }

    if (coff_hdr.Characteristics & IMAGE_FILE_RELOCS_STRIPPED)
    {
        printf("only relocatable DLLs are supported\n");
        fclose(fp);
        return (10);
    }

    optional_hdr = malloc(coff_hdr.SizeOfOptionalHeader);
    if (optional_hdr == NULL)
    {
        printf("Insufficient memory to load PE optional header\n");
        fclose(fp);
        return (11);
    }
    if (fread(optional_hdr, coff_hdr.SizeOfOptionalHeader, 1, fp) != 1)
    {
        printf("Error occured while reading PE optional header\n");
        free(optional_hdr);
        fclose(fp);
        return (12);
    }
    if (optional_hdr->Magic != MAGIC_PE32)
    {
        printf("Unknown PE optional header magic: %04x\n",
               optional_hdr->Magic);
        free(optional_hdr);
        fclose(fp);
        return (13);
    }

    section_table = malloc(coff_hdr.NumberOfSections * sizeof(Coff_section));
    if (section_table == NULL)
    {
        printf("Insufficient memory to load PE section headers\n");
        free(optional_hdr);
        fclose(fp);
        return (14);
    }
    if (fread(section_table,
              coff_hdr.NumberOfSections * sizeof(Coff_section),
              1,
              fp) != 1)
    {
        printf("Error occured while reading PE optional header\n");
        free(section_table);
        free(optional_hdr);
        fclose(fp);
        return (15);
    }

    /* Allocates memory for the process.
     * Size of image is obtained from the optional header. */
    dllStart = malloc(optional_hdr->SizeOfImage);
    if (dllStart == NULL)
    {
        printf("Insufficient memory to load PE program\n");
        free(section_table);
        free(optional_hdr);
        fclose(fp);
        return (16);
    }

    /* Loads all sections at their addresses. */
    for (section = section_table;
         section < section_table + (coff_hdr.NumberOfSections);
         section++)
    {
        /* Virtual size of a section is larger than in file,
         * so the difference is filled with 0. */
        if ((section->VirtualSize) > (section->SizeOfRawData))
        {
            memset((dllStart
                    + (section->VirtualAddress)
                    + (section->SizeOfRawData)),
                   0,
                   (section->VirtualSize) - (section->SizeOfRawData));
        }
        if ((fseek(fp, section->PointerToRawData, SEEK_SET) != 0)
            || fread(dllStart + (section->VirtualAddress),
                     section->SizeOfRawData,
                     1,
                     fp) != 1)
        {
            printf("Error occured while reading PE section\n");
            free(section_table);
            free(optional_hdr);
            fclose(fp);
            return (17);
        }
    }
    fclose(fp);

    if (dllStart != optional_hdr->ImageBase)
    {
        /* Relocations are not stripped, so the executable can be relocated. */
        if (optional_hdr->NumberOfRvaAndSizes > DATA_DIRECTORY_REL)
        {
            IMAGE_DATA_DIRECTORY *data_dir = (((IMAGE_DATA_DIRECTORY *)
                                               (optional_hdr + 1))
                                              + DATA_DIRECTORY_REL);
            /* Difference between preferred address and real address. */
            unsigned long image_diff;
            int lower_dllStart;
            Base_relocation_block *rel_block = ((Base_relocation_block *)
                                                (dllStart
                                                 + (data_dir
                                                    ->VirtualAddress)));
            Base_relocation_block *end_rel_block;

            if (optional_hdr->ImageBase > dllStart)
            {
                /* Image is loaded  at lower address than preferred. */
#ifdef W32HACK
                image_diff = (unsigned char *)optional_hdr->ImageBase
                             - dllStart;
#else
                image_diff = optional_hdr->ImageBase
                             - dllStart;
#endif
                lower_dllStart = 1;
            }
            else
            {
                image_diff = dllStart
                             - optional_hdr->ImageBase;
                lower_dllStart = 0;
            }
            end_rel_block = rel_block + ((data_dir->Size)
                                         / sizeof(Base_relocation_block));

            for (; rel_block < end_rel_block;)
            {
                unsigned short *cur_rel = (unsigned short *)rel_block;
                unsigned short *end_rel;

                end_rel = (cur_rel
                           + ((rel_block->BlockSize)
                              / sizeof(unsigned short)));
                cur_rel = (unsigned short *)(rel_block + 1);

                for (; cur_rel < end_rel; cur_rel++)
                {
                    /* Top 4 bits indicate the type of relocation. */
                    unsigned char rel_type = (*cur_rel) >> 12;
                    void *rel_target = (dllStart + (rel_block->PageRva)
                                        + ((*cur_rel) & 0x0fff));

                    if (rel_type == IMAGE_REL_BASED_ABSOLUTE) continue;
                    if (rel_type == IMAGE_REL_BASED_HIGHLOW)
                    {
                        if (lower_dllStart)
                            (*((unsigned long *)rel_target)) -= image_diff;
                        else (*((unsigned long *)rel_target)) += image_diff;
                    }
                    else
                    {
                        printf("Unknown PE relocation type: %u\n",
                               rel_type);
                    }
                }

                rel_block = (Base_relocation_block *)cur_rel;
            }
        }
    }

    if (optional_hdr->NumberOfRvaAndSizes > DATA_DIRECTORY_EXPORT_TABLE)
    {
        IMAGE_DATA_DIRECTORY *data_dir = (((IMAGE_DATA_DIRECTORY *)
                                           (optional_hdr + 1))
                                          + DATA_DIRECTORY_EXPORT_TABLE);
        IMAGE_EXPORT_DIRECTORY *export_dir;
        unsigned long *functionTable;
        unsigned long *nameTable;
        unsigned short *ordinalTable;
        unsigned long *thunk;

        if (data_dir->Size == 0)
        {
            printf("DLL has no export tables\n");
            free(section_table);
            free(optional_hdr);
            return (18);
        }

        export_dir = ((void *)(dllStart + (data_dir->VirtualAddress)));
        functionTable = (void *)(dllStart + (export_dir->AddressOfFunctions));
        nameTable = (void *)(dllStart + (export_dir->AddressOfNames));
        ordinalTable = (void *)(dllStart
                                + (export_dir->AddressOfNameOrdinals));

        /* The importing itself.
         * Import Address Table of the executable is parsed
         * and function addresses are written
         * when they are found. */
        for (thunk = (void *)(exeStart + (import_desc->FirstThunk));
             *thunk != 0;
             thunk++)
        {
            if ((*thunk) & 0x80000000)
            {
                /* Bit 31 set, import by ordinal. */
                *thunk = ((unsigned long)
                          (dllStart
                           + (functionTable[((*thunk) & 0x7fffffff)
                                            - (export_dir->Base)])));
            }
            else
            {
                /* Import by name. */
                unsigned char *hintname = exeStart + ((*thunk) & 0x7fffffff);
                int i;

                /* The first 2 bytes are hint index,
                 * so they are skipped to get the name. */
                hintname += 2;
                for (i = 0; i < (export_dir->NumberOfNames); i++)
                {
                    if (strcmp((char *)hintname, (char *)dllStart + (nameTable[i]))) continue;
                    break;
                }
                if (i == (export_dir->NumberOfNames))
                {
                    printf("Function %s not found in DLL\n", hintname);
                    free(section_table);
                    free(optional_hdr);
                    return (1);
                }
                /* PE specification says that the ordinals in ordinal table
                 * are biased by Ordinal Base,
                 * but it does not seem to be correct. */
                *thunk = (unsigned long)(dllStart
                                         + (functionTable[ordinalTable[i]]));
            }
        }
        /* After the DLL is bound, time/date stamp is copied. */
        import_desc->TimeDateStamp = export_dir->TimeDateStamp;
    }
    if (optional_hdr->NumberOfRvaAndSizes > DATA_DIRECTORY_IMPORT_TABLE)
    {
        IMAGE_DATA_DIRECTORY *data_dir = (((IMAGE_DATA_DIRECTORY *)
                                           (optional_hdr + 1))
                                          + DATA_DIRECTORY_IMPORT_TABLE);
        if (data_dir->Size != 0)
        {
            IMAGE_IMPORT_DESCRIPTOR *import_desc = ((void *)
                                                    (dllStart
                                                     + (data_dir
                                                        ->VirtualAddress)));

            /* Each descriptor is for one DLL
             * and the array has a null terminator. */
            for (; import_desc->OriginalFirstThunk; import_desc++)
            {
                if (exeloadLoadPEDLL(dllStart, import_desc))
                {
                    printf("Failed to load DLL %s\n",
                           dllStart + (import_desc->Name));
                    free(section_table);
                    free(optional_hdr);
                    return (19);
                }
            }
        }
    }

#ifndef NO_DLLENTRY
    /* Entry point is optional for DLLs. */
    if (optional_hdr->AddressOfEntryPoint)
    {
        int ret = callDllEntry(dllStart + (optional_hdr->AddressOfEntryPoint),
                               dllStart,
                               1, /* fdwReason = DLL_PROCESS_ATTACH */
                               (void *)1); /* lpvReserved = non-NULL */

        if (ret == 0)
        {
            printf("Initialization of DLL %s failed\n",
                   exeStart + (import_desc->Name));
            free(section_table);
            free(optional_hdr);
            return (20);
        }
    }
#endif

    /* Frees memory not needed by the DLL. */
    free(section_table);
    free(optional_hdr);

    return (0);
}

static int exeloadLoadLX(unsigned char **entry_point,
                         FILE *fp,
                         unsigned long e_lfanew)
{
    unsigned char firstbit[2];
    long newpos;
    size_t readbytes;

    if ((fseek(fp, e_lfanew, SEEK_SET) != 0)
        || (fread(firstbit, sizeof(firstbit), 1, fp) != 1)
        || (memcmp(firstbit, "LX", 2) != 0))
    {
        return (1);
    }
    /* LX seems to be called LE sometimes. */
    printf("LX (other name LE) is not supported\n");

    return (2);
}

extern int __shift;
extern int __incr;

static int exeloadLoadNE(unsigned char **entry_point,
                         FILE *fp,
                         unsigned char **loadloc,
                         unsigned long e_lfanew)
{
    unsigned char firstbit[0x40];
    long newpos;
    size_t readbytes;
    unsigned char segtable[8];
    unsigned long offs1;
    unsigned int len1;
    unsigned long offs2;
    unsigned int len2;
    unsigned char numreloc[2];
    unsigned char reloc[8];
    unsigned char *codeptr;
    unsigned char *dataptr;
    int nr;
    int x;
    unsigned int lloffs;
    unsigned char zzz[2];
    unsigned int zzz_num;
    unsigned char *csalias;

    if ((fseek(fp, e_lfanew, SEEK_SET) != 0)
        || (fread(firstbit, sizeof(firstbit), 1, fp) != 1)
        || (memcmp(firstbit, "NE", 2) != 0))
    {
        return (1);
    }
    
#ifdef __32BIT__
    printf("NE not supported for 32-bit\n");
    return (2);
#else

    fread(segtable, sizeof(segtable), 1, fp);
    offs1 = segtable[0] | (segtable[1] << 8);
    offs1 *= 2;
    len1 = segtable[2] | (segtable[3] << 8);

    fread(segtable, sizeof(segtable), 1, fp);
    offs2 = segtable[0] | (segtable[1] << 8);
    offs2 *= 2;
    len2 = segtable[2] | (segtable[3] << 8);

    if (*loadloc == NULL)
    {
#if 0
        *loadloc = malloc(65536UL * 2 + 0x100 + 48 + 12);
#else
        *loadloc = malloc(65000UL);
        if (*loadloc != NULL)
        {
            unsigned short seg;
            seg = (unsigned long)(*loadloc) >> 16;
            if (DosCreateCSAlias(seg, &seg) != 0)
            {
                printf("failed to create executable memory\n");
                for (;;) ;
            }
            csalias = (void *)
                      (((unsigned long)seg << 16) | (unsigned int)*loadloc);
            /* printf("new load is %p\n", csalias); */
        }
#endif
        if (*loadloc == NULL)
        {
            printf("alloc failed\n");
            return (1);
        }
#if 0
        *loadloc += 12;
#endif
    }
    
    codeptr = *loadloc;
    /* printf("codeptr is %p\n", codeptr); */
    /* first 65536 is wasted - for alignment */
    /* second 65536 is for data */
    /* third 65535 is for "far" BSS */
    dataptr = calloc(1, 65536UL + 65536UL + 65536UL);
    /* printf("dataptr is %p\n", dataptr); */
    if (dataptr == NULL)
    {
        printf("malloc2 failed\n");
        return (1);
    }
    
    codeptr -= 4;
    dataptr += (65536UL - 4);
    
    /* skip first 4 bytes of code - should be NOPs */
    fseek(fp, offs1 + 4, SEEK_SET);

#if 0
    /* PDOS/86 needs this currently */
    fread(*loadloc + 0x100 + 48, len1, 1, fp);
#else
    fread(codeptr + 4, len1 - 4, 1, fp);
#endif

    fseek(fp, offs2, SEEK_SET);
#if 0
    /* PDOS/86 needs this currently */
    fread(*loadloc + 65536UL + 0x100 + 48, len2, 1, fp);
#else
    fread(dataptr, len2, 1, fp);
#endif

#if 0
    /* PDOS/86 needs this currently */
    codeptr = *loadloc + 0x100 + 48;
#else
    /* codeptr = *loadloc; */
#endif

#if 0
    dataptr = codeptr + 65536UL;
#endif

    fseek(fp, offs1 + len1, SEEK_SET);
    fread(numreloc, sizeof numreloc, 1, fp);
    
    nr = numreloc[0] | (numreloc[1] << 8);
    
    /* printf("nr is %d\n", nr); */
    for (x = 0; x < nr; x++)
    {
        /* 2 is segment, 3 is a far address,
           5 is an offset */
        fread(reloc, sizeof reloc, 1, fp);
        if ((reloc[0] != 2)
            && (reloc[0] != 3)
            && (reloc[0] != 5))
        {
            printf("unknown3 reloc type %x\n", reloc[0]);
            for (;;) ;
        }
#if 0
        printf("0 is %02X, 1 is %02X\n", reloc[0], reloc[1]);
#endif
        /* 0 = internal reference */
        /* 1 = import ordinal */
        /* 2 = import name */
        if ((reloc[1] != 0x00) && (reloc[1] != 0x02)
            && (reloc[1] != 0x01))
        {
            printf("unknown4 reloc type %x\n", reloc[1]);
            for (;;) ;
        }
        lloffs = reloc[2] | (reloc[3] << 8);
        while (1)
        {
            memcpy(zzz, codeptr + lloffs, 2);
            zzz_num = zzz[0] | (zzz[1] << 8);
            if (reloc[0] == 5)
            {
                /* only support an offset that is a reference
                   to AHINCR - not sure if AHSHIFT is also
                   required - if it is, need more work to find
                   out which variable this is */
                /* And for use on an 8086, the AHINCR is 0x1000,
                   which represents 64k when used as a segment
                   increment */
                /* AHSHIFT is 12 decimal - how many bits to shift
                   left to get to 0x1000 */
                /* Also note that although Windows returns a segment
                   of FFFF for its AHINCR and AHSHIFT functions, that
                   is not actually used for anything, so no effort has
                   been made to reproduce that. Also note that these
                   functions have two underscores in front of them
                   I think */
#if 0
                (codeptr + lloffs)[0] = 0x00;
                (codeptr + lloffs)[1] = 0x10;
#endif
                if (reloc[1] == 1)
                {
                    unsigned int ordnum;
                    
                    ordnum = (reloc[7] << 8) | reloc[6];
                    /* printf("ordnum is %u\n", ordnum); */
                    if (ordnum == 135) /* AHSHIFT or DosHugeShift */
                    {
                        (codeptr + lloffs)[0] = __shift & 0xff;
                        (codeptr + lloffs)[1] = __shift >> 8;
                    }
                    else if (ordnum == 136) /* AHINCR or DosHugeIncr */
                    {
                        (codeptr + lloffs)[0] = __incr & 0xff;
                        (codeptr + lloffs)[1] = __incr >> 8;
                    }
                    else
                    {
                        printf("unknown/unsupported ordinal %u\n", ordnum);
                        for (;;) ;
                    }
                }
                else if (reloc[1] == 0)
                {
                    unsigned int existing;
                    unsigned int newval;
                    
                    existing = (codeptr[lloffs + 1] << 8) | codeptr[lloffs];
                    existing = 0; /* seems doco is wrong - this is a chain */
                    newval = (reloc[7] << 8) + reloc[6];
                    existing += newval;
                    codeptr[lloffs] = existing & 0xff;
                    codeptr[lloffs + 1] = (existing >> 8) & 0xff;
                }
                else
                {
                    printf("unknown8 reloc %x\n", reloc[1]);
                    for (;;) ;
                }
            }
            else if (reloc[4] == 1) /* code */
            {
                codeptr[lloffs] = ((unsigned long)csalias >> 16) & 0xff;
                codeptr[lloffs + 1] = ((unsigned long)csalias >> 24) & 0xff;
            }
            else if (reloc[4] == 2) /* data */
            {
                codeptr[lloffs] = ((unsigned long)dataptr >> 16) & 0xff;
                codeptr[lloffs + 1] = ((unsigned long)dataptr >> 24) & 0xff;
            }
            if (zzz_num == 0xffffU) break;
            lloffs = zzz_num;
        }
    }

    fseek(fp, offs2 + len2, SEEK_SET);
    memset(numreloc, 0x00, sizeof numreloc);
    fread(numreloc, sizeof numreloc, 1, fp);
    nr = numreloc[0] | (numreloc[1] << 8);
    /* printf("nr is %x\n", nr); */
    
    for (x = 0; x < nr; x++)
    {
        unsigned int adjust = 0; /* zero? */
        
        fread(reloc, sizeof reloc, 1, fp);
        /* 2 = segment, 3 = far address */
        /* MSC only uses 2 */
        /* Watcom unknown */
        if (reloc[0] != 2)
        {
            printf("unknown1 reloc type %x\n", reloc[0]);
            for (;;) ;
        }
        if (reloc[0] == 3)
        {
            adjust = 0; /* make this 2? */
        }
        /* 0 = internal reference */
        if (reloc[1] != 0x0)
        {
            printf("unknown7 flag %x\n", reloc[1]);
            for (;;) ;
        }
        lloffs = reloc[2] | ((unsigned int)reloc[3] << 8);
        while (1)
        {
            memcpy(zzz, dataptr + lloffs + adjust, 2);

            zzz_num = zzz[0] | ((unsigned int)zzz[1] << 8);
            
            /* 4 = additive */
            if (reloc[1] == 0x04)
            {
                printf("not accepting additive for now\n");
                for (;;) ;
                if (reloc[4] == 1) /* code */
                {
                    *(unsigned int *)(dataptr + lloffs + 2)
                        = (((unsigned long)csalias >> 16) & 0xffffU);
                }
                else if (reloc[4] == 2) /* data */
                {
                    *(unsigned int *)(dataptr + lloffs + 2)
                        = (((unsigned long)dataptr >> 16) & 0xffffU);
                }
                memcpy(dataptr + lloffs, &reloc[6], 2);
                break;
            }
            /* 0 = internal reference */
            if (reloc[1] != 0x00)
            {
                printf("unknown2 reloc type %x\n", reloc[1]);
                for (;;) ;
            }
            if (reloc[4] == 1) /* code */
            {
                dataptr[lloffs + adjust] = ((unsigned long)csalias >> 16) & 0xff;
                dataptr[lloffs + adjust + 1] = ((unsigned long)csalias >> 24) & 0xff;
            }
            else if (reloc[4] == 2) /* data */
            {
                dataptr[lloffs + adjust] = ((unsigned long)dataptr >> 16) & 0xff;
                dataptr[lloffs + 1 + adjust] = ((unsigned long)dataptr >> 24) & 0xff;
            }
            else if (reloc[4] == 3) /* bss */
            {
                dataptr[lloffs + adjust] = ((unsigned long)(dataptr + 65536UL) >> 16) & 0xff;
                dataptr[lloffs + 1 + adjust] = ((unsigned long)(dataptr + 65536UL) >> 24) & 0xff;
            }
            else
            {
                printf("unknown reloc4 %x\n", reloc[4]);
                for (;;) ;
            }
            if (zzz_num == 0xffffU) break;
            lloffs = zzz_num;
        }
    }
#if 0
    *entry_point = codeptr;
#else
    *entry_point = csalias;
#endif
    return (0);
#endif
}

#endif /* NEED_MZ */

#if NEED_MVS
/* Structures here are documented in Appendix B and E of
   MVS Program Management: Advanced Facilities SA22-7644-14:
   http://publibfp.dhe.ibm.com/cgi-bin/bookmgr/BOOKS/iea2b2b1/CCONTENTS
   and the IEBCOPY component is documented here:
   Appendix B of OS/390 DFSMSdfp Utilities SC26-7343-00
   or Appendix B of the z/OS equivalent SC26-7414-05 available here at
   http://publibz.boulder.ibm.com/epubs/pdf/dgt2u140.pdf
*/

#define PE_DEBUG 1

static int fixPE(unsigned char *buf,
                 size_t *len,
                 unsigned char **entry,
                 unsigned long rlad)
{
    unsigned char *p;
    unsigned char *q;
    /* int z; */
    typedef struct {
        unsigned char pds2name[8];
        unsigned char unused1[19];
        unsigned char pds2epa[3];
        unsigned char pds2ftb1;
        unsigned char pds2ftb2;
        unsigned char pds2ftb3;
    } IHAPDS;
    IHAPDS *ihapds;
    int rmode;
    int amode;
    int ent;
    int rec = 0;
    int corrupt = 1;
    int rem = *len;
    int l;
    int l2;
    int lastt = -1;
    unsigned char *upto = buf;
    
    if ((*len <= 8) || (*((int *)buf + 1) != 0xca6d0f))
    {
        /* printf("Not an MVS PE executable\n"); */
        return (-1);
    }
#if PE_DEBUG
    printf("MVS PE total length is %lu\n", (unsigned long)*len);
#endif
    p = buf;
    while (1)
    {
        rec++;
        l = *(short *)p;
        /* keep track of remaining bytes, and ensure they really exist */
        if (l > rem)
        {
            break;
        }
        rem -= l;
#if PE_DEBUG
        printf("rec %d, offset %d is len %d\n", rec, p - buf, l);
#endif
#if 0
        if (1)
        {
            for (z = 0; z < l; z++)
            {
                printf("z %d %x %c\n", z, p[z],
                       isprint((unsigned char)p[z]) ? p[z] : ' ');
            }
        }
#endif
        if (rec == 3) /* directory record */
        {
            /* there should only be one directory entry, 
               which is 4 + 276 + 12 */
            if (l < 292)
            {
                break;
            }
            q = p + 24;
            l2 = *(short *)q;
            if (l2 < 32) break;
            ihapds = (IHAPDS *)(q + 2);
            rmode = ihapds->pds2ftb2 & 0x10;
/* do we care about alias entry point amode? */
#if 0
            amode = (ihapds->pds2ftb2 & 0x0c) >> 2;
#else
            amode = ihapds->pds2ftb2 & 0x03;
#endif
            ent = ((unsigned long)ihapds->pds2epa[0] << 16)
                | ((unsigned long)ihapds->pds2epa[1] << 8)
                | ihapds->pds2epa[2];
            *entry = buf + ent;
#if PE_DEBUG
            printf("module name is %.8s\n", ihapds->pds2name);
            printf("rmode is %s\n", rmode ? "ANY" : "24");
            printf("amode is ");
            if (amode == 0)
            {
                printf("24");
            }
            else if (amode == 2)
            {
                printf("31");
            }
            else if (amode == 1)
            {
                printf("64");
            }
            else if (amode == 3)
            {
                printf("ANY");
            }
            printf("\n");
            printf("entry point is %x\n", ent);
#endif            
        }
        else if (rec > 3)
        {
            int t;
            int r2;
            int l2;
            int term = 0;
            
            if (l < (4 + 12))
            {
                break;
            }
            q = p + 4 + 10;
            r2 = l - 4 - 10;
            while (1)
            {
                l2 = *(short *)q;
                r2 -= sizeof(short);
                if (l2 > r2)
                {
                    term = 1;
                    break;
                }
                r2 -= l2;

                if (l2 == 0) break;
                q += sizeof(short);
#if PE_DEBUG
                printf("load module record is of type %2x (len %5d)"
                       " offset %d\n", 
                       *q, l2, q - p);
#endif

                t = *q;
                if ((lastt == 1) || (lastt == 3) || (lastt == 0x0d))
                {
#if PE_DEBUG
                    printf("rectype: program text\n");
#endif
                    memmove(upto, q, l2);
                    upto += l2;
                    t = -1;
                    if (lastt == 0x0d)
                    {
                        term = 1;
                        corrupt = 0;
                        break;
                    }
                }
                else if (t == 0x20)
                {
                    /* printf("rectype: CESD\n"); */
                }
                else if (t == 1)
                {
                    /* printf("rectype: Control\n"); */
                }
                else if (t == 0x0d)
                {
                    /* printf("rectype: Control, about to end\n"); */
                }
                else if (t == 2)
                {
                    /* printf("rectype: RLD\n"); */
                    if (processRLD(buf, rlad, q, l2) != 0)
                    {
                        term = 1;
                        break;
                    }
                }
                else if (t == 3)
                {
                    int l3;
                    
                    /* printf("rectype: Dicionary = Control + RLD\n"); */
                    l3 = *(short *)(q + 6) + 16;
#if 0
                    printf("l3 is %d\n", l3);
#endif
                    if (processRLD(buf, rlad, q, l3) != 0)
                    {
                        term = 1;
                        break;
                    }
                }
                else if (t == 0x0e)
                {
                    /* printf("rectype: Last record of module\n"); */
                    if (processRLD(buf, rlad, q, l2) != 0)
                    {
                        term = 1;
                        break;
                    }
                    term = 1;
                    corrupt = 0;
                    break;
                }
                else if (t == 0x80)
                {
                    /* printf("rectype: CSECT\n"); */
                }
                else
                {
                    /* printf("rectype: unknown %x\n", t); */
                }
#if 0
                if ((t == 0x20) || (t == 2))
                {
                    for (z = 0; z < l; z++)
                    {
                        printf("z %d %x %c\n", z, q[z], 
                               isprint(q[z]) ? q[z] : ' ');
                    }
                }
#endif
                lastt = t;

                q += l2;
                if (r2 == 0)
                {
#if PE_DEBUG
                    printf("another clean exit\n");
#endif
                    break;
                }
                else if (r2 < (10 + sizeof(short)))
                {
                    /* printf("another unclean exit\n"); */
                    term = 1;
                    break;
                }
                r2 -= 10;
                q += 10;
            }
            if (term) break;            
        }
        p = p + l;
        if (rem == 0)
        {
#if PE_DEBUG
            printf("breaking cleanly\n");
#endif
        }
        else if (rem < 2)
        {
            break;
        }
    }
    if (corrupt)
    {
        printf("corrupt module\n");
        return (-1);
    }
#if 0
    printf("dumping new module\n");
#endif
    *len = upto - buf; /* return new module length */
    return (0);
}


static int processRLD(unsigned char *buf,
                      unsigned long rlad,
                      unsigned char *rld,
                      int len)
{
    unsigned char *r;
    int cont = 0;
    unsigned char *fin;
    int negative;
    int ll;
    int a;
    long newval;
    unsigned int *zaploc;
    
    r = (unsigned char *)rld + 16;
    fin = rld + len;
    while (r != fin)
    {
        if (!cont)
        {
            r += 4; /* skip R & P */
            if (r >= fin)
            {
                printf("corrupt1 at position %x\n", r - rld - 4);
                return (-1);
            }
        }
        negative = *r & 0x02;
        if (negative)
        {
            printf("got a negative adjustment - unsupported\n");
            return (-1);
        }
        ll = (*r & 0x0c) >> 2;
        ll++;
        if ((ll != 4) && (ll != 3))
        {
            printf("untested and unsupported relocation %d\n", ll);
            return (-1);
        }
        if (ll == 3)
        {
            if (rlad > 0xffffff)
            {
                printf("AL3 prevents relocating this module to %lx\n", rlad);
                return (-1);
            }
        }
        cont = *r & 0x01; /* do we have A & F continous? */
        r++;
        if ((r + 3) > fin)
        {
            printf("corrupt2 at position %x\n", r - rld);
            return (-1);
        }
        a = ((unsigned long)r[0] << 16)
            || ((unsigned long)r[1] << 8)
            || r[2];
        /* +++ need bounds checking on this OS code */
        /* printf("need to zap %d bytes at offset %6x\n", ll, a); */
        zaploc = (unsigned int *)(buf + a);

        /* This old code doesn't look right. The integer is misaligned */
        /* zaploc = (unsigned int *)(buf + a - ((ll == 3) ? 1 : 0)); */

        newval = *zaploc;
        /* printf("which means that %8x ", newval); */
        newval += rlad;
        /* printf("becomes %8x\n", newval); */
        *zaploc = newval;
        r += 3;
    }
    return (0);
}
#endif
