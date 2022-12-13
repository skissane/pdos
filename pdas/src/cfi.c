/******************************************************************************
 * @file            process.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include    <stdio.h>

#include    "as.h"
#include    "cfi.h"

static int warned = 0;

static void handler_cfi_dummy (char **pp) {

    if (!warned) {

        warned = 1;
        as_warn ("CFI not supported yet, CFI pseudo-ops ignored (max. 1 warning per run)");

    }

    ignore_rest_of_line (pp);

}

static struct pseudo_op_entry pseudo_op_table[] = {

    { "cfi_sections",          &handler_cfi_dummy  },
    { "cfi_startproc",         &handler_cfi_dummy  },
    { "cfi_endproc",           &handler_cfi_dummy  },
    { "cfi_personality",       &handler_cfi_dummy  },
    { "cfi_personality_id",    &handler_cfi_dummy  },
    { "cfi_fde_data",          &handler_cfi_dummy  },
    { "cfi_lsda",              &handler_cfi_dummy  },
    { "cfi_inline_lsda",       &handler_cfi_dummy  },
    { "cfi_def_cfa",           &handler_cfi_dummy  },
    { "cfi_def_cfa_register",  &handler_cfi_dummy  },
    { "cfi_def_cfa_offset",    &handler_cfi_dummy  },
    { "cfi_adjust_cfa_offset", &handler_cfi_dummy  },
    { "cfi_offset",            &handler_cfi_dummy  },
    { "cfi_val_offset",        &handler_cfi_dummy  },
    { "cfi_rel_offset",        &handler_cfi_dummy  },
    { "cfi_register",          &handler_cfi_dummy  },
    { "cfi_restore",           &handler_cfi_dummy  },
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
