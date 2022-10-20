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

#include "dep.h"
#include "commands.h"
#include "rule.h"
#include "xmalloc.h"
#include "hashtab.h"

static struct hashtab *rules_hashtab = NULL;
suffix_rule *suffix_rules = NULL;

static hash_value_t hash_rule (const void *p)
{
    const struct rule *rule = (const struct rule *) p;
    
    return hashtab_help_default_hash_string (rule->name);
}

static int equal_rule (const void *p1, const void *p2)
{
    const struct rule *rule1 = (const struct rule *) p1;
    const struct rule *rule2 = (const struct rule *) p2;
    
    return strcmp (rule1->name, rule2->name) == 0;
}

static void free_rule_internal (void *p)
{
    struct rule *rule = (struct rule *) p;

    free (rule->name);

    if (rule->cmds) {
        free (rule->cmds->text);
        free (rule->cmds);
    }

    if (rule->deps) {
        struct dep *dep, *next_dep;

        for (dep = rule->deps; dep; dep = next_dep) {
            next_dep = dep->next;
            free (dep->name);
            free (dep);
        }
    }

    free (rule);
}

void rules_init (void)
{
    rules_hashtab = hashtab_create_hashtab (0,
                                            &hash_rule,
                                            &equal_rule,
                                            &xmalloc,
                                            &free);
}

void rules_destroy (void)
{
    struct suffix_rule *next_suffix_rule;
    
    hashtab_for_each_element (rules_hashtab, &free_rule_internal);
    hashtab_destroy_hashtab (rules_hashtab);

    for (; suffix_rules; suffix_rules = next_suffix_rule) {
        next_suffix_rule = suffix_rules->next;
        free (suffix_rules->first);
        free (suffix_rules->second);
        if (suffix_rules->cmds) {
            free (suffix_rules->cmds->text);
            free (suffix_rules->cmds);
        }
        free (suffix_rules);
    }
}

void rule_add (char *name, struct dep *deps, struct commands *cmds)
{
    struct rule *r = xmalloc (sizeof(*r));

    r->name = xstrdup (name);
    r->deps = deps;
    r->cmds = cmds;

    hashtab_insert (rules_hashtab, r);
}

struct rule *rule_find (const char *name)
{
    struct rule fake = { NULL, NULL, NULL };

    fake.name = (char *) name;
    
    return (struct rule *) hashtab_find (rules_hashtab, &fake);
}

void rule_add_suffix (char *name, struct commands *cmds)
{
    struct suffix_rule *s = xmalloc (sizeof(*s));
    char *p = strchr (name + 1, '.');

    *p = '\0';
    s->first = xstrdup (name);
    *p = '.';
    s->second = xstrdup (p);

    s->cmds = cmds;

    s->next = suffix_rules;
    suffix_rules = s;
}
