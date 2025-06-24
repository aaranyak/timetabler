/* timetabler is a program that can generate timetables based on certain constraints */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "parse_constraints.h"
#include "opencl_utils.h"
#include "init_population.h"
#include <CL/cl.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char **argv) {
    /* Do any kind of testing/debugging from here */
    char *file_path = 0; if (argc - 1) file_path = argv[1]; /* Get first argument */ 
    srandom(time(NULL)); /* INIT PRNG */
    if (!file_path) return 1; /* Only continue if a file has been provided */
    Constraints *constraints = parse_constraints_file(file_path); /* Do this */
    cl_context context; cl_program program; cl_command_queue commands; /* Otherthings that are not buffers */
    cl_mem row_matrices, col_matrices, seed_tables, row_gen_buffer, col_gen_buffer, eval_tables, evaluations, copy_indices, possiblities, requirements; /* These are all the buffers that get outed */
    int error = init_program(constraints, 10, &context, &program, &commands, &row_matrices, &col_matrices, &seed_tables, &col_gen_buffer, &row_gen_buffer, &eval_tables, &evaluations, &copy_indices, &possiblities, &requirements); /* Use this function to get that stuff */
    printf("Is there any error? - %s\n", error ? "Yes" : "No");
    return 0;
}
