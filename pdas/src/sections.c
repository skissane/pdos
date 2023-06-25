/******************************************************************************
 * @file            sections.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "as.h"

struct section {

    char *name;
    
    struct symbol *symbol;
    struct frag_chain *frag_chain;
    struct section *next;

    flag_int flags;
    int alignment_power;
    
    void *object_format_dependent_data;
    unsigned int number;

};

static struct section internal_sections[4];
static struct symbol section_symbols[4];

static int frags_chained = 0;

section_t sections = NULL;

section_t undefined_section;
section_t absolute_section;
section_t expr_section;
section_t reg_section;
section_t text_section;
section_t data_section;
section_t bss_section;

section_t current_section;
subsection_t current_subsection;

struct frag_chain *current_frag_chain;

static void subsection_set (subsection_t subsection) {

    struct frag_chain *frag_chain, **p_next;
    
    if (frags_chained && subsection) {
        as_internal_error_at_source_at (__FILE__, __LINE__, NULL, 0,
                                        "frags have been already chained, switching to subsection %li is invalid", subsection);
    }
    
    for (frag_chain = *(p_next = &(current_section->frag_chain)); frag_chain; frag_chain = *(p_next = &(frag_chain->next))) {
    
        if (frag_chain->subsection >= subsection) {
            break;
        }
    
    }
    
    if (!frag_chain || frag_chain->subsection != subsection) {
    
        struct frag_chain *new_frag_chain = xmalloc (sizeof (*new_frag_chain));
        
        new_frag_chain->last_frag  = new_frag_chain->first_frag  = frag_alloc ();
        new_frag_chain->last_fixup = new_frag_chain->first_fixup = NULL;
        new_frag_chain->cfi_frag_chain_data = NULL;
        new_frag_chain->subsection = subsection;
        
        *p_next = new_frag_chain;
        new_frag_chain->next = frag_chain;
        
        frag_chain = new_frag_chain;
    
    }
    
    current_frag_chain = frag_chain;
    current_frag       = current_frag_chain->last_frag;
    
    current_subsection = subsection;

}

static section_t find_or_make_section_by_name (const char *name) {

    section_t section, *p_next;
    
    for (p_next = &sections, section = sections; section; p_next = &(section->next), section = *p_next) {
        
        if (strcmp (name, section->name) == 0) {
            break;
        }
    
    }
    
    if (section == NULL) {
    
        section = xmalloc (sizeof (*section));
        section->name = xstrdup (name);
        
        section->symbol = symbol_create (name, section, 0, &zero_address_frag);
        section->symbol->flags |= SYMBOL_FLAG_SECTION_SYMBOL;
        symbol_add_to_chain (section->symbol);
        
        section->frag_chain = NULL;
        section->next = NULL;

        section->flags = 0;
        section->alignment_power = -1;
        
        section->object_format_dependent_data = NULL;
        section->number = 0;
        
        *p_next = section;
    
    }
    
    return section;

}

struct symbol *section_symbol (section_t section) {
    return section->symbol;
}

section_t section_subsection_set (section_t section, subsection_t subsection) {

    current_section = section;
    
    subsection_set (subsection);
    return section;

}

section_t section_set (section_t section) {
    return section_subsection_set (section, 0);
}

section_t section_subsection_set_by_name (const char *name, subsection_t subsection) {
    return section_subsection_set (find_or_make_section_by_name (name), subsection);
}

section_t section_set_by_name (const char *name) {
    return section_subsection_set_by_name (name, 0);
}

section_t section_get_next_section (section_t section) {
    return section->next;
}

const char *section_get_name (section_t section) {
    return section->name;
}

unsigned int section_get_number (section_t section) {
    return section->number;
}

unsigned int sections_get_count (void) {

    section_t section;
    unsigned int count;

    for (section = sections, count = 0; section; section = section->next, count++);
    return count;

}

void sections_chain_subsection_frags (void) {

    section_t section;
    
    for (section = sections; section; section = section->next) {
    
        struct frag_chain *frag_chain;
        
        section->frag_chain->subsection = 0;
        
        for (frag_chain = section->frag_chain->next; frag_chain; frag_chain = frag_chain->next) {
        
            section->frag_chain->last_frag->next = frag_chain->first_frag;
            section->frag_chain->last_frag = frag_chain->last_frag;
            
            if (section->frag_chain->first_fixup) {
            
                section->frag_chain->last_fixup->next = frag_chain->first_fixup;
                section->frag_chain->last_fixup = frag_chain->last_fixup;
            
            } else {
            
                section->frag_chain->first_fixup = frag_chain->first_fixup;
                section->frag_chain->last_fixup = frag_chain->last_fixup;
            
            }
        
        }
    
    }
    
    frags_chained = 1;

}

void sections_init (void) {

#define CREATE_INTERNAL_SECTION(section_var, section_name, section_index) \
    (section_var) = &internal_sections[(section_index)]; \
    (section_var)->name = (section_name); \
    (section_var)->symbol = &section_symbols[(section_index)]; \
    (section_var)->symbol->name    = (section_name); \
    (section_var)->symbol->section = (section_var); \
    (section_var)->symbol->frag    = &zero_address_frag; \
    symbol_set_value ((section_var)->symbol, 0); \
    (section_var)->symbol->flags  |= SYMBOL_FLAG_SECTION_SYMBOL

    CREATE_INTERNAL_SECTION (undefined_section, "*UND*",  0);
    CREATE_INTERNAL_SECTION (absolute_section,  "*ABS*",  1);
    CREATE_INTERNAL_SECTION (expr_section,      "*EXPR*", 2);
    CREATE_INTERNAL_SECTION (reg_section,       "*REG*",  3);

#undef CREATE_INTERNAL_SECTION
    
    text_section      = section_set_by_name (".text");
    section_set_flags (text_section,
                       SECTION_FLAG_LOAD
                       | SECTION_FLAG_ALLOC
                       | SECTION_FLAG_READONLY
                       | SECTION_FLAG_CODE);

    data_section      = section_set_by_name (".data");
    section_set_flags (data_section,
                       SECTION_FLAG_LOAD
                       | SECTION_FLAG_ALLOC
                       | SECTION_FLAG_DATA);
    
    bss_section       = section_set_by_name (".bss");
    section_set_flags (bss_section,
                       SECTION_FLAG_ALLOC);
    
    /* .text section is the default section. */
    section_set (text_section);

}

