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

static unsigned int hex_value(unsigned int c)
{
    if ((c >= '0') && (c <= '9')) return (c - '0');
    if ((c >= 'a') && (c <= 'f')) return (c + 10 - 'a');
    if ((c >= 'A') && (c <= 'F')) return (c + 10 - 'A');

    printf("CPPLIB Internal error %s:%u", __FILE__, __LINE__);
    return 0;
}

int cpp_interpret_string(cpp_reader *reader, const cpp_string *in,
                         cpp_string *out, enum cpp_tokentype type)
{
    const char *p = in->text;
    char *p2;
    
    if ((type != CPP_STRING) && (type != CPP_CHAR))
    {
        printf("+++strings other than CPP_STRING and CPP_CHAR"
               " are not supported %d %s\n", __LINE__, __FILE__);
        return (1);
    }

    p2 = xmalloc(in->len);

    out->text = p2;
    for ( ;
         p < in->text + in->len;
         p++, p2++)
    {
        if (*p == '"') p++;
        if (*p == '\\')
        {
            p++;
            switch (*p)
            {
                case 'a': *p2 = '\a'; continue;
                case 'b': *p2 = '\b'; continue;
                case 'f': *p2 = '\f'; continue;
                case 'n': *p2 = '\n'; continue;
                case 'r': *p2 = '\r'; continue;
                case 't': *p2 = '\t'; continue;
                case 'v': *p2 = '\v'; continue;
                case '0': *p2 = '\0'; continue;
                case 'x': {
                    const char *orig = p;
                    unsigned int h;
                    p++;
                    if (!ISXDIGIT(*p))
                    {
                        cpp_error(reader, CPP_DL_ERROR,
                                  "\\x used"
                                  " with no following hexadecimal digits");
                    }
                    h = 0;
                    while (ISXDIGIT(*p))
                    {
                        h *= 16;
                        h += hex_value(*p);
                        p++;
                    }
                    p--;
                    if (h > 0xff)
                    {
                        cpp_error(reader, CPP_DL_WARNING,
                                  "hexadecimal escape sequence out of range");
                        *(p2++) = '\\';
                        p = orig;
                    }
                    else
                    {
                        *p2 = h;
                        continue;
                    }
                    break;
                }
                case '1': case '2': case '3':
                case '4': case '5': case '6':
                case '7': {
                    const char *orig = p;
                    unsigned int h;

                    h = 0;
                    while ((*p >= '0') && (*p <= '7'))
                    {
                        h *= 8;
                        h += *p - '0';
                        p++;
                    }
                    p--;
                    if (h > 0xff)
                    {
                        cpp_error(reader, CPP_DL_WARNING,
                                  "octal escape sequence out of range");
                        *(p2++) = '\\';
                        p = orig;
                    }
                    else
                    {
                        *p2 = h;
                        continue;
                    }
                    break;;
                }
            }
        }
        *p2 = *p;
    }
    return (0);
}

cppchar_t narrow_str_to_charconst(cpp_reader *reader, cpp_string str,
                                  unsigned int *pchars_seen, int *unsignedp,
                                  enum cpp_tokentype type)
{
    size_t width = CPP_OPTION(reader, char_precision);
    cppchar_t result = str.text[1];

    if ((type != CPP_STRING) && (type != CPP_CHAR))
    {
        printf("+++strings other than CPP_STRING and CPP_CHAR"
               " are not supported %d %s\n", __LINE__, __FILE__);
        return (1);
    }
    
    *pchars_seen = 1;
    *unsignedp = 0;

    if (width < 32)
    {
        size_t mask = (((cppchar_t)1) << width) - 1;
        if (*unsignedp || !(result & (1 << (width - 1))))
        {
            result &= mask;
        }
        else result |= ~mask;
    }
    return (result);
}

cppchar_t cpp_interpret_charconst(cpp_reader *reader, const cpp_token *token,
                                  unsigned int *pchars_seen, int *unsignedp)
{
    cpp_string str = {0, 0};
    int wide = token->type != CPP_CHAR;
    cppchar_t result;

    if (token->value.string.len == (size_t)(2 + wide))
    {
        cpp_error(reader, CPP_DL_ERROR, "empty character constant");
        *pchars_seen = 0;
        *unsignedp = 0;

        return (0);
    }
    else if (cpp_interpret_string(reader, &(token->value.string), &str,
                                  token->type))
    {
        *pchars_seen = 0;
        *unsignedp = 0;

        return (0);
    }

    if (wide)
    {
        printf("+++wide chars not supported %d %s\n", __LINE__, __FILE__);
        return (0);
    }
    else
    {
        result = narrow_str_to_charconst(reader, str, pchars_seen, unsignedp,
                                         token->type);
    }

    if (str.text != token->value.string.text) free((void *)(str.text));

    return (result);
}
