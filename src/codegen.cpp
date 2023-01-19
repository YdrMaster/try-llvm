#include "ast.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

#include <map>

static std::unique_ptr<llvm::LLVMContext> THE_CONTEXT;
static std::unique_ptr<llvm::Module> THE_MODULE;
static std::unique_ptr<llvm::IRBuilder<>> BUILDER;
static std::map<std::string, llvm::Value *> NAMED_VALUES;
static std::unique_ptr<llvm::legacy::FunctionPassManager> THE_FPM;
static std::map<std::string, std::unique_ptr<PrototypeAST>> FUNCTION_PROTOS;

static llvm::Value *log_error_v(const char *str) {
    log_error(str);
    return nullptr;
}
static llvm::Function *get_function(const std::string &name) {
    // First, see if the function has already been added to the current module.
    if (auto *f = THE_MODULE->getFunction(name)) return f;

    // If not, check whether we can codegen the declaration from some existing prototype.
    auto fi = FUNCTION_PROTOS.find(name);
    if (fi != FUNCTION_PROTOS.end()) return fi->second->codegen();

    // If no existing prototype exists, return null.
    return nullptr;
}

llvm::ExitOnError EXIT_ON_ERROR;
std::unique_ptr<llvm::orc::KaleidoscopeJIT> THE_JIT;
void initialize_module_and_pass_manager() {
    // Open a new context and module.
    THE_CONTEXT = std::make_unique<llvm::LLVMContext>();
    THE_MODULE = std::make_unique<llvm::Module>("my cool jit", *THE_CONTEXT);
    THE_MODULE->setDataLayout(THE_JIT->getDataLayout());

    // Create a new builder for the module.
    BUILDER = std::make_unique<llvm::IRBuilder<>>(*THE_CONTEXT);

    // Create a new pass manager attached to it.
    THE_FPM = std::make_unique<llvm::legacy::FunctionPassManager>(THE_MODULE.get());

    // Do simple "peephole" optimizations and bit-twiddling optzns.
    THE_FPM->add(llvm::createInstructionCombiningPass());
    // Reassociate expressions.
    THE_FPM->add(llvm::createReassociatePass());
    // Eliminate Common SubExpressions.
    THE_FPM->add(llvm::createGVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    THE_FPM->add(llvm::createCFGSimplificationPass());

    THE_FPM->doInitialization();
}
void update_module_and_pass_manager() {
    EXIT_ON_ERROR(THE_JIT->addModule(llvm::orc::ThreadSafeModule(std::move(THE_MODULE), std::move(THE_CONTEXT))));
    initialize_module_and_pass_manager();
}
void update_function_proto(std::unique_ptr<PrototypeAST> &&proto_ast) {
    FUNCTION_PROTOS[proto_ast->get_name()] = std::move(proto_ast);
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
    const auto callee_f = get_function(callee);
    if (!callee_f)
        return log_error_v("Unknown function referenced");

    // If argument mismatch error.
    if (callee_f->arg_size() != args.size())
        return log_error_v("Incorrect # arguments passed");

    std::vector<llvm::Value *> args_v;
    for (auto &arg : args) {
        const auto c = arg->codegen();
        if (!c) return nullptr;
        args_v.push_back(c);
    }

    return BUILDER->CreateCall(callee_f, args_v, "calltmp");
}

llvm::Function *PrototypeAST::codegen() {
    // Make the function type: double(double,double,...) etc.
    const auto ty_double = llvm::Type::getDoubleTy(*THE_CONTEXT);
    auto ft = llvm::FunctionType::get(ty_double, std::vector<llvm::Type *>(args.size(), ty_double), false);
    auto f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, name, THE_MODULE.get());
    // Set names for all arguments.
    unsigned idx = 0;
    for (auto &arg : f->args()) arg.setName(args[idx++]);
    return f;
}

llvm::Function *FunctionAST::codegen() {
    // Transfer ownership of the prototype to the FunctionProtos map.
    const auto name = proto->get_name();
    update_function_proto(std::move(proto));
    auto the_function = get_function(name);
    if (!the_function) return nullptr;

    // Create a new basic block to start insertion into.
    auto bb = llvm::BasicBlock::Create(*THE_CONTEXT, "entry", the_function);
    BUILDER->SetInsertPoint(bb);

    // Record the function arguments in the NamedValues map.
    NAMED_VALUES.clear();
    for (auto &arg : the_function->args())
        NAMED_VALUES[std::string(arg.getName())] = &arg;

    if (auto ret_val = body->codegen()) {
        // Finish off the function.
        BUILDER->CreateRet(ret_val);
        // Validate the generated code, checking for consistency.
        llvm::verifyFunction(*the_function);
        // Run the optimizer on the function.
#ifdef USE_OPT
        THE_FPM->run(*the_function);
#endif
        return the_function;
    } else {
        // Error reading body, remove function.
        the_function->eraseFromParent();
        return nullptr;
    }
}

