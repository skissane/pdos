/******************************************************************************
 * @file            libas.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#if     defined (_WIN32)
# define    PATHSEP                     ";"
#else
# define    PATHSEP                     ":"
#endif

void as_parse_args (int *pargc, char ***pargv, int optind);
void dynarray_add (int *nb_ptr, void *ptab, void *data);
