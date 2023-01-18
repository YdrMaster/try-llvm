#ifndef __AST_H__
#define __AST_H__

#include "KaleidoscopeJIT.h"

#include "llvm/IR/Function.h"

#include <memory>
#include <string>
#include <vector>

/// ExprAST - Base class for all expression nodes.
class ExprAST {
public:
    virtual ~ExprAST() {}
    virtual llvm::Value *codegen() = 0;
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
    double val;

public:
    explicit NumberExprAST(double val)
        : val(val) {}
    llvm::Value *codegen() override;
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
    std::string name;

public:
    explicit VariableExprAST(std::string name)
        : name(std::move(name)) {}
    llvm::Value *codegen() override;
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
    char op;
    std::unique_ptr<ExprAST> lhs, rhs;

public:
    BinaryExprAST(char op,
                  std::unique_ptr<ExprAST> lhs,
                  std::unique_ptr<ExprAST> rhs)
        : op(op),
          lhs(std::move(lhs)),
          rhs(std::move(rhs)) {}
    llvm::Value *codegen() override;
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
    std::string callee;
    std::vector<std::unique_ptr<ExprAST>> args;

public:
    CallExprAST(std::string callee,
                std::vector<std::unique_ptr<ExprAST>> args)
        : callee(std::move(callee)),
          args(std::move(args)) {}
    llvm::Value *codegen() override;
};

/// IfExprAST - Expression class for if/then/else.
class IfExprAST : public ExprAST {
    std::unique_ptr<ExprAST> cond, then, else_;

public:
    IfExprAST(std::unique_ptr<ExprAST> cond,
              std::unique_ptr<ExprAST> then,
              std::unique_ptr<ExprAST> else_)
        : cond(std::move(cond)),
          then(std::move(then)),
          else_(std::move(else_)) {}
    llvm::Value *codegen() override;
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names
/// (thus implicitly the number of arguments the function takes).
class PrototypeAST {
    std::string name;
    std::vector<std::string> args;

public:
    PrototypeAST(std::string name, std::vector<std::string> args)
        : name(std::move(name)), args(std::move(args)) {}
    llvm::Function *codegen();

    inline const auto &get_name() const { return name; }
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST {
    std::unique_ptr<PrototypeAST> proto;
    std::unique_ptr<ExprAST> body;

public:
    FunctionAST(std::unique_ptr<PrototypeAST> proto,
                std::unique_ptr<ExprAST> body)
        : proto(std::move(proto)),
          body(std::move(body)) {}
    llvm::Function *codegen();
};

/// CURRENT_TOKEN/getNextToken - Provide a simple token buffer.
/// CURRENT_TOKEN is the current token the parser is looking at.
/// get_next_token reads another token from the lexer and updates CurTok with its results.
extern int CURRENT_TOKEN;
int get_next_token();

std::unique_ptr<ExprAST> log_error(const char *);
std::unique_ptr<FunctionAST> parse_definition();
std::unique_ptr<FunctionAST> parse_top_level_expr();
std::unique_ptr<PrototypeAST> parse_extern();

extern llvm::ExitOnError EXIT_ON_ERROR;
extern std::unique_ptr<llvm::orc::KaleidoscopeJIT> THE_JIT;
void initialize_module_and_pass_manager();
void update_module_and_pass_manager();
void update_function_proto(std::unique_ptr<PrototypeAST> &&proto_ast);

#endif// __AST_H__
