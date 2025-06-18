CC = gcc # Compiler
MAKE = make
CFLAGS = -g -Wno-misleading-indentation -Wno-deprecated-declarations -Wall # Compile Flags
LDFLAGS = -lm # Linker FLags
NAME = timetabler

OBJECT_FILES = parse_constraints.o # All the object files I want to link

all: $(NAME) clean # Build tasks

$(NAME): main.c $(OBJECT_FILES) # Compile executable
	$(CC) -o $(NAME) -no-pie $(CFLAGS) main.c $(OBJECT_FILES) $(LDFLAGS) # Compile and link

parse_constraints.o: parse_constraints.c
	$(CC) -c -o parse_constraints.o $(CFLAGS) parse_constraints.c

.PHONY: clean # Not a real target
clean: # Cleanup tasks
	rm $(OBJECT_FILES) # Delete all this stuff

