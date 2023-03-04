%option noyywrap

%{
	#include <iostream>
    #include <string>
    //#include "../includes/basic_types.hpp"

    std::size_t line_no = 1;
%}

ws [ \t]+
alpha [A-Za-z]
dig   [0-9]
float {dig}"."({dig}+)
ops "+"|"-"|"*"|"^"|"="
bops "<"|">"|"=="|"!="|"<="|">="
fmops "nl"|"tab"|"space"

quote "'"

name "_"*{alpha}({alpha}|{dig}|"_")*

str {quote}({name}|{dig}|{name}{dig}|{dig}{name})*{quote}

token {name}|{dig}|{float}|{ops}|{bops}|{fmops}|{str}

init ("var"|"const"){ws}{name}{ws}"="{ws}({token}({ws}{token}){0,2})|("$array_at"{ws}{name}{ws}{dig})|("$array_len"{ws}{name})|("$array_pop"{ws}{name})

assign {name}{ws}"="{ws}({token}({ws}{token}){0,2})|("$array_at"{ws}{name}{ws}{dig})|("$array_len"{ws}{name})|("$array_pop"{ws}{name})

array_pop "$array_pop"{ws}{name}

array_init "var[]"{ws}{name}{ws}"="{ws}("["({ws}{token}){0,3}{ws}"]"|("$array_at"{ws}{name}{ws}{dig})|{array_pop})

array_set "$array_set"{ws}{name}{ws}{dig}+{ws}({name}|{token})

array_append "$array_append"{ws}{name}{ws}({name}|{token})

print "print"({ws}{token}){1,3}

println "println"({ws}{token}){1,3}

readint "readint"({ws}{token}){1,3}

readfloat "readfloat"({ws}{token}){1,3}

readstr "readstr"({ws}{token}){1,3}

while "while"{ws}{token}({ws}{token}){0,2}{ws}"{"

if "if"{ws}{token}({ws}{token}){0,2}{ws}"{"

else "else"{ws}"{"


%%

{array_pop} { std::cout << line_no << " " << YYText() << " -> array_pop\n"; }

{init} { std::cout << line_no << " " << YYText() << " -> init\n"; }

{array_init} { std::cout << line_no << " " << YYText() << " -> array_init\n"; }

{array_set} { std::cout << line_no << " " << YYText() << " -> array_set\n"; }

{array_append} { std::cout << line_no << " " << YYText() << " -> array_append\n"; }

{assign} { std::cout << line_no << " " << YYText() << " -> assign\n"; }

{print} { std::cout << line_no << " " << YYText() << " -> print\n"; }

{println} { std::cout << line_no << " " << YYText() << " -> println\n"; }

{readint} { std::cout << line_no << " " << YYText() << " -> readint\n"; }

{readfloat} { std::cout << line_no << " " << YYText() << " -> readfloat\n"; }

{readstr} { std::cout << line_no << " " << YYText() << " -> readstr\n"; }

{while} { std::cout << line_no << " " << YYText() << " -> while\n"; }

{if} { std::cout << line_no << " " << YYText() << " -> if\n"; }

{else} { std::cout << line_no << " " << YYText() << " -> else\n"; }

{ws}|"}"

\n { ++line_no; }

. {
    std::string error_seg(YYText());
    error_seg.reserve(20);
    char c;
    
    while ((c = yyinput()) != 0 && c != EOF)
    {
        if (c == '\n')
        {
            unput(c);
            break;
        }
        error_seg += c;
    }

    std::cout << "invalid syntax at line: " << line_no << " -> " 
    << " \033[1;31m'" << error_seg << "'\033[0m\n";
    error_seg.clear();
}

%%


int main(int argc, char** argv)
{
    FlexLexer* lexer = new yyFlexLexer;
    lexer->yylex();
}