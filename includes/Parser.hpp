#ifndef _PARSER_HPP_
#define _PARSER_HPP_

#include <string>
#include <vector>
#include <array>
#include <sstream>
#include "basic_types.hpp"


namespace gvl
{
    class Parser
    {
        public:

            class ParseTimeError : public Error
            {
                public:

                    ParseTimeError(const std::string& error_msg, std::size_t error_line_no)
                        : Error(error_msg, error_line_no)
                    {}
            };

        public:

            static std::vector<std::string> split_to_lines(std::istringstream& iss);

            static std::istringstream read_file_content(const std::string& file_name, std::vector<char>& buffer);

            Parser(const std::vector<std::string>& lines, const std::array<std::string, gvl::args_max_num>* args);

            inline const Program& get_parsed_program() const { return this->parsed_program; }

            static inline const std::size_t get_line_no() { return line_no; }

        private:
            
            static std::size_t line_no;

            Program parsed_program;
    };
}

#endif