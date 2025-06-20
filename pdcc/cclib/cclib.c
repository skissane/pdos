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
#include "diag.h"
#include "lexer.h"
#include "support.h"
#include "xmalloc.h"

void cc_codegen (cc_reader *reader, const cc_expr *expr);

/* Some memory needs to be allocated until the end of program
 * and then all of it can be freed together instead of individually.
 */
#define pxmalloc xmalloc

typedef struct cc_id_decl {
    struct cc_id_decl *prev;
    const char *name;
    cc_tree decl;
} cc_id_decl;

typedef struct cc_scope {
    struct cc_scope *outer;
    struct cc_id_decl *id_decl;
} cc_scope;

static cc_tree entering_function;
static cc_scope *cur_scope;

static void add_to_scope (const char *name, cc_tree decl)
{
    cc_id_decl *id_decl = xmalloc (sizeof *id_decl);

    id_decl->prev = cur_scope->id_decl;
    cur_scope->id_decl = id_decl;
    id_decl->name = name;
    id_decl->decl = decl;
}

static void enter_scope (void)
{
    cc_scope *scope = xmalloc (sizeof *scope);

    scope->outer = cur_scope;
    cur_scope = scope;
    scope->id_decl = NULL;

    if (entering_function) {
        size_t i;
        cc_expr *fn = (void *)entering_function;

        for (i = 0; i < fn->data.decl.var->type.data.f.n_params; i++) {
            add_to_scope (fn->data.decl.var->type.data.f.params[i].name,
                          &fn->data.decl.var->type.data.f.params[i]);
        }

        entering_function = NULL;
    }
}

static void exit_scope (void)
{
    cc_id_decl *id_decl;
    cc_scope *scope = cur_scope;

    cur_scope = cur_scope->outer;
    for (id_decl = scope->id_decl; id_decl; id_decl = scope->id_decl) {
        scope->id_decl = id_decl->prev;
        free (id_decl);
    }
    free (scope);
}

static void search_scope (cc_token *tok)
{
    cc_scope *scope;
    cc_id_decl *id_decl;

    tok->id_kind = CC_ID_ID;

    for (scope = cur_scope; scope; scope = scope->outer) {
        for (id_decl = scope->id_decl; id_decl; id_decl = id_decl->prev) {
            if (!strcmp (tok->data.name, id_decl->name)) {
                tok->decl = id_decl->decl;
                if (TREE_TYPE (tok->decl) == CC_TREE_TYPE) {
                    tok->id_kind = CC_ID_TYPE;
                }
                return;
            }
        }
    }
}

#define cc_parse_expression ccpexp

/** Stores all the scoped blocks for lookup of symbols */
static cc_expr *g_scoped_blocks = NULL;
static size_t g_n_scoped_blocks = 0;
static void cc_add_scoped_block(cc_expr block)
{
    g_scoped_blocks = xrealloc(g_scoped_blocks, (g_n_scoped_blocks + 1)
                               * sizeof(cc_expr));
    g_scoped_blocks[g_n_scoped_blocks++] = block;
}

static cc_expr *cc_get_scoped_block(size_t id)
{
    size_t i;
    for (i = 0; i < g_n_scoped_blocks; i++)
        if (g_scoped_blocks[i].id == id)
            return &g_scoped_blocks[i];
    return NULL;
}

static void cc_dump_type(cc_reader *reader, const cc_type *type);
static void cc_dump_variable(cc_reader *reader, const cc_variable *var);
static void cc_dump_expr(cc_reader *reader, const cc_expr *expr);
static void cc_resolve_symbol(cc_reader *reader, cc_token *tok);
static cc_expr cc_parse_assignment_expr (cc_reader *reader);
cc_expr cc_parse_expression (cc_reader *reader);
static cc_expr cc_parse_compound_statement (cc_reader *reader);
static cc_expr cc_parse_declaration_or_fndef (cc_reader *reader, int fndef_allowed);

static void cc_dump_member(cc_reader *reader, const cc_member *member)
{
    cc_dump_type(reader, &member->type);
    printf("%s;\n", member->name);
}

static void cc_dump_type(cc_reader *reader, const cc_type *type)
{
    size_t i;
    for (i = 0; i < type->ptr_depth; i++)
    {
        if (type->ptr[i].flags & CC_PTRINFO_CONST)
            printf("const ");
        if (type->ptr[i].flags & CC_PTRINFO_VOLATILE)
            printf("volatile ");
        printf("*");
    }
    
    switch (type->mode)
    {
    case CC_TYPE_STRUCT:
        printf("struct %s {\n", type->name ? type->name : "<anonymous>");
        for (i = 0; i < type->data.s_or_u.n_members; i++)
            cc_dump_member(reader, &type->data.s_or_u.members[i]);
        printf("} ");
        break;
    case CC_TYPE_CHAR:
        printf("char ");
        break;
    case CC_TYPE_INT:
        printf("int ");
        break;
    case CC_TYPE_FLOAT:
        printf("float ");
        break;
    case CC_TYPE_LDOUBLE:
        printf("long double ");
        break;
    case CC_TYPE_LLONG:
        printf("long long ");
        break;
    case CC_TYPE_FUNCTION:
        cc_dump_type(reader, type->data.f.return_type);
        printf("<%s> ", type->name == NULL ? "anonymous" : type->name);
        break;
    default:
        printf("Unknown mode %x for type\n", type->mode);
        abort();
    }
}

static void cc_dump_variable(cc_reader *reader, const cc_variable *var)
{
    switch (var->linkage)
    {
    case CC_LINKAGE_AUTO:
        printf("auto ");
        break;
    case CC_LINKAGE_EXTERN:
        printf("extern ");
        break;
    case CC_LINKAGE_INLINE:
        printf("inline ");
        break;
    case CC_LINKAGE_THREAD_LOCAL:
        printf("thread_local ");
        break;
    case CC_LINKAGE_STATIC:
        printf("static ");
        break;
    case CC_LINKAGE_COROUTINE:
        printf("coroutine ");
        break;
    default:
        printf("Unknown linkage %x\n", var->linkage);
        abort();
    }
    cc_dump_type(reader, &var->type);
    if (var->type.mode == CC_TYPE_FUNCTION)
    {
        size_t i;
        printf("%s (", var->name);
        for (i = 0; i < var->type.data.f.n_params; i++)
        {
            const cc_param *param = &var->type.data.f.params[i];
            cc_dump_type(reader, &param->type);
            printf("%s, ", param->name);
        }

        if (var->type.data.f.variadic)
            printf("...");
        printf(")");

        if (var->block_expr)
        {
            printf("(Block of %s)::", var->name);
            cc_dump_expr(reader, var->block_expr);
        }
    }
    printf("\n");
}

