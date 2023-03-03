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
#include "hashtab.h"

extern int doing_inference_rule_commands;

static struct hashtab *variables_hashtab = NULL;

static hash_value_t hash_variable (const void *p)
{
    const struct variable *variable = (const struct variable *) p;
    
    return hashtab_help_default_hash_string (variable->name);
}

static int equal_variable (const void *p1, const void *p2)
{
    const struct variable *variable1 = (const struct variable *) p1;
    const struct variable *variable2 = (const struct variable *) p2;
    
    return strcmp (variable1->name, variable2->name) == 0;
}

static struct variable *find_variable_internal (const char *name)
{
    struct variable fake = { NULL, NULL };

    fake.name = (char *) name;
    
    return (struct variable *) hashtab_find (variables_hashtab, &fake);
}

static void free_variable_internal (void *p)
{
    struct variable *var = (struct variable *) p;

    free (var->name);
    free (var->value);
    free (var);
}

void variables_init (void)
{
    variables_hashtab = hashtab_create_hashtab (0,
                                                &hash_variable,
                                                &equal_variable,
                                                &xmalloc,
                                                &free);
}

void variables_destroy (void)
{
    hashtab_for_each_element (variables_hashtab, &free_variable_internal);
    hashtab_destroy_hashtab (variables_hashtab);
}

variable *variable_add(char *name, char *value, enum variable_origin origin)
{
    variable *var = xmalloc(sizeof(*var));

    var->name = name;
    var->value = value;
    var->origin = origin;

    if (hashtab_insert (variables_hashtab, var)) {

        fprintf(stderr, "failed to insert variable '%s' into hashtab\n", var->name);
        exit(EXIT_FAILURE);

    }

    return (var);
}

variable *variable_find(char *name)
{
    variable *var;

    var = find_variable_internal (name);

    if (var && !doing_inference_rule_commands && var->name[0] == '<' && var->name[1] == '\0')
    {
        fprintf(stderr, "\"$<\" is not valid outside of inference rules, exiting\n");
        exit(EXIT_FAILURE);
    }

    return (var);
}

void variable_change(char *name, char *value, enum variable_origin origin)
{
    variable *var = variable_find(name);
    if (var == NULL) variable_add(xstrdup(name), value, origin);
    else
    {
        switch (origin) {

            case VAR_ORIGIN_AUTOMATIC:
            case VAR_ORIGIN_COMMAND_LINE:
            
                /* Overrides everything. */
                break;

            case VAR_ORIGIN_FILE:

                if (var->origin == VAR_ORIGIN_FILE) {
                    break;
                }

                return;

        }
        
        free(var->value);
        var->value = value;
        var->origin = origin;
    }
}

static char *variable_suffix_replace(char *body,
                                     const char *from_s,
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

char *variable_expand_line (char *line)
{
    size_t pos = 0;
    
    while (line[pos]) {
        
        if (line[pos] == '$') {
            
            char *new, *replacement = "";
            char *p_after_variable;
            variable *var = NULL;
            char *alloc_replacement = NULL;

            if (line[pos + 1] == '$') {
                pos += 2;
                continue;
            }

            if (line[pos + 1] == '(' || line[pos + 1] == '{') {
                
                char *body = line + pos + 2;
                char *q = body;
                char *content;
                
                int paren_inbalance = 1;
                char opening_paren = line[pos + 1];
                
                while (paren_inbalance) {

                    if (*q == opening_paren) {
                        paren_inbalance++;
                    } else if (*q == ')' && opening_paren == '(') {
                        paren_inbalance--;
                    } else if (*q == '}' && opening_paren == '{') {
                        paren_inbalance--;
                    } else if (*q == '\0') {
                        fprintf(stderr, "unterminated variable reference. Stop.\n");
                        exit(EXIT_FAILURE);
                    }
                    
                    q++;
                    
                }
                q--;
                p_after_variable = q + 1;
                content = variable_expand_line (xstrndup (body, q - body));

                /*
                 * Content is one of the following:
                 * 1. Function call in the form (function and its arguments)
                 * 2. Suffix replacement in the form (variable_name : from = to)
                 * 3. Just variable name: (variable_name).
                 */

                /* Currently no functions are supported. */
                
                if ((q = strchr (content, '='))) {

                    char *colon = strchr (content, ':');

                    if (colon && colon != content && colon < q) {

                        char *from, *to, *p;

                        for (p = colon; p != content && isspace (p[-1]); p--) {}
                        *p = '\0';

                        var = variable_find (content);

                        for (from = colon + 1; isspace (*from); from++) {}
                        for (p = q; p != from && isspace (p[-1]); p--) {}
                        *p = '\0';

                        for (to = q + 1; isspace (*to); to++) {}
                        for (p = to + strlen (to); p != to && isspace (p[-1]); p--) {}
                        *p = '\0';

                        /* If from is empty, variable value should be used as is and suffix replacement should be ignored. */
                        if (*from && var) {
                            alloc_replacement = variable_suffix_replace (var->value, from, to);
                        }
                        
                    }

                    /* Otherwise it is just a bad variable name and should result in "" replacement. */
                    
                } else {
                    var = variable_find (content);
                }
                
                free (content);
                
            } else {

                char name[2] = {0, 0};
                
                p_after_variable = line + pos + 2;
                name[0] = line[pos + 1];
                var = variable_find (name);
                
            }

            if (var) replacement = var->value;
            if (alloc_replacement) replacement = alloc_replacement;

            new = xmalloc (pos + strlen (replacement) + strlen (p_after_variable) + 1);
            memcpy (new, line, pos);
            memcpy (new + pos, replacement, strlen (replacement));
            memcpy (new + pos + strlen (replacement), p_after_variable, strlen (p_after_variable) + 1);
            free (line);
            line = new;

            free (alloc_replacement);
            continue;
            
        }
        
        pos++;
    }
    
    return line;
}

void parse_var_line(char *line, enum variable_origin origin)
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

    variable_change (line, xstrdup(p + 1), origin);
}
