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

struct linebuf {
    char *start;
    size_t size;
    FILE *f;
};

static long read_line(struct linebuf *lbuf)
{
    long lines_read = 0;
    size_t offset;
    char *p = lbuf->start;
    char *end = lbuf->start + lbuf->size;

    while (fgets(p, end - p, lbuf->f))
    {
        p += strlen(p);
        offset = p - lbuf->start;

        lines_read++;

        if (offset >= 2)
        {
            if ((p[-1] == '\n') && (p[-2] != '\\'))
            {
                p[-1] = '\0';
                break;
            }
        }
        else if (offset >= 1)
        {
            if (p[-1] == '\n')
            {
                p[-1] = '\0';
                break;
            }
        }

        if (end - p >= 80) continue;

        lbuf->size *= 2;
        lbuf->start = xrealloc(lbuf->start, lbuf->size);

        p = lbuf->start + offset;
        end = lbuf->start + lbuf->size;
    }
    
    return (lines_read);
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
         old_ns = ns, ns = ns->next, free(old_ns))
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

        if (isspace(line[0]))
        {
            if (filenames)
            {
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
        while (isspace(*p)) p++;
        if (*p == '\0') continue;

        if (strchr(p, '='))
        {
            record_waiting_files();
            parse_var_line(p);
            continue;
        }

        /* +++Check for "include"! */
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

                line = variable_expand_line(xstrdup(line));
                colon = strchr(line, ':');
                if (colon == NULL)
                {
                    fprintf(stderr, "Missing ':' in rule line!\n");
                    continue;
                }

                *colon = '\0';
                filenames = parse_nameseq(line, sizeof (*filenames));

                depstr = xstrdup(colon + 1);
                free(line);
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

void read_makefile(const char *filename)
{
    struct linebuf lbuf;

    lbuf.f = fopen(filename, "r");
    if (lbuf.f == NULL)
    {
        printf("Unable to open %s: %s\n", filename, strerror(errno));
        return;
    }

    lbuf.size = 256;
    lbuf.start = xmalloc(lbuf.size);

    read_lbuf(&lbuf, 1);

    free(lbuf.start);
    fclose(lbuf.f);
}
