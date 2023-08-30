/******************************************************************************
 * @file            hashtab.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <stddef.h>

typedef size_t hash_value_t;

/* Callback function pointer types. */
typedef hash_value_t (*hashtab_hash_element_func_t) (const void *);
typedef int (*hashtab_elements_equal_func_t) (const void *, const void *);
typedef void (*hashtab_free_func_t) (void *);
typedef void *(*hashtab_malloc_func_t) (size_t);

struct hashtab *hashtab_create_hashtab (size_t starting_size,
                                        hashtab_hash_element_func_t hash_element_func,
                                        hashtab_elements_equal_func_t elements_equal_func,
                                        hashtab_malloc_func_t malloc_func,
                                        hashtab_free_func_t free_func);
const void *hashtab_find (struct hashtab *hashtab, const void *element);

void hashtab_delete (struct hashtab *hashtab, const void *element);
void hashtab_for_each_element (struct hashtab *hashtab, void (*element_callback) (void *));
void hashtab_destroy_hashtab (struct hashtab *hashtab);

/** The return value is the element that was not possible to insert into the table. */
const void *hashtab_insert (struct hashtab *hashtab, const void *element);

/** Functions that can be used for implementing the hashtab callbacks. */
hash_value_t hashtab_help_default_hash_string (const void *p);
