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
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>

#include    "as.h"
#include    "cfi.h"

static void emit_expr (struct expr *expr, unsigned int nbytes) {

    fixup_new_expr (current_frag, current_frag->fixed_size, nbytes, expr, 0, RELOC_TYPE_DEFAULT);
    frag_increase_fixed_size (nbytes);

}

static void output_uleb128 (value_t value) {

    do {

        unsigned int byte = value & 0x7f;

        value >>= 7;
        if (value) {
            byte |= 0x80;
        }

        frag_append_1_char (byte);

    } while (value);

}

static void output_sleb128 (offset_t value) {

    int more;
    int negative;
    value_t u_value;

    negative = value < 0;
    u_value = value;

    do {

        unsigned int byte = u_value & 0x7f;

        u_value >>= 7;
        if (negative) {
            u_value |= ~(((value_t)-(offset_t)1) >> 7);
        }

        more = !(((u_value == (value_t)-(offset_t)1) && (byte & 0x40))
                 || ((u_value == 0) && ((byte & 0x40) == 0)));
        if (more) {
            byte |= 0x80;
        }

        frag_append_1_char (byte);

    } while (more);

}

#define DEFAULT_CODE_ALIGNMENT_FACTOR 1
#define DEFAULT_DATA_ALIGNMENT_FACTOR (-4)
#define DEFAULT_RETURN_ADDRESS_REG 8

struct cie {

    struct cie *next;

    struct symbol *start_address;
    struct cfi_instruction_data *data, *end_data;

};

static int warned = 0;

static struct fde *all_fde = NULL;
static struct fde **last_fde_p = &all_fde;
static struct cie *all_cie = NULL;

static void out_nbyte_value (long value, int nbytes) {

    machine_dependent_number_to_chars (frag_increase_fixed_size (nbytes), value, nbytes);

}

static struct cfi_instruction_data *alloc_cfi_instruction_data (void) {

    struct cfi_instruction_data *cfi_instruction_data = xmalloc (sizeof (*cfi_instruction_data));
    struct fde *fde = current_frag_chain->cfi_frag_chain_data->fde;

    *(fde->last_data_p) = cfi_instruction_data;
    fde->last_data_p = &(cfi_instruction_data->next);

    return cfi_instruction_data;

}

static void cfi_add_CFA_instruction_reg (int instruction, unsigned int reg) {

    struct cfi_instruction_data *cfi_instruction_data = alloc_cfi_instruction_data ();

    cfi_instruction_data->instruction = instruction;
    cfi_instruction_data->u.reg = reg;

}

static void cfi_add_CFA_instruction_offset (int instruction, offset_t offset) {

    struct cfi_instruction_data *cfi_instruction_data = alloc_cfi_instruction_data ();

    cfi_instruction_data->instruction = instruction;
    cfi_instruction_data->u.offset = offset;

}

static void cfi_add_CFA_instruction_reg_offset (int instruction, unsigned int reg, offset_t offset) {

    struct cfi_instruction_data *cfi_instruction_data = alloc_cfi_instruction_data ();

    cfi_instruction_data->instruction = instruction;
    cfi_instruction_data->u.ro.reg = reg;
    cfi_instruction_data->u.ro.offset = offset;

}

static void cfi_add_DW_CFA_advance_loc (struct symbol *label) {

    struct cfi_instruction_data *cfi_instruction_data = alloc_cfi_instruction_data ();

    cfi_instruction_data->instruction = DW_CFA_advance_loc;
    cfi_instruction_data->u.ll.label1 = current_frag_chain->cfi_frag_chain_data->last_address;
    cfi_instruction_data->u.ll.label2 = label;

    current_frag_chain->cfi_frag_chain_data->last_address = label;

}

void cfi_add_CFA_offset (unsigned int reg, offset_t offset) {

    cfi_add_CFA_instruction_reg_offset (DW_CFA_offset, reg, offset);

}

void cfi_add_CFA_restore (unsigned int reg) {

    cfi_add_CFA_instruction_reg (DW_CFA_restore, reg);

}

void cfi_add_CFA_def_cfa (unsigned int reg, offset_t offset) {

    cfi_add_CFA_instruction_reg_offset (DW_CFA_def_cfa, reg, offset);

}

