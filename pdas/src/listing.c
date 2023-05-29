/******************************************************************************
 * @file            listing.c
 *****************************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "as.h"

struct listing_message {

    char *message;
    struct listing_message *next;

};

struct ll {

    char *line;
    const char *filename;
    unsigned long line_number;
    
    struct frag *frag;
    
    unsigned long where;
    unsigned long size;
    
    int variant_frag;
    struct ll *next;
    
    struct listing_message *messages, *last_message;

};

static struct ll *first_line = NULL;
static struct ll *last_line = NULL;

static void internal_add_line (char *line, const char *filename, unsigned long line_number) {

    struct ll *ll = xmalloc (sizeof (*ll));

    memset (ll, 0, sizeof (*ll));
    
    ll->line = line;
    ll->filename = filename;
    ll->line_number = line_number;
    ll->frag = current_frag;
    
    if (current_frag) {
        ll->where = current_frag->fixed_size;
    }
    
    if (first_line == NULL) {
    
        first_line = ll;
        last_line = ll;
    
    } else {
    
        last_line->next = ll;
        last_line = ll;
    
    }
    
    ll->messages = ll->last_message = NULL;

}

void add_listing_line (char *real_line, size_t real_line_len, const char *filename, unsigned long line_number) {

    char *line;
    size_t start, i;
    
    for (start = 0, i = 0; i < real_line_len; i++) {
    
        if (real_line[i] == '\n') {
        
            line = xmalloc (i - start + 1);
            
            memcpy (line, real_line + start, i - start);
            line[i - start] = '\0';
            
            internal_add_line (line, filename, line_number);
            line_number++;
            
            if (i == real_line_len - 1) {
                return;
            }
            
            start = i + 1;
        
        }
    
    }
    
    line = xmalloc (i - start + 1);
    
    memcpy (line, real_line + start, i - start);
    line[i - start + 1] = '\0';
    
    internal_add_line (line, filename, line_number);

}

void add_listing_message (char *message, const char *filename, unsigned long line_number) {

    struct ll *ll;
    
    for (ll = first_line; ll; ll = ll->next) {
    
        if (ll->line_number == line_number && strcmp (ll->filename, filename) == 0) {
        
            struct listing_message *lm = xmalloc (sizeof (*lm));
            
            lm->message = message;
            lm->next = NULL;
            
            if (ll->last_message) {
                ll->last_message->next = lm;
            } else {
                ll->messages = lm;
            }
            
            ll->last_message = lm;
            return;
        
        }
    
    }

}

void generate_listing (void) {

    struct ll *ll;
    struct symbol *symbol;
    
    FILE *f = stdout;
    
    if (state->listing) {
    
        if ((f = fopen (state->listing, "w")) == NULL) {
        
            as_error_at (NULL, 0, "Unable to open '%s' as listing file");
            return;
        
        }
    
    }
    
    for (ll = first_line; ll; ll = ll->next) {
    
        struct listing_message *lm;
        unsigned long i, size;
        
        if (ll->frag == NULL) {
            size = 0;
        } else if (ll->variant_frag) {
            size = ll->frag->fixed_size - ll->where;
        } else {
            size = ll->size;
        }
        
        fprintf (f, "%05lu    ", ll->line_number);
        
        for (i = 0; i < size; i++) {
        
            if ((i > 0) && ((i % 4) == 0)) {
                fprintf (f, "\n%05lu    ", ll->line_number);
            }
            
            fprintf (f, "%02X", ll->frag->buf[ll->where + i]);
        
        }
        
        if ((i > 0) && ((i % 4) == 0)) {
            fprintf (f, "\n%05lu    ", ll->line_number);
        }
        
        i %= 4;
        
        for ( ; i < 8; i++) {
            fprintf (f, "  ");
        }
        
        if (ll->frag) {
            fprintf (f, "%04lx", ll->frag->address + ll->where);
        } else {
            fprintf (f, "     ");
        }
        
        fprintf (f, "    %s\n", ll->line);
        
        for (lm = ll->messages; lm; lm = lm->next) {
            fprintf (f, "*****  %s\n", lm->message);
        }
    
    }
    
    if (symbols != NULL) {
    
        unsigned long defined_symbols = 0;
        unsigned long undefined_symbols = 0;
        
        for (symbol = symbols; symbol; symbol = symbol->next) {
        
            if (symbol_is_section_symbol (symbol)) {
                continue;
            }
            
            if (symbol_is_undefined (symbol)) {
                undefined_symbols++;
            } else {
                defined_symbols++;
            }
        
        }
        
        if (defined_symbols > 0) {
        
            fprintf (f, "\nDEFINED SYMBOLS:\n\n");
            
            for (symbol = symbols; symbol; symbol = symbol->next) {
            
                if (symbol_is_section_symbol (symbol)) {
                    continue;
                }
                
                if (symbol_is_undefined (symbol)) {
                    continue;
                }
                
                fprintf (f, "    %08"PRIxVALUE"    %s\n", symbol_get_value (symbol), symbol_get_name (symbol));
            
            }
        
        } else {
            fprintf (f, "\nNO DEFINED SYMBOLS\n");
        }
        
        if (undefined_symbols > 0) {
        
            fprintf (f, "\nUNDEFINED SYMBOLS:\n\n");
            
            for (symbol = symbols; symbol; symbol = symbol->next) {
            
                if (symbol_is_section_symbol (symbol)) {
                    continue;
                }
                
                if (symbol_is_undefined (symbol)) {
                    fprintf (f, "    %08"PRIxVALUE"    %s\n", symbol_get_value (symbol), symbol_get_name (symbol));
                }
            
            }
        
        } else {
            fprintf (f, "\nNO UNDEFINED SYMBOLS\n");
        }
    
    }
    
    if (state->listing) { fclose (f); }

}

void update_listing_line (struct frag *frag) {

    if (last_line == NULL || last_line->frag == NULL) { return; }
    
    if (last_line->frag->next == frag) {
        last_line->variant_frag = 1;
    } else {
        last_line->size = last_line->frag->fixed_size - last_line->where;
    }

}

void listing_destroy (void)
{
    struct ll *ll, *next_ll;

    for (ll = first_line; ll; ll = next_ll) {
        struct listing_message *lm, *next_lm;

        for (lm = ll->messages; lm; lm = next_lm) {
            next_lm = lm->next;
            
            free (lm->message);
            free (lm);
        }

        next_ll = ll->next;

        free (ll->line);
        free (ll);
    }
}

