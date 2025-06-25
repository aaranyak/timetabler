/* generation_gpu.c - contains kernels used for generating the timetable */

// Kernels that apply the matrix on a column (this is a 2d kernel)
/* First dimension is the index of the row/col in the population
 * Second dimension is the index of the possibility in the row/col
*/

/* Whenever indexing the table buffers, the dimensions are {population, sections, sessions, possibilities}
 * Whenever indexing the row matrices the dimensions are {population, row, col-to, possibility-to, col-from possibility-from}
 * for col matrices, the dimensions are {population, col, row-to, possibility-to, row-from, possibility-from}
*/


__kernel void apply_row_matrix(__global float *source, __global float *destination, __global float *row_matrix, int population, int sessions, int sections, int possibilities) {
    /* This applies the row matrix transporting from the col buffer to the row buffer */
    int row_index = get_global_id(0); /* Get the index of the row in the population */
    int dest_index = get_global_id(1); /* Get the index of the possibility in the row */ 
    float weighted_sum = 0; /* We add the stuff to this number here */
    int row_length = sessions * possibilities; /* This is for indexing purposes */
    int matrix_offset = (row_length * row_index + dest_index) * row_length; /* Use this for indexing the matrix */
    int table_offset = row_length * row_index; /* Offset in the tables that we index from */
    
    // This is part of a parallelised matrix multiplication.
    for (int from_index = 0; from_index < row_length; from_index++)  /* Loop through all the cells in the row */
        weighted_sum += source[table_offset + from_index] * row_matrix[matrix_offset + from_index]; /* Do the multiplication */
    
    destination[table_offset + dest_index] = weighted_sum; /* Output the result */
}

__kernel void apply_col_matrix(__global float *source, __global float *destination, __global float *col_matrix, int population, int sessions, int sections, int possibilities) {
    /* This applies the column matrix transporting from the row buffer to the col buffer */
    int col_index = get_global_id(0); /* Get the index of the target column */
    int dest_index = get_global_id(1); /* Get the index of the cell in the column */
    float weighted_sum = 0; /* Accumulator of sorts */
    int col_length = sections * possibilities; /* This is used for calculating indices */
    int matrix_offset = (col_length * col_index + dest_index) * col_length; /* This is used for indexing the matrix */
    int table_offset = col_index - col_index % sessions; /* Calculate the offset of the table in the population */
    int row_length = sessions * possibilities; /* Necessary for indexing the source table */
    int col_offset = (col_index % sessions) * possibilities; /* Faster indexing */
    // The parellalised matrix multiplication
    for (int from_index = 0; from_index < col_length; from_index++) /* Loop through all the cells in a column */
        weighted_sum += source[table_offset + (from_index - from_index % possibilities) * row_length + col_offset + from_index % possibilities] = col_matrix[matrix_offset + from_index]; /* Indexing the source is difficult, since rows come before columns */

    destination[table_offset + (dest_index - dest_index % possibilities) * row_length + col_offset + dest_index % possibilities] = weighted_sum; /* Deja vu, isn't it */
}

__kernel void generate_max_table(__global float *source, __global float *destination, __global ulong *teach_table, int population, int sessions, int sections, int possibilities) {
    /* This converts a set of possible timetables into the maximum timetable */

    int cell_id = get_global_id(0); /* The cell in the entire population */
    int source_offset = cell_id * possibilities; /* The position of the cell in the source */

    float max_value = source[source_offset]; /* Sample the first as the maximum value */
    int max_index = 0; /* Sync this with the max value */
    for (int index = 0; index < possibilities; index++) { /* Loop through all the possibilities in the cell */
        float value = source[source_offset + index]; /* Get the value in that position */
        if (value > max_value) { /* If this is better */
            max_index = index; max_value = value; /* update the maxima */
        }
    }

    destination[cell_id] = teach_table[max_index]; /* And set the value in the destination */
}
