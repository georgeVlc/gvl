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


int main(int argc, char** argv)
{
    assert(argc >= 2 && argc - 2 <= gvl::args_max_num);

    std::vector<char> buffer;
    std::istringstream iss(gvl::Parser::read_file_content(argv[1], buffer));
    
    std::vector<std::string> lines(gvl::Parser::split_to_lines(iss));
    
    std::array<std::string, gvl::args_max_num> args;
    
    for (int i = 2; i < argc; ++i)
        args[i] = argv[i];

    try 
    {
        gvl::Parser parser(lines, &args);

        gvl::Interpreter interpreter(parser.get_parsed_program());

        interpreter.execute_program();

        interpreter.print_vars();

    }
    catch (const gvl::Parser::ParseTimeError& e) 
    {
        std::cout << e.what() << "\n"; 
    }

}