/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

#include "inc_path.h"
#include "xmalloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

include_paths *ic_create_include_paths(void)
{
    int i;
    include_paths *ic = xmalloc(sizeof(*ic));

    for (i = 0; i < INCLUDE_PATH_MAX; i++)
    {
        ic->heads[i] = NULL;
        ic->tails[i] = NULL;
    }

    return (ic);
}

void ic_add_cpp_dir_to_chain(include_paths *ic,
                             cpp_dir *dir,
                             enum include_path_type type)
{
    if (ic->tails[type]) ic->tails[type]->next = dir;
    else ic->heads[type] = dir;

    ic->tails[type] = dir;
}

void ic_free_cpp_dir(cpp_dir *dir)
{
    if (dir->name != NULL) free(dir->name);
    free(dir);
}

/* path must point to permanently allocated string. */
void ic_add_path(include_paths *ic,
                 char *path,
                 enum include_path_type type)
{
    cpp_dir *dir;
    /* Removes extra slashes at the end of path. */
    size_t len = strlen(path);
    char *end = path + len - 1;
    char *start = path + (((len > 2) && (path[1] == ':')) ? 3 : 1);

    for (;
         (end > start) && ((*end == '\\') || (*end == '/'));
         end--)
    {
        *end = '\0';
    }

    dir = xmalloc(sizeof(*dir));
    dir->next = NULL;
    dir->name = path;

    ic_add_cpp_dir_to_chain(ic, dir, type);
}

static void merge_paths(include_paths *ic)
{
    if (ic->tails[INCLUDE_PATH_SYSTEM])
        (ic->tails[INCLUDE_PATH_SYSTEM]->next
         = ic->heads[INCLUDE_PATH_AFTER_SYSTEM]);
    else
        (ic->heads[INCLUDE_PATH_SYSTEM]
         = ic->heads[INCLUDE_PATH_AFTER_SYSTEM]);

    if (ic->tails[INCLUDE_PATH_ANGLED])
        (ic->tails[INCLUDE_PATH_ANGLED]->next
         = ic->heads[INCLUDE_PATH_SYSTEM]);
    else
        (ic->heads[INCLUDE_PATH_ANGLED]
         = ic->heads[INCLUDE_PATH_SYSTEM]);

    if (ic->tails[INCLUDE_PATH_QUOTE])
        (ic->tails[INCLUDE_PATH_QUOTE]->next
         = ic->heads[INCLUDE_PATH_ANGLED]);
    else
        (ic->heads[INCLUDE_PATH_QUOTE]
         = ic->heads[INCLUDE_PATH_ANGLED]);
}

void ic_register_include_chains(include_paths *ic, cpp_reader *reader)
{
    merge_paths(ic);

    cpp_set_include_chains(reader,
                           ic->heads[INCLUDE_PATH_QUOTE],
                           ic->heads[INCLUDE_PATH_ANGLED],
                           0);
}
