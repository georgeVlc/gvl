#ifndef _INTERPRETER_HPP_
#define _INTERPRETER_HPP_

#include "Parser.hpp"
#include "Calculator.hpp"
#include <unordered_map>
#include <unordered_set>
#include <set>
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

    
    //bool operator==(const std::pair<TokenSv, Statement*>& lhs, std::pair<TokenSv, Statement*>& rhs) 
    //{ return lhs.first == rhs.first && lhs.second == rhs.second; }

    struct HashTokenStmtPair
    {
        std::size_t operator()(const std::pair<TokenSv, Statement*>& p) const { return std::hash<TokenSv>()(p.first) + std::hash<Statement*>()(p.second); }
    };

    class Interpreter
    {
        public:

            class Info
            {};

            using VarLikeMap = std::unordered_map<TokenSv, VarLike>;
            using ExeFunc = std::function<Info(Interpreter&, const Statement&)>;
            using ExePlan = std::vector<std::pair<const Statement&, ExeFunc>>;


            Interpreter(const Program& program);

            void execute_program();

            inline const VarLikeMap& get_var_map() const { return variables; }

            inline const Calculator& get_calculator() const { return calculator; }

            inline const std::unordered_map<TokenSv, char> get_format_keywords() const { return format_keywords; }

            inline const std::unordered_set<std::pair<TokenSv, Statement*>, HashTokenStmtPair> get_ud_funcs() const { return ud_funcs; }

            inline const ExePlan& get_exe_plan() const { return exe_plan; }

            void print_vars() const;

        private:

            static Info execute_init(Interpreter& interpreter, const Statement& stmt);

            static Info execute_array_init(Interpreter& interpreter, const Statement& stmt);

            static Info execute_array_append(Interpreter& interpreter, const Statement& stmt);

            static Info execute_array_pop(Interpreter& interpreter, const Statement& stmt);

            static Info execute_array_set(Interpreter& interpreter, const Statement& stmt);
            
            static Info execute_assign(Interpreter& interpreter, const Statement& stmt);
            
            static Info execute_print_related(Interpreter& interpreter, const Statement& stmt);
            
            static Info execute_read_related(Interpreter& interpreter, const Statement& stmt);
            
            static Info execute_block(Interpreter& interpreter, const Statement& stmt);

            static Info execute_call_func(Interpreter& interpreter, const Statement& stmt);

            static void clear_scope(Interpreter& interpreter, std::vector<TokenSv>& var_names);

        private:

            static VarLikeMap variables;
            static std::array<Token, args_max_num> args;
            static std::size_t block_lvl;
            static std::unordered_map<TokenSv, char> format_keywords;
            static std::unordered_set<std::pair<TokenSv, Statement*>, HashTokenStmtPair> ud_funcs;
            
            std::vector<TokenSv> tmp_var_names;
            Calculator calculator;
            ExePlan exe_plan;
    };
}

#endif