static void cc_dump_expr(cc_reader *reader, const cc_expr *expr)
{
    size_t i;
    switch (expr->type)
    {
    case CC_EXPR_NONE:
        printf("/* none */");
        break;
    case CC_EXPR_CAST:
        printf("cast(");
        cc_dump_type(reader, &expr->data.cast.type);
        printf(")");
        break;
    case CC_EXPR_DECL:
        printf("declare(");
        cc_dump_variable(reader, expr->data.decl.var);
        printf(")");
        break;
    case CC_EXPR_VARREF:
        printf("var_ref(");
        cc_dump_variable(reader, expr->data.var_ref.var);
        printf(")");
        break;
    case CC_EXPR_IF:
        printf("if (");
        cc_dump_expr(reader, expr->data.if_else.cond_expr);
        printf(") \n");
        cc_dump_expr(reader, expr->data.if_else.body_expr);
        break;
    case CC_EXPR_RETURN:
        printf("return ");
        cc_dump_expr(reader, expr->data.ret.ret_expr);
        break;
    case CC_EXPR_CALL:
        printf("call ");
        if (expr->data.call.callee_func)
            cc_dump_variable(reader, expr->data.call.callee_func);
        else if (expr->data.call.callee)
            cc_dump_expr(reader, expr->data.call.callee);
        else
        {
            printf("No function on call expression???");
            abort();
        }

        printf(" (");
        for (i = 0; i < expr->data.call.n_params; i++)
        {
            const cc_expr *param_expr = &expr->data.call.params[i];
            cc_dump_expr(reader, param_expr);
        }
        printf(")");
        break;
    case CC_EXPR_STRING:
        printf("{");
        for (i = 0; i < strlen(expr->data.string.data); i++)
            printf("%u, ", expr->data.string.data[i]);
        printf("}");
        break;
    case CC_EXPR_CONSTANT:
        printf("(%lu)", expr->data._const.numval);
        break;
    case CC_EXPR_BLOCK:
        printf("(Start-Block) {\n");

        printf("types=%u\n", expr->data.block.n_types);
        for (i = 0; i < expr->data.block.n_types; i++)
            cc_dump_type(reader, expr->data.block.types[i]);

        printf("vars=%u\n", expr->data.block.n_vars);
        for (i = 0; i < expr->data.block.n_vars; i++)
            cc_dump_variable(reader, expr->data.block.vars[i]);

        for (i = 0; i < expr->data.block.n_exprs; i++)
            cc_dump_expr(reader, &expr->data.block.exprs[i]);
        printf("} (End-Block)\n");
        break;
    default:
        printf("Unknown expr type %u\n", expr->type);
        break;
    }
}

static void ignore_arrays (cc_reader *reader)
{
    while (reader->curr_token->type == CC_TOKEN_LBRACKET) {
        while (reader->curr_token->type != CC_TOKEN_RBRACKET) {
            cc_consume_token(reader);
        }
        cc_consume_token(reader);
    }
}

/**
 * @brief Adds a type to the current scope
 * 
 * @param reader Reader object
 * @return cc_type* Type allocated (not owned)
 */
static cc_type *cc_add_type(cc_reader *reader)
{
    reader->curr_block->data.block.types
        = xrealloc(reader->curr_block->data.block.types,
                    (reader->curr_block->data.block.n_types + 1)
                    * sizeof(*reader->curr_block->data.block.types));
    reader->curr_block->data.block.types[reader->curr_block->data.block.n_types] = pxmalloc (sizeof (cc_type));
    return reader->curr_block->data.block.types[reader->curr_block->data.block.n_types++];
}

/**
 * @brief Adds a variable to the current scope
 * 
 * @param reader Reader object
 * @return cc_variable* Variable allocated (not owned)
 */
static cc_variable *cc_add_variable(cc_reader *reader)
{
    reader->curr_block->data.block.vars
        = xrealloc(reader->curr_block->data.block.vars,
                    (reader->curr_block->data.block.n_vars + 1)
                    * sizeof(*reader->curr_block->data.block.vars));
    reader->curr_block->data.block.vars[reader->curr_block->data.block.n_vars] = pxmalloc (sizeof (cc_variable));
    return reader->curr_block->data.block.vars[reader->curr_block->data.block.n_vars++];
}

/**
 * @brief Parses an expression of the form:
 * <type> <name> [<array_expr>]
 * 
 * @param reader 
 * @return cc_member 
 */
cc_member cc_parse_member(cc_reader *reader)
{
    cc_member member = {0};
    member.type = cc_parse_type(reader);
    if (reader->curr_token->type != CC_TOKEN_IDENT)
        cc_report(reader, CC_DL_ERROR, "Expected identifier after type on "
                                       "member declaration");
    TOKEN_GETSTR(member.name, reader->curr_token->data.name);
    cc_consume_token(reader);
    ignore_arrays (reader);
    return member;
}

/**
 * @brief Parses const-volatile qualifiers, stops at first non-cv
 * 
 * @param reader Reader object
 * @param type Type to parse cv from
 */
static void cc_parse_cv(cc_reader *reader, cc_type *type)
{
    while (1)
    {
        if (reader->curr_token->type == CC_TOKEN_KW_CONST)
        {
            if ((type->ptr[type->ptr_depth].flags & CC_PTRINFO_CONST) != 0)
                cc_report(reader, CC_DL_WARNING, "Redundant qualifier");
            type->ptr[type->ptr_depth].flags |= CC_PTRINFO_CONST;
        }
        else if (reader->curr_token->type == CC_TOKEN_KW_VOLATILE)
        {
            if ((type->ptr[type->ptr_depth].flags & CC_PTRINFO_VOLATILE) != 0)
                cc_report(reader, CC_DL_WARNING, "Redundant qualifier");
            type->ptr[type->ptr_depth].flags |= CC_PTRINFO_VOLATILE;
        }
        else
            break;
        cc_consume_token(reader);
    }
}

