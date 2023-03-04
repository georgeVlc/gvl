stmt_type: get_stmt_type.yy.o
	g++ get_stmt_type.yy.o -o stmt_type;

get_stmt_type.yy.cc: get_stmt_type.l
	flex++ -o get_stmt_type.yy.cc get_stmt_type.l ;

stmt_type.o: get_stmt_type.yy.cc
	g++ -c -std=c++20 -Wall -Werror -g get_stmt_type.yy.cc;

clean_get_stmt_type:
	rm *get_stmt_type.yy*

get_stmt_type: stmt_type
	./stmt_type < stmts2.txt
