/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

typedef struct fixup {
    struct frag *frag;
    unsigned long where;
    unsigned int size;
    symbolS *add_symbol;
    long add_number;
    int pcrel;

    int done;

    struct fixup *next;
} fixupS;

fixupS *fixup_new(fragS *frag,
                  unsigned long where,
                  int size,
                  symbolS *add_symbol,
                  unsigned long add_number,
                  int pcrel);
fixupS *fixup_new_expr(fragS *frag,
                       unsigned long where,
                       int size,
                       exprS *expr,
                       int pcrel);

void write_object_file(void);
