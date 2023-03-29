# gvl
A very small and simple, dynamically typed, interpreted language written solely in C++20(ish) from scratch.

So far gvl supports the very basics such as variable/constant initialization, variable assignment statements, standard I/O operations, if and while statements.

The language also supports the 'array' data structure which in reality is much closer to the way a python list behaves than the traditional array.
Meaning append/pop operations and the ability to store different data types within an array including other arrays.

Supported data types include: int, double, string, bool, array 

Examples will be added once the project is relatively finished, but for now most of the '.gvl' files in input_files/ demonstrate valid gvl code.

So far there is some minimal syntx error detection but no run-time error checking yet.