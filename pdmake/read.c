/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "dep.h"
#include "commands.h"
#include "variable.h"
#include "rule.h"
#include "xmalloc.h"
#include "read.h"

extern variable *default_goal_var;

struct include_dir {
    const char *name;
    struct include_dir *next;
};

static struct include_dir *include_dirs = NULL;
static struct include_dir **last_include_dir_p = &include_dirs;

static void include_makefile (const char *filename);

struct linebuf {
    char *start;
    size_t size;
    FILE *f;
    char *memory;
};

static char *memory_fgets (char *str, int num, char **source_p)
{
    char *source = *source_p;
    char *p = strchr (source, '\n');

    if (source[0] == '\0') return NULL;

    if (p && p - source < num - 1) {
        memcpy (str, source, p - source + 1);
        str[p - source + 1] = '\0';
        *source_p += p - source + 1;
        return str;
    }

    if (strlen (source) > num - 1) {
        memcpy (str, source, num - 1);
        str[num - 1] = '\0';
        *source_p += num - 1;
        return str;
    }

    strcpy (str, source);
    *source_p += strlen (source);
    return str;
}

static long read_line (struct linebuf *lbuf)
{
    long lines_read = 0;
    char *p = lbuf->start;
    char *end = lbuf->start + lbuf->size;

    while (lbuf->f ? fgets (p, end - p, lbuf->f) : memory_fgets (p, end - p, &(lbuf->memory))) {
        
        size_t offset;

        p += strlen (p);
        offset = p - lbuf->start;

        lines_read++;

        if (offset >= 2) {
            if ((p[-1] == '\n') && (p[-2] != '\\')) {
                p[-1] = '\0';
                break;
            }
        } else if (offset >= 1) {
            if (p[-1] == '\n') {
                p[-1] = '\0';
                break;
            }
        }

        if (end - p >= 80) continue;

        lbuf->size *= 2;
        lbuf->start = xrealloc (lbuf->start, lbuf->size);

        p = lbuf->start + offset;
        end = lbuf->start + lbuf->size;
        
    }
    
    return lines_read;
}

static void remove_backslash_newlines(char *line)
{
    char *in = line, *out = line;
    char *p = strchr(in, '\n');
    if (p == NULL) return;

    do {
        size_t out_line_len = p - in - 1;
        
        if (out != in) memmove(out, in, out_line_len);

        out += out_line_len;
        in = p + 1;

        while (isspace(*in)) in++;
        *(out++) = ' ';
    } while ((p = strchr(in, '\n')));

    memmove(out, in, strlen(in) + 1);
}

static void remove_comment(char *line)
{
    char *p = strchr(line, '#');
    if (p) *p = '\0';
}

void *parse_nameseq(char *line, size_t size)
{
    struct nameseq *start = NULL;
    struct nameseq **pp = &start;
    char *p, *temp;

    temp = xmalloc(strlen(line) + 1);

#define add_nameseq(_name) \
    do { \
        *pp = xmalloc(size); \
        (*pp)->next = NULL; \
        (*pp)->name = xstrdup(_name); \
        pp = &((*pp)->next); \
    } while (0)

    p = line;

    while (1)
    {
        char *p2;
        
        while (isspace(*p)) p++;
        if (*p == '\0') break;

        p2 = p;
        while (*p2 && !isspace(*p2)) p2++;

        memcpy(temp, p, p2 - p);
        temp[p2 - p] = '\0';
        add_nameseq(temp);

        p = p2;
    }
    
    free(temp);
#undef add_nameseq
    return (start);
}

