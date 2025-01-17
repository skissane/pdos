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

#define NUM_REGS 16+1

/* we may choose to create a function that interprets tabs
   and converts them to spaces */
#define tprintf fprintf

typedef enum cc_reg_type {
    CC_REGTYPE_NOINDEX, /* cannot be used as index register */
    CC_REGTYPE_INDEX, /* can be used to index */
    CC_REGTYPE_TEMP1, /* first temporary register */
    CC_REGTYPE_TEMP2, /* second temporary register */
    CC_REGTYPE_SP, /* Stack pointer */
    CC_REGTYPE_FP /* Frame pointer */
} cc_reg_type;

struct cc_reg_info {
    const char *name;
    char in_use;
    cc_reg_type type;
};

struct cc_reg_info regs[NUM_REGS] = {
    { "0", 0, CC_REGTYPE_NOINDEX },
    { "1", 0, CC_REGTYPE_INDEX },
    { "2", 0, CC_REGTYPE_INDEX },
    { "3", 0, CC_REGTYPE_INDEX },
    { "4", 0, CC_REGTYPE_INDEX },
    { "5", 0, CC_REGTYPE_INDEX },
    { "6", 0, CC_REGTYPE_INDEX },
    { "7", 0, CC_REGTYPE_INDEX },
    { "8", 0, CC_REGTYPE_TEMP2 },
    { "9", 0, CC_REGTYPE_TEMP1 },
    { "10", 0, CC_REGTYPE_INDEX },
    { "11", 0, CC_REGTYPE_INDEX },
    { "12", 0, CC_REGTYPE_INDEX },
    { "13", 0, CC_REGTYPE_INDEX },
    { "14", 0, CC_REGTYPE_INDEX },
    { "15", 0, CC_REGTYPE_INDEX },
    { NULL,  0, 0 }
};

static size_t stack_size;

static size_t cc_i370gen_push(cc_reader *reader, const cc_expr *expr);
static void cc_i370gen_prologue(cc_reader *reader, const cc_expr *expr);
static void cc_i370gen_epilogue(cc_reader *reader, const cc_expr *expr);
static void cc_i370gen_runtime_funptr(cc_reader *reader, const cc_expr *expr);
static void cc_i370gen_return(cc_reader *reader, const cc_expr *expr);
static void cc_i370gen_top(cc_reader *reader, const cc_expr *expr);
void cc_codegen(cc_reader *reader, const cc_expr *expr);

/**
 * @brief Push an expression into the stack
 * 
 * @param reader Reader object
 * @param expr Expression to push into the stack
 * @return size_t Pushed bytes
 */
static size_t cc_i370gen_push(cc_reader *reader, const cc_expr *expr)
{
    size_t i, len;
    switch (expr->type)
    {
    case CC_EXPR_NONE:
        tprintf(reader->output, "* NONE");
        break;
    case CC_EXPR_STRING:
        tprintf(reader->output, "\tBAL\t%d,@@T%u\n", CC_REGTYPE_TEMP1, expr->id);
        tprintf(reader->output, "@@S%u\tEQU\t*\n", expr->id);
        len = strlen(expr->data.string.data) + 1;
        for (i = 0; i < len; i++)
            tprintf(reader->output, "\tDC X'%02X'\n", expr->data.string.data[i]);
        tprintf(reader->output, "*\n");
        tprintf(reader->output, "@@T%u\tDS\t0H\n", expr->id);
        tprintf(reader->output, "\tST\t9,%d(,13)\n", 88 + stack_size);
        return 4;
    case CC_EXPR_CONSTANT:
#if 0
        tprintf(reader->output, "\tBAL\t%d,@@T%u\n", CC_REGTYPE_TEMP1, expr->id);
        tprintf(reader->output, "@@S%u\tDC\tF'%lu'\n", expr->data._const.numval);
        tprintf(reader->output, "@@T%u\tDS\t0H\n", expr->id);
        tprintf(reader->output, "\tL\t9,0(,9)\n");
#else
        tprintf(reader->output, "\tL\t9,=F'%lu'\n", expr->data._const.numval);
#endif
        tprintf(reader->output, "\tST\t9,%d(,13)\n", 88 + stack_size);
        return 4;
    case CC_EXPR_VARREF:
        if (expr->data.var_ref.var->linkage == CC_LINKAGE_AUTO)
        {
            tprintf(reader->output, "\tL\t9,%d(13)\n",
                    88
                    /* + stack_size ??? */
                    + reader->curr_block->data.block.stack_depth
                    + expr->data.var_ref.var->block_offset
                    );
        }
        else
        {
            tprintf(reader->output, "\tL\t9,=A(%s)\n", expr->data.var_ref.var->name);
            tprintf(reader->output, "\tL\t9,0(9)\n");
        }
        tprintf(reader->output, "\tST\t9,%d(,13)\n", 88 + stack_size);
        return 4;
    case CC_EXPR_ADDRESSOF:
        if ((expr->data.var_ref.var->type.mode != CC_TYPE_FUNCTION)
            && (expr->data.var_ref.var->linkage == CC_LINKAGE_AUTO))
        {
            tprintf(reader->output, "\tLA\t9,%d(13)\n",
                    88
                    /* + stack_size ??? */
                    + reader->curr_block->data.block.stack_depth
                    + expr->data.var_ref.var->block_offset
                    );
        }
        else
        {
            tprintf(reader->output, "\tL\t9,=A(%s)\n", expr->data.var_ref.var->name);
        }
        tprintf(reader->output, "\tST\t9,%d(,13)\n", 88 + stack_size);
        return 4;
    default:
        printf("Unknown expr type %u for prologue\n", expr->type);
        abort();
    }
    return 0;
}

