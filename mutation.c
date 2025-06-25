/* mutation.c - implements the code for applying a mutation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "parse_constraints.h"
#include "opencl_utils.h"
#include "init_population.h"
#include "genetic_algorithm.h"
#include <CL/cl.h>
#include <unistd.h>
#include <time.h>

int apply_mutations(Constraints *constraints, int population, cl_program program, cl_command_queue commands, cl_mem row_matrices, cl_mem col_matrices, cl_mem seed_tables) {
    /* Slightly changes the values in the matrices and the seed tables */

    cl_int error; /* Use this for error handling */
    cl_event event_queue[2]; /* This will hold events for queuing the kernels */
    cl_kernel row_mutation = clCreateKernel(program, "apply_mutation", &error); /* Attempt to create a kernel here */
    if (error != CL_SUCCESS) goto create_row_mutation_error; /* Handle errors */
    cl_kernel col_mutation = clCreateKernel(program, "apply_mutation", &error); /* Attempt to create a kernel here */
    if (error != CL_SUCCESS) goto create_col_mutation_error; /* Handle errors */
    cl_kernel seed_mutation = clCreateKernel(program, "apply_mutation", &error); /* Attempt to create a kernel here */
    if (error != CL_SUCCESS) goto create_seed_mutation_error; /* Handle errors */

    // Set the kernel arguments
    int seed_1 = random(), seed_2 = random(), seed_3 = random(); /* Some random numbers */
    error = clSetKernelArg(row_mutation, 0, sizeof(cl_mem), &row_matrices);
    error |= clSetKernelArg(row_mutation, 1, sizeof(int), &seed_1);
    error |= clSetKernelArg(col_mutation, 0, sizeof(cl_mem), &col_matrices);
    error |= clSetKernelArg(col_mutation, 1, sizeof(int), &seed_2);
    error |= clSetKernelArg(seed_mutation, 0, sizeof(cl_mem), &seed_tables);
    error |= clSetKernelArg(seed_mutation, 1, sizeof(int), &seed_3);
    if (error != CL_SUCCESS) goto mutation_generic_error; /* Handle errors for all of the above */

    // Enqueue kernels
    const size_t row_size = population * constraints->num_sections * pow(constraints->num_sessions * constraints->num_possible, 2);
    const size_t col_size = population * constraints->num_sessions * pow(constraints->num_sections * constraints->num_possible, 2);
    const size_t seed_size = population * constraints->num_sections * constraints->num_sessions * constraints->num_possible;

    error = clEnqueueNDRangeKernel(commands, row_mutation, 1, 0, &row_size, 0, 0, 0, &event_queue[0]); /* Enqueue the first kernel */
    if (error != CL_SUCCESS) goto mutation_generic_error; /* Handle errors */
    error = clEnqueueNDRangeKernel(commands, col_mutation, 1, 0, &col_size, 0, 1, event_queue, &event_queue[1]); /*Queue second kernel*/
    if (error != CL_SUCCESS) goto mutation_generic_error; /* Handle errors */
    error = clEnqueueNDRangeKernel(commands, seed_mutation, 1, 0, &seed_size, 0, 2, event_queue, 0); /* Enqueue the third kernel */
    if (error != CL_SUCCESS) goto mutation_generic_error; /* Handle errors */

    clFinish(commands); /* Get done with mutating stuff */
    clReleaseKernel(row_mutation); clReleaseKernel(col_mutation); clReleaseKernel(seed_mutation); /* Genetic Engineering */

    return 0; /* No errors if it reaches here */

// Handle Errors
mutation_generic_error:
    clFinish(commands); /* Wait for the kernels to be run */
    clReleaseKernel(seed_mutation);
create_seed_mutation_error:
    clReleaseKernel(col_mutation);
create_col_mutation_error:
    clReleaseKernel(row_mutation); /* Free this */
create_row_mutation_error:
    return 1;
}
