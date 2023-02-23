# gvl
A small and simple, both dynamically and strongly typed interpreted language written solely in C++ from scratch. Still under development, long way to go.

So far gvl supports the very basics such as variable/constant initialization, variable assignment statements, basic I/O operations, if and while statements.

The language also supports the 'array' data structure which in reality is much closer to the way a python list behaves than the traditional array.
Meaning append/pop operations and the ability to store different data types including nested arrays.

Supported data types include: int, float, array 
(there is no string or bool support but soon will be).

In order to initialize a variable or an array the user doesn't declare the type but simple uses the keyword 'var' ('var[]' in the case of an array init), followed by the variable/array name and some sort of value (to avoid invalid read/writes, ensure correct use and avoid type conflicts).

Examples will be added once the project is relatively finished, but for now all the '.gvl' files in input_files/ demonstrate valid gvl code.