static void cc_parse_struct_member_decl(cc_reader *reader, cc_type *type)
{
    cc_type *st_type = cc_add_type(reader);
    *st_type = *type;
    assert(reader->curr_token->type == CC_TOKEN_LBRACE);
    cc_consume_token(reader); /* Skip opening brace */
    while (reader->curr_token->type != CC_TOKEN_RBRACE)
    {
        if (type->mode == CC_TYPE_ENUM) {
            cc_consume_token(reader);
            continue;
        }
        /* <member-decl> <,> <member-decl> <,> <...> <;> */
        do
        {
            cc_member *member;
            if (reader->curr_token->type == CC_TOKEN_SEMICOLON)
                break;
            /* Add members of struct recursively, don't forget to
             * reallocate properly! */
            st_type->data.s_or_u.members
                = xrealloc(st_type->data.s_or_u.members,
                        (st_type->data.s_or_u.n_members + 1)
                        * sizeof(cc_member));
            member = &st_type->data.s_or_u.members[
                        st_type->data.s_or_u.n_members++];
            *member = cc_parse_member(reader);
            if (reader->curr_token->type != CC_TOKEN_COMMA)
                break;
            if (reader->curr_token->type != CC_TOKEN_IDENT)
                cc_report(reader, CC_DL_ERROR, "Expected identifier or "
                    "semicolon after comma but got \"%s\" instead",
                    g_token_info[reader->curr_block->type].name);
        } while (reader->curr_token->type == CC_TOKEN_COMMA);
        if (reader->curr_token->type != CC_TOKEN_SEMICOLON)
            cc_report(reader, CC_DL_ERROR, "Expected semicolon after member "
                "declaration but got \"%s\" instead",
                g_token_info[reader->curr_block->type].name);
        cc_consume_token(reader);
    }
    if (reader->curr_token->type != CC_TOKEN_RBRACE)
        cc_report(reader, CC_DL_ERROR, "Missing \"}\" terminating brace after "
            "member declarations");
    cc_consume_token(reader); /* Skip closing brace */
    *type = *st_type;
}

cc_type cc_parse_type(cc_reader *reader)
{
    cc_type type = {CC_TREE_TYPE};
    cc_parse_cv(reader, &type);
    type.ptr_depth++;

    if (reader->curr_token->type == CC_TOKEN_KW_UNION
     || reader->curr_token->type == CC_TOKEN_KW_STRUCT
     || reader->curr_token->type == CC_TOKEN_KW_ENUM)
    {
        if (reader->curr_token->type == CC_TOKEN_KW_UNION)
            type.mode = CC_TYPE_UNION;
        else if (reader->curr_token->type == CC_TOKEN_KW_STRUCT)
            type.mode = CC_TYPE_STRUCT;
        else if (reader->curr_token->type == CC_TOKEN_KW_ENUM)
            type.mode = CC_TYPE_ENUM;
        cc_consume_token(reader);

        /* Optional identifier after enum/union/struct */
        if (reader->curr_token->type == CC_TOKEN_IDENT)
        {
            type.name = reader->curr_token->data.name;
            cc_consume_token(reader);
        } else {
            type.name = "<anonymous>";
        }

        /* Syntax of the form:
         * <struct|union|enum> <identifier> { <members> } <var-expr>; */
        if (reader->curr_token->type == CC_TOKEN_LBRACE)
            cc_parse_struct_member_decl(reader, &type);
    } else {
        if (reader->curr_token->type == CC_TOKEN_IDENT) {
            search_scope (reader->curr_token);
        }

        if (reader->curr_token->id_kind == CC_ID_TYPE) {
            /* Oversimplified for now, discards const and volatile. */
            type = *(cc_type *)reader->curr_token->decl;
            cc_consume_token (reader);
        }
        
        for ( ; reader->curr_token->type != CC_TOKEN_IDENT; reader->curr_token++)
        {
            if (reader->curr_token->type == CC_TOKEN_KW_VOID) {
                type.mode = CC_TYPE_VOID;
            } else if (reader->curr_token->type == CC_TOKEN_KW_INT) {
                if (type.mode != CC_TYPE_CHAR
                    && type.mode != CC_TYPE_SHORT
                    && type.mode != CC_TYPE_LONG) {
                    type.mode = CC_TYPE_INT;
                }
            } else if (reader->curr_token->type == CC_TOKEN_KW_SHORT) {
                type.mode = CC_TYPE_SHORT;
            } else if (reader->curr_token->type == CC_TOKEN_KW_CHAR) {
                type.mode = CC_TYPE_CHAR;
            } else if (reader->curr_token->type == CC_TOKEN_KW_LONG) {
                type.mode = CC_TYPE_LONG;
            } else if (reader->curr_token->type == CC_TOKEN_KW_FLOAT) {
                type.mode = CC_TYPE_FLOAT;
            } else if (reader->curr_token->type == CC_TOKEN_KW_DOUBLE) {
                type.mode = CC_TYPE_DOUBLE;
            } else if (reader->curr_token->type == CC_TOKEN_KW_SIGNED) {
                type.data.sign = 1;
            } else if (reader->curr_token->type == CC_TOKEN_KW_UNSIGNED) {
                if (type.mode == CC_TYPE_FLOAT || type.mode == CC_TYPE_DOUBLE
                    || type.mode == CC_TYPE_LDOUBLE || type.mode == CC_TYPE_UNION
                    || type.mode == CC_TYPE_STRUCT || type.mode == CC_TYPE_ENUM)
                    cc_report(reader, CC_DL_ERROR, "This combination of types"
                                                   "do not support sign "
                                                   "qualifiers");
                
                if (type.mode != CC_TYPE_CHAR && type.mode != CC_TYPE_SHORT
                    && type.mode != CC_TYPE_LONG && type.mode != CC_TYPE_LLONG
                    && type.mode != CC_TYPE_INT) {
                    type.mode = CC_TYPE_INT;
                }
                type.data.sign = 0;
            } else break;
        }
    }

    /* Pointers of the form <*> <cv> may come now */
    while(1)
    {
        if (reader->curr_token->type == CC_TOKEN_ASTERISK)
        {
            if (type.ptr_depth >= MAX_PTR_DEPTH)
                cc_report(reader, CC_DL_ERROR, "Pointer is too deep!");
#ifdef __CC64__
#else
            cc_consume_token(reader);
#endif
            cc_parse_cv(reader, &type);
            type.ptr_depth++;
        }
        else
            break;
    }
    return type;
}

/**
 * @brief Obtain a label id from a reader
 * 
 * @param reader Reader object
 * @return size_t Label id
 */
size_t cc_get_unique_id(cc_reader *reader)
{
    return ++reader->id;
}

size_t cc_get_type_size(cc_reader *reader, const cc_type *type)
{
    if (type->ptr_depth)
        return reader->ptr_bits;

    switch (type->mode)
    {
    case CC_TYPE_CHAR:
        return reader->char_bits;
    case CC_TYPE_DOUBLE:
        return reader->long_bits;
    case CC_TYPE_LDOUBLE:
        return reader->llong_bits;
    case CC_TYPE_FLOAT:
        return reader->int_bits;
    case CC_TYPE_INT:
        return reader->int_bits;
    case CC_TYPE_LLONG:
        return reader->llong_bits;
    case CC_TYPE_LONG:
        return reader->long_bits;
    case CC_TYPE_SHORT:
        return reader->short_bits;
    case CC_TYPE_VOID:
        return 0;
    case CC_TYPE_STRUCT: {
        size_t i, total_size = 0;
        for (i = 0; i < type->data.s_or_u.n_members; i++)
        {
            cc_member *member = &type->data.s_or_u.members[i];
            total_size += cc_get_type_size(reader, &member->type);
        }
        return total_size;
    }
    case CC_TYPE_UNION: {
        size_t i, max_size = 0;
        for (i = 0; i < type->data.s_or_u.n_members; i++)
        {
            cc_member *member = &type->data.s_or_u.members[i];
            size_t size = cc_get_type_size(reader, &member->type);
            if (size > max_size)
                max_size = size;
        }
        return max_size;
    }
    case CC_TYPE_ENUM:
        printf("sizeof struct (enum?) not supported");
        abort();
    case CC_TYPE_FUNCTION:
        printf("sizeof a function instead of a func ptr");
        abort();
    }
    
    printf("Unknown type mode %x", type->mode);
    abort();
}

