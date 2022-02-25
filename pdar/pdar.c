/* pdar.c
 *
 * Public Domain Archiver
 * Jesus Diaz
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <https://unlicense.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

/* #define DEBUG 1 */

char *strdup(const char *s);

static void *xmalloc(size_t size)
{
    void *p = malloc(size);
#ifdef DEBUG
    printf("allocated %p (size=%u)\n", p, (unsigned)size);
#endif
    if (p == NULL)
    {
        printf("out of memory: %s\n", strerror(errno));
        abort();
    }
    return p;
}

static void *xrealloc(void *p, size_t size)
{
    void *np = realloc(p, size);
#ifdef DEBUG
    printf("reallocated %p -> %p (size=%u)\n", p, np, (unsigned)size);
#endif
    if (np == NULL)
    {
        printf("out of memory: %s\n", strerror(errno));
        abort();
    }
    return np;
}

static void xfree(void *p)
{
#ifdef DEBUG
    printf("freed %p\n", p);
#endif
    free(p);
    return;
}

struct ar_raw_header
{
    char signature[8];
};

struct ar_raw_file_header
{
    char name[16];
    char modified_timestamp[12];
    char owner_id[6];
    char user_id[6];
    char file_mode[8];
    char file_size[10];
    char eol[2];
};

#define AR_SIGNATURE "!<arch>\x0A"
#define AR_SYMDEF "__.SYMDEF"
#define AR_SYMDEF_SORTED "__.SYMDEF SORTED"

#define AR_STR_BUFFER tmpbuf

#define AR_STR_SET_PRINTF(x, fmt, arg)     \
    sprintf(AR_STR_BUFFER, "%-" fmt, arg); \
    memcpy(x, AR_STR_BUFFER, sizeof x);

#define AR_STR_GET(x)                               \
    memset(AR_STR_BUFFER, 0, sizeof AR_STR_BUFFER); \
    memcpy(AR_STR_BUFFER, x, sizeof x);             \
    AR_STR_BUFFER[sizeof x] = '\0';

static char AR_STR_BUFFER[255];

#define CNF_DELETE 0x01      /* d - delete */
#define CNF_MOVE 0x02        /* m - move */
#define CNF_PRINT 0x04       /* p - print */
#define CNF_QAPPEND 0x08     /* q - quick append */
#define CNF_AREPLACE 0x10    /* r - add and replace */
#define CNF_TABLE_PRINT 0x20 /* t - table print list */
#define CNF_EXTRACT 0x40     /* x - extract */
#define CNF_ADD_AFTER 0x80   /* a - add new files (after existing) */
#define CNF_ADD_BEFORE 0x100 /* b - add new files (before existing) */
#define CNF_CREATE 0x200     /* c - create the archive if it does not exist */
#define CNF_VERBOSE 0x400    /* v - verbose */

static size_t get_filesize(const char *name)
{
    size_t size;
    FILE *fp;
    fp = fopen(name, "rb");
    if (fp == NULL)
    {
        printf("can't read file %s\n", name);
        abort();
    }

    /* get the file size */
    if (fseek(fp, 0, SEEK_END) != 0)
    {
        printf("can't seek file %s (trying second method): %s\n", name, strerror(errno));

        /* try fgetc until feof method */
        if (fseek(fp, 0, SEEK_SET) != 0)
        {
            printf("can't seek file, second method failed for %s: %s\n", name, strerror(errno));
            printf("note: relying that the file pointer is on offset=0\n");
        }

        size = 0;
        while (!feof(fp))
        {
            fgetc(fp);
            size++;
        }
        fclose(fp);
        return size;
    }

    size = ftell(fp);
    fclose(fp);
    return size;
}

static int copy_to_file(FILE *out, FILE *in, size_t size)
{
    void *buffer;

    buffer = xmalloc(size);

    if (fread(buffer, 1, size, in) != size)
    {
        printf("can't read input: %s\n", strerror(errno));
        return -1;
    }

    if (fwrite(buffer, 1, size, out) != size)
    {
        printf("can't write output: %s\n", strerror(errno));
        return -1;
    }

    xfree(buffer);
    return 0;
}

