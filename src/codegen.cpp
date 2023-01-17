﻿#include "ast.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

#include <map>

std::unique_ptr<llvm::LLVMContext> THE_CONTEXT;
std::unique_ptr<llvm::Module> THE_MODULE;
std::unique_ptr<llvm::IRBuilder<>> BUILDER;
std::map<std::string, llvm::Value *> NAMED_VALUES;

static llvm::Value *log_error_v(const char *str) {
    log_error(str);
    return nullptr;
}

void initialize_module() {
    // Open a new context and module.
    THE_CONTEXT = std::make_unique<llvm::LLVMContext>();
    THE_MODULE = std::make_unique<llvm::Module>("my cool jit", *THE_CONTEXT);

    // Create a new builder for the module.
    BUILDER = std::make_unique<llvm::IRBuilder<>>(*THE_CONTEXT);
}

llvm::Value *NumberExprAST::codegen() {
    return llvm::ConstantFP::get(*THE_CONTEXT, llvm::APFloat(val));
}

llvm::Value *VariableExprAST::codegen() {
    // Look this variable up in the function.
    const auto v = NAMED_VALUES[name];
    if (!v) return log_error_v("Unknown variable name");
    return v;
}

llvm::Value *BinaryExprAST::codegen() {
    auto l = lhs->codegen(), r = rhs->codegen();
    if (!l || !r) return nullptr;

    switch (op) {
        case '+':
            return BUILDER->CreateFAdd(l, r, "addtmp");
        case '-':
            return BUILDER->CreateFSub(l, r, "subtmp");
        case '*':
            return BUILDER->CreateFMul(l, r, "multmp");
        case '<':
            l = BUILDER->CreateFCmpULT(l, r, "cmptmp");
            // Convert bool 0/1 to double 0.0 or 1.0
            return BUILDER->CreateUIToFP(l, llvm::Type::getDoubleTy(*THE_CONTEXT), "booltmp");
        default:
            return log_error_v("invalid binary operator");
    }
}

llvm::Value *CallExprAST::codegen() {
    // Look up the name in the global module table.
    const auto callee_f = THE_MODULE->getFunction(callee);
    if (!callee_f)
        return log_error_v("Unknown function referenced");

    // If argument mismatch error.
    if (callee_f->arg_size() != args.size())
        return log_error_v("Incorrect # arguments passed");

    std::vector<llvm::Value *> args_v;
    for (unsigned i = 0, e = args.size(); i != e; ++i) {
        args_v.push_back(args[i]->codegen());
        if (!args_v.back()) return nullptr;
    }

    return BUILDER->CreateCall(callee_f, args_v, "calltmp");
}

llvm::Function *PrototypeAST::codegen() {
    // Make the function type: double(double,double) etc.
    std::vector<llvm::Type *> doubles(args.size(), llvm::Type::getDoubleTy(*THE_CONTEXT));
    auto ft = llvm::FunctionType::get(llvm::Type::getDoubleTy(*THE_CONTEXT), doubles, false);
    auto f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, name, THE_MODULE.get());
    // Set names for all arguments.
    unsigned idx = 0;
    for (auto &arg : f->args()) arg.setName(args[idx++]);
    return f;
}

llvm::Function *FunctionAST::codegen() {
    // First, check for an existing function from a previous 'extern' declaration.
    auto the_function = THE_MODULE->getFunction(proto->get_name());
    if (!the_function) the_function = proto->codegen();
    if (!the_function) return nullptr;
    // Create a new basic block to start insertion into.
    auto bb = llvm::BasicBlock::Create(*THE_CONTEXT, "entry", the_function);
    BUILDER->SetInsertPoint(bb);

    // Record the function arguments in the NAMED_VALUES map.
    NAMED_VALUES.clear();
    for (auto &arg : the_function->args())
        NAMED_VALUES[std::string(arg.getName())] = &arg;

    if (auto ret_val = body->codegen()) {
        // Finish off the function.
        BUILDER->CreateRet(ret_val);
        // Validate the generated code, checking for consistency.
        llvm::verifyFunction(*the_function);
        return the_function;
    }

    // Error reading body, remove function.
    the_function->eraseFromParent();
    return nullptr;
}
