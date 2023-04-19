/******************************************************************************
 * @file            options.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
struct short_option {
    char letter;
    int index, flags;
};

struct long_option {
    const char *name;
    size_t name_len;
    int index, flags;
};

#define OPTION_NO_ARG            (1U << 0)
#define OPTION_HAS_ARG           (1U << 1)
#define OPTION_HAS_OPTIONAL_ARG  (1U << 2)

