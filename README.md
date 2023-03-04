# gvl
A small and simple, both dynamically and strongly typed interpreted language written solely in C++ from scratch. Still under development, long way to go.

So far gvl supports the very basics such as variable/constant initialization, variable assignment statements, standard I/O operations, if and while statements.

The language also supports the 'array' data structure which in reality is much closer to the way a python list behaves than the traditional array.
Meaning append/pop operations and the ability to store different data types within an array including other arrays.

Supported data types include: int, double, string, bool, array 

Examples will be added once the project is relatively finished, but for now most of the '.gvl' files in input_files/ demonstrate valid gvl code.

There is no real error detection/recovery yet, as far as syntax errors and run-time errors are conserned, but i'm working on providing one using the flex&yacc combo in the future.
