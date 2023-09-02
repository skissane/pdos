/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

#include "cpplib.h"

enum include_path_type {
    INCLUDE_PATH_QUOTE,
    INCLUDE_PATH_ANGLED,
    INCLUDE_PATH_SYSTEM,
    INCLUDE_PATH_AFTER_SYSTEM,
    INCLUDE_PATH_MAX
};

typedef struct {
    cpp_dir *heads[INCLUDE_PATH_MAX];
    cpp_dir *tails[INCLUDE_PATH_MAX];
    cpp_dir *to_free_chain;
} include_paths;

include_paths *ic_create_include_paths(void);
void ic_destroy_include_paths (include_paths *ic);
    
void ic_add_cpp_dir_to_chain(include_paths *ic,
                             cpp_dir *dir,
                             enum include_path_type type);
void ic_free_cpp_dir(cpp_dir *dir);
void ic_add_path(include_paths *ic,
                 char *path,
                 enum include_path_type type);
void ic_register_include_chains(include_paths *ic, cpp_reader *reader);
