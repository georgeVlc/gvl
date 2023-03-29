#ifndef _BASIC_TYPES_HPP_
#define _BASIC_TYPES_HPP_

#include <string>
#include <string_view>
#include <sstream>
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
        CALL_FUNC,
        DEF_FUNC,
        RETURN,
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

    class Error
    {
        public:

            Error(const std::string& error, std::size_t line_no)
               : error_msg(error), error_line_no(line_no)
            {}

            virtual std::string what() const 
            {
                std::ostringstream oss;
                oss << error_msg << ", at line: " << error_line_no;
                return oss.str(); 
            }


        protected:

            std::string error_msg;
            std::size_t error_line_no;
    };
}


#endif