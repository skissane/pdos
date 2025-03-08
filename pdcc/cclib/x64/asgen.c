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
#include <ctype.h>
#include <assert.h>
#include "cclib.h"
#include "support.h"
#include "xmalloc.h"

#define NUM_REGS 9
typedef enum cc_reg_type {
    CC_REGTYPE_XL_XH, /* Has XL/XH counterparts */
    CC_REGTYPE_SP, /* Stack pointer */
    CC_REGTYPE_FP /* Frame pointer */
} cc_reg_type;

struct cc_reg_info {
    const char *name;
    char in_use;
    cc_reg_type type;
};

struct cc_reg_info regs[NUM_REGS] = {
    { "eax", 0, CC_REGTYPE_XL_XH },
    { "ebx", 0, CC_REGTYPE_XL_XH },
    { "ecx", 0, CC_REGTYPE_XL_XH },
    { "edx", 0, CC_REGTYPE_XL_XH },
    { "esi", 0, CC_REGTYPE_XL_XH },
    { "edi", 0, CC_REGTYPE_XL_XH },
    { "ebp", 0, CC_REGTYPE_XL_XH },
    { "esp", 0, CC_REGTYPE_XL_XH },
    { NULL,  0, 0 }
};

static unsigned long stack_size;

static size_t cc_x64gen_push (cc_reader *reader, const cc_expr *expr);
static void cc_x64gen_prologue (cc_reader *reader, const cc_variable *var);
static void cc_x64gen_epilogue (cc_reader *reader, const cc_expr *expr);
static void cc_x64gen_runtime_funptr (cc_reader *reader, const cc_expr *expr);
static void cc_x64gen_return (cc_reader *reader, const cc_expr *expr);
static void cc_x64gen_top (cc_reader *reader, const cc_expr *expr);
void cc_codegen (cc_reader *reader, const cc_expr *expr);

/**
 * @brief Push an expression into the stack
 * 
 * @param reader Reader object
 * @param expr Expression to push into the stack
 * @return size_t Pushed bytes
 */
static size_t cc_x64gen_push (cc_reader *reader, const cc_expr *expr)
{
    size_t i, len;
    switch (expr->type)
    {
    case CC_EXPR_NONE:
        fprintf(reader->output, ";; none");
        break;
    case CC_EXPR_STRING:
        fprintf(reader->output, "\tpushl $S%u\n", expr->id);
        fprintf(reader->output, "\tjmp S%u_end\n", expr->id);
        fprintf(reader->output, "S%u:\n", expr->id);
        len = strlen(expr->data.string.data);
        for (i = 0; i < len; i++)
            fprintf(reader->output, ".byte 0x%x\n", expr->data.string.data[i]);
        fprintf(reader->output, ".byte 0x%x\n", expr->data.string.data[i]);
        fprintf(reader->output, "\n");
        fprintf(reader->output, "S%u_end:\n", expr->id);
        return 4;
    case CC_EXPR_CONSTANT:
        fprintf(reader->output, "\tpushl $%lu\n", expr->data._const.numval);
        return 4;
#if 0
    case CC_EXPR_VARREF:
        if (expr->data.var_ref.var->linkage == CC_LINKAGE_AUTO)
        {
            size_t size;
            size = cc_get_variable_size(reader, expr->data.var_ref.var) / 8;
            fprintf(reader->output, "\tpushl -%d(%%ebp)\n",
                    reader->curr_block->data.block.stack_depth
                    + expr->data.var_ref.var->block_offset
                    + size);
        }
        else
        {
            fprintf(reader->output, "\tpushl _%s\n", expr->data.var_ref.var->name);
        }
        return 4;
    case CC_EXPR_ADDRESSOF:
        if ((expr->data.var_ref.var->type.mode != CC_TYPE_FUNCTION)
            && (expr->data.var_ref.var->linkage == CC_LINKAGE_AUTO))
        {
            size_t size;
            size = cc_get_variable_size(reader, expr->data.var_ref.var) / 8;
            fprintf(reader->output, "\tleal -%d(%%ebp), %%eax\n"
                                    "\tpushl %%eax\n",
                    reader->curr_block->data.block.stack_depth
                    + expr->data.var_ref.var->block_offset
                    + size);
        }
        else
        {
            fprintf(reader->output, "\tpushl $_%s\n", expr->data.var_ref.var->name);
        }
        return 4;
#endif
    default:
        printf("Unknown expr type %u for prologue\n", expr->type);
        abort();
    }
    return 0;
}

