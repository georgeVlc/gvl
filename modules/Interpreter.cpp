#include "../includes/Interpreter.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <iostream>
#include <cctype>
#include <map>
#include <set>
#include <cassert>
#include <ranges>
#include <algorithm>


std::ostream& operator<<(std::ostream& out, const gvl::VarLikeType type)
{
    static std::map<gvl::VarLikeType, std::string> strings;
    if (strings.size() == 0)
    {
        #define INSERT_ELEMENT(p) strings[p] = #p
                INSERT_ELEMENT(gvl::VarLikeType::BOOL);     
                INSERT_ELEMENT(gvl::VarLikeType::DOUBLE);     
                INSERT_ELEMENT(gvl::VarLikeType::INT);
                INSERT_ELEMENT(gvl::VarLikeType::STRING);
                INSERT_ELEMENT(gvl::VarLikeType::ARRAY);             
                INSERT_ELEMENT(gvl::VarLikeType::NONE);                
        #undef INSERT_ELEMENT
    }   

    return out << strings[type];
}


gvl::Interpreter::VarLikeMap gvl::Interpreter::variables;

std::array<gvl::Token, gvl::args_max_num> gvl::Interpreter::args;

std::size_t gvl::Interpreter::block_lvl = 0;

std::unordered_map<gvl::TokenSv, char> gvl::Interpreter::format_keywords = { 
    std::pair<gvl::TokenSv, char>("nl", '\n'),
    std::pair<gvl::TokenSv, char>("tab", '\t'),
    std::pair<gvl::TokenSv, char>("space", ' ')
};

std::unordered_set<std::pair<gvl::TokenSv, gvl::Statement*>, gvl::HashTokenStmtPair> gvl::Interpreter::ud_funcs;


static bool is_number(const gvl::TokenSv& tokenSv)
{
    return std::find_if(tokenSv.begin(), 
        tokenSv.end(), [](unsigned char c) { return !std::isdigit(c) && c != '.'; }) == tokenSv.end();
}

static gvl::VarLikeType get_varlike_type(gvl::TokenSv sv)
{
    using namespace std::string_view_literals;

    if (sv.empty())
        return gvl::VarLikeType::NONE;

    if (sv.compare("true"sv) == 0 || sv.compare("false"sv) == 0)
        return gvl::VarLikeType::BOOL;
    else if (sv.starts_with('\'') || !is_number(sv))     // prev cond:sv.starts_with('\'') || std::count_if(sv.begin(),sv.end(),[](char c){ return !(std::isalnum(c)); }) <= 1
        return gvl::VarLikeType::STRING;
    else
    {
        const auto& non_digit =
            std::find_if(sv.begin(), sv.end(), [](char c) { return !std::isdigit(c); });

        if (non_digit == sv.end())
            return gvl::VarLikeType::INT;

        if (static_cast<std::string_view>(non_digit).starts_with("."sv))
        {
            const auto& not_zero = 
                std::find_if(non_digit, sv.end(), [](char c) { return c != '0'; });

            if (not_zero == sv.end())
                return gvl::VarLikeType::INT;

            return gvl::VarLikeType::DOUBLE; 
        }
    }

    return gvl::VarLikeType::NONE;
}

static void print_array_elements(const auto& array_elements, const gvl::Interpreter::VarLikeMap& vmap)
{
    for (const auto& element : array_elements)
    {
        if (vmap.contains(element))
        {
            std::cout << " [ ";
            print_array_elements( vmap.at(element).array_elements, vmap);
            std::cout << "] ";
        }
        else
            std::cout << element << " ";
    }
}

void gvl::Interpreter::print_vars() const
{
    using namespace std::string_view_literals;

    for (const auto& [ name, varlike ] : this->variables)
    {
        std::cout << "Name: "sv << name << "\tValue: "sv << varlike.value
        << "\tType: "sv << varlike.type;
        
        if (!varlike.array_elements.empty())
        {
            std::cout << "\tArray Elements: [ "sv;
            print_array_elements(varlike.array_elements, this->variables);
            std::cout << "]"sv;
        }
        std::cout << "\n"sv;
    }
    
}

static gvl::Token get_varlike_value(const gvl::Interpreter interpreter, gvl::TokenSv sv)
{
    gvl::Token result;

    try { result = interpreter.get_var_map().at(sv).value; } catch (std::out_of_range&) { result = sv; }

    return result;
}

struct ExpressionEvaluation
{
    gvl::Token result;
    gvl::VarLikeType type=gvl::VarLikeType::NONE;
    std::vector<gvl::Token> array_values;
};