size_t cc_get_variable_size(cc_reader *reader, const cc_variable *var)
{
    size_t len = cc_get_type_size(reader, &var->type);
    return len;
}

/**
 * @brief Parses a parameter in a function declaration, a expression of the type
 * <type> <ident> (,)
 * 
 * @param reader 
 * @return cc_param 
 */
cc_param cc_parse_param_decl(cc_reader *reader)
{
    cc_param param = {CC_TREE_PARAM};
    param.type = cc_parse_type(reader);
    if (reader->curr_token->type == CC_TOKEN_IDENT)
    {
        TOKEN_GETSTR(param.name, reader->curr_token->data.name);
        cc_consume_token(reader);
    }
    return param;
}

/**
 * @brief Collects call parameters from the token stream and transforms
 * them into a call expression
 * 
 * @param reader Reader object
 * @param call Call expression
 * @return cc_expr 
 */
static void cc_parse_call_params(cc_reader *reader, cc_expr *call)
{
    while (reader->curr_token->type != CC_TOKEN_RPAREN)
    {
        cc_expr expr = cc_parse_assignment_expr (reader);
        if (reader->curr_token->type == CC_TOKEN_COMMA)
        {
            cc_consume_token(reader);
            call->data.call.params = xrealloc(call->data.call.params,
                                              (call->data.call.n_params + 1)
                                              * sizeof(cc_expr));
            call->data.call.params[call->data.call.n_params++] = expr;
            memset(&expr, 0, sizeof(expr));
            continue;
        }
        else if (reader->curr_token->type == CC_TOKEN_RPAREN)
        {
            cc_consume_token(reader);
            call->data.call.params = xrealloc(call->data.call.params,
                                              (call->data.call.n_params + 1)
                                              * sizeof(cc_expr));
            call->data.call.params[call->data.call.n_params++] = expr;
            return;
        }
        else
            cc_report(reader, CC_DL_ERROR, "Unexpected token \"%s\" inside "
                                           "call parameters",
                g_token_info[reader->curr_token->type].name);
    }
}

static int cc_resolve_symbol_1(cc_reader *reader, cc_token *tok,
                                const cc_expr *expr)
{
    size_t i;
    assert(tok->type == CC_TOKEN_IDENT);
    if (expr == NULL)
        return -1;

    for (i = 0; i < expr->data.block.n_vars; i++)
    {
        cc_variable *var = expr->data.block.vars[i];
        if (!strcmp(var->name, tok->data.name))
        {
            tok->decl = var;
            return 0;
        }
    }

    for (i = 0; i < expr->data.block.n_types; i++)
    {
        cc_type *type = expr->data.block.types[i];
        if (!strcmp(type->name, tok->data.name))
        {
            tok->id_kind = CC_ID_TYPE;
            tok->decl = type;
            return 0;
        }
    }
    return cc_resolve_symbol_1(reader, tok,
        cc_get_scoped_block(expr->data.block.parent_id));
}

/**
 * @brief Resolves a symbol, makes the token TOK either a TYPE or an IDENTIFIER
 * (I think it is VARIABLE, not IDENTIFIER)
 * depending on the given context
 * 
 * @param reader Reader object
 * @param tok Token to resolve
 */
static void cc_resolve_symbol(cc_reader *reader, cc_token *tok)
{
    assert(tok->type == CC_TOKEN_IDENT);
    if (!cc_resolve_symbol_1(reader, tok, reader->curr_block))
        return;
    
    if (!cc_resolve_symbol_1(reader, tok, reader->root_expr))
        return;
    
    cc_report(reader, CC_DL_ERROR, "Can't resolve identifier '%s' to a variable "
        "or a typename", tok->data.name);
}

#define cc_peek_token(reader) ((reader)->curr_token)

static cc_token *cc_peek_2nd_token (cc_reader *reader)
{
    if (reader->curr_token->type == CC_TOKEN_EOF) {
        return reader->curr_token;
    }

    return reader->curr_token + 1;
}

static int cc_next_token_starts_declaration (cc_reader *reader)
{
    switch (cc_peek_token (reader)->type) {
        case CC_TOKEN_IDENT:
        {
#ifdef __CC64__
            cc_token ident_tok;
#else
            cc_token ident_tok = *cc_peek_token (reader);
#endif
            search_scope (&ident_tok);
            if (ident_tok.id_kind == CC_ID_TYPE) return 1;
            
            return 0;
        }
        
        /* <type> <decl> */
        case CC_TOKEN_KW_AUTO:
        case CC_TOKEN_KW_DOUBLE:
        case CC_TOKEN_KW_FAR:
        case CC_TOKEN_KW_FLOAT:
        case CC_TOKEN_KW_INT:
        case CC_TOKEN_KW_LONG:
        case CC_TOKEN_KW_CHAR:
        case CC_TOKEN_KW_STRUCT:
        case CC_TOKEN_KW_UNION:
        case CC_TOKEN_KW_ENUM:
        case CC_TOKEN_KW_SHORT:
        case CC_TOKEN_KW_SIGNED:
        case CC_TOKEN_KW_UNSIGNED:
        case CC_TOKEN_KW_EXTERN:
        case CC_TOKEN_KW_STATIC:
        case CC_TOKEN_KW_VOLATILE:
        case CC_TOKEN_KW_VOID:
        case CC_TOKEN_KW_CONST:
        case CC_TOKEN_KW_TYPEDEF:
            return 1;
    }

    return 0;
}

