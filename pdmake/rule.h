/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

typedef struct rule {
    char *name;
    struct dep *deps;
    struct commands *cmds;
} rule;

typedef struct suffix_rule {
    struct suffix_rule *next;

    char *first;
    char *second;

    struct commands *cmds;
} suffix_rule;

extern struct suffix_rule *suffix_rules;

void rules_init (void);
void rules_destroy (void);

void rule_add (const char *name, struct dep *deps, const struct commands *cmds);
struct rule *rule_find (const char *name);

void rule_search_and_build (char *name);

#define rule_add_suffix _Prasu
void rule_add_suffix (char *name, const struct commands *cmds);
