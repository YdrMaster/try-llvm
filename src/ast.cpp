#include "ast.h"
#include "lexer.h"

#include <iostream>
#include <map>

int CURRENT_TOKEN;
int get_next_token() { return CURRENT_TOKEN = get_token(); }

static std::unique_ptr<ExprAST> parse_number_expr();
static std::unique_ptr<ExprAST> parse_paren_expr();
static std::unique_ptr<ExprAST> parse_identifier_expr();
static std::unique_ptr<ExprAST> parse_primary();
static std::unique_ptr<ExprAST> parse_expression();
static std::unique_ptr<ExprAST> parse_bin_op_rhs(int expr_prec, std::unique_ptr<ExprAST> lhs);
static std::unique_ptr<PrototypeAST> parse_prototype();

/// definition ::= 'def' prototype expression
std::unique_ptr<FunctionAST> parse_definition() {
    get_next_token();// eat def.
    auto proto = parse_prototype();
    if (!proto) return nullptr;
    auto e = parse_expression();
    return e ? std::make_unique<FunctionAST>(std::move(proto), std::move(e)) : nullptr;
}

/// external ::= 'extern' prototype
std::unique_ptr<PrototypeAST> parse_extern() {
    get_next_token();// eat extern.
    return parse_prototype();
}

/// toplevelexpr ::= expression
std::unique_ptr<FunctionAST> parse_top_level_expr() {
    auto e = parse_expression();
    // Make an anonymous proto.
    return e ? std::make_unique<FunctionAST>(
                   std::make_unique<PrototypeAST>("__anon_expr", std::vector<std::string>()),
                   std::move(e))
             : nullptr;
}

/// log_error* - These are little helper functions for error handling.
std::unique_ptr<ExprAST> log_error(const char *str) {
    std::cerr << "error: " << str << std::endl;
    return nullptr;
}
static std::unique_ptr<PrototypeAST> log_error_p(const char *str) {
    log_error(str);
    return nullptr;
}

/// get_token_precedence - Get the precedence of the pending binary operator token.
static int get_token_precedence() {
    if (!isascii(CURRENT_TOKEN)) return -1;
    /// BINOP_PRECEDENCE - This holds the precedence for each binary operator that is defined.
    const static auto binop_precedence = std::map<char, int>{
        {'<', 10},
        {'+', 20},
        {'-', 20},
        {'*', 30},
    };
    // Make sure it's a declared binop.
    auto ans = binop_precedence.find(CURRENT_TOKEN);
    return ans != binop_precedence.end() ? ans->second : -1;
}

/// numberexpr ::= number
static std::unique_ptr<ExprAST> parse_number_expr() {
    auto ans = std::make_unique<NumberExprAST>(NUM_VAL);
    get_next_token();// consume the number
    return ans;
}

/// parenexpr ::= '(' expression ')'
static std::unique_ptr<ExprAST> parse_paren_expr() {
    get_next_token();// eat (.
    auto v = parse_expression();
    if (!v) return nullptr;

    if (CURRENT_TOKEN != ')') return log_error("expected ')'");
    get_next_token();// eat ).
    return v;
}

/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'
static std::unique_ptr<ExprAST> parse_identifier_expr() {
    auto id_name = std::move(IDENTIFIER_STR);
    get_next_token();// eat identifier.

    // Simple variable ref.
    if (CURRENT_TOKEN != '(') return std::make_unique<VariableExprAST>(id_name);
    // Call.
    get_next_token();// eat (
    std::vector<std::unique_ptr<ExprAST>> args;
    while (CURRENT_TOKEN != ')') {
        auto a = parse_expression();
        if (!a) return nullptr;
        args.emplace_back(std::move(a));
        switch (CURRENT_TOKEN) {
            case ',':
                get_next_token();
            case ')':
                break;
            default:
                return log_error("Expected ')' or ',' in argument list");
        }
    }
    // Eat the ')'.
    get_next_token();
    return std::make_unique<CallExprAST>(std::move(id_name), std::move(args));
}

/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
static std::unique_ptr<ExprAST> parse_primary() {
    switch (CURRENT_TOKEN) {
        case tok_identifier:
            return parse_identifier_expr();
        case tok_number:
            return parse_number_expr();
        case '(':
            return parse_paren_expr();
        case tok_if:
            return parse_if_expr();
        default:
            return log_error("unknown token when expecting an expression");
    }
}

/// expression ::= primary binoprhs
static std::unique_ptr<ExprAST> parse_expression() {
    auto lhs = parse_primary();
    return lhs ? parse_bin_op_rhs(0, std::move(lhs)) : nullptr;
}

/// binoprhs ::= ('+' primary)*
static std::unique_ptr<ExprAST> parse_bin_op_rhs(int expr_prec, std::unique_ptr<ExprAST> lhs) {
    // If this is a binop, find its precedence.
    while (true) {
        auto tok_prec = get_token_precedence();

        // If this is a binop that binds at least as tightly as the current binop,
        // consume it, otherwise we are done.
        if (tok_prec < expr_prec) return lhs;

        // Okay, we know this is a binop.
        auto bin_op = CURRENT_TOKEN;
        get_next_token();// eat binop

        // Parse the primary expression after the binary operator.
        auto rhs = parse_primary();
        if (!rhs) return nullptr;

        // If BinOp binds less tightly with RHS than the operator after RHS,
        // let the pending operator take RHS as its LHS.
        auto next_prec = get_token_precedence();
        if (tok_prec < next_prec) {
            if (!(rhs = parse_bin_op_rhs(tok_prec + 1, std::move(rhs)))) return nullptr;
        }

        // Merge LHS/RHS.
        lhs = std::make_unique<BinaryExprAST>(bin_op, std::move(lhs), std::move(rhs));
    }// loop around to the top of the while loop.
}

/// prototype ::= id '(' id* ')'
static std::unique_ptr<PrototypeAST> parse_prototype() {
    if (CURRENT_TOKEN != tok_identifier) return log_error_p("Expected function name in prototype");

    auto fn_name = std::move(IDENTIFIER_STR);
    get_next_token();

    if (CURRENT_TOKEN != '(') return log_error_p("Expected '(' in prototype");

    std::vector<std::string> arg_names;
    while (get_next_token() == tok_identifier) arg_names.emplace_back(std::move(IDENTIFIER_STR));
    if (CURRENT_TOKEN != ')') return log_error_p("Expected ')' in prototype");

    // success.
    get_next_token();// eat ')'.

    return std::make_unique<PrototypeAST>(std::move(fn_name), std::move(arg_names));
}

/// ifexpr ::= 'if' expression 'then' expression 'else' expression
static std::unique_ptr<ExprAST> parse_if_expr() {
    get_next_token();// eat the if.

    // condition.
    auto cond = parse_expression();
    if (!cond) return nullptr;

    if (CURRENT_TOKEN != tok_then)
        return log_error("expected then");
    get_next_token();// eat the then

    auto then = parse_expression();
    if (!then) return nullptr;

    if (CURRENT_TOKEN != tok_else)
        return log_error("expected else");

    get_next_token();

    auto else_ = parse_expression();
    if (!else_) return nullptr;

    return std::make_unique<IfExprAST>(
        std::move(cond),
        std::move(then),
        std::move(else_));
}
