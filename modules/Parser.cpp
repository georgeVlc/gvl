#include "../includes/Parser.hpp"
#include <string>
#include <vector>
#include <array>
#include <iterator>
#include <fstream>
#include <memory>
#include <cassert>
#include <iostream>
#include <string_view>


std::size_t gvl::Parser::line_no = 1;


static bool valid_stmt_tokens_no(std::uint32_t low, std::uint32_t high, std::uint32_t sz)
{
    bool is_valid = (sz < low || sz > high) ? false : true;
    return is_valid;
}

std::istringstream gvl::Parser::read_file_content(const std::string& file_name, std::vector<char>& buffer)
{
    std::ifstream inob(file_name, std::ios_base::in | std::ios::binary);

    inob.seekg(0,std::ios::end);
    std::streampos length = inob.tellg();
    inob.seekg(0,std::ios::beg);

    buffer.reserve(length);
    inob.read(&buffer[0], length);

    std::istringstream iss;
    iss.rdbuf()->pubsetbuf(&buffer[0], length);
    
    inob.close();

    return iss;
}

std::vector<std::string> gvl::Parser::split_to_lines(std::istringstream& iss)
{
    std::vector<std::string> lines;
    std::string line;

    while (std::getline(iss, line, '\n'))
    {
        if (line.starts_with("#"))
            continue;
        lines.push_back(line);
    }

    return lines;
}

static std::vector<std::string> split_string_into_vector(const std::string& str)
{
    std::stringstream ss(str);
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> vstrings(begin, end);
    
    return vstrings;
}

static gvl::StatementType set_statement_type(const std::vector<gvl::Token>& tokens)
{
    if (tokens.size() == 1)
        throw gvl::Parser::ParseTimeError{ "invalid statement type", gvl::Parser::get_line_no() };

    using gvl::StatementType;
    StatementType type = 
    
    tokens.front() == "var" ? StatementType::INIT :
    tokens.front() == "var[]" ? StatementType::ARRAY_INIT :
    tokens.front() == "$array_append" ? StatementType::ARRAY_APPEND :
    tokens.front() == "$array_set" ? StatementType::ARRAY_SET :
    tokens.front() == "$array_pop" ? StatementType::ARRAY_POP :
    tokens.front() == "const" ? StatementType::CONST :
    tokens.front() == "print" ? StatementType::PRINT :
    tokens.front() == "println" ? StatementType::PRINTLN :
    tokens.front() == "readchar" ? StatementType::READCHAR :
    tokens.front() == "readint" ? StatementType::READINT :
    tokens.front() == "readfloat" ? StatementType::READFLOAT :
    tokens.front() == "readstr" ? StatementType::READSTR :
    tokens.front() == "readln" ? StatementType::READLN : 
    tokens.front() == "if" ? StatementType::IF :
    tokens.front() == "else" ? StatementType::ELSE :
    tokens.front() == "while" ? StatementType::WHILE :
    tokens.front() == "}" ? StatementType::BRACKET :
    tokens.front() == "function" ? StatementType::DEF_FUNC :
    tokens.front() == "call" ? StatementType::CALL_FUNC :
    tokens.front() == "return" ? StatementType::RETURN :
    tokens[1] == "=" ? StatementType::ASSIGN : StatementType::NONE;

    if (type == StatementType::NONE)
        throw gvl::Parser::ParseTimeError{ "invalid statement type", gvl::Parser::get_line_no() };


    return type;
}

static bool statement_is_block(gvl::StatementType type)
{
    return 
        type == gvl::StatementType::IF || type == gvl::StatementType::ELSE || 
        type == gvl::StatementType::WHILE || type == gvl::StatementType::DEF_FUNC;
}

