#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <string>

enum Token {
    tok_eof = -1,
    // commands
    tok_def = -2,
    tok_extern = -3,
    // primary
    tok_identifier = -4,
    tok_number = -5,
};

extern std::string IDENTIFIER_STR;
extern double NUM_VAL;

int get_token();

#endif// __TOKEN_H__