static std::pair<gvl::Token, gvl::VarLikeType> get_value_and_type(gvl::TokenSv tokenSv, const gvl::Interpreter& interpreter)
{
    std::pair<gvl::Token, gvl::VarLikeType> pair;

    try 
    {
        gvl::VarLike vl = interpreter.get_var_map().at(tokenSv);
        pair.first = vl.value;
    } 
    catch (const std::out_of_range&) 
    { 
        
        if (interpreter.get_format_keywords().contains(tokenSv))
            pair.first = interpreter.get_format_keywords().at(tokenSv);
        else
            pair.first = tokenSv;
    }

    pair.second = get_varlike_type(pair.first);

    return pair;
}

static ExpressionEvaluation evaluate_expression(const gvl::Interpreter& interpreter, const gvl::Statement& stmt)
{
    using namespace std::string_view_literals;
    using namespace gvl;

    Token l, r, tmp, result, index;
    ExpressionEvaluation expreval;    
        

    if (stmt.expression.left.compare("$array_at"sv) == 0)
    {
        index = get_varlike_value(interpreter, stmt.expression.right);
        
        result = interpreter.get_var_map().at
        (stmt.expression.middle).array_elements.at(std::stoi(index));

        try 
        {
            const auto& array_elements = interpreter.get_var_map().at(result).array_elements;
            if (!array_elements.empty())
            {
                expreval.type = gvl::VarLikeType::ARRAY;
                for (const auto& element : array_elements)
                    expreval.array_values.push_back(element);
            }

        } 
        catch (std::out_of_range&) { expreval.type = get_varlike_type(result); }
        
        expreval.result = result;
        return expreval;
    }
    else if (stmt.expression.left.compare("$array_len"sv) == 0)
    {
        expreval.result = std::to_string(interpreter.get_var_map().at(stmt.expression.middle).array_elements.size());
        expreval.type = VarLikeType::INT;
        return expreval;
    }
    else if (stmt.expression.left.compare("$array_pop"sv) == 0)
    {
        const gvl::VarLike& varlike = interpreter.get_var_map().at(stmt.expression.middle);
        
        assert(!varlike.array_elements.empty());

        const auto& back = varlike.array_elements.back();

        expreval.result = back;
        expreval.type = get_varlike_type(back);

        try 
        {
            const std::vector<Token>& elements = interpreter.get_var_map().at(back).array_elements;
            expreval.type = VarLikeType::ARRAY;

            while (!elements.empty())
            {
                expreval.array_values.push_back(elements.back());
                const_cast<std::vector<Token>&> (elements).pop_back();

            }
        } 
        catch (std::out_of_range&) { const_cast<std::vector<Token>&> (varlike.array_elements).pop_back(); }

        return expreval;
    }

    VarLike vl_left, vl_right;
    VarLikeType l_type = VarLikeType::NONE, r_type = VarLikeType::NONE;

    std::pair<Token, VarLikeType> pair = get_value_and_type(stmt.expression.left, interpreter);
    l_type = pair.second;
    l = pair.first;

    if (!stmt.expression.right.empty())
    {
        pair = get_value_and_type(stmt.expression.right, interpreter);
        r_type = pair.second;
        r = pair.first;
    }

    assert(l_type != VarLikeType::NONE);

    tmp = l;
    
    if (l_type == VarLikeType::STRING)
    {
        if (interpreter.get_format_keywords().contains(stmt.expression.middle))
            tmp += interpreter.get_format_keywords().at(stmt.expression.middle);
        tmp += r;
    }
    else if (l_type == VarLikeType::BOOL)
    {
        tmp += ' ';
        tmp += stmt.expression.middle;
        if (!stmt.expression.right.empty())
        {
            tmp += ' ';
            tmp += stmt.expression.right;
        }
    }
    else
    {
        tmp += stmt.expression.middle;
        tmp += r;
    }

    //std::cout << "l: " << l << " l_type: " << l_type << 
    //" r:" << r << " r_type: " << r_type << " tmp: " << tmp << "\n";

    if (l_type == VarLikeType::STRING)
    {
        tmp.erase(std::remove(tmp.begin(), tmp.end(), '\''), tmp.end());
        expreval.result = tmp;
        expreval.type = VarLikeType::STRING;
        return expreval;
    }
    else
    {
        Calculator calculator = interpreter.get_calculator();
        calculator.set_expression(tmp);

        if (l_type == VarLikeType::INT)
        {
            if (r_type == VarLikeType::DOUBLE)
            {
                expreval.result = std::to_string(calculator.evaluate<double>());
                expreval.type = VarLikeType::DOUBLE;
            }
            else
            {
                expreval.result = std::to_string(calculator.evaluate<int>());
                expreval.type = VarLikeType::INT;
            }
        }
        else if (l_type == VarLikeType::DOUBLE)
        {
            expreval.result = std::to_string(calculator.evaluate<double>());
            expreval.type = VarLikeType::DOUBLE;
        }
        else if (l_type == VarLikeType::BOOL)
        {
            expreval.result = r.empty() ? l : calculator.evaluate_basic_bool_expression(l, r, stmt.expression.middle) ?
                "true" : "false";
            expreval.type = VarLikeType::BOOL;
        }
        
        return expreval;
    }
}

