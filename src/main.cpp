#include "ast.h"
#include "lexer.h"

#include "llvm/Support/raw_ostream.h"

#include <iostream>

/// top ::= definition | external | expression | ';'
int main() {
    std::cout << "ready> ";
    std::cout.flush();
    get_next_token();
    initialize_module();
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
                        // Remove the anonymous expression.
                        fn_ir->eraseFromParent();
                    }
                }
                // Skip token for error recovery.
                else
                    get_next_token();
                break;
        }
    }
}
