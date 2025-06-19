/* timetabler is a program that can generate timetables based on certain constraints */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "parse_constraints.h"
#include "opencl_utils.h"
#include <CL/cl.h>

int main(int argc, char **argv) {
    /* Do any kind of testing/debugging from here */
    char *file_path = 0; if (argc - 1) file_path = argv[1]; /* Get first argument */ 
    if (!file_path) return 1; /* Only continue if a file has been provided */
    Constraints *constraints = parse_constraints_file(file_path); /* Do this */
    cl_device_id device = get_usable_gpu();
    char device_name[256]; size_t name_size; /* Stuff used to get the device name */
    clGetDeviceInfo(device, CL_DEVICE_NAME, 256, device_name, &name_size); /* Get device name */ 
    device_name[name_size] = 0; /* Add NT */
    printf("Our GPU is - %s!!!\n", device_name); /* Print the device name */
    return 0;
}
