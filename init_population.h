/* header file for init_population.c */
#ifndef INIT_POPULATION_H
#define INIT_POPULATION_H
int init_program(Constraints *constraints, int population, cl_context *context_out, cl_program *program_out, cl_command_queue *commands_out, cl_mem *row_matrices_out, cl_mem *col_matrices_out, cl_mem *seed_tables_out, cl_mem *col_gen_out, cl_mem *row_gen_out, cl_mem *eval_table_out, cl_mem *evals_out, cl_mem *copy_table_out, cl_mem *possiblities_out, cl_mem *requirements_out);

#endif
