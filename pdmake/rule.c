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

void rule_add (const char *name, struct dep *deps, const struct commands *cmds)
{
    struct rule *r;

    if (!(r = rule_find (name))) {
        r = xmalloc (sizeof *r);
        r->name = xstrdup (name);
        r->deps = NULL;
        r->cmds = NULL;
        hashtab_insert (rules_hashtab, r);
    }

    if (cmds) {
        if (r->cmds) {
            fprintf (stderr, "warning: overriding commands for target '%s'\n", name);
            free (r->cmds->text);
            free (r->cmds);
        }

        r->cmds = xmalloc (sizeof *r->cmds);
        r->cmds->text = xstrdup (cmds->text);
        r->cmds->len = cmds->len;
    }
        
    if (deps) {
        struct dep *new_dep;

        for (new_dep = deps; new_dep; new_dep = new_dep->next) {
            struct dep *old_dep;

            if (!r->deps) {
                r->deps = xmalloc (sizeof *r->deps);
                r->deps->name = xstrdup (new_dep->name);
                r->deps->next = NULL;
                continue;
            }
            
            for (old_dep = r->deps; 1; old_dep = old_dep->next) {
                if (strcmp (new_dep->name, old_dep->name) == 0) break;

                if (!old_dep->next) {
                    old_dep->next = xmalloc (sizeof *old_dep->next);
                    old_dep->next->name = xstrdup (new_dep->name);
                    old_dep->next->next = NULL;
                    break;
                }
            }
        }
    }
}

struct rule *rule_find (const char *name)
{
    struct rule fake = { NULL, NULL, NULL };

    fake.name = (char *) name;
    
    return (struct rule *) hashtab_find (rules_hashtab, &fake);
}

void rule_add_suffix (char *name, const struct commands *cmds)
{
    struct suffix_rule *s;
    char *first, *second;
    char *p = strchr (name + 1, '.');

    if (p) {
        *p = '\0';
        first = xstrdup (name);
        *p = '.';
        second = xstrdup (p);
    } else {
        first = xstrdup (name);
        second = NULL;
    }

    for (s = suffix_rules; s; s = s->next) {
        if (((second && s->second && strcmp (second, s->second) == 0)
             || (!second && !s->second))
            && strcmp (first, s->first) == 0) break;
    }

    if (!s) {
        s = xmalloc (sizeof *s);
        s->first = first;
        s->second = second;
        s->cmds = NULL;
        s->next = suffix_rules;
        suffix_rules = s;
    } else {
        free (first);
        free (second);
    }

    if (cmds) {
        if (s->cmds) {
            fprintf (stderr, "warning: overriding commands for target '%s'\n", name);
            free (s->cmds->text);
            free (s->cmds);
        }

        s->cmds = xmalloc (sizeof *s->cmds);
        s->cmds->text = xstrdup (cmds->text);
        s->cmds->len = cmds->len;
    }
}
