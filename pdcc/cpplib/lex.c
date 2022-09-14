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

void _cpp_init_tokenrow(tokenrow *row)
{
    row->start = xmalloc(sizeof(cpp_token) * TOKENROW_LEN);
    row->end = row->start + TOKENROW_LEN;
    row->prev = NULL;
    row->next = NULL;
}

static tokenrow *next_tokenrow(tokenrow *row)
{
    if (row->next == NULL)
    {
        row->next = xmalloc(sizeof(tokenrow));
        _cpp_init_tokenrow(row->next);
        row->next->prev = row;
    }

    return (row->next);
}

void _cpp_process_line_notes(cpp_reader *reader)
{
    cpp_mffc *mffc = reader->mffc;

    for (;;)
    {
        _cpp_line_note *note = &(mffc->notes[mffc->cur_note]);
        
        if (note->pos > mffc->pos) break;

        mffc->cur_note++;

        if (note->type == '\\' || (note->type == ' '))
        {
            if (note->type == ' ')
            {
                cpp_error(reader, CPP_DL_WARNING,
                          "backslash and new line separated by space");
            }

            if (mffc->next_line > (mffc->end))
            {
                cpp_error(reader, CPP_DL_WARNING,
                          "backslash-new line at the end of file");
                mffc->next_line = mffc->end;
            }

            mffc->line_base = note->pos;
            CPP_INCREMENT_LINE(reader, 0);
        }
        else if (_cpp_trigraph_map[note->type])
        {
            /* +++Add -Wtrigraphs */
        }
        else
        {
            printf("CPPLIB Internal error %s:%u\n", __FILE__, __LINE__);
            return;
        }
    }
}

/* Skips a C block comment.
 * End of the comment is found by finding '*' before '/'.
 * Returns 1, if the comment is terminated by end of file, otherwise 0.
 *
 * reader->mffc->pos points to * at the beginning of comment. */
int _cpp_skip_block_comment(cpp_reader *reader)
{
    cpp_mffc *mffc = reader->mffc;
    const char *pos = mffc->pos;
    char c;

    pos++;
    if (*pos == '/') pos++;

    for (;;)
    {
        c = *(pos++);

        if (c == '/')
        {
            if (pos[-2] == '*')
            {
                break;
            }
        }
        else if (c == '\n')
        {
            mffc->pos = pos - 1;
            _cpp_process_line_notes(reader);
            if (mffc->next_line >= mffc->end) return (1);
            _cpp_clean_line(reader);

            CPP_INCREMENT_LINE(reader, mffc->next_line - mffc->line_base);

            pos = mffc->pos;
        }
    }

    mffc->pos = pos;
    _cpp_process_line_notes(reader);
    return (0);
}

static void skip_line_comment(cpp_reader *reader)
{
    cpp_mffc *mffc = reader->mffc;

    while (*(mffc->pos) != '\n') mffc->pos++;
    
    _cpp_process_line_notes(reader);
}

static void skip_whitespace(cpp_reader *reader, unsigned char c)
{
    cpp_mffc *mffc = reader->mffc;
    int saw_NUL = 0;

    do {
        if (c == ' ' || c == '\t') ;
        else if (c == '\0') saw_NUL = 1;
        else if (reader->state.in_directive)
        {
            cpp_error(reader, CPP_DL_WARNING,
                      "%s in preprocessor directive",
                      c == '\f' ? "\\f" : "\\v");
        }
        c = *(mffc->pos++);
    } while (ISNVSPACE(c));

    if (saw_NUL)
        cpp_error(reader, CPP_DL_WARNING,
                  "null character(s) ignored");

    mffc->pos--;
}

static cpp_unknown *read_ident(cpp_reader *reader,
                               const char *start)
{
    cpp_unknown *unknown;
    size_t len;
    const char *pos = start + 1;

    while (ISIDNUM(*pos))
        pos++;

    len = pos - start;

    unknown = cpp_find(reader, start, len);

    if ((unknown == reader->spec_unknowns.n___VA_ARGS__)
        && (reader->state.va_args_fine == 0))
    {
        cpp_error(reader, CPP_DL_WARNING,
                  "__VA_ARGS__ can only appear"
                  " in expansion of a variadic macro");
    }

    return (unknown);
}