void record_files(struct nameseq *filenames,
                  char *commands, size_t commands_index,
                  char *depstr)
{
    struct commands *cmds;
    struct dep *deps;
    struct nameseq *ns, *old_ns;

    if (commands_index > 0)
    {
        cmds = xmalloc(sizeof(*cmds));
        cmds->text = xstrndup(commands, commands_index);
        cmds->len = commands_index;
    }
    else cmds = NULL;

    if (depstr == NULL) deps = NULL;
    else
    {
        deps = parse_nameseq(depstr, sizeof(*deps));
    }

    for (ns = filenames, old_ns = NULL;
         ns;
         old_ns = ns, ns = ns->next, free (old_ns->name), free (old_ns))
    {
        if ((ns->name[0] == '.')
            && (strchr(ns->name, '\\') == NULL)
            && (strchr(ns->name, '/') == NULL))
        {
            rule_add_suffix(ns->name, cmds);
        }
        else
        {
            rule_add(ns->name, deps, cmds);
        }
    }

    free(depstr);
}

static void read_lbuf(struct linebuf *lbuf, int set_default)
{
    long lines_read;
    struct nameseq *filenames = NULL;
    char *commands;
    size_t commands_index = 0, commands_size = 256;
    char *clean = NULL;
    size_t clean_size = 0;
    char *depstr;

    commands = xmalloc(commands_size);

#define record_waiting_files() \
    do { \
        if (filenames) \
        { \
            record_files(filenames, commands, commands_index, depstr); \
            filenames = NULL; \
        } \
        commands_index = 0; \
    } while (0)

    while ((lines_read = read_line(lbuf)))
    {
        char *line = lbuf->start;
        char *p;

        if (line[0] == '\0') continue;

        if (line[0] == ' ' || line[0] == '\t')
        {
            if (filenames)
            {
                while (line[0] == ' ' || line[0] == '\t') line++;

                if (commands_index + strlen(line) + 1 > commands_size)
                {
                    commands_size = (commands_index + strlen(line) + 1) * 2;
                    commands = xrealloc(commands, commands_size);
                }
                memcpy(&(commands[commands_index]), line, strlen(line) + 1);
                commands_index += strlen(line) + 1;
                commands[commands_index - 1] = '\n';

                continue;
            }
        }

        if (strlen(line) + 1 > clean_size)
        {
            clean_size = strlen(line) + 1;
            free(clean);
            clean = xmalloc(clean_size);
        }
        strcpy(clean, line);

        remove_backslash_newlines(clean);
        remove_comment(clean);
        p = clean;

        /* Skip blank lines */
        while (isspace (*p)) p++;
        if (*p == '\0') continue;

        if (strncmp (p, "include", 7) == 0 && (isspace (p[7]) || p[7] == '\0')) {

            char *q;

            p += 7;

            /* All trailing whitespace should be stripped first. */
            for (q = p + strlen (p); q > p && isspace (q[-1]); q--) {}
            *q = '\0';

            p = line = variable_expand_line (xstrdup (p));

            while (1) {

                char saved_c;

                for (; isspace (*p); p++) {}

                for (q = p; *q && !isspace (*q); q++) {}

                if (q == p) break;

                saved_c = *q;
                *q = '\0';

                include_makefile (p);
                *q = saved_c;
                p = q;
                
            }

            free (line);
            continue;
            
        }

        if (strchr (p, '='))
        {
            record_waiting_files ();
            parse_var_line (p, VAR_ORIGIN_FILE);
            continue;
        }

        {
            char *semicolonp, *commentp;

            record_waiting_files();

            semicolonp = strchr(line, ';');
            commentp = strchr(line, '#');

            if (commentp && semicolonp && (commentp < semicolonp))
            {
                *commentp = '\0';
                semicolonp = NULL;
            }
            else if (semicolonp)
            {
                *(semicolonp++) = '\0';
            }

            remove_backslash_newlines(line);

            {
                char *colon;

                line = variable_expand_line (xstrdup (line));
                colon = strchr (line, ':');
                if (colon == NULL) {
                    
                    for (p = line; isspace (*p); p++) {}
                    
                    /* Empty lines created by variable expansion should be ignored. */
                    if (*p) {
                        fprintf(stderr, "Missing ':' in rule line!\n");
                    }
                    free (line);
                    continue;
                    
                }

                *colon = '\0';
                filenames = parse_nameseq (line, sizeof (*filenames));

                depstr = xstrdup (colon + 1);
                free (line);
            }

            if (semicolonp)
            {
                if (commands_index + strlen(semicolonp) + 1 > commands_size)
                {
                    commands_size = (commands_index
                                     + strlen(semicolonp) + 1) * 2;
                    commands = xrealloc(commands, commands_size);
                }
                memcpy(&(commands[commands_index]), semicolonp,
                       strlen(semicolonp) + 1);
                commands_index += strlen(semicolonp) + 1;
            }

            if (set_default && (default_goal_var->value[0] == '\0'))
            {
                struct nameseq *ns;

                for (ns = filenames; ns; ns = ns->next)
                {
                    if ((ns->name[0] == '.')
                        && (strchr(ns->name, '\\') == 0)
                        && (strchr(ns->name, '/') == 0))
                    {
                        continue;
                    }

                    free (default_goal_var->value);
                    default_goal_var->value = xstrdup(ns->name);
                    break;
                }
            }
        }
    }

    record_waiting_files();

    free(clean);
    free(commands);
}

