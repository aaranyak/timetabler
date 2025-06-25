/* Header file for genetic_algorithm.c */

// For convenience reasons, this is also the header file for
/*  -> generation.c
    -> mutation.c
    -> evaluation.c
    -> replication.c
*/
#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H
int generate_timetables(Constraints *constraints, int population, int iterations, cl_program program, cl_command_queue commands, cl_mem row_matrices, cl_mem col_matrices, cl_mem seed_tables, cl_mem col_gen_buffer, cl_mem row_gen_buffer, cl_mem eval_tables, cl_mem teach_table);

int apply_mutations(Constraints *constraints, int population, cl_program program, cl_command_queue commands, cl_mem row_matrices, cl_mem col_matrices, cl_mem seed_tables);

#endif
