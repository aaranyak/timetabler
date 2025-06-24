/* init_population.c - initialises the population of timetables for the genetic algorithm */
#include <stdio.h>
#include "parse_constraints.h"
#include "opencl_utils.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <CL/cl.h>

int init_program(Constraints *constraints, int population, cl_context *context_out, cl_program *program_out, cl_command_queue *commands_out, cl_mem *row_matrices_out, cl_mem *col_matrices_out, cl_mem *seed_tables_out, cl_mem *col_gen_out, cl_mem *row_gen_out, cl_mem *eval_table_out, cl_mem *evals_out, cl_mem *copy_table_out, cl_mem *possiblities_out, cl_mem *requirements_out) {
    /* This initialises all the required stuff for the genetic algorithm.
        -> Setting up a compute context and queue for the GPU Kernels
        -> Compiling all opencl programs required.
        -> Setting up all memory required for the program
         -> Row and Column Matrix Buffers
         -> Seed Tables, Col Generation, and Row Generation
         -> Evaluation Tables
         -> Possibility Decoding Tables
         -> Requirements Table
        -> Fill buffers w random stuff
     */

    cl_int error; /* Use for error checking */
    // Get the GPU, and Create the program context.
    cl_device_id gpu = get_usable_gpu(); /* Get the first GPU you find, so that we can use it */ 
    cl_context context = clCreateContext(0, 1, &gpu, 0, 0, &error); /* Attempt a context creation */ 
    if (error != CL_SUCCESS) { /* Check if this worked */
        printf("Sorry, couldn't create the CL Context.\n");
        goto init_cl_context_error; /* Cleanup stage */
    }
    cl_command_queue commands = clCreateCommandQueue(context, gpu, 0, &error); /* Create a command queue */
    if (error != CL_SUCCESS) { /* Error handling */
        printf("Sorry, failed to create command queue\n");
        goto init_cl_queue_error; /* Cleanup */
    }

    // Compile programs
    char *random_generators_source = load_source_string("random_generators_gpu.c"); /* PRNG, Initialisations, Mutations */
    const char *sources[1] = {random_generators_source}; /* Array holding all programs */
    const size_t lengths[1] = {strlen(random_generators_source) - 1}; /* Lengths of all programs */
    unsigned int num_sources = 1; /* Number of source files that go in the program */
    cl_program program = clCreateProgramWithSource(context, num_sources, sources, lengths, &error); /* Create the program */
    if (error != CL_SUCCESS) {
        free(random_generators_source); /* Make sure to do this */
        printf("Sorry, couldn't load the program sources\n");
        goto load_sources_error;
    }
    error = clBuildProgram(program, 1, &gpu, "", 0, 0); /* Compeel the program */
    if (error != CL_SUCCESS) { /* If da program failed to compile, register a formal complaint to the great kaktus of the sky for it */
        free(random_generators_source); /* Don't forget this */
        printf("Sorry, your gpu code was bad - here is what went wrong\n");
        char *program_log = get_program_log(program, gpu); /* Get da log */
        printf("%s\n", program_log); free(program_log); /* Print it, free it */
        goto create_row_matrix_error; /* Clean up */
    }
    free(random_generators_source); /* Free source strings */
    // Matrix Buffers and Seed Tables

    int row_pop_size = population * constraints->num_sections * pow(constraints->num_sessions * constraints->num_possible, 2); /* This is the size of the buffer containing row matrices  */
    int col_pop_size = population * constraints->num_sessions * pow(constraints->num_sections * constraints->num_possible, 2); /* This is the size of the buffer containing col matrices  */
    int seed_pop_size = population * constraints->num_sessions * constraints->num_sections * constraints->num_possible; /* Size of the buffer containing the seed tables (that the matrices operate on) */

    cl_mem row_matrices = clCreateBuffer(context, CL_MEM_READ_WRITE, row_pop_size * sizeof(float), 0, &error); /* Optimise sessions */
    if (error != CL_SUCCESS) { /* If something went wrong here */
        printf("Sorry, failed to create the row matrix\n");
        goto create_row_matrix_error;
    } /* This is slightly irritating, but is the most reasonable debugging method I know of */
    cl_mem col_matrices = clCreateBuffer(context, CL_MEM_READ_WRITE, col_pop_size * sizeof(float), 0, &error); /* Optimise collisions */
    if (error != CL_SUCCESS) { /* If something went wrong here */
        printf("Sorry, failed to create the col matrix\n");
        goto create_col_matrix_error;
    }
    cl_mem seed_tables = clCreateBuffer(context, CL_MEM_READ_WRITE, seed_pop_size * sizeof(float), 0, &error); /* Optimise this stuff */
    if (error != CL_SUCCESS) { /* If something went wrong here */
        printf("Sorry, failed to create the seed table\n");
        goto create_seed_table_error;
    }


    // Create Gen Buffers
    cl_mem col_gen_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, seed_pop_size * sizeof(float), 0, &error); /* Generators */
    if (error != CL_SUCCESS) { /* Error handling */
        printf("Sorry, failed to create a generator buffer\n");
        goto create_col_buffer_error;
    }
    cl_mem row_gen_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, seed_pop_size * sizeof(float), 0, &error); /* Generators */
    if (error != CL_SUCCESS) { /* Error handling */
        printf("Sorry, failed to create a generator buffer\n");
        goto create_row_buffer_error;
    }


    // Create Eval Buffers
    cl_mem eval_table = clCreateBuffer(context, CL_MEM_READ_WRITE, constraints->num_sessions * constraints->num_sections * sizeof(uint64_t) * population, 0, &error); /* The table of resultant timetables */
    if (error != CL_SUCCESS) {
        printf("Sorry, failed to create the eval-table buffer\n");
        goto create_eval_table_error;
    }
    cl_mem evaluations = clCreateBuffer(context, CL_MEM_WRITE_ONLY, population * sizeof(float), 0, &error); /* Output evaluations */
    if (error != CL_SUCCESS) {
        printf("Sorry, failed to create the eval output buffer\n");
        goto create_evaluations_buffer_error;
    }

    // Create Copy Buffers
    cl_mem copy_indices = clCreateBuffer(context, CL_MEM_READ_ONLY, population * sizeof(int), 0, &error); /* Output evaluations */
    if (error != CL_SUCCESS) {
        printf("Sorry, failed to create the copy indices buffer\n");
        goto create_copy_buffer_error;
    }

    // Create buffers for requirements and possiblity tables
    cl_mem possiblities = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, constraints->num_possible * sizeof(uint64_t), constraints->teach_table, &error); /* This contains the teach table */
    if (error != CL_SUCCESS) {
        printf("Sorry, failed to create the teach table buffer\n");
        goto create_teach_table_error;
    }
    cl_mem requirements = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, constraints->num_sections * constraints->num_subjects, constraints->sessions_table, &error); /* This contains the sessions table */
    if (error != CL_SUCCESS) {
        printf("Sorry, failed to create the requirements table buffer\n");
        goto create_sessions_table_error;
    }

    // Initialise Matrices With Random Values
    
    // Create kernels that will do so.
    cl_kernel init_row_matrices = clCreateKernel(program, "init_float_buffer", &error); /* Create a kernel */
    if (error != CL_SUCCESS) goto create_row_matrix_kernel_error; /* Error check */
    cl_kernel init_col_matrices = clCreateKernel(program, "init_float_buffer", &error); /* Create a kernel */
    if (error != CL_SUCCESS) goto create_col_matrix_kernel_error; /* Error check */
    cl_kernel init_seed_tables = clCreateKernel(program, "init_float_buffer", &error); /* Create a kernel */
    if (error != CL_SUCCESS) goto create_seed_table_kernel_error; /* Error check */
    
    int seed_1 = random(), seed_2 = random(), seed_3 = random(); /* Random seeds for initing these things */
    error = clSetKernelArg(init_row_matrices, 0, sizeof(cl_mem), &row_matrices); /* Set the argument of this thing */
    error |= clSetKernelArg(init_col_matrices, 0, sizeof(cl_mem), &col_matrices); /* Add the buffer as first argument */
    error |= clSetKernelArg(init_seed_tables, 0, sizeof(cl_mem), &seed_tables); /* Since CL_SUCCESS = 0, we can take advantage */
    error |= clSetKernelArg(init_row_matrices, 1, sizeof(int), &seed_1); /* of that to use this debugging pattern */
    error |= clSetKernelArg(init_col_matrices, 1, sizeof(int), &seed_2); /* in situations where no new memory is allocated */
    error |= clSetKernelArg(init_seed_tables, 1, sizeof(int), &seed_3); /* so we don't need to repeat goto flags to much. */
    if (error != CL_SUCCESS) { /* I should have thought of that ages ago */
        printf("Error setting the arguments to the initialization kernels\n");
        goto set_init_kernel_args_error;
    }
    const size_t row_init_size = (size_t)row_pop_size, col_init_size = (size_t)col_pop_size, seed_init_size = (size_t)seed_pop_size;
    cl_event event_list[2]; /* Events that will be waited for */

    // Time to ENQUEUE those kernels
    error = clEnqueueNDRangeKernel(commands, init_row_matrices, 1, 0, &row_init_size, 0, 0, 0, &event_list[0]); /* Enqueue the first */
    if (error != CL_SUCCESS) {
        printf("Something went badly wrong in enqueuing the row matrices kernel\n");
        goto set_init_kernel_args_error;
    }
    error = clEnqueueNDRangeKernel(commands, init_col_matrices, 1, 0, &col_init_size, 0, 1, event_list, &event_list[1]); /* for cols */
    if (error != CL_SUCCESS) {
        printf("Something went badly wrong in enqueuing the col matrices kernel\n");
        goto set_init_kernel_args_error;
    }

    error = clEnqueueNDRangeKernel(commands, init_seed_tables, 1, 0, &seed_init_size, 0, 2, event_list, 0); /* Ditto for the seeds */
    if (error != CL_SUCCESS) {
        printf("Something went badly wrong in enqueuing the seed tables kernel\n");
        goto set_init_kernel_args_error;
    }

    error = clFinish(commands); /* Wait for the kernels to have executed */
    if (error != CL_SUCCESS) {
        printf("During the init stage, something went RONGEDY RONG RONG.\n");
        goto set_init_kernel_args_error;
    }

    // Clean up
    clReleaseKernel(init_row_matrices); clReleaseKernel(init_col_matrices); clReleaseKernel(init_seed_tables); /* Free these kernels */

    // Return all the going-to-be-used data
    *context_out = context; *program_out = program; *commands_out = commands; /* Return this stuff */
    *row_matrices_out = row_matrices; *col_matrices_out = col_matrices; *seed_tables_out = seed_tables; /* Ditto here */
    *col_gen_out = col_gen_buffer; *row_gen_out = row_gen_buffer; *eval_table_out = eval_table; /* Out some more buffers */
    *evals_out = evaluations; *copy_table_out = copy_indices; *possiblities_out = possiblities; *requirements_out = requirements;

    // FINALLY!!!

    return 0; /* Don't go forward */

    // Error Handling
