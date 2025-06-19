/* init_population.c - initialises the population of timetables for the genetic algorithm */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void init_population(Constraints *constraints, float **rows_pop_out, float **cols_pop_out, float **seed_out, float population) {
    /* This function randomly initialises a fixed-size population of row generators, column generators and seeds 
     * The row generators and column generators are a matrix with a size of (length * possiblities)^2.
     * The seed is a table of row * col * possiblities, and is used to begin the generation
     */
    // The sizes of the sets of generator matrices, index them as so - population[index][row/col][col/row 1][combo 1][col/row 2][combo 2];
    int row_pop_size = population * pow((constraints->num_sessions * constraints->num_possible), 2) * constraints->num_sections;
    int col_pop_size = population * pow((constraints->num_sections * constraints->num_possible), 2) * constraints->num_sessions;
    int seed_pop_size = population * constraints->num_sections * constraints->num_possible * constraints->num_sessions; /* :) */
    float *row_matrices = (float*)malloc(sizeof(float) * row_pop_size); /* This is the array that you have to index weirdly */
    float *col_matrices = (float*)malloc(sizeof(float) * col_pop_size); /* Don't worry I'll create an inline to handle that */
    float *seed_tables = (float*)malloc(sizeof(float) * seed_pop_size); /* This is a slightly easier one to handle, luckily */
   
    // Now use the power of your GPU just fill these buffers with random numbers!!!!
