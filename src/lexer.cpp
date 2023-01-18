#include "lexer.h"

#include <cctype>
#include <utility>

std::string IDENTIFIER_STR;
double NUM_VAL;

int get_token() {
    static int LAST_CHAR = ' ';
    // Skip any whitespace.
    while (isspace(LAST_CHAR)) LAST_CHAR = getchar();
    // Identifier: [a-zA-Z][a-zA-Z0-9]*
    if (isalpha(LAST_CHAR)) {
        std::string str;
        do {
            str += std::exchange(LAST_CHAR, getchar());
        } while (isalnum(LAST_CHAR));

        IDENTIFIER_STR = std::move(str);
        return IDENTIFIER_STR == "extern" ? tok_extern
               : IDENTIFIER_STR == "def"  ? tok_def
               : IDENTIFIER_STR == "if"   ? tok_if
               : IDENTIFIER_STR == "then" ? tok_then
               : IDENTIFIER_STR == "else" ? tok_else
                                          : tok_identifier;
    }
    // Number: [0-9.]+
    if (isdigit(LAST_CHAR) || LAST_CHAR == '.') {
        std::string str;
        do {
            str += std::exchange(LAST_CHAR, getchar());
        } while (isdigit(LAST_CHAR) || LAST_CHAR == '.');

        NUM_VAL = strtod(str.c_str(), nullptr);
        return tok_number;
    }
    // Comment until end of line.
    if (LAST_CHAR == '#')
        while (true)
            switch (LAST_CHAR = getchar()) {
                case EOF:
                    return tok_eof;
                case '\r':
                case '\n':
                    return get_token();
                default:
                    break;
            }
    // Check for end of file. Don't eat the EOF.
    // Otherwise, just return the character as its ascii value.
    return LAST_CHAR == EOF ? tok_eof : std::exchange(LAST_CHAR, getchar());
}
