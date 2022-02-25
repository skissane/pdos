/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "as.h"

#include "xmalloc.h"

typedef struct ll {
    char *line;
    unsigned long line_number;
    fragS *frag;
    unsigned long where;
    unsigned long size;
    int variant_frag;

    struct ll *next;
} listing_line;

static listing_line *first_line = NULL;
static listing_line *last_line = NULL;

void add_listing_line(char *line, unsigned long line_number)
{
    listing_line *ll = xmalloc(sizeof(*ll));

    memset(ll, '\0', sizeof(*ll));

    ll->line = xstrdup(line);
    ll->line_number = line_number;
    ll->frag = current_frag;
    if (current_frag) ll->where = current_frag->fixed_size;
    
    if (first_line == NULL)
    {
        first_line = ll;
        last_line = ll;
    }
    else
    {
        last_line->next = ll;
        last_line = ll;
    }
}

void update_listing_line(fragS *frag)
{
    if (last_line == NULL) return;
    if (last_line->frag == NULL) return;
    
    if (last_line->frag->next == frag)
    {
        last_line->variant_frag = 1;
    }
    else
    {
        last_line->size = last_line->frag->fixed_size - last_line->where;
    }
}

void generate_listing(void)
{
    listing_line *ll;

    for (ll = first_line;
         ll;
         ll = ll->next)
    {
        unsigned long size, i;

        if (ll->frag == NULL) size = 0;
        else if (ll->variant_frag)
        {
            size = ll->frag->next->address - ll->frag->address - ll->where;
        }
        else size = ll->size;

        printf("%lu ", ll->line_number);

        if (ll->frag)
        {
            printf("%04X ", ll->frag->address + ll->where);
        }
        else printf("     ");
        
        for (i = 0; i < size; i++)
        {
            printf("%02x", ll->frag->buf[ll->where + i]);
        }

        printf("    %s", ll->line);
    }
}
