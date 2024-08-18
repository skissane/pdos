/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crc32.h"
#include "deflate.h"
#include "version.h"
#include "zip.h"

#define PERROR_IF(cnd, msg) do { if (cnd) { perror(msg); exit(1); } } while (0)

static void *xmalloc(size_t size)
{
        void *ptr = malloc(size);
        PERROR_IF(ptr == NULL, "malloc");
        return ptr;
}

static void *xrealloc(void *ptr, size_t size)
{
        ptr = realloc(ptr, size);
        PERROR_IF(ptr == NULL, "realloc");
        return ptr;
}

static uint8_t *read_file(const char *filename, size_t *file_sz)
{
        FILE *f;
        uint8_t *buf;
        size_t buf_cap;

        f = fopen(filename, "rb");
        PERROR_IF(f == NULL, "fopen");

        buf_cap = 4096;
        buf = xmalloc(buf_cap);

        *file_sz = 0;
        while (feof(f) == 0) {
                if (buf_cap - *file_sz == 0) {
                        buf_cap *= 2;
                        buf = xrealloc(buf, buf_cap);
                }

                *file_sz += fread(&buf[*file_sz], 1, buf_cap - *file_sz, f);
                PERROR_IF(ferror(f), "fread");
        }

        PERROR_IF(fclose(f) != 0, "fclose");
        return buf;
}

static void write_file(const char *filename, const uint8_t *data, size_t n)
{
        FILE *f;

        f = fopen(filename, "wb");
        PERROR_IF(f == NULL, "fopen");
        PERROR_IF(fwrite(data, 1, n, f) != n, "fwrite");
        PERROR_IF(fclose(f) != 0, "fclose");
}

static void list_zip(const char *filename)
{
        uint8_t *zip_data;
        size_t zip_sz;
        zip_t z;
        zipiter_t it;
        zipmemb_t m;

        printf("Listing ZIP archive: %s\n\n", filename);

        zip_data = read_file(filename, &zip_sz);

        if (!zip_read(&z, zip_data, zip_sz)) {
                printf("Failed to parse ZIP file!\n");
                exit(1);
        }

        if (z.comment_len != 0) {
                printf("%.*s\n\n", (int)z.comment_len, z.comment);
        }

        for (it = z.members_begin; it != z.members_end; it = m.next) {
                m = zip_member(&z, it);
                printf("%.*s\n", (int)m.name_len, m.name);
        }

        printf("\n");

        free(zip_data);
}

static char *terminate_str(const char *str, size_t n)
{
        char *p = xmalloc(n + 1);
        memcpy(p, str, n);
        p[n] = '\0';
        return p;
}

static void extract_zip(const char *filename)
{
        uint8_t *zip_data;
        size_t zip_sz;
        zip_t z;
        zipiter_t it;
        zipmemb_t m;
        char *tname;
        uint8_t *uncomp_data;

        printf("Extracting ZIP archive: %s\n\n", filename);

        zip_data = read_file(filename, &zip_sz);

        if (!zip_read(&z, zip_data, zip_sz)) {
                printf("Failed to read ZIP file!\n");
                exit(1);
        }

        if (z.comment_len != 0) {
                printf("%.*s\n\n", (int)z.comment_len, z.comment);
        }

        for (it = z.members_begin; it != z.members_end; it = m.next) {
                m = zip_member(&z, it);

                if (m.is_dir) {
                        printf(" (Skipping dir: %.*s)\n",
                               (int)m.name_len, m.name);
                        continue;
                }

                if (memchr(m.name, '/',  m.name_len) != NULL ||
                    memchr(m.name, '\\', m.name_len) != NULL) {
                        printf(" (Skipping file in dir: %.*s)\n",
                               (int)m.name_len, m.name);
                        continue;
                }

                switch (m.method) {
                case ZIP_STORE:   printf("  Extracting: "); break;
                case ZIP_SHRINK:  printf(" Unshrinking: "); break;
                case ZIP_REDUCE1:
                case ZIP_REDUCE2:
                case ZIP_REDUCE3:
                case ZIP_REDUCE4: printf("   Expanding: "); break;
                case ZIP_IMPLODE: printf("   Exploding: "); break;
                case ZIP_DEFLATE: printf("   Inflating: "); break;
                }
                printf("%.*s", (int)m.name_len, m.name);
                fflush(stdout);

                uncomp_data = xmalloc(m.uncomp_size);
                if (!zip_extract_member(&m, uncomp_data)) {
                        printf("  Error: decompression failed!\n");
                        exit(1);
                }

                if (crc32(uncomp_data, m.uncomp_size) != m.crc32) {
                        printf("  Error: CRC-32 mismatch!\n");
                        exit(1);
                }

                tname = terminate_str((const char*)m.name, m.name_len);
                write_file(tname, uncomp_data, m.uncomp_size);
                printf("\n");

                free(uncomp_data);
                free(tname);
        }

        printf("\n");
        free(zip_data);
}

