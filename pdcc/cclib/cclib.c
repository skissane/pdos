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

static void cc_dump_type(cc_reader *reader, const cc_type *type);
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
        cc_dump_variable(reader, &expr->data.decl.var);
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
        {
            const cc_type *type = &expr->data.block.types[i];
            cc_dump_type(reader, type);
        }

        printf("vars=%u\n", expr->data.block.n_vars);
        for (i = 0; i < expr->data.block.n_vars; i++)
        {
            const cc_variable *var = &expr->data.block.vars[i];
            cc_dump_variable(reader, var);
            if (var->block_expr)
                cc_dump_expr(reader, var->block_expr);
        }

        for (i = 0; i < expr->data.block.n_exprs; i++)
            cc_dump_expr(reader, &expr->data.block.exprs[i]);
        printf("} (End-Block)\n");
        break;
    default:
        printf("Unknown expr type %u\n", expr->type);
        break;
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
                    * sizeof(cc_type));
    return &reader->curr_block->data.block.types[
        reader->curr_block->data.block.n_types++];
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
                    * sizeof(cc_variable));
    return &reader->curr_block->data.block.vars[
        reader->curr_block->data.block.n_vars++];
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
    cc_type type = {0};
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
        }

        /* Syntax of the form:
         * <struct|union|enum> <identifier> { <members> } <var-expr>; */
        if (reader->curr_token->type == CC_TOKEN_LBRACE)
            cc_parse_struct_member_decl(reader, &type);
        return type;
    }
    
    for ( ; reader->curr_token->type != CC_TOKEN_IDENT; reader->curr_token++)
    {
        if (reader->curr_token->type == CC_TOKEN_KW_INT)
        {
            if (type.mode != CC_TYPE_CHAR && type.mode != CC_TYPE_SHORT
             && type.mode != CC_TYPE_LONG)
            {
                type.mode = CC_TYPE_INT;
            }
        }
        else if (reader->curr_token->type == CC_TOKEN_KW_SHORT)
        {
            type.mode = CC_TYPE_SHORT;
        }
        else if (reader->curr_token->type == CC_TOKEN_KW_CHAR)
        {
            type.mode = CC_TYPE_CHAR;
        }
        else if (reader->curr_token->type == CC_TOKEN_KW_LONG)
        {
            type.mode = CC_TYPE_LONG;
        }
        else if (reader->curr_token->type == CC_TOKEN_KW_SIGNED)
        {
            type.data.sign = 1;
        }
        else if (reader->curr_token->type == CC_TOKEN_KW_UNSIGNED)
        {
            if (type.mode == CC_TYPE_FLOAT || type.mode == CC_TYPE_DOUBLE
             || type.mode == CC_TYPE_LDOUBLE || type.mode == CC_TYPE_UNION
             || type.mode == CC_TYPE_STRUCT || type.mode == CC_TYPE_ENUM)
                cc_report(reader, CC_DL_ERROR, "This combination of types"
                                               "do not support sign "
                                               "qualifiers");
            
            if (type.mode != CC_TYPE_CHAR && type.mode != CC_TYPE_SHORT
             && type.mode != CC_TYPE_LONG && type.mode != CC_TYPE_LLONG
             && type.mode != CC_TYPE_INT)
            {
                type.mode = CC_TYPE_INT;
            }
            type.data.sign = 0;
        }
        else
            break;
    }

    /* Pointers of the form <*> <cv> may come now */
    while(1)
    {
        if (reader->curr_token->type == CC_TOKEN_ASTERISK)
        {
            if (type.ptr_depth >= MAX_PTR_DEPTH)
                cc_report(reader, CC_DL_ERROR, "Pointer is too deep!");
            cc_consume_token(reader);
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
        printf("sizeof struct not supported");
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
    cc_param param = {0};
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
        cc_expr expr = cc_parse_statment(reader);
        if (reader->curr_token->type == CC_TOKEN_COMMA)
        {
            cc_consume_token(reader);
            call->data.call.params = xrealloc(call->data.call.params,
                                              (call->data.call.n_params + 1)
                                              * sizeof(cc_expr));
            call->data.call.params[call->data.call.n_params++] = expr;
            expr = ((cc_expr){0});
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

static void cc_resolve_symbol_1(cc_reader *reader, cc_token *tok,
                                const cc_expr *expr)
{
    size_t i;
    assert(tok->type == CC_TOKEN_IDENT);
    for (i = 0; i < expr->data.block.n_vars; i++)
    {
        cc_variable *var = &expr->data.block.vars[i];
        if (!strcmp(var->name, tok->data.name))
        {
            tok->type = CC_TOKEN_VARIABLE;
            tok->data.var = var;
            return;
        }
    }
    for (i = 0; i < expr->data.block.n_types; i++)
    {
        cc_type *type = &expr->data.block.types[i];
        if (!strcmp(type->name, tok->data.name))
        {
            tok->type = CC_TOKEN_TYPE;
            tok->data.type = type;
            return;
        }
    }
}

/**
 * @brief Resolves a symbol, makes the token TOK either a TYPE or an IDENTIFIER
 * depending on the given context
 * 
 * @param reader Reader object
 * @param tok Token to resolve
 */
static void cc_resolve_symbol(cc_reader *reader, cc_token *tok)
{
    assert(tok->type == CC_TOKEN_IDENT);
    cc_resolve_symbol_1(reader, tok, reader->curr_block);
    cc_report(reader, CC_DL_ERROR, "Can't resolve identifier %s to a variable "
                                   "or a typename", tok->data.name);
}

static cc_expr cc_parse_assignment_expr(cc_reader *reader)
{
    cc_expr expr = {0};
    while (reader->curr_token->type != CC_TOKEN_EOF
        && reader->curr_token->type != CC_TOKEN_SEMICOLON)
    {

    }
    return expr;
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
        cc_consume_token(reader);
    }
    cc_consume_token(reader);
    return expr;
}

cc_expr cc_parse_expression(cc_reader *reader)
{
    cc_expr expr = {0};
    expr.id = cc_get_unique_id(reader);
    if (reader->curr_token->type == CC_TOKEN_LPAREN)
    {
        cc_consume_token(reader);
        expr.type = CC_EXPR_CAST;
        expr.data.cast.type = cc_parse_type(reader);
        if (reader->curr_token->type != CC_TOKEN_RPAREN)
            cc_report(reader, CC_DL_ERROR, "Missing terminating \")\" in "
                                           "cast");
        cc_consume_token(reader);
        return expr;
    }
    else if (reader->curr_token->type == CC_TOKEN_IDENT)
    {
        cc_token ident_tok = cc_consume_token(reader);
        cc_resolve_symbol(reader, &ident_tok);
        if (reader->curr_token->type == CC_TOKEN_LPAREN
         && ident_tok.type == CC_TOKEN_VARIABLE)
        {
            /* TODO: support runtime computed expressions like
               (ptr + cum)(parm, aprm2, ...); */
            size_t i;
            cc_consume_token(reader);

            /* Find function */
            if (ident_tok.type != CC_TOKEN_VARIABLE)
                cc_report(reader, CC_DL_ERROR, "Expected a variable name"
                                               "on function call");
            expr.type = CC_EXPR_CALL;
            expr.data.call.callee_func = ident_tok.data.var;
            expr.data.call.callee_type = ident_tok.data.var->type;
            cc_parse_call_params(reader, &expr);
            return expr;
        }
        else if(ident_tok.type == CC_TOKEN_VARIABLE)
        {
            expr.type = CC_EXPR_VARREF;
            expr.data.var_ref.var = ident_tok.data.var;
            return expr;
        }
        cc_report(reader, CC_DL_ERROR, "Unresolvable identifier \"%s\"",
            ident_tok.data.name);
    }
    /* <string> ? */
    else if (reader->curr_token->type == CC_TOKEN_STRING)
    {
        expr.type = CC_EXPR_STRING;
        expr.data.string.data = reader->curr_token->data.string;
        cc_consume_token(reader);
        return expr;
    }
    /* <number> ? */
    else if (reader->curr_token->type == CC_TOKEN_NUMBER)
    {
        expr.type = CC_EXPR_CONSTANT;
        expr.data._const.numval = reader->curr_token->data.numval;
        cc_consume_token(reader);
        return expr;
    }
    /* <type> <decl> */
    else if (reader->curr_token->type == CC_TOKEN_KW_AUTO
          || reader->curr_token->type == CC_TOKEN_KW_DOUBLE
          || reader->curr_token->type == CC_TOKEN_KW_FAR
          || reader->curr_token->type == CC_TOKEN_KW_FLOAT
          || reader->curr_token->type == CC_TOKEN_KW_INT
          || reader->curr_token->type == CC_TOKEN_KW_LONG
          || reader->curr_token->type == CC_TOKEN_KW_CHAR
          || reader->curr_token->type == CC_TOKEN_KW_STRUCT
          || reader->curr_token->type == CC_TOKEN_KW_UNION
          || reader->curr_token->type == CC_TOKEN_KW_ENUM
          || reader->curr_token->type == CC_TOKEN_KW_SHORT
          || reader->curr_token->type == CC_TOKEN_KW_SIGNED
          || reader->curr_token->type == CC_TOKEN_KW_UNSIGNED
          || reader->curr_token->type == CC_TOKEN_IDENT)
    {
        expr.type = CC_EXPR_DECL;
        if (reader->curr_token->type == CC_TOKEN_IDENT)
            cc_resolve_symbol(reader, reader->curr_token);
        expr.data.decl.var = cc_parse_variable(reader);
        *cc_add_variable(reader) = expr.data.decl.var;
        return expr;
    }
    cc_report(reader, CC_DL_ERROR, "Expected an expression but got \"%s\"",
        g_token_info[reader->curr_token->type].name);
}

/**
 * @brief Parses an expression in the context of a block
 * 
 * @param reader Reader object
 * @return cc_expr Expression resulting from parsing
 */
cc_expr cc_parse_statment(cc_reader *reader)
{
    cc_expr expr = {0};
    expr.id = cc_get_unique_id(reader);
    /* <expr> ; */
    if (reader->curr_token->type == CC_TOKEN_SEMICOLON)
    {
        cc_consume_token(reader);
        return expr;
    }
    /* return <expr> ; */
    else if (reader->curr_token->type == CC_TOKEN_KW_RETURN)
    {
        cc_consume_token(reader);
        expr.type = CC_EXPR_RETURN;
        expr.data.ret.ret_expr = xcalloc(sizeof(cc_expr));
        *expr.data.ret.ret_expr = cc_parse_expression(reader);
        return expr;
    }
    /* if ( <expr> ) <block> */
    else if (reader->curr_token->type == CC_TOKEN_KW_IF)
    {
        cc_consume_token(reader);
        if (reader->curr_token->type != CC_TOKEN_LPAREN)
            cc_report(reader, CC_DL_ERROR, "Expected \"(\" after if block");
        cc_consume_token(reader);

        expr.type = CC_EXPR_IF;
        expr.data.if_else.cond_expr = xcalloc(sizeof(cc_expr));
        *expr.data.if_else.cond_expr = cc_parse_condition(reader,
                                                          CC_TOKEN_RPAREN);
        /* Multiline block */
        expr.data.if_else.body_expr = xcalloc(sizeof(cc_expr));
        if (reader->curr_token->type == CC_TOKEN_LBRACE)
        {
            cc_consume_token(reader);
            *expr.data.if_else.body_expr = cc_parse_block_expr(reader);
        }
        /* Single line block */
        else
            *expr.data.if_else.body_expr = cc_parse_statment(reader);
        return expr;
    }
    /* Fallback to declaration :) */
    return cc_parse_expression(reader);
}

/**
 * @brief Parses a block expression
 * 
 * @param reader Reader object
 * @return cc_expr Block expression
 */
cc_expr cc_parse_block_expr(cc_reader *reader)
{
    cc_expr expr = {0};
    expr.id = cc_get_unique_id(reader);
    expr.type = CC_EXPR_BLOCK;
    if (reader->curr_token->type != CC_TOKEN_LBRACE)
        cc_report(reader, CC_DL_ERROR, "Expected an opening brace \"}\"");
    while (reader->curr_token->type != CC_TOKEN_RBRACE)
    {
        cc_expr st_expr = cc_parse_statment(reader); /* Statment */
        expr.data.block.exprs = xrealloc(expr.data.block.exprs,
                                        (expr.data.block.n_exprs + 1)
                                        * sizeof(cc_expr));
        expr.data.block.exprs[expr.data.block.n_exprs++] = st_expr;
        if (reader->curr_token->type == CC_TOKEN_EOF)
            cc_report(reader, CC_DL_ERROR, "Expected a closing brace \"{\"");
    }
    return expr;
}

cc_variable cc_parse_variable(cc_reader *reader)
{
    cc_variable var = {0};
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
    if (reader->curr_token->type == CC_TOKEN_LPAREN) /* Function arguments */
    {
        cc_param param = {0};
        /* Make current type be the return type */
        var.type.data.f.return_type = xcalloc(sizeof(cc_type));
        *var.type.data.f.return_type = var.type;
        var.type.data.f.return_type->data.f.return_type = NULL;
        var.type.mode = CC_TYPE_FUNCTION;
        cc_consume_token(reader);
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

/**
 * @brief Review and make a final pass to expressions
 * 
 * @param reader 
 */
static void cc_review_exprs(cc_reader *reader)
{
    size_t i;
    for (i = 0; i < reader->curr_block->data.block.n_vars; i++)
    {
        const cc_variable *var = &reader->curr_block->data.block.vars[i];
        if (var->block_expr)
        {
            /* Delete empty exprs */
        }
    }
    cc_dump_expr(reader, reader->root_expr);
}

int cc_parse_file(cc_reader *reader)
{
    char line[80];
    while (fgets(line, sizeof(line), reader->input) != 0)
        cc_lex_line(reader, line);
    cc_dump_tokens(reader);

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

    reader->root_expr = xcalloc(sizeof(cc_expr));
    reader->curr_block = reader->root_expr;
    *cc_add_type(reader) = cc_parse_type(reader);
    *cc_add_variable(reader) = cc_parse_variable(reader);
    *cc_add_variable(reader) = cc_parse_variable(reader);
    cc_review_exprs(reader);
    cc_i386gen(reader, reader->root_expr);
    printf("+++End of compilation\n");
    return 0;
}