void cfi_add_CFA_def_cfa_register (unsigned int reg) {

    cfi_add_CFA_instruction_reg (DW_CFA_def_cfa_register, reg);

}

void cfi_add_CFA_def_cfa_offset (offset_t offset) {

    cfi_add_CFA_instruction_offset (DW_CFA_def_cfa_offset, offset);

}

static void output_cfi_instruction (struct cfi_instruction_data *cfi_instruction) {

    switch (cfi_instruction->instruction) {

        case DW_CFA_advance_loc:

            if (symbol_get_frag (cfi_instruction->u.ll.label1)
                == symbol_get_frag (cfi_instruction->u.ll.label2)) {

                address_t delta = symbol_get_value (cfi_instruction->u.ll.label2) - symbol_get_value (cfi_instruction->u.ll.label1);

                delta /= DEFAULT_CODE_ALIGNMENT_FACTOR;

                if (delta == 0) {
                    /* Nothing needs to be done. */
                } else if (delta <= 0x3F) {
                    frag_append_1_char (DW_CFA_advance_loc + delta);
                } else if (delta <= 0xFF) {
                    frag_append_1_char (DW_CFA_advance_loc1);
                    out_nbyte_value (delta, 1);
                } else if (delta <= 0xFFFF) {
                    frag_append_1_char (DW_CFA_advance_loc2);
                    out_nbyte_value (delta, 2);
                } else {
                    frag_append_1_char (DW_CFA_advance_loc4);
                    out_nbyte_value (delta, 4);
                }
                
            } else {

                as_internal_error_at_source_at (__FILE__, __LINE__,
                                                NULL, 0,
                                                "+++DW_CFA_advance_loc across multiple frags",
                                                cfi_instruction->instruction);

            }
            break;

        case DW_CFA_offset:

            frag_append_1_char (DW_CFA_offset + cfi_instruction->u.ro.reg);
            output_uleb128 (cfi_instruction->u.ro.offset / DEFAULT_DATA_ALIGNMENT_FACTOR);
            break;

        case DW_CFA_restore:

            frag_append_1_char (DW_CFA_restore + cfi_instruction->u.reg);
            break;

        case DW_CFA_def_cfa:

            frag_append_1_char (DW_CFA_def_cfa);
            output_uleb128 (cfi_instruction->u.ro.reg);
            output_uleb128 (cfi_instruction->u.ro.offset);
            break;

        case DW_CFA_def_cfa_register:

            frag_append_1_char (DW_CFA_def_cfa_register);
            output_uleb128 (cfi_instruction->u.reg);
            break;

        case DW_CFA_def_cfa_offset:

            frag_append_1_char (DW_CFA_def_cfa_offset);
            output_uleb128 (cfi_instruction->u.offset);
            break;

        default:

            as_internal_error_at_source_at (__FILE__, __LINE__,
                                            NULL, 0,
                                            "+++not yet supported CFI instruction 0x%x",
                                            cfi_instruction->instruction);
            break;

    }

}

static void output_cie (struct cie *cie) {

    struct symbol *after_length_address, *end_address;
    struct expr expr_s;
    struct cfi_instruction_data *i;

    cie->start_address = symbol_temp_new_now ();

    after_length_address = symbol_temp_make ();
    end_address = symbol_temp_make ();

    /* 4 byte CIE length that does not include the length field. */
    expr_s.type = EXPR_TYPE_SUBTRACT;
    expr_s.add_symbol = end_address;
    expr_s.op_symbol = after_length_address;
    expr_s.add_number = 0;

    emit_expr (&expr_s, 4);
    symbol_set_value_now (after_length_address);
    
    /* 4 byte CIE ID must be 0. */
    out_nbyte_value (0, 4);

    /* 1 byte CIE version. */
    out_nbyte_value (1, 1);

    /* Augmentation string, should be "zR". */
    frag_append_1_char ('z');
    frag_append_1_char ('R');
    frag_append_1_char ('\0');

    /* uleb128 Code alignment factor. */
    output_uleb128 (DEFAULT_CODE_ALIGNMENT_FACTOR);
    /* sleb128 Data alignment factor. */
    output_sleb128 (DEFAULT_DATA_ALIGNMENT_FACTOR);
    /* 1 byte Return address register. */
    frag_append_1_char (DEFAULT_RETURN_ADDRESS_REG);

    /* uleb128 Augmentation data length. */
    output_uleb128 (1);
    /* Augmentation data: 1 byte Pointer encoding for the address pointers in attached FDE. */
    frag_append_1_char (0x1B);
    
    /* Initial instructions. */
    for (i = cie->data; i != cie->end_data; i = i->next) {
        output_cfi_instruction (i);
    }

    /* Mandatory alignment. */
    frag_align (2, DW_CFA_nop, 0);
    symbol_set_value_now (end_address);

}

