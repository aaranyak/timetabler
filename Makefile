CC = gcc # Compiler
MAKE = make
CFLAGS = -g -Wno-misleading-indentation -Wno-deprecated-declarations -Wall # Compile Flags
LDFLAGS = -lm -lOpenCL# Linker FLags
NAME = timetabler

OBJECT_FILES = parse_constraints.o opencl_utils.o init_population.o # All the object files I want to link

all: $(NAME) clean # Build tasks

$(NAME): main.c $(OBJECT_FILES) # Compile executable
	$(CC) -o $(NAME) -no-pie $(CFLAGS) main.c $(OBJECT_FILES) $(LDFLAGS) # Compile and link

parse_constraints.o: parse_constraints.c
	$(CC) -c -o parse_constraints.o $(CFLAGS) parse_constraints.c

opencl_utils.o: opencl_utils.c
	$(CC) -c -o opencl_utils.o $(CFLAGS) opencl_utils.c

init_population.o: init_population.c
	$(CC) -c -o init_population.o $(CFLAGS) init_population.c

.PHONY: clean # Not a real target
clean: # Cleanup tasks
	rm $(OBJECT_FILES) # Delete all this stuff

