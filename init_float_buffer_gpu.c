/* init_float_buffer_gpu.c, an opencl program that fills a buffer of floats with random numbers */

int stateless_prng(int seed) {
    /* Generates a pseudo-random int using the pcg prng (pcg-random.org) */
    

__kernel void init_float_buffer(float *buffer, int random_seed) {
    /* Initialise this float buffer */
    int index = get_global_id(0); /* Get the index that we put this in */
    buffer[index] = 2.718; /* I assure you this is random */
}