static void cc_i370gen_prologue(cc_reader *reader, const cc_expr *expr)
{
    fprintf(reader->output, "\tSTM\t(14,12),12(13)\n");
    stack_size = 0;
}

static void cc_i370gen_epilogue(cc_reader *reader, const cc_expr *expr)
{
    size_t i;
    if (expr == NULL)
    {
        tprintf(reader->output, "\tBR\t14\n");
        return;
    }

    if (expr->type == CC_EXPR_BLOCK)
    {
        tprintf(reader->output, "\tLM\t(14,12),12(13)\n");
        tprintf(reader->output, "\tBR\t14\n");
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
static void cc_i370gen_runtime_funptr(cc_reader *reader, const cc_expr *expr)
{
    printf("Dynamic call for runtime not implemented yet\n");
    abort();
}

/**
 * @brief Generate return instructions for i370
 * 
 * @param reader Reader object
 * @param expr Expression **of** the return, eg: expr->data.ret.ret_expr
 */
static void cc_i370gen_return(cc_reader *reader, const cc_expr *expr)
{
    if (expr == NULL)
        return;
    
    switch (expr->type)
    {
    case CC_EXPR_NONE:
        break;
    case CC_EXPR_CONSTANT:
        tprintf(reader->output, "\tL\t15,=F'%u\n", expr->data._const.numval);
        break;
    default:
        printf("unknown expr %u\n", expr->type);
        abort();
    }
}

static void cc_i370gen_decl(cc_reader *reader, const cc_variable *var)
{
    size_t size = cc_get_variable_size(reader, var);
    size /= 8; /* variable size given in bits - convert to bytes */
    stack_size += size;
}

static void cc_i370gen_if(cc_reader *reader, const cc_expr *cond_expr,
                          const cc_expr *body_expr)
{
    tprintf(reader->output, "*\t/* TODO: comparison */\n");
}

static void cc_i370gen_binary_op (cc_reader *reader, const cc_expr *expr)
{
    cc_i370gen_top (reader, expr->data.binary_op.left);
    tprintf (reader->output, "\tL\t9,72(,13)\n");
    tprintf (reader->output, "\tST\t15,0(,9)\n"); /* push R15 */
    tprintf (reader->output, "\tLA\t9,4(,9)\n");
    tprintf (reader->output, "\tST\t9,72(,13)\n");
    cc_i370gen_top (reader, expr->data.binary_op.right);
    tprintf (reader->output, "\tL\t9,72(,13)\n");
    tprintf (reader->output, "\tS\t9,=F'4'\n");
    tprintf (reader->output, "\tL\t8,0(,9)\n");
    tprintf (reader->output, "\tST\t9,72(,13)\n");
    
    switch (expr->type) {
        case CC_EXPR_MUL:
            tprintf (reader->output, "\tMR\t15,8\n");
            break;
        
        case CC_EXPR_PLUS:
            tprintf (reader->output, "\tAR\t15,8\n");
            break;

        case CC_EXPR_MINUS:
            tprintf (reader->output, "\tSR\t15,8\n");
            break;

#if 0
        /* not sure what instruction is required */
        case CC_EXPR_LSHIFT:
            tprintf (reader->output, "\tshl %%cl, %%eax\n");
            break;
#endif

    }
}

static void cc_i370gen_top(cc_reader *reader, const cc_expr *expr)
{
    int len;
    size_t i;
    switch (expr->type)
    {
        case CC_EXPR_NONE:
            break;
        case CC_EXPR_MUL:
        case CC_EXPR_PLUS:
        case CC_EXPR_MINUS:
        case CC_EXPR_LSHIFT:
            cc_i370gen_binary_op (reader, expr);
            break;
        case CC_EXPR_DECL:
            cc_i370gen_decl(reader, &expr->data.decl.var);
            break;
        case CC_EXPR_IF:
            cc_i370gen_if(reader, expr->data.if_else.cond_expr,
                          expr->data.if_else.body_expr);
            break;
        case CC_EXPR_RETURN:
            cc_i370gen_return(reader, expr->data.ret.ret_expr);
            break;
        case CC_EXPR_CALL:
            stack_size = 0;
            /* Push arguments onto the stack, left to right */
            /* because on the mainframe the defacto stack grows up */
            for (i = 0; i < expr->data.call.n_params; i++)
            {
                const cc_expr *param_expr = &expr->data.call.params[i];
                stack_size += cc_i370gen_push(reader, param_expr);
            }

            fprintf(reader->output, "\tL\t15,=A(");
            if (expr->data.call.callee_func)
            {
                tprintf(reader->output, "%s)\n", expr->data.call.callee_func->name);
                tprintf(reader->output, "\tBALR\t14,15\n");
            }
            else if (expr->data.call.callee)
                cc_i370gen_runtime_funptr(reader, expr->data.call.callee);
            else
            {
                printf("No function on call expression???");
                abort();
            }

            /* Pop stack */
            /* maybe 2 passes solves this problem, as the maximum parameter
               stack is normally R13-addressable? */
            /* tprintf(reader->output, "\taddl $%u, %%esp\n", stack_size); */
            stack_size = 0;
            break;
        case CC_EXPR_STRING:
#if 0
            printf("str_%u: db ", expr->id);
            for (i = 0; i < strlen(expr->data.string.data); i++)
            {
                printf("0x%x", expr->data.string.data[i]);
                if (i < strlen(expr->data.string.data) - 1)
                    printf(", ");
            }
            printf("\n");
#endif
            tprintf(reader->output, "\tBAL\t%d,@@T%u\n", CC_REGTYPE_TEMP1, expr->id);
            tprintf(reader->output, "@@S%u\tEQU\t*\n", expr->id);
            len = strlen(expr->data.string.data) + 1;
            for (i = 0; i < len; i++)
                tprintf(reader->output, "\tDC X'%02X'\n", expr->data.string.data[i]);
            tprintf(reader->output, "*\n");
            tprintf(reader->output, "@@T%u\tDS\t0H\n", expr->id);

            break;
        case CC_EXPR_CONSTANT:
#if 0
            printf("%lu", expr->data._const.numval);
#endif
            tprintf (reader->output, "\tL\t15,=F'%lu'\n", expr->data._const.numval);
            break;
        case CC_EXPR_BLOCK:
          {
            cc_expr *oldblock;
            oldblock = reader->curr_block;
            tprintf(reader->output, "* {\n");
            reader->curr_block = (cc_expr *)expr;
            for (i = 0; i < expr->data.block.n_exprs; i++)
                cc_i370gen_top(reader, &expr->data.block.exprs[i]);
            tprintf(reader->output, "* }\n");
            reader->curr_block = oldblock;
            break;
          }
        default:
            printf("Unknown expr type %u\n", expr->type);
            abort();
    }
}

static void cc_i370gen_variable(cc_reader *reader, const cc_variable *var)
{
    if (var->linkage != CC_LINKAGE_EXTERN)
    {
        if (var->type.mode == CC_TYPE_INT)
        {
            tprintf(reader->output, "\tDS 0F\n");
        }
        tprintf(reader->output, "\tENTRY %s\n", var->name);
        tprintf(reader->output, "%s\tEQU\t*\n", var->name);
        if (var->type.mode == CC_TYPE_INT)
        {
            tprintf(reader->output, "\tDC\tF'0'\n");
        }
    }
    else
        tprintf(reader->output, "* %s\n", var->name);

    switch (var->type.mode)
    {
    case CC_TYPE_FUNCTION:
        if (var->block_expr)
        {
            cc_i370gen_prologue(reader, var->block_expr);    
            cc_i370gen_top(reader, var->block_expr);
            cc_i370gen_epilogue(reader, var->block_expr);
        }
        break;
    default:
        break;
    }
}

void cc_codegen(cc_reader *reader, const cc_expr *expr)
{
    size_t i;
    assert(expr->type == CC_EXPR_BLOCK);
    reader->curr_block = (cc_expr *)expr;
    tprintf(reader->output, "* bits 32\n");
    for (i = 0; i < expr->data.block.n_vars; i++)
        cc_i370gen_variable(reader, &expr->data.block.vars[i]);
}
