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
#include "internal.h"
#include "xmalloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

unsigned char _cpp_trigraph_map[0x100];

static void init_trigraph_map(void)
{
    _cpp_trigraph_map['='] = '#';
    _cpp_trigraph_map[')'] = ']';
    _cpp_trigraph_map['!'] = '|';
    _cpp_trigraph_map['('] = '[';
    _cpp_trigraph_map['\''] = '^';
    _cpp_trigraph_map['>'] = '}';
    _cpp_trigraph_map['/'] = '\\';
    _cpp_trigraph_map['<'] = '{';
    _cpp_trigraph_map['-'] = '~';
}

static void init(void)
{
    static int initialized = 0;

    if (initialized == 0)
    {
        initialized = 1;

        init_trigraph_map();
    }
}

void cpp_set_lang(cpp_reader *reader, enum c_lang lang)
{
    CPP_OPTION(reader, lang) = lang;
    
    CPP_OPTION(reader, trigraphs) = 1;
    CPP_OPTION(reader, c99) = (lang == STD_C17) ? 1 : 0;
    CPP_OPTION(reader, extended_numbers) = (lang == STD_C17) ? 1 : 0;
}

cpp_reader *cpp_create_reader(enum c_lang lang, symtab *tab)
{
    cpp_reader *reader;

    init();

    reader = xmalloc(sizeof(*reader));
    memset(reader, '\0', sizeof(*reader));

    cpp_set_lang(reader, lang);
    CPP_OPTION(reader, precision) = CHAR_BIT * sizeof(long);
    CPP_OPTION(reader, char_precision) = CHAR_BIT;
    CPP_OPTION(reader, int_precision) = CHAR_BIT * sizeof(int);
    CPP_OPTION(reader, unsigned_char) = 0;

    /* Location is not forced by default. */
    reader->forced_location.file = NULL;

    /* Sets static tokens. */
    reader->end.type = CPP_END;
    reader->end.flags = 0;
    reader->avoid_paste.type = CPP_PADDING;
    reader->avoid_paste.value.source = NULL;

    /* Prepares memory for tokens. */
    _cpp_init_tokenrow(&(reader->base_tokenrow));
    reader->cur_tokenrow = &(reader->base_tokenrow);
    reader->cur_token = reader->cur_tokenrow->start;

    /* Fake empty directory for files without search path. */
    reader->no_search_path.name = "";
    reader->no_search_path.next = NULL;

    reader->quote_include = NULL;
    reader->angled_include = NULL;

    reader->quote_ignores_source_dir = 0;

    /* Stack for expressions. */
    reader->op_stack = NULL;
    reader->op_end = NULL;
    _cpp_expand_op_stack(reader);

    _cpp_init_symtab(reader, tab);

    reader->date = NULL;
    reader->Ztime = NULL;

    return (reader);
}

void cpp_destroy_reader(cpp_reader *reader)
{
    _cpp_destroy_symtab (reader);
    _cpp_destroy_tokenrows (&(reader->base_tokenrow));
    free (reader->macro_memory);
    free (reader->op_stack);
    free (reader->date);
    free (reader->Ztime);
    free (reader);
}

struct builtin_macro {
    const char *name;
    const size_t len;
    const unsigned int value;
};

#define STR_AND_LEN(str) (str), sizeof(str) - 1
#define A(name, value) {STR_AND_LEN(name), value}

static const struct builtin_macro builtin[] = {
    A("__DATE__", BT_DATE),
    A("__FILE__", BT_FILE),
    A("__LINE__", BT_LINE),
    A("__TIME__", BT_TIME),
    A("_Pragma",  BT_PRAGMA)
};

void cpp_init_spec_builtin(cpp_reader *reader)
{
    const struct builtin_macro *bt;
    size_t len = sizeof(builtin) / sizeof(*builtin);

    for (bt = builtin; bt < builtin + len; bt++)
    {
        cpp_unknown *unknown = cpp_find(reader, bt->name, bt->len);
        unknown->type = UNKNOWN_BUILTIN_MACRO;
        unknown->value.builtin = bt->value;
    }
}

void cpp_init_builtins(cpp_reader *reader, int hosted)
{
    cpp_init_spec_builtin(reader);
    
    _cpp_define_builtin(reader, "__STDC__ 1");

    if (CPP_OPTION(reader, lang) == STD_C17)
    {
        _cpp_define_builtin(reader, "__STDC_VERSION__ 201710L");
    }

    if (hosted) _cpp_define_builtin(reader, "__STDC_HOSTED__ 1");
    else _cpp_define_builtin(reader, "__STDC_HOSTED__ 0");
}

const char *cpp_read_main_file(cpp_reader *reader, const char *name)
{
    const char *ret = NULL;
    location_t loc;

    loc.file = NULL;
    
    reader->main_file = _cpp_find_file(reader, name, &(reader->no_search_path),
                                       0, 0, 0, loc);
    if (_cpp_file_not_found(reader->main_file)) goto end;
    if (_cpp_add_file(reader, reader->main_file, 0, loc)) goto end;
    
    ret = name;
end:
    return (ret);
}