set_init_kernel_args_error:
    clReleaseKernel(init_seed_tables);
create_seed_table_kernel_error:
    clReleaseKernel(init_col_matrices);
create_col_matrix_kernel_error:
    clReleaseKernel(init_row_matrices);
create_row_matrix_kernel_error:
    clReleaseMemObject(requirements);
create_sessions_table_error:
    clReleaseMemObject(possiblities);
create_teach_table_error:
    clReleaseMemObject(copy_indices);
create_copy_buffer_error:
    clReleaseMemObject(evaluations);
create_evaluations_buffer_error:
    clReleaseMemObject(eval_table);
create_eval_table_error:
    clReleaseMemObject(row_gen_buffer);
create_row_buffer_error:
    clReleaseMemObject(col_gen_buffer);
create_col_buffer_error:
    clReleaseMemObject(seed_tables);
create_seed_table_error:
    clReleaseMemObject(col_matrices);
create_col_matrix_error:
    clReleaseMemObject(row_matrices);
create_row_matrix_error:
    clReleaseProgram(program); /* Clean this up */
load_sources_error: /* In case the the source loading fails */
    clReleaseCommandQueue(commands); /* Freedom! */
init_cl_queue_error: /* In case the queue init fails */
    clReleaseContext(context); /* Free this thing */
init_cl_context_error: /* In case the context init fails */
    return 1; /* Something went rong */
}
