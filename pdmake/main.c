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

#include "variable.h"
#include "read.h"
#include "dep.h"
#include "commands.h"
#include "rule.h"
#include "xmalloc.h"

static int dry_run = 0; /* Run no commands. */
static int ignore_errors = 0;

variable *default_goal_var;

static rule *rules = NULL;
static suffix_rule *suffix_rules = NULL;

void rule_add(char *name, struct dep *deps, struct commands *cmds)
{
    rule *r = xmalloc(sizeof(*r));

    r->name = name;
    r->deps = deps;
    r->cmds = cmds;

    r->next = rules;
    rules = r;
}

void rule_add_suffix(char *name, struct commands *cmds)
{
    suffix_rule *s = xmalloc(sizeof(*s));
    char *p = strchr(name + 1, '.');

    *p = '\0';
    s->first = xstrdup(name);
    *p = '.';
    s->second = xstrdup(p);

    s->cmds = cmds;

    s->next = suffix_rules;
    suffix_rules = s;
}

void rule_use(rule *r, char *name)
{
    struct dep *dep;
    char *new_cmds;
    char *p, *q;
    char *star_name;
    
    for (dep = r->deps; dep; dep = dep->next)
    {
        rule_search_and_build(dep->name);
    }

    if (r->cmds == NULL) return;

    star_name = xstrdup(name);
    p = strrchr(star_name, '.');
    if (p) *p = '\0';

    variable_change("@", xstrdup(name));
    variable_change("*", star_name);

    p = r->cmds->text;
    q = strchr(p, '\n');
    while (1)
    {
        *q = '\0';
        new_cmds = xstrdup(p);
        *q = '\n';
        new_cmds = variable_expand_line(new_cmds);
        printf("%s\n", new_cmds);

        if (!dry_run)
        {
            int error = system(new_cmds);

            if (!ignore_errors && error)
            {
                fprintf(stderr, "[%s] Error %d\n", name, error);
                exit(1);
            }
        }
        free(new_cmds);

        p = q + 1;
        q = strchr(p, '\n');
        if (q == NULL) break;
    }
}

void suffix_rule_use(suffix_rule *s, char *name)
{
    char *lesser_name, *star_name;
    char *p;
    char *new_cmds;
    char *q;
    
    if (s->cmds == NULL) return;

    lesser_name = xmalloc(strlen(name) + strlen(s->first) + 1);
    memcpy(lesser_name, name, strlen(name) + 1);
    p = strrchr(lesser_name, '.');
    if (p) *p = '\0';
    strcat(lesser_name, s->first);

    star_name = xstrdup(name);
    p = strrchr(star_name, '.');
    if (p) *p = '\0';

    variable_change("<", lesser_name);
    variable_change("*", star_name);

    p = s->cmds->text;
    q = strchr(p, '\n');
    while (1)
    {
        *q = '\0';
        new_cmds = xstrdup(p);
        *q = '\n';
        new_cmds = variable_expand_line(new_cmds);
        printf("%s\n", new_cmds);

        if (!dry_run)
        {
            int error = system(new_cmds);

            if (!ignore_errors && error)
            {
                fprintf(stderr, "[%s] Error %d\n", name, error);
                exit(1);
            }
        }
        free(new_cmds);

        p = q + 1;
        q = strchr(p, '\n');
        if (q == NULL) break;
    }   
}

int file_exists(char *name)
{
    FILE *f;

    f = fopen(name, "r");
    if (f)
    {
        fclose(f);
        return (1);
    }

    return (0);
}

char *find_target(char *target)
{
    variable *vpath_var;
    char *vpath;

    if (file_exists(target)) return (target);

    vpath_var = variable_find("VPATH");
    if (vpath_var == NULL) return (NULL);

    vpath = vpath_var->value;

    for (;
         vpath && *vpath;
         )
    {
        char *vpath_part = vpath;
        char saved_c;
        char *new_target;

        /* Skips the initial whitespace. */
        while ((*vpath == ' ') && (*vpath == '\t') && (*vpath == ';')) vpath++;

        /* Finds the end of the current part. */
        while ((*vpath != ' ') && (*vpath != '\t')
               && (*vpath != ';') && (*vpath != '\0')) vpath++;

        saved_c = *vpath;
        *vpath = '\0';

        new_target = xmalloc(strlen(vpath_part) + 1 + strlen(target) + 1);
        strcpy(new_target, vpath_part);
        strcat(new_target, "/");
        strcat(new_target, target);

        *vpath = saved_c;

        if (file_exists(new_target)) return (new_target);

        free(new_target);

        vpath = strchr(vpath, ';');
        if (vpath == NULL) break;
        vpath++;
    }

    return (NULL);
}

