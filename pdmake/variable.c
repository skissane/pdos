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
#include "read.h"
#include "xmalloc.h"
#include "hashtab.h"

extern int doing_inference_rule_commands;

struct built_in_function {
    const char *name;
    char *(*function) (char *input);
};

static char *function_eval (char *input)
{
    read_memory_makefile (input);

    /* The result of eval must always be an empty string. */
    return NULL;
}

static struct built_in_function built_in_functions[] = {
    {"eval", &function_eval},
    {NULL, NULL}
};

static struct hashtab *built_in_functions_hashtab = NULL;
static struct hashtab *variables_hashtab = NULL;

static hash_value_t hash_built_in_function (const void *p)
{
    const struct built_in_function *function = (const struct built_in_function *) p;
    
    return hashtab_help_default_hash_string (function->name);
}

static int equal_built_in_function (const void *p1, const void *p2)
{
    const struct built_in_function *function1 = (const struct built_in_function *) p1;
    const struct built_in_function *function2 = (const struct built_in_function *) p2;
    
    return strcmp (function1->name, function2->name) == 0;
}

static struct built_in_function *find_built_in_function (const char *name)
{
    struct built_in_function fake = {NULL, NULL};

    fake.name = name;

    return (struct built_in_function *) hashtab_find (built_in_functions_hashtab, &fake);
}

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
    struct built_in_function *function;

    built_in_functions_hashtab = hashtab_create_hashtab (0,
                                                         &hash_built_in_function,
                                                         &equal_built_in_function,
                                                         &xmalloc,
                                                         &free);
    for (function = built_in_functions; function->name; function++) {
        if (hashtab_insert (built_in_functions_hashtab, function)) {
            fprintf(stderr, "failed to insert built-in function '%s' into hashtab\n", function->name);
            exit(EXIT_FAILURE);
        }
    }

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
    hashtab_destroy_hashtab (built_in_functions_hashtab);
}

variable *variable_add (char *name, char *value, enum variable_origin origin)
{
    variable *var = xmalloc (sizeof (*var));

    var->name = name;
    var->value = value;
    var->flavor = VAR_FLAVOR_RECURSIVELY_EXPANDED;
    var->origin = origin;

    if (hashtab_insert (variables_hashtab, var)) {
        fprintf(stderr, "failed to insert variable '%s' into hashtab\n", var->name);
        exit(EXIT_FAILURE);
    }

    return var;
}

variable *variable_find (char *name)
{
    variable *var;

    var = find_variable_internal (name);

    if (var && !doing_inference_rule_commands && var->name[0] == '<' && var->name[1] == '\0') {
        fprintf (stderr, "\"$<\" is not valid outside of inference rules, exiting\n");
        exit (EXIT_FAILURE);
    }

    return var;
}

variable *variable_change (char *name, char *value, enum variable_origin origin)
{
    variable *var = variable_find (name);
    
    if (var == NULL) return variable_add (xstrdup (name), value, origin);
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

                free (value);
                return NULL;

        }
        
        free(var->value);
        var->value = value;
        var->origin = origin;

        return var;
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

                /* "$$" is used to escape '$'. */
                p_after_variable = line + pos + 2;
                pos += 1;
                
            } else if (line[pos + 1] == '(' || line[pos + 1] == '{') {
                
                char *body = line + pos + 2;
                char *q = body;
                char *content;
                struct built_in_function *function;
                
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

                {
                    char saved_c;

                    for (q = content; *q && !isspace (*q); q++) {}
                    saved_c = *q;
                    *q = '\0';

                    function = find_built_in_function (content);
                    
                    *q = saved_c;
                }
                
                if (function) {
                    
                    for (; isspace (*q); q++) {}
                    alloc_replacement = function->function (q);
                    
                } else if ((q = strchr (content, '='))) {

                    char *colon = strchr (content, ':');

                    if (colon && colon != content && colon < q) {

                        char *from, *to, *p;

                        /*
                         * Whitespace before the colon must not be stripped
                         * as variable names can contain trailing whitespace.
                         * ("ABC  $(nothing) = something")
                         */
                        *colon = '\0';
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
                
            } else if (line[pos + 1]) {

                char name[2] = {0, 0};
                
                p_after_variable = line + pos + 2;
                name[0] = line[pos + 1];
                var = variable_find (name);
                
            } else {
                /* '$' at the end of line should be treated as a bad variable name, not an error. */
                p_after_variable = line + pos + 1;
            }

            if (var) replacement = var->value;
            if (alloc_replacement) replacement = alloc_replacement;

            new = xmalloc (pos + strlen (replacement) + strlen (p_after_variable) + 1);
            memcpy (new, line, pos);
            memcpy (new + pos, replacement, strlen (replacement));
            memcpy (new + pos + strlen (replacement), p_after_variable, strlen (p_after_variable) + 1);
            free (line);
            line = new;

            /* Simply expanded variables should be only copied without trying to expand the value. */
            if (!alloc_replacement && var && var->flavor == VAR_FLAVOR_SIMPLY_EXPANDED) {
                pos += strlen (replacement);
            }

            free (alloc_replacement);
            continue;
            
        }
        
        pos++;
    }
    
    return line;
}

