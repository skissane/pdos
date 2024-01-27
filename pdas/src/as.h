/******************************************************************************
 * @file            as.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <inttypes.h>

enum { AS_FORMAT_A_OUT, AS_FORMAT_COFF, AS_FORMAT_ELF };

#if     defined (__GNUC__)
# define    NORETURN            __attribute__ ((noreturn))
#elif   defined (_WIN32)
# define    NORETURN            __declspec (noreturn)
#else
# define    NORETURN
#endif

struct as_state {

    char **files;
    int nb_files;
    
    char **include_paths;
    int nb_include_paths;
    
    char *outfile;
    int format;
    
    int generate_listing;
    char *listing;

};

#define     ARRAY_SIZE(arr)             (sizeof (arr) / sizeof (arr[0]))

extern struct as_state *state;
extern const char *program_name;

#define     LEX_NAME_PART               0x0001
#define     LEX_NAME_START              0x0002

extern char lex_table[];

#define     is_name_beginner(c)         (lex_table[(c)] & LEX_NAME_START)
#define     is_name_part(c)             (lex_table[(c)] & LEX_NAME_PART)

extern char is_end_of_line[];

typedef     struct section              *section_t;
typedef     signed long                 subsection_t;

typedef enum {

    RELAX_TYPE_NONE_NEEDED = 0,
    RELAX_TYPE_ALIGN,
    RELAX_TYPE_ALIGN_CODE,
    RELAX_TYPE_ORG,
    RELAX_TYPE_SPACE,
    RELAX_TYPE_CFI,
    RELAX_TYPE_MACHINE_DEPENDENT

} relax_type_t;

typedef     unsigned int                relax_subtype_t;

typedef enum {

    RELOC_TYPE_DEFAULT,
    RELOC_TYPE_RVA

} reloc_type_t;

typedef unsigned long address_t;

typedef int_fast64_t offset_t;
typedef uint_fast64_t value_t;
#define PRIuVALUE PRIuFAST64
#define PRIxVALUE PRIxFAST64
#define PRIXVALUE PRIXFAST64
#define PRIiOFFSET PRIiFAST64

typedef unsigned long flag_int;

#include    "frags.h"
#include    "expr.h"
#include    "symbols.h"
#include    "write.h"
#include    "sections.h"

#define     FAKE_LABEL_NAME             "FAKE_PDAS_SYMBOL"

extern section_t sections;

extern section_t undefined_section;
extern section_t absolute_section;
extern section_t expr_section;
extern section_t reg_section;
extern section_t text_section;
extern section_t data_section;
extern section_t bss_section;

#define     SECTION_IS_NORMAL(section)  \
    ((section != undefined_section) && (section != absolute_section) && (section != expr_section) && (section != reg_section))

extern section_t current_section;
extern subsection_t current_subsection;

struct pseudo_op_entry {

    const char *name;
    void (*handler) (char **pp);

};

/* a_out.c */
struct pseudo_op_entry *a_out_get_pseudo_op_table (void);
void write_a_out_file (void);

/* i386_as.c */
struct pseudo_op_entry *machine_dependent_get_pseudo_op_table (void);
const char *machine_dependent_get_line_separators (void);
const char *machine_dependent_get_comment_at_the_start_of_line_beginners (void);

void machine_dependent_assemble_line (char *line);
int machine_dependent_force_relocation_local (struct fixup *fixup);

offset_t machine_dependent_estimate_size_before_relax (struct frag *frag, section_t section);
offset_t machine_dependent_pcrel_from (struct fixup *fixup);
offset_t machine_dependent_relax_frag (struct frag *frag, section_t section, offset_t change);

void machine_dependent_apply_fixup (struct fixup *fixup, value_t value);
void machine_dependent_finish_frag (struct frag *frag);
void machine_dependent_init (void);
void machine_dependent_destroy (void);

enum expr_type machine_dependent_parse_operator (char **pp, char *name, char *original_saved_c, unsigned int operands);

int machine_dependent_need_index_operator (void);
int machine_dependent_parse_name (char **pp, struct expr *expr, char *name, char *original_saved_c);

void machine_dependent_parse_operand (char **pp, struct expr *expr);

section_t machine_dependent_simplified_expression_read_into (char **pp, struct expr *expr);
void machine_dependent_number_to_chars (unsigned char *p, value_t number, unsigned long size);

/* coff.c */
struct pseudo_op_entry *coff_get_pseudo_op_table (void);
void write_coff_file (void);
void coff_x86_set_bits (int bits);

/* elf.c */
struct pseudo_op_entry *elf_get_pseudo_op_table (void);
void write_elf_file (void);

/* error.c */
unsigned long as_get_error_count (void);

void as_error (const char *fmt, ...);
void as_error_at (const char *filename, unsigned long line_number, const char *fmt, ...);
NORETURN void as_fatal_error_at (const char *filename, unsigned long line_number, const char *fmt, ...);
void as_warn (const char *fmt, ...);
void as_warn_at (const char *filename, unsigned long line_number, const char *fmt, ...);

NORETURN void as_internal_error_at_source (const char *code_filename,
                                  unsigned long code_line_number,
                                  const char *fmt,
                                  ...);
NORETURN void as_internal_error_at_source_at (const char *code_filename,
                                     unsigned long code_line_number,
                                     const char *filename,
                                     unsigned long line_number,
                                     const char *fmt,
                                     ...);

/* libas.c */
char *skip_whitespace (char *p);

void *xmalloc (size_t size);
void *xrealloc (void *ptr, size_t size);

int   xstrcasecmp (const char *s1, const char *s2);
char *xstrdup (const char *str);
char *xstrsep (char **stringp, const char *delim);

/* listing.c */
void add_listing_line (char *real_line, size_t real_line_len, const char *filename, unsigned long line_number);
void add_listing_message (char *message, const char *filename, unsigned long line_number);
void generate_listing (void);
void update_listing_line (struct frag *frag);
void listing_destroy (void);

/* load_line.c */
int load_line (char **line_p, char **line_end_p, char **real_line_p, size_t *real_line_len_p,
    unsigned long *newlines_p, FILE *input_file, void **load_line_internal_data_p);

void *load_line_create_internal_data (unsigned long *new_line_number_p);
void  load_line_destroy_internal_data (void *load_line_internal_data);

/* process.c */
char get_symbol_name_end (char **pp);
char *find_end_of_line (char *line);

int process (const char *fname);

void demand_empty_rest_of_line (char **pp);
void get_filename_and_line_number (const char **filename_p, unsigned long *line_number_p);
void handler_asciz (char **pp);
void handler_ignore (char **pp);
void handler_lcomm (char **pp);
void ignore_rest_of_line (char **pp);
void process_init (void);
void process_destroy (void);
