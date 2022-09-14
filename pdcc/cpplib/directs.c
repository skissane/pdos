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
#include "support.h"
#include "xmalloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Stack of conditionals currently in progress. */
struct if_stack {
    struct if_stack *next;
    int skip_elses; /* Should future #else and #elif be skippec? */
    int was_skipping; /* skipping == 1 on entry? */
    int type; /* Directive type. */
};  

typedef void (*directive_handler)(cpp_reader *);
typedef struct directive {
    directive_handler handler;
    const char *name;
    size_t len;
    unsigned int flags;
} directive;

/* Directive flags. */
#define CONDITIONAL 0x1 /* Conditional. */
#define IF_CONDITIONAL 0x2 /* Opening conditional. */
#define INCLUDE 0x4 /* Header names are read. */
#define EXPAND 0x8 /* Tokens are always macro-expanded. */

#define DIRECTIVE_TABLE \
D(if, D_IF, CONDITIONAL | IF_CONDITIONAL | EXPAND) \
D(ifdef, D_IFDEF, CONDITIONAL | IF_CONDITIONAL) \
D(ifndef, D_IFNDEF, CONDITIONAL | IF_CONDITIONAL) \
D(elif, D_ELIF, CONDITIONAL | EXPAND) \
D(else, D_ELSE, CONDITIONAL) \
D(endif, D_ENDIF, CONDITIONAL) \
D(include, D_INCLUDE, INCLUDE | EXPAND) \
D(define, D_DEFINE, 0) \
D(undef, D_UNDEF, 0) \
D(line, D_LINE, EXPAND) \
D(error, D_ERROR, 0) \
D(pragma, D_PRAGMA, 0)

/* Uses the directive table to generate prototypes, enum and structures. */
#define D(name, ename, flags) static void do_##name(cpp_reader *);
DIRECTIVE_TABLE
#undef D
static void do_linemarker(cpp_reader *);

enum {
#define D(name, ename, flags) ename,
    DIRECTIVE_TABLE
#undef D
    DIRECTIVE_COUNT
};

