/******************************************************************************
 * @file            sections.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#define SECTION_FLAG_ALLOC      (1U << 0)
#define SECTION_FLAG_LOAD       (1U << 1)
#define SECTION_FLAG_READONLY   (1U << 2)
#define SECTION_FLAG_CODE       (1U << 3)
#define SECTION_FLAG_DATA       (1U << 4)
#define SECTION_FLAG_NEVER_LOAD (1U << 5)
#define SECTION_FLAG_DEBUGGING  (1U << 6)
#define SECTION_FLAG_EXCLUDE    (1U << 7)
#define SECTION_FLAG_NOREAD     (1U << 8)
#define SECTION_FLAG_SHARED     (1U << 9)

struct frag_chain {

    struct frag *first_frag;
    struct frag *last_frag;
    
    struct fixup *first_fixup;
    struct fixup *last_fixup;

    struct cfi_frag_chain_data *cfi_frag_chain_data;
    
    subsection_t subsection;
    struct frag_chain *next;

};

extern struct frag_chain *current_frag_chain;

struct symbol *section_symbol (section_t section);

section_t section_subsection_set (section_t section, subsection_t subsection);
section_t section_set (section_t section);
section_t section_subsection_set_by_name (const char *name, subsection_t subsection);
section_t section_set_by_name (const char *name);
section_t section_get_next_section (section_t section);

const char *section_get_name (section_t section);

unsigned int section_get_number (section_t section);
unsigned int sections_get_count (void);

void sections_chain_subsection_frags (void);
void sections_init (void);
void section_set_object_format_dependent_data (section_t section, void *data);

void sections_number (unsigned int start_at);
void *section_get_object_format_dependent_data (section_t section);

section_t section_find_by_number (unsigned int number);

void section_set_flags (section_t section, unsigned int flags);
unsigned int section_get_flags (section_t section);
void section_set_alignment_power (section_t section, int alignment_power);
int section_get_alignment_power (section_t section);
