/******************************************************************************
 * @file            cstr.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
typedef struct CString {

    int size;
    void *data;
    
    int size_allocated;

} CString;

void cstr_ccat (CString *cstr, int ch);
void cstr_cat (CString *cstr, const char *str, int len);

void cstr_new (CString *cstr);
void cstr_free (CString *cstr);