static cc_expr cc_parse_postfix_expr (cc_reader *reader)
{
    cc_expr expr = {0};
    expr.id = cc_get_unique_id(reader);

    switch (cc_peek_token (reader)->type) {
#ifndef __CC64__
        case CC_TOKEN_IDENT: {
#ifdef __CC64__
            cc_token ident_tok;
#else
            cc_token ident_tok = *cc_peek_token (reader);
#endif
            search_scope (&ident_tok);
            
            cc_consume_token(reader);
            if (ident_tok.id_kind == CC_ID_ID && !ident_tok.decl) {
                cc_report (reader, CC_DL_ERROR, "'%s' undeclared",
                           ident_tok.data.name);
                break;
            }
            if (cc_peek_token (reader)->type == CC_TOKEN_LPAREN
                && ident_tok.id_kind == CC_ID_ID)
            {
                /* TODO: support runtime computed expressions like
                   (ptr + cum)(parm, aprm2, ...); */
                cc_variable *var;
#ifdef __CC64__
#else
                cc_consume_token(reader);
#endif

                /* Find function */
                if (TREE_TYPE (ident_tok.decl) != CC_TREE_VAR) {
                    cc_report (reader, CC_DL_ERROR,
                               "calling something other than variable is not yet supported");
                }
                var = ident_tok.decl;
                expr.type = CC_EXPR_CALL;
                expr.data.call.callee_func = var;
                expr.data.call.callee_type = var->type;
                cc_parse_call_params(reader, &expr);
                return expr;
            }
            else if (ident_tok.id_kind == CC_ID_ID)
            {
                expr.type = CC_EXPR_VARREF;
                expr.data.var_ref.var = ident_tok.decl;
                return expr;
            }
            cc_report (reader, CC_DL_ERROR, "expected expression");
            break;
        }
        /* <string> ? */
        case CC_TOKEN_STRING:
            expr.type = CC_EXPR_STRING;
            expr.data.string.data = cc_peek_token (reader)->data.string;
#ifdef __CC64__
#else
            cc_consume_token(reader);
#endif
            return expr;
        /* <number> ? */
        case CC_TOKEN_NUMBER:
            expr.type = CC_EXPR_CONSTANT;
            expr.data._const.numval = cc_peek_token (reader)->data.numval;
#ifdef __CC64__
#else
            cc_consume_token(reader);
#endif
            return expr;

        case CC_TOKEN_LPAREN:
            cc_consume_token(reader);
            expr = cc_parse_expression (reader);
            if (cc_peek_token (reader)->type != CC_TOKEN_RPAREN) {
                cc_report (reader, CC_DL_ERROR, "Missing terminating \")\" in "
                                                "primary expression");
            } else cc_consume_token (reader);
            return expr;

#endif
        default:
            cc_report (reader, CC_DL_ERROR, "Expected an expression but got \"%s\"",
                       g_token_info[cc_peek_token (reader)->type].name);
            return expr;
    }
}

static cc_expr cc_parse_cast_expr (cc_reader *reader)
{
#ifndef __CC64__
    if (cc_peek_token (reader)->type == CC_TOKEN_LPAREN
        && cc_peek_2nd_token (reader)->type == CC_TOKEN_IDENT) {
        cc_expr expr = {0};
        expr.id = cc_get_unique_id (reader);
        
        cc_consume_token (reader);
        if (reader->curr_token->type == CC_TOKEN_IDENT)
        {
            expr.type = CC_EXPR_CAST;
            expr.data.cast.type = cc_parse_type(reader);
            if (cc_peek_token (reader)->type != CC_TOKEN_RPAREN)
                cc_report (reader, CC_DL_ERROR, "Missing terminating \")\" in "
                                                "cast");
            cc_consume_token (reader);
        }

        expr.data.cast.expr = xcalloc (1, sizeof *expr.data.cast.expr);
        *expr.data.cast.expr = cc_parse_postfix_expr (reader);
        return expr;
    }
#endif

    return cc_parse_postfix_expr (reader);
}

static cc_expr cc_parse_binary_expr (cc_reader *reader)
{
    enum {
        PRIO_MULTIPLICATIVE = 0,
        PRIO_ADDITIVE,
        PRIO_SHIFT,
        PRIO_RELATIONAL,
        PRIO_EQUALITY,
        PRIO_AND,
        PRIO_XOR,
        PRIO_OR,
        PRIO_LOGICAL_AND,
        PRIO_LOGICAL_OR,
        PRIO_MAX
    };
    struct {
        cc_expr left;
        int type;
        int prio;
    } stack[PRIO_MAX];
    int sp = 0;

    while (1) {
        cc_expr left;
        int type, prio;

        left = cc_parse_cast_expr (reader);

        switch (cc_peek_token (reader)->type) {
            case CC_TOKEN_ASTERISK:
                type = CC_EXPR_MUL;
                prio = PRIO_MULTIPLICATIVE;
                break;

            case CC_TOKEN_DIV:
                type = CC_EXPR_DIV;
                prio = PRIO_MULTIPLICATIVE;
                break;

            case CC_TOKEN_REM:
                type = CC_EXPR_REM;
                prio = PRIO_MULTIPLICATIVE;
                break;
            
            case CC_TOKEN_PLUS:
                type = CC_EXPR_PLUS;
                prio = PRIO_ADDITIVE;
                break;

            case CC_TOKEN_MINUS:
                type = CC_EXPR_MINUS;
                prio = PRIO_ADDITIVE;
                break;

            case CC_TOKEN_LSHIFT:
                type = CC_EXPR_LSHIFT;
                prio = PRIO_SHIFT;
                break;

            case CC_TOKEN_RSHIFT:
                type = CC_EXPR_RSHIFT;
                prio = PRIO_SHIFT;
                break;

            case CC_TOKEN_LT:
                type = CC_EXPR_LT;
                prio = PRIO_RELATIONAL;
                break;

            case CC_TOKEN_GT:
                type = CC_EXPR_GT;
                prio = PRIO_RELATIONAL;
                break;

            case CC_TOKEN_LTE:
                type = CC_EXPR_LTE;
                prio = PRIO_RELATIONAL;
                break;

            case CC_TOKEN_GTE:
                type = CC_EXPR_GTE;
                prio = PRIO_RELATIONAL;
                break;

            case CC_TOKEN_EQ:
                type = CC_EXPR_EQ;
                prio = PRIO_EQUALITY;
                break;

            case CC_TOKEN_NEQ:
                type = CC_EXPR_NEQ;
                prio = PRIO_EQUALITY;
                break;

            case CC_TOKEN_AMPERSAND:
                type = CC_EXPR_BIT_AND;
                prio = PRIO_AND;
                break;

            case CC_TOKEN_BIT_XOR:
                type = CC_EXPR_BIT_XOR;
                prio = PRIO_XOR;
                break;

            case CC_TOKEN_BIT_OR:
                type = CC_EXPR_BIT_OR;
                prio = PRIO_OR;
                break;

            case CC_TOKEN_AND:
                type = CC_EXPR_AND;
                prio = PRIO_LOGICAL_AND;
                break;

            case CC_TOKEN_OR:
                type = CC_EXPR_OR;
                prio = PRIO_LOGICAL_OR;
                break;

            default:
                while (sp) {
                    cc_expr expr = {0};

                    sp--;
                    expr.id = cc_get_unique_id (reader);
                    expr.data.binary_op.left = xcalloc (1, sizeof *expr.data.binary_op.left);
                    expr.data.binary_op.right = xcalloc (1, sizeof *expr.data.binary_op.right);

                    *expr.data.binary_op.left = stack[sp].left;
                    expr.type = stack[sp].type;
                    *expr.data.binary_op.right = left;
                    left = expr;
                }
                
                return left;
        }

#ifndef __CC64__
        cc_consume_token(reader);

        while (sp && stack[sp - 1].prio <= prio) {
            cc_expr expr = {0};
            
            sp--;
            expr.id = cc_get_unique_id (reader);
            expr.data.binary_op.left = xcalloc (1, sizeof *expr.data.binary_op.left);
            expr.data.binary_op.right = xcalloc (1, sizeof *expr.data.binary_op.right);

            *expr.data.binary_op.left = stack[sp].left;
            expr.type = stack[sp].type;
            *expr.data.binary_op.right = left;
            left = expr;
        }
#endif

        stack[sp].left = left;
        stack[sp].type = type;
        stack[sp].prio = prio;
        sp++;
    }
}