llvm::Value *IfExprAST::codegen() {
    auto cond_v = cond->codegen();
    if (!cond_v) return nullptr;

    // Convert condition to a bool by comparing non-equal to 0.0.
    cond_v = BUILDER->CreateFCmpONE(cond_v, llvm::ConstantFP::get(*THE_CONTEXT, llvm::APFloat(0.0)), "ifcond");

    auto the_function = BUILDER->GetInsertBlock()->getParent();

    // Create blocks for the then and else cases.
    // Insert the 'then' block at the end of the function.
    auto then_bb = llvm::BasicBlock::Create(*THE_CONTEXT, "then", the_function);
    auto else_bb = llvm::BasicBlock::Create(*THE_CONTEXT, "else");
    auto merge_bb = llvm::BasicBlock::Create(*THE_CONTEXT, "ifcont");

    BUILDER->CreateCondBr(cond_v, then_bb, else_bb);

    // Emit then value.
    BUILDER->SetInsertPoint(then_bb);

    auto then_v = then->codegen();
    if (!then_v) return nullptr;

    BUILDER->CreateBr(merge_bb);
    // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
    then_bb = BUILDER->GetInsertBlock();

    // Emit else block.
    the_function->getBasicBlockList().push_back(else_bb);
    BUILDER->SetInsertPoint(else_bb);

    auto else_v = else_->codegen();
    if (!else_v) return nullptr;

    BUILDER->CreateBr(merge_bb);
    // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
    else_bb = BUILDER->GetInsertBlock();

    // Emit merge block.
    the_function->getBasicBlockList().push_back(merge_bb);
    BUILDER->SetInsertPoint(merge_bb);
    auto pn = BUILDER->CreatePHI(llvm::Type::getDoubleTy(*THE_CONTEXT), 2, "iftmp");

    pn->addIncoming(then_v, then_bb);
    pn->addIncoming(else_v, else_bb);
    return pn;
}

llvm::Value *ForExprAST::codegen() {
    // Emit the start code first, without 'variable' in scope.
    auto start_val = start->codegen();
    if (!start_val) return nullptr;

    // Make the new basic block for the loop header, inserting after current block.
    auto the_function = BUILDER->GetInsertBlock()->getParent();
    auto preheader_bb = BUILDER->GetInsertBlock();
    auto loop_bb = llvm::BasicBlock::Create(*THE_CONTEXT, "loop", the_function);

    // Insert an explicit fall through from the current block to the LoopBB.
    BUILDER->CreateBr(loop_bb);

    // Start insertion in LoopBB.
    BUILDER->SetInsertPoint(loop_bb);

    // Start the PHI node with an entry for Start.
    auto variable = BUILDER->CreatePHI(llvm::Type::getDoubleTy(*THE_CONTEXT), 2, var_name.c_str());
    variable->addIncoming(start_val, preheader_bb);

    // Within the loop, the variable is defined equal to the PHI node.
    // If it shadows an existing variable, we have to restore it, so save it now.
    auto [it, b] = NAMED_VALUES.insert(std::make_pair(var_name, variable));
    auto old_val = b ? nullptr : std::exchange(it->second, variable);

    // Emit the body of the loop. This, like any other expr, can change the current BB.
    // Note that we ignore the value computed by the body, but don't allow an error.
    if (!body->codegen()) return nullptr;

    // Emit the step value.
    auto step_val = step ? step->codegen() : llvm::ConstantFP::get(*THE_CONTEXT, llvm::APFloat(1.0));
    if (!step_val) return nullptr;

    auto next_var = BUILDER->CreateFAdd(variable, step_val, "nextvar");

    // Compute the end condition.
    auto end_cond = end->codegen();
    if (!end_cond) return nullptr;

    // Convert condition to a bool by comparing non-equal to 0.0.
    end_cond = BUILDER->CreateFCmpONE(end_cond, llvm::ConstantFP::get(*THE_CONTEXT, llvm::APFloat(0.0)), "loopcond");

    // Create the "after loop" block and insert it.
    auto loop_end_bb = BUILDER->GetInsertBlock();
    auto after_bb = llvm::BasicBlock::Create(*THE_CONTEXT, "afterloop", the_function);

    // Insert the conditional branch into the end of LoopEndBB.
    BUILDER->CreateCondBr(end_cond, loop_bb, after_bb);

    // Any new code will be inserted in AfterBB.
    BUILDER->SetInsertPoint(after_bb);

    // Add a new entry to the PHI node for the backedge.
    variable->addIncoming(next_var, loop_end_bb);

    // Restore the unshadowed variable.
    if (old_val)
        NAMED_VALUES[var_name] = old_val;
    else
        NAMED_VALUES.erase(var_name);

    // for expr always returns 0.0.
    return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*THE_CONTEXT));
}