static void output_fde (struct fde *fde,
                        struct cie *cie,
                        struct cfi_instruction_data *first_instruction_not_in_cie) {

    struct symbol *after_length_address, *end_address;
    struct expr expr_s;

    after_length_address = symbol_temp_make ();
    end_address = symbol_temp_make ();

    /* 4 byte FDE length that does not include the length field. */
    expr_s.type = EXPR_TYPE_SUBTRACT;
    expr_s.add_symbol = end_address;
    expr_s.op_symbol = after_length_address;
    expr_s.add_number = 0;

    emit_expr (&expr_s, 4);
    symbol_set_value_now (after_length_address);

    /* 4 byte FDE CIE Pointer (offset that is subtracted from this position to get to the CIE). */
    expr_s.type = EXPR_TYPE_SUBTRACT;
    expr_s.add_symbol = after_length_address;
    expr_s.op_symbol = cie->start_address;
    expr_s.add_number = 0;

    emit_expr (&expr_s, 4);

    /* 4 byte PC Begin. */
    expr_s.type = EXPR_TYPE_SYMBOL;
    expr_s.add_symbol = fde->start_address;
    expr_s.op_symbol = NULL;
    /* No idea why should be 4 added to the field. */
    expr_s.add_number = 4;

    /* Should be PC relative for some reason. */
    fixup_new_expr (current_frag, current_frag->fixed_size, 4, &expr_s, 1, RELOC_TYPE_DEFAULT);
    frag_increase_fixed_size (4);

    /* 4 byte PC Range. */
    expr_s.type = EXPR_TYPE_SUBTRACT;
    expr_s.add_symbol = fde->end_address;
    expr_s.op_symbol = fde->start_address;
    expr_s.add_number = 0;

    emit_expr (&expr_s, 4);

    /* uleb128 Augmentation data length. (No augmentation data is used in FDE.) */
    output_uleb128 (0);

    /* Call frame instructions. */
    for (;
         first_instruction_not_in_cie;
         first_instruction_not_in_cie = first_instruction_not_in_cie->next) {
        output_cfi_instruction (first_instruction_not_in_cie);
    }

    /* Mandatory alignment. */
    frag_align (2, DW_CFA_nop, 0);
    symbol_set_value_now (end_address);

}

/* If both CIE instruction and FDE instruction are provided, they are compared.
 * If only 1 instruction is provided,
 * it is decided whether the instruction should be placed into CIE. */
static int initial_cie_instruction_equal (struct cfi_instruction_data *i,
                                          struct cfi_instruction_data *j) {

    if (j && i->instruction != j->instruction) {
        return 0;
    }

    switch (i->instruction) {

        case DW_CFA_offset:
        case DW_CFA_def_cfa:

            if (j) {
                if (i->u.ro.reg != j->u.ro.reg || i->u.ro.offset != j->u.ro.offset) {
                    return 0;
                }
            }
            break;

        case DW_CFA_restore:
        case DW_CFA_def_cfa_register:

            if (j) {
                if (i->u.reg != j->u.reg) {
                    return 0;
                }
            }
            break;

        case DW_CFA_def_cfa_offset:

            if (j) {
                if (i->u.offset != j->u.offset) {
                    return 0;
                }
            }
            break;

        default:

            return 0;
            break;

    }

    return 1;

}

