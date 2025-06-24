/* opencl_utils.c - some utilities to run opencl kernels */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opencl_utils.h"
#include <CL/cl.h>

char *load_source_string(char *path) {
    /* Loads an opencl file as a string */
    FILE *handle = fopen(path, "r"); /* Open the file */
    if (!handle) return 0; /* Here */
    int length; for (length = 0; !feof(handle); length++) fgetc(handle); rewind(handle); /* Calculate the length of the file */
    char *string = (char*)malloc(length + 1); string[length] = 0; /* String with null terminator */
    for (int i = 0; i < length; i++) string[i] = fgetc(handle); /* Load the stuff into the file */
    fclose(handle); /* Close the file */
    return string; /* Here you go */
}

cl_device_id get_usable_gpu() {
    /* Scans for devices in the system that are gpus */
    cl_int status; /* Checking return status */
    cl_device_id device_id; /* The device id we return */
    status = clGetDeviceIDs(0, CL_DEVICE_TYPE_GPU, 1, &device_id, 0); /* Get the first gpu we find */
    if (status == CL_SUCCESS) return device_id; /* Get the device */
    printf("Sorry, a gpu couldn't be loaded\n"); /* Error */
    return 0;
}

void get_gpu_name(cl_device_id gpu, char name[256]) {
    /* Get the name of a gpu */
    size_t name_size; /* Stuff used to get the device name */
    clGetDeviceInfo(gpu, CL_DEVICE_NAME, 256, name, &name_size); /* Get device name */
    name[name_size] = 0; /* Add NT */
}

char *get_program_log(cl_program program, cl_device_id gpu) {
    /* Get the build log */
    size_t log_size; clGetProgramBuildInfo(program, gpu, CL_PROGRAM_BUILD_LOG, 0, 0, &log_size); /* Get size */
    char *log_str = (char*)malloc(log_size); /* Dynamically allocate string */
    clGetProgramBuildInfo(program, gpu, CL_PROGRAM_BUILD_LOG, log_size, log_str, 0); /* Get data */
    return log_str;
}
