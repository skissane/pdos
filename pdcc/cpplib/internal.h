/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

#include "symtab.h"

#include <stdlib.h>

#define TOKENROW_LEN 256

#define VALID_SIGN(c, prev_c) \
 (((c == '+') || (c == '-')) \
  && ((prev_c == 'e') || (prev_c == 'E') \
      || (((prev_c == 'p') || (prev_c == 'P')) \
          && CPP_OPTION(reader, extended_numbers))))

#define CPP_OPTION(reader, option) ((reader)->options.option)

#define CPP_INCREMENT_LINE(reader, ignored) ((reader)->mffc->line++)

union utoken {
    const cpp_token *token;
    const cpp_token **ptoken;
};

enum unknown2_tokens_kind
{
    TOKENS_KIND_DIRECT, /* * */
    TOKENS_KIND_INDIRECT /* ** */
};

typedef struct cpp_unknown2 {
    struct cpp_unknown2 *next;
    
    union utoken first;
    union utoken last;

    cpp_unknown *macro;
    void *memory;

    enum unknown2_tokens_kind tokens_kind;
} cpp_unknown2;

typedef struct {
    const char *pos;

    /* Type of note. '\\' means an escaped newline,
     * ' ' means an escaped newline with space before '\\',
     * 9 last trigraph characters mean trigraphs,
     * 0 means already processed note
     * and everything else is invalid. */
    unsigned int type;
} _cpp_line_note;

/* Memory for file content. */
typedef struct cpp_mffc {
    const char *pos;
    const char *line_base;
    const char *next_line;

    const char *start; /* Pointer for free(). */
    const char *end;

    _cpp_line_note *notes; /* Array of notes. */
    unsigned int cur_note;
    unsigned int notes_used;
    unsigned int notes_cap;

    struct cpp_mffc *prev;

    struct _cpp_file *file;
    /* Position tracking. */
    const char *filename;
    int line;

    struct if_stack *if_stack;

    /* 1, if the next clean line is needed. */
    int need_line;

    /* 1, if trigraphs and escaped newlines are already processed. */
    int from_stage3;
} cpp_mffc;

typedef struct tokenrow {
    struct tokenrow *prev, *next;
    cpp_token *start, *end;
} tokenrow;

struct state {
    int in_directive;
    int prevent_expansion;
    int collecting_args;
    int collecting_header_names;
    int skipping;
    int directive_wants_padding;

    int va_args_fine;

    /* Nonzero to skip evaluating part of an expression. */
    int skip_eval;
};

struct spec_unknowns {
    cpp_unknown *n_defined; /* "defined" operator. */
    cpp_unknown *n___VA_ARGS__; /* __VA_ARGS__ */
};

struct directive;

struct cpp_reader {
    cpp_mffc *mffc;

    tokenrow base_tokenrow;
    tokenrow *cur_tokenrow;
    cpp_token *cur_token;

    unsigned int tokens_infront;

    location_t forced_location;

    const struct directive *directive;
    cpp_token directive_result;
    location_t directive_line;

    cpp_token end; /* CPP_END token. */
    cpp_token avoid_paste; /* CPP_PADDING token. */

    cpp_unknown2 *unknown2;

    /* Nonzero, if tokens should not be discarded at the end of line. */
    int keep_tokens;

    struct state state;

    struct spec_unknowns spec_unknowns;

    char *macro_memory;
    unsigned int macro_memory_size;

    symtab *tab;
    int own_table;

    struct cpp_callbacks callbacks;

    cpp_dir *quote_include; /* "" */
    cpp_dir *angled_include; /* <> */
    cpp_dir no_search_path;

    /* #include "" ignores directory, in what is the file with #include. */
    int quote_ignores_source_dir;

    struct _cpp_file *main_file;

    /* Expression stack. */
    struct op *op_stack, *op_end;

    cpp_options options;

    int about_to_expand_macro;
    location_t invocation_location;

    char *date;
    char *time;

    void *help;
    void *help2;
};

enum include_type {
    INCLUDE_TYPE_INCLUDE,
    INCLUDE_TYPE_COMMAND_LINE,
    INCLUDE_TYPE_DEFAULT
};

extern unsigned char _cpp_trigraph_map[0x100];

/* lex.c */

#define _cpp_lex_token_direct _cppltd
#define _cpp_lex_token _cppltok
#define _cpp_remaining_tokens_in_unknown2 _cpprtiu
#define _cpp_init_tokenrow _cppitr

void _cpp_init_tokenrow(tokenrow *row);
void _cpp_clean_line(cpp_reader *reader);
cpp_token *_cpp_lex_token_direct(cpp_reader *reader);
cpp_token *_cpp_lex_token(cpp_reader *reader);
cpp_token *_cpp_temp_token(cpp_reader *reader);
unsigned int _cpp_remaining_tokens_in_unknown2(cpp_unknown2 *unknown2);
const cpp_token *_cpp_token_from_unknown2_at(cpp_unknown2 *unknown2,
                                             unsigned int index);

/* core.c */

#define _cpp_return_tokens _cpprtok
#define _cpp_return_tokens_to_lexer _cpprttl
#define _cpp_remove_unknown2 _cppru2

int _cpp_equiv_tokens(const cpp_token *t1, const cpp_token *t2);
void _cpp_remove_unknown2(cpp_reader *);
void _cpp_return_tokens_to_lexer(cpp_reader *reader, unsigned int count);
void _cpp_return_tokens(cpp_reader *reader, unsigned int count);

/* macro.c */

#define _cpp_define_macro _cppdm

int _cpp_define_macro(cpp_reader *reader, cpp_unknown *unknown);
void _cpp_undefine_macro(cpp_unknown *unknown);

/* directs.c */

#define _cpp_add_mffc _cppamffc
#define _cpp_init_directives _cppidir

void _cpp_init_directives(cpp_reader *reader);
int _cpp_handle_directive(cpp_reader *reader);
void _cpp_define_builtin(cpp_reader *reader, const char *);
cpp_mffc *_cpp_add_mffc(cpp_reader *, char *, size_t,
                        int from_stage3);
void _cpp_remove_mffc(cpp_reader *);
void _cpp_change_file(cpp_reader *, enum cpp_reasons,
                      const char *, unsigned long);

/* files.c */

#define _cpp_add_file _cppaf
#define _cpp_add_include _cppai

typedef struct _cpp_file _cpp_file;
_cpp_file *_cpp_find_file(cpp_reader *reader,
                          const char *name,
                          cpp_dir *start_dir,
                          int ignored,
                          int angled,
                          int ignored2,
                          location_t loc);
void _cpp_free_file(_cpp_file *file);
int _cpp_file_not_found(_cpp_file *file);
int _cpp_add_file(cpp_reader *reader,
                  _cpp_file *file,
                  int ignored,
                  location_t loc);
int _cpp_add_include(cpp_reader *reader,
                     const char *name,
                     int angled_name,
                     enum include_type type,
                     location_t loc);

/* expr.c */

#define _cpp_process_expr _cpppe

int _cpp_process_expr(cpp_reader *reader, int is_if);
struct op *_cpp_expand_op_stack(cpp_reader *reader);

/* idents.c */
void _cpp_init_symtab(cpp_reader *reader, symtab *tab);
void _cpp_destroy_symtab(cpp_reader *reader);