static struct cie *select_cie_for_fde (struct fde *fde,
                                       struct cfi_instruction_data **first_instruction_not_in_cie_p) {

    struct cie *cie;
    struct cfi_instruction_data *i, *j;

    for (cie = all_cie; cie; cie = cie->next) {

        for (i = cie->data, j = fde->data;
             i != cie->end_data && j != NULL;
             i = i->next, j = j->next) {

            if (!initial_cie_instruction_equal (i, j)) {
                break;
            }

        }

        if (i == cie->end_data) {

            *first_instruction_not_in_cie_p = j;
            return cie;

        }

    }

    cie = xmalloc (sizeof (*cie));
    cie->next = all_cie;
    all_cie = cie;

    cie->data = fde->data;

    for (i = cie->data; i; i = i->next) {

        if (!initial_cie_instruction_equal (i, NULL)) {
            break;
        }

    }

    cie->end_data = i;
    *first_instruction_not_in_cie_p = i;

    output_cie (cie);

    return cie;

}

void cfi_finish (void) {

    section_t eh_frame_section;
    struct fde *fde;
    struct cfi_instruction_data *first_instruction_not_in_cie;

    if (warned) {
        return;
    }

    if (all_fde == NULL) {
        return;
    }

    eh_frame_section = section_set_by_name (".eh_frame");
    section_set_flags (eh_frame_section,
                       SECTION_FLAG_ALLOC
                       | SECTION_FLAG_LOAD
                       | SECTION_FLAG_DATA
                       | SECTION_FLAG_READONLY);

    for (fde = all_fde; fde; fde = fde->next) {

        struct cie *cie;

        if (fde->end_address == NULL) {

            as_error_at (NULL, 0, "open CFI at the end of file; missing .cfi_endproc directive");
            fde->end_address = fde->start_address;

        }

        cie = select_cie_for_fde (fde, &first_instruction_not_in_cie);
        output_fde (fde, cie, first_instruction_not_in_cie);

    }

}

static void handler_cfi_startproc (char **pp) {

    struct fde *fde;

    if (current_frag_chain->cfi_frag_chain_data) {

        as_error ("previous CFI entry not closed (missing .cfi_endproc)");
        ignore_rest_of_line (pp);
        return;

    }

    fde = xmalloc (sizeof (*fde));
    memset (fde, 0, sizeof (*fde));

    *last_fde_p = fde;
    last_fde_p = &(fde->next);

    current_frag_chain->cfi_frag_chain_data = xmalloc (sizeof (*(current_frag_chain->cfi_frag_chain_data)));
    current_frag_chain->cfi_frag_chain_data->fde = fde;

    fde->start_address = symbol_temp_new_now ();
    current_frag_chain->cfi_frag_chain_data->last_address = fde->start_address;

    fde->last_data_p = &(fde->data);

    /* Initial instructions default for all call frames. */
    cfi_add_CFA_def_cfa (4 /* esp */, -DEFAULT_DATA_ALIGNMENT_FACTOR);
    cfi_add_CFA_offset (DEFAULT_RETURN_ADDRESS_REG, DEFAULT_DATA_ALIGNMENT_FACTOR);

    demand_empty_rest_of_line (pp);

}

static void handler_cfi_endproc (char **pp) {

    if (current_frag_chain->cfi_frag_chain_data == NULL) {

        as_error (".cfi_endproc without corresponding .cfi_startproc");
        ignore_rest_of_line (pp);
        return;

    }

    current_frag_chain->cfi_frag_chain_data->fde->end_address = symbol_temp_new_now ();
    free (current_frag_chain->cfi_frag_chain_data);
    current_frag_chain->cfi_frag_chain_data = NULL;

    demand_empty_rest_of_line (pp);

}

static void handler_cfi_dummy (char **pp) {

    if (!warned) {

        warned = 1;
        as_warn ("this CFI pseudo-op is not supported yet, discarding all CFI (max. 1 warning per run)");

    }

    ignore_rest_of_line (pp);

}

static void cfi_parse_separator (char **pp) {

    *pp = skip_whitespace (*pp);
    if (**pp == ',') {
        ++*pp;
    } else {
        as_error ("missing separator");
    }

}

static unsigned int cfi_parse_reg (char **pp) {

    return get_result_of_absolute_expression (pp);

}

