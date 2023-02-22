#include <iostream>
#include <cassert>
#include <vector>
#include <array>
#include <string>
#include <sstream>
#include <ranges>
#include <algorithm>
#include "includes/Parser.hpp"
#include "includes/Interpreter.hpp"
#include <map>


std::ostream& operator<<(std::ostream& out, const gvl::StatementType value)
{
    static std::map<gvl::StatementType, std::string> strings;
    if (strings.size() == 0)
    {
        #define INSERT_ELEMENT(p) strings[p] = #p
                INSERT_ELEMENT(gvl::StatementType::READCHAR);     
                INSERT_ELEMENT(gvl::StatementType::READINT);     
                INSERT_ELEMENT(gvl::StatementType::READFLOAT);             
                INSERT_ELEMENT(gvl::StatementType::READSTR);     
                INSERT_ELEMENT(gvl::StatementType::READLN);     
                INSERT_ELEMENT(gvl::StatementType::PRINT);             
                INSERT_ELEMENT(gvl::StatementType::PRINTLN);     
                INSERT_ELEMENT(gvl::StatementType::INIT);     
                INSERT_ELEMENT(gvl::StatementType::CONST);     
                INSERT_ELEMENT(gvl::StatementType::ASSIGN);             
                INSERT_ELEMENT(gvl::StatementType::IF);             
                INSERT_ELEMENT(gvl::StatementType::ELSE);     
                INSERT_ELEMENT(gvl::StatementType::WHILE);
                INSERT_ELEMENT(gvl::StatementType::BRACKET);
                INSERT_ELEMENT(gvl::StatementType::ARRAY_INIT);   
                INSERT_ELEMENT(gvl::StatementType::ARRAY_APPEND);     
                INSERT_ELEMENT(gvl::StatementType::ARRAY_POP);     
                INSERT_ELEMENT(gvl::StatementType::ARRAY_SET);     
                INSERT_ELEMENT(gvl::StatementType::NONE);                     
        #undef INSERT_ELEMENT
    }   

    return out << strings[value];
}

std::ostream& operator<<(std::ostream& out, const gvl::Statement& stmt)
{
    out << "----------------------------------------------------------\n";

    out << stmt.type << "\n";

    for (const auto& token : stmt.line)
        out << token << "|";

    out << "\n";

    out << stmt.expression.left << "|" << stmt.expression.middle << "|" << stmt.expression.right << "\n";
    
    if (!stmt.main_body.empty())
    {
        out << "\t\t++++++++++++++++-PRINTING BODY-+++++++++++++++++\n";
        for (const auto& sub_stmt : stmt.main_body)
        {
            out << sub_stmt << "\n";
        }
    }

    return out;
}


int main(int argc, char** argv)
{
    assert(argc >= 2 && argc - 2 <= gvl::args_max_num);

    std::vector<char> buffer;
    std::istringstream iss(gvl::Parser::read_file_content(argv[1], buffer));
    
    std::vector<std::string> lines(gvl::Parser::split_to_lines(iss));
    
    //std::ranges::copy(lines, std::ostream_iterator<std::string>(std::cout, "\n"));
    

    std::array<std::string, gvl::args_max_num> args;
    
    for (int i = 2; i < argc; ++i)
        args[i] = argv[i];


    gvl::Parser parser(lines, &args);

    const auto& stmts = parser.get_parsed_program().statements;

    for (const auto& stmt : stmts)
        std::cout << stmt << "\n";

    std::cout << "\n----------------------=EXECUTING GVL PROGRAM=----------------------\n";

    gvl::Interpreter interpreter(parser.get_parsed_program());
    interpreter.execute_program();

    std::cout << "\n----------------------=END OF GVL PROGRAM EXECUTION=----------------------\n";   
    interpreter.print_vars();
}