void parse_var_line (char *line, enum variable_origin origin)
{
    enum {
        VAR_ASSIGN,
        VAR_CONDITIONAL_ASSIGN,
        VAR_APPEND,
        VAR_SHELL
    } opt = VAR_ASSIGN;
    enum variable_flavor flavor = VAR_FLAVOR_RECURSIVELY_EXPANDED;
    variable *var;
    char *var_name, *new_value;
    char *equals_sign = strchr (line, '=');
    char *p;

    if (equals_sign == NULL)
    {
        fprintf(stderr, "+++Invalid variable definition!\n");
        return;
    }

    p = equals_sign;

    switch (p - line) {

        default:
            if (p[-1] == ':' && p[-2] == ':' && p[-3] == ':') {
                flavor = VAR_FLAVOR_IMMEDIATELY_EXPANDED;
                p -= 3;
                break;
            }
            /* Fall through */

        case 2:
            if (p[-1] == ':' && p[-2] == ':') {
                flavor = VAR_FLAVOR_SIMPLY_EXPANDED;
                p -= 2;
                break;
            }
            /* Fall through */

        case 1:
            if (p[-1] == ':') {
                flavor = VAR_FLAVOR_SIMPLY_EXPANDED;
                p--;
                break;
            }
            if (p[-1] == '?') {
                opt = VAR_CONDITIONAL_ASSIGN;
                p--;
                break;
            }
            if (p[-1] == '+') {
                opt = VAR_APPEND;
                p--;
                break;
            }
            if (p[-1] == '!') {
                opt = VAR_SHELL;
                p--;
                break;
            }
            break;

        case 0:
            break;

    }
    
    /* Any <blank> characters immediately
     * before the equals sign must be ignored. */
    for (; p > line && isspace (p[-1]); p--) {}

    var_name = variable_expand_line (xstrndup (line, p - line));

    if (*var_name == '\0') {
        fprintf(stderr, "empty variable name. Stop.\n");
        exit(EXIT_FAILURE);
    }

    /* ?= assignment only takes effect when the variable isn't set yet */
    var = variable_find (var_name);
    if (opt == VAR_CONDITIONAL_ASSIGN && var) {
        free (var_name);
        return;
    }

    /* Any <blank> characters immediately
     * after the equals sign must be ignored. */
    for (p = equals_sign;
         isspace (p[1]);
         p++) ;

    new_value = xstrdup (p + 1);

    if (opt == VAR_ASSIGN || opt == VAR_CONDITIONAL_ASSIGN) {
        switch (flavor) {

            case VAR_FLAVOR_RECURSIVELY_EXPANDED:
                /* Nothing needs to be done about the value for this type. */
                break;

            case VAR_FLAVOR_SIMPLY_EXPANDED:
                new_value = variable_expand_line (new_value);
                break;

            case VAR_FLAVOR_IMMEDIATELY_EXPANDED:
                new_value = variable_expand_line (new_value);
                {
                    size_t dollar_count;
                    char *temp, *p2;

                    for (dollar_count = 0, p = new_value; *p; p++) {
                        if (*p == '$') dollar_count++;
                    }

                    temp = xmalloc (strlen (new_value) + 1 + dollar_count);
                    for (p = new_value, p2 = temp; *p; p++, p2++) {
                        *p2 = *p;
                        if (*p == '$') {
                            p2[1] = '$';
                            p2++;
                        }
                    }
                    *p2 = '\0';
                    free (new_value);
                    new_value = temp;
                }
                break;

        }
    } else if (opt == VAR_APPEND) {
        fprintf(stderr, "+++internal error: += not supported yet. Stop.\n");
        exit(EXIT_FAILURE);
    } else if (opt == VAR_SHELL) {
        fprintf(stderr, "+++internal error: != not supported yet. Stop.\n");
        exit(EXIT_FAILURE);
    }

    if ((var = variable_change (var_name, new_value, origin))) {
        var->flavor = flavor;
    }
    free (var_name);
}