static cc_expr cc_parse_assignment_expr (cc_reader *reader)
{
#if 0
    cc_expr expr = {0};
    while (reader->curr_token->type != CC_TOKEN_EOF
        && reader->curr_token->type != CC_TOKEN_SEMICOLON)
    {

    }
    return expr;
#endif
    return cc_parse_binary_expr (reader);
}

static cc_expr cc_parse_condition(cc_reader *reader, cc_token_type stop_type)
{
    cc_expr expr = {0};
    expr.id = cc_get_unique_id(reader);
    expr.type = CC_EXPR_BLOCK;
    while (reader->curr_token->type != stop_type)
    {
        if (reader->curr_token->type == CC_TOKEN_LT)
        {
            
        }
        else if (reader->curr_token->type == CC_TOKEN_LBRACE
              || reader->curr_token->type == CC_TOKEN_RBRACE
              || reader->curr_token->type == CC_TOKEN_COLON
              || (stop_type != CC_TOKEN_SEMICOLON
              && reader->curr_token->type == CC_TOKEN_SEMICOLON))
            cc_report(reader, CC_DL_ERROR, "Unexpected token %s in "
                                           "conditional expression",
                g_token_info[reader->curr_token->type].name);
#ifdef __CC64__
#else
        cc_consume_token(reader);
#endif
    }
    cc_consume_token(reader);
    return expr;
}

cc_expr cc_parse_expression (cc_reader *reader)
{
    cc_expr expr = {0};
    expr.id = cc_get_unique_id(reader);
    switch (reader->curr_token->type)
    {
#ifdef __CC64__
#else
    case CC_TOKEN_AMPERSAND:
      {
        cc_token ident_tok;

        expr.type = CC_EXPR_ADDRESSOF;
        cc_consume_token(reader);

        ident_tok = *reader->curr_token;
        if (ident_tok.type != CC_TOKEN_IDENT)
        {
            cc_report(reader, CC_DL_ERROR, "Expected an identifier but got \"%s\"",
                      g_token_info[reader->curr_token->type].name);
        }
        else
        {
            cc_resolve_symbol(reader, &ident_tok);
            if (ident_tok.type != CC_TOKEN_VARIABLE)
            {
                cc_report(reader, CC_DL_ERROR, "Expected a variable but got \"%s\"",
                          g_token_info[reader->curr_token->type].name);
            }
            else
            {
                expr.data.var_ref.var = ident_tok.decl;
                cc_consume_token(reader);
                return expr;
            }
        }
      }
#endif
        default:
            return cc_parse_assignment_expr (reader);
    }
}

/**
 * @brief Parses statement inside compound statement
 * 
 * @param reader Reader object
 * @return cc_expr Expression resulting from parsing
 */
cc_expr cc_parse_statement(cc_reader *reader)
{
    cc_expr expr = {0};
    expr.id = cc_get_unique_id(reader);
    /* <expr> ; */
    if (reader->curr_token->type == CC_TOKEN_SEMICOLON)
    {
        cc_consume_token(reader);
        return expr;
    }
    /* <block> {} <block-end> */
    else if (reader->curr_token->type == CC_TOKEN_LBRACE)
    {
#ifdef __CC64__
        return expr;
#else
        return cc_parse_compound_statement (reader);
#endif
    }
    /* return <expr> ; */
    else if (reader->curr_token->type == CC_TOKEN_KW_RETURN)
    {
#ifdef __CC64__
#else
        cc_consume_token(reader);
#endif
        expr.type = CC_EXPR_RETURN;
        expr.data.ret.ret_expr = xcalloc (1, sizeof(cc_expr));
#ifdef __CC64__
#else
        *expr.data.ret.ret_expr = cc_parse_expression(reader);
#endif
        return expr;
    }
    /* if ( <expr> ) <block> */
    else if (reader->curr_token->type == CC_TOKEN_KW_IF)
    {
#ifdef __CC64__
#else
        cc_consume_token(reader);
#endif
        if (reader->curr_token->type != CC_TOKEN_LPAREN)
            cc_report(reader, CC_DL_ERROR, "Expected \"(\" after if block");
#ifdef __CC64__
#else
        cc_consume_token(reader);
#endif
        expr.type = CC_EXPR_IF;
        expr.data.if_else.cond_expr = xcalloc (1, sizeof(cc_expr));
#ifdef __CC64__
#else
        *expr.data.if_else.cond_expr = cc_parse_condition(reader,
                                                          CC_TOKEN_RPAREN);
#endif
        /* Multiline block */
        expr.data.if_else.body_expr = xcalloc (1,sizeof(cc_expr));
        if (reader->curr_token->type == CC_TOKEN_LBRACE)
#ifdef __CC64__
;
#else
            *expr.data.if_else.body_expr = cc_parse_compound_statement (reader);
#endif
        /* Single line block */
        else
#ifdef __CC64__
#else
            *expr.data.if_else.body_expr = cc_parse_statement(reader);
#endif
        return expr;
    }
#ifdef __CC64__
    return expr;
#else
    expr = cc_parse_expression (reader);
    if (cc_peek_token (reader)->type != CC_TOKEN_SEMICOLON) {
        cc_report (reader, CC_DL_ERROR, "Expected a semicolon but got \"%s\"",
                   g_token_info[cc_peek_token (reader)->type].name);
    }
    cc_consume_token (reader);
    return expr;
#endif
}

/**
 * @brief Parses a compound statement
 * 
 * @param reader Reader object
 * @return cc_expr Block expression
 */
