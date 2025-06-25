/* generation.c - generates timetables from seeds and matrices */
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

int generate_timetables(Constraints *constraints, int population, int iterations, cl_program program, cl_command_queue commands, cl_mem row_matrices, cl_mem col_matrices, cl_mem seed_tables, cl_mem col_gen_buffer, cl_mem row_gen_buffer, cl_mem eval_tables, cl_mem teach_table) {
    /* This function generates timetables from the seeds using the matricies */

    // First, copy the seed tables into the col buffer, since we start with the rows
    cl_int error; /* Error handler */
    cl_event *event_queue = (cl_event*)malloc(sizeof(cl_event) * ((iterations*2) + 2)); /* This is the event queue, that holds events */
    int event_queue_size = 0; /* The current size of the event queue */
    
    // Set up kernels
    cl_kernel row_generator = clCreateKernel(program, "apply_row_matrix", &error); /* Attempt to create a kernel here */ 
    if (error != CL_SUCCESS) goto create_row_gen_kernel_error; /* If something goes wrong */
    cl_kernel col_generator = clCreateKernel(program, "apply_col_matrix", &error); /* Attempt to create a kernel here */ 
    if (error != CL_SUCCESS) goto create_col_gen_kernel_error; /* If something goes wrong */
    cl_kernel eval_generator = clCreateKernel(program, "generate_max_table", &error); /* Attempt to create the kernel for evals */
    if (error != CL_SUCCESS) goto create_eval_gen_kernel_error; /* If something goes wrong */

    // Set kernel arguments
    error = clSetKernelArg(row_generator, 0, sizeof(cl_mem), &col_gen_buffer); /* Seting the arguments for the row gen kernel */
    error |= clSetKernelArg(row_generator, 1, sizeof(cl_mem), &row_gen_buffer);
    error |= clSetKernelArg(row_generator, 2, sizeof(cl_mem), &row_matrices);
    error |= clSetKernelArg(row_generator, 3, sizeof(int), &population);
    error |= clSetKernelArg(row_generator, 4, sizeof(int), &(constraints->num_sessions));
    error |= clSetKernelArg(row_generator, 5, sizeof(int), &(constraints->num_sections));
    error |= clSetKernelArg(row_generator, 6, sizeof(int), &(constraints->num_possible));
    error |= clSetKernelArg(col_generator, 0, sizeof(cl_mem), &row_gen_buffer); /* Seting the arguments for the col gen kernel */
    error |= clSetKernelArg(col_generator, 1, sizeof(cl_mem), &col_gen_buffer);
    error |= clSetKernelArg(col_generator, 2, sizeof(cl_mem), &col_matrices);
    error |= clSetKernelArg(col_generator, 3, sizeof(int), &population);
    error |= clSetKernelArg(col_generator, 4, sizeof(int), &(constraints->num_sessions));
    error |= clSetKernelArg(col_generator, 5, sizeof(int), &(constraints->num_sections));
    error |= clSetKernelArg(col_generator, 6, sizeof(int), &(constraints->num_possible));
    error |= clSetKernelArg(eval_generator, 0, sizeof(cl_mem), &col_gen_buffer); /* Seting the arguments for the eval gen kernel */
    error |= clSetKernelArg(eval_generator, 1, sizeof(cl_mem), &eval_tables);
    error |= clSetKernelArg(eval_generator, 2, sizeof(cl_mem), &teach_table);
    error |= clSetKernelArg(eval_generator, 3, sizeof(int), &population);
    error |= clSetKernelArg(eval_generator, 4, sizeof(int), &(constraints->num_sessions));
    error |= clSetKernelArg(eval_generator, 5, sizeof(int), &(constraints->num_sections));
    error |= clSetKernelArg(eval_generator, 6, sizeof(int), &(constraints->num_possible));
    if (error != CL_SUCCESS) goto generate_timetables_other_error; /* Error handling */
    
    // Time to copy all the data from the seeds to the col buffer
    error |= clEnqueueCopyBuffer(commands, seed_tables, col_gen_buffer, 0, 0, population * constraints->num_sessions * constraints->num_sections * constraints->num_possible, 0, 0, &event_queue[0]); event_queue_size++; /* Copy the seeds into the first gen buffer */
    if (error != CL_SUCCESS) goto generate_timetables_other_error; /* Error handling */
    const size_t row_work_sizes[2] = {population * constraints->num_sections, constraints->num_sessions * constraints->num_possible};
    const size_t col_work_sizes[2] = {population * constraints->num_sessions, constraints->num_sections * constraints->num_possible};
    // Now, time to enqueue the kernels - we generate for a specific number of iterations
    for (int i = 0; i < iterations; i++) { /* For this number of iterations */
        error = clEnqueueNDRangeKernel(commands, row_generator, 2, 0, row_work_sizes, 0, event_queue_size, event_queue, &event_queue[event_queue_size]); event_queue_size++; /* Enqueue the kernel and add to the event queue */
        if (error != CL_SUCCESS) goto generate_timetables_other_error; /* Error handling */
        error = clEnqueueNDRangeKernel(commands, col_generator, 2, 0, col_work_sizes, 0, event_queue_size, event_queue, &event_queue[event_queue_size]); event_queue_size++; /* Enqueue the kernel and add to the event queue */
        if (error != CL_SUCCESS) goto generate_timetables_other_error; /* Error handling */
    } /* Done enqueueing all the generator kernels */
    
    // Enqueue the kernel for creating the eval tables
    const size_t eval_table_size = population * constraints->num_sessions * constraints->num_sections; /* Number of work items in eval */
    error = clEnqueueNDRangeKernel(commands, eval_generator, 1, 0, &eval_table_size, 0, event_queue_size, event_queue, &event_queue[event_queue_size]); event_queue_size++; /* Enqueue the kernel and add to the event queue */
    if (error != CL_SUCCESS) goto generate_timetables_other_error; /* Error handling */
    
    clFinish(commands); /* Wait for this to be done */

    // Clean up
    clReleaseKernel(row_generator); clReleaseKernel(col_generator); clReleaseKernel(eval_generator); /* Free this stuff */
    free(event_queue); /* Free this thing */
    return 0;
generate_timetables_other_error:
    clFinish(commands); /* make sure all kernels are finished */
    clReleaseKernel(eval_generator);
create_eval_gen_kernel_error:
    clReleaseKernel(col_generator);
create_col_gen_kernel_error:
    clReleaseKernel(row_generator); /* Free this thing */
create_row_gen_kernel_error:
    free(event_queue);
    return 1;

}