void rule_search_and_build(char *name)
{
    rule *r;
    suffix_rule *s;
    char *suffix;

    for (r = rules; r; r = r->next)
    {
        if (strcmp(name, r->name) == 0) break;
    }

    if (r)
    {
        rule_use(r, name);
        return;
    }

    suffix = strrchr(name, '.');
    if (suffix)
    {
        /* Set here because $@ shall evaluate to FULL target name
         * of the current target.
         * This means that the name should not be modified
         * by the search for target. */
        variable_change("@", xstrdup(name));

        for (s = suffix_rules; s; s = s->next)
        {
            if (strcmp(suffix, s->second) == 0)
            {
                char *prereq_name, *p;
                char *new_name;

                /* Creates the name of the prerequisite. */
                prereq_name = xmalloc(strlen(name) + strlen(s->first) + 1);
                memcpy(prereq_name, name, strlen(name) + 1);
                p = strrchr(prereq_name, '.');
                *p = '\0';
                strcat(prereq_name, s->first);

                /* Tries to find the prerequisite. */
                new_name = find_target(prereq_name);
                free(prereq_name);

                if (new_name == NULL) continue; /* Not found. */

                if (strcmp(prereq_name, new_name) == 0) break;

                /* Restore the original suffix in the new name. */
                if (strlen(s->first) < strlen(s->second))
                {
                    new_name = xrealloc(new_name,
                                        (strlen(new_name)
                                         + strlen(s->second)
                                         - strlen(s->first)));
                }
                p = strrchr(new_name, '.');
                *p = '\0';
                strcat(new_name, s->second);

                name = new_name;

                break;
            }
        }

        if (s)
        {
            suffix_rule_use(s, name);
            return;
        }
    }

    {
        char *new_name = find_target(name);

        if (new_name == NULL)
        {
            fprintf(stderr, "No rule to make target `%s'. Stop.", name);
            exit(1);
        }
    }

}

void help(void)
{
    printf("Usage: pdmake [options] [target]...\n");
    printf("Options:\n");
    printf("  -B, --always-make   "
           "Make everything regardless of timestamps.\n");
    printf("  -f, --file FILE     "
           "Read FILE as a makefile.\n");
    printf("  -h, --help          "
           "Print this message and exit.\n");
    printf("  -i, --ignore-errors "
           "Ignore errors from commands.\n");
    printf("  -n, --dry_run       "
           "Run no commands, only print them.\n");
}

int main(int argc, char **argv)
{
    int i;
    char *name = "Makefile";
    char *goal = NULL;
    
    default_goal_var = variable_add(xstrdup(".DEFAULT_GOAL"), xstrdup(""));

    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            switch (argv[i][1])
            {
                case 'B':
                    printf("Rebuilding everything, regardless of timestamps.\n");
                    break;

                case 'f':
                    if (argv[i][2] == '\0')
                    {
                        i++;
                        if (i == argc)
                        {
                            printf("option requires an argument -- f\n");
                            return (0);
                        }
                        name = argv[i];
                    }
                    else
                    {
                        name = argv[i] + 2;
                    }
                    break;

                case 'h':
                    help();
                    return (0);

                case 'i':
                    ignore_errors = 1;
                    break;

                case 'n':
                    dry_run = 1;
                    break;

                case '-':
                    if (strcmp("always-make", argv[i] + 2) == 0)
                    {
                        printf("Rebuilding everything, regardless of timestamps.\n");
                    }
                    else if (strcmp("file", argv[i] + 2) == 0)
                    {
                        i++;
                        if (i == argc)
                        {
                            printf("option `--file' requires an argument\n");
                            return (0);
                        }
                        name = argv[i];
                    }
                    else if (strcmp("help", argv[i] + 2) == 0)
                    {
                        help();
                        return (0);
                    }
                    else if (strcmp("ignore-errors", argv[i] + 2) == 0)
                    {
                        ignore_errors = 1;
                    }
                    else if (strcmp("dry_run", argv[i] + 2) == 0)
                    {
                        dry_run = 1;
                    }
                    else printf("Unknown switch! Use -h for help.\n");
                    break;

                default: printf("Unknown switch! Use -h for help.\n"); break;
            }
        }
        else
        {
            goal = argv[i];
        }
    }

    read_makefile(name);

    if (goal == NULL) goal = default_goal_var->value;

    /* No goal is set, so there is nothing to do. */
    if (strcmp(goal, "") == 0) return (0);
    
    rule_search_and_build(goal);
    
    return (0);
}
