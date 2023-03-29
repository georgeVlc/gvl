CC = g++ 
CXXFLAGS = -std=c++20 -Wall -Werror -g
MODULES = modules/
OBJS = main.o $(MODULES)Parser.o $(MODULES)Interpreter.o
PROGRAM = gvl
INCLUDES = includes/
ARGS = input_files/errors.gvl


$(PROGRAM): $(OBJS)
	$(CC) $(OBJS) -o $(PROGRAM)


Parser.o: $(MODULES)Parser.cpp
	$(CC) -c $(CXXFLAGS) $(MODULES)Parser.cpp -I ../$(INCLUDES)


Interpreter.o: $(MODULES)Interpreter.cpp
	$(CC) -c $(CXXFLAGS) $(MODULES)Interpreter.cpp -I ../$(INCLUDES)


main.o: main.cpp
	$(CC) -c $(CXXFLAGS) main.cpp


clean:
	rm -f $(OBJS)


run: $(PROGRAM)
	./$(PROGRAM) $(ARGS)
	

runv: $(PROGRAM)
	valgrind ./$(PROGRAM) $(ARGS)