/* moves elements and inserts something in the middle of a file (not recommended for big files) */
static int finsert(FILE *fp, const void *data, size_t size)
{
    FILE *tmp_fp;
    long blksize = 4096;
    long start;
    void *buffer;

    tmp_fp = tmpfile();
    if (tmp_fp == NULL)
    {
        printf("can't create temporal file: %s\n", strerror(errno));
        return -1;
    }

    buffer = xmalloc(blksize);
    start = ftell(fp);
#ifdef DEBUG
    printf("inserting %u bytes of data at %ld\n", (unsigned)size, start);
#endif

    /* write data to tmp file */
    while (!feof(fp))
    {
        size_t rem;

        rem = fread(buffer, 1, blksize, fp);
        if (!rem)
        {
            break;
        }

        if (fwrite(buffer, 1, rem, tmp_fp) != rem)
        {
            printf("write error: %s\n", strerror(errno));
            return -1;
        }
    }
    /* write the inserted things */
    fseek(fp, start, SEEK_SET);
    fwrite(data, 1, size, fp);
    /* read back from the temporal file and write the rest */
    rewind(tmp_fp);
    while (!feof(tmp_fp))
    {
        size_t rem;

        rem = fread(buffer, 1, blksize, tmp_fp);
        if (!rem)
        {
            break;
        }

        if (fwrite(buffer, 1, rem, fp) != rem)
        {
            printf("write error: %s\n", strerror(errno));
            return -1;
        }
    }
    xfree(buffer);

    /* data from start_pos has now been moved to start_pos + stride */
    fclose(tmp_fp);
    return 0;
}

static int ar_next_file(FILE *fp, struct ar_raw_file_header *file_hdr)
{
    if (fread(file_hdr, 1, sizeof(*file_hdr), fp) < sizeof(*file_hdr))
    {
#ifdef DEBUG
        printf("can't read data: %s\n", strerror(errno));
#endif
        return -1;
    }

    AR_STR_GET(file_hdr->file_size);
    if (atoi(AR_STR_BUFFER) <= 0 && strncmp(AR_SYMDEF, file_hdr->name, strlen(AR_SYMDEF)))
    {
        printf("corrupt ar file at ");
        AR_STR_GET(file_hdr->name);
        printf("%s - with a size of ", AR_STR_BUFFER);
        AR_STR_GET(file_hdr->file_size);
        printf("%i\n", atoi(AR_STR_BUFFER));
        return -1;
    }
    return 0;
}

/* skip the body of a file */
static int ar_skip_file_body(FILE *fp, struct ar_raw_file_header *file_hdr)
{
    AR_STR_GET(file_hdr->file_size);
    if (fseek(fp, atol(AR_STR_BUFFER), SEEK_CUR) != 0)
    {
#ifdef DEBUG
        printf("can't seek file\n");
#endif
        return -1;
    }

    /* skip padding (if any) */
    if (ftell(fp) % 2 != 0)
    {
        if (fseek(fp, 1, SEEK_CUR) != 0)
        {
#ifdef DEBUG
            printf("can't seek file\n");
#endif
            return -1;
        }
    }

#ifdef DEBUG
    printf("skipping %li bytes\n", atol(AR_STR_BUFFER));
#endif
    return 0;
}

/* print contents of the archive */
static int ar_do_print(const char *ar_str)
{
    FILE *fp;
#ifdef DEBUG
    printf("print\n");
#endif

    fp = fopen(ar_str, "rb");
    if (fp == NULL)
    {
        printf("can't open archive: %s\n", strerror(errno));
        return -1;
    }

    fseek(fp, sizeof(struct ar_raw_header), SEEK_SET);
    while (!feof(fp))
    {
        struct ar_raw_file_header file_hdr;

        if (ar_next_file(fp, &file_hdr) != 0)
        {
            break;
        }

        if (ar_skip_file_body(fp, &file_hdr) != 0)
        {
            break;
        }

        AR_STR_GET(file_hdr.name);
        printf("%s\n", AR_STR_BUFFER);
    }
    fclose(fp);
    return 0;
}

