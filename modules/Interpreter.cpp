#include "../includes/Interpreter.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <iostream>
#include <cctype>
#include <map>
#include <cassert>


gvl::Interpreter::VarLikeMap gvl::Interpreter::variables;
std::array<gvl::Token, gvl::args_max_num> gvl::Interpreter::args;
std::size_t gvl::Interpreter::block_lvl = 0;


static gvl::VarLikeType get_varlike_type(gvl::TokenSv sv)
{
    const auto& non_alpha = 
        std::find_if(sv.begin(), sv.end(), [](char c) { return !std::isalpha(c) && !std::isspace(c); });

    using namespace std::string_view_literals;

    if (non_alpha == sv.end())
    {
        if (sv.compare("true"sv) == 0 || sv.compare("false"sv) == 0)
            return gvl::VarLikeType::BOOL;
        else
            return gvl::VarLikeType::STRING;
    }
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

    try {
        result = interpreter.get_var_map().at(sv).value;
    } catch (std::out_of_range&) {
        result = sv;  
    }

    return result;
}

struct ExpressionEvaluation
{
    gvl::Token result;
    gvl::VarLikeType type=gvl::VarLikeType::NONE;
    std::vector<gvl::Token> array_values;
};

static ExpressionEvaluation evaluate_expression(const gvl::Interpreter& interpreter, 
    const gvl::Statement& stmt, gvl::VarLikeType known_type=gvl::VarLikeType::NONE)
{
    using namespace std::string_view_literals;
    using gvl::VarLikeType;
    using gvl::Token;
    using gvl::TokenSv;

    Token l, r, tmp, result, index;
    ExpressionEvaluation expreval;    
        

    if (stmt.expression.left.compare("array_at"sv) == 0)
    {
        index = get_varlike_value(interpreter, stmt.expression.right);
        
        result = interpreter.get_var_map().at
        (stmt.expression.middle).array_elements.at(std::stoi(index));

        try {
            const auto& array_elements = interpreter.get_var_map().at(result).array_elements;
            if (!array_elements.empty())
            {
                expreval.type = gvl::VarLikeType::ARRAY;
                for (const auto& element : array_elements)
                    expreval.array_values.push_back(element);
            }

        } catch (std::out_of_range&) { expreval.type = get_varlike_type(result); }
        
        expreval.result = result;
        return expreval;
    }
    else if (stmt.expression.left.compare("array_len"sv) == 0)
    {
        expreval.result = std::to_string(interpreter.get_var_map().at(stmt.expression.middle).array_elements.size());
        expreval.type = VarLikeType::INT;
        return expreval;
    }
    else if (stmt.expression.left.compare("array_pop"sv) == 0)
    {
        const gvl::VarLike& varlike = interpreter.get_var_map().at(stmt.expression.middle);
        
        assert(!varlike.array_elements.empty());

        const auto& back = varlike.array_elements.back();

        expreval.result = back;
        expreval.type = get_varlike_type(back);

        try {
            const std::vector<Token>& elements = interpreter.get_var_map().at(back).array_elements;
            expreval.type = VarLikeType::ARRAY;

            while (!elements.empty())
            {
                expreval.array_values.push_back(elements.back());
                const_cast<std::vector<Token>&> (elements).pop_back();

            }
        } catch (std::out_of_range&) { const_cast<std::vector<Token>&> (varlike.array_elements).pop_back(); }

        return expreval;
    }

    result = get_varlike_value(interpreter, stmt.expression.left);

    if (!result.empty())
        l = result;
    else
        l = stmt.expression.left;

    result = get_varlike_value(interpreter, stmt.expression.right);

    if (!result.empty())
        r = result;
    else
        r = stmt.expression.right;

    VarLikeType l_type = get_varlike_type(l);
    assert(l_type != VarLikeType::NONE);
    VarLikeType r_type = r.empty() ? VarLikeType::NONE : get_varlike_type(r);

    // check if l middle r is valid
    
    tmp = l;
    tmp += stmt.expression.middle;
    tmp += r;

    if (l_type == VarLikeType::STRING || r_type == VarLikeType::STRING)
    {
        expreval.result = tmp;
        expreval.type = VarLikeType::STRING;
        return expreval;
    }
    else if (l_type == VarLikeType::BOOL || r_type == VarLikeType::BOOL)
    {
        expreval.result = tmp;
        expreval.type = VarLikeType::BOOL;
        return expreval;
    }
    else
    {
        Calculator calculator = interpreter.get_calculator();
        calculator.set_expression(tmp);

        if (l_type == VarLikeType::INT && (r_type == VarLikeType::INT || r_type == VarLikeType::NONE))
        {
            expreval.result = std::to_string(calculator.evaluate<int>());
            expreval.type = VarLikeType::INT;
        }
        else if (l_type == VarLikeType::DOUBLE || r_type == VarLikeType::DOUBLE)
        {
            expreval.result = std::to_string(calculator.evaluate<double>());
            expreval.type = VarLikeType::DOUBLE;
        }
        
        return expreval;
    }
}

