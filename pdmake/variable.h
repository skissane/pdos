/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

enum variable_flavor {
    VAR_FLAVOR_RECURSIVELY_EXPANDED,
    VAR_FLAVOR_SIMPLY_EXPANDED,
    VAR_FLAVOR_IMMEDIATELY_EXPANDED
};

enum variable_origin {
    VAR_ORIGIN_AUTOMATIC,
    VAR_ORIGIN_COMMAND_LINE,
    VAR_ORIGIN_FILE
};    

typedef struct variable {
    char *name;
    char *value;
    enum variable_flavor flavor;
    enum variable_origin origin;
} variable;

#define variables_init _Pvinit
void variables_init (void);
#define variables_destroy _Pvdest
void variables_destroy (void);

#define variable_add _Pvadd
variable *variable_add (char *name, char *value, enum variable_origin origin);
#define variable_find _Pvfind
variable *variable_find (char *name);
#define variable_change _Pvxchg
variable *variable_change (char *name, char *value, enum variable_origin origin);

#define variable_expand_line _Pvxplin
char *variable_expand_line(char *line);

#define parse_var_line _Ppvrlin
void parse_var_line(char *line, enum variable_origin origin);