/* quick append a file to the archive */
static int ar_do_qappend(const char *ar_str, int argc, char **argv)
{
    register int i;
    FILE *fp;
#ifdef DEBUG
    printf("quick append\n");
#endif
    fp = fopen(ar_str, "ab+");
    fseek(fp, 0, SEEK_END);
    for (i = 3; i < argc; i++)
    {
        struct ar_raw_file_header file_hdr;
        size_t size = get_filesize(argv[i]);
        FILE *file_fp;

        /* fill metadata */
        AR_STR_SET_PRINTF(file_hdr.name, "16s", argv[i]);
        AR_STR_SET_PRINTF(file_hdr.file_size, "10i", (int)size);
        AR_STR_SET_PRINTF(file_hdr.file_mode, "8i", 0);
        AR_STR_SET_PRINTF(file_hdr.owner_id, "6i", 0);
        AR_STR_SET_PRINTF(file_hdr.user_id, "6i", 0);
        AR_STR_SET_PRINTF(file_hdr.modified_timestamp, "12i", 0);
        file_hdr.eol[0] = 0x60;
        file_hdr.eol[1] = 0x0A;
        fwrite(&file_hdr, 1, sizeof file_hdr, fp);

        /* write the file body */
        file_fp = fopen(argv[i], "rb");
        if (file_fp == NULL)
        {
            printf("can't open file %s\n", argv[i]);
            return -1;
        }
        copy_to_file(fp, file_fp, size);
        fclose(file_fp);

        /* padding - insert a newline */
        if (ftell(fp) % 2 != 0)
        {
            char ch = 0x0A;
            if (fwrite(&ch, 1, sizeof ch, fp) != sizeof ch)
            {
                printf("write error: %s\n", strerror(errno));
                return -1;
            }
        }

        printf("appended %s - %i bytes\n", argv[i], (int)size);
    }
    fclose(fp);
    return 0;
}

struct aout_header
{
    unsigned long n_midmag;
    unsigned long n_text;
    unsigned long n_data;
    unsigned long bss_size;
    unsigned long n_syms;
    unsigned long entry;
    unsigned long n_textrel;
    unsigned long n_datarel;
};

struct aout_relocation
{
    long m_uAddress;
    /* Symnum   24 - Symbol number
     * PC Rel   1 - If set means the linker is modifying a PC-Relative mc-instruction
     * Length   2 - Log base 2 of the size of the pointer in bytes
     * Extern   1 - Set if requires an external reference
     * BaseRel  1 - If set the symbol is relocation on offset of a GOT table
     * JmpTable 1 - If set the symbol is relocated on an offset of the PLT table
     * Relative 1 - If set then this symbol is relative to the image address the exec is loaded at
     * Copy     1 - If set then the symbol data must be copied somewhere else
     */
    unsigned long flags;
};

enum
{
    E_AOUTSYMTYP_UNDEF = 0,
    E_AOUTSYMTYP_ABSOLUTE = 2,
    E_AOUTSYMTYP_TEXT = 4,
    E_AOUTSYMTYP_DATA = 6,
    E_AOUTSYMTYP_BSS = 8,
    E_AOUTSYMTYP_FN = 15
};

struct aout_nlist
{
    long stroff;
    unsigned char type;
    char other;
    short desc;
    long value; /* Address into section */
};

struct ranlib_sym
{
    long stridx; /* string index in string table */
    long off;    /* file-index of the symbol */
};

