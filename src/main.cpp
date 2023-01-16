#include "ast.h"
#include "lexer.h"

#include <iostream>
#include <thread>

/// top ::= definition | external | expression | ';'
int main() {
    std::thread x([] { std::cout << "chaos"; });
    std::cout << "ready> ";
    std::cout.flush();
    get_next_token();
    while (true) {
        std::cout << "ready> ";
        std::cout.flush();
        switch (CUR_TOK) {
            case tok_eof:
                return 0;
            case ';':
                get_next_token();
                break;// ignore top-level semicolons.
            case tok_def:
                if (parse_definition())
                    std::cout << "Parsed a function definition." << std::endl;
                // Skip token for error recovery.
                else
                    get_next_token();
                break;
            case tok_extern:
                if (parse_extern())
                    std::cout << "Parsed an extern." << std::endl;
                // Skip token for error recovery.
                else
                    get_next_token();
                break;
            default:
                // Evaluate a top-level expression into an anonymous function.
                if (parse_top_level_expr())
                    std::cout << "Parsed a top-level expr." << std::endl;
                // Skip token for error recovery.
                else
                    get_next_token();
                break;
        }
    }
}
