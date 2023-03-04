%option noyywrap

%{
	#include <iostream>

	int line_no = 0;
%}

ws [ \t]+
alpha [A-Za-z]
dig   [0-9]
name ({alpha}|\$)({alpha}|{dig}|[_.\-/$])* 


%%


{name} { std::cout << "line:" << line_no << "\t" << YYText() << " is valid\n"; }

\n { ++line_no; }

. {
    char c;
    while ((c = yyinput()) != 0)
    {
        if (c == '\n')
        {
            ++line_no;
            break;
        }
    }
  }


%%


int main(int argc, char** argv)
{
    FlexLexer* lexer = new yyFlexLexer;
    lexer->yylex();
}