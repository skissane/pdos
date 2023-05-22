/******************************************************************************
 * @file            hashtab.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/

/**
 * This hash table implementation is based on the following points:
 * Open addressing
 * Linear probing
 * Robin Hood hashing
 * Prime number of slots
 * Upper probe limit equal to max (ln2 (number of slots), 4)
 */
#include <math.h>
#include <stddef.h>

#include "hashtab.h"

#define max(a, b) ((a) < (b) ? (b) : (a))

#define MINIMAL_PROBE_LIMIT 4

/* The primes are closest largest primes to powers of two. */
static const size_t prime_tab[] = {

    31, 61, 127, 251, 509, 1021, 2039, 4093, 8191,
    16381, 32749, 65521, 131071, 262139, 524287,
    1048573, 2097143, 4194301, 8388593, 16777213,
    33554393, 67108859, 134217689, 268435399,
    536870909, 1073741789, 2147483647

};

static size_t find_prime_greater_than (size_t n)
{
    size_t i;
    
    for (i = 0; i < sizeof (prime_tab) / sizeof (prime_tab[0]); i++) {
        if (prime_tab[i] > n) return prime_tab[i];
    }
    
    return 0;
}

static int internal_log2 (size_t n)
{
    int i;
    
    for (i = 0; n; n >>= 1, i++);
    return i;
}

struct hashtab_entry {

    const void *element;
    int distance_from_desired;
    
#define EMPTY_ENTRY_DISTANCE (-1)

};

struct hashtab {

    struct hashtab_entry *entries;
    
    size_t size;
    size_t number_of_elements;
    
    int probe_limit;
    
    double max_load_factor;
    size_t max_number_of_elements;
    
    /* Callbacks. */
    hashtab_hash_element_func_t hash_element_func;
    hashtab_elements_equal_func_t elements_equal_func;
    hashtab_malloc_func_t malloc_func;
    hashtab_free_func_t free_func;

};

static int rehash (struct hashtab *hashtab, size_t target_size)
{
    struct hashtab old_hashtab = *hashtab;
    struct hashtab_entry *entry;
    
    size_t i;
    
    hashtab->number_of_elements = 0;
    hashtab->size = find_prime_greater_than (target_size);

    if (hashtab->size < target_size) goto failure;
    
    hashtab->probe_limit = max (internal_log2 (hashtab->size), MINIMAL_PROBE_LIMIT);
    hashtab->max_number_of_elements = ceil (hashtab->max_load_factor * hashtab->size);
    
    /* Allocates size + probe_limit + 1 so no bounds checking needs to be done. */
    hashtab->entries = hashtab->malloc_func (sizeof (hashtab->entries[0]) * (hashtab->size + hashtab->probe_limit + 1));
    
    if (hashtab->entries == NULL) goto failure;
    
    for (i = hashtab->size + hashtab->probe_limit + 1, entry = hashtab->entries; i; i--, entry++) {
        entry->element = NULL;
        entry->distance_from_desired = EMPTY_ENTRY_DISTANCE;
    }
    
    /* Inserts the entries from the old table into the new table. */
    if (old_hashtab.size) {
        for (i = old_hashtab.size + old_hashtab.probe_limit, entry = old_hashtab.entries; i; i--, entry++) {
            if (entry->distance_from_desired != EMPTY_ENTRY_DISTANCE) {
                if (hashtab_insert (hashtab, entry->element)) {
                    old_hashtab.free_func (hashtab->entries);
                    goto failure;
                }
            }
        }
        old_hashtab.free_func (old_hashtab.entries);    
    }
    
    return 0;
    
failure:
    *hashtab = old_hashtab;
    return 1;
}

struct hashtab *hashtab_create_hashtab (size_t starting_size,
                                        hashtab_hash_element_func_t hash_element_func,
                                        hashtab_elements_equal_func_t elements_equal_func,
                                        hashtab_malloc_func_t malloc_func,
                                        hashtab_free_func_t free_func)
{
    struct hashtab *hashtab = malloc_func (sizeof (*hashtab));
    
    if (hashtab == NULL) return NULL;
    
    hashtab->max_load_factor = 0.5;
    
    hashtab->hash_element_func = hash_element_func;
    hashtab->elements_equal_func = elements_equal_func;
    hashtab->malloc_func = malloc_func;
    hashtab->free_func = free_func;
    
    hashtab->size = 0;
    
    if (rehash (hashtab, starting_size)) {
        free_func (hashtab);
        return NULL;
    
    }
    
    return hashtab;
}