static cc_expr cc_parse_compound_statement (cc_reader *reader)
{
    cc_expr *old_block = reader->curr_block;
    cc_expr expr = {0};
    expr.id = cc_get_unique_id(reader);
    expr.type = CC_EXPR_BLOCK;
    if (reader->curr_token->type != CC_TOKEN_LBRACE)
        cc_report(reader, CC_DL_ERROR, "Expected a brace \"{\"");
#ifdef __CC64__
#else
    cc_consume_token(reader);
#endif
    expr.data.block.parent_id = reader->curr_block->id; /* Set parent rel */
    expr.data.block.stack_depth = 0;
    if (old_block != reader->root_expr)
    {
        expr.data.block.stack_depth = old_block->data.block.n_vars * 4;
                                      + old_block->data.block.stack_depth;
    }
    reader->curr_block = &expr; /* Temporarily set new block */
    enter_scope ();
    while (reader->curr_token->type != CC_TOKEN_RBRACE)
    {
        expr.data.block.exprs = xrealloc(expr.data.block.exprs,
                                        (expr.data.block.n_exprs + 1)
                                        * sizeof(cc_expr));
        if (cc_next_token_starts_declaration (reader)) {
            expr.data.block.exprs[expr.data.block.n_exprs++]
                = cc_parse_declaration_or_fndef (reader, 0);
        } else {
            expr.data.block.exprs[expr.data.block.n_exprs++]
                = cc_parse_statement(reader); /* Save statement into block */
        }
        if (reader->curr_token->type == CC_TOKEN_EOF)
            cc_report(reader, CC_DL_ERROR, "Expected a brace \"}\"");
    }
    exit_scope ();
    reader->curr_block = old_block; /* Restore old block */
    cc_add_scoped_block(expr); /* Save a copy of the block into the scoped
                                * block list so we will have access to the
                                * symbols */
#ifdef __CC64__
#else
    cc_consume_token(reader);
#endif
    return expr;
}

cc_variable cc_parse_variable(cc_reader *reader)
{
    cc_variable var = {CC_TREE_VAR};
    var.linkage = CC_LINKAGE_AUTO;
    while (1) /* Parse storage qualifiers */
    {
        if (reader->curr_token->type == CC_TOKEN_KW_EXTERN)
        {
            if (var.linkage == CC_LINKAGE_STATIC)
                cc_report(reader, CC_DL_ERROR, "Static can't be extern");
            var.linkage = CC_LINKAGE_EXTERN;
        }
        else if (reader->curr_token->type == CC_TOKEN_KW_STATIC)
        {
            if (var.linkage == CC_LINKAGE_EXTERN)
                cc_report(reader, CC_DL_ERROR, "Extern can't be static");
            var.linkage = CC_LINKAGE_STATIC;
        }
        else if (reader->curr_token->type == CC_TOKEN_KW_INLINE)
            var.linkage = CC_LINKAGE_INLINE;
        else if (reader->curr_token->type == CC_TOKEN_KW_AUTO)
            var.linkage = CC_LINKAGE_AUTO;
        else
            break;
        cc_consume_token(reader);
    }
    var.type = cc_parse_type(reader); /* Now parse the type */
    if (reader->curr_token->type == CC_TOKEN_IDENT)
    {
        TOKEN_GETSTR(var.name, reader->curr_token->data.name);
        cc_consume_token(reader);
    }
    if (reader->curr_token->type == CC_TOKEN_LBRACKET) {
        ignore_arrays (reader);
    }
    if (reader->curr_token->type == CC_TOKEN_LPAREN) /* Function arguments */
    {
        cc_param param;
        /* Make current type be the return type */
        var.type.data.f.return_type = xcalloc (1, sizeof(cc_type));
        *var.type.data.f.return_type = var.type;
        var.type.data.f.return_type->data.f.return_type = NULL;
        var.type.data.f.params = NULL;
        var.type.data.f.n_params = 0;
        var.type.data.f.variadic = 0;
        var.type.mode = CC_TYPE_FUNCTION;
        cc_consume_token(reader);
        if (cc_peek_token (reader)->type == CC_TOKEN_KW_VOID
            && cc_peek_2nd_token (reader)->type == CC_TOKEN_RPAREN) {
            /* a single void is ignored */
            cc_consume_token(reader);
        }
        while (reader->curr_token->type != CC_TOKEN_RPAREN)
        {
            if (reader->curr_token->type == CC_TOKEN_ELLIPSIS)
            {
                var.type.data.f.variadic = 1;
                cc_consume_token(reader);
                if (reader->curr_token->type != CC_TOKEN_RPAREN)
                    cc_report(reader, CC_DL_ERROR, "Expected a \")\" "
                                                  "immediately after ellipsis"
                                                  "");
                break;
            }
            param = cc_parse_param_decl(reader);
            param.index = var.type.data.f.n_params;
            var.type.data.f.params = xrealloc(var.type.data.f.params,
                                            (var.type.data.f.n_params + 1)
                                            * sizeof(cc_param));
            var.type.data.f.params[var.type.data.f.n_params++] = param;
            if (reader->curr_token->type == CC_TOKEN_COMMA)
                cc_consume_token(reader);
            else if (reader->curr_token->type == CC_TOKEN_RPAREN)
                break;
            else
                cc_report(reader, CC_DL_ERROR, "Expected a comma after "
                    "parameter declaration but got \"%s\" instead",
                    g_token_info[reader->curr_token->type].name);
        }
        if (reader->curr_token->type != CC_TOKEN_RPAREN)
            cc_report(reader, CC_DL_ERROR, "Unterminated \")\" in function"
                                           "argument declaration");
        cc_consume_token(reader);
    }

    /* if we hit a semicolon, and there was no extern keyword,
       and this is a function, then we now assume extern */
    if ((reader->curr_token->type == CC_TOKEN_SEMICOLON)
        && (var.type.mode == CC_TYPE_FUNCTION)
        && (var.linkage == CC_LINKAGE_AUTO)
       )
    {
        var.linkage = CC_LINKAGE_EXTERN;
    }

    /* Now, for normal variables we could end, or we could abort */
    if (reader->curr_token->type == CC_TOKEN_LBRACE
     || reader->curr_token->type == CC_TOKEN_ASSIGN
     || reader->curr_token->type == CC_TOKEN_SEMICOLON)
        return var;

    cc_report(reader, CC_DL_ERROR, "Unexpected token \"%s\" on declaration",
              g_token_info[reader->curr_token->type].name);
}

static void cc_delete_redundant(cc_reader *reader, const cc_expr *expr)
{

}

