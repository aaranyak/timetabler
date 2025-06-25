/* random_generators_gpu.c has the code for prngs, etc. */

int stateless_prng(unsigned int seed) {
    /* Generates a pseudo-random int using the pcg prng (pcg-random.org) */
    seed = seed * 747796405 + 2891336453;
    seed = ((seed >> ((seed >> 28) + 4)) ^ seed) * 277803737;
    seed = (seed >> 22) ^ seed;
    seed = seed * 747796405 + 2891336453;
    seed = ((seed >> ((seed >> 28) + 4)) ^ seed) * 277803737;
    seed = (seed >> 22) ^ seed;
    return seed; /* Something I got off the internet */
}


__kernel void init_float_buffer(__global float *buffer, int random_seed) {
    /* Initialise this float buffer */
    int index = get_global_id(0); /* Get the index that we put this in */
    buffer[index] = (float)stateless_prng(index ^ random_seed) / 4294967295.0; /* I assure you this is random */
}
