#ifndef _INTERPRETER_HPP_
#define _INTERPRETER_HPP_

#include "Parser.hpp"
#include "Calculator.hpp"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <functional>


namespace gvl
{
    enum class VarLikeType
    {
        BOOL,
        INT,
        DOUBLE,
        STRING,
        ARRAY,
        NONE
    };
    
    struct VarLike
    {
        VarLikeType type=VarLikeType::NONE;
        TokenSv name;
        Token value;
        bool is_const=false;
        std::vector<Token> array_elements;
    };

    
    class Interpreter
    {
        public:

            class Error
            {};

            using VarLikeMap = std::unordered_map<TokenSv, VarLike>;
            using ExeFunc = std::function<Error(Interpreter&, const Statement&)>;
            using ExePlan = std::vector<std::pair<const Statement&, ExeFunc>>;


            Interpreter(const Program& program);

            void execute_program();

            inline const VarLikeMap& get_var_map() const { return variables; }

            inline const Calculator& get_calculator() const { return calculator; }

            void print_vars() const;

        private:

            static Error execute_init(Interpreter& interpreter, const Statement& stmt);

            static Error execute_array_init(Interpreter& interpreter, const Statement& stmt);

            static Error execute_array_append(Interpreter& interpreter, const Statement& stmt);

            static Error execute_array_pop(Interpreter& interpreter, const Statement& stmt);

            static Error execute_array_set(Interpreter& interpreter, const Statement& stmt);
            
            static Error execute_assign(Interpreter& interpreter, const Statement& stmt);
            
            static Error execute_print_related(Interpreter& interpreter, const Statement& stmt);
            
            static Error execute_read_related(Interpreter& interpreter, const Statement& stmt);
            
            static Error execute_block(Interpreter& interpreter, const Statement& stmt);

            static void clear_scope(Interpreter& interpreter, std::vector<TokenSv>& var_names);

        private:

            static VarLikeMap variables;
            static std::array<Token, args_max_num> args;
            static std::size_t block_lvl;

            std::vector<TokenSv> tmp_var_names;
            Calculator calculator;
            ExePlan exe_plan;
    };
}

#endif