const void *hashtab_find (struct hashtab *hashtab, const void *element)
{
    size_t index = hashtab->hash_element_func (element) % hashtab->size;
    struct hashtab_entry *entry = hashtab->entries + index;
    
    int distance;
    
    for (distance = 0; entry->distance_from_desired >= distance; distance++, entry++) {
        if (hashtab->elements_equal_func (element, entry->element)) {
            return entry->element;
        }
    }
    
    return NULL;
}

/* The return value is the element that was not possible to insert into the table. */
const void *hashtab_insert (struct hashtab *hashtab, const void *element)
{
    size_t index = hashtab->hash_element_func (element) % hashtab->size;
    struct hashtab_entry *entry = hashtab->entries + index;
    
    int distance_from_desired, swap_distance_from_desired;
    const void *swap_element;
    
    for (distance_from_desired = 0; entry->distance_from_desired >= distance_from_desired; distance_from_desired++, entry++) {
        if (hashtab->elements_equal_func (element, entry->element)) {
            return element;
        }
    }
    
    if (distance_from_desired == hashtab->probe_limit || hashtab->number_of_elements + 1 > hashtab->max_number_of_elements) {
        if (rehash (hashtab, hashtab->size * 2)) return element;        
        return hashtab_insert (hashtab, element);
    } else if (entry->distance_from_desired == EMPTY_ENTRY_DISTANCE) {
        entry->element = element;
        entry->distance_from_desired = distance_from_desired;
        
        hashtab->number_of_elements++;
        return NULL;
    }
    
    swap_element = entry->element;
    entry->element = element;
    element = swap_element;
    
    swap_distance_from_desired = entry->distance_from_desired;
    entry->distance_from_desired = distance_from_desired;
    distance_from_desired = swap_distance_from_desired;
    
    for (distance_from_desired++, entry++; ; entry++) {
        if (entry->distance_from_desired == EMPTY_ENTRY_DISTANCE) {
            entry->element = element;
            entry->distance_from_desired = distance_from_desired;
            
            hashtab->number_of_elements++;
            return NULL;
        } else if (entry->distance_from_desired < distance_from_desired) {
            swap_element = entry->element;
            entry->element = element;
            element = swap_element;
            
            swap_distance_from_desired = entry->distance_from_desired;
            entry->distance_from_desired = distance_from_desired;
            distance_from_desired = swap_distance_from_desired;
            
            distance_from_desired++;
        } else {
            if (++distance_from_desired == hashtab->probe_limit) {
                if (rehash (hashtab, hashtab->size * 2)) return element;                
                return hashtab_insert (hashtab, element);
            }
        }
    }
}

void hashtab_delete (struct hashtab *hashtab, const void *element)
{
    size_t index = hashtab->hash_element_func (element) % hashtab->size;
    struct hashtab_entry *entry = hashtab->entries + index, *next;
    
    int distance;
    
    for (distance = 0; entry->distance_from_desired >= distance; distance++, entry++) {
        if (hashtab->elements_equal_func (element, entry->element)) {
            goto found;
        }
    }
    
    return;
    
found:
    entry->element = NULL;
    entry->distance_from_desired = EMPTY_ENTRY_DISTANCE;
    hashtab->number_of_elements--;
    
    for (next = entry + 1; next->distance_from_desired > 0; entry++, next++) {
        entry->element = next->element;
        entry->distance_from_desired = next->distance_from_desired - 1;
        next->element = NULL;
        next->distance_from_desired = EMPTY_ENTRY_DISTANCE;
    }
}

void hashtab_for_each_element (struct hashtab *hashtab, void (*element_callback) (void *))
{
    struct hashtab_entry *entry;
    size_t i;
    
    for (i = hashtab->size + hashtab->probe_limit, entry = hashtab->entries; i; i--, entry++) {
        if (entry->distance_from_desired != EMPTY_ENTRY_DISTANCE) {
            element_callback ((void *) entry->element);
        }
    }
}

void hashtab_destroy_hashtab (struct hashtab *hashtab)
{
    hashtab->free_func (hashtab->entries);
    hashtab->free_func (hashtab);
}

/** Functions that can be used for implementing the hashtab callbacks. */
hash_value_t hashtab_help_default_hash_string (const void *p)
{
    const unsigned char *str = (const unsigned char *) p;
    unsigned char c;
    
    hash_value_t result = 0;
    
    while ((c = *(str++))) {
        result = result * 67 + c - 113;
    }
    
    return result;
}
