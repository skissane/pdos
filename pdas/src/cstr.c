/******************************************************************************
 * @file            cstr.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include    <stdlib.h>
#include    <string.h>

#include    "as.h"
#include    "cstr.h"

static void cstr_realloc (CString *cstr, int new_size) {

    int size = cstr->size_allocated;
    
    if (size < 8) {
        size = 8;
    }
    
    while (size < new_size) {
        size *= 2;
    }
    
    cstr->data = xrealloc (cstr->data, size);
    cstr->size_allocated = size;

}

void cstr_ccat (CString *cstr, int ch) {

    int size = cstr->size + 1;
    
    if (size > cstr->size_allocated) {
        cstr_realloc (cstr, size);
    }
    
    ((unsigned char *) cstr->data)[size - 1] = ch;
    cstr->size = size;

}

void cstr_cat (CString *cstr, const char *str, int len) {

    int size;
    
    if (len <= 0) {
        len = strlen (str) + 1 + len;
    }
    
    size = cstr->size + len;
    
    if (size > cstr->size_allocated) {
        cstr_realloc (cstr, size);
    }
    
    memmove (((unsigned char *) cstr->data) + cstr->size, str, len);
    cstr->size = size;

}

void cstr_new (CString *cstr) {
    memset (cstr, 0, sizeof (CString));
}

void cstr_free (CString *cstr) {

    free (cstr->data);
    cstr_new (cstr);

}
