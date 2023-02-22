#ifndef _BASIC_TYPES_HPP_
#define _BASIC_TYPES_HPP_

#include <string>
#include <string_view>
#include <vector>
#include <array>

namespace gvl
{
    static constexpr short args_max_num = 10;

    using Token = std::string;
    using TokenSv = std::string_view;

    enum class StatementType
    {
        READCHAR,
        READINT,
        READFLOAT,
        READSTR,
        READLN,
        PRINT,
        PRINTLN,
        INIT,
        CONST,
        ASSIGN,
        IF,
        ELSE,
        WHILE,
        BRACKET,
        ARRAY_INIT,
        ARRAY_APPEND,
        ARRAY_SET,
        ARRAY_POP,
        NONE
    };

    struct Expression
    {
        Token left;
        Token middle;
        Token right;
    };

    struct Statement
    {
        StatementType type;
        std::vector<Token> line;
        Expression expression;
        std::vector<Statement> main_body;
        std::vector<Statement> second_body;
    };

    struct Program
    {
        using StmtContainer = std::vector<Statement>;

        StmtContainer statements;
        std::array<Token, args_max_num> args;
    };
}


#endif