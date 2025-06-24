/* Header file for opencl_utis.h */
#ifndef OPENCL_UTILS_H
#define OPENCL_UTILS_H
#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>
char *load_source_string(char *path);
cl_device_id get_usable_gpu();
void get_gpu_name(cl_device_id gpu, char name[256]);
char *get_program_log(cl_program program, cl_device_id gpu);
#endif