static void cc_x64gen_prologue (cc_reader *reader, const cc_variable *var)
{
    switch (var->type.data.f.n_params) {
        default: fprintf (reader->output, "\tmov 32[rsp], r9\n");
        case 3: fprintf (reader->output, "\tmov 24[rsp], r8\n");
        case 2: fprintf (reader->output, "\tmov 16[rsp], rdx\n");
        case 1: fprintf (reader->output, "\tmov 8[rsp], rcx\n");
        case 0: break;
    }
    fprintf(reader->output, "\tpush rbp\n");
    fprintf(reader->output, "\tmov rbp, rsp\n");
    stack_size = 0;
}

static void cc_x64gen_epilogue (cc_reader *reader, const cc_expr *expr)
{
    if (expr == NULL) {
        fprintf(reader->output, "\tret\n");
        return;
    }

    if (expr->type == CC_EXPR_BLOCK) {
        fprintf (reader->output, "\tmov rsp, rbp\n");
        fprintf (reader->output, "\tpop rbp\n");
        fprintf (reader->output, "\tret\n");
        return;
    }
    
    printf ("Unknown expr type %u for epilogue\n", expr->type);
    abort ();
}

/**
 * @brief Handle function pointers computed at runtime
 * 
 * @param reader Reader object
 * @param expr Expression containing the expressions to be computed AT runtime
 */
static void cc_x64gen_runtime_funptr (cc_reader *reader, const cc_expr *expr)
{
    printf("Dynamic call for runtime not implemented yet\n");
    abort();
}

/**
 * @brief Generate return instructions for x64
 * 
 * @param reader Reader object
 * @param expr Expression **of** the return, eg: expr->data.ret.ret_expr
 */
static void cc_x64gen_return (cc_reader *reader, const cc_expr *expr)
{
    if (expr == NULL) return;
    
    switch (expr->type) {
        case CC_EXPR_NONE:
            break;
        
        case CC_EXPR_CONSTANT:
            cc_x64gen_top (reader, expr);
            break;
        
        default:
            printf ("unknown expr %u\n", expr->type);
            abort ();
    }
}

static void cc_x64gen_decl (cc_reader *reader, const cc_variable *var)
{
    size_t size = cc_get_variable_size(reader, var);
    size /= 8; /* variable size given in bits - convert to bytes */
    stack_size += size;
    fprintf(reader->output, "\tsubl $%u, %%esp #_%s, %lu\n", size, var->name,
            stack_size);
}

static void cc_x64gen_if (cc_reader *reader, const cc_expr *cond_expr,
                           const cc_expr *body_expr)
{
    fprintf(reader->output, "\t/* TODO: comparison */\n");
}

static void cc_x64gen_binary_op (cc_reader *reader, const cc_expr *expr)
{
    cc_x64gen_top (reader, expr->data.binary_op.left);
    fprintf (reader->output, "\tpush rax\n");
    cc_x64gen_top (reader, expr->data.binary_op.right);
    fprintf (reader->output, "\tmov ecx, eax\n");
    fprintf (reader->output, "\tpop rax\n");
    
    switch (expr->type) {
        case CC_EXPR_MUL:
            fprintf (reader->output, "\tmul ecx\n");
            break;
        
        case CC_EXPR_PLUS:
            fprintf (reader->output, "\tadd eax, ecx\n");
            break;

        case CC_EXPR_MINUS:
            fprintf (reader->output, "\tsub eax, ecx\n");
            break;

        case CC_EXPR_LSHIFT:
            fprintf (reader->output, "\tshl eax, cl\n");
            break;
    }
}