static void aout_get_symbols(FILE *fp, int file_idx, struct ranlib_sym **symbols, size_t *n_symbols, char **strtab, size_t *strtab_size)
{
    char tmpbuf[80];
    struct aout_header aout_hdr;
    long start;
    long strtab_start;
    long sym_start;

    start = ftell(fp);
    fread(&aout_hdr, 1, sizeof aout_hdr, fp);
    fseek(fp, aout_hdr.n_text, SEEK_CUR);    /* skip text */
    fseek(fp, aout_hdr.n_data, SEEK_CUR);    /* skip data */
    fseek(fp, aout_hdr.n_textrel, SEEK_CUR); /* skip text relocations */
    fseek(fp, aout_hdr.n_datarel, SEEK_CUR); /* skip data relocations */

    /* read the entire symbol section (thats the only thing that matters rn) */
    sym_start = ftell(fp);
    strtab_start = sym_start + aout_hdr.n_syms;
    while (sym_start < strtab_start)
    {
        struct aout_nlist nlist;
        int stridx;
        char *strptr;

        /* read the symbol thing */
        fread(&nlist, 1, sizeof nlist, fp);
        sym_start = ftell(fp);
        if ((nlist.type == 5) || (nlist.type == 7) || (nlist.type == 9))
        {
            /* steal the string from the string table super-quick because we're a lil' sneaky :-) */
            fseek(fp, strtab_start + nlist.stroff, SEEK_SET);
            fread(tmpbuf, 1, sizeof tmpbuf, fp);
            fseek(fp, sym_start, SEEK_SET);
            /* now back to normal stuff */

            strptr = tmpbuf;
            stridx = *strtab_size;
            /* strings in the string table are null terminated */
            *strtab_size += strlen(strptr) + 1;
            *strtab = xrealloc(*strtab, *strtab_size);
            memcpy(&(*strtab)[stridx], strptr, strlen(strptr) + 1);
            /* and add the symbol */
            *symbols = xrealloc(*symbols, sizeof(struct ranlib_sym) * ((*n_symbols) + 1));
            (*symbols)[*n_symbols].stridx = stridx;
            (*symbols)[*n_symbols].off = start - sizeof(struct ar_raw_file_header);
            /* file_idx; */
            (*n_symbols)++;
        }
    }

    /* reset back and pretend this never happened */
    fseek(fp, start, SEEK_SET);
}