static void handler_cfi_instruction (char **pp, int instruction) {

    offset_t offset;
    unsigned int reg;

    if (symbol_get_frag (current_frag_chain->cfi_frag_chain_data->last_address) != current_frag
        || symbol_get_value (current_frag_chain->cfi_frag_chain_data->last_address) != current_frag->fixed_size) {
        cfi_add_DW_CFA_advance_loc (symbol_temp_new_now ());
    }

    switch (instruction) {

        case DW_CFA_offset:

            reg = cfi_parse_reg (pp);
            cfi_parse_separator (pp);
            offset = get_result_of_absolute_expression (pp);
            cfi_add_CFA_offset (reg, offset);
            break;

        case DW_CFA_restore:

            reg = cfi_parse_reg (pp);
            cfi_add_CFA_restore (reg);
            break;

        case DW_CFA_def_cfa:

            reg = cfi_parse_reg (pp);
            cfi_parse_separator (pp);
            offset = get_result_of_absolute_expression (pp);
            cfi_add_CFA_def_cfa (reg, offset);
            break;

        case DW_CFA_def_cfa_register:

            reg = cfi_parse_reg (pp);
            cfi_add_CFA_def_cfa_register (reg);
            break;

        case DW_CFA_def_cfa_offset:

            offset = get_result_of_absolute_expression (pp);
            cfi_add_CFA_def_cfa_offset (offset);
            break;

    }

    demand_empty_rest_of_line (pp);

}

static void handler_cfi_def_cfa (char **pp) {

    handler_cfi_instruction (pp, DW_CFA_def_cfa);

}

static void handler_cfi_def_cfa_register (char **pp) {

    handler_cfi_instruction (pp, DW_CFA_def_cfa_register);

}

static void handler_cfi_def_cfa_offset (char **pp) {

    handler_cfi_instruction (pp, DW_CFA_def_cfa_offset);

}

static void handler_cfi_offset (char **pp) {

    handler_cfi_instruction (pp, DW_CFA_offset);

}

static void handler_cfi_restore (char **pp) {

    handler_cfi_instruction (pp, DW_CFA_restore);

}

static struct pseudo_op_entry pseudo_op_table[] = {

    { "cfi_sections",          &handler_cfi_dummy  },
    { "cfi_startproc",         &handler_cfi_startproc  },
    { "cfi_endproc",           &handler_cfi_endproc  },
    { "cfi_personality",       &handler_cfi_dummy  },
    { "cfi_personality_id",    &handler_cfi_dummy  },
    { "cfi_fde_data",          &handler_cfi_dummy  },
    { "cfi_lsda",              &handler_cfi_dummy  },
    { "cfi_inline_lsda",       &handler_cfi_dummy  },
    { "cfi_def_cfa",           &handler_cfi_def_cfa  },
    { "cfi_def_cfa_register",  &handler_cfi_def_cfa_register  },
    { "cfi_def_cfa_offset",    &handler_cfi_def_cfa_offset  },
    { "cfi_adjust_cfa_offset", &handler_cfi_dummy  },
    { "cfi_offset",            &handler_cfi_offset  },
    { "cfi_val_offset",        &handler_cfi_dummy  },
    { "cfi_rel_offset",        &handler_cfi_dummy  },
    { "cfi_register",          &handler_cfi_dummy  },
    { "cfi_restore",           &handler_cfi_restore  },
    { "cfi_undefined",         &handler_cfi_dummy  },
    { "cfi_same_value",        &handler_cfi_dummy  },
    { "cfi_remember_state",    &handler_cfi_dummy  },
    { "cfi_restore_state",     &handler_cfi_dummy  },
    { "cfi_return_column",     &handler_cfi_dummy  },
    { "cfi_signal_frame",      &handler_cfi_dummy  },
    { "cfi_window_save",       &handler_cfi_dummy  },
    { "cfi_escape",            &handler_cfi_dummy  },
    { "cfi_val_encoded_addr",  &handler_cfi_dummy  },
    { 0,                       0                   }

};

struct pseudo_op_entry *cfi_get_pseudo_op_table (void) {
    return pseudo_op_table;
}
