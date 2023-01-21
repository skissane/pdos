/******************************************************************************
 * @file            cfi.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
/* Operand is encoded inside the low 6 bits of the following 3 instructions. */
#define DW_CFA_advance_loc        (0x1U << 6)
#define DW_CFA_offset             (0x2U << 6)
#define DW_CFA_restore            (0x3U << 6)

#define DW_CFA_nop                0
#define DW_CFA_set_loc            0x01
#define DW_CFA_advance_loc1       0x02
#define DW_CFA_advance_loc2       0x03
#define DW_CFA_advance_loc4       0x04
#define DW_CFA_offset_extended    0x05
#define DW_CFA_restore_extended   0x06
#define DW_CFA_undefined          0x07
#define DW_CFA_same_value         0x08
#define DW_CFA_register           0x09
#define DW_CFA_remember_state     0x0a
#define DW_CFA_restore_state      0x0b
#define DW_CFA_def_cfa            0x0c
#define DW_CFA_def_cfa_register   0x0d
#define DW_CFA_def_cfa_offset     0x0e
#define DW_CFA_def_cfa_expression 0x0f
#define DW_CFA_expression         0x10
#define DW_CFA_offset_extended_sf 0x11
#define DW_CFA_def_cfa_sf         0x12
#define DW_CFA_def_cfa_offset_sf  0x13
#define DW_CFA_val_offset         0x14
#define DW_CFA_val_offset_sf      0x15
#define DW_CFA_val_expression     0x16
#define DW_CFA_lo_user            0x1c
#define DW_CFA_hi_user            0x3f


struct cfi_instruction_data {

    struct cfi_instruction_data *next;

    int instruction;

    union {

        struct {
            unsigned int reg;
            offset_t offset;
        } ro;

        unsigned int reg;
        
        offset_t offset;

        struct {
            struct symbol *label1;
            struct symbol *label2;
        } ll;

    } u;

};

struct cfi_frag_chain_data {
    
    struct fde *fde;
    struct symbol *last_address;

};

struct fde {

    struct fde *next;

    struct symbol *start_address;
    struct symbol *end_address;

    struct cfi_instruction_data *data;
    struct cfi_instruction_data **last_data_p;

};

struct pseudo_op_entry *cfi_get_pseudo_op_table (void);

void cfi_finish (void);
int cfi_estimate_size_before_relax (struct frag *frag);
int cfi_relax_frag (struct frag *frag);
void cfi_finish_frag (struct frag *frag);
