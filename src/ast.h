#ifndef __AST_H__
#define __AST_H__

#include <memory>
#include <string>
#include <vector>

/// ExprAST - Base class for all expression nodes.
class ExprAST {
public:
    virtual ~ExprAST() {}
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
    double val;

public:
    explicit NumberExprAST(double val)
        : val(val) {}
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
    std::string name;

public:
    explicit VariableExprAST(std::string name)
        : name(move(name)) {}
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
    char op;
    std::unique_ptr<ExprAST> lhs, rhs;

public:
    BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
        : op(op), lhs(move(lhs)), rhs(move(rhs)) {}
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
    std::string callee;
    std::vector<std::unique_ptr<ExprAST>> args;

public:
    CallExprAST(std::string callee, std::vector<std::unique_ptr<ExprAST>> args)
        : callee(move(callee)), args(move(args)) {}
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class PrototypeAST {
    std::string name;
    std::vector<std::string> args;

public:
    PrototypeAST(std::string name, std::vector<std::string> args)
        : name(move(name)), args(move(args)) {}

    inline const auto &get_name() const { return name; }
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST {
    std::unique_ptr<PrototypeAST> proto;
    std::unique_ptr<ExprAST> body;

public:
    FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body)
        : proto(move(proto)), body(move(body)) {}
};

/// CUR_TOK/getNextToken - Provide a simple token buffer.
/// CUR_TOK is the current token the parser is looking at.
/// get_next_token reads another token from the lexer and updates CurTok with its results.
extern int CUR_TOK;
int get_next_token();

std::unique_ptr<FunctionAST> parse_definition();
std::unique_ptr<PrototypeAST> parse_extern();
std::unique_ptr<FunctionAST> parse_top_level_expr();

#endif// __AST_H__
