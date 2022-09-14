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

static size_t stack_size;

static size_t cc_i386gen_push(cc_reader *reader, const cc_expr *expr);
static void cc_i386gen_prologue(cc_reader *reader, const cc_expr *expr);
static void cc_i386gen_epilogue(cc_reader *reader, const cc_expr *expr);
static void cc_i386gen_runtime_funptr(cc_reader *reader, const cc_expr *expr);
static void cc_i386gen_return(cc_reader *reader, const cc_expr *expr);
static void cc_i386gen_top(cc_reader *reader, const cc_expr *expr);
void cc_i386gen(cc_reader *reader, const cc_expr *expr);

/**
 * @brief Push an expression into the stack
 * 
 * @param reader Reader object
 * @param expr Expression to push into the stack
 * @return size_t Pushed bytes
 */
static size_t cc_i386gen_push(cc_reader *reader, const cc_expr *expr)
{
    switch (expr->type)
    {
    case CC_EXPR_NONE:
        fprintf(reader->output, "# none");
        break;
    case CC_EXPR_STRING:
        fprintf(reader->output, "\tpush str_%u\n", expr->id);
        return 4;
    case CC_EXPR_CONSTANT:
        fprintf(reader->output, "\tpush %lu", expr->data._const.numval);
        return 4;
    default:
        printf("Unknown expr type %u for prologue\n", expr->type);
        abort();
    }
    return 0;
}

static void cc_i386gen_prologue(cc_reader *reader, const cc_expr *expr)
{
    fprintf(reader->output, "\tmov esp, ebp\n");
    fprintf(reader->output, "\tpush ebp\n");
    stack_size = 0;
}

static void cc_i386gen_epilogue(cc_reader *reader, const cc_expr *expr)
{
    size_t i;
    if (expr == NULL)
    {
        fprintf(reader->output, "\tret\n");
        return;
    }

    if (expr->type == CC_EXPR_BLOCK)
    {
        fprintf(reader->output, "\tpop ebp\n");
        fprintf(reader->output, "\tmov ebp, esp\n");
        fprintf(reader->output, "\tret\n");
        return;
    }
    printf("Unknown expr type %u for epilogue\n", expr->type);
    abort();
}

/**
 * @brief Handle function pointers computed at runtime
 * 
 * @param reader Reader object
 * @param expr Expression containing the expressions to be computed AT runtime
 */
static void cc_i386gen_runtime_funptr(cc_reader *reader, const cc_expr *expr)
{
    printf("Dynamic call for runtime not implemented yet\n");
    abort();
}

/**
 * @brief Generate return instructions for i386
 * 
 * @param reader Reader object
 * @param expr Expression **of** the return, eg: expr->data.ret.ret_expr
 */
static void cc_i386gen_return(cc_reader *reader, const cc_expr *expr)
{
    if (expr == NULL)
        return;
    
    switch (expr->type)
    {
    case CC_EXPR_NONE:
        break;
    case CC_EXPR_CONSTANT:
        fprintf(reader->output, "\tmov eax, %u\n", expr->data._const.numval);
        break;
    default:
        printf("unknown expr %u\n", expr->type);
        abort();
    }
}

static void cc_i386gen_decl(cc_reader *reader, const cc_variable *var)
{
    size_t size = cc_get_variable_size(var);
    stack_size += size;
    fprintf(reader->output, "\tsub %u, esp #%s, %u\n", size, var->name,
            stack_size);
}

static void cc_i386gen_if(cc_reader *reader, const cc_expr *cond_expr,
                          const cc_expr *body_expr)
{
    fprintf(reader->output, "\tcmp fuck you\n");
}

static void cc_i386gen_top(cc_reader *reader, const cc_expr *expr)
{
    size_t i;
    switch (expr->type)
    {
    case CC_EXPR_NONE:
        break;
    case CC_EXPR_DECL:
        cc_i386gen_decl(reader, &expr->data.decl.var);
        break;
    case CC_EXPR_IF:
        cc_i386gen_if(reader, expr->data.if_else.cond_expr,
                      expr->data.if_else.body_expr);
        break;
    case CC_EXPR_RETURN:
        cc_i386gen_return(reader, expr->data.ret.ret_expr);
        break;
    case CC_EXPR_CALL:
        /* Push arguments to the stack */
        for (i = 0; i < expr->data.call.n_params; i++)
        {
            const cc_expr *param_expr = &expr->data.call.params[i];
            stack_size += cc_i386gen_push(reader, param_expr);
        }

        fprintf(reader->output, "\tcall ");
        if (expr->data.call.callee_func)
            fprintf(reader->output, "%s\n", expr->data.call.callee_func->name);
        else if (expr->data.call.callee)
            cc_i386gen_runtime_funptr(reader, expr->data.call.callee);
        else
        {
            printf("No function on call expression???");
            abort();
        }

        /* Pop stack */
        fprintf(reader->output, "\tadd esp, %u\n", stack_size);
        break;
    case CC_EXPR_STRING:
        printf("str_%u: db ", expr->id);
        for (i = 0; i < strlen(expr->data.string.data); i++)
        {
            printf("0x%x", expr->data.string.data[i]);
            if (i < strlen(expr->data.string.data) - 1)
                printf(", ");
        }
        printf("\n");
        break;
    case CC_EXPR_CONSTANT:
        printf("%lu", expr->data._const.numval);
        break;
    case CC_EXPR_BLOCK:
        printf("# {\n");
        for (i = 0; i < expr->data.block.n_exprs; i++)
            cc_i386gen_top(reader, &expr->data.block.exprs[i]);
        printf("# }\n");
        break;
    default:
        printf("Unknown expr type %u\n", expr->type);
        abort();
    }
}

void cc_i386gen(cc_reader *reader, const cc_expr *expr)
{
    size_t i;
    printf("+++Generated i386 assembly\n");

    fprintf(reader->output, "bits 32\n");
    for (i = 0; i < expr->data.block.n_vars; i++)
    {
        const cc_variable *var = &expr->data.block.vars[i];
        if (var->type.mode == CC_TYPE_FUNCTION && var->block_expr)
        {
            fprintf(reader->output, "%s:\n", var->name);
            cc_i386gen_prologue(reader, var->block_expr);    
            cc_i386gen_top(reader, var->block_expr);
            cc_i386gen_epilogue(reader, var->block_expr);
        }
    }
}
