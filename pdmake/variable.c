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

#include "variable.h"
#include "xmalloc.h"

extern int doing_inference_rule_commands;

static variable *vars = NULL;

variable *variable_add(char *name, char *value)
{
    variable *var = xmalloc(sizeof(*var));

    var->name = name;
    var->len = strlen(name);
    var->value = value;

    var->next = vars;
    vars = var;

    return (var);
}

variable *variable_find(char *name)
{
    size_t len = strlen(name);
    variable *var;

    for (var = vars; var; var = var->next)
    {
        if ((var->len == len) && (strcmp(name, var->name) == 0)) break;
    }

    if (var && !doing_inference_rule_commands && var->name[0] == '<' && var->name[1] == '\0')
    {
        fprintf(stderr, "\"$<\" is not valid outside of inference rules, exiting\n");
        exit(EXIT_FAILURE);
    }

    return (var);
}

void variable_change(char *name, char *value)
{
    variable *var = variable_find(name);
    if (var == NULL) var = variable_add(xstrdup(name), value);
    else
    {
        free(var->value);
        var->value = value;
    }
}

static char *variable_suffix_replace(char *body, const char *from_s,
    const char *to_s)
{
    char *new_body = xstrdup(body);
    char *p;
    while ((p = strstr(new_body, from_s)) != NULL)
    {
        if (strlen(from_s) == strlen(to_s)) /* Equally sized */
        {
            memcpy(p, to_s, strlen(to_s));
        }
        else if (strlen(from_s) > strlen(to_s)) /* Deflate */
        {
            size_t rem = strlen(from_s) - strlen(to_s);
            memcpy(p, to_s, strlen(to_s));
            while (rem--)
            {
                p[strlen(to_s) + rem] = ' ';
            }
        }
        else /* Inflate */
        {
            size_t rem = strlen(to_s) - strlen(from_s);
            /* Move the entire string rem characters to make space for to_s */
            new_body = xrealloc(new_body, strlen(new_body) + rem);
            memmove(p + rem, p, strlen(p) + 1);
            memcpy(p, to_s, strlen(to_s));
        }
    }
    return new_body;
}

#define expect_char(t, s, c) \
    t = strchr(s, c); \
    if (t == NULL) { fprintf(stderr, "Expected '%c'\n", c); return (line); }

#define expect_token(t, s, delim) \
    t = strtok(s, delim); \
    if (t == NULL) { fprintf(stderr, "Expected '" delim "'\n"); return (line); }

char *variable_expand_line(char *line)
{
    size_t pos = 0;
    while (line[pos])
    {
        if (line[pos] == '$')
        {
            char *new, *replacement = "", *alloc_repl = NULL;
            char *p, *t;
            variable *var = NULL;
            char name[2] = {0, 0};

            if (line[pos + 1] == '$')
            {
                pos += 2;
                continue;
            }

            if (line[pos + 1] == '(')
            {
                char *body = line + pos + 2;
                char *q = body, *s;
                char *cmd, *cmd_body;
                
                size_t cmd_len = 0;
                while (isalpha(*q)) {
                    q++;
                    cmd_len++;
                }

                cmd = xstrndup(body, cmd_len);
                cmd_body = xstrndup(body + cmd_len, strlen(body));
                cmd_body = variable_expand_line(cmd_body);

                /*printf("ACMD=%s\n", cmd_body);*/
                t = p = strrchr(cmd_body, ')');
                if (t == NULL)
                {
                    fprintf(stderr, "Expected '%c'\n", ')');
                    return (line);
                }
                *t = '\0';
                /*printf("CMDB=%s\n", cmd_body);*/

                s = cmd_body;
                while (isspace(*s)) s++;
                if (!strcmp(cmd, "subst"))
                {
                    char *from_s, *to_s, *to_repl;
                    char *token;
                    expect_token(token, s, ",");
                    from_s = xstrdup(token);
                    expect_token(token, NULL, ",");
                    to_s = xstrdup(token);
                    expect_token(token, NULL, ",");
                    to_repl = xstrdup(token);
                    alloc_repl = variable_suffix_replace(to_repl, from_s, to_s);
                    free(from_s);
                    free(to_s);
                    free(to_repl);
                }
                else
                {
                    /* Take in account suffix replacements of the form:
                    * $(VAR:src=dst) */
                    p = strchr(body, ':');
                    /*printf("[%s -> %s]\n", p, line + pos);*/
                    if (p != NULL)
                    {
                        variable *src_var;
                        char *s1, *s2;
                        /* TODO: This could be done more efficiently :^) */
                        char *varname = xstrdup(body);
                        char *equals_sign = strchr(p, '=');
                        if (equals_sign == NULL)
                        {
                            fprintf(stderr, "+++Invalid suffix replacement!\n");
                            return (line);
                        }
                        expect_char(t, varname, ':');
                        *t = '\0';

                        p++; /* Skip colon ':' */
                        s1 = xstrdup(p);
                        expect_char(t, s1, '=');
                        *t = '\0';
                        s2 = xstrdup(equals_sign + 1);
                        expect_char(t, s2, ')');
                        *t = '\0';
                        /*printf("REPL=%s,SRC=%s,DST=%s\n", varname, s1, s2);*/

                        src_var = variable_find(varname);
                        free(varname);

                        alloc_repl = variable_suffix_replace(src_var->value, s1, s2);
                        /*printf("VAR=%s\n", alloc_repl);*/
                        free(s1);
                        free(s2);

                        p = strrchr(body, ')');
                        *p = '\0';
                    }
                    else
                    {
                        p = strchr(body, ')');
                        if (p == NULL)
                        {
                            fprintf(stderr, "+++Invalid variable usage!\n");
                            return (line);
                        }

                        *p = '\0';
                        p++;
                        
                        var = variable_find(line + pos + 2);
                    }
                }

                free(cmd_body);
                free(cmd);
            }
            else
            {
                p = line + pos + 2;
                name[0] = line[pos + 1];
                var = variable_find(name);
            }

            if (var) replacement = var->value;
            if (alloc_repl) replacement = alloc_repl;

            new = xmalloc(pos + strlen(replacement) + strlen(p) + 1);
            memcpy(new, line, pos);
            memcpy(new + pos, replacement, strlen(replacement));
            memcpy(new + pos + strlen(replacement), p, strlen(p) + 1);
            free(line);
            line = new;
            if (alloc_repl) free(alloc_repl);
            continue;
        }
        pos++;
    }
    return (line);
}

void parse_var_line(char *line)
{
    char *equals_sign = strchr(line, '=');
    variable *var;
    int opt = 0;
    int expand = 0;
    char *p;

    if (equals_sign == NULL)
    {
        fprintf(stderr, "+++Invalid variable definition!\n");
        return;
    }

    p = equals_sign;
    /* Overrideable ?= assignment */
    if (p[-1] == '?')
    {
        opt = 1;
    }
    /* "Expand on define instead of usage", however we won't for now and it
     * shouldn't cause any issues */
    else if (p[-1] == ':')
    {
        expand = 1;
    }

    /* Any <blank> characters immediately
     * before the equals sign must be ignored. */
    for ( ;
         (p > line) && isspace(p[-1]);
         p--) ;
    *p = '\0';

    /* Any <blank> characters immediately
     * after the equals sign must be ignored. */
    for (p = equals_sign;
         isspace(p[1]);
         p++) ;
    
    /* ?= assignment only takes effect when the variable isn't set yet */
    var = variable_find(p + 1);
    if (opt && var)
    {
        return;
    }

    variable_add(xstrdup(line), xstrdup(p + 1));
}