static void read_number(cpp_reader *reader, cpp_string *string)
{
    const char *start = reader->mffc->pos - 1;
    const char *cur;
    char *result;

    do {
        cur = reader->mffc->pos;

        while (ISIDNUM(*cur) || *cur == '.'
               || VALID_SIGN(*cur, cur[-1]))
        {
            cur++;
        }

        reader->mffc->pos = cur;
    } while (0 /*+++Finish UCNs*/);

    string->len = cur - start;
    result = xmalloc(string->len + 1);
    strncpy(result, start, string->len);
    string->text = result;
}

static void create_literal(cpp_reader *reader,
                           cpp_token *token,
                           const char *start, size_t len,
                           enum cpp_tokentype type)
{
    char *result = xmalloc(len + 1);
    strncpy(result, start, len);

    token->type = type;
    token->value.string.text = result;
    token->value.string.len = len;
}

static void read_string(cpp_reader *reader,
                        cpp_token *token,
                        const char *start)
{
    enum cpp_tokentype type;
    const char *pos = start;
    int saw_NUL = 0;
    char terminator;

    terminator = *(pos++);
    if (terminator == 'L' || terminator == 'U') terminator = *(pos++);
    else if (terminator == 'u')
    {
        terminator = *(pos++);
        if (terminator == 'u') terminator = *(pos++);
    }

    if (terminator == '"')
    {
        type = (*start == 'L' ? CPP_LSTRING :
               (*start == 'U' ? CPP_USTRING :
                (*start == 'u' ?
                 (start[1] == '8' ? CPP_u8STRING : CPP_uSTRING ) :
                 (CPP_STRING))));
    }
    else if (terminator == '\'')
    {
        type = (*start == 'L' ? CPP_LCHAR :
               (*start == 'U' ? CPP_UCHAR :
                (*start == 'u' ? CPP_uCHAR :
                 (CPP_CHAR))));
    }
    else
    {
        terminator = '>', type = CPP_HEADER_NAME;
    }

    for (;;)
    {
        char c = *(pos++);

        if ((c == '\\')
            /* In #include-like directives, terminators are not escapable. */
            && (reader->state.collecting_header_names == 0)
            && (*pos != '\n'))
        {
            pos++;
        }
        else if (c == terminator) break;
        else if (c == '\n')
        {
            pos--;
            if (terminator == '>')
            {
                token->type = CPP_LESS;
                return;
            }
            type = CPP_OTHER;
            break;
        }
        else if (c == '\0')
        {
            saw_NUL = 1;
        }
    }

    if (saw_NUL && (reader->state.skipping == 0))
    {
        cpp_error(reader, CPP_DL_WARNING,
                  "null character(s) preserved in literal");
    }

    if (type == CPP_OTHER)
    {
        cpp_error(reader, CPP_DL_WARNING,
                  "missing terminating %c character\n",
                  (int)terminator);
    }

    reader->mffc->pos = pos;
    create_literal(reader, token, start, pos - start, type);
}

static void add_line_note(cpp_mffc *mffc,
                          const char *pos,
                          unsigned int type)
{
    if (mffc->notes_used == mffc->notes_cap)
    {
        mffc->notes_cap = (mffc->notes_cap * 2 + 256);
        mffc->notes = xrealloc(mffc->notes,
                               sizeof (*(mffc->notes)) * mffc->notes_cap);
    }

    mffc->notes[mffc->notes_used].pos = pos;
    mffc->notes[mffc->notes_used].type = type;
    mffc->notes_used++;
}

const char *find(const char *s)
{
    while (!((*s == '\n') || (*s == '\r')
             || (*s == '\\') || (*s == '?')))
    {
        s++;
    }

    return (s);
}