void sections_destroy (void)
{
    section_t section, next_section;
    
    for (section = sections; section; section = next_section) {
        struct frag_chain *frag_chain, *next_frag_chain;
        
        for (frag_chain = section->frag_chain; frag_chain; frag_chain = next_frag_chain) {
            if (!frags_chained || frag_chain == section->frag_chain) {
                struct frag *frag, *next_frag;
                struct fixup *fixup, *next_fixup;

                for (frag = frag_chain->first_frag; frag; frag = next_frag) {
                    next_frag = frag->next;
                    frag_destroy (frag);
                }
                for (fixup = frag_chain->first_fixup; fixup; fixup = next_fixup) {
                    next_fixup = fixup->next;
                    free (fixup);
                }
            }
            
            next_frag_chain = frag_chain->next;
            free (frag_chain);
        }
        
        next_section = section->next;

        free (section->name);
        free (section);
    }
}

void section_set_object_format_dependent_data (section_t section, void *data) {
    section->object_format_dependent_data = data;
}

void sections_number (unsigned int start_at) {

    section_t section;
    
    for (section = sections; section; section = section->next) {
        section->number = start_at++;
    }

}

void *section_get_object_format_dependent_data (section_t section) {
    return section->object_format_dependent_data;
}

section_t section_find_by_number (unsigned int number) {

    section_t section;
    
    for (section = sections; section; section = section->next) {
        
        if (section->number == number) {
            break;
        }
    
    }

    return section;

}

void section_set_flags (section_t section, flag_int flags)
{
    section->flags = flags;
}

flag_int section_get_flags (section_t section)
{
    return section->flags;
}

void section_set_alignment_power (section_t section, int alignment_power)
{
    section->alignment_power = alignment_power;
}

int section_get_alignment_power (section_t section)
{
    return section->alignment_power;
}

void section_record_alignment_power (section_t section, int alignment_power)
{
    if (alignment_power > section->alignment_power) {
        section->alignment_power = alignment_power;
    }
}
