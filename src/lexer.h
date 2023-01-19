#ifndef __LEXER_H__
#define __LEXER_H__

#include <string>

/// The lexer returns tokens [0-255] if it is an unknown character,
/// otherwise one of these for known things.
enum Token {
    tok_eof = -1,

    // commands
    tok_def = -2,
    tok_extern = -3,

    // primary
    tok_identifier = -4,
    tok_number = -5,

    // control
    tok_if = -6,
    tok_then = -7,
    tok_else = -8,
    tok_for = -9,
    tok_in = -10,
};

extern std::string IDENTIFIER_STR;// Filled in if tok_identifier
extern double NUM_VAL;            // Filled in if tok_number

/// get_token - Return the next token from standard input.
int get_token();

#endif// __LEXER_H__