void _cpp_clean_line(cpp_reader *reader)
{
    cpp_mffc *mffc = reader->mffc;
    char *s = (char *)(mffc->pos = mffc->line_base = mffc->next_line);
    char c, *d, *p;

    mffc->cur_note = mffc->notes_used = 0;
    mffc->need_line = 0;
    if (mffc->from_stage3 == 0)
    {
        const char *backslash = NULL;
        while (1)
        {
            /* Optimized search for '\n', '\r', '\\', '?'. */
            s = (char *)find(s);
            if (*s == '\\')
            {
                backslash = s++;
            }
            else if (*s == '?')
            {
                if ((s[1] == '?') && _cpp_trigraph_map[(int)s[2]])
                {
                    /* Adds line note for -Wtrigraphs. */
                    add_line_note(mffc, s, s[2]);
                    if (CPP_OPTION(reader, trigraphs))
                    {
                        d = s;
                        *d = _cpp_trigraph_map[(int)s[2]];
                        s += 2;
                        goto slow_path;
                    }
                }
                /* Not a trigraph. */
                s++;
            }
            else break;
        }

        /* *s is '\r' or '\n'. */
        d = s;

        if (s == mffc->end) goto end;

        if ((s[0] == '\r') && (s[1] == '\n'))
        {
            s++;
            if (s == mffc->end) goto end;
        }

        if (backslash == NULL) goto end;

        /* Checks for escaped new line. */
        p = d;
        while (ISNVSPACE(p[-1])) p--;
        if (p - 1 != backslash) goto end;

        add_line_note(mffc, p - 1, p == d ? '\\' : ' ');
        d = p - 2;
        mffc->next_line = p - 1;

    slow_path:
        while (1)
        {
            c = *(++s);
            *(++d) = c;

            if (c == '\r' || c == '\n')
            {
                if ((c == '\r') && (s != mffc->end) && (s[1] == '\n'))
                    s++;
                
                if (s == mffc->end) break;

                /* Escaped new line? */
                p = d;
                while ((p != mffc->next_line) && ISNVSPACE(p[-1])) p--;
                if (p == mffc->next_line || p[-1] != '\\') break;

                add_line_note(mffc, p - 1, p == d ? '\\' : ' ');
                d = p - 2;
                mffc->next_line = p - 1;
            }
            else if (c == '?' && (s[1] == '?') && _cpp_trigraph_map[(int)s[2]])
            {
                /* Adds line note for -Wtrigraphs. */
                add_line_note(mffc, s, s[2]);
                if (CPP_OPTION(reader, trigraphs))
                {
                    *d = _cpp_trigraph_map[(int)s[2]];
                    s += 2;
                }
            }
        }
    }
    else
    {
        while (*s != '\n' && *s != '\r') s++;

        d = s;
        if ((s[0] == '\r') && (s != mffc->end) && (s[1] == '\n'))
            s++;
    }

end:
    *d = '\n';
    /* Guard note, that should not be processed. */
    add_line_note(mffc, d + 1, '\n');
    mffc->next_line = s + 1;    
}

/* Returns 1, if new line was loaded. */
int _cpp_get_new_line(cpp_reader *reader)
{
    /* New line cannot start while in directive. */
    if(reader->state.in_directive)
        return (0);

    for (;;)
    {
        cpp_mffc *mffc = reader->mffc;

        if (mffc->need_line == 0)
            return (1);
        
        if (mffc->next_line < mffc->end)
        {
            _cpp_clean_line(reader);
            return (1);
        }

        /* End of mffc. */
        /* If collecting_args, new mffc will not be read. */
        if (reader->state.collecting_args)
        {
            return (0);
        }

        _cpp_remove_mffc(reader);
        if (reader->mffc == NULL)
        {
            return (0);
        }
    }
}

#define IF_NEXT_THEN(a1, a2, a3) \
do { \
    result->type = a3; \
    if (mffc->pos[0] == a1) \
    { \
        mffc->pos++; \
        result->type = a2; \
    } \
} while (0)