#define D(name, ename, flags) {do_##name, #name, \
                               sizeof (#name) - 1, flags},
static const directive directives[] = {
    DIRECTIVE_TABLE
};
#undef D

static const directive dir_linemarker = {
    do_linemarker, "#", 1, 0
};

/* Prototypes. */
static void start_directive(cpp_reader *reader);
static void end_directive(cpp_reader *reader, int skip_line);
static void skip_rest_of_line(cpp_reader *reader);

/* Checks for validity of macro name for #define, #undef,
 * #ifdef a #ifndef. is_define_or_undef is 1,
 * if this call is from processing #define or #undef. */
static cpp_unknown *read_macro_name(cpp_reader *reader,
                                    int is_define_or_undef)
{
    cpp_token *token = _cpp_lex_token(reader);

    if (token->type == CPP_IDENT)
    {
        cpp_unknown *unknown = token->value.unknown;

        if (is_define_or_undef
            && (unknown == reader->spec_unknowns.n_defined))
        {
            cpp_error(reader, CPP_DL_ERROR,
                      "\"defined\" cannot be used as a macro name");
        }
        else return (unknown);
    }
    else if (token->type == CPP_END)
    {
        cpp_error(reader, CPP_DL_ERROR,
                  "no macro name given in #%s directive",
                  reader->directive->name);
    }
    else
    {
        cpp_error(reader, CPP_DL_ERROR,
                  "macro names must be identifiers");
    }

    return (NULL);
}

/* Helper function for check_end_of_line(). */
static void check_end_of_line_2(cpp_reader *reader, int expand)
{
    if ((reader->cur_token[-1].type != CPP_END)
        && ((expand
             ? cpp_get_token(reader)
             : _cpp_lex_token(reader))
            ->type != CPP_END))
    {
        cpp_error(reader, CPP_DL_WARNING,
                  "extra tokens at the end of #%s directive",
                  reader->directive->name);
    }
}

static void check_end_of_line_else_endif(cpp_reader *reader)
{
    check_end_of_line_2(reader, 0);
}

/* Makes sure there are no stray tokens at the end of line.
 * if expand is 1, tokens expanding to nothing are fine. */
static void check_end_of_line(cpp_reader *reader, int expand)
{
    check_end_of_line_2(reader, expand);
}

/* Gets token, but skips all CPP_PADDING. */
static const cpp_token *get_token_without_padding(cpp_reader *reader)
{
    for (;;)
    {
        const cpp_token *result = cpp_get_token(reader);
        if (result->type != CPP_PADDING) return (result);
    }
}

static char *glue_header_name(cpp_reader *reader)
{
    const cpp_token *token;
    size_t total = 0, capacity = 2048;
    char *memory = xmalloc(capacity);
    for (;;)
    {
        size_t len;
        
        token = get_token_without_padding(reader);
        if (token->type == CPP_GREATER) break;
        if (token->type == CPP_END)
        {
            cpp_error(reader, CPP_DL_ERROR,
                      "missing terminating > character");
            break;
        }

        /* +2 for space infront and '\0' behind. */
        len = cpp_token_len(token) + 2;
        if (total + len > capacity)
        {
            capacity = (capacity + len) * 2;
            memory = xrealloc(memory, capacity);
        }

        if (token->flags & AFTER_WHITE) memory[total++] = ' ';

        total = (cpp_spell_token(reader, token, &(memory[total]), 1) - memory);
    }

    memory[total] = '\0';

    return (memory);
}

static char *parse_include(cpp_reader *reader,
                           int *pangled,
                           location_t *loc)
{
    char *name;
    const cpp_token *token = get_token_without_padding(reader);
    *loc = token->src_loc;
    if ((token->type == CPP_STRING)
        || (token->type == CPP_HEADER_NAME))
    {
        name = xmalloc(token->value.string.len - 1);
        strncpy(name, token->value.string.text + 1,
                token->value.string.len - 2);
        *pangled = (token->type == CPP_HEADER_NAME);
    }
    else if (token->type == CPP_LESS)
    {
        name = glue_header_name(reader);
        *pangled = 1;
    }
    else
    {
        cpp_error(reader, CPP_DL_ERROR,
                  "#%s expects \"FILENAME\" alebo <FILENAME>",
                  reader->directive->name);
        return (NULL);
    }

    if (reader->directive == &(directives[D_PRAGMA]))
        ;
    else check_end_of_line(reader, 1);
    
    return (name);
}

void run_directive(cpp_reader *reader, int number_dir,
                   char *p, size_t len)
{
    _cpp_add_mffc(reader, p, len, 1 /* from_stage3 */);
    start_directive(reader);

    _cpp_clean_line(reader);

    reader->directive = &(directives[number_dir]);
    reader->directive->handler(reader);

    end_directive(reader, 1);

    _cpp_remove_mffc(reader);
}

/* Processes string str, as if it was a body of #define.
 * If str is only identifier, dit gets value 1.
 * Otherwise, the format should be identifier=definition. */
void cpp_define(cpp_reader *reader, const char *str)
{
    char *p;
    char *q;
    size_t len;
    
    len = strlen(str);
    p = xmalloc(len + 3);

    memcpy(p, str, len);
    q = strchr(str, '=');
    if (q) p[q - str] = ' ';
    else
    {
        p[len++] = ' ';
        p[len++] = '1';
    }
    p[len] = '\n';

    run_directive(reader, D_DEFINE, p, len);
}

void _cpp_define_builtin(cpp_reader *reader, const char *str)
{
    char *p;
    size_t len = strlen(str);
    p = xmalloc(len + 1);

    memcpy(p, str, len);
    p[len] = '\n';

    run_directive(reader, D_DEFINE, p, len);
}

static void add_conditional(cpp_reader *reader, int skip,
                             int type, const cpp_unknown *unknown)
{
    struct if_stack *ifs;
    cpp_mffc *mffc = reader->mffc;

    ifs = xmalloc(sizeof(*ifs));

    ifs->next = mffc->if_stack;
    ifs->skip_elses = reader->state.skipping || !skip;
    ifs->was_skipping = reader->state.skipping;
    ifs->type = type;

    reader->state.skipping = skip;
    mffc->if_stack = ifs;
}

static int str_to_linenum(const char *text,
                          size_t len,
                          unsigned long *line,
                          int *overflow)
{
    unsigned long l = 0, l_prev = 0;
    unsigned char c;

    *overflow = 0;
    while (len--)
    {
        c = *(text++);
        if (!ISDIGIT(c)) return (1);

        l *= 10;
        l += c - '0';
        if (l < l_prev) *overflow = 1;
        l_prev = l;
    }

    *line = l;

    return (0);
}    

/* Subfunction of do_linemarker(). Reads possible flags after file name.
 * prev is previous flag seen, 0, if this is the first flag.
 * Returns the flag, if it is valid, 0 at the end of directive. */
static unsigned int read_flag(cpp_reader *reader, unsigned int prev)
{
    const cpp_token *token = _cpp_lex_token(reader);

    if ((token->type == CPP_NUMBER) && (token->value.string.len == 1))
    {
        unsigned int flag = token->value.string.text[0] - '0';

        if ((flag > prev) && (flag <= 4)
            && ((flag != 4) || (prev == 3))
            && ((flag != 2) || (prev == 0)))
        {
            return (flag);
        }
    }

    if (token->type != CPP_END)
    {
        cpp_error(reader, CPP_DL_ERROR,
                  "invalid flag \"%s\" in line directive",
                  cpp_token_as_text(token));
    }
    
    return (0);
}

static void do_if(cpp_reader *reader)
{
    int skip = 1;

    if (!(reader->state.skipping))
    {
        skip = _cpp_process_expr(reader, 1) == 0;
    }

    add_conditional(reader, skip, D_IF, 0);
}

static void do_ifdef(cpp_reader *reader)
{
    int skip = 1;

    if (!(reader->state.skipping))
    {
        cpp_unknown *unknown = read_macro_name(reader, 0);

        if (unknown)
        {
            skip = !cpp_macro_s(unknown);
            check_end_of_line(reader, 0);
        }
    }

    add_conditional(reader, skip, D_IFDEF, 0);
}

static void do_ifndef(cpp_reader *reader)
{
    int skip = 1;
    cpp_unknown *unknown = 0;

    if (!(reader->state.skipping))
    {
        unknown = read_macro_name(reader, 0);

        if (unknown)
        {
            skip = cpp_macro_s(unknown);
            check_end_of_line(reader, 0);
        }
    }

    add_conditional(reader, skip, D_IFNDEF, unknown);
}

static void do_elif(cpp_reader *reader)
{
    struct if_stack *ifs = reader->mffc->if_stack;

    if (ifs == NULL)
    {
        cpp_error(reader, CPP_DL_ERROR,
                  "#elif without #if");
    }
    else
    {
        if (ifs->type == D_ELSE)
        {
            cpp_error(reader, CPP_DL_ERROR,
                      "#elif after #else");
        }
        ifs->type = D_ELIF;

        if (ifs->skip_elses)
            reader->state.skipping = 1;
        else
        {
            reader->state.skipping = _cpp_process_expr(reader, 0) == 0;
            ifs->skip_elses = !(reader->state.skipping);
        }
    }
}

static void do_else(cpp_reader *reader)
{
    struct if_stack *ifs = reader->mffc->if_stack;

    if (ifs == NULL)
    {
        cpp_error(reader, CPP_DL_ERROR,
                  "#else without #if");
    }
    else
    {
        if (ifs->type == D_ELSE)
        {
            cpp_error(reader, CPP_DL_ERROR,
                      "#else after #else");
        }
        ifs->type = D_ELSE;

        /* Skips all future (erroneous) #else and #elif. */
        reader->state.skipping = ifs->skip_elses;
        ifs->skip_elses = 1;

        if (!(ifs->was_skipping))
            check_end_of_line_else_endif(reader);
    }
}

static void do_endif(cpp_reader *reader)
{
    cpp_mffc *mffc = reader->mffc;
    struct if_stack *ifs = mffc->if_stack;

    if (ifs == NULL)
    {
        cpp_error(reader, CPP_DL_ERROR,
                  "#endif without #if");
    }
    else
    {
        if (!(ifs->was_skipping))
            check_end_of_line_else_endif(reader);

        mffc->if_stack = ifs->next;
        reader->state.skipping = ifs->was_skipping;
        free(ifs);
    }
}

static void do_include(cpp_reader *reader)
{
    int angled; /* <name> */
    location_t loc;
    char *filename = parse_include(reader, &angled, &loc);
    if (filename == NULL) return;

    if (*filename == '\0')
    {
        cpp_error_with_line(reader, CPP_DL_ERROR,
                            loc, 0,
                            "empty file name in #%s",
                            reader->directive->name);
        free(filename);
        return;
    }

    /* Removes accumulated unknown2. */
    skip_rest_of_line(reader);
    _cpp_add_include(reader, filename, angled, INCLUDE_TYPE_INCLUDE, loc);
    free(filename);
}

static void do_define(cpp_reader *reader)
{
    cpp_unknown *unknown = read_macro_name(reader, 1);
    if (unknown)
    {
        _cpp_define_macro(reader, unknown);
    }
}

static void do_undef(cpp_reader *reader)
{
    cpp_unknown *unknown = read_macro_name(reader, 1);

    if (unknown)
    {
        if (cpp_macro_s(unknown))
        {
            _cpp_undefine_macro(unknown);
        }
    }

    check_end_of_line(reader, 0);
}

static void do_line(cpp_reader *reader)
{
    const cpp_token *token;
    unsigned long line;
    unsigned long maximum = CPP_OPTION(reader, c99) ? 2147483647 : 32768;
    int overflow;
    const char *new_file = reader->mffc->filename;

    token = cpp_get_token(reader);
    if ((token->type != CPP_NUMBER)
        || str_to_linenum(token->value.string.text,
                          token->value.string.len,
                          &line, &overflow))
    {
        if (token->type == CPP_END)
        {
            cpp_error(reader, CPP_DL_ERROR,
                      "unexpected end of line after #line");
        }
        else
        {
            cpp_error(reader, CPP_DL_ERROR,
                      "\"%s\" after #line is not a positive integer",
                      cpp_token_as_text(token));
        }
        return;
    }

    if ((line == 0) || (line > maximum) || (overflow))
    {
        cpp_error(reader, CPP_DL_WARNING,
                  "line number out of range");
    }

    token = cpp_get_token(reader);
    if (token->type == CPP_STRING)
    {
        cpp_string s = {0, NULL};
        if (cpp_interpret_string(reader, &(token->value.string),
                                 &s, CPP_STRING) == 0)
        {
            new_file = (const char *)(s.text);
        }
                                 
        check_end_of_line(reader, /* expand */ 1);
    }
    else if (token->type != CPP_END)
    {
        cpp_error(reader, CPP_DL_ERROR,
                  "\"%s\" is not a valid filename",
                  cpp_token_as_text(token));
        return;
    }

    skip_rest_of_line(reader);

    _cpp_change_file(reader, CPP_REASON_RENAME, new_file, line);
}

static void do_error(cpp_reader *reader)
{
    char *line;
    
    reader->state.prevent_expansion++;
    line = cpp_output_line_to_string(reader, reader->directive->name);
    reader->state.prevent_expansion--;

    cpp_error(reader, CPP_DL_ERROR,
              "%s", line);
    free(line);
}

static void do_pragma(cpp_reader *reader)
{
    printf("+++IMPLEMENT do_pragma %s:%u\n", __FILE__, __LINE__);
    return;
}

static void do_linemarker(cpp_reader *reader)
{
    const cpp_token *token;
    unsigned long line;
    int overflow;
    const char *new_file = reader->mffc->filename;
    enum cpp_reasons reason = CPP_REASON_RENAME;
    unsigned int flag;

    /* Returns token, so the number can be read again. */
    _cpp_return_tokens(reader, 1);

    token = cpp_get_token(reader);
    if ((token->type != CPP_NUMBER)
        || str_to_linenum(token->value.string.text,
                          token->value.string.len,
                          &line, &overflow))
    {
        /* It is impossible to get CPP_END here,
         * so cpp_token_as_text() is safe. */
        cpp_error(reader, CPP_DL_ERROR,
                  "\"%s\" after # is not a positive integer",
                  cpp_token_as_text(token));
        return;
    }

    token = cpp_get_token(reader);
    if (token->type == CPP_STRING)
    {
        cpp_string s = {0, NULL};
        if (cpp_interpret_string(reader, &(token->value.string),
                                 &s, CPP_STRING) == 0)
        {
            new_file = (const char *)(s.text);
        }

        flag = read_flag(reader, 0);
        if (flag == 1)
        {
            reason = CPP_REASON_ENTER;
            flag = read_flag(reader, flag);
        }
        else if (flag == 2)
        {
            reason = CPP_REASON_LEAVE;
            flag = read_flag(reader, flag);
        }

        if (flag == 3)
        {
            /* +++FINISH */;
            flag = read_flag(reader, flag);
            if (flag == 4)
            {
                ;
            }
        }
                                 
        check_end_of_line(reader, /* expand */ 1);
    }
    else if (token->type != CPP_END)
    {
        cpp_error(reader, CPP_DL_ERROR,
                  "\"%s\" is not a valid filename",
                  cpp_token_as_text(token));
        return;
    }

    skip_rest_of_line(reader);

    _cpp_change_file(reader, reason, new_file, line);
}

static void start_directive(cpp_reader *reader)
{
    reader->state.in_directive = 1;
    reader->directive_result.type = CPP_PADDING;

    if (reader->forced_location.file)
    {
        reader->directive_line = reader->forced_location;
    }
    else
    {
        reader->directive_line.file = reader->mffc->filename;
        reader->directive_line.line = reader->mffc->line;
        reader->directive_line.column = 1;
    }
}

/* Skips remaining tokens in a directive. */
static void skip_rest_of_line(cpp_reader *reader)
{
    /* Discards accumulated unknown2. */
    while (reader->unknown2 != NULL)
    {
        _cpp_remove_unknown2(reader);
    }

    if (reader->cur_token[-1].type != CPP_END)
    {
        while (_cpp_lex_token(reader)->type != CPP_END) ;
    }
}

static void end_directive(cpp_reader *reader, int skip_line)
{
    if (skip_line)
    {
        skip_rest_of_line(reader);
        if (reader->keep_tokens == 0)
        {
            reader->cur_tokenrow = &(reader->base_tokenrow);
            reader->cur_token = reader->cur_tokenrow->start;
        }
    }

    /* Restores state. */
    reader->state.in_directive = 0;
    reader->state.collecting_header_names = 0;
    reader->directive = 0;
}

/* Returns 0, if directive was handled completely,
 * 1, if the line should be further processed. */
int _cpp_handle_directive(cpp_reader *reader)
{
    const directive *dir = 0;
    const cpp_token *dname;
    int collected_arguments = reader->state.collecting_args;
    int skip = 1;

    if (collected_arguments)
    {
        cpp_error(reader, CPP_DL_WARNING,
                  "embedding a directive in macro arguments"
                  " causes undefined behaviour");
        reader->state.collecting_args = 0;
        reader->state.prevent_expansion = 0;
    }

    start_directive(reader);
    dname = _cpp_lex_token(reader);
    if (dname->type == CPP_IDENT)
    {
        if (dname->value.unknown->is_directive)
            dir = &(directives[dname->value.unknown->directive_index]);
    }
    else if (dname->type == CPP_NUMBER)
    {
        dir = &dir_linemarker;
    }

    if (dir)
    {
        /* In skipped conditionals are all non-conditional directives ignored,
         * but header names need to be correctly read anyways. */
        reader->state.collecting_header_names = dir->flags & INCLUDE;
        reader->state.directive_wants_padding = dir->flags & INCLUDE;
        if (reader->state.skipping && !(dir->flags & CONDITIONAL))
        {
            dir = 0;
        }
    }
    else if (dname->type == CPP_END) ; /* Null directive. */
    else
    {
        if (reader->state.skipping == 0)
        {
            cpp_error(reader, CPP_DL_ERROR,
                      "invalid preprocessor directive #%s",
                      cpp_token_as_text(dname));
        }
    }

    reader->directive = dir;
    if (dir) reader->directive->handler(reader);

    end_directive(reader, skip);
    if (collected_arguments)
    {
        /* Restores state, if macro arguments were collected. */
        reader->state.collecting_args = 2;
        reader->state.prevent_expansion = 1;
    }

    return (!skip);
}

/* Puts all directives into cpp_unknown table. */
void _cpp_init_directives(cpp_reader *reader)
{
    unsigned int i;

    for (i = 0; i < DIRECTIVE_COUNT; i++)
    {
        cpp_unknown *unknown = cpp_find(reader, directives[i].name,
                                        directives[i].len);

        unknown->is_directive = 1;
        unknown->directive_index = i;
    }
}

cpp_mffc *_cpp_add_mffc(cpp_reader *reader,
                        char *start,
                        size_t size,
                        int from_stage3)
{
    cpp_mffc *mffc = xmalloc(sizeof(*mffc));

    memset(mffc, '\0', sizeof(*mffc));

    mffc->next_line = mffc->start = start;
    mffc->end = start + size;
    mffc->from_stage3 = from_stage3;
    mffc->need_line = 1;
    mffc->prev = reader->mffc;

    mffc->file = NULL;

    reader->mffc = mffc;

    return (mffc);
}

void _cpp_remove_mffc(cpp_reader *reader)
{
    cpp_mffc *old = reader->mffc;
    struct if_stack *ifs, *next_ifs;

    for (ifs = old->if_stack;
         ifs;
         ifs = next_ifs)
    {
        cpp_error(reader, CPP_DL_ERROR,
                  "unterminated #%s",
                  directives[ifs->type].name);
        next_ifs = ifs->next;
        free(ifs);
    }
    
    /* In case #endif is missing. */
    reader->state.skipping = 0;

    reader->mffc = old->prev;

    if (reader->mffc)
    {
        if (old->file)
        {
            _cpp_change_file(reader,
                             CPP_REASON_LEAVE,
                             reader->mffc->filename,
                             reader->mffc->line);
        }
    }

    free(old->notes);
    free((void *)(old->start));
    free(old);
}

void _cpp_change_file(cpp_reader *reader, enum cpp_reasons reason,
                      const char *file, unsigned long line)
{
    location_t loc;
    
    loc.line = line;
    loc.file = file;

    reader->mffc->line = line;
    reader->mffc->filename = file;
            
    if (reader->callbacks.file_change)
    {
        reader->callbacks.file_change(reader, loc, reason);
    }
}