gvl::Interpreter::Info gvl::Interpreter::execute_init(Interpreter& interpreter, const Statement& stmt)
{ 
    VarLike varlike;
    varlike.name = stmt.line[1];

    if (!interpreter.variables.contains(varlike.name))
    {
        const ExpressionEvaluation& expreval = evaluate_expression(interpreter, stmt);
        varlike.value = expreval.result;
        varlike.array_elements = expreval.array_values;
        varlike.type = expreval.type;
        varlike.is_const = stmt.type == StatementType::CONST;
        
        interpreter.variables[varlike.name] = varlike;
        
        if (Interpreter::block_lvl > 0)
            interpreter.tmp_var_names.push_back(varlike.name);
    }

    return gvl::Interpreter::Info(); 
}

gvl::Interpreter::Info gvl::Interpreter::execute_assign(Interpreter& interpreter, const Statement& stmt)
{ 
    const Token& name = stmt.line.front();
    VarLike& varlike = interpreter.variables.at(name);

    if (!varlike.is_const)
        varlike.value = evaluate_expression(interpreter, stmt).result;


    return gvl::Interpreter::Info(); 
}

static void print_token(const gvl::Interpreter& interpreter, gvl::TokenSv token)
{
    using namespace std::string_view_literals;

    if (interpreter.get_var_map().contains(token))
        std::cout << interpreter.get_var_map().at(token).value;
    else if (interpreter.get_format_keywords().contains(token))
        std::cout << interpreter.get_format_keywords().at(token);
    else if (!token.empty())
    {
        if (token.find('\'') != std::string_view::npos)
            std::copy(token.begin() + 1, token.end() - 1, std::ostream_iterator<char>(std::cout, ""));
        else
            std::cout << token;
    }
}

gvl::Interpreter::Info gvl::Interpreter::execute_print_related(Interpreter& interpreter, const Statement& stmt)
{ 
    const Expression& tokens = stmt.expression;
    
    print_token(interpreter, tokens.left);
    print_token(interpreter, tokens.middle);
    print_token(interpreter, tokens.right);
    
    using namespace std::string_view_literals;
    
    if (stmt.type == StatementType::PRINTLN)
        std::cout << "\n"sv;

    return gvl::Interpreter::Info(); 
}

template <typename T>
    requires std::integral<T> || std::floating_point<T>
static void read_token(gvl::Interpreter& interpreter, gvl::TokenSv token)
{
    if (!token.empty())
    {
        T value = T();
        std::cin >> value;
        const_cast<gvl::VarLike&> (interpreter.get_var_map().at(token)).value = std::to_string(value);
        std::cin.ignore(std::numeric_limits<std::streamsize>::max());
    }
}

gvl::Interpreter::Info gvl::Interpreter::execute_read_related(Interpreter& interpreter, const Statement& stmt)
{
    if (stmt.type == gvl::StatementType::READINT)
    {
        read_token<int>(interpreter, stmt.expression.left);
        read_token<int>(interpreter, stmt.expression.middle);
        read_token<int>(interpreter, stmt.expression.right);
    }
    else if (stmt.type == gvl::StatementType::READFLOAT)
    {
        read_token<float>(interpreter, stmt.expression.left);
        read_token<float>(interpreter, stmt.expression.middle);
        read_token<float>(interpreter, stmt.expression.right);
    }

    return gvl::Interpreter::Info(); 
}