cpp_token *_cpp_lex_token_direct(cpp_reader *reader)
{
    cpp_token *result = reader->cur_token++;
    cpp_mffc *mffc;
    int c;

new_line:
    result->flags = 0;
    mffc = reader->mffc;
    if (mffc == NULL) return NULL;
    if (mffc->need_line)
    {
        if (_cpp_get_new_line(reader) == 0)
        {
            result->type = CPP_END;

            return (result);
        }
        if (reader->keep_tokens == 0)
        {
            reader->cur_tokenrow = &(reader->base_tokenrow);
            result = reader->cur_tokenrow->start;
            reader->cur_token = result + 1;
        }
        /* flags are in undefined state before this point. */
        result->flags = START_OF_LINE;
        if (reader->state.collecting_args == 2)
        {
            result->flags |= AFTER_WHITE;
        }
    }
    mffc = reader->mffc;
    
skipped_comment:
    /* Position tracking. */
    result->src_loc.line = mffc->line;
    
skipped_whitespace:
    if (mffc->pos >= mffc->notes[mffc->cur_note].pos)
    {
        _cpp_process_line_notes(reader);
        result->src_loc.line = reader->mffc->line;
    }
    c = *(mffc->pos++);
    if (reader->forced_location.file)
    {
        result->src_loc = reader->forced_location;
    }
    else
    {
        result->src_loc.column = mffc->pos - mffc->line_base;
        result->src_loc.file = mffc->filename;
    }

    switch (c)
    {
        case ' ': case '\t': case '\f': case '\v': case '\0':
            result->flags |= AFTER_WHITE;
            skip_whitespace(reader, c);
            goto skipped_whitespace;
        
        case '\n':
            if (mffc->pos < mffc->end)
                CPP_INCREMENT_LINE(reader, 0);
            mffc->need_line = 1;
            goto new_line;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            result->type = CPP_NUMBER;
            read_number(reader, &(result->value.string));
            break;

        case 'L':
        case 'u':
        case 'U':
            if ((mffc->pos[0] == '\'')
                || (mffc->pos[0] == '"')
                || ((c == 'u')
                    && (mffc->pos[0] == '8')
                    && (mffc->pos[1] == '"')))
            {
                read_string(reader, result, mffc->pos - 1);
                break;
            }
            /* FALLTHROUGH */

        case '_':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
        case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
        case 'o': case 'p': case 'q': case 'r': case 's': case 't':          
        case 'v': case 'w': case 'x': case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
        case 'H': case 'I': case 'J': case 'K':           case 'M': case 'N':
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T':          
        case 'V': case 'W': case 'X': case 'Y': case 'Z':
            result->type = CPP_IDENT;
            result->value.unknown = read_ident(reader, mffc->pos - 1);
            mffc->pos += result->value.unknown->cell.len - 1;
            break;

        case '\'':
        case '"':
            read_string(reader, result, mffc->pos - 1);
            break;

        case '/':
            if (mffc->pos[0] == '*')
            {
                if (_cpp_skip_block_comment(reader))
                {
                    cpp_error(reader, CPP_DL_ERROR,
                              "unterminated comment");
                }
            }
            else if (mffc->pos[0] == '/')
            {
                /* +++Add warnings for C89/90. */
                skip_line_comment(reader);
            }
            else if (mffc->pos[0] == '=')
            {
                mffc->pos++, result->type = CPP_DIV_EQ;
                break;
            }
            else
            {
                result->type = CPP_DIV;
                break;
            }

            if (1)
            {
                result->flags |= AFTER_WHITE;
                goto skipped_comment;
            }

            break;

        case '<':
            if (reader->state.collecting_header_names)
            {
                read_string(reader, result, mffc->pos - 1);
                if (result->type != CPP_LESS) break;
            }

            result->type = CPP_LESS;
            if (mffc->pos[0] == '=')
            {
                mffc->pos++, result->type = CPP_LESS_EQ;
            }
            else if (mffc->pos[0] == '<')
            {
                mffc->pos++;
                IF_NEXT_THEN('=', CPP_LSHIFT_EQ, CPP_LSHIFT);
            }
            else
            {
                if (mffc->pos[0] == ':')
                {
                    mffc->pos++;
                    result->flags |= DIGRAPH;
                    result->type = CPP_LSQUARE;
                }
                else if (mffc->pos[0] == '%')
                {
                    mffc->pos++;
                    result->flags |= DIGRAPH;
                    result->type = CPP_LBRACE;
                }
            }
            break;

        case '>':
            result->type = CPP_GREATER;
            if (mffc->pos[0] == '=')
            {
                mffc->pos++, result->type = CPP_GREATER_EQ;
            }
            else if (mffc->pos[0] == '>')
            {
                mffc->pos++;
                IF_NEXT_THEN('=', CPP_RSHIFT_EQ, CPP_RSHIFT);
            }
            break;

        case '%':
            result->type = CPP_MODULO;
            if (mffc->pos[0] == '=')
            {
                mffc->pos++, result->type = CPP_MODULO_EQ;
            }
            else
            {
                if (mffc->pos[0] == ':')
                {
                    mffc->pos++;
                    result->flags |= DIGRAPH;
                    result->type = CPP_HASH;
                    if (mffc->pos[0] == '%' && mffc->pos[1] == ':')
                    {
                        mffc->pos += 2, result->type = CPP_PASTE;
                    }
                }
                else if (mffc->pos[0] == '>')
                {
                    mffc->pos++;
                    result->flags |= DIGRAPH;
                    result->type = CPP_RBRACE;
                }
            }
            break;

        case '.':
            result->type = CPP_DOT;
            if (ISDIGIT(mffc->pos[1]))
            {
                result->type = CPP_NUMBER;
                read_number(reader, &(result->value.string));
            }
            else if (mffc->pos[0] == '.' && mffc->pos[1] == '.')
            {
                mffc->pos += 2, result->type = CPP_ELLIPSIS;
            }
            break;

        case '+':
            result->type = CPP_PLUS;
            if (mffc->pos[0] == '+')
            {
                mffc->pos++, result->type = CPP_PLUS_PLUS;
            }
            else if (mffc->pos[0] == '=')
            {
                mffc->pos++, result->type = CPP_PLUS_EQ;
            }
            break;

        case '-':
            result->type = CPP_MINUS;
            if (mffc->pos[0] == '>')
            {
                mffc->pos++, result->type = CPP_DEREF;
            }
            else if (mffc->pos[0] == '-')
            {
                mffc->pos++, result->type = CPP_MINUS_MINUS;
            }
            else if (mffc->pos[0] == '=')
            {
                mffc->pos++, result->type = CPP_MINUS_EQ;
            }
            break;

        case '&':
            result->type = CPP_AND;
            if (mffc->pos[0] == '&')
            {
                mffc->pos++, result->type = CPP_AND_AND;
            }
            else if (mffc->pos[0] == '=')
            {
                mffc->pos++, result->type = CPP_AND_EQ;
            }
            break;

        case '|':
            result->type = CPP_OR;
            if (mffc->pos[0] == '|')
            {
                mffc->pos++, result->type = CPP_OR_OR;
            }
            else if (mffc->pos[0] == '=')
            {
                mffc->pos++, result->type = CPP_OR_EQ;
            }
            break;

        case ':':
            result->type = CPP_COLON;
            if (mffc->pos[0] == '>')
            {
                mffc->pos++;
                result->flags |= DIGRAPH;
                result->type = CPP_RSQUARE;
            }
            break;

        case '*':
            IF_NEXT_THEN('=', CPP_STAR_EQ, CPP_STAR); break;
        case '=':
            IF_NEXT_THEN('=', CPP_EQ_EQ, CPP_EQ); break;
        case '!':
            IF_NEXT_THEN('=', CPP_NOT_EQ, CPP_NOT); break;
        case '^':
            IF_NEXT_THEN('=', CPP_XOR_EQ, CPP_XOR); break;
        case '#':
            IF_NEXT_THEN('#', CPP_PASTE, CPP_HASH); break;

        case '~': result->type = CPP_COMPLEMENT; break;
        case '?': result->type = CPP_QUERY; break;
        case ',': result->type = CPP_COMMA; break;
        case '(': result->type = CPP_LPAREN; break;
        case ')': result->type = CPP_RPAREN; break;
        case '[': result->type = CPP_LSQUARE; break;
        case ']': result->type = CPP_RSQUARE; break;
        case '{': result->type = CPP_LBRACE; break;
        case '}': result->type = CPP_RBRACE; break;
        case ';': result->type = CPP_SEMICOLON; break;

        case '@': result->type = CPP_ATSIGN; break;

        default:
            create_literal(reader, result, mffc->pos - 1, 1, CPP_OTHER);
            break;
    }

    return (result);
}

