/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

/* Available object file targets:
 * TARGET_OBJECT_FILE_A_OUT
 * TARGET_OBJECT_FILE_COFF */

extern char *output_file_name;
extern int flag_generate_listing;

typedef struct section *sectionT;

extern sectionT undefined_section;
extern sectionT text_section;
extern sectionT data_section;
extern sectionT bss_section;

extern sectionT current_section;

/* Relaxation. */
typedef enum {
    relax_type_none_needed = 0,

    relax_type_align,

    relax_type_align_code,

    relax_type_machine_dependent
} relax_typeT;

typedef unsigned int relax_subtypeT;

/* a_out.c */
void object_dependent_write_object_file(void);

/* read.c */
#define LEX_NAME_PART  0x1
#define LEX_NAME_START 0x2

extern char lex_table[];

#define is_name_beginner(c) (lex_table[(c)] & LEX_NAME_START)
#define is_name_part(c)     (lex_table[(c)] & LEX_NAME_PART)

extern char *input_line_pointer;

#define SKIP_WHITESPACE() \
 do {if (*input_line_pointer == ' ') input_line_pointer++;} while(0)

char get_symbol_end(void);
void read_a_source_file(const char *filename);

#include "symbols.h"
#include "expr.h"
#include "frags.h"
#include "write.h"
#include "sections.h"

/* sections.c */
void sections_init(void);
void section_set(sectionT section);
symbolS *section_symbol(sectionT section);

/* error.c */
unsigned long as_get_error_count(void);
void as_error(char *format, ...);

/* listing.c */
void add_listing_line(char *line, unsigned long line_number);
void update_listing_line(fragS *frag);
void generate_listing(void);

/* as_i386.c */
#include "as_i386.h"
void machine_dependent_assemble_line(char *line);
long machine_dependent_pcrel_from(fixupS *fixup);
void machine_dependent_apply_fixup(fixupS *fixup,
                                   unsigned long value);
long machine_dependent_estimate_size_before_relax(fragS *frag,
                                                  sectionT section);
long machine_dependent_relax_frag(fragS *frag, sectionT section, long change);
void machine_dependent_finish_frag(fragS *frag);