gvl::Interpreter::Info gvl::Interpreter::execute_block(Interpreter& interpreter, const Statement& stmt)
{
    bool is_true = stmt.type == StatementType::DEF_FUNC ? true : false;

    if (stmt.type != StatementType::DEF_FUNC)
    {
        Token left_operand, right_operand, oper;
        TokenSv result;

        result = get_varlike_value(interpreter, stmt.expression.left);

        if (!result.empty())
            left_operand = result;
        else
            left_operand = stmt.expression.left;
        
        result = get_varlike_value(interpreter, stmt.expression.right);

        if (!result.empty())
            right_operand = result;
        else
            right_operand = stmt.expression.right;
        
        oper = stmt.expression.middle;
            
        Calculator calculator;
        is_true = calculator.evaluate_basic_expression<double>(left_operand, right_operand, oper);
    }

    if (is_true)
    {
        ++Interpreter::block_lvl;
        Program program;
        program.statements = stmt.main_body;
        program.args = interpreter.args;
        Interpreter sub(program);
        sub.execute_program();
        --Interpreter::block_lvl;
        
        clear_scope(sub, sub.tmp_var_names);

        if (stmt.type == StatementType::WHILE)
            return execute_block(interpreter, stmt);
    }

    return gvl::Interpreter::Info();
}

static void add_element_in_array(const gvl::Interpreter& interpreter, gvl::VarLike& array, const gvl::Token& element)
{
    if (!element.empty())
        array.array_elements.push_back(element);
} 

gvl::Interpreter::Info gvl::Interpreter::execute_array_init(Interpreter& interpreter, const Statement& stmt)
{
    VarLike array;
    array.name = stmt.line[1];
    array.value = array.name;
    array.type = VarLikeType::ARRAY;
    array.is_const = false;

    const Token index = get_varlike_value(interpreter, stmt.expression.right);

    if (!interpreter.variables.contains(array.name))
    {
        if (stmt.expression.left.compare("$array_at") == 0)
        {
            const auto& right_side_array_name = 
            interpreter.get_var_map().at(stmt.expression.middle).array_elements.at(std::stoi(index));

            for (const auto& array_elem : interpreter.get_var_map().at(right_side_array_name).array_elements)
                add_element_in_array(interpreter, array, get_varlike_value(interpreter, array_elem));
        }
        else if (stmt.expression.left.compare("$array_pop") == 0)
        {
            const auto& right_side_array_name = interpreter.get_var_map().at(stmt.expression.middle).array_elements.back();
            
            for (const auto& array_elem : interpreter.get_var_map().at(right_side_array_name).array_elements)
                add_element_in_array(interpreter, array, get_varlike_value(interpreter, array_elem));
            
            const_cast<std::vector<Token>&> (interpreter.get_var_map().at(stmt.expression.middle).array_elements).pop_back();
        }
        else
        {
            add_element_in_array(interpreter, array, get_varlike_value(interpreter, stmt.expression.left));
            add_element_in_array(interpreter, array, get_varlike_value(interpreter, stmt.expression.middle));
            add_element_in_array(interpreter, array, get_varlike_value(interpreter, stmt.expression.right));
        }

        interpreter.variables[array.name] = array;
        
        if (Interpreter::block_lvl > 0)
            interpreter.tmp_var_names.push_back(array.name);
    }

    return gvl::Interpreter::Info();
}

gvl::Interpreter::Info gvl::Interpreter::execute_array_set(Interpreter& interpreter, const Statement& stmt)
{
    const VarLike& target_array = interpreter.get_var_map().at(stmt.expression.left);

    if (!target_array.is_const)
    {
        const Token& idx(get_varlike_value(interpreter, stmt.expression.middle));
        const Token& set_value(get_varlike_value(interpreter, stmt.expression.right));
        
        const std::vector<Token>& target_array_values(target_array.array_elements);
        const_cast<std::vector<Token>&> (target_array_values).at(std::stoi(idx)) = set_value;
    }

    return gvl::Interpreter::Info();
}

gvl::Interpreter::Info gvl::Interpreter::execute_array_append(Interpreter& interpreter, const Statement& stmt)
{
    VarLike& varlike = interpreter.variables.at(stmt.line[1]);
    varlike.array_elements.push_back(get_varlike_value(interpreter, stmt.expression.middle));

    return gvl::Interpreter::Info();
}

gvl::Interpreter::Info gvl::Interpreter::execute_array_pop(Interpreter& interpreter, const Statement& stmt)
{
    VarLike& varlike = interpreter.variables.at(stmt.line[1]);
    const auto& result = get_varlike_value(interpreter, stmt.expression.middle);

    try 
    {
        const auto& array_elemenets = interpreter.variables.at(result).array_elements;
        if (!array_elemenets.empty())
        {
            for (auto it = array_elemenets.begin(); it != array_elemenets.end(); ++it)
                varlike.array_elements.pop_back();
        }
    } catch (std::out_of_range&) 
    {
        varlike.array_elements.pop_back();
    }

    return gvl::Interpreter::Info();
}

