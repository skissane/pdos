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
#include "xmalloc.h"

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

char *variable_expand_line(char *line)
{
    size_t pos = 0;

    while (line[pos])
    {
        if (line[pos] == '$')
        {
            char *p;
            variable *var;
            char name[2] = {0, 0};
            char *new;
            char *replacement = "";
            
            if (line[pos + 1] == '$')
            {
                pos += 2;
                continue;
            }

            if (line[pos + 1] == '(')
            {
                p = strchr(line + pos + 2, ')');

                if (p == NULL)
                {
                    fprintf(stderr, "+++Invalid variable usage!\n");
                    return (line);
                }

                *p = '\0';
                p++;
                
                var = variable_find(line + pos + 2);
            }
            else
            {
                p = line + pos + 2;
                name[0] = line[pos + 1];

                var = variable_find(name);
            }

            if (var) replacement = var->value;

            new = xmalloc(pos + strlen(replacement) + strlen(p) + 1);
            memcpy(new, line, pos);
            memcpy(new + pos, replacement, strlen(replacement));
            memcpy(new + pos + strlen(replacement), p, strlen(p) + 1);
            free(line);
            line = new;

            continue;
        }

        pos++;
    }

    return (line);
}

void parse_var_line(char *line)
{
    char *equals_sign = strchr(line, '=');
    char *p;

    if (equals_sign == NULL)
    {
        fprintf(stderr, "+++Invalid variable definition!\n");
        return;
    }

    /* Any <blank> characters immediately
     * before the equals sign must be ignored. */
    for (p = equals_sign;
         (p > line) && ((p[-1] == ' ') || (p[-1] == '\t'));
         p--) ;
    *p = '\0';

    /* Any <blank> characters immediately
     * after the equals sign must be ignored. */
    for (p = equals_sign;
         (p[1] == ' ') || (p[1] == '\t');
         p++) ;

    variable_add(xstrdup(line), xstrdup(p + 1));
}