cpp_token *_cpp_lex_token(cpp_reader *reader)
{
    cpp_token *result;

    for (;;)
    {
        if (reader->cur_token == reader->cur_tokenrow->end)
        {
            reader->cur_tokenrow = next_tokenrow(reader->cur_tokenrow);
            reader->cur_token = reader->cur_tokenrow->start;
        }
        
        if (reader->tokens_infront)
        {
            reader->tokens_infront--;
            result = reader->cur_token++;
        }
        else result = _cpp_lex_token_direct(reader);
        if (result == NULL) return NULL;
        
        if (result->flags & START_OF_LINE)
        {
            if (result->type == CPP_HASH
                /* If collecting_args == 1, starting '(' is collected,
                 * so this token will be reread later. */
                && reader->state.collecting_args != 1)
            {
                if (_cpp_handle_directive(reader) == 0)
                {
                    if (reader->directive_result.type == CPP_PADDING)
                        continue;
                    result = &(reader->directive_result);
                }
            }

            if (reader->callbacks.line_change
                && !(reader->state.skipping))
            {
                reader->callbacks.line_change(reader,
                                              result,
                                              reader->state.collecting_args);
            }
        }

        /* Tokens are not skipped in directives. */
        if (reader->state.in_directive) break;

        if (!(reader->state.skipping) || (result->type == CPP_END))
            break;
    }

    return (result);
}

