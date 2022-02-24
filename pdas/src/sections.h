/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

typedef struct frag_chain {
    struct frag *first_frag;
    struct frag *last_frag;

    fixupS *first_fixup;
    fixupS *last_fixup;
} frag_chainS;

extern frag_chainS *current_frag_chain;
