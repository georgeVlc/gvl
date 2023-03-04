#ifndef _CALCULATOR_HPP_
#define _CALCULATOR_HPP_

#include <string_view>
#include <string>
#include <sstream>
#include <stack>
#include <concepts>
#include <cmath>


class Calculator
{
    public:

        using string = std::string;
    
        static constexpr char operators[] = "+-*/%^>=<";
        static constexpr char whitespace_chars[] = "' ' ";

        class Exception
        {
            public:

                Exception() = default;

                Exception(const string& in_message)
                    : message(in_message)
                {}
            
                const std::string& what() const { return this->message; }

            private:

                std::string message;
        };

    public:

        Calculator() = default;

        Calculator(const string& in_expression)
            : expression(in_expression)
        {}

        void set_expression(const string& expression) noexcept { this->expression = expression; }

        string get_expression() const noexcept { return this->expression; }

        template <typename T>
        T evaluate() const
        { 
            try {
                const string postfix_expression = infix_to_postfix(expression);
                const T value = evaluate_postfix_expression<T>(postfix_expression);
                return value;
            } catch(Exception& e) { throw e; }
        }

    public:

        static bool is_operator(char c) noexcept
        {
            for (std::size_t i = 0; i < sizeof(operators); ++i)
                if (operators[i] == c)
                    return true;
            return false;
        }

        static bool is_parenthesis(char c) noexcept
        {
            return c == '(' || c == ')';
        }

        static bool is_whitespace(char c) noexcept
        {
            for (std::size_t i = 0; i < sizeof(whitespace_chars); ++i)
                if (whitespace_chars[i] == c)
                    return true;
            return false;
        }

        template <typename T>
        static T evaluate_basic_group(const T& left_operand, const T& right_operand, char oper)
        {
            switch (oper)
            {
                case '+':
                    return left_operand + right_operand;
                case '-':
                    return left_operand - right_operand;
                case '*':
                    return left_operand * right_operand;
                case '/':
                    return left_operand / right_operand;
                case '^':
                    return std::pow(left_operand, right_operand);
                case '%':
                    if constexpr (std::is_same<T, int>())
                        return left_operand % right_operand;
                    else
                        throw Exception{ "invalid operator '%' used for non integral type"};
                default:
                    throw Exception{ "invalid operator '" + oper + '\'' };
            }
        }

        template <typename T>
        static bool evaluate_basic_expression(std::string_view left_operand, std::string_view right_operand, std::string_view oper)
        {
            T l = T();
            T r = T();
            {
                std::istringstream iss(static_cast<std::string>(left_operand));
                iss >> l;
            }
            
            {
                std::istringstream iss(static_cast<std::string>(right_operand));
                iss >> r;
            }
            
            if (oper == "==")
                return l == r;
            else if (oper == ">=")
                return l >= r;
            else if (oper == ">")
                return l > r;
            else if (oper == "<=")
                return l <= r;
            else if (oper == "<")
                return l < r;
            else
                throw Exception{};
        }

        static bool evaluate_basic_bool_expression(std::string_view left_operand, std::string_view right_operand, std::string_view oper)
        {
            bool left_as_bool = left_operand.compare("true") ? true : false;
            bool right_as_bool = right_operand.empty() ? left_as_bool : right_operand.compare("true") ? true : false;
            
            if (oper.compare("and") == 0)
                return left_as_bool && right_as_bool;
            return left_as_bool || right_as_bool;
        }

        template <typename T>
        static T evaluate_postfix_expression(const string& postfix_expression)
            requires std::integral<T> || std::floating_point<T>
        {
            std::stack<string> operands;

            for (std::size_t i = 0; i < postfix_expression.size() - 1; ++i)
            {
                const char c = postfix_expression[i];

                if (is_whitespace(c))
                    continue;
                else if (!is_operator(c))
                {
                    std::istringstream iss(postfix_expression.substr(i));

                    try {
                        const string& operand = read_operand(iss);
                        operands.push(operand);
                        
                        if (operand.size() > 1)
                            i += operand.size() - 1;
                    } catch (const Exception& e) { throw e; }
                }
                else if (operands.size() >= 2)
                {
                    const string& str_r_operand = operands.top();
                    operands.pop();
                    const string& str_l_operand = operands.top();
                    operands.pop();
                    
                    std::istringstream iss(str_l_operand + " " + str_r_operand);
                    T l_operand = T();
                    T r_operand = T();
                    iss >> l_operand >> r_operand;
                    operands.push(std::to_string(evaluate_basic_group<T>(l_operand, r_operand, c)));
                }
            }

            std::istringstream iss(operands.top());
            T value = T();
            iss >> value;
            return value;
        }

        static unsigned short int precedence(char oper)
        {
            if (oper == '+' || oper == '-')
                return 0;
            else if (oper == '*' || oper == '/' || oper == '%')
                return 1;
            else if (oper == '^')
                return 2;
            else
                throw Exception{ "invalid operator" };
        }

        static bool has_higher_or_equal_precedence(char oper1, char oper2) noexcept
        {
            return precedence(oper1) >= precedence(oper2);
        }

        static string infix_to_postfix(const string& infix_expression)
        {
            string postfix_expression;
            std::stack<char> s;
            
            for (std::size_t i = 0; i < infix_expression.size(); ++i)
            {
                const char c = infix_expression[i];

                if (std::isalnum(c))
                {
                    std::istringstream iss(infix_expression.substr(i));
                    const string& operand = read_operand(iss);
                    postfix_expression += operand + " ";
                    
                    if (operand.size() > 1)
                        i += operand.size() - 1;
                }
                else if (c != '(' && c != ')')
                {
                    while (!s.empty() && s.top() != '(' && has_higher_or_equal_precedence(s.top(), c))
                    {
                        const string str(1, s.top());
                        postfix_expression += str + " ";
                        s.pop();
                    }
                    s.push(c);
                }
                else if (c == '(')
                    s.push(c);
                else if (c == ')')
                {
                    while (!s.empty() && s.top() != '(')
                    {
                        const string str(1, s.top());
                        postfix_expression += str + " ";
                        s.pop();
                    }
                    s.pop();
                }
            }

            while (!s.empty())
            {
                const string str(1, s.top());
                postfix_expression += str + " ";
                s.pop();
            }

            return postfix_expression;	
        }

        static string read_operand(std::istringstream& iss)
        {
            string operand;
    
            for (char c; iss.get(c);)
            {
                if (is_operator(c) || is_whitespace(c))
                    break;
                if (!is_parenthesis(c))
                    operand += c;
            }
            
            if (operand.empty())
                throw Exception{ "no number within given stream" };

            return operand;
        }

    private:

        string expression;
};

#endif