int read_makefile (const char *filename)
{
    struct linebuf lbuf;
    struct variable *makefile_list;
    char *new_value;

    lbuf.f = fopen (filename, "r");
    if (lbuf.f == NULL) {
        return 1;
    }

    lbuf.size = 256;
    lbuf.start = xmalloc(lbuf.size);

    makefile_list = variable_find ("MAKEFILE_LIST");
    if (makefile_list) {

        size_t old_len = strlen (makefile_list->value);
        
        new_value = xmalloc (old_len + 1 + strlen (filename) + 1);
        memcpy (new_value, makefile_list->value, old_len);
        new_value[old_len] = ' ';
        strcpy (new_value + old_len + 1, filename);
        variable_change ("MAKEFILE_LIST", new_value, VAR_ORIGIN_FILE);
        
    } else {
        
        new_value = xmalloc (1 + strlen (filename) + 1);
        new_value[0] = ' ';
        strcpy (new_value + 1, filename);
        variable_change ("MAKEFILE_LIST", new_value, VAR_ORIGIN_FILE);
        
    }

    read_lbuf (&lbuf, 1);

    free (lbuf.start);
    fclose (lbuf.f);

    return 0;
}

void read_memory_makefile (char *memory)
{
    struct linebuf lbuf;

    lbuf.f = NULL;
    lbuf.memory = memory;
    lbuf.size = 256;
    lbuf.start = xmalloc(lbuf.size);

    read_lbuf (&lbuf, 1);

    free (lbuf.start);
}

void include_dir_add (const char *name)
{
    struct include_dir *idir = xmalloc (sizeof (*idir));

    idir->name = name;
    idir->next = NULL;
    *last_include_dir_p = idir;
    last_include_dir_p = &idir->next;
}

void include_dirs_destroy (void)
{
    struct include_dir *idir;

    for (idir = include_dirs; idir; idir = include_dirs) {
        include_dirs = idir->next;
        free (idir);
    }

    last_include_dir_p = &include_dirs;
}

static void include_makefile (const char *filename)
{
    struct include_dir *idir;

    if (read_makefile (filename) == 0) return;

    for (idir = include_dirs; idir; idir = idir->next) {
        char *new_name = xmalloc (strlen (idir->name) + 1 + strlen (filename) + 1);

        strcpy (new_name, idir->name);
        strcat (new_name, "/");
        strcat (new_name, filename);

        if (read_makefile (new_name) == 0) {
            free (new_name);
            return;
        }

        free (new_name);
    }

    fprintf (stderr, "Unable to include %s: %s. Stop.\n", filename, strerror(errno));
    exit(EXIT_FAILURE);
}