int is_verbose = 0;
int main(int argc, char **argv)
{
    register int i, j;
    int conf = 0;
    int opt;
    const char *opt_str, *ar_str;
    FILE *fp;
    struct ar_raw_header hdr = {0};

    if (argc < 3)
    {
        printf("%s [options] [archive] [files...]\n", argv[0]);
        return -1;
    }

    /* then comes the archiver after the options */
    ar_str = argv[2];
#ifdef DEBUG
    printf("opening archive %s\n", ar_str);
#endif

    /* parse [options] */
    j = 0;
    opt_str = argv[1];
    while ((opt = opt_str[j]) != '\0')
    {
        switch (opt)
        {
        case 'd':
            conf |= CNF_DELETE;
#ifdef DEBUG
            printf("delete\n");
#endif
            break;
        case 'm':
            conf |= CNF_MOVE;
#ifdef DEBUG
            printf("move\n");
#endif
            break;
        case 'p':
            conf |= CNF_PRINT;
            ar_do_print(ar_str);
            break;
        case 'q':
            conf |= CNF_QAPPEND;
            ar_do_qappend(ar_str, argc, argv);
            break;
        case 'r':
            conf |= CNF_AREPLACE;
#ifdef DEBUG
            printf("add-replace\n");
#endif
            break;
        case 's':
#ifdef DEBUG
            printf("symbol-generate\n");
#endif
            /*  */
            fp = fopen(ar_str, "rb+");
            /* insert an empty symdef file at the start of the archive */ {
                struct ar_raw_file_header file_hdr;
                struct ranlib_sym *symbols = NULL;
                size_t n_symbols = 0;
                char *strtab = NULL;
                size_t strtab_size = 0;

                /* construct the symbol definitions table */
                i = 0;
                fseek(fp, sizeof hdr, SEEK_SET);
                while (!feof(fp))
                {
                    char *p;

                    /* obtain list of symbols */
                    if (ar_next_file(fp, &file_hdr) != 0)
                    {
                        break;
                    }

                    AR_STR_GET(file_hdr.name);
                    p = strchr(AR_STR_BUFFER, '.');
                    if (p != NULL)
                    {
                        /* an object file */
                        if (p[1] == 'o')
                        {
                            aout_get_symbols(fp, i, &symbols, &n_symbols, &strtab, &strtab_size);
                        }
                    }

                    if (ar_skip_file_body(fp, &file_hdr) != 0)
                    {
                        break;
                    }

                    i++;
                }

#if 0
                    /* string table */
                    AR_STR_SET_PRINTF(file_hdr.name, "16s", "//");
                    AR_STR_SET_PRINTF(file_hdr.file_size, "10i", (int)strtab_size);
                    AR_STR_SET_PRINTF(file_hdr.file_mode, "8i", 0);
                    AR_STR_SET_PRINTF(file_hdr.owner_id, "6i", 0);
                    AR_STR_SET_PRINTF(file_hdr.user_id, "6i", 0);
                    AR_STR_SET_PRINTF(file_hdr.modified_timestamp, "12i", 0);
                    file_hdr.eol[0] = 0x60;
                    file_hdr.eol[1] = 0x0A;
                    fseek(fp, sizeof hdr, SEEK_SET);
                    finsert(fp, &file_hdr, sizeof file_hdr);
                    fseek(fp, sizeof hdr + sizeof file_hdr, SEEK_SET);
                    finsert(fp, strtab, strtab_size);
#endif

                /* symdef */
                AR_STR_SET_PRINTF(file_hdr.name, "16s", AR_SYMDEF);
                AR_STR_SET_PRINTF(file_hdr.file_size, "10i",
                                  (int)(sizeof(long) + sizeof(struct ranlib_sym) * n_symbols + sizeof(strtab_size) + strtab_size));
                AR_STR_SET_PRINTF(file_hdr.file_mode, "8i", 0);
                AR_STR_SET_PRINTF(file_hdr.owner_id, "6i", 0);
                AR_STR_SET_PRINTF(file_hdr.user_id, "6i", 0);
                AR_STR_SET_PRINTF(file_hdr.modified_timestamp, "12i", 0);
                file_hdr.eol[0] = 0x60;
                file_hdr.eol[1] = 0x0A;
                fseek(fp, sizeof hdr, SEEK_SET);
                finsert(fp, &file_hdr, sizeof file_hdr);
                fseek(fp, sizeof hdr + sizeof file_hdr, SEEK_SET);
                {
                    long x;
                    x = sizeof(struct ranlib_sym) * n_symbols;
                    finsert(fp, &x, sizeof x);
                }
                {
                    int y;
                    for (y = 0; y < n_symbols; y++)
                    {
                        symbols[y].off +=
                            sizeof file_hdr + sizeof(long) + sizeof(struct ranlib_sym) * n_symbols + sizeof(strtab_size) + strtab_size;
                    }
                }
                fseek(fp, sizeof hdr + sizeof file_hdr + sizeof(long), SEEK_SET);
                finsert(fp, symbols, sizeof(struct ranlib_sym) * n_symbols);

                fseek(fp, sizeof hdr + sizeof file_hdr + sizeof(long) + (sizeof(struct ranlib_sym) * n_symbols), SEEK_SET);
                finsert(fp, &strtab_size, sizeof(strtab_size));

                fseek(fp, sizeof hdr + sizeof file_hdr + sizeof(long) + (sizeof(struct ranlib_sym) * n_symbols) + sizeof(strtab_size), SEEK_SET);
                finsert(fp, strtab, strtab_size);
            }
            fclose(fp);
            break;
        case 't':
#ifdef DEBUG
            printf("table-print\n");
#endif
            printf("-------------------------------------------------------\n");
            printf("| %10s | %10s | %8s |\n", "Name", "Size", "Mode");
            printf("-------------------------------------------------------\n");
            fp = fopen(ar_str, "rb");
            fseek(fp, sizeof hdr, SEEK_SET);
            while (!feof(fp))
            {
                struct ar_raw_file_header file_hdr;

                if (ar_next_file(fp, &file_hdr) != 0)
                {
                    break;
                }

                if (ar_skip_file_body(fp, &file_hdr) != 0)
                {
                    break;
                }

                AR_STR_GET(file_hdr.name);
                printf("| %10s ", AR_STR_BUFFER);
                AR_STR_GET(file_hdr.file_size);
                printf("| %10s ", AR_STR_BUFFER);
                AR_STR_GET(file_hdr.file_mode);
                printf("| %8s ", AR_STR_BUFFER);
                printf("|\n");
            }
            fclose(fp);
            printf("-------------------------------------------------------\n");
            break;
        case 'x':
#ifdef DEBUG
            printf("extract\n");
#endif
            fp = fopen(ar_str, "rb");
            fseek(fp, sizeof hdr, SEEK_SET);
            while (!feof(fp))
            {
                struct ar_raw_file_header file_hdr;
                FILE *out;

                fread(&file_hdr, 1, sizeof file_hdr, fp);
                AR_STR_GET(file_hdr.file_size);
                if (atoi(AR_STR_BUFFER) <= 0 && strncmp(AR_SYMDEF, file_hdr.name, strlen(AR_SYMDEF)))
                {
                    printf("corrupt ar file at ");
                    AR_STR_GET(file_hdr.name);
                    printf("%s - with a size of ", AR_STR_BUFFER);
                    AR_STR_GET(file_hdr.file_size);
                    printf("%i\n", atoi(AR_STR_BUFFER));
                    return -1;
                }

                /* write to file */
                AR_STR_GET(file_hdr.name);
                out = fopen(AR_STR_BUFFER, "ab+");
                if (out == NULL)
                {
                    printf("can't write file \"");
                    printf("%s\"", AR_STR_BUFFER);
                    return -1;
                }
                AR_STR_GET(file_hdr.file_size);
                copy_to_file(out, fp, atoi(AR_STR_BUFFER));
                fclose(out);
                AR_STR_GET(file_hdr.name);
                printf("written %s", AR_STR_BUFFER);
                AR_STR_GET(file_hdr.file_size);
                printf(" (%i bytes)\n", atoi(AR_STR_BUFFER));
            }
            fclose(fp);
            break;
        case 'c':
#ifdef DEBUG
            printf("create\n");
#endif
            /* create the archive if specified so (preventing errors down the line) */
            fp = fopen(ar_str, "rb");
            if (fp == NULL)
            {
                /* File does not exist? - create it */
                fp = fopen(ar_str, "wb+");
                if (fp == NULL)
                {
                    printf("unable to create archive %s: %s\n", ar_str, strerror(errno));
                    return -1;
                }
                fputs(AR_SIGNATURE, fp);
            }
            fclose(fp);
            break;
        case 'v':
            is_verbose = 1;
            break;
        case 'V':
            printf("PDAR: Public Domain Archiver v1.0\n");
            return 0;
        case 'h':
            printf("pdar [vVhcxtsrqpmd] [optional files...]\n");
            printf("%10s display help\n", "h");
            printf("%10s enable verbosity\n", "v");
            printf("%10s add a file\n", "q");
            printf("%10s display version information\n", "V");
            printf("%10s create an archive if it does not exist\n", "c");
            printf("%10s generate symbols index\n", "s");
            printf("Commands are executed as they appear (cV would first evaluate 'c' then 'V')\n");
            return 0;
        default:
            break;
        }
        ++j;
    }
    return 0;
}