static gvl::Expression set_statement_expression(gvl::StatementType type, const std::vector<gvl::Token>& tokens)
{
    gvl::Expression expression;

    if (type == gvl::StatementType::ASSIGN)
    {
        expression.left = tokens[2];
        const std::size_t sz = tokens.size();

        if (!valid_stmt_tokens_no(3, 5, sz))
            throw gvl::Parser::ParseTimeError{ "invalid number of tokens", gvl::Parser::get_line_no() };

        if (sz >= 4)
        {
            expression.middle = tokens[3];
            if (sz >= 5)
                expression.right = tokens[4];
        }
    }    
    else if (type == gvl::StatementType::INIT || type == gvl::StatementType::CONST)
    {
        expression.left = tokens[3];
        const std::size_t sz = tokens.size();

        if (!valid_stmt_tokens_no(4, 6, sz))
            throw gvl::Parser::ParseTimeError{ "invalid number of tokens", gvl::Parser::get_line_no() };

        if (sz >= 5)
        {
            expression.middle = tokens[4];
            if (sz >= 6)
                expression.right = tokens[5];
        }
    }
    else if (type == gvl::StatementType::ARRAY_INIT)
    {
        const std::size_t sz = tokens.size();

        if (!valid_stmt_tokens_no(4, 7, sz))
            throw gvl::Parser::ParseTimeError{ "invalid number of tokens", gvl::Parser::get_line_no() };

        if (sz == 5)
        {
            if (tokens[3].compare("$array_pop") == 0)
            {
                expression.left = tokens[3];
                expression.middle = tokens[4];
                return expression;
            }
        }
        else if (sz >= 6)
        {
            if (tokens[3].compare("$array_at") == 0)
            {
                expression.left = tokens[3];
                expression.middle = tokens[4];
                expression.right = tokens[5];
                return expression;
            }

            expression.left = tokens[4];

            if (sz >= 7)
            {
                expression.middle = tokens[5];
                if (sz == 8)
                    expression.right = tokens[6];
                else
                    throw gvl::Parser::ParseTimeError{ "invalid number of tokens", gvl::Parser::get_line_no() };
            }
        }
    }
    else if (type == gvl::StatementType::ARRAY_APPEND)
    {
        const std::size_t sz = tokens.size();

        if (sz == 3)
        {
            expression.left = tokens[1];
            expression.middle = tokens[2];
        }
        else
            throw gvl::Parser::ParseTimeError{ "invalid number of tokens", gvl::Parser::get_line_no() };

    }
    else if (type == gvl::StatementType::ARRAY_SET)
    {
        const std::size_t sz = tokens.size();

        if (sz == 4)
        {
            expression.left = tokens[1];
            expression.middle = tokens[2];
            expression.right = tokens[3];
        }
        else
            throw gvl::Parser::ParseTimeError{ "invalid number of tokens", gvl::Parser::get_line_no() };

    }
    else if (type == gvl::StatementType::DEF_FUNC)  // function name : arg1 arg2 arg3 {
    {
        const std::size_t sz = tokens.size();

        if (sz >= 4)
        {
            expression.left = tokens[3];
            if (sz >= 5)
            {
                expression.middle = tokens[4];
                if (sz >= 6)
                    expression.right = tokens[5];
            }
        }
    }
    else if (type == gvl::StatementType::CALL_FUNC)     // call name arg1 arg2 arg3
    {
        const std::size_t sz = tokens.size();
        
        if (sz >= 3)
        {
            expression.left = tokens[2];
            if (sz >= 4)
            {
                expression.middle = tokens[3];
                if (sz >= 5)
                    expression.right = tokens[4];
            }
        }
    }
    else if (type != gvl::StatementType::ELSE && type != gvl::StatementType::BRACKET)
    {
        expression.left = tokens[1];
        const std::size_t sz = tokens.size();
        
        if (sz >= 3)
        {
            expression.middle = tokens[2];
            if (sz >= 4)
                expression.right = tokens[3];
        }
    }

    return expression;
}

static std::vector<gvl::Statement> set_statement_body(const std::vector<std::string>& lines, auto& it)
{
    std::vector<gvl::Statement> body;
    std::vector<std::string> sub_lines;
    std::size_t cbracket_cnt = 1;
    ++it;

    for (; it != lines.end(); ++it)
    {
        if ((*it).back() == '}')
        {
            --cbracket_cnt;
            if (cbracket_cnt == 0)
                break;
        }
        else if ((*it).back() == '{')
            ++cbracket_cnt;

        sub_lines.push_back(*it);
    }

    gvl::Parser p(sub_lines, nullptr);
    body = p.get_parsed_program().statements;
    
    return body;
}

gvl::Parser::Parser(const std::vector<std::string>& lines, const std::array<std::string, gvl::args_max_num>* args)
{
    if (args != nullptr)
        this->parsed_program.args = *args;

    for (auto it = lines.begin(); it != lines.end(); ++it)
    {
        if ((*it).empty())
        {
            ++this->line_no;
            continue;
        }

        Statement stmt;

        stmt.line = split_string_into_vector(*it);

        if (stmt.line.empty())
            continue;

        stmt.type = set_statement_type(stmt.line);
        
        stmt.expression = set_statement_expression(stmt.type, stmt.line);

        if (statement_is_block(stmt.type))
            stmt.main_body = set_statement_body(lines, it);

        this->parsed_program.statements.push_back(stmt);
        ++this->line_no;
    }
}