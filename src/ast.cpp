#include "ast.h"
#include "token.h"

#include <iostream>
#include <map>

int get_next_token() {
    return CUR_TOK = get_token();
}

ExprAST *parse_number_expr();
ExprAST *parse_paren_expr();
ExprAST *parse_identifier_expr();
ExprAST *parse_primary();
ExprAST *parse_expression();
ExprAST *parse_bin_op_rhs(int expr_prec, ExprAST *lhs);
PrototypeAST *parse_prototype();

/// definition ::= 'def' prototype expression
FunctionAST *parse_definition() {
    get_next_token();// eat def.
    auto proto = parse_prototype();
    if (!proto) return nullptr;
    auto e = parse_expression();
    return e ? new FunctionAST(proto, e) : nullptr;
}

/// external ::= 'extern' prototype
PrototypeAST *parse_extern() {
    get_next_token();// eat extern.
    return parse_prototype();
}

/// toplevelexpr ::= expression
FunctionAST *parse_top_level_expr() {
    auto e = parse_expression();
    // Make an anonymous proto.
    return e ? new FunctionAST(new PrototypeAST("", {}), e) : nullptr;
}

/// Error* - These are little helper functions for error handling.
static ExprAST *error(const char *str) {
    std::cerr << "error: " << str << std::endl;
    return nullptr;
}
static PrototypeAST *error_p(const char *str) {
    error(str);
    return nullptr;
}
static FunctionAST *error_f(const char *str) {
    error(str);
    return nullptr;
}

/// get_tok_precedence - Get the precedence of the pending binary operator token.
static int get_tok_precedence() {
    if (!isascii(CUR_TOK)) return -1;
    /// BINOP_PRECEDENCE - This holds the precedence for each binary operator that is defined.
    const static auto binop_precedence = std::map<char, int>{
        {'<', 10},
        {'+', 20},
        {'-', 20},
        {'*', 30},
    };
    // Make sure it's a declared binop.
    auto ans = binop_precedence.find(CUR_TOK);
    return ans != binop_precedence.end() ? ans->second : -1;
}

/// numberexpr ::= number
static ExprAST *parse_number_expr() {
    auto ans = new NumberExprAST(NUM_VAL);
    get_next_token();// consume the number
    return ans;
}

/// parenexpr ::= '(' expression ')'
static ExprAST *parse_paren_expr() {
    get_next_token();// eat (.
    auto v = parse_expression();
    if (!v) return nullptr;

    if (CUR_TOK != ')') return error("expected ')'");
    get_next_token();// eat ).
    return v;
}

/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'
static ExprAST *parse_identifier_expr() {
    auto id_name = move(IDENTIFIER_STR);
    get_next_token();// eat identifier.

    // Simple variable ref.
    if (CUR_TOK != '(') return new VariableExprAST(id_name);
    // Call.
    get_next_token();// eat (
    std::vector<ExprAST *> args;
    while (CUR_TOK != ')') {
        auto a = parse_expression();
        if (!a) return nullptr;
        args.push_back(a);
        switch (CUR_TOK) {
            case ',':
                get_next_token();
            case ')':
                break;
            default:
                return error("Expected ')' or ',' in argument list");
        }
    }
    return new CallExprAST(id_name, args);
}

/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
static ExprAST *parse_primary() {
    switch (CUR_TOK) {
        case tok_identifier:
            return parse_identifier_expr();
        case tok_number:
            return parse_number_expr();
        case '(':
            return parse_paren_expr();
        default:
            std::cout << CUR_TOK << std::endl;
            return error("unknown token when expecting an expression");
    }
}


/// expression ::= primary binoprhs
static ExprAST *parse_expression() {
    auto lhs = parse_primary();
    return lhs ? parse_bin_op_rhs(0, lhs) : nullptr;
}

/// binoprhs ::= ('+' primary)*
static ExprAST *parse_bin_op_rhs(int expr_prec, ExprAST *lhs) {
    // If this is a binop, find its precedence.
    while (true) {
        auto tok_prec = get_tok_precedence();

        // If this is a binop that binds at least as tightly as the current binop,
        // consume it, otherwise we are done.
        if (tok_prec < expr_prec) return lhs;

        // Okay, we know this is a binop.
        auto bin_op = CUR_TOK;
        get_next_token();// eat binop

        // Parse the primary expression after the binary operator.
        auto rhs = parse_primary();
        if (!rhs) return nullptr;

        // If BinOp binds less tightly with RHS than the operator after RHS,
        // let the pending operator take RHS as its LHS.
        auto next_prec = get_tok_precedence();
        if (tok_prec < next_prec) {
            rhs = parse_bin_op_rhs(tok_prec + 1, rhs);
            if (!rhs) return nullptr;
        }

        // Merge LHS/RHS.
        lhs = new BinaryExprAST(bin_op, lhs, rhs);
    }// loop around to the top of the while loop.
}

/// prototype ::= id '(' id* ')'
static PrototypeAST *parse_prototype() {
    if (CUR_TOK != tok_identifier)
        return error_p("Expected function name in prototype");

    auto fn_name = move(IDENTIFIER_STR);
    get_next_token();

    if (CUR_TOK != '(')
        return error_p("Expected '(' in prototype");

    std::vector<std::string> arg_names;
    while (get_next_token() == tok_identifier)
        arg_names.push_back(move(IDENTIFIER_STR));
    if (CUR_TOK != ')')
        return error_p("Expected ')' in prototype");

    // success.
    get_next_token();// eat ')'.

    return new PrototypeAST(fn_name, arg_names);
}