void zip_callback(const char *filename, method_t method,
                  uint32_t size, uint32_t comp_size)
{
        switch (method) {
        case ZIP_STORE:
                printf("   Stored: "); break;
        case ZIP_SHRINK:
                printf("   Shrunk: "); break;
        case ZIP_REDUCE1:
        case ZIP_REDUCE2:
        case ZIP_REDUCE3:
        case ZIP_REDUCE4:
                printf("  Reduced: "); break;
        case ZIP_IMPLODE:
                printf(" Imploded: "); break;
        case ZIP_DEFLATE:
                printf(" Deflated: "); break;
        }

        printf("%s", filename);
        if (method != ZIP_STORE) {
                assert(size != 0 && "Empty files should use Store.");
                printf(" (%.0f%%)", 100 - 100.0 * comp_size / size);
        }
        printf("\n");
}

static void create_zip(const char *zip_filename, const char *comment,
                       method_t method, uint16_t n,
                       const char *const *filenames)
{
        time_t mtime;
        time_t *mtimes;
        uint8_t **file_data;
        uint32_t *file_sizes;
        size_t file_size, zip_size;
        uint8_t *zip_data;
        uint16_t i;

        printf("Creating ZIP archive: %s\n\n", zip_filename);

        if (comment != NULL) {
                printf("%s\n\n", comment);
        }

        mtime = time(NULL);

        file_data = xmalloc(sizeof(file_data[0]) * n);
        file_sizes = xmalloc(sizeof(file_sizes[0]) * n);
        mtimes = xmalloc(sizeof(mtimes[0]) * n);

        for (i = 0; i < n; i++) {
                file_data[i] = read_file(filenames[i], &file_size);
                if (file_size >= UINT32_MAX) {
                        printf("%s is too large!\n", filenames[i]);
                        exit(1);
                }
                file_sizes[i] = (uint32_t)file_size;
                mtimes[i] = mtime;
        }

        zip_size = zip_max_size(n, filenames, file_sizes, comment);
        if (zip_size == 0) {
                printf("zip writing not possible");
                exit(1);
        }

        zip_data = xmalloc(zip_size);
        zip_size = zip_write(zip_data, n, filenames,
                             (const uint8_t *const *)file_data,
                             file_sizes, mtimes, comment, method, zip_callback);

        write_file(zip_filename, zip_data, zip_size);
        printf("\n");

        free(zip_data);
        for (i = 0; i < n; i++) {
                free(file_data[i]);
        }
        free(mtimes);
        free(file_sizes);
        free(file_data);
}

static void print_usage(const char *argv0)
{
        printf("Usage:\n\n");
        printf("  %s list <zipfile>\n", argv0);
        printf("  %s extract <zipfile>\n", argv0);
        printf("  %s create <zipfile> "
               "[-m <method>] [-c <comment>] <files...>\n", argv0);
        printf("\n");
        printf("  Supported compression methods: \n");
        printf("  store, shrink, reduce, implode, deflate (default).\n");
        printf("\n");
}

static bool parse_method_flag(int argc, char **argv, int *i, method_t *m)
{
        if (*i + 1 >= argc) {
                return false;
        }
        if (strcmp(argv[*i], "-m") != 0) {
                return false;
        }

        if (strcmp(argv[*i + 1], "store") == 0) {
                *m = ZIP_STORE;
        } else if (strcmp(argv[*i + 1], "shrink") == 0) {
                *m = ZIP_SHRINK;
        } else if (strcmp(argv[*i + 1], "reduce") == 0) {
                *m = ZIP_REDUCE4;
        } else if (strcmp(argv[*i + 1], "implode") == 0) {
                *m = ZIP_IMPLODE;
        } else if (strcmp(argv[*i + 1], "deflate") == 0) {
                *m = ZIP_DEFLATE;
        } else {
                print_usage(argv[0]);
                printf("Unknown compression method: '%s'.\n", argv[*i + 1]);
                exit(1);
        }
        *i += 2;
        return true;
}

static bool parse_comment_flag(int argc, char **argv, int *i, const char **c)
{
        if (*i + 1 >= argc) {
                return false;
        }
        if (strcmp(argv[*i], "-c") != 0) {
                return false;
        }
        *c = argv[*i + 1];
        *i += 2;
        return true;
}

int main(int argc, char **argv) {
        const char *comment = NULL;
        method_t method = ZIP_DEFLATE;
        int i;

        printf("\n");
        printf("HWZIP " VERSION " -- A simple ZIP program ");
        printf("from https://www.hanshq.net/zip.html\n");
        printf("\n");

        if (argc == 3 && strcmp(argv[1], "list") == 0) {
                list_zip(argv[2]);
        } else if (argc == 3 && strcmp(argv[1], "extract") == 0) {
                extract_zip(argv[2]);
        } else if (argc >= 3 && strcmp(argv[1], "create") == 0) {
                i = 3;
                while (parse_method_flag(argc, argv, &i, &method) ||
                       parse_comment_flag(argc, argv, &i, &comment)) {
                }
                assert(i <= argc);

                create_zip(argv[2], comment, method, (uint16_t)(argc - i),
                           (const char *const *)&argv[i]);
        } else {
                print_usage(argv[0]);
                return 1;
        }

        return 0;
}
