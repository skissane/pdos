/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

typedef struct variable {
    char *name;
    char *value;
} variable;

void variables_init (void);
void variables_destroy (void);

#define variable_add _Pvadd
variable *variable_add(char *name, char *value);
#define variable_find _Pvfind
variable *variable_find(char *name);
#define variable_change _Pvxchg
void variable_change(char *name, char *value);

#define variable_expand_line _Pvxplin
char *variable_expand_line(char *line);

#define parse_var_line _Ppvrlin
void parse_var_line(char *line);
