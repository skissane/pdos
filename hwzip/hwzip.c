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

#ifdef HAVE_FOLDER
#include "folder.h"
#endif

#define MAX_FILES 0xFFFF

#ifdef POS_EXTEND
#include <fasc.h>
#endif

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


#ifdef HAVE_FOLDER
static char *xstrdup(const char *name)
{
    char *x;

    x = xmalloc(strlen(name) + 1);
    if (x == NULL) return (x);
    strcpy(x, name);
    return (x);
}
#endif

static uint8_t *read_file(const char *filename, size_t *file_sz)
{
        FILE *fp;
        uint8_t *buf;
        size_t buf_cap;

        fp = fopen(filename, "rb");
        PERROR_IF(fp == NULL, "fopen");

        fseek(fp, 0, SEEK_END);
        buf_cap = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        buf = (uint8_t*)xmalloc(buf_cap);
        PERROR_IF(buf == NULL, "xmalloc");

        *file_sz = fread(buf, sizeof(uint8_t), buf_cap, fp);
        PERROR_IF(ferror(fp), "fread");

        PERROR_IF(fclose(fp) != 0, "fclose");
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
#ifdef POS_EXTEND
                {
                    int x;
                    for (x = 0; x < m.name_len; x++)
                    {
                        m.name[x] = fasc(m.name[x]);
                    }
                }
#endif
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

#ifdef HAVE_FOLDER
static void expand_folders(const char *name, char **file_names, uint16_t *n)
{
	FOLDER *ffd;
    	char *entry;
    	int l, le;
    	char fldr[FILENAME_MAX];

    	ffd = openfldr(name);
    	if (!ffd) {
		if (*n < MAX_FILES) {
			file_names[*n] = xstrdup(name);
			(*n)++;
		}
		return;
    	}
    	if (strcmp(name, ".")) {
	        l = strlen(name);
	        if (l >= sizeof(fldr) -1) {
		    return;
	        }
	        memcpy(fldr,  name, l);
   		if(fldr[l-1] == '/') {
			l--;
    		}
    	} else {
		l = 0;
		fldr[0] = '\0';
    	}
    	while ((entry = readfldr(ffd)) != NULL) {
		fldr[l] = '\0';
		le = strlen(entry);
		if (le < 1 || le >= (sizeof(fldr) - l - 2)) {
		        printf(" (Skipping file: %s/%s)\n", fldr, entry);
		        continue;
		}
	    	if (l) {
			fldr[l + 1] = '/';
			memcpy(fldr + l + 1,  entry, le + 1);
	    	} else {
			memcpy(fldr + l,  entry, le + 1);
	    	}
		if (entry[le - 1] == '/') {
	    		expand_folders(fldr, file_names, n);
		} else {
			if (*n < MAX_FILES) {
				file_names[*n] = xstrdup(fldr);
				(*n)++;
			}
		}

    	}
    	closefldr(ffd);
}

static void make_folder(char *tname)
{
	char *slash;
	char c;
	slash = strrchr(tname, '/');
	if (!slash) {
	    slash = strrchr(tname, '\\');
	}
	if (!slash) {
		return;
	}
	c = *slash;
	*slash = '\0';
    	mkfldr(tname);
	*slash = c;
}
#endif

static int is_relative(const uint8_t *name, size_t len)
{
    size_t i;

    if (len < 1) {
	return 0;
    }
    if (name[0] == '/' || name[0] == '\\' || name[0] == '~') {
	return 0;
    }
    i = 0;
    while (i < len) {
	switch (name[i]) {
	case '<':
	case '>':
	case ':':
	case '"':
	case '|':
	case '?':
	case '*':
#ifndef POS_EXTEND
	case 0x7F:
#endif
	    return 0;
	case '.':
	    if (i + 1 < len && name[i+1] == '.') {
		return 0;
	    }
	    break;
        default:
	    if (name[i] < ' ') {
		return 0;
	    }
	}
	i++;
    }
    i = len - 1;
    if (name[i] == ' ' || name[i] == '.') {
	return 0;
    }
    return 1;
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


#ifdef POS_EXTEND
/* this code is only designed to convert "standard" text files. And by that,
   I mean the "internal file attributes" should have the low bit set to indicate
   that it is a text file (this is not actually being checked, it is just assumed).
   The filename or directory name should have '/', not '\' as the path separator.
   Files should contain ASCII data, not EBCDIC. And nor should they contain
   "extended ASCII". And line endings should be just LF, not CRLF - not sure if
   this means that the "line record format" should be set to 3 (Unix) instead
   of 0 (MSDOS). Files are all read and write and if the concept exists, executable
   as well. There is no expectation that file timestamps or any other attribute
   is preserved. Although all of this is required and expected, we do minimal
   checking currently, for expediency. */

                {
                    int x;
                    for (x = 0; x < m.name_len; x++)
                    {
                        m.name[x] = fasc(m.name[x]);
                    }
                }
#endif

                if (m.is_dir) {

#ifndef HAVE_FOLDER
                        if (1) {
#else
		        if (!is_relative(m.name, m.name_len)) {
#endif
			    printf(" (Skipping dir: %.*s)\n",
				   (int)m.name_len, m.name);
			} else {
#ifdef HAVE_FOLDER
			    printf(" Creating dir: %.*s\n",
				   (int)m.name_len, m.name);
			    tname = terminate_str((const char *) m.name, m.name_len);
			    mkfldr(tname);
			    free(tname);
#endif
		        }
                        continue;
                }

                if (!is_relative(m.name, m.name_len)) {
                        printf(" (Skipping file : %.*s)\n",
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
#ifdef HAVE_FOLDER
		make_folder(tname);
#endif
#ifdef POS_EXTEND
                {
                    uint32_t x;
                    for (x = 0; x < m.uncomp_size; x++)
                    {
                        uncomp_data[x] = fasc(uncomp_data[x]);
                    }
                }
#endif
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
#ifdef HAVE_FOLDER
	char **file_names;
	uint16_t file_n;
#endif
        size_t file_size, zip_size;
        uint8_t *zip_data;
        uint16_t i;

        printf("Creating ZIP archive: %s\n\n", zip_filename);

        if (comment != NULL) {
                printf("%s\n\n", comment);
        }

        mtime = time(NULL);

#ifndef HAVE_FOLDER
         file_data = xmalloc(sizeof(file_data[0]) * n);
         file_sizes = xmalloc(sizeof(file_sizes[0]) * n);
         mtimes = xmalloc(sizeof(mtimes[0]) * n);
#else
	file_names = xmalloc(sizeof(file_names[0]) * MAX_FILES);
	file_n = 0;
	for (i = 0; i < n; i++) {
		expand_folders(filenames[i], file_names, &file_n);
	}
	if (file_n >= MAX_FILES) {
        	printf("too many files!\n");
                exit(1);
        }

        file_data = xmalloc(sizeof(file_data[0]) * file_n);
        file_sizes = xmalloc(sizeof(file_sizes[0]) * file_n);
        mtimes = xmalloc(sizeof(mtimes[0]) * file_n);
#endif

#ifndef HAVE_FOLDER
        for (i = 0; i < n; i++) {
                file_data[i] = read_file(filenames[i], &file_size);
#else
        for (i = 0; i < file_n; i++) {
                file_data[i] = read_file(file_names[i], &file_size);
#endif

                if (file_size >= UINT32_MAX) {

#ifndef HAVE_FOLDER
                        printf("%s is too large!\n", filenames[i]);
#else
                        printf("%s is too large!\n", file_names[i]);
#endif
                        exit(1);
                }
                file_sizes[i] = (uint32_t)file_size;
                mtimes[i] = mtime;
        }

#ifndef HAVE_FOLDER
        zip_size = zip_max_size(n, filenames, file_sizes, comment);
#else
        zip_size = zip_max_size(file_n, (const char *const *)file_names, file_sizes, comment);
#endif

        if (zip_size == 0) {
                printf("zip writing not possible");
                exit(1);
        }

        zip_data = xmalloc(zip_size);

#ifndef HAVE_FOLDER
        zip_size = zip_write(zip_data, n, filenames,
#else
        zip_size = zip_write(zip_data, file_n, (const char *const *)file_names,
#endif
                             (const uint8_t *const *)file_data,
                             file_sizes, mtimes, comment, method, zip_callback);

        write_file(zip_filename, zip_data, zip_size);
        printf("\n");

        free(zip_data);

#ifndef HAVE_FOLDER
        for (i = 0; i < n; i++) {
#else
        for (i = 0; i < file_n; i++) {
#endif
                free(file_data[i]);

#ifdef HAVE_FOLDER
		free(file_names[i]);
#endif

        }
        free(mtimes);
        free(file_sizes);
        free(file_data);
#ifdef HAVE_FOLDER
	free(file_names);
#endif
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