gvl::Interpreter::Info gvl::Interpreter::execute_call_func(Interpreter& interpreter, const Statement& stmt)
{
    const std::unordered_set<std::pair<TokenSv, Statement*>, HashTokenStmtPair>& udfs(interpreter.get_ud_funcs());

    auto it = std::find_if(udfs.begin(), udfs.end(), 
        [stmt](const std::pair<TokenSv, Statement*>& p){ return p.first.compare(stmt.line[1]) == 0; });
    

    if (it != udfs.end())
    {
        Statement& func_stmt = *(it->second);
        std::unordered_map<Token, Token> params;

        if (!stmt.expression.left.empty())
            params.insert(std::pair<Token, Token>(func_stmt.expression.left, stmt.expression.left));
        if (!stmt.expression.middle.empty())
            params.insert(std::pair<Token, Token>(func_stmt.expression.middle, stmt.expression.middle));
        if (!stmt.expression.right.empty())
            params.insert(std::pair<Token, Token>(func_stmt.expression.right, stmt.expression.right)); 

        for (Statement& sub_stmt : func_stmt.main_body)
        {
            for (Token& token : sub_stmt.line)
            {
                if (params.contains(token))
                    token = params[token];
            }

            if (params.contains(sub_stmt.expression.left))
                sub_stmt.expression.left = params[sub_stmt.expression.left];
            if (params.contains(sub_stmt.expression.left))
                sub_stmt.expression.middle = params[sub_stmt.expression.middle];
            if (params.contains(sub_stmt.expression.right))
                sub_stmt.expression.right = params[sub_stmt.expression.right];            
        }
        
        interpreter.execute_block(interpreter, *(it->second));
    }

    return gvl::Interpreter::Info();
}

void gvl::Interpreter::clear_scope(gvl::Interpreter& interpreter, std::vector<TokenSv>& var_names)
{
    for (const auto& var_name : var_names)
    {
        try { Interpreter::variables.erase(var_name); }
        catch (...) {}
    }

    var_names.clear();
}

static bool is_read_related(gvl::StatementType type)
{
    using gvl::StatementType;
    return type == StatementType::READCHAR || type == StatementType::READFLOAT || 
            type == StatementType::READINT || type == StatementType::READLN || 
            type == StatementType::READSTR;
}

gvl::Interpreter::Interpreter(const Program& program)
{
    const Program::StmtContainer& stmts = program.statements;
    this->args = program.args;

    {
        VarLike vl;
        vl.is_const = true;
        vl.name = "$ARGS";
        vl.value = "$ARGS";
        vl.type = VarLikeType::ARRAY;
        
        for (const Token& arg : this->args)
        {
            if (!arg.empty())
                vl.array_elements.push_back(arg);
        }
        
        variables[vl.name] = vl;
    }

    for (const Statement& stmt : stmts)
    {
        StatementType type = stmt.type;
        ExeFunc f;

        type == StatementType::INIT || type == StatementType::CONST ? f = execute_init :
        type == StatementType::ARRAY_INIT ? f = execute_array_init :
        type == StatementType::ARRAY_APPEND ? f = execute_array_append :
        type == StatementType::ARRAY_POP ? f = execute_array_pop :
        type == StatementType::ARRAY_SET ? f = execute_array_set :
        type == StatementType::ASSIGN ? f = execute_assign :
        type == StatementType::CALL_FUNC ? f = execute_call_func :
        type == StatementType::PRINT || stmt.type == StatementType::PRINTLN ? f = execute_print_related :
        is_read_related(stmt.type) ? f = execute_read_related :
        type == StatementType::IF || type == StatementType::WHILE ? f = execute_block : f = nullptr;

        this->exe_plan.push_back(std::pair<const Statement&, ExeFunc>(stmt, f));
    }
}

void gvl::Interpreter::execute_program()
{
    for (const auto& p : this->exe_plan)
    {
        if (p.first.type == StatementType::DEF_FUNC)
            this->ud_funcs.insert(std::pair<TokenSv, Statement*>(const_cast<Token&>(p.first.line[1]), const_cast<Statement*>(&p.first)));
        else if (p.second)
            p.second(*this, p.first);
        else
            return;
    }
}