static cc_expr cc_parse_declaration_or_fndef (cc_reader *reader, int fndef_allowed)
{
    cc_expr expr = {0};
    expr.id = cc_get_unique_id(reader);
    switch (cc_peek_token (reader)->type)
    {
    case CC_TOKEN_IDENT:
    {
#ifdef __CC64__
        cc_token ident_tok;
#else
        cc_token ident_tok = *cc_peek_token (reader);
#endif
        search_scope (&ident_tok);
        if (ident_tok.id_kind == CC_ID_TYPE) goto decl;
        
        goto default_;
    }
    
    /* <type> <decl> */
    case CC_TOKEN_KW_AUTO:
    case CC_TOKEN_KW_DOUBLE:
    case CC_TOKEN_KW_FAR:
    case CC_TOKEN_KW_FLOAT:
    case CC_TOKEN_KW_INT:
    case CC_TOKEN_KW_LONG:
    case CC_TOKEN_KW_CHAR:
    case CC_TOKEN_KW_STRUCT:
    case CC_TOKEN_KW_UNION:
    case CC_TOKEN_KW_ENUM:
    case CC_TOKEN_KW_SHORT:
    case CC_TOKEN_KW_SIGNED:
    case CC_TOKEN_KW_UNSIGNED:
    case CC_TOKEN_KW_EXTERN:
    case CC_TOKEN_KW_STATIC:
    case CC_TOKEN_KW_VOLATILE:
    case CC_TOKEN_KW_VOID:
    case CC_TOKEN_KW_CONST:
decl:
    {
        cc_variable s_var, *var;
#ifdef __CC64__
#else
        s_var = cc_parse_variable(reader);
        if (s_var.name == NULL) {
            /* Declaration without declarator (variable name) is allowed. */
            if (cc_peek_token (reader)->type != CC_TOKEN_SEMICOLON) {
                cc_report (reader, CC_DL_ERROR, "Expected a semicolon but got \"%s\"",
                           g_token_info[cc_peek_token (reader)->type].name);
            }
            cc_consume_token (reader);
            return expr;
        }
#endif
        expr.type = CC_EXPR_DECL;
        var = expr.data.decl.var = cc_add_variable(reader); /* Add to list of variables */
        *var = s_var;
        add_to_scope (var->name, var);
        var->block_offset = (reader->curr_block->data.block.n_vars - 1) * 4;
        /* Now fuck you and make a good parsing of that lbrace */
        if (cc_peek_token (reader)->type == CC_TOKEN_LBRACE)
        {
            if (!fndef_allowed) {
                cc_report (reader, CC_DL_WARNING, "nested functions are not allowed");
            }
            entering_function = &expr;
            var->block_expr = xcalloc (1, sizeof(cc_expr));
#ifdef __CC64__
#else
            *var->block_expr = cc_parse_compound_statement (reader);
#endif
        } else {
            if (cc_peek_token (reader)->type != CC_TOKEN_SEMICOLON) {
                cc_report (reader, CC_DL_ERROR, "Expected a semicolon but got \"%s\"",
                           g_token_info[cc_peek_token (reader)->type].name);
            }
            cc_consume_token (reader);
        }
        return expr;
    }

#ifndef __CC64__
    case CC_TOKEN_KW_TYPEDEF:
    {
        cc_type s_type;
        
        cc_consume_token (reader);
        s_type = cc_parse_type (reader);

        if (cc_peek_token (reader)->type != CC_TOKEN_IDENT) {
            cc_report(reader, CC_DL_ERROR, "Expected an identifier but got \"%s\"",
                      g_token_info[cc_peek_token (reader)->type].name);
        } else {
            cc_type *type;

            type = cc_add_type (reader);
            *type = s_type;
            type->name = cc_peek_token (reader)->data.name;
            add_to_scope (type->name, type);
        }
        cc_consume_token (reader);

        if (reader->curr_token->type == CC_TOKEN_LBRACKET) {
            ignore_arrays (reader);
        }
        
        if (cc_peek_token (reader)->type != CC_TOKEN_SEMICOLON) {
            cc_report (reader, CC_DL_ERROR, "Expected a semicolon but got \"%s\"",
                       g_token_info[cc_peek_token (reader)->type].name);
        }
        cc_consume_token (reader);
        
        return expr;
    }
#endif

    default:
default_:
        cc_report (reader, CC_DL_ERROR, "Expected a declaration but got \"%s\"",
                   g_token_info[cc_peek_token (reader)->type].name);
        break;
    }
}

static cc_expr cc_parse_external_declaration (cc_reader *reader)
{
#ifndef __CC64__
    if (cc_peek_token (reader)->type == CC_TOKEN_SEMICOLON) {
        cc_expr expr = {0};
        cc_report (reader, CC_DL_WARNING, "extra ';' outside of a function is not allowed");
        cc_consume_token (reader);
        return expr;
    }
#endif

    return cc_parse_declaration_or_fndef (reader, 1);
}

/**
 * @brief Perform the parsing until EOF is reached
 * @param reader 
 */
static void cc_parser_do(cc_reader *reader)
{
    cc_expr *expr;
    expr = reader->root_expr = xcalloc (1, sizeof(cc_expr));
    expr->id = cc_get_unique_id(reader);
    expr->type = CC_EXPR_BLOCK;
    reader->curr_block = expr;
    enter_scope ();
    while (reader->curr_token->type != CC_TOKEN_EOF)
    {
        expr->data.block.exprs = xrealloc(expr->data.block.exprs,
                                          (expr->data.block.n_exprs + 1)
                                          * sizeof(cc_expr));
        expr->data.block.exprs[expr->data.block.n_exprs++]
            = cc_parse_external_declaration (reader);
    }

    /* TODO: constant propagation, operator precedence,
     * elimination offsets, packing, string-optimizations (such as using
     * the stack for small strings), pattern matching, behaviour detection
     * etc, etc, etc */
#ifdef DEBUG
    cc_dump_expr(reader, reader->root_expr);
#endif
    exit_scope ();
}

int cc_parse_file(cc_reader *reader)
{
    if (reader->input == NULL) {
        cc_lex_with_preprocess (reader);
    } else {
        char line[80];
        
        while (fgets(line, sizeof(line), reader->input) != 0)
            cc_lex_line(reader, line);
    }
#ifdef DEBUG
    cc_dump_tokens(reader);
#endif

    if (reader->tokens == NULL)
    {
        cc_report(reader, CC_DL_WARNING, "No code to parse");
        return 0;
    }

    /* Add EOF token */
    reader->tokens = xrealloc(reader->tokens, (reader->n_tokens + 1)
                                              * sizeof(cc_token));
    reader->tokens[reader->n_tokens++].type = CC_TOKEN_EOF;
    reader->curr_token = &reader->tokens[0];

    cc_parser_do (reader);
    cc_codegen (reader, reader->root_expr);
#ifdef DEBUG
    printf("+++End of compilation\n");
#endif
    return 0;
}