const cpp_token *cpp_peek_token(cpp_reader *reader, size_t index)
{
    cpp_unknown2 *unknown2 = reader->unknown2;
    const cpp_token *token;
    unsigned int count;
    void (*line_change)(cpp_reader *, const cpp_token *, int);

    while (unknown2)
    {
        unsigned int p = _cpp_remaining_tokens_in_unknown2(unknown2);

        if (index < p) return (_cpp_token_from_unknown2_at(unknown2, index));

        index -= p;
        unknown2 = unknown2->next;
    }

    /* More tokens need to be read without destroying the old ones. */
    count = index;
    reader->keep_tokens++;

    /* For peeked tokens line_change is temporary turned off
     * until they are read for real. */
    line_change = reader->callbacks.line_change;
    reader->callbacks.line_change = NULL;

    do {
        token = _cpp_lex_token(reader);
        if (token->type == CPP_END)
        {
            index--;
            break;
        }
    } while (index--);

    _cpp_return_tokens_to_lexer(reader, count - index);
    reader->callbacks.line_change = line_change;
    reader->keep_tokens--;

    return (token);
}

cpp_token *_cpp_temp_token(cpp_reader *reader)
{
    cpp_token *new, *old;

    /* _cpp_lex_token() never leaves
     * reader->cur_token == reader->cur_tokenrow->start.*/
    old = reader->cur_token - 1;

    if (reader->tokens_infront)
    {
        unsigned int tokens_infront = reader->tokens_infront;
        tokenrow *row = reader->cur_tokenrow;

        if (tokens_infront < row->end - reader->cur_token)
        {
            memmove(reader->cur_token + 1, reader->cur_token,
                    sizeof(cpp_token) * tokens_infront);
        }
        else
        {
            printf("+++TEST _cpp_temp_token\n");
            tokens_infront -= row->end - reader->cur_token;

            while (1)
            {
                row = next_tokenrow(row);
                if (row->end - row->start > tokens_infront)
                {
                    break;
                }
                tokens_infront -= row->end - row->start;
            }

            memmove(row->start + 1, row->start,
                    sizeof(cpp_token) * tokens_infront);
            while (1)
            {
                row->start[0] = row->prev->end[-1];
                row = row->prev;
                if (row == reader->cur_tokenrow) break;
                memmove(row->start + 1, row->start,
                        sizeof(cpp_token) * (row->end - row->start - 1));
            }

            if (reader->cur_token + 1 < row->end)
            {
                memmove(reader->cur_token + 1, reader->cur_token,
                        sizeof(cpp_token)
                        * (row->end - reader->cur_token - 1));
            }
        }
    }
    
    if (reader->cur_token == reader->cur_tokenrow->end)
    {
        reader->cur_tokenrow = next_tokenrow(reader->cur_tokenrow);
        reader->cur_token = reader->cur_tokenrow->start;
    }

    new = reader->cur_token++;
    new->src_loc = old->src_loc;

    return (new);
}

unsigned int _cpp_remaining_tokens_in_unknown2(cpp_unknown2 *unknown2)
{
    switch (unknown2->tokens_kind)
    {
        case TOKENS_KIND_DIRECT: return (unknown2->last.token
                                         - unknown2->first.token);
        case TOKENS_KIND_INDIRECT: return (unknown2->last.ptoken
                                           - unknown2->first.ptoken);
    }

    printf("CPPLIB Internal error %s:%u\n", __FILE__, __LINE__);
    return 0;
}

const cpp_token *_cpp_token_from_unknown2_at(cpp_unknown2 *unknown2,
                                         unsigned int index)
{
    switch (unknown2->tokens_kind)
    {
        case TOKENS_KIND_DIRECT: return (&(unknown2->first.token[index]));
        case TOKENS_KIND_INDIRECT: return (unknown2->first.ptoken[index]);
    }

    printf("CPPLIB Internal error %s:%u\n", __FILE__, __LINE__);
    return NULL;
}

void cpp_force_token_locations(cpp_reader *reader, location_t lok)
{
    reader->forced_location = lok;
}

void cpp_stop_forcing_token_locations(cpp_reader *reader)
{
    reader->forced_location.file = NULL;
}
