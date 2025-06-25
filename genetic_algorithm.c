/* genetic_algorithm.c - coordinates the steps of the genetic algorithm */
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

int genetic_algorithm(Constraints *constraints, int population, int iterations, int gen_steps, cl_context context, cl_command_queue commands, cl_program program, cl_mem row_matrices, cl_mem col_matrices, cl_mem seed_tables, cl_mem col_gen_buffer, cl_mem row_gen_buffer, cl_mem eval_tables, cl_mem evaluations, cl_mem copy_indices, cl_mem possibilities, cl_mem requirements) {
    /* Runs the genetic algorithm to optimise the gen matrices in the buffer */
    int error;
    // Thus we begin our genetic algorithm
    for (int index = 0; index < iterations; index++) { /* Run the algorithm for a fixed number of iterations */
        /* The genetic algorithm has four stages, in order being:
            -> Mutation - The values in the matricies and seeds are changed ever so slightly
            -> Generation - Timetables are generated using the matricies
            -> Evaluation - Timetables are evaluated using a cost function
            -> Replication - The better timetables are replicated using some algorithm I need to think of.
        */

        // Stage 1 - Mutation (Skip on the first iteration)
        if (index) { /* Only run mutation if not the first iteration */
            error = apply_mutations(constraints, population, program, commands, row_matrices, col_matrices, seed_tables); /* Mutate */
            if (error) { /* Handle error */
                printf("Sorry, something... err... an incident? occured during the mutation phase (don't panic)?\n");
                return 1; /* Return with error */
            }
        }

        // Stage 2 - Generation
        error = generate_timetables(constraints, population, gen_steps, program, commands, row_matrices, col_matrices, seed_tables, col_gen_buffer, row_gen_buffer, eval_tables, possibilities); /* Run the function that generates the timetables in the buffers */
        if (error) { /* Handle it */
            printf("Oopsie, something went horribly wrong while generating timetables... sorry?\n");
            return 1;
        }

        // Stage 3 - Evaluation
        // TODO: Evaluation

        // Stage 4 - Replication
        // TODO: Replication

        continue; /* Afterwards, I'll be using a goto label stack to handle errors */

    }

    return 0;

}