gvl::Interpreter::Error gvl::Interpreter::execute_init(Interpreter& interpreter, const Statement& stmt)
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

    return gvl::Interpreter::Error(); 
}

gvl::Interpreter::Error gvl::Interpreter::execute_assign(Interpreter& interpreter, const Statement& stmt)
{ 
    const Token& name = stmt.line.front();
    VarLike& varlike = interpreter.variables.at(name);

    if (!varlike.is_const)
        varlike.value = evaluate_expression(interpreter, stmt, varlike.type).result;

    return gvl::Interpreter::Error(); 
}

static void print_token(const gvl::Interpreter& interpreter, gvl::TokenSv token, char delim=' ')
{
    using namespace std::string_view_literals;

    if (interpreter.get_var_map().contains(token))
        std::cout << interpreter.get_var_map().at(token).value << delim;
    else if (token.starts_with("nl"sv))
        std::cout << "\n"sv;
    else if (token.starts_with("tabl"sv))
        std::cout << "\t"sv;
    else if (token.starts_with("space"sv))
        std::cout << " "sv;
    else if (!token.empty())
        std::cout << token << delim;
}

gvl::Interpreter::Error gvl::Interpreter::execute_print_related(Interpreter& interpreter, const Statement& stmt)
{ 
    const Expression& tokens = stmt.expression;
    
    print_token(interpreter, tokens.left);
    print_token(interpreter, tokens.middle);
    print_token(interpreter, tokens.right);
    
    using namespace std::string_view_literals;
    
    if (stmt.type == StatementType::PRINTLN)
        std::cout << "\n"sv;

    return gvl::Interpreter::Error(); 
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
    }
}

gvl::Interpreter::Error gvl::Interpreter::execute_read_related(Interpreter& interpreter, const Statement& stmt)
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

    return gvl::Interpreter::Error(); 
}

gvl::Interpreter::Error gvl::Interpreter::execute_block(Interpreter& interpreter, const Statement& stmt)
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
    bool is_true = calculator.evaluate_basic_bool_expression<double>(left_operand, right_operand, oper);

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


    return gvl::Interpreter::Error();
}

static void add_element_in_array(const gvl::Interpreter& interpreter, gvl::VarLike& array, const gvl::Token& element)
{
    if (!element.empty())
        array.array_elements.push_back(element);
} 

gvl::Interpreter::Error gvl::Interpreter::execute_array_init(Interpreter& interpreter, const Statement& stmt)
{
    VarLike array;
    array.name = stmt.line[1];
    array.value = array.name;
    array.type = VarLikeType::ARRAY;
    array.is_const = false;

    const Token index = get_varlike_value(interpreter, stmt.expression.right);

    if (!interpreter.variables.contains(array.name))
    {
        if (stmt.expression.left.compare("array_at") == 0)
        {
            const auto& right_side_array_name = 
            interpreter.get_var_map().at(stmt.expression.middle).array_elements.at(std::stoi(index));

            for (const auto& array_elem : interpreter.get_var_map().at(right_side_array_name).array_elements)
                add_element_in_array(interpreter, array, get_varlike_value(interpreter, array_elem));
        }
        else if (stmt.expression.left.compare("array_pop") == 0)
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

    return gvl::Interpreter::Error();
}

gvl::Interpreter::Error gvl::Interpreter::execute_array_set(Interpreter& interpreter, const Statement& stmt)
{
    const VarLike& target_array = interpreter.get_var_map().at(stmt.expression.left);

    if (!target_array.is_const)
    {
        const Token& idx(get_varlike_value(interpreter, stmt.expression.middle));
        const Token& set_value(get_varlike_value(interpreter, stmt.expression.right));
        
        const std::vector<Token>& target_array_values(target_array.array_elements);
        const_cast<std::vector<Token>&> (target_array_values).at(std::stoi(idx)) = set_value;
    }

    return gvl::Interpreter::Error();
}

gvl::Interpreter::Error gvl::Interpreter::execute_array_append(Interpreter& interpreter, const Statement& stmt)
{
    VarLike& varlike = interpreter.variables.at(stmt.line[1]);
    varlike.array_elements.push_back(get_varlike_value(interpreter, stmt.expression.middle));

    return gvl::Interpreter::Error();
}

gvl::Interpreter::Error gvl::Interpreter::execute_array_pop(Interpreter& interpreter, const Statement& stmt)
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

    return gvl::Interpreter::Error();
}

void gvl::Interpreter::clear_scope(gvl::Interpreter& interpreter, std::vector<TokenSv>& var_names)
{
    for (const auto& var_name : var_names)
    {
        try
        {
            //std::cout << "\tDeleting: " << var_name << " value: " << Interpreter::variables.at(var_name).value << std::endl;
            Interpreter::variables.erase(var_name);  
        }
        catch (...)
        {
            //std::cout << var_name << " doesnt exist in vars\n";
        }
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
        if (p.second)
            p.second(*this, p.first);
        else
            return;
    }
}