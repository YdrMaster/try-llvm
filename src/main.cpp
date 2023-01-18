#include "ast.h"
#include "lexer.h"

#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/Support/TargetSelect.h"

#include <iostream>

/// top ::= definition | external | expression | ';'
int main() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    std::cout << "ready> ";
    std::cout.flush();
    get_next_token();

    THE_JIT = EXIT_ON_ERROR(llvm::orc::KaleidoscopeJIT::Create());
    initialize_module_and_pass_manager();

    while (true) {
        std::cout << "ready> ";
        std::cout.flush();
        switch (CURRENT_TOKEN) {
            case tok_eof:
                return 0;

            case ';':
                get_next_token();
                break;// ignore top-level semicolons.

            case tok_def:
                if (auto fn_ast = parse_definition()) {
                    if (auto fn_ir = fn_ast->codegen()) {
                        std::cout << "Parsed a function definition." << std::endl;
                        fn_ir->print(llvm::outs());
                        std::cout << std::endl;
                    }
                }
                // Skip token for error recovery.
                else
                    get_next_token();
                break;

            case tok_extern:
                if (auto proto_ast = parse_extern()) {
                    if (auto fn_ir = proto_ast->codegen()) {
                        std::cout << "Parsed an extern:" << std::endl;
                        fn_ir->print(llvm::outs());
                        std::cout << std::endl;
                    }
                }
                // Skip token for error recovery.
                else
                    get_next_token();
                break;

            default:
                // Evaluate a top-level expression into an anonymous function.
                if (auto fn_ast = parse_top_level_expr()) {
                    if (auto fn_ir = fn_ast->codegen()) {
                        std::cout << "Parsed a top-level expr:" << std::endl;
                        fn_ir->print(llvm::outs());
                        std::cout << std::endl;

                        // Create a ResourceTracker to track JIT'd memory allocated to our
                        // anonymous expression -- that way we can free it after executing.
                        auto rt = THE_JIT->getMainJITDylib().createResourceTracker();

                        update_module_and_pass_manager();

                        // Search the JIT for the __anon_expr symbol.
                        auto expr_symbol = EXIT_ON_ERROR(THE_JIT->lookup("__anon_expr"));

                        // Get the symbol's address and cast it to the right type (takes no
                        // arguments, returns a double) so we can call it as a native function.
                        auto fp = (double (*)()) expr_symbol.getAddress();
                        std::cout << "Evaluated to " << fp() << std::endl;

                        // Delete the anonymous expression module from the JIT.
                        EXIT_ON_ERROR(rt->remove());
                    }
                }
                // Skip token for error recovery.
                else
                    get_next_token();
                break;
        }
    }
}