static void cc_x64gen_top (cc_reader *reader, const cc_expr *expr)
{
    size_t i;
    switch (expr->type) {
        case CC_EXPR_NONE:
            break;
        
        case CC_EXPR_MUL:
        case CC_EXPR_PLUS:
        case CC_EXPR_MINUS:
        case CC_EXPR_LSHIFT:
            cc_x64gen_binary_op (reader, expr);
            break;
        
        case CC_EXPR_DECL:
            cc_x64gen_decl (reader, expr->data.decl.var);
            break;
        
        case CC_EXPR_IF:
            cc_x64gen_if (reader, expr->data.if_else.cond_expr,
                          expr->data.if_else.body_expr);
            break;
        
        case CC_EXPR_RETURN:
            cc_x64gen_return (reader, expr->data.ret.ret_expr);
            break;
        
        case CC_EXPR_CALL:
            stack_size = 0;
            /* Push arguments onto the stack, right to left */
            i = expr->data.call.n_params;
            while (i-- > 0)
            {
                const cc_expr *param_expr = &expr->data.call.params[i];
                cc_x64gen_top (reader, param_expr);
                switch (i) {
                    case 0:
                        fprintf (reader->output, "\tmov rcx, rax\n");
                        break;

                    case 1:
                        fprintf (reader->output, "\tmov rdx, rax\n");
                        break;

                    case 2:
                        fprintf (reader->output, "\tmov r8, rax\n");
                        break;
                    
                    case 3:
                        fprintf (reader->output, "\tmov r9, rax\n");
                        break;

                    default:
                        fprintf (reader->output, "\tpush rax\n");
                        stack_size += 8;
                        break;
                }
            }
            /* Caller must always allocate enough shadow space to store 4 register parameters
             * even if not that many parameters are used.
             */
            fprintf (reader->output, "\tsub rsp, 32\n");
            stack_size += 32;

            fprintf(reader->output, "\tcall ");
            if (expr->data.call.callee_func)
                fprintf (reader->output, "%s\n", expr->data.call.callee_func->name);
            else if (expr->data.call.callee)
                cc_x64gen_runtime_funptr (reader, expr->data.call.callee);
            else
            {
                printf("No function on call expression???");
                abort();
            }
            
            /* Pop stack */
            fprintf (reader->output, "\tadd rsp, %lu\n", stack_size);
            stack_size = 0;
            break;
        
        case CC_EXPR_STRING:
            fprintf(reader->output, "\tlea rax, S%u[rip]\n", expr->id);
            fprintf(reader->output, "\tjmp S%u_end\n", expr->id);
            fprintf(reader->output, "S%u:\n", expr->id);
            fprintf(reader->output, ".asciz \"%s\"\n", expr->data.string.data);
            fprintf(reader->output, "\n");
            fprintf(reader->output, "S%u_end:\n", expr->id);
            break;
        
        case CC_EXPR_CONSTANT:
            fprintf (reader->output, "\tmov eax, %lu\n", expr->data._const.numval);
            break;

        case CC_EXPR_VARREF:
            if (TREE_TYPE (expr->data.var_ref.var) == CC_TREE_PARAM) {
                const cc_param *param = expr->data.var_ref.var;
                fprintf (reader->output, "\tmov rax, %u[rbp]\n", 16 + param->index * 8);
            } else {
                printf("Unknown varref tree type %u\n", TREE_TYPE (expr->data.var_ref.var));
                abort();
            }
            break;
        
        case CC_EXPR_BLOCK:
            {
                cc_expr *oldblock;
                oldblock = reader->curr_block;
                fprintf (reader->output, "# {\n");
                reader->curr_block = (cc_expr *)expr;
                for (i = 0; i < expr->data.block.n_exprs; i++) {
                    cc_x64gen_top (reader, &expr->data.block.exprs[i]);
                }
                fprintf (reader->output, "# }\n");
                reader->curr_block = oldblock;
            }
            break;
        
        default:
            printf("Unknown expr type %u\n", expr->type);
            abort();
    }
}

static void cc_x64gen_variable (cc_reader *reader, const cc_variable *var)
{
    if (var->linkage != CC_LINKAGE_EXTERN) {
        if (var->type.mode == CC_TYPE_INT) {
            fprintf (reader->output, ".data\n");
            fprintf (reader->output, ".balign 4\n");
        }
        fprintf (reader->output, ".global %s\n", var->name);
        fprintf (reader->output, "%s:\n", var->name);
        if (var->type.mode == CC_TYPE_INT) {
            fprintf (reader->output, ".long 0\n");
            fprintf (reader->output, ".text\n");
        }
    } else fprintf (reader->output, "# %s\n", var->name);

    switch (var->type.mode) {
        case CC_TYPE_FUNCTION:
            if (var->block_expr) {
                cc_x64gen_prologue (reader, var);    
                cc_x64gen_top (reader, var->block_expr);
                cc_x64gen_epilogue (reader, var->block_expr);
            }
            break;
        default:
            break;
    }
}

void cc_codegen (cc_reader *reader, const cc_expr *expr)
{
    size_t i;
    assert (expr->type == CC_EXPR_BLOCK);
    reader->curr_block = (cc_expr *)expr;
    fprintf (reader->output, "# bits 64\n");
    fprintf (reader->output, ".intel_syntax noprefix\n");
    for (i = 0; i < expr->data.block.n_vars; i++)
        cc_x64gen_variable (reader, expr->data.block.vars[